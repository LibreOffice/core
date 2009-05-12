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
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "vbacommandbar.hxx"
#include "vbacommandbarcontrols.hxx"
#include "vbahelper.hxx"


using namespace com::sun::star;
using namespace ooo::vba;

ScVbaCommandBar::ScVbaCommandBar( const uno::Reference< XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, sal_Int32 nModuleType ) throw (uno::RuntimeException) : CommandBar_BASE( xParent, xContext )
{
    // it's a menu bar
    // only supporting factory menus ( no custom menus )
    m_xParentHardRef.set( xParent, uno::UNO_QUERY_THROW );
    initCommandBar();
    switch( nModuleType )
    {
        case 0:
            m_sMenuModuleName = rtl::OUString::createFromAscii( "com.sun.star.sheet.SpreadsheetDocument" );
            break;
        case 1:
            m_sMenuModuleName = rtl::OUString::createFromAscii( "com.sun.star.text.TextDocument" );
            break;
        default:
            m_sMenuModuleName = rtl::OUString::createFromAscii( "com.sun.star.text.TextDocument" );
    }
    getMenuSettings();
    m_bIsMenu = sal_True;
    m_bCustom = sal_False;
}
ScVbaCommandBar::ScVbaCommandBar( const uno::Reference< XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, rtl::OUString sToolBarName, sal_Bool bTemporary, sal_Bool bCreate ) throw (uno::RuntimeException) :  CommandBar_BASE( xParent, xContext )
{
    // it's a tool bar
    m_xParentHardRef.set( xParent, uno::UNO_QUERY_THROW );
    initCommandBar();
    m_bTemporary = bTemporary;
    m_bCreate = bCreate;
    // get OOo ToolBarName
    CommandBarNameMap::const_iterator iter = mCommandBarNameMap.find( sToolBarName.toAsciiLowerCase() );
    if( iter != mCommandBarNameMap.end() )
    {
        m_sToolBarName = iter->second;
    }
    else
    {
        m_sToolBarName = sToolBarName;
    }
    m_sUIName = m_sToolBarName;
    m_bIsMenu = sal_False;
    getToolBarSettings( m_sToolBarName );
}
void
ScVbaCommandBar::initCommandBar() throw (uno::RuntimeException)
{
    m_pScVbaCommandBars = dynamic_cast< ScVbaCommandBars* >( m_xParentHardRef.get() );
    if ( !m_pScVbaCommandBars )
        throw uno::RuntimeException( rtl::OUString::createFromAscii( "Parent needs to be a ScVbaCommandBars"), uno::Reference< uno::XInterface >() );
    m_bIsMenu = sal_False;
    m_bCustom = sal_False;
    m_bTemporary = sal_True;
    m_sToolBarName = rtl::OUString::createFromAscii("");
    m_sUIName = rtl::OUString::createFromAscii("");
    m_sMenuModuleName = rtl::OUString::createFromAscii( "com.sun.star.sheet.SpreadsheetDocument" );
}
void
ScVbaCommandBar::getToolBarSettings( rtl::OUString sToolBarName ) throw( uno::RuntimeException )
{
    rtl::OUString sFactoryToolBar = rtl::OUString::createFromAscii("private:resource/toolbar/") + sToolBarName.toAsciiLowerCase();
    rtl::OUString sCustomToolBar = rtl::OUString::createFromAscii("private:resource/toolbar/custom_toolbar_") + sToolBarName;
    uno::Reference< lang::XMultiServiceFactory > xMSF( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference< ui::XModuleUIConfigurationManagerSupplier > xUICfgManagerSup( xMSF->createInstance(rtl::OUString::createFromAscii("com.sun.star.ui.ModuleUIConfigurationManagerSupplier") ), uno::UNO_QUERY_THROW );
    m_xUICfgManager.set( xUICfgManagerSup->getUIConfigurationManager( m_pScVbaCommandBars->GetModuleName() ), uno::UNO_QUERY_THROW );
    m_xUICfgPers.set( m_xUICfgManager, uno::UNO_QUERY_THROW );
    if( m_xUICfgManager->hasSettings( sFactoryToolBar ) )
    {
        // exsiting standard ToolBar
        m_xBarSettings.set( m_xUICfgManager->getSettings( sFactoryToolBar, sal_True ), uno::UNO_QUERY_THROW );
        m_sToolBarName = sFactoryToolBar;
    }
    else if( m_xUICfgManager->hasSettings( sCustomToolBar ) )
    {
        // exisiting custom ToolBar
        m_xBarSettings.set( m_xUICfgManager->getSettings( sCustomToolBar, sal_True ), uno::UNO_QUERY_THROW );
        m_sToolBarName = sCustomToolBar;
        m_bCustom = sal_True;
    }
    else if( m_bCreate )
    {
        // new custom ToolBar
        m_xBarSettings.set( m_xUICfgManager->createSettings(), uno::UNO_QUERY_THROW );
        m_sToolBarName = sCustomToolBar;
        m_bCustom = sal_True;
        addCustomBar();
    }
    else
        throw uno::RuntimeException( rtl::OUString::createFromAscii("ToolBar do not exist"), uno::Reference< uno::XInterface >() );
    if( m_pScVbaCommandBars->GetWindows()->hasByName( m_sToolBarName ) )
    {
        uno::Any aToolBar = m_pScVbaCommandBars->GetWindows()->getByName( m_sToolBarName );
        aToolBar >>= m_aToolBar;
    }
}
void
ScVbaCommandBar::addCustomBar()
{
    uno::Reference< beans::XPropertySet > xPropertySet( m_xBarSettings, uno::UNO_QUERY_THROW );
    xPropertySet->setPropertyValue(rtl::OUString::createFromAscii("UIName"), uno::makeAny( m_sUIName ));

    if( m_xUICfgManager->hasSettings(m_sToolBarName) )
    {
        m_xUICfgManager->replaceSettings( m_sToolBarName, uno::Reference< container::XIndexAccess > (m_xBarSettings, uno::UNO_QUERY_THROW ) );
    }
    else
    {
        m_xUICfgManager->insertSettings( m_sToolBarName,  uno::Reference< container::XIndexAccess > (m_xBarSettings, uno::UNO_QUERY_THROW ) );
    }
    if( !m_bTemporary )
    {
        m_xUICfgPers->store();
    }
}
void
ScVbaCommandBar::getMenuSettings()
{
    try
    {
        rtl::OUString sMenuBar = rtl::OUString::createFromAscii( "private:resource/menubar/menubar" );
        uno::Reference< lang::XMultiServiceFactory > xMSF( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
        uno::Reference< ui::XModuleUIConfigurationManagerSupplier > xUICfgManagerSup( xMSF->createInstance(rtl::OUString::createFromAscii("com.sun.star.ui.ModuleUIConfigurationManagerSupplier") ), uno::UNO_QUERY_THROW );
        m_xUICfgManager.set( xUICfgManagerSup->getUIConfigurationManager( m_sMenuModuleName ), uno::UNO_QUERY_THROW );
        m_xUICfgPers.set( m_xUICfgManager, uno::UNO_QUERY_THROW );
        m_xBarSettings.set( m_xUICfgManager->getSettings( sMenuBar, sal_True ), uno::UNO_QUERY_THROW );
    }
    catch ( uno::Exception e)
    {
        OSL_TRACE( "getMenuSetting got a error\n" );
    }
}
::rtl::OUString SAL_CALL
ScVbaCommandBar::getName() throw ( uno::RuntimeException )
{
    // This will get a "NULL length string" when Name is not set.
    uno::Reference< beans::XPropertySet > xPropertySet( m_xBarSettings, uno::UNO_QUERY_THROW );
    uno::Any aName = xPropertySet->getPropertyValue( rtl::OUString::createFromAscii("UIName") );
    rtl::OUString sName;
    aName >>= sName;
    if( sName.getLength() < 1 && !m_bIsMenu )
    {
        uno::Reference< container::XNameAccess > xNameAccess( m_pScVbaCommandBars->GetWindows(), uno::UNO_QUERY_THROW );
        if( xNameAccess->hasByName( m_sToolBarName ) )
        {
            beans::PropertyValues aToolBar;
            xNameAccess->getByName( m_sToolBarName ) >>= aToolBar;
            sal_Int32 nCount = aToolBar.getLength();
            beans::PropertyValue aPropertyValue;
            for( sal_Int32 i = 0; i < nCount; i++ )
            {
                aPropertyValue = aToolBar[i];
                if( aPropertyValue.Name.equals( rtl::OUString::createFromAscii("UIName") ) )
                {
                    aPropertyValue.Value >>= sName;
                    return sName;
                }
            }
        }
    }
    return sName;
}
void SAL_CALL
ScVbaCommandBar::setName( const ::rtl::OUString& _name ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xPropertySet( m_xBarSettings, uno::UNO_QUERY_THROW );
    xPropertySet->setPropertyValue( rtl::OUString::createFromAscii("UIName"), uno::makeAny( _name ) );
    uno::Reference< container::XIndexAccess > xIndexAccess( m_xBarSettings, uno::UNO_QUERY_THROW );

    if( m_xUICfgManager->hasSettings( m_sToolBarName ) )
    {
        m_xUICfgManager->replaceSettings( m_sToolBarName, xIndexAccess );
    }
    else
    {
        // toolbar not found
    }
    if( !m_bTemporary )
    {
        m_xUICfgPers->store();
    }
}
::sal_Bool SAL_CALL
ScVbaCommandBar::getVisible() throw (uno::RuntimeException)
{
    sal_Bool bVisible = sal_False;
    try
    {
        sal_Int32 i = 0;
        while( !m_aToolBar[i].Name.equals( rtl::OUString::createFromAscii( "Visible" ) ) )
        {
            i++;
        }
        m_aToolBar[i].Value >>= bVisible;
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
        uno::Reference< frame::XFrame > xFrame( getCurrentDocument()->getCurrentController()->getFrame(), uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xPropertySet( xFrame, uno::UNO_QUERY_THROW );
        uno::Reference< frame::XLayoutManager > xLayoutManager( xPropertySet->getPropertyValue( rtl::OUString::createFromAscii("LayoutManager") ), uno::UNO_QUERY_THROW );
        if( _visible )
        {
            xLayoutManager->createElement( m_sToolBarName );
            xLayoutManager->showElement( m_sToolBarName );
        }
        else
        {
            xLayoutManager->hideElement( m_sToolBarName );
            xLayoutManager->destroyElement( m_sToolBarName );
        }
    }
    catch( uno::Exception e )
    {
        OSL_TRACE( "SetVisible get an exception\n" );
    }
}
void SAL_CALL
ScVbaCommandBar::Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    if( m_bCustom )
    {
        if( m_xUICfgManager->hasSettings( m_sToolBarName ) )
        {
            m_xUICfgManager->removeSettings(m_sToolBarName);
            // make it permanent
            if( !m_bTemporary )
            {
                m_xUICfgPers->store();
            }
        }
        else
        {
            // toolbar not found
            // TODO throw Error
        }
        uno::Reference< container::XNameContainer > xNameContainer( m_pScVbaCommandBars->GetWindows(), uno::UNO_QUERY_THROW );
        if( xNameContainer->hasByName( m_sToolBarName ) )
        {
            xNameContainer->removeByName( m_sToolBarName );
        }
    }
}
uno::Any SAL_CALL
ScVbaCommandBar::Controls( const uno::Any& aIndex ) throw (script::BasicErrorException, uno::RuntimeException)
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
            throw uno::RuntimeException( rtl::OUString::createFromAscii(  "invalid index" ), uno::Reference< uno::XInterface >() );
    }
    return uno::makeAny( xCommandBarControls );
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
