/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _FMTRFMRK_HXX
#define _FMTRFMRK_HXX


#include <bf_svtools/poolitem.hxx>
namespace binfilter {

class SwTxtRefMark;
// ATT_REFMARK *******************************************************

class SwFmtRefMark : public SfxPoolItem
{
    friend class SwTxtRefMark;
    SwTxtRefMark* pTxtAttr;		// mein TextAttribut

    // geschuetzter CopyCtor
    SwFmtRefMark& operator=(const SwFmtRefMark& rRefMark);
    String aRefName;

public:
    SwFmtRefMark( const String& rTxt );
    SwFmtRefMark( const SwFmtRefMark& rRefMark );
    ~SwFmtRefMark( );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
    virtual SvStream&		Store(SvStream &, USHORT nIVer) const;

    const SwTxtRefMark *GetTxtRefMark() const	{ return pTxtAttr; }
    SwTxtRefMark *GetTxtRefMark()				{ return pTxtAttr; }

    inline       String &GetRefName()       { return aRefName; }
    inline const String &GetRefName() const { return aRefName; }
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
