/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtrfmrk.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:52:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _FMTRFMRK_HXX
#define _FMTRFMRK_HXX


#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

class SwTxtRefMark;

// ATT_REFMARK *******************************************************

class SwFmtRefMark : public SfxPoolItem
{
    friend class SwTxtRefMark;
    SwTxtRefMark* pTxtAttr;     // mein TextAttribut

    // geschuetzter CopyCtor
    SwFmtRefMark& operator=(const SwFmtRefMark& rRefMark);
    String aRefName;

public:
    SwFmtRefMark( const String& rTxt );
    SwFmtRefMark( const SwFmtRefMark& rRefMark );
    ~SwFmtRefMark( );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    const SwTxtRefMark *GetTxtRefMark() const   { return pTxtAttr; }
    SwTxtRefMark *GetTxtRefMark()               { return pTxtAttr; }

    inline       String &GetRefName()       { return aRefName; }
    inline const String &GetRefName() const { return aRefName; }
};

#endif

