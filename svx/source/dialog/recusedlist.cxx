/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <vcl/svapp.hxx>
#include <vcl/builderfactory.hxx>
#include <svx/recusedlist.hxx>

//TODO cambia i nomi della classe e dei file!


sal_UCS4 RecentlyUsedCharMap::getRecentlyChar(int nIndex){
  if (nIndex>=0){
    if(!m_bFull)
      return RecentlyUsedChars[m_FreeIndex-nIndex-1].cChar;
    else
      return RecentlyUsedChars[(m_Lenght+(m_FreeIndex-1-nIndex))%32].cChar;
  }
  else
    return (char) 0;
}
vcl::Font RecentlyUsedCharMap::getRecentlyCharFont(int nIndex) {
  if (nIndex>=0){
    if(!m_bFull)
      return RecentlyUsedChars[m_FreeIndex-nIndex-1].aFont;
    else
      return RecentlyUsedChars[(m_Lenght+(m_FreeIndex-1-nIndex))%32].aFont;
  }

}

void RecentlyUsedCharMap::pushRecentlyChar(sal_UCS4 cChar,vcl::Font aFont){
  int exist;


  exist=Search(cChar, aFont);

  if (exist==-1){
    RecentlyUsedChars[m_FreeIndex].cChar=cChar;
    RecentlyUsedChars[m_FreeIndex].aFont=aFont;
    if (m_FreeIndex < ARRAY_LENGHT_RECENTLY_USED_CHAR-1)
      m_FreeIndex++;
    else{
      m_FreeIndex=0;
      m_bFull=true;
    }

   if(m_bFull){
     m_Lenght=ARRAY_LENGHT_RECENTLY_USED_CHAR;
   }else{
     m_Lenght= m_FreeIndex;
   }
  }else{
    Sort(exist,cChar,aFont);
  }


}
int  RecentlyUsedCharMap::getFreeIndex(){
  return m_FreeIndex;
}

int RecentlyUsedCharMap::getLenght(){
  return m_Lenght;
}

int RecentlyUsedCharMap::Search(sal_UCS4 cChar,vcl::Font aFont){
  bool notfound=true;
  int i=0;
  while(notfound && i<m_Lenght){
    if(RecentlyUsedChars[i].cChar==cChar && RecentlyUsedChars[i].aFont==aFont){
      notfound=false;
    }else{
      i++;
    }
  }
  if (notfound)
    return -1;
  else
    return i;
}

void RecentlyUsedCharMap::Sort(int nFoundedChar,sal_UCS4 cChar,vcl::Font aFont){
  if (!m_bFull){
    for(int i=nFoundedChar; i<m_FreeIndex-1;i++){
      RecentlyUsedChars[i].cChar=RecentlyUsedChars[i+1].cChar;
      RecentlyUsedChars[i].aFont=RecentlyUsedChars[i+1].aFont;
    }
      RecentlyUsedChars[m_FreeIndex-1].cChar=cChar;
      RecentlyUsedChars[m_FreeIndex-1].aFont=aFont;
  }else{
    int ncicli;
    ncicli=ARRAY_LENGHT_RECENTLY_USED_CHAR-nFoundedChar+m_FreeIndex-1;
    for(int i=0; i<ncicli;i++){
      RecentlyUsedChars[(nFoundedChar+i)%32].cChar=RecentlyUsedChars[((nFoundedChar+i+1)%32)].cChar;
      RecentlyUsedChars[(nFoundedChar+i)%32].aFont=RecentlyUsedChars[((nFoundedChar+i+1)%32)].aFont;
    }
    RecentlyUsedChars[(m_Lenght+m_FreeIndex-1)%32].cChar=cChar;
    RecentlyUsedChars[(m_Lenght+m_FreeIndex-1)%32].aFont=aFont;
  }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */