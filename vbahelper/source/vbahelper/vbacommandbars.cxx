/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <ooo/vba/office/MsoBarType.hpp>

#include "vbacommandbars.hxx"
#include "vbacommandbar.hxx"

using namespace com::sun::star;
using namespace ooo::vba;


typedef ::cppu::WeakImplHelper1< container::XEnumeration > CommandBarEnumeration_BASE;

class CommandBarEnumeration : public CommandBarEnumeration_BASE
{
    uno::Reference< XHelperInterface > m_xParent;
    uno::Reference< uno::XComponentContext > m_xContext;
    VbaCommandBarHelperRef m_pCBarHelper;
    uno::Sequence< rtl::OUString > m_sNames;
    sal_Int32 m_nCurrentPosition;
public:
    CommandBarEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, VbaCommandBarHelperRef pHelper) throw ( uno::RuntimeException ) : m_xParent( xParent ), m_xContext( xContext ), m_pCBarHelper( pHelper ) , m_nCurrentPosition( 0 )
    {
        uno::Reference< container::XNameAccess > xNameAccess = m_pCBarHelper->getPersistentWindowState();
        m_sNames = xNameAccess->getElementNames();
    }
    virtual sal_Bool SAL_CALL hasMoreElements() throw ( uno::RuntimeException )
    {
        if( m_nCurrentPosition < m_sNames.getLength() )
            return sal_True;
        return sal_False;
    }
    virtual uno::Any SAL_CALL nextElement() throw ( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
    {
        // FIXME: should be add menubar
        if( hasMoreElements() )
        {
            rtl::OUString sResourceUrl( m_sNames[ m_nCurrentPosition++ ] );
            if( sResourceUrl.indexOf( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:resource/toolbar/")) ) != -1 )
            {
                uno::Reference< container::XIndexAccess > xCBarSetting = m_pCBarHelper->getSettings( sResourceUrl );
                uno::Reference< XCommandBar > xCommandBar( new ScVbaCommandBar( m_xParent, m_xContext, m_pCBarHelper, xCBarSetting, sResourceUrl, sal_False, sal_False ) );
             }
             else
                return nextElement();
        }
        else
            throw container::NoSuchElementException();
        return uno::Any();
    }
};

ScVbaCommandBars::ScVbaCommandBars( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess, const uno::Reference< frame::XModel >& xModel ) throw ( uno::RuntimeException ) : CommandBars_BASE( xParent, xContext, xIndexAccess )
{
    m_pCBarHelper.reset( new VbaCommandBarHelper( mxContext, xModel ) );
    m_xNameAccess = m_pCBarHelper->getPersistentWindowState();
}

ScVbaCommandBars::~ScVbaCommandBars()
{
}

// XEnumerationAccess
uno::Type SAL_CALL
ScVbaCommandBars::getElementType() throw ( uno::RuntimeException )
{
    return XCommandBar::static_type( 0 );
}

uno::Reference< container::XEnumeration >
ScVbaCommandBars::createEnumeration() throw ( uno::RuntimeException )
{
    return uno::Reference< container::XEnumeration >( new CommandBarEnumeration( this, mxContext, m_pCBarHelper ) );
}

uno::Any
ScVbaCommandBars::createCollectionObject( const uno::Any& aSource )
{
    // aSource should be a name at this time, because of the class is API wrapper.
    rtl::OUString sResourceUrl;
    uno::Reference< container::XIndexAccess > xBarSettings;
    rtl::OUString sBarName;
    sal_Bool bMenu = sal_False;
    uno::Any aRet;

    if( aSource >>= sBarName )
    {
        // some built-in command bars
        if( m_pCBarHelper->getModuleId().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.sheet.SpreadsheetDocument") ) )
        {
            if( sBarName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM("Worksheet Menu Bar") ) )
            {
                // spreadsheet menu bar
                sResourceUrl = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ITEM_MENUBAR_URL ));
                bMenu = sal_True;
            }
            else if( sBarName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM("Cell") ) )
            {
                // EVIL HACK (tm): spreadsheet cell context menu as dummy object without functionality
                aRet <<= uno::Reference< XCommandBar >( new VbaDummyCommandBar( this, mxContext, sBarName, office::MsoBarType::msoBarTypePopup ) );
            }
        }
        else if( m_pCBarHelper->getModuleId().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.TextDocument") ) )
        {
            if( sBarName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM("Menu Bar") ) )
            {
                // text processor menu bar
                sResourceUrl = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ITEM_MENUBAR_URL ));
                bMenu = sal_True;
            }
        }

        // nothing found - try to resolve from name
        if( !aRet.hasValue() && (sResourceUrl.getLength() == 0) )
        {
            sResourceUrl = m_pCBarHelper->findToolbarByName( m_xNameAccess, sBarName );
            bMenu = sal_False;
        }
    }

    if( sResourceUrl.getLength() )
    {
        xBarSettings = m_pCBarHelper->getSettings( sResourceUrl );
        aRet <<= uno::Reference< XCommandBar >( new ScVbaCommandBar( this, mxContext, m_pCBarHelper, xBarSettings, sResourceUrl, bMenu, sal_False ) );
    }

    if( !aRet.hasValue() )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Toolbar do not exist") ), uno::Reference< uno::XInterface >() );

    return aRet;
}

