/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "vbacommandbarcontrol.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

uno::Any lcl_getPropertyValue( beans::PropertyValues aPropertyValues, rtl::OUString sPropertyName )
{
    sal_Int32 nCount = aPropertyValues.getLength();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        if( aPropertyValues[i].Name.equalsIgnoreAsciiCase( sPropertyName ) )
        {
            return aPropertyValues[i].Value;
        }
    }
    return uno::Any();
}

beans::PropertyValues lcl_repalcePropertyValue( beans::PropertyValues aPropertyValues, rtl::OUString sPropertyName, uno::Any aValue )
{
    sal_Int32 nCount = aPropertyValues.getLength();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        if( aPropertyValues[i].Name.equalsIgnoreAsciiCase( sPropertyName ) )
        {
            aPropertyValues[i].Value = aValue;
            return aPropertyValues;
        }
    }
    return aPropertyValues;
}

ScVbaCommandBarControl::ScVbaCommandBarControl( const uno::Reference< XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, rtl::OUString sName ) throw (uno::RuntimeException) : CommandBarControl_BASE( xParent, xContext ), m_sName( sName )
{
    // exsiting CommandBarBarControl
    m_xParentHardRef.set( xParent, uno::UNO_QUERY_THROW );
    initObjects();
    if( m_xBarSettings->hasElements() )
    {
        ScVbaCommandBarControl* pParentCommandBarControl = m_pCommandBarControls->GetParentCommandBarControl();
        if( pParentCommandBarControl )
        {
            beans::PropertyValues aPropertyValues;
            pParentCommandBarControl->GetCurrentSettings()->getByIndex( pParentCommandBarControl->GetPosition() ) >>= aPropertyValues;
            pParentCommandBarControl->SetPropertyValues( aPropertyValues );
            m_xCurrentSettings.set( lcl_getPropertyValue( pParentCommandBarControl->GetPropertyValues(), rtl::OUString::createFromAscii( "ItemDescriptorContainer" ) ), uno::UNO_QUERY_THROW );
        }
        if( !m_xCurrentSettings.is() )
        {
            m_xCurrentSettings.set( m_xUICfgManager->getSettings( m_pCommandBarControls->GetParentToolBarName(), sal_True ), uno::UNO_QUERY_THROW );
        }
        for( sal_Int32 i = 0; i < m_xCurrentSettings->getCount(); i++ )
        {
            beans::PropertyValues aPropertyValuesTemp;
            m_xCurrentSettings->getByIndex( i ) >>= aPropertyValuesTemp;
            // Label always empty in OOo
            rtl::OUString sLabel;
            lcl_getPropertyValue( aPropertyValuesTemp, rtl::OUString::createFromAscii( "Label" ) ) >>= sLabel;
            if( sLabel.equalsIgnoreAsciiCase( sName ) )
            {
                m_nPosition = i;
                break;
            }
            // using CammandURL to find
            rtl::OUString sCommandURL;
            lcl_getPropertyValue( aPropertyValuesTemp, rtl::OUString::createFromAscii( "CommandURL" ) ) >>= sCommandURL;
            sal_Int32 nLastIndex = sCommandURL.lastIndexOf( rtl::OUString::createFromAscii(":") );
            if( ( nLastIndex != -1 ) && ( ( nLastIndex + 1 ) < sCommandURL.getLength() ) )
            {
                sCommandURL = sCommandURL.copy( nLastIndex + 1 );
            }
            if( sCommandURL.equalsIgnoreAsciiCase( sName ) )
            {
                m_nPosition = i;
                break;
            }
        }
        if( m_nPosition  == -1 )
            throw uno::RuntimeException( rtl::OUString::createFromAscii("The CommandBarControl do not exist"), uno::Reference< uno::XInterface >() );
    }
    if( m_bIsMenu )
    {
        m_sBarName = rtl::OUString::createFromAscii("private:resource/menubar/menubar");
    }
    else
    {
        m_sBarName = m_pCommandBarControls->GetParentToolBarName();
    }
    m_bTemporary = sal_True;
}
ScVbaCommandBarControl::ScVbaCommandBarControl( const uno::Reference< XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, rtl::OUString sName, rtl::OUString sCommand, sal_Int32 nPosition, sal_Bool bTemporary ) throw (uno::RuntimeException) :  CommandBarControl_BASE( xParent, xContext ), m_nPosition( nPosition ), m_bTemporary( bTemporary )
{
    m_xParentHardRef.set( xParent, uno::UNO_QUERY_THROW );
    initObjects();
    if( sName.getLength() > 0 )
    {
        m_sName = sName;
    }
    if( sCommand.getLength() > 0 )
    {
        m_sCommand = sCommand;
    }
    else
    {
        m_sCommand = rtl::OUString::createFromAscii("vnd.openoffice.org:") + sName;
    }
    if( m_bIsMenu )
    {
        m_sBarName = rtl::OUString::createFromAscii("private:resource/menubar/menubar");
        createNewMenuBarControl();
    }
    else
    {
        m_sBarName = m_pCommandBarControls->GetParentToolBarName();
        createNewToolBarControl();
    }
}

