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
#include "vbacommandbarcontrols.hxx"
#include "vbacommandbarcontrol.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

typedef ::cppu::WeakImplHelper1< container::XEnumeration > CommandBarControlEnumeration_BASE;
class CommandBarControlEnumeration : public CommandBarControlEnumeration_BASE
{
    //uno::Reference< uno::XComponentContext > m_xContext;
    CommandBarControls_BASE* m_pCommandBarControls;
    sal_Int32 m_nCurrentPosition;
public:
    CommandBarControlEnumeration( CommandBarControls_BASE* pCommandBarControls ) : m_pCommandBarControls( pCommandBarControls ), m_nCurrentPosition( 0 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements() throw ( uno::RuntimeException )
    {
        if( m_nCurrentPosition < m_pCommandBarControls->getCount() )
            return sal_True;
        return sal_False;
    }
    virtual uno::Any SAL_CALL nextElement() throw ( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
    {
        if( hasMoreElements() )
        {
            return m_pCommandBarControls->createCollectionObject( uno::makeAny( m_nCurrentPosition++ ) );
        }
        else
            throw container::NoSuchElementException();
    }
};

ScVbaCommandBarControls::ScVbaCommandBarControls( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess>& xIndexAccess, VbaCommandBarHelperRef pHelper, const uno::Reference< container::XIndexAccess>& xBarSettings, const rtl::OUString& sResourceUrl ) throw (uno::RuntimeException) : CommandBarControls_BASE( xParent, xContext, xIndexAccess ), pCBarHelper( pHelper ), m_xBarSettings( xBarSettings ), m_sResourceUrl( sResourceUrl )
{
    m_bIsMenu = sResourceUrl.equalsAscii( ITEM_MENUBAR_URL ) ? sal_True : sal_False;
}

uno::Sequence< beans::PropertyValue > ScVbaCommandBarControls::CreateMenuItemData( const rtl::OUString& sCommandURL,
                                                                                   const rtl::OUString& sHelpURL,
                                                                                   const rtl::OUString& sLabel,
                                                                                   sal_uInt16 nType,
                                                                                   const uno::Any& aSubMenu,
                                                                                   sal_Bool isVisible,
                                                                                   sal_Bool isEnabled )
{
    uno::Sequence< beans::PropertyValue > aProps(7);

    aProps[0].Name = rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_COMMANDURL );
    aProps[0].Value <<= sCommandURL;
    aProps[1].Name = rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_HELPURL );
    aProps[1].Value <<= sHelpURL;
    aProps[2].Name = rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_LABEL );
    aProps[2].Value <<= sLabel;
    aProps[3].Name = rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_TYPE );
    aProps[3].Value <<= nType;
    aProps[4].Name = rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_CONTAINER );
    aProps[4].Value = aSubMenu;
    aProps[5].Name = rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_ISVISIBLE );
    aProps[5].Value <<= isVisible;
    aProps[6].Name = rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_ENABLED );
    aProps[6].Value <<= isEnabled;

    return aProps;
}

uno::Sequence< beans::PropertyValue > ScVbaCommandBarControls::CreateToolbarItemData( const rtl::OUString& sCommandURL, const rtl::OUString& sHelpURL, const rtl::OUString& sLabel, sal_uInt16 nType, const uno::Any& aSubMenu, sal_Bool isVisible, sal_Int32 nStyle )
{
    uno::Sequence< beans::PropertyValue > aProps(7);

    aProps[0].Name = rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_COMMANDURL );
    aProps[0].Value <<= sCommandURL;
    aProps[1].Name = rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_HELPURL );
    aProps[1].Value <<= sHelpURL;
    aProps[2].Name = rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_LABEL );
    aProps[2].Value <<= sLabel;
    aProps[3].Name = rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_TYPE );
    aProps[3].Value <<= nType;
    aProps[4].Name = rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_CONTAINER );
    aProps[4].Value = aSubMenu;
    aProps[5].Name = rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_ISVISIBLE );
    aProps[5].Value <<= isVisible;
    aProps[6].Name = rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_STYLE );
    aProps[6].Value <<= nStyle;

    return aProps;
}

// XEnumerationAccess
uno::Type SAL_CALL
ScVbaCommandBarControls::getElementType() throw ( uno::RuntimeException )
{
    return XCommandBarControl::static_type( 0 );
}

uno::Reference< container::XEnumeration >
ScVbaCommandBarControls::createEnumeration() throw ( uno::RuntimeException )
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
    getPropertyValue( aProps, rtl::OUString::createFromAscii( ITEM_DESCRIPTOR_CONTAINER ) ) >>= xSubMenu;
    ScVbaCommandBarControl* pNewCommandBarControl = NULL;
    if( xSubMenu.is() )
        pNewCommandBarControl = new ScVbaCommandBarPopup( this, mxContext, m_xIndexAccess, pCBarHelper, m_xBarSettings, m_sResourceUrl, nPosition, sal_True );
    else
        pNewCommandBarControl = new ScVbaCommandBarButton( this, mxContext, m_xIndexAccess, pCBarHelper, m_xBarSettings, m_sResourceUrl, nPosition, sal_True );

    return uno::makeAny( uno::Reference< XCommandBarControl > ( pNewCommandBarControl ) );
}

// Methods
uno::Any SAL_CALL
ScVbaCommandBarControls::Item( const uno::Any& aIndex, const uno::Any& /*aIndex*/ ) throw (uno::RuntimeException)
{
    sal_Int32 nPosition = -1;
    if( aIndex.getValueTypeClass() == uno::TypeClass_STRING )
    {
        rtl::OUString sName;
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

    return createCollectionObject( uno::makeAny( nPosition ) );
}

uno::Reference< XCommandBarControl > SAL_CALL
ScVbaCommandBarControls::Add( const uno::Any& Type, const uno::Any& Id, const uno::Any& Parameter, const uno::Any& Before, const uno::Any& Temporary ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // Parameter is not supported
    // the following name needs to be individually created;
    rtl::OUString sLabel( rtl::OUString::createFromAscii("Custom") );
    rtl::OUString sCommandUrl = rtl::OUString::createFromAscii( CUSTOM_MENU_STR ) + sLabel;
    sal_Int32 nType = office::MsoControlType::msoControlButton;
    sal_Int32 nPosition = 0;
    sal_Bool bTemporary = sal_True;

    if( Type.hasValue() )
    {
        Type >>= nType;
    }

    if( nType != office::MsoControlType::msoControlButton &&
        nType != office::MsoControlType::msoControlPopup )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );

    if( Id.hasValue() || Parameter.hasValue( ) )
    {
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Not implemented") ), uno::Reference< uno::XInterface >() );
    }

    if( Before.hasValue() )
        Before >>= nPosition;
    else
        nPosition = m_xIndexAccess->getCount();

    if( Temporary.hasValue() )
        Temporary >>= bTemporary;

    uno::Any aSubMenu;
    if( nType == office::MsoControlType::msoControlPopup )
    {
        // it is a Popmenu
        uno::Reference< lang::XSingleComponentFactory > xSCF( m_xBarSettings, uno::UNO_QUERY_THROW );
        aSubMenu <<= xSCF->createInstanceWithContext( mxContext );
    }

    // create control
    uno::Sequence< beans::PropertyValue > aProps;
    rtl::OUString sHelpUrl;
    sal_uInt16 nItemType = 0;
    if( IsMenu() )
    {
        aProps = CreateMenuItemData( sCommandUrl, sHelpUrl, sLabel, nItemType, aSubMenu, true, true );
    }
    else
    {
        sal_Bool isVisible = sal_True;
        sal_Int32 nStyle = 0;
        aProps = CreateToolbarItemData( sCommandUrl, sHelpUrl, sLabel, nItemType, aSubMenu, isVisible, nStyle );
    }


    uno::Reference< container::XIndexContainer > xIndexContainer( m_xIndexAccess, uno::UNO_QUERY_THROW );
    xIndexContainer->insertByIndex( nPosition, uno::makeAny( aProps ) );

    pCBarHelper->ApplyChange( m_sResourceUrl, m_xBarSettings );

    ScVbaCommandBarControl* pNewCommandBarControl = NULL;
    if( nType == office::MsoControlType::msoControlPopup )
        pNewCommandBarControl = new ScVbaCommandBarPopup( this, mxContext, m_xIndexAccess, pCBarHelper, m_xBarSettings, m_sResourceUrl, nPosition, bTemporary );
    else
        pNewCommandBarControl = new ScVbaCommandBarButton( this, mxContext, m_xIndexAccess, pCBarHelper, m_xBarSettings, m_sResourceUrl, nPosition, bTemporary );

    return uno::Reference< XCommandBarControl >( pNewCommandBarControl );
}

// XHelperInterface
rtl::OUString&
ScVbaCommandBarControls::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaCommandBarControls") );
    return sImplName;
}
uno::Sequence<rtl::OUString>
ScVbaCommandBarControls::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.CommandBarControls" ) );
    }
    return aServiceNames;
}

// ============================================================================

class VbaDummyIndexAccess : public ::cppu::WeakImplHelper1< container::XIndexAccess >
{
public:
    inline VbaDummyIndexAccess() {}
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
        { return 0; }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 /*Index*/ ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
        { throw lang::IndexOutOfBoundsException(); }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
        { return XCommandBarControl::static_type( 0 ); }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException)
        { return false; }
};

// ----------------------------------------------------------------------------

VbaDummyCommandBarControls::VbaDummyCommandBarControls(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext ) throw (uno::RuntimeException) :
    CommandBarControls_BASE( xParent, xContext, new VbaDummyIndexAccess )
{
}

// XEnumerationAccess
uno::Type SAL_CALL VbaDummyCommandBarControls::getElementType() throw ( uno::RuntimeException )
{
    return XCommandBarControl::static_type( 0 );
}

uno::Reference< container::XEnumeration > VbaDummyCommandBarControls::createEnumeration() throw ( uno::RuntimeException )
{
    return uno::Reference< container::XEnumeration >( new CommandBarControlEnumeration( this ) );
}

uno::Any VbaDummyCommandBarControls::createCollectionObject( const uno::Any& /*aSource*/ )
{
    return uno::Any( uno::Reference< XCommandBarControl >() );
}

// Methods
uno::Any SAL_CALL VbaDummyCommandBarControls::Item( const uno::Any& /*aIndex*/, const uno::Any& /*aIndex*/ ) throw (uno::RuntimeException)
{
    return uno::Any( uno::Reference< XCommandBarControl >() );
}

uno::Reference< XCommandBarControl > SAL_CALL VbaDummyCommandBarControls::Add(
        const uno::Any& /*Type*/, const uno::Any& /*Id*/, const uno::Any& /*Parameter*/, const uno::Any& /*Before*/, const uno::Any& /*Temporary*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return uno::Reference< XCommandBarControl >();
}

// XHelperInterface
rtl::OUString& VbaDummyCommandBarControls::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("VbaDummyCommandBarControls") );
    return sImplName;
}

uno::Sequence<rtl::OUString> VbaDummyCommandBarControls::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.CommandBarControls" ) );
    }
    return aServiceNames;
}
