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
#include "vbacommandbar.hxx"
#include "vbacommandbarcontrols.hxx"
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <ooo/vba/office/MsoBarType.hpp>

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaCommandBar::ScVbaCommandBar( const uno::Reference< ov::XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, VbaCommandBarHelperRef pHelper, const uno::Reference< container::XIndexAccess >& xBarSettings, const rtl::OUString& sResourceUrl, sal_Bool bIsMenu, sal_Bool bTemporary ) throw( uno::RuntimeException ) : CommandBar_BASE( xParent, xContext ), pCBarHelper( pHelper ), m_xBarSettings( xBarSettings ), m_sResourceUrl( sResourceUrl ), m_bIsMenu( bIsMenu ), m_bTemporary( bTemporary )
{
}

::rtl::OUString SAL_CALL
ScVbaCommandBar::getName() throw ( uno::RuntimeException )
{
    // This will get a "NULL length string" when Name is not set.
    uno::Reference< beans::XPropertySet > xPropertySet( m_xBarSettings, uno::UNO_QUERY_THROW );
    uno::Any aName = xPropertySet->getPropertyValue( rtl::OUString::createFromAscii("UIName") );
    rtl::OUString sName;
    aName >>= sName;
    if( sName.getLength() < 1 )
    {
        if( m_bIsMenu )
        {
            if( m_sResourceUrl.equalsAscii( ITEM_MENUBAR_URL ) )
            {
                if( pCBarHelper->getModuleId().equalsAscii("com.sun.star.sheet.SpreadsheetDocument") )
                    sName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Worksheet Menu Bar") );
                else if( pCBarHelper->getModuleId().equalsAscii("com.sun.star.text.TextDocument") )
                    sName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Menu Bar") );
                return sName;
            }
        }
        // Toolbar name
        uno::Reference< container::XNameAccess > xNameAccess = pCBarHelper->getPersistentWindowState();
        if( xNameAccess->hasByName( m_sResourceUrl ) )
        {
            uno::Sequence< beans::PropertyValue > aToolBar;
            xNameAccess->getByName( m_sResourceUrl ) >>= aToolBar;
            getPropertyValue( aToolBar, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("UIName") ) ) >>= sName;
        }
    }
    return sName;
}
void SAL_CALL
ScVbaCommandBar::setName( const ::rtl::OUString& _name ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xPropertySet( m_xBarSettings, uno::UNO_QUERY_THROW );
    xPropertySet->setPropertyValue( rtl::OUString::createFromAscii("UIName"), uno::makeAny( _name ) );

    pCBarHelper->ApplyChange( m_sResourceUrl, m_xBarSettings );
}
::sal_Bool SAL_CALL
ScVbaCommandBar::getVisible() throw (uno::RuntimeException)
{
    // menu bar is allways visible in OOo
    if( m_bIsMenu )
        return sal_True;

    sal_Bool bVisible = sal_False;
    try
    {
        uno::Reference< container::XNameAccess > xNameAccess = pCBarHelper->getPersistentWindowState();
        if( xNameAccess->hasByName( m_sResourceUrl ) )
        {
            uno::Sequence< beans::PropertyValue > aToolBar;
            xNameAccess->getByName( m_sResourceUrl ) >>= aToolBar;
            getPropertyValue( aToolBar, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Visible") ) ) >>= bVisible;
        }
    }
    catch ( uno::Exception e )
    {
    }
    return bVisible;
}
void SAL_CALL
ScVbaCommandBar::setVisible( ::sal_Bool _visible ) throw (uno::RuntimeException)
{
    try
    {
        uno::Reference< frame::XLayoutManager > xLayoutManager = pCBarHelper->getLayoutManager();
        if( _visible )
        {
            xLayoutManager->createElement( m_sResourceUrl );
            xLayoutManager->showElement( m_sResourceUrl );
        }
        else
        {
            xLayoutManager->hideElement( m_sResourceUrl );
            xLayoutManager->destroyElement( m_sResourceUrl );
        }
    }
    catch( uno::Exception e )
    {
        OSL_TRACE( "SetVisible get an exception\n" );
    }
}

::sal_Bool SAL_CALL
ScVbaCommandBar::getEnabled() throw (uno::RuntimeException)
{
    // emulated with Visible
    return getVisible();
}

void SAL_CALL
ScVbaCommandBar::setEnabled( sal_Bool _enabled ) throw (uno::RuntimeException)
{
    // emulated with Visible
    setVisible( _enabled );
}

void SAL_CALL
ScVbaCommandBar::Delete(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    pCBarHelper->removeSettings( m_sResourceUrl );
    uno::Reference< container::XNameContainer > xNameContainer( pCBarHelper->getPersistentWindowState(), uno::UNO_QUERY_THROW );
    if( xNameContainer->hasByName( m_sResourceUrl ) )
    {
        xNameContainer->removeByName( m_sResourceUrl );
    }
}
uno::Any SAL_CALL
ScVbaCommandBar::Controls( const uno::Any& aIndex ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< XCommandBarControls > xCommandBarControls( new ScVbaCommandBarControls( this, mxContext, m_xBarSettings, pCBarHelper, m_xBarSettings, m_sResourceUrl ) );
    if( aIndex.hasValue() )
    {
        return xCommandBarControls->Item( aIndex, uno::Any() );
    }
    return uno::makeAny( xCommandBarControls );
}

sal_Int32 SAL_CALL
ScVbaCommandBar::Type() throw (script::BasicErrorException, uno::RuntimeException)
{
    // #FIXME support msoBarTypePopup
    sal_Int32 nType = office::MsoBarType::msoBarTypePopup;
    nType = m_bIsMenu? office::MsoBarType::msoBarTypeNormal : office::MsoBarType::msoBarTypeMenuBar;
    return nType;
}

uno::Any SAL_CALL
ScVbaCommandBar::FindControl( const uno::Any& /*aType*/, const uno::Any& /*aId*/, const uno::Any& /*aTag*/, const uno::Any& /*aVisible*/, const uno::Any& /*aRecursive*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // alwayse fail to find control
    return uno::makeAny( uno::Reference< XCommandBarControl > () );
}

rtl::OUString&
ScVbaCommandBar::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaCommandBar") );
    return sImplName;
}
uno::Sequence<rtl::OUString>
ScVbaCommandBar::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.CommandBar" ) );
    }
    return aServiceNames;
}


VbaDummyCommandBar::VbaDummyCommandBar(
        const uno::Reference< ov::XHelperInterface > xParent,
        const uno::Reference< uno::XComponentContext > xContext,
        const ::rtl::OUString& rName, sal_Int32 nType ) throw( uno::RuntimeException ) :
    CommandBar_BASE( xParent, xContext ),
    maName( rName ),
    mnType( nType )
{
}

::rtl::OUString SAL_CALL VbaDummyCommandBar::getName() throw ( uno::RuntimeException )
{
    return maName;
}

void SAL_CALL VbaDummyCommandBar::setName( const ::rtl::OUString& _name ) throw (uno::RuntimeException)
{
    maName = _name;
}

::sal_Bool SAL_CALL VbaDummyCommandBar::getVisible() throw (uno::RuntimeException)
{
    return sal_True;
}

void SAL_CALL VbaDummyCommandBar::setVisible( ::sal_Bool /*_visible*/ ) throw (uno::RuntimeException)
{
}

::sal_Bool SAL_CALL VbaDummyCommandBar::getEnabled() throw (uno::RuntimeException)
{
    // emulated with Visible
    return getVisible();
}

void SAL_CALL VbaDummyCommandBar::setEnabled( sal_Bool _enabled ) throw (uno::RuntimeException)
{
    // emulated with Visible
    setVisible( _enabled );
}

void SAL_CALL VbaDummyCommandBar::Delete(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // no-op
}

uno::Any SAL_CALL VbaDummyCommandBar::Controls( const uno::Any& aIndex ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< XCommandBarControls > xCommandBarControls( new VbaDummyCommandBarControls( this, mxContext ) );
    if( aIndex.hasValue() )
        return xCommandBarControls->Item( aIndex, uno::Any() );
    return uno::Any( xCommandBarControls );
}

sal_Int32 SAL_CALL VbaDummyCommandBar::Type() throw (script::BasicErrorException, uno::RuntimeException)
{
    return mnType;
}

uno::Any SAL_CALL VbaDummyCommandBar::FindControl( const uno::Any& /*aType*/, const uno::Any& /*aId*/, const uno::Any& /*aTag*/, const uno::Any& /*aVisible*/, const uno::Any& /*aRecursive*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return uno::Any( uno::Reference< XCommandBarControl >() );
}

rtl::OUString& VbaDummyCommandBar::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("VbaDummyCommandBar") );
    return sImplName;
}

uno::Sequence< rtl::OUString > VbaDummyCommandBar::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.CommandBar" ) );
    }
    return aServiceNames;
}