// XCommandBars
uno::Reference< XCommandBar > SAL_CALL
ScVbaCommandBars::Add( const css::uno::Any& Name, const css::uno::Any& /*Position*/, const css::uno::Any& /*MenuBar*/, const css::uno::Any& Temporary ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    // FIXME: only support to add Toolbar
    // Position - MsoBar MenuBar - sal_Bool
    // Currently only the Name is supported.
    rtl::OUString sName;
    if( Name.hasValue() )
        Name >>= sName;

    rtl::OUString sResourceUrl;
    if( sName.getLength() )
    {
        sResourceUrl = m_pCBarHelper->findToolbarByName( m_xNameAccess, sName );
        if( sResourceUrl.getLength() )
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Toolbar exists") ), uno::Reference< uno::XInterface >() );
    }
    else
    {
        sName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Custom1") );
    }

    sal_Bool bTemporary = sal_False;
    if( Temporary.hasValue() )
        Temporary >>= bTemporary;

    sResourceUrl = VbaCommandBarHelper::generateCustomURL();
    uno::Reference< container::XIndexAccess > xBarSettings( m_pCBarHelper->getSettings( sResourceUrl ), uno::UNO_QUERY_THROW );
    uno::Reference< XCommandBar > xCBar( new ScVbaCommandBar( this, mxContext, m_pCBarHelper, xBarSettings, sResourceUrl, sal_False, bTemporary ) );
    xCBar->setName( sName );
    return xCBar;
}
sal_Int32 SAL_CALL
ScVbaCommandBars::getCount() throw(css::uno::RuntimeException)
{
    // Filter out all toolbars from the window collection
    sal_Int32 nCount = 1; // there is a Menubar in OOo
    uno::Sequence< ::rtl::OUString > allNames = m_xNameAccess->getElementNames();
    for( sal_Int32 i = 0; i < allNames.getLength(); i++ )
    {
        if(allNames[i].indexOf( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:resource/toolbar/")) ) != -1 )
        {
            nCount++;
        }
    }
    return nCount;
}

// ScVbaCollectionBaseImpl
uno::Any SAL_CALL
ScVbaCommandBars::Item( const uno::Any& aIndex, const uno::Any& /*aIndex2*/ ) throw( uno::RuntimeException )
{
    if( aIndex.getValueTypeClass() == uno::TypeClass_STRING )
    {
        return createCollectionObject( aIndex );
    }

    // hardcode if "aIndex = 1" that would return "main menu".
    sal_Int16 nIndex = 0;
    aIndex >>= nIndex;
    if( nIndex == 1 )
    {
        uno::Any aSource;
        if( m_pCBarHelper->getModuleId().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.sheet.SpreadsheetDocument" ) ) )
            aSource <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Worksheet Menu Bar"));
        else if( m_pCBarHelper->getModuleId().equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.TextDocument")) )
            aSource <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Menu Bar"));
        if( aSource.hasValue() )
            return createCollectionObject( aSource );
    }
    return uno::Any();
}

// XHelperInterface
rtl::OUString&
ScVbaCommandBars::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaCommandBars") );
    return sImplName;
}
uno::Sequence<rtl::OUString>
ScVbaCommandBars::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.CommandBars" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
