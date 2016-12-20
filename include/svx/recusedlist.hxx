/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SVX_RUCHARMAPLIST_HXX
#define INCLUDED_SVX_RUCHARMAPLIST_HXX

#include <svx/svxdllapi.h>

#define ARRAY_LENGHT_RECENTLY_USED_CHAR 32

namespace svx
{
    struct SvxShowCharSetItem;
    class SvxShowCharSetVirtualAcc;
}



class SVX_DLLPUBLIC RecentlyUsedCharMap{
public:
  sal_UCS4  getRecentlyChar(int nIndex);
  vcl::Font getRecentlyCharFont(int nIndex);

  void      pushRecentlyChar(sal_UCS4 cChar,vcl::Font aFont);

  int       getFreeIndex();
  int       getLenght();

private:

  int       m_FreeIndex=0;
  bool      m_bFull=false;
  int       m_Lenght;

  struct    RecentlyUsedChar{
              sal_UCS4 cChar;
              vcl::Font aFont;
  }         RecentlyUsedChars[ARRAY_LENGHT_RECENTLY_USED_CHAR];

private:
  int       Search(sal_UCS4 cChar,vcl::Font aFont);
  void      Sort(int nFoundedChar,sal_UCS4 cChar,vcl::Font aFont);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */