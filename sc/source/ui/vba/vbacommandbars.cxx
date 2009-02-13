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

#include "vbacommandbars.hxx"
#include "vbacommandbar.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

typedef ::cppu::WeakImplHelper1< container::XEnumeration > CommandBarEnumeration_BASE;

static rtl::OUString sSpreadsheetDocumentUrl( rtl::OUString::createFromAscii( "com.sun.star.sheet.SpreadsheetDocument" ) );
static rtl::OUString sTextDocumentUrl( rtl::OUString::createFromAscii( "com.sun.star.text.TextDocument" ) );
static rtl::OUString sWindowStateConfUrl( rtl::OUString::createFromAscii( "com.sun.star.ui.WindowStateConfiguration" ) );

class CommandBarEnumeration : public CommandBarEnumeration_BASE
{
    uno::Reference< uno::XComponentContext > m_xContext;
    uno::Reference< XCommandBars > m_xCommandBars;
    uno::Sequence< rtl::OUString > m_sNames;
    sal_Int32 m_nCurrentPosition;
public:
    CommandBarEnumeration( const uno::Reference< uno::XComponentContext > xContext, const uno::Reference< XCommandBars > xCommandBars, const uno::Sequence< rtl::OUString > sNames ) : m_xContext( xContext ), m_xCommandBars( xCommandBars ), m_sNames( sNames ), m_nCurrentPosition( 0 )
    {
    }
    virtual sal_Bool SAL_CALL hasMoreElements() throw ( uno::RuntimeException )
    {
        if( m_nCurrentPosition < m_sNames.getLength() )
            return sal_True;
        return sal_False;
    }
    virtual uno::Any SAL_CALL nextElement() throw ( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
    {
        if( hasMoreElements() )
        {
            rtl::OUString sName( m_sNames[ m_nCurrentPosition ] );
            m_nCurrentPosition = m_nCurrentPosition + 1;
            if( sName.indexOf( rtl::OUString::createFromAscii("private:resource/toolbar/") ) != -1 )
            {
                sal_Int32 nLastIndex = sName.lastIndexOf( rtl::OUString::createFromAscii( "/" ) );
                if( ( nLastIndex != -1 ) && ( ( nLastIndex + 1 ) < sName.getLength() ) )
                {
                   sName = sName.copy( nLastIndex + 1);
                   if( sName.getLength() > 0 )
                   {
                        uno::Reference< XHelperInterface > xHelperInterface( m_xCommandBars, uno::UNO_QUERY_THROW );
                        uno::Reference< XCommandBar > xCommandBar( new ScVbaCommandBar( xHelperInterface, m_xContext, sName, sal_True, sal_False ) );
                        if( xCommandBar.is() )
                            return uno::makeAny( xCommandBar );
                        else
                            return nextElement();
                    }
                    else
                        return nextElement();
                    }
             }
             else
                return nextElement();
        }
        else
            throw container::NoSuchElementException();
        return uno::Any();
    }
};


ScVbaCommandBars::ScVbaCommandBars( const uno::Reference< XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, const uno::Reference< container::XIndexAccess > xIndexAccess ) : CommandBars_BASE( xParent, xContext, xIndexAccess )
{
    retrieveObjects();
}
void
ScVbaCommandBars::retrieveObjects() throw ( uno::RuntimeException )
{
    uno::Reference< lang::XServiceInfo > xServiceInfo( getCurrentDocument(), uno::UNO_QUERY_THROW );
    if( xServiceInfo->supportsService( sSpreadsheetDocumentUrl ) )
    {
        m_sModuleName = sSpreadsheetDocumentUrl;
    }
    else if( xServiceInfo->supportsService( sTextDocumentUrl ) )
    {
        m_sModuleName = sTextDocumentUrl;
    }
    else
        throw uno::RuntimeException( rtl::OUString::createFromAscii( "Unsupported Document" ), uno::Reference< uno::XInterface >() );

    uno::Reference < lang::XMultiServiceFactory > xMSF( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference < container::XNameAccess > xNameAccess( xMSF->createInstance( sWindowStateConfUrl ), uno::UNO_QUERY_THROW );
    m_xNameAccess.set( xNameAccess->getByName( m_sModuleName ), uno::UNO_QUERY_THROW );
}

// XEnumerationAccess
uno::Type SAL_CALL
ScVbaCommandBars::getElementType() throw ( uno::RuntimeException )
{
    return XCommandBars::static_type( 0 );
}
uno::Reference< container::XEnumeration >
ScVbaCommandBars::createEnumeration() throw ( uno::RuntimeException )
{
    return uno::Reference< container::XEnumeration >( new CommandBarEnumeration( mxContext, this, m_xNameAccess->getElementNames() ) );
}

uno::Any
ScVbaCommandBars::createCollectionObject( const uno::Any& aSource )
{
    // aSource should be a name at this time, because of the class is API wrapper.
    rtl::OUString sToolBarName;
    if( aSource >>= sToolBarName )
    {
        sToolBarName = sToolBarName.toAsciiLowerCase();
        if( sToolBarName.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii("Worksheet Menu Bar") ) )
        {
            return uno::makeAny( uno::Reference< XCommandBar > ( new ScVbaCommandBar( this, mxContext, 0 ) ) );
        }
        else if( sToolBarName.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii("Menu Bar") ) )
        {
            return uno::makeAny( uno::Reference< XCommandBar > ( new ScVbaCommandBar( this, mxContext, 1 ) ) );
        }
        else if( checkToolBarExist( sToolBarName ) )
        {
            return uno::makeAny( uno::Reference< XCommandBar > (new ScVbaCommandBar( this, mxContext, sToolBarName, sal_True, sal_False ) ) );
        }
    }
    return uno::Any();
}

// XCommandBars
uno::Reference< XCommandBar > SAL_CALL
ScVbaCommandBars::Add( const css::uno::Any& Name, const css::uno::Any& /*Position*/, const css::uno::Any& /*MenuBar*/, const css::uno::Any& Temporary ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    // Position - MsoBar MenuBar - sal_Bool
    // Currently only the Name is supported.
    rtl::OUString sName;
    if( !( Name >>= sName ) )
    {
        sName = rtl::OUString::createFromAscii("Custom1");
    }
    sal_Bool bTemporary = false;
    if( !( Temporary >>= bTemporary ) )
    {
        bTemporary = sal_True;
    }
    return uno::Reference< XCommandBar >( new ScVbaCommandBar( this, mxContext, sName.toAsciiLowerCase(), bTemporary, sal_True ) );
}
sal_Int32 SAL_CALL
ScVbaCommandBars::getCount() throw(css::uno::RuntimeException)
{
    // Filter out all toolbars from the window collection
    sal_Int32 nCount = 0;
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
    return uno::Any();
}

sal_Bool
ScVbaCommandBars::checkToolBarExist( rtl::OUString sToolBarName )
{
    CommandBarNameMap::const_iterator iter = mCommandBarNameMap.find( sToolBarName.toAsciiLowerCase() );
    if( iter != mCommandBarNameMap.end() )
    {
        return sal_True;
    }
    uno::Sequence< ::rtl::OUString > allNames = m_xNameAccess->getElementNames();
    for( sal_Int32 i = 0; i < allNames.getLength(); i++ )
    {
        if(allNames[i].indexOf( rtl::OUString::createFromAscii("private:resource/toolbar/") ) != -1 )
        {
            if( allNames[i].indexOf( sToolBarName ) != -1 )
            {
                return sal_True;
            }
        }
    }
    return sal_False;
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

