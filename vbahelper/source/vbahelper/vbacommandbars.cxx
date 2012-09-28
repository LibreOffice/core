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
    uno::Sequence< OUString > m_sNames;
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
            OUString sResourceUrl( m_sNames[ m_nCurrentPosition++ ] );
            if( sResourceUrl.indexOf( "private:resource/toolbar/" ) != -1 )
            {
                uno::Reference< container::XIndexAccess > xCBarSetting = m_pCBarHelper->getSettings( sResourceUrl );
                uno::Reference< XCommandBar > xCommandBar( new ScVbaCommandBar( m_xParent, m_xContext, m_pCBarHelper, xCBarSetting, sResourceUrl, sal_False ) );
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
    OUString sResourceUrl;
    uno::Reference< container::XIndexAccess > xBarSettings;
    OUString sBarName;
    sal_Bool bMenu = sal_False;
    uno::Any aRet;

    if( aSource >>= sBarName )
    {
        // some built-in command bars
        if( m_pCBarHelper->getModuleId() == "com.sun.star.sheet.SpreadsheetDocument" )
        {
            if( sBarName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM("Worksheet Menu Bar") ) )
            {
                // spreadsheet menu bar
                sResourceUrl = ITEM_MENUBAR_URL;
                bMenu = sal_True;
            }
            else if( sBarName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM("Cell") ) )
            {
                // EVIL HACK (tm): spreadsheet cell context menu as dummy object without functionality
                aRet <<= uno::Reference< XCommandBar >( new VbaDummyCommandBar( this, mxContext, sBarName, office::MsoBarType::msoBarTypePopup ) );
            }
        }
        else if( m_pCBarHelper->getModuleId() == "com.sun.star.text.TextDocument" )
        {
            if( sBarName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM("Menu Bar") ) )
            {
                // text processor menu bar
                sResourceUrl = ITEM_MENUBAR_URL;
                bMenu = sal_True;
            }
        }

        // nothing found - try to resolve from name
        if( !aRet.hasValue() && sResourceUrl.isEmpty() )
        {
            sResourceUrl = m_pCBarHelper->findToolbarByName( m_xNameAccess, sBarName );
            bMenu = sal_False;
        }
    }

    if( !sResourceUrl.isEmpty() )
    {
        xBarSettings = m_pCBarHelper->getSettings( sResourceUrl );
        aRet <<= uno::Reference< XCommandBar >( new ScVbaCommandBar( this, mxContext, m_pCBarHelper, xBarSettings, sResourceUrl, bMenu ) );
    }

    if( !aRet.hasValue() )
        throw uno::RuntimeException( "Toolbar do not exist" , uno::Reference< uno::XInterface >() );

    return aRet;
}

// XCommandBars
uno::Reference< XCommandBar > SAL_CALL
ScVbaCommandBars::Add( const css::uno::Any& Name, const css::uno::Any& /*Position*/, const css::uno::Any& /*MenuBar*/, const css::uno::Any& /*Temporary*/ ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    // FIXME: only support to add Toolbar
    // Position - MsoBar MenuBar - sal_Bool
    // Currently only the Name is supported.
    OUString sName;
    if( Name.hasValue() )
        Name >>= sName;

    OUString sResourceUrl;
    if( !sName.isEmpty() )
    {
        sResourceUrl = m_pCBarHelper->findToolbarByName( m_xNameAccess, sName );
        if( !sResourceUrl.isEmpty() )
            throw uno::RuntimeException( "Toolbar exists" , uno::Reference< uno::XInterface >() );
    }
    else
    {
        sName = "Custom1";
    }

    sResourceUrl = VbaCommandBarHelper::generateCustomURL();
    uno::Reference< container::XIndexAccess > xBarSettings( m_pCBarHelper->getSettings( sResourceUrl ), uno::UNO_QUERY_THROW );
    uno::Reference< XCommandBar > xCBar( new ScVbaCommandBar( this, mxContext, m_pCBarHelper, xBarSettings, sResourceUrl, sal_False ) );
    xCBar->setName( sName );
    return xCBar;
}
sal_Int32 SAL_CALL
ScVbaCommandBars::getCount() throw(css::uno::RuntimeException)
{
    // Filter out all toolbars from the window collection
    sal_Int32 nCount = 1; // there is a Menubar in OOo
    uno::Sequence< ::OUString > allNames = m_xNameAccess->getElementNames();
    for( sal_Int32 i = 0; i < allNames.getLength(); i++ )
    {
        if(allNames[i].indexOf( "private:resource/toolbar/" ) != -1 )
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
        if( m_pCBarHelper->getModuleId() == "com.sun.star.sheet.SpreadsheetDocument" )
            aSource <<= OUString("Worksheet Menu Bar");
        else if( m_pCBarHelper->getModuleId() == "com.sun.star.text.TextDocument" )
            aSource <<= OUString("Menu Bar");
        if( aSource.hasValue() )
            return createCollectionObject( aSource );
    }
    return uno::Any();
}

// XHelperInterface
OUString
ScVbaCommandBars::getServiceImplName()
{
    return OUString("ScVbaCommandBars");
}

uno::Sequence<OUString>
ScVbaCommandBars::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.CommandBars";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
