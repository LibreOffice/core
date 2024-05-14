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
#include "vbacommandbarcontrols.hxx"
#include "vbacommandbarcontrol.hxx"
#include <com/sun/star/lang/XSingleComponentFactory.hpp>

#include <comphelper/propertyvalue.hxx>
#include <rtl/ref.hxx>
#include <utility>

using namespace com::sun::star;
using namespace ooo::vba;

namespace {

class CommandBarControlEnumeration : public ::cppu::WeakImplHelper< container::XEnumeration >
{
    //uno::Reference< uno::XComponentContext > m_xContext;
    CommandBarControls_BASE* m_pCommandBarControls;
    sal_Int32 m_nCurrentPosition;
public:
    explicit CommandBarControlEnumeration( CommandBarControls_BASE* pCommandBarControls ) : m_pCommandBarControls( pCommandBarControls ), m_nCurrentPosition( 0 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements() override
    {
        if( m_nCurrentPosition < m_pCommandBarControls->getCount() )
            return true;
        return false;
    }
    virtual uno::Any SAL_CALL nextElement() override
    {
        if( !hasMoreElements() )
            throw container::NoSuchElementException();

        return m_pCommandBarControls->createCollectionObject( uno::Any( m_nCurrentPosition++ ) );
    }
};

}

ScVbaCommandBarControls::ScVbaCommandBarControls( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess>& xIndexAccess, VbaCommandBarHelperRef  pHelper, uno::Reference< container::XIndexAccess>  xBarSettings, const OUString& sResourceUrl ) : CommandBarControls_BASE( xParent, xContext, xIndexAccess ), pCBarHelper(std::move( pHelper )), m_xBarSettings(std::move( xBarSettings )), m_sResourceUrl( sResourceUrl )
{
    m_bIsMenu = sResourceUrl == ITEM_MENUBAR_URL;
}

uno::Sequence< beans::PropertyValue > ScVbaCommandBarControls::CreateMenuItemData( const OUString& sCommandURL,
                                                                                   const OUString& sHelpURL,
                                                                                   const OUString& sLabel,
                                                                                   sal_uInt16 nType,
                                                                                   const uno::Any& aSubMenu,
                                                                                   bool isVisible,
                                                                                   bool isEnabled )
{
    uno::Sequence< beans::PropertyValue > aProps{
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_COMMANDURL, sCommandURL),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_HELPURL, sHelpURL),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_LABEL, sLabel),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_TYPE, nType),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_CONTAINER, aSubMenu),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_ISVISIBLE, isVisible),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_ENABLED, isEnabled)
    };

    return aProps;
}

uno::Sequence< beans::PropertyValue > ScVbaCommandBarControls::CreateToolbarItemData( const OUString& sCommandURL,
                                                                                      const OUString& sHelpURL,
                                                                                      const OUString& sLabel,
                                                                                      sal_uInt16 nType,
                                                                                      const uno::Any& aSubMenu,
                                                                                      bool isVisible,
                                                                                      sal_Int32 nStyle )
{
    uno::Sequence< beans::PropertyValue > aProps{
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_COMMANDURL, sCommandURL),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_HELPURL, sHelpURL),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_LABEL, sLabel),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_TYPE, nType),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_CONTAINER, aSubMenu),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_ISVISIBLE, isVisible),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_STYLE, nStyle)
    };

    return aProps;
}

// XEnumerationAccess
uno::Type SAL_CALL
ScVbaCommandBarControls::getElementType()
{
    return cppu::UnoType<XCommandBarControl>::get();
}

uno::Reference< container::XEnumeration >
ScVbaCommandBarControls::createEnumeration()
{
    return uno::Reference< container::XEnumeration >( new CommandBarControlEnumeration( this ) );
}

uno::Any
ScVbaCommandBarControls::createCollectionObject( const uno::Any& aSource )
{
    sal_Int32 nPosition = -1;
    aSource >>= nPosition;
    uno::Sequence< beans::PropertyValue > aProps;
    m_xIndexAccess->getByIndex( nPosition ) >>= aProps;
    uno::Reference< container::XIndexAccess > xSubMenu;
    getPropertyValue( aProps, ITEM_DESCRIPTOR_CONTAINER ) >>= xSubMenu;
    rtl::Reference<ScVbaCommandBarControl> pNewCommandBarControl;
    if( xSubMenu.is() )
        pNewCommandBarControl = new ScVbaCommandBarPopup( this, mxContext, m_xIndexAccess, pCBarHelper, m_xBarSettings, m_sResourceUrl, nPosition );
    else
        pNewCommandBarControl = new ScVbaCommandBarButton( this, mxContext, m_xIndexAccess, pCBarHelper, m_xBarSettings, m_sResourceUrl, nPosition );

    return uno::Any( uno::Reference< XCommandBarControl > ( pNewCommandBarControl ) );
}

// Methods
uno::Any SAL_CALL
ScVbaCommandBarControls::Item( const uno::Any& aIndex, const uno::Any& /*aIndex*/ )
{
    sal_Int32 nPosition = -1;
    if( aIndex.getValueTypeClass() == uno::TypeClass_STRING )
    {
        OUString sName;
        aIndex >>= sName;
        nPosition = VbaCommandBarHelper::findControlByName( m_xIndexAccess, sName, m_bIsMenu );
    }
    else
    {
        aIndex >>= nPosition;
    }

    if( nPosition < 0 || nPosition >= getCount() )
    {
        throw uno::RuntimeException();
    }

    return createCollectionObject( uno::Any( nPosition ) );
}

