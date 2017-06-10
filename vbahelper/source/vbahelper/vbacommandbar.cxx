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
#include "vbacommandbar.hxx"
#include "vbacommandbarcontrols.hxx"
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <ooo/vba/office/MsoBarType.hpp>

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaCommandBar::ScVbaCommandBar( const uno::Reference< ov::XHelperInterface >& xParent,
                                  const uno::Reference< uno::XComponentContext >& xContext,
                                  VbaCommandBarHelperRef const & pHelper,
                                  const uno::Reference< container::XIndexAccess >& xBarSettings,
                                  const OUString& sResourceUrl, bool bIsMenu )
   : CommandBar_BASE( xParent, xContext ), pCBarHelper( pHelper ), m_xBarSettings( xBarSettings ), m_sResourceUrl( sResourceUrl ), m_bIsMenu( bIsMenu )
{
}

OUString SAL_CALL
ScVbaCommandBar::getName()
{
    // This will get a "NULL length string" when Name is not set.
    uno::Reference< beans::XPropertySet > xPropertySet( m_xBarSettings, uno::UNO_QUERY_THROW );
    uno::Any aName = xPropertySet->getPropertyValue( "UIName" );
    OUString sName;
    aName >>= sName;
    if( sName.isEmpty() )
    {
        if( m_bIsMenu )
        {
            if( m_sResourceUrl == ITEM_MENUBAR_URL )
            {
                if( pCBarHelper->getModuleId() == "com.sun.star.sheet.SpreadsheetDocument" )
                    sName = "Worksheet Menu Bar";
                else if( pCBarHelper->getModuleId() == "com.sun.star.text.TextDocument" )
                    sName = "Menu Bar";
                return sName;
            }
        }
        // Toolbar name
        uno::Reference< container::XNameAccess > xNameAccess = pCBarHelper->getPersistentWindowState();
        if( xNameAccess->hasByName( m_sResourceUrl ) )
        {
            uno::Sequence< beans::PropertyValue > aToolBar;
            xNameAccess->getByName( m_sResourceUrl ) >>= aToolBar;
            getPropertyValue( aToolBar, "UIName" ) >>= sName;
        }
    }
    return sName;
}
void SAL_CALL
ScVbaCommandBar::setName( const OUString& _name )
{
    uno::Reference< beans::XPropertySet > xPropertySet( m_xBarSettings, uno::UNO_QUERY_THROW );
    xPropertySet->setPropertyValue( "UIName" , uno::makeAny( _name ) );

    pCBarHelper->ApplyTempChange( m_sResourceUrl, m_xBarSettings );
}
sal_Bool SAL_CALL
ScVbaCommandBar::getVisible()
{
    // menu bar is always visible in OOo
    if( m_bIsMenu )
        return true;

    bool bVisible = false;
    try
    {
        uno::Reference< container::XNameAccess > xNameAccess = pCBarHelper->getPersistentWindowState();
        if( xNameAccess->hasByName( m_sResourceUrl ) )
        {
            uno::Sequence< beans::PropertyValue > aToolBar;
            xNameAccess->getByName( m_sResourceUrl ) >>= aToolBar;
            getPropertyValue( aToolBar, "Visible" ) >>= bVisible;
        }
    }
    catch (const uno::Exception&)
    {
    }
    return bVisible;
}
void SAL_CALL
ScVbaCommandBar::setVisible( sal_Bool _visible )
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
    catch(const uno::Exception&)
    {
        SAL_INFO("vbahelper", "SetVisible get an exception" );
    }
}

sal_Bool SAL_CALL
ScVbaCommandBar::getEnabled()
{
    // emulated with Visible
    return getVisible();
}

void SAL_CALL
ScVbaCommandBar::setEnabled( sal_Bool _enabled )
{
    // emulated with Visible
    setVisible( _enabled );
}

