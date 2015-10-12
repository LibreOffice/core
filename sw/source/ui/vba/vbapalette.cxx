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

#include "vbapalette.hxx"
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/word/WdColor.hpp>
#include <ooo/vba/word/WdColorIndex.hpp>
#include <sal/macros.h>

using namespace ::ooo::vba;
using namespace ::ooo::vba::word;
using namespace ::com::sun::star;

static const sal_Int32 ColorTable[] =
{
WdColor::wdColorAutomatic, // 0
WdColor::wdColorBlack,             // 1
WdColor::wdColorBlue,               // 2
WdColor::wdColorTurquoise,     // 3
WdColor::wdColorBrightGreen, // 4
WdColor::wdColorPink,               // 5
WdColor::wdColorRed,                 // 6
WdColor::wdColorYellow,           // 7
WdColor::wdColorWhite,             // 8
WdColor::wdColorDarkBlue,       // 9
WdColor::wdColorTeal,               // 10
WdColor::wdColorGreen,             // 11
WdColor::wdColorViolet,           // 12
WdColor::wdColorDarkRed,         // 13
WdColor::wdColorDarkYellow,   // 14
WdColor::wdColorGray50,           // 15
WdColor::wdColorGray25,           // 16
};

typedef ::cppu::WeakImplHelper< container::XIndexAccess > XIndexAccess_BASE;

class DefaultPalette : public XIndexAccess_BASE
{
public:
   DefaultPalette(){}

    // Methods XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException, std::exception) override
    {
        return SAL_N_ELEMENTS(ColorTable);
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( sal_Int32( ColorTable[ Index ] ) );
    }

    // Methods XElementAcess
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException, std::exception) override
    {
        return ::cppu::UnoType<sal_Int32>::get();
    }
    virtual sal_Bool SAL_CALL hasElements() throw (uno::RuntimeException, std::exception) override
    {
        return sal_True;
    }

};

VbaPalette::VbaPalette()
{
    mxPalette = new DefaultPalette();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
