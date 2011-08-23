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
#ifndef _FMTFLD_HXX
#define _FMTFLD_HXX


#include <bf_svtools/poolitem.hxx>
#include <calbck.hxx>
namespace binfilter {

class SwField;
class SwTxtFld;
// ATT_FLD ***********************************

class SwFmtFld : public SfxPoolItem, public SwClient
{
    friend class SwTxtFld;
    friend void _InitCore();

    SwField *pField;
    SwTxtFld* pTxtAttr;		// mein TextAttribut
    SwFmtFld();				// das default-Attibut

    // geschuetzter CopyCtor
    SwFmtFld& operator=(const SwFmtFld& rFld);
public:
    TYPEINFO();

    SwFmtFld( const SwField &rFld );
    SwFmtFld( const SwFmtFld& rAttr );
    ~SwFmtFld();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
    virtual SvStream&		Store(SvStream &, USHORT nIVer) const;

    virtual void Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );
    virtual BOOL GetInfo( SfxPoolItem& rInfo ) const;

    const SwField *GetFld() const	{ return pField; }
    SwField *GetFld()				{ return pField; }

    const SwTxtFld *GetTxtFld() const	{ return pTxtAttr; }
    SwTxtFld *GetTxtFld()				{ return pTxtAttr; }

    BOOL IsFldInDoc() const;
    BOOL IsProtect() const;
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