void SAL_CALL
ScVbaCommandBar::Delete(  )
{
    pCBarHelper->removeSettings( m_sResourceUrl );
    uno::Reference< container::XNameContainer > xNameContainer( pCBarHelper->getPersistentWindowState(), uno::UNO_QUERY_THROW );
    if( xNameContainer->hasByName( m_sResourceUrl ) )
    {
        xNameContainer->removeByName( m_sResourceUrl );
    }
}
uno::Any SAL_CALL
ScVbaCommandBar::Controls( const uno::Any& aIndex )
{
    uno::Reference< XCommandBarControls > xCommandBarControls( new ScVbaCommandBarControls( this, mxContext, m_xBarSettings, pCBarHelper, m_xBarSettings, m_sResourceUrl ) );
    if( aIndex.hasValue() )
    {
        return xCommandBarControls->Item( aIndex, uno::Any() );
    }
    return uno::makeAny( xCommandBarControls );
}

sal_Int32 SAL_CALL
ScVbaCommandBar::Type()
{
    // #FIXME support msoBarTypePopup
    sal_Int32 nType = office::MsoBarType::msoBarTypePopup;
    nType = m_bIsMenu? office::MsoBarType::msoBarTypeNormal : office::MsoBarType::msoBarTypeMenuBar;
    return nType;
}

uno::Any SAL_CALL
ScVbaCommandBar::FindControl( const uno::Any& /*aType*/, const uno::Any& /*aId*/, const uno::Any& /*aTag*/, const uno::Any& /*aVisible*/, const uno::Any& /*aRecursive*/ )
{
    // alwayse fail to find control
    return uno::makeAny( uno::Reference< XCommandBarControl > () );
}

OUString
ScVbaCommandBar::getServiceImplName()
{
    return OUString("ScVbaCommandBar");
}

uno::Sequence<OUString>
ScVbaCommandBar::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.CommandBar";
    }
    return aServiceNames;
}


VbaDummyCommandBar::VbaDummyCommandBar(
        const uno::Reference< ov::XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const OUString& rName ) :
    CommandBar_BASE( xParent, xContext ),
    maName( rName )
{
}

OUString SAL_CALL VbaDummyCommandBar::getName()
{
    return maName;
}

void SAL_CALL VbaDummyCommandBar::setName( const OUString& _name )
{
    maName = _name;
}

sal_Bool SAL_CALL VbaDummyCommandBar::getVisible()
{
    // #STUB
    return true;
}

void SAL_CALL VbaDummyCommandBar::setVisible( sal_Bool /*_visible*/ )
{
    // #STUB
}

sal_Bool SAL_CALL VbaDummyCommandBar::getEnabled()
{
    // emulated with Visible
    return getVisible();
}

void SAL_CALL VbaDummyCommandBar::setEnabled( sal_Bool _enabled )
{
    // emulated with Visible
    setVisible( _enabled );
}

void SAL_CALL VbaDummyCommandBar::Delete(  )
{
    // no-op
    // #STUB
}

uno::Any SAL_CALL VbaDummyCommandBar::Controls( const uno::Any& aIndex )
{
    uno::Reference< XCommandBarControls > xCommandBarControls( new VbaDummyCommandBarControls( this, mxContext ) );
    if( aIndex.hasValue() )
        return xCommandBarControls->Item( aIndex, uno::Any() );
    return uno::Any( xCommandBarControls );
}

sal_Int32 SAL_CALL VbaDummyCommandBar::Type()
{
    return office::MsoBarType::msoBarTypePopup;
}

uno::Any SAL_CALL VbaDummyCommandBar::FindControl( const uno::Any& /*aType*/, const uno::Any& /*aId*/, const uno::Any& /*aTag*/, const uno::Any& /*aVisible*/, const uno::Any& /*aRecursive*/ )
{
    return uno::Any( uno::Reference< XCommandBarControl >() );
}

OUString VbaDummyCommandBar::getServiceImplName()
{
    return OUString("VbaDummyCommandBar");
}

uno::Sequence< OUString > VbaDummyCommandBar::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.CommandBar";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