uno::Reference< XCommandBarControl > SAL_CALL
ScVbaCommandBarControls::Add( const uno::Any& Type, const uno::Any& Id, const uno::Any& Parameter, const uno::Any& Before, SAL_UNUSED_PARAMETER const uno::Any& )
{
    // Parameter is not supported
    // the following name needs to be individually created;
    OUString sLabel(u"Custom"_ustr);
    OUString sCommandUrl( CUSTOM_MENU_STR + sLabel);
    sal_Int32 nType = office::MsoControlType::msoControlButton;
    sal_Int32 nPosition = 0;

    if( Type.hasValue() )
    {
        Type >>= nType;
    }

    if( nType != office::MsoControlType::msoControlButton &&
        nType != office::MsoControlType::msoControlPopup )
        throw uno::RuntimeException( u"Not implemented"_ustr );

    if( Id.hasValue() || Parameter.hasValue( ) )
    {
        throw uno::RuntimeException( u"Not implemented"_ustr );
    }

    if( Before.hasValue() )
        Before >>= nPosition;
    else
        nPosition = m_xIndexAccess->getCount();

    uno::Any aSubMenu;
    if( nType == office::MsoControlType::msoControlPopup )
    {
        // it is a Popmenu
        uno::Reference< lang::XSingleComponentFactory > xSCF( m_xBarSettings, uno::UNO_QUERY_THROW );
        aSubMenu <<= xSCF->createInstanceWithContext( mxContext );
    }

    // create control
    uno::Sequence< beans::PropertyValue > aProps;
    sal_uInt16 nItemType = 0;
    if( m_bIsMenu )
    {
        aProps = CreateMenuItemData( sCommandUrl, u""_ustr, sLabel, nItemType, aSubMenu, true, true );
    }
    else
    {
        aProps = CreateToolbarItemData( sCommandUrl, u""_ustr, sLabel, nItemType, aSubMenu, true/*isVisible*/, 0/*nStyle*/ );
    }


    uno::Reference< container::XIndexContainer > xIndexContainer( m_xIndexAccess, uno::UNO_QUERY_THROW );
    xIndexContainer->insertByIndex( nPosition, uno::Any( aProps ) );

    pCBarHelper->ApplyTempChange( m_sResourceUrl, m_xBarSettings );

    rtl::Reference<ScVbaCommandBarControl> pNewCommandBarControl;
    if( nType == office::MsoControlType::msoControlPopup )
        pNewCommandBarControl = new ScVbaCommandBarPopup( this, mxContext, m_xIndexAccess, pCBarHelper, m_xBarSettings, m_sResourceUrl, nPosition );
    else
        pNewCommandBarControl = new ScVbaCommandBarButton( this, mxContext, m_xIndexAccess, pCBarHelper, m_xBarSettings, m_sResourceUrl, nPosition );

    return pNewCommandBarControl;
}

// XHelperInterface
OUString
ScVbaCommandBarControls::getServiceImplName()
{
    return u"ScVbaCommandBarControls"_ustr;
}

uno::Sequence<OUString>
ScVbaCommandBarControls::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.CommandBarControls"_ustr
    };
    return aServiceNames;
}

namespace {

class VbaDummyIndexAccess : public ::cppu::WeakImplHelper< container::XIndexAccess >
{
public:
    VbaDummyIndexAccess() {}
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) override
        { return 0; }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 /*Index*/ ) override
        { throw lang::IndexOutOfBoundsException(); }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override
        { return cppu::UnoType<XCommandBarControl>::get(); }
    virtual sal_Bool SAL_CALL hasElements(  ) override
        { return false; }
};

}

VbaDummyCommandBarControls::VbaDummyCommandBarControls(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext ) :
    CommandBarControls_BASE( xParent, xContext, new VbaDummyIndexAccess )
{
}

// XEnumerationAccess
uno::Type SAL_CALL VbaDummyCommandBarControls::getElementType()
{
    return cppu::UnoType<XCommandBarControl>::get();
}

uno::Reference< container::XEnumeration > VbaDummyCommandBarControls::createEnumeration()
{
    return uno::Reference< container::XEnumeration >( new CommandBarControlEnumeration( this ) );
}

uno::Any VbaDummyCommandBarControls::createCollectionObject( const uno::Any& /*aSource*/ )
{
    return uno::Any( uno::Reference< XCommandBarControl >() );
}

// Methods
uno::Any SAL_CALL VbaDummyCommandBarControls::Item( const uno::Any& /*aIndex*/, const uno::Any& /*aIndex*/ )
{
    return uno::Any( uno::Reference< XCommandBarControl >() );
}

uno::Reference< XCommandBarControl > SAL_CALL VbaDummyCommandBarControls::Add(
        const uno::Any& /*Type*/, const uno::Any& /*Id*/, const uno::Any& /*Parameter*/, const uno::Any& /*Before*/, const uno::Any& /*Temporary*/ )
{
    return uno::Reference< XCommandBarControl >();
}

// XHelperInterface
OUString VbaDummyCommandBarControls::getServiceImplName()
{
    return u"VbaDummyCommandBarControls"_ustr;
}

uno::Sequence<OUString> VbaDummyCommandBarControls::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.CommandBarControls"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
