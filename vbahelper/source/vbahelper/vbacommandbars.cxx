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
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>

#include "vbacommandbars.hxx"
#include "vbacommandbar.hxx"

using namespace com::sun::star;
using namespace ooo::vba;


typedef ::cppu::WeakImplHelper1< container::XEnumeration > CommandBarEnumeration_BASE;

class CommandBarEnumeration : public CommandBarEnumeration_BASE
{
    uno::Reference< XHelperInterface > m_xParent;
    uno::Reference< uno::XComponentContext > m_xContext;
    VbaCommandBarHelperRef pCBarHelper;
    uno::Sequence< rtl::OUString > m_sNames;
    sal_Int32 m_nCurrentPosition;
public:
    CommandBarEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, VbaCommandBarHelperRef pHelper) throw ( uno::RuntimeException ) : m_xParent( xParent ), m_xContext( xContext ), pCBarHelper( pHelper ) , m_nCurrentPosition( 0 )
    {
        uno::Reference< container::XNameAccess > xNameAccess = pCBarHelper->getPersistentWindowState();
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
            if( sResourceUrl.indexOf( rtl::OUString::createFromAscii("private:resource/toolbar/") ) != -1 )
            {
                uno::Reference< container::XIndexAccess > xCBarSetting = pCBarHelper->getSettings( sResourceUrl );
                uno::Reference< XCommandBar > xCommandBar( new ScVbaCommandBar( m_xParent, m_xContext, pCBarHelper, xCBarSetting, sResourceUrl, sal_False, sal_False ) );
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
    pCBarHelper.reset( new VbaCommandBarHelper( mxContext, xModel ) );
    m_xNameAccess = pCBarHelper->getPersistentWindowState();
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
    return uno::Reference< container::XEnumeration >( new CommandBarEnumeration( this, mxContext, pCBarHelper ) );
}

uno::Any
ScVbaCommandBars::createCollectionObject( const uno::Any& aSource )
{
    // aSource should be a name at this time, because of the class is API wrapper.
    rtl::OUString sResourceUrl;
    uno::Reference< container::XIndexAccess > xBarSettings;
    rtl::OUString sBarName;
    sal_Bool bMenu = sal_False;
    if( aSource >>= sBarName )
    {
        if( sBarName.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii("Worksheet Menu Bar") )
            || sBarName.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii("Menu Bar") ) )
        {
            // menu bar
            sResourceUrl = rtl::OUString::createFromAscii( ITEM_MENUBAR_URL );
            bMenu = sal_True;
        }
        else
        {
            sResourceUrl = pCBarHelper->findToolbarByName( m_xNameAccess, sBarName );
            bMenu = sal_False;
        }
    }

    if( sResourceUrl.getLength() )
        xBarSettings = pCBarHelper->getSettings( sResourceUrl );
    else
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Toolbar do not exist") ), uno::Reference< uno::XInterface >() );

    return uno::Any( uno::Reference< XCommandBar >( new ScVbaCommandBar( this, mxContext, pCBarHelper, xBarSettings, sResourceUrl, bMenu, sal_False ) ) );
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
        sResourceUrl = pCBarHelper->findToolbarByName( m_xNameAccess, sName );
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
    uno::Reference< container::XIndexAccess > xBarSettings( pCBarHelper->getSettings( sResourceUrl ), uno::UNO_QUERY_THROW );
    uno::Reference< XCommandBar > xCBar( new ScVbaCommandBar( this, mxContext, pCBarHelper, xBarSettings, sResourceUrl, sal_False, bTemporary ) );
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
        if(allNames[i].indexOf( rtl::OUString::createFromAscii("private:resource/toolbar/") ) != -1 )
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
        if( pCBarHelper->getModuleId().equalsAscii( "com.sun.star.sheet.SpreadsheetDocument" ) )
            aSource <<= rtl::OUString::createFromAscii( "Worksheet Menu Bar" );
        else if( pCBarHelper->getModuleId().equalsAscii("com.sun.star.text.TextDocument") )
            aSource <<= rtl::OUString::createFromAscii( "Menu Bar" );
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

