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

#include "oox/ole/axcontrolhelper.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include "tokens.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/drawingml/color.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::container::XNameContainer;
using ::com::sun::star::graphic::XGraphic;
using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::form::XForm;
using ::com::sun::star::form::XFormsSupplier;
using ::oox::core::FilterBase;

namespace oox {
namespace ole {

// ============================================================================

namespace {

const sal_uInt32 AX_COLORTYPE_MASK          = 0xFF000000;
const sal_uInt32 AX_COLORTYPE_CLIENT        = 0x00000000;
const sal_uInt32 AX_COLORTYPE_PALETTE       = 0x01000000;
const sal_uInt32 AX_COLORTYPE_BGR           = 0x02000000;
const sal_uInt32 AX_COLORTYPE_SYSCOLOR      = 0x80000000;

const sal_uInt32 AX_PALETTECOLOR_MASK       = 0x0000FFFF;
const sal_uInt32 AX_BGRCOLOR_MASK           = 0x00FFFFFF;
const sal_uInt32 AX_SYSTEMCOLOR_MASK        = 0x0000FFFF;

// ----------------------------------------------------------------------------

/** Returns the UNO RGB color from the passed encoded OLE BGR color. */
inline sal_Int32 lclDecodeBgrColor( sal_uInt32 nAxColor )
{
    return static_cast< sal_Int32 >( ((nAxColor & 0x0000FF) << 16) | (nAxColor & 0x00FF00) | ((nAxColor & 0xFF0000) >> 16) );
}

} // namespace

// ============================================================================

AxControlHelper::AxControlHelper( const FilterBase& rFilter, AxDefaultColorMode eColorMode ) :
    mrFilter( rFilter ),
    meColorMode( eColorMode ),
    mbHasFormQuerried( false )
{
}

AxControlHelper::~AxControlHelper()
{
}

Reference< XForm > AxControlHelper::getControlForm() const
{
    if( !mbHasFormQuerried )
    {
        mbHasFormQuerried = true;
        mxForm = createControlForm();   // virtual call
    }
    return mxForm;
}

sal_Int32 AxControlHelper::convertColor( sal_uInt32 nAxColor ) const
{
    static const sal_Int32 spnSystemColors[] =
    {
        XML_scrollBar,      XML_background,     XML_activeCaption,  XML_inactiveCaption,
        XML_menu,           XML_window,         XML_windowFrame,    XML_menuText,
        XML_windowText,     XML_captionText,    XML_activeBorder,   XML_inactiveBorder,
        XML_appWorkspace,   XML_highlight,      XML_highlightText,  XML_btnFace,
        XML_btnShadow,      XML_grayText,       XML_btnText,        XML_inactiveCaptionText,
        XML_btnHighlight,   XML_3dDkShadow,     XML_3dLight,        XML_infoText,
        XML_infoBk
    };

    switch( nAxColor & AX_COLORTYPE_MASK )
    {
        case AX_COLORTYPE_CLIENT:
            switch( meColorMode )
            {
                case AX_DEFAULTCOLORMODE_BGR:       return lclDecodeBgrColor( nAxColor );
                case AX_DEFAULTCOLORMODE_PALETTE:   return mrFilter.getPaletteColor( nAxColor & AX_PALETTECOLOR_MASK );
            }
        break;

        case AX_COLORTYPE_PALETTE:
            return mrFilter.getPaletteColor( nAxColor & AX_PALETTECOLOR_MASK );

        case AX_COLORTYPE_BGR:
            return lclDecodeBgrColor( nAxColor );

        case AX_COLORTYPE_SYSCOLOR:
            return mrFilter.getSystemColor( STATIC_ARRAY_SELECT( spnSystemColors, nAxColor & AX_SYSTEMCOLOR_MASK, XML_TOKEN_INVALID ), API_RGB_WHITE );
    }
    OSL_ENSURE( false, "AxControlHelper::convertColor - unknown color type" );
    return 0;
}

// ============================================================================

AxEmbeddedControlHelper::AxEmbeddedControlHelper( const FilterBase& rFilter,
        const Reference< XDrawPage >& rxDrawPage, AxDefaultColorMode eColorMode ) :
    AxControlHelper( rFilter, eColorMode ),
    mxDrawPage( rxDrawPage )
{
}

Reference< XForm > AxEmbeddedControlHelper::createControlForm() const
{
    Reference< XForm > xForm;
    try
    {
        Reference< XFormsSupplier > xFormsSupplier( mxDrawPage, UNO_QUERY_THROW );
        Reference< XNameContainer > xFormsNC( xFormsSupplier->getForms(), UNO_SET_THROW );
        OUString aFormName = CREATE_OUSTRING( "Standard" );
        if( xFormsNC->hasByName( aFormName ) )
        {
            xFormsNC->getByName( aFormName ) >>= xForm;
        }
        else
        {
            Reference< XForm > xNewForm( getFilter().getModelFactory()->createInstance( CREATE_OUSTRING( "com.sun.star.form.component.Form" ) ), UNO_QUERY_THROW );
            xFormsNC->insertByName( aFormName, Any( xNewForm ) );
            // return the form if insertion did not fail
            xForm = xNewForm;
        }
    }
    catch( Exception& )
    {
    }
    return xForm;
}

// ============================================================================

} // namespace ole
} // namespace oox

