
#include "nest/bird.h"
#include "lib/ip.h"
#include "lib/net.h"

const u16 net_addr_length[] = {
  [NET_IP4] = sizeof(net_addr_ip4),
  [NET_IP6] = sizeof(net_addr_ip6),
  [NET_VPN4] = sizeof(net_addr_vpn4),
  [NET_VPN6] = sizeof(net_addr_vpn6)
};

int
net_format(const net_addr *N, char *buf, int buflen)
{
  net_addr_union *n = (void *) N;

  /* FIXME: quick hack */
  switch (n->n.type)
  {
  case NET_IP4:
    return bsnprintf(buf, buflen, "%I/%d", n->ip4.prefix, n->ip4.pxlen);
  case NET_IP6:
    return bsnprintf(buf, buflen, "%I/%d", n->ip6.prefix, n->ip6.pxlen);
  case NET_VPN4:
    return bsnprintf(buf, buflen, "%u:%u %I/%d", (u32) (n->vpn4.rd >> 32), (u32) n->vpn4.rd, n->vpn4.prefix, n->vpn4.pxlen);
  case NET_VPN6:
    return bsnprintf(buf, buflen, "%u:%u %I/%d", (u32) (n->vpn6.rd >> 32), (u32) n->vpn6.rd, n->vpn6.prefix, n->vpn6.pxlen);
  }

  return 0;
}


ip_addr
net_pxmask(const net_addr *a)
{
  switch (a->type)
  {
  case NET_IP4:
  case NET_VPN4:
    return ipa_from_ip4(ip4_mkmask(net4_pxlen(a)));

  case NET_IP6:
  case NET_VPN6:
    return ipa_from_ip6(ip6_mkmask(net6_pxlen(a)));

  default:
    return IPA_NONE;
  }
}


static inline int net_validate_ip4(const net_addr_ip4 *n)
{
  return (n->pxlen <= IP4_MAX_PREFIX_LENGTH) &&
    ip4_zero(ip4_and(n->prefix, ip4_not(ip4_mkmask(n->pxlen))));
}

static inline int net_validate_ip6(const net_addr_ip6 *n)
{
  return (n->pxlen <= IP6_MAX_PREFIX_LENGTH) &&
    ip6_zero(ip6_and(n->prefix, ip6_not(ip6_mkmask(n->pxlen))));
}

int
net_validate(const net_addr *N)
{
  switch (a->type)
  {
  case NET_IP4:
  case NET_VPN4:
    return net_validate_ip4((net_addr_ip4 *) N);

  case NET_IP6:
  case NET_VPN6:
    return net_validate_ip6((net_addr_ip6 *) N);

  default:
    return 0;
  }
}

int
net_classify(const net_addr *N)
{
  net_addr_union *n = (void *) N;

  switch (n->n.type)
  {
  case NET_IP4:
  case NET_VPN4:
    return ip4_zero(n->ip4.prefix) ? (IADDR_HOST | SCOPE_UNIVERSE) : ip4_classify(n->ip4.prefix);

  case NET_IP6:
  case NET_VPN6:
    return ip6_zero(n->ip6.prefix) ? (IADDR_HOST | SCOPE_UNIVERSE) : ip6_classify(&n->ip6.prefix);
  }

  return 0;
}