void
ScVbaCommandBarControl::initObjects() throw (uno::RuntimeException)
{
    m_pCommandBarControls = dynamic_cast< ScVbaCommandBarControls* >( m_xParentHardRef.get() );
    if( !m_pCommandBarControls )
        throw uno::RuntimeException( rtl::OUString::createFromAscii( "Parent needs to be a ScVbaCommandBarControls"), uno::Reference< uno::XInterface >() );
    m_xUICfgManager.set( m_pCommandBarControls->GetUICfgManager(), uno::UNO_QUERY_THROW );
    m_xUICfgPers.set( m_pCommandBarControls->GetUICfgPers(), uno::UNO_QUERY_THROW );
    m_xBarSettings.set( m_pCommandBarControls->GetBarSettings(), uno::UNO_QUERY_THROW );
    m_bIsMenu = m_pCommandBarControls->IsMenu();
    m_sName = rtl::OUString::createFromAscii( "Custom" );
    m_nPosition = -1;
}

void
ScVbaCommandBarControl::createNewMenuBarControl()
{
    uno::Sequence< beans::PropertyValue > aPropertys(4);
    aPropertys[0].Name = rtl::OUString::createFromAscii("CommandURL");
    aPropertys[0].Value <<= m_sCommand;
    aPropertys[1].Name = rtl::OUString::createFromAscii("Label");
    aPropertys[1].Value <<= m_sName;
    aPropertys[2].Name = rtl::OUString::createFromAscii("Type");
    aPropertys[2].Value <<= m_nType;
    aPropertys[3].Name = rtl::OUString::createFromAscii("ItemDescriptorContainer");

    m_xBarSettings->insertByIndex( m_nPosition, uno::makeAny( aPropertys ) );
    uno::Reference< beans::XPropertySet > xPropertySet( m_xBarSettings, uno::UNO_QUERY_THROW );
    rtl::OUString sUIName;
    xPropertySet->getPropertyValue( rtl::OUString::createFromAscii("UIName") ) >>= sUIName;

    uno::Reference< lang::XSingleComponentFactory > xMenuMSF( m_xBarSettings, uno::UNO_QUERY_THROW );
    if( m_pCommandBarControls->GetParentCommandBar() != NULL )
    {
        // create a new menu
        m_xBarSettings->insertByIndex( m_nPosition, uno::makeAny( aPropertys ) );
        m_xCurrentSettings.set( m_xBarSettings, uno::UNO_QUERY_THROW );
    }
    else if( m_pCommandBarControls->GetParentCommandBarControl() != NULL )
    {
        // create a new menu entry
        // change the parent MenuItem to a PopupMenu
        ScVbaCommandBarControl* pPc = m_pCommandBarControls->GetParentCommandBarControl();
        beans::PropertyValues aPropertyValues;
        pPc->GetCurrentSettings()->getByIndex( pPc->GetPosition() ) >>= aPropertyValues;
        pPc->SetPropertyValues( aPropertyValues );

        // has the property already been set?
        if( lcl_getPropertyValue( pPc->GetPropertyValues(), rtl::OUString::createFromAscii("ItemDescriptorContainer") ).hasValue() )
        {
            lcl_repalcePropertyValue(  pPc->GetPropertyValues(), rtl::OUString::createFromAscii("ItemDescriptorContainer"), uno::makeAny( xMenuMSF->createInstanceWithContext( mxContext ) ) );
            pPc->GetCurrentSettings()->replaceByIndex( pPc->GetPosition(), uno::makeAny( pPc->GetPropertyValues() ) );
        }
        m_xCurrentSettings.set( lcl_getPropertyValue( pPc->GetPropertyValues(), rtl::OUString::createFromAscii("ItemDescriptorContainer") ), uno::UNO_QUERY_THROW );
        m_xCurrentSettings->insertByIndex( m_nPosition, uno::makeAny( m_aPropertyValues ) );
    }
    if( m_xUICfgManager->hasSettings( m_sBarName ) )
    {
        m_xUICfgManager->replaceSettings( m_sBarName, uno::Reference< container::XIndexAccess > (m_xBarSettings, uno::UNO_QUERY_THROW ) );
    }
    else
    {
        m_xUICfgManager->insertSettings( m_sBarName, uno::Reference< container::XIndexAccess > (m_xBarSettings, uno::UNO_QUERY_THROW ) );
    }
    if( !m_bTemporary )
    {
        m_xUICfgPers->store();
    }
}

void
ScVbaCommandBarControl::createNewToolBarControl()
{
    uno::Sequence< beans::PropertyValue > aPropertys(4);
    aPropertys[0].Name = rtl::OUString::createFromAscii("CommandURL");
    aPropertys[0].Value <<= m_sCommand;
    aPropertys[1].Name = rtl::OUString::createFromAscii("Label");
    aPropertys[1].Value <<= m_sName;
    aPropertys[2].Name = rtl::OUString::createFromAscii("Type");
    aPropertys[2].Value <<= m_nType;
    aPropertys[3].Name = rtl::OUString::createFromAscii("IsVisible");
    aPropertys[3].Value <<= sal_True;

    m_xBarSettings->insertByIndex( m_nPosition, uno::makeAny( aPropertys ) );
    uno::Reference< beans::XPropertySet > xPropertySet( m_xBarSettings, uno::UNO_QUERY_THROW );
    rtl::OUString sUIName;
    xPropertySet->getPropertyValue( rtl::OUString::createFromAscii("UIName") ) >>= sUIName;

    m_xCurrentSettings.set( m_xBarSettings, uno::UNO_QUERY_THROW );
    if( m_xUICfgManager->hasSettings( m_sBarName ) )
    {
        m_xUICfgManager->replaceSettings( m_sBarName, uno::Reference< container::XIndexAccess > (m_xBarSettings, uno::UNO_QUERY_THROW ) );
    }
    else
    {
        m_xUICfgManager->insertSettings( m_sBarName, uno::Reference< container::XIndexAccess > (m_xBarSettings, uno::UNO_QUERY_THROW ) );
    }
    if( !m_bTemporary )
    {
        m_xUICfgPers->store();
    }
}

