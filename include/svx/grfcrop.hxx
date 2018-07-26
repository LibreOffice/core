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

#ifndef INCLUDED_SVX_GRFCROP_HXX
#define INCLUDED_SVX_GRFCROP_HXX

#include <svl/poolitem.hxx>
#include <svx/svxdllapi.h>

#define GRFCROP_VERSION_SWDEFAULT       0
#define GRFCROP_VERSION_MOVETOSVX       1

class SVX_DLLPUBLIC SvxGrfCrop : public SfxPoolItem
{
    sal_Int32   nLeft, nRight, nTop, nBottom;
public:
    SvxGrfCrop( sal_uInt16  );
    SvxGrfCrop( sal_Int32 nLeft,    sal_Int32 nRight,
                sal_Int32 nTop,     sal_Int32 nBottom,
                sal_uInt16  );
    virtual ~SvxGrfCrop() override;

    SvxGrfCrop(SvxGrfCrop const &) = default;
    SvxGrfCrop(SvxGrfCrop &&) = default;
    SvxGrfCrop & operator =(SvxGrfCrop const &) = default;
    SvxGrfCrop & operator =(SvxGrfCrop &&) = default;

    // "pure virtual methods" from SfxPoolItem
    virtual bool                operator==( const SfxPoolItem& ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;
    virtual bool QueryValue( css::uno::Any& rVal,
                             sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal,
                           sal_uInt8 nMemberId ) override;

    void SetLeft( sal_Int32 nVal )      { nLeft = nVal; }
    void SetRight( sal_Int32 nVal )     { nRight = nVal; }
    void SetTop( sal_Int32 nVal )       { nTop = nVal; }
    void SetBottom( sal_Int32 nVal )    { nBottom = nVal; }

    sal_Int32 GetLeft() const           { return nLeft; }
    sal_Int32 GetRight() const          { return nRight; }
    sal_Int32 GetTop() const            { return nTop; }
    sal_Int32 GetBottom() const         { return nBottom; }
};

#endif  // INCLUDED_SVX_GRFCROP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
