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

#include <sal/macros.h>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include "excelvbahelper.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

/** Standard EGA colors, bright. */
#define EXC_PALETTE_EGA_COLORS_LIGHT \
            0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF
/** Standard EGA colors, dark. */
#define EXC_PALETTE_EGA_COLORS_DARK \
            0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080, 0xC0C0C0, 0x808080

static const ColorData spnDefColorTable8[] =
{
/*  8 */    EXC_PALETTE_EGA_COLORS_LIGHT,
/* 16 */    EXC_PALETTE_EGA_COLORS_DARK,
/* 24 */    0x9999FF, 0x993366, 0xFFFFCC, 0xCCFFFF, 0x660066, 0xFF8080, 0x0066CC, 0xCCCCFF,
/* 32 */    0x000080, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x800080, 0x800000, 0x008080, 0x0000FF,
/* 40 */    0x00CCFF, 0xCCFFFF, 0xCCFFCC, 0xFFFF99, 0x99CCFF, 0xFF99CC, 0xCC99FF, 0xFFCC99,
/* 48 */    0x3366FF, 0x33CCCC, 0x99CC00, 0xFFCC00, 0xFF9900, 0xFF6600, 0x666699, 0x969696,
/* 56 */    0x003366, 0x339966, 0x003300, 0x333300, 0x993300, 0x993366, 0x333399, 0x333333
};

typedef ::cppu::WeakImplHelper< container::XIndexAccess > XIndexAccess_BASE;

class DefaultPalette : public XIndexAccess_BASE
{
public:
   DefaultPalette(){}

    // Methods XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount() override
    {
        return SAL_N_ELEMENTS(spnDefColorTable8);
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( sal_Int32( spnDefColorTable8[ Index ] ) );
    }

    // Methods XElementAccess
    virtual uno::Type SAL_CALL getElementType() override
    {
        return ::cppu::UnoType<sal_Int32>::get();
    }
    virtual sal_Bool SAL_CALL hasElements() override
    {
        return true;
    }

};

ScVbaPalette::ScVbaPalette( const uno::Reference< frame::XModel >& rxModel ) :
    m_pShell( excel::getDocShell( rxModel ) )
{
}

uno::Reference< container::XIndexAccess >
ScVbaPalette::getDefaultPalette()
{
    return new DefaultPalette();
}

uno::Reference< container::XIndexAccess >
ScVbaPalette::getPalette() const
{
    uno::Reference< container::XIndexAccess > xIndex;
    uno::Reference< beans::XPropertySet > xProps;
    if ( m_pShell )
        xProps.set( m_pShell->GetModel(), uno::UNO_QUERY_THROW );
    else
        throw uno::RuntimeException("Can't extract palette, no doc shell" );
    xIndex.set( xProps->getPropertyValue("ColorPalette"), uno::UNO_QUERY );
    if ( !xIndex.is() )
        return new DefaultPalette();
    return xIndex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
