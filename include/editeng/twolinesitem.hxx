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
#ifndef INCLUDED_EDITENG_TWOLINESITEM_HXX
#define INCLUDED_EDITENG_TWOLINESITEM_HXX

#include <sal/types.h>
#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;

class EDITENG_DLLPUBLIC SvxTwoLinesItem : public SfxPoolItem
{
    sal_Unicode cStartBracket, cEndBracket;
    bool bOn;
public:
    SvxTwoLinesItem( bool bOn /*= true*/,
                     sal_Unicode nStartBracket /*= 0*/,
                     sal_Unicode nEndBracket /*= 0*/,
                     sal_uInt16 nId  );
    virtual ~SvxTwoLinesItem() override;

    SvxTwoLinesItem(SvxTwoLinesItem const &) = default;
    SvxTwoLinesItem(SvxTwoLinesItem &&) = default;
    SvxTwoLinesItem & operator =(SvxTwoLinesItem const &) = default;
    SvxTwoLinesItem & operator =(SvxTwoLinesItem &&) = default;

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual sal_uInt16          GetVersion( sal_uInt16 nFFVer ) const override;

    bool GetValue() const                       { return bOn; }
    void SetValue( bool bFlag )                 { bOn = bFlag; }

    sal_Unicode GetStartBracket() const         { return cStartBracket; }
    void SetStartBracket( sal_Unicode c )       { cStartBracket = c; }

    sal_Unicode GetEndBracket() const           { return cEndBracket; }
    void SetEndBracket( sal_Unicode c )         { cEndBracket = c; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
