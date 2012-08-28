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

#include <list>
#include <svl/poolitem.hxx>
#include <svl/brdcst.hxx>
#include <svl/smplhint.hxx>

#include "swdllapi.h"
#include <calbck.hxx>

class SwField;
class SwTxtFld;
class SwView;
class SwFieldType;

// ATT_FLD
class SW_DLLPUBLIC SwFmtFld : public SfxPoolItem, public SwClient, public SfxBroadcaster
{
    friend class SwTxtFld;
    friend void _InitCore();

    SwField *pField;
    SwTxtFld* pTxtAttr;

    SwFmtFld(); ///< Default attibute.

    /* Protected CopyCtor.
     @@@ copy construction allowed, but copy assignment is not? @@@ */
    SwFmtFld& operator=(const SwFmtFld& rFld);

protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);
    virtual void SwClientNotify( const SwModify& rModify, const SfxHint& rHint );

public:
    TYPEINFO();

    /// Single argument constructors shall be explicit.
    explicit SwFmtFld( const SwField &rFld );

    /// @@@ copy construction allowed, but copy assignment is not? @@@
    SwFmtFld( const SwFmtFld& rAttr );

    virtual ~SwFmtFld();

    /// "Pure virtual methods" of SfxPoolItem.
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    virtual sal_Bool GetInfo( SfxPoolItem& rInfo ) const;

    const SwField *GetFld() const   { return pField; }
    SwField *GetFld()               { return pField; }

    /**
       Sets current field.

       @param pField          new field

       @attention The current field will be destroyed before setting the new field.
     */
    void SetFld(SwField * pField);

    const SwTxtFld *GetTxtFld() const   { return pTxtAttr; }
    SwTxtFld *GetTxtFld()               { return pTxtAttr; }

    void RegisterToFieldType( SwFieldType& );
    sal_Bool IsFldInDoc() const;
    sal_Bool IsProtect() const;
};

class SW_DLLPUBLIC SwFmtFldHint : public SfxHint
{
#define SWFMTFLD_INSERTED   1
#define SWFMTFLD_REMOVED    2
#define SWFMTFLD_FOCUS      3
#define SWFMTFLD_CHANGED    4
#define SWFMTFLD_LANGUAGE   5

    const SwFmtFld* pFld;
    sal_Int16 nWhich;
    const SwView* pView;

public:
    SwFmtFldHint( const SwFmtFld* p, sal_Int16 n, const SwView* pV = 0)
        : pFld(p)
        , nWhich(n)
        , pView(pV)
    {}

    TYPEINFO();
    const SwFmtFld* GetField() const { return pFld; }
    sal_Int16 Which() const { return nWhich; }
    const SwView* GetView() const { return pView; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
