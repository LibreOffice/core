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
#ifndef INCLUDED_SW_INC_FMTRUBY_HXX
#define INCLUDED_SW_INC_FMTRUBY_HXX

#include <svl/poolitem.hxx>

class SwTextRuby;

class SW_DLLPUBLIC SwFormatRuby : public SfxPoolItem
{
    friend class SwTextRuby;

    OUString sRubyText;                      ///< The ruby txt.
    OUString sCharFormatName;                  ///< Name of the charformat.
    SwTextRuby* pTextAttr;                    ///< The TextAttribute.
    sal_uInt16 nCharFormatId;                  ///< PoolId of the charformat.
    sal_uInt16 nPosition;                   ///< Position of the Ruby-character.
    sal_uInt16 nAdjustment;                 ///< Specific adjustment of the Ruby-ch.

public:
    SwFormatRuby( const OUString& rRubyText );
    SwFormatRuby( const SwFormatRuby& rAttr );
    virtual ~SwFormatRuby();

    SwFormatRuby& operator=( const SwFormatRuby& rAttr );

    // "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper* pIntl = 0 ) const override;

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const SwTextRuby* GetTextRuby() const         { return pTextAttr; }

    OUString GetText() const                    { return sRubyText; }
    void SetText( const OUString& rText )        { sRubyText = rText; }

    OUString GetCharFormatName() const             { return sCharFormatName; }
    void SetCharFormatName( const OUString& rNm )  { sCharFormatName = rNm; }

    sal_uInt16 GetCharFormatId() const                 { return nCharFormatId; }
    void SetCharFormatId( sal_uInt16 nNew )            { nCharFormatId = nNew; }

    sal_uInt16 GetPosition() const                  { return nPosition; }
    void SetPosition( sal_uInt16 nNew )             { nPosition = nNew; }

    sal_uInt16 GetAdjustment() const                { return nAdjustment; }
    void SetAdjustment( sal_uInt16 nNew )           { nAdjustment = nNew; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
