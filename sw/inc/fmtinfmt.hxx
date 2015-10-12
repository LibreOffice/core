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
#ifndef INCLUDED_SW_INC_FMTINFMT_HXX
#define INCLUDED_SW_INC_FMTINFMT_HXX

#include <svl/poolitem.hxx>
#include "swdllapi.h"

class SvxMacro;
class SvxMacroTableDtor;
class SwTextINetFormat;
class IntlWrapper;

// ATT_INETFMT

class SW_DLLPUBLIC SwFormatINetFormat: public SfxPoolItem
{
    friend class SwTextINetFormat;

    OUString msURL;                  ///< URL.
    OUString msTargetFrame;          ///< Target frame for URL.
    OUString msINetFormatName;
    OUString msVisitedFormatName;
    OUString msHyperlinkName;        ///< Name of the link.
    SvxMacroTableDtor* mpMacroTable;
    SwTextINetFormat* mpTextAttr;         ///< My TextAttribute.
    sal_uInt16 mnINetFormatId;
    sal_uInt16 mnVisitedFormatId;
public:
    SwFormatINetFormat( const OUString& rURL, const OUString& rTarget );
    SwFormatINetFormat( const SwFormatINetFormat& rAttr );
    SwFormatINetFormat();                     ///< For TypeInfo.
    virtual ~SwFormatINetFormat();

    TYPEINFO_OVERRIDE();

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper* pIntl = 0 ) const override;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId ) override;

    const SwTextINetFormat* GetTextINetFormat() const
    {
        return mpTextAttr;
    }

    const OUString& GetValue() const
    {
        return msURL;
    }

    const OUString& GetName() const
    {
        return msHyperlinkName;
    }
    void SetName( const OUString& rNm )
    {
        msHyperlinkName = rNm;
    }

    const OUString& GetTargetFrame() const
    {
        return msTargetFrame;
    }

    void SetINetFormatAndId(
            const OUString& rNm,
            const sal_uInt16 nId )
    {
        msINetFormatName = rNm;
        mnINetFormatId = nId;
    }

    const OUString& GetINetFormat() const
    {
        return msINetFormatName;
    }

    sal_uInt16 GetINetFormatId() const
    {
        return mnINetFormatId;
    }

    void SetVisitedFormatAndId(
            const OUString& rNm,
            const sal_uInt16 nId )
    {
        msVisitedFormatName = rNm;
        mnVisitedFormatId = nId;
    }

    const OUString& GetVisitedFormat() const
    {
        return msVisitedFormatName;
    }

    sal_uInt16 GetVisitedFormatId() const
    {
        return mnVisitedFormatId;
    }

    /// Set a new MacroTable or clear the current one.
    void SetMacroTable( const SvxMacroTableDtor* pTable = 0 );
    const SvxMacroTableDtor* GetMacroTable() const
    {
        return mpMacroTable;
    }

    /// Macro getter and setter.
    void SetMacro(
            sal_uInt16 nEvent,
            const SvxMacro& rMacro );
    const SvxMacro* GetMacro( sal_uInt16 nEvent ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
