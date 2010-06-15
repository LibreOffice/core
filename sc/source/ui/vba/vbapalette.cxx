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

#include "vbapalette.hxx"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
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

typedef ::cppu::WeakImplHelper1< container::XIndexAccess > XIndexAccess_BASE;

class DefaultPalette : public XIndexAccess_BASE
{
public:
   DefaultPalette(){}

    // Methods XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException)
    {
        return sizeof(spnDefColorTable8) / sizeof(spnDefColorTable8[0]);
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
    if ( Index < 0 || Index >= getCount() )
        throw lang::IndexOutOfBoundsException();
        return uno::makeAny( sal_Int32( spnDefColorTable8[ Index ] ) );
    }

    // Methods XElementAcess
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException)
    {
        return ::getCppuType( (sal_Int32*)0 );
    }
    virtual ::sal_Bool SAL_CALL hasElements() throw (uno::RuntimeException)
    {
        return sal_True;
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
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Can't extract palette, no doc shell" ) ), uno::Reference< uno::XInterface >() );
    xIndex.set( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ColorPalette") ) ), uno::UNO_QUERY );
    if ( !xIndex.is() )
        return new DefaultPalette();
    return xIndex;
}
