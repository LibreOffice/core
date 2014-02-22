/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "vbapalette.hxx"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/word/WdColor.hpp>
#include <ooo/vba/word/WdColorIndex.hpp>
#include <sal/macros.h>

using namespace ::ooo::vba;
using namespace ::ooo::vba::word;
using namespace ::com::sun::star;

static const sal_Int32 ColorTable[] =
{
WdColor::wdColorAutomatic, 
WdColor::wdColorBlack,             
WdColor::wdColorBlue,               
WdColor::wdColorTurquoise,     
WdColor::wdColorBrightGreen, 
WdColor::wdColorPink,               
WdColor::wdColorRed,                 
WdColor::wdColorYellow,           
WdColor::wdColorWhite,             
WdColor::wdColorDarkBlue,       
WdColor::wdColorTeal,               
WdColor::wdColorGreen,             
WdColor::wdColorViolet,           
WdColor::wdColorDarkRed,         
WdColor::wdColorDarkYellow,   
WdColor::wdColorGray50,           
WdColor::wdColorGray25,           
};

typedef ::cppu::WeakImplHelper1< container::XIndexAccess > XIndexAccess_BASE;

class DefaultPalette : public XIndexAccess_BASE
{
public:
   DefaultPalette(){}

    
    virtual ::sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException)
    {
        return SAL_N_ELEMENTS(ColorTable);
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( sal_Int32( ColorTable[ Index ] ) );
    }

    
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException)
    {
        return ::getCppuType( (sal_Int32*)0 );
    }
    virtual ::sal_Bool SAL_CALL hasElements() throw (uno::RuntimeException)
    {
        return sal_True;
    }

};

VbaPalette::VbaPalette()
{
    mxPalette = new DefaultPalette();
}

uno::Reference< container::XIndexAccess >
VbaPalette::getPalette() const
{

    return mxPalette;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
