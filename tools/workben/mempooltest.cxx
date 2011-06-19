/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "tools/mempool.hxx"

struct MempoolTest
{
  int m_int;

  DECL_FIXEDMEMPOOL_NEWDEL(MempoolTest);
};

IMPL_FIXEDMEMPOOL_NEWDEL(MempoolTest, 0, 0);

int main()
{
  MempoolTest * p = new MempoolTest();
  if (p != 0)
    delete p;
  return 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