::rtl::OUString SAL_CALL
ScVbaCommandBarControl::getCaption() throw ( uno::RuntimeException )
{
    // "Label" always empty
    rtl::OUString sCaption;
    beans::PropertyValues aPropertyValues;
    if( m_xCurrentSettings.is() )
    {
        m_xCurrentSettings->getByIndex( m_nPosition ) >>= aPropertyValues;
        lcl_getPropertyValue( aPropertyValues, rtl::OUString::createFromAscii("Label") ) >>= sCaption;
    }
    else if( m_xBarSettings.is() )
    {
        m_xBarSettings->getByIndex( m_nPosition ) >>= aPropertyValues;
        lcl_getPropertyValue( aPropertyValues, rtl::OUString::createFromAscii("Label") ) >>= sCaption;
    }
    return sCaption;
}
void SAL_CALL
ScVbaCommandBarControl::setCaption( const ::rtl::OUString& _caption ) throw (uno::RuntimeException)
{
    if( m_xCurrentSettings.is() )
    {
        beans::PropertyValues aPropertyValues;
        m_xCurrentSettings->getByIndex( m_nPosition ) >>= aPropertyValues;
        beans::PropertyValues aNewPropertyValues;
        aNewPropertyValues = lcl_repalcePropertyValue( aPropertyValues, rtl::OUString::createFromAscii("Label"), uno::makeAny( _caption ) );
        m_xCurrentSettings->replaceByIndex( m_nPosition, uno::makeAny( aNewPropertyValues ) );
        if( m_xUICfgManager->hasSettings( m_sBarName ) )
        {
            m_xUICfgManager->replaceSettings( m_sBarName, uno::Reference< container::XIndexAccess > (m_xBarSettings, uno::UNO_QUERY_THROW ) );
        }
        else
        {
            m_xUICfgManager->insertSettings( m_sBarName, uno::Reference< container::XIndexAccess > (m_xBarSettings, uno::UNO_QUERY_THROW ) );
        }
        // make it permanent
        if( !m_bTemporary )
        {
            m_xUICfgPers->store();
        }
    }
}
::rtl::OUString SAL_CALL
ScVbaCommandBarControl::getOnAction() throw (uno::RuntimeException)
{
    if( m_xCurrentSettings.is() )
    {
        beans::PropertyValues aPropertyValues;
        m_xCurrentSettings->getByIndex( m_nPosition ) >>= aPropertyValues;
        rtl::OUString sCommandURL;
        lcl_getPropertyValue( aPropertyValues, rtl::OUString::createFromAscii( "CommandURL" ) ) >>= sCommandURL;
        return sCommandURL;
    }
    return ::rtl::OUString();
}
void SAL_CALL
ScVbaCommandBarControl::setOnAction( const ::rtl::OUString& _onaction ) throw (uno::RuntimeException)
{
    if( m_xCurrentSettings.is() )
    {
        beans::PropertyValues aPropertyValues;
        m_xCurrentSettings->getByIndex( m_nPosition ) >>= aPropertyValues;
        beans::PropertyValues aNewPropertyValues;
        aNewPropertyValues = lcl_repalcePropertyValue( aPropertyValues, rtl::OUString::createFromAscii("CommandURL"), uno::makeAny( _onaction ) );
        m_xCurrentSettings->replaceByIndex( m_nPosition, uno::makeAny( aNewPropertyValues ) );
        if( m_xUICfgManager->hasSettings( m_sBarName ) )
        {
            m_xUICfgManager->replaceSettings( m_sBarName, uno::Reference< container::XIndexAccess > (m_xBarSettings, uno::UNO_QUERY_THROW ) );
        }
        else
        {
            m_xUICfgManager->insertSettings( m_sBarName, uno::Reference< container::XIndexAccess > (m_xBarSettings, uno::UNO_QUERY_THROW ) );
        }
        // make it permanent
        if( !m_bTemporary )
        {
            m_xUICfgPers->store();
        }
    }
}
::sal_Bool SAL_CALL
ScVbaCommandBarControl::getVisible() throw (uno::RuntimeException)
{
    // not possible in UNO?
    return sal_True;
}
void SAL_CALL
ScVbaCommandBarControl::setVisible( ::sal_Bool /*_visible*/ ) throw (uno::RuntimeException)
{
    // "IsVisilbe"
}
void SAL_CALL
ScVbaCommandBarControl::Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    if( m_xCurrentSettings.is() )
    {
        m_xCurrentSettings->removeByIndex( m_nPosition );

        if( m_xUICfgManager->hasSettings( m_sBarName ) )
        {
            m_xUICfgManager->replaceSettings( m_sBarName, uno::Reference< container::XIndexAccess > (m_xBarSettings, uno::UNO_QUERY_THROW ) );
        }
        else
        {
            m_xUICfgManager->insertSettings( m_sBarName, uno::Reference< container::XIndexAccess > (m_xBarSettings, uno::UNO_QUERY_THROW ) );
        }
        // make it permanent
        if( !m_bTemporary )
        {
            m_xUICfgPers->store();
        }
    }
}
uno::Any SAL_CALL
ScVbaCommandBarControl::Controls( const uno::Any& aIndex ) throw (script::BasicErrorException, uno::RuntimeException)
{
    sal_Int32 nIndex;
    uno::Reference< XCommandBarControls > xCommandBarControls( new ScVbaCommandBarControls( this, mxContext, uno::Reference< container::XIndexAccess >() ) );
    if( aIndex.hasValue() )
    {
        if( aIndex >>= nIndex )
        {
            uno::Reference< XCommandBarControl > xCommandBarControl( xCommandBarControls->Item( aIndex, uno::Any() ), uno::UNO_QUERY_THROW );
            return uno::makeAny( xCommandBarControl );
        }
        else
            throw uno::RuntimeException( rtl::OUString::createFromAscii( "invalid index" ), uno::Reference< uno::XInterface >() );
    }
    return uno::makeAny( xCommandBarControls );
}
rtl::OUString&
ScVbaCommandBarControl::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaCommandBarControl") );
    return sImplName;
}
uno::Sequence<rtl::OUString>
ScVbaCommandBarControl::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.CommandBarControl" ) );
    }
    return aServiceNames;
}
