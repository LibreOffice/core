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
#include <svl/SfxBroadcaster.hxx>
#include <svl/smplhint.hxx>

#include "swdllapi.h"
#include <calbck.hxx>

class SwField;
class SwTextField;
class SwView;
class SwFieldType;

// ATT_FLD
class SW_DLLPUBLIC SwFormatField
    : public SfxPoolItem
    , public SwModify
    , public SfxBroadcaster
{
    friend void _InitCore();
    SwFormatField( sal_uInt16 nWhich ); // for default-Attibute

    css::uno::WeakReference<css::text::XTextField> m_wXTextField;

    SwField* mpField;
    SwTextField* mpTextField; // the TextAttribute

    // @@@ copy construction allowed, but copy assignment is not? @@@
    SwFormatField& operator=(const SwFormatField& rField) = delete;

protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;
    virtual void SwClientNotify( const SwModify& rModify, const SfxHint& rHint ) override;

public:

    /// Single argument constructors shall be explicit.
    explicit SwFormatField( const SwField &rField );

    /// @@@ copy construction allowed, but copy assignment is not? @@@
    SwFormatField( const SwFormatField& rAttr );

    virtual ~SwFormatField();

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;

    virtual bool GetInfo( SfxPoolItem& rInfo ) const override;

    void InvalidateField();

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

    const SwTextField* GetTextField() const
    {
        return mpTextField;
    }
    SwTextField* GetTextField()
    {
        return mpTextField;
    }
    void SetTextField( SwTextField& rTextField );
    void ClearTextField();

    void RegisterToFieldType( SwFieldType& );
    bool IsFieldInDoc() const;
    bool IsProtect() const;

    SAL_DLLPRIVATE css::uno::WeakReference<css::text::XTextField> const& GetXTextField() const
            { return m_wXTextField; }
    SAL_DLLPRIVATE void SetXTextField(css::uno::Reference<css::text::XTextField> const& xTextField)
            { m_wXTextField = xTextField; }
    void dumpAsXml(struct _xmlTextWriter* pWriter) const override;
};

enum class SwFormatFieldHintWhich
{
    INSERTED   = 1,
    REMOVED    = 2,
    FOCUS      = 3,
    CHANGED    = 4,
    LANGUAGE   = 5
};

class SW_DLLPUBLIC SwFormatFieldHint : public SfxHint
{
    const SwFormatField*   pField;
    SwFormatFieldHintWhich nWhich;
    const SwView*     pView;

public:
    SwFormatFieldHint( const SwFormatField* p, SwFormatFieldHintWhich n, const SwView* pV = nullptr)
        : pField(p)
        , nWhich(n)
        , pView(pV)
    {}

    const SwFormatField* GetField() const { return pField; }
    SwFormatFieldHintWhich Which() const { return nWhich; }
    const SwView* GetView() const { return pView; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
