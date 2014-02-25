/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SW_INC_FMTFLD_HXX
#define INCLUDED_SW_INC_FMTFLD_HXX

#include <com/sun/star/text/XTextField.hpp>

#include <cppuhelper/weakref.hxx>
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
    friend void _InitCore();
    SwFmtFld( sal_uInt16 nWhich ); // for default-Attibute

    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::text::XTextField> m_wXTextField;

    SwField* mpField;
    SwTxtFld* mpTxtFld; // the TextAttribute

    // @@@ copy construction allowed, but copy assignment is not? @@@
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
    virtual bool            operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    virtual bool GetInfo( SfxPoolItem& rInfo ) const;

    const SwField* GetField() const
    {
        return mpField;
    }
    SwField* GetField()
    {
        return mpField;
    }

    /**
       Sets current field.

       @param pField          new field

       @attention The current field will be destroyed before setting the new field.
     */
    void SetField( SwField * pField );

    const SwTxtFld* GetTxtFld() const
    {
        return mpTxtFld;
    }
    SwTxtFld* GetTxtFld()
    {
        return mpTxtFld;
    }
    void SetTxtFld( SwTxtFld& rTxtFld );
    void ClearTxtFld();

    void RegisterToFieldType( SwFieldType& );
    bool IsFldInDoc() const;
    sal_Bool IsProtect() const;

    SAL_DLLPRIVATE ::com::sun::star::uno::WeakReference<
        ::com::sun::star::text::XTextField> const& GetXTextField() const
            { return m_wXTextField; }
    SAL_DLLPRIVATE void SetXTextField(::com::sun::star::uno::Reference<
                    ::com::sun::star::text::XTextField> const& xTextField)
            { m_wXTextField = xTextField; }
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
