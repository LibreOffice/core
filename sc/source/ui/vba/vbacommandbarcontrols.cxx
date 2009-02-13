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
#include "vbacommandbarcontrols.hxx"
#include "vbacommandbarcontrol.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

typedef ::cppu::WeakImplHelper1< container::XEnumeration > CommandBarControlEnumeration_BASE;
class CommandBarControlEnumeration : public CommandBarControlEnumeration_BASE
{
    //uno::Reference< uno::XComponentContext > m_xContext;
    ScVbaCommandBarControls* m_pCommandBarControls;
    sal_Int32 m_nCurrentPosition;
public:
    CommandBarControlEnumeration( ScVbaCommandBarControls* pCommandBarControls ) : m_pCommandBarControls( pCommandBarControls ), m_nCurrentPosition( 0 ) {}
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
            rtl::OUString sName = m_pCommandBarControls->GetControlNameByIndex( m_nCurrentPosition );
            m_nCurrentPosition = m_nCurrentPosition + 1;
            if( sName.getLength() > 0 )
                return m_pCommandBarControls->createCollectionObject( uno::makeAny( sName ) );
            else
                return nextElement();
        }
        else
            throw container::NoSuchElementException();
        return uno::Any();
    }
};

ScVbaCommandBarControls::ScVbaCommandBarControls( const uno::Reference< XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, const uno::Reference< container::XIndexAccess> xIndexAccess ) throw (uno::RuntimeException) : CommandBarControls_BASE( xParent, xContext, xIndexAccess )
{
    m_bIsMenu = sal_False;
    m_bHasElements = sal_False;
    m_xParentHardRef.set( xParent, uno::UNO_QUERY_THROW );
    m_pCommandBar = dynamic_cast< ScVbaCommandBar* >( m_xParentHardRef.get() );
    m_pCommandBarControl = dynamic_cast< ScVbaCommandBarControl* >( m_xParentHardRef.get() );
    if( m_pCommandBar )
    {
        m_xUICfgManager.set( m_pCommandBar->GetUICfgManager(), uno::UNO_QUERY_THROW );
        m_xUICfgPers.set( m_pCommandBar->GetUICfgPers(), uno::UNO_QUERY_THROW );
        m_xBarSettings.set( m_pCommandBar->GetBarSettings(), uno::UNO_QUERY_THROW );
        m_bIsMenu = m_pCommandBar->IsMenu();
        if( m_xBarSettings->hasElements() )
        {
            m_bHasElements = sal_True;
        }
    }
    else if( m_pCommandBarControl )
    {
    }
    else
    {
        throw uno::RuntimeException( rtl::OUString::createFromAscii("Parent needs to be a ScVbaCommandBar or a ScVbaCommandBarControl"), uno::Reference< uno::XInterface >() );
    }
}
rtl::OUString
ScVbaCommandBarControls::GetControlNameByIndex( const sal_Int32 nIndex ) throw ( uno::RuntimeException )
{
    sal_Int32 nCount = 0;
    if( m_bHasElements )
    {
        sal_Int32 nBarSettingsCount = m_xBarSettings->getCount();
        for( sal_Int32 i = 0; i < nBarSettingsCount; i++ )
        {
            beans::PropertyValues aMenuValues;
            m_xBarSettings->getByIndex( i ) >>= aMenuValues;
            for( sal_Int32 j = 0; j < aMenuValues.getLength(); j++ )
            {
                if( aMenuValues[j].Name.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "CommandURL" ) ) )
                {
                    nCount++;
                    if( nIndex == nCount )
                    {
                        rtl::OUString sCommandURL;
                        aMenuValues[j].Value >>= sCommandURL;
                        sal_Int32 nLastIndex = sCommandURL.lastIndexOf( rtl::OUString::createFromAscii(":") );
                        if( ( nLastIndex != -1 ) && ( ( nLastIndex +1 ) < sCommandURL.getLength() ) )
                        {
                            sCommandURL = sCommandURL.copy( nLastIndex + 1 );
                            return sCommandURL;
                        }
                    }
                }
            }
        }
    }
    else
        throw  uno::RuntimeException( rtl::OUString::createFromAscii( "Out of bound" ), uno::Reference< uno::XInterface >() );
    return rtl::OUString();
}

// Attributes
sal_Int32 SAL_CALL
ScVbaCommandBarControls::getCount() throw (uno::RuntimeException)
{
    sal_Int32 nCount = 0;
    if( m_bHasElements )
    {
        sal_Int32 nBarSettingsCount = m_xBarSettings->getCount();
        for( sal_Int32 i = 0; i < nBarSettingsCount; i++ )
        {
            beans::PropertyValues aMenuValues;
            m_xBarSettings->getByIndex( i ) >>= aMenuValues;
            for( sal_Int32 j = 0; j < aMenuValues.getLength(); j++ )
            {
                if( aMenuValues[j].Name.equalsIgnoreAsciiCase( rtl::OUString::createFromAscii( "CommandURL" ) ) )
                {
                    nCount++;
                }
            }
        }
    }
    return nCount;
}
// XEnumerationAccess
uno::Type SAL_CALL
ScVbaCommandBarControls::getElementType() throw ( uno::RuntimeException )
{
    return XCommandBarControls::static_type( 0 );
}
uno::Reference< container::XEnumeration >
ScVbaCommandBarControls::createEnumeration() throw ( uno::RuntimeException )
{
    return uno::Reference< container::XEnumeration >( new CommandBarControlEnumeration( this ) );
}
uno::Any
ScVbaCommandBarControls::createCollectionObject( const uno::Any& aSource )
{
    // only surport the aSource as a name string, because this class is a API wrapper
    rtl::OUString sName;
    if( aSource >>= sName )
        return uno::makeAny( uno::Reference< XCommandBarControl > ( new ScVbaCommandBarControl( this, mxContext, sName ) ) );
    return uno::Any();
}

// Methods
uno::Any SAL_CALL
ScVbaCommandBarControls::Item( const uno::Any& aIndex, const uno::Any& /*aIndex*/ ) throw (uno::RuntimeException)
{
    if( aIndex.getValueTypeClass() == uno::TypeClass_STRING )
    {
        return createCollectionObject( aIndex );
    }
    sal_Int32 nIndex = 0;
    if( aIndex >>= nIndex )
    {
        return createCollectionObject( uno::makeAny( GetControlNameByIndex( nIndex ) ) );
    }

    return uno::Any();
}
uno::Reference< XCommandBarControl > SAL_CALL
ScVbaCommandBarControls::Add( const uno::Any& Type, const uno::Any& Id, const uno::Any& /*Parameter*/, const uno::Any& Before, const uno::Any& Temporary ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // Parameter is not supported
    // the following name needs to be individually created;
    rtl::OUString sCaption( rtl::OUString::createFromAscii("custom Control") );
    rtl::OUString sCommand( rtl::OUString::createFromAscii("macro:///Standard.Module1.Test()") );
    sal_Int32 nType =0;
    sal_Int32 nPosition = 0;
    sal_Int32 nId;
    sal_Bool bTemporary = sal_True;

    if( Type.hasValue() )
        if( Type >>= nType )
        {
            // evalute the type of the new control
        }
    if( Id.hasValue() )
        if( Id >>= nId )
        {
            // evalute the action of the new control
        }
    if( Before.hasValue() )
        if( Before >>= nPosition )
        {
            // evalute the position of the new Control
        }
    if( Temporary.hasValue() )
        if( Temporary >>= bTemporary )
        {
            // evalute the temporary of the new Control
        }
    return uno::Reference< XCommandBarControl > ( new ScVbaCommandBarControl( this, mxContext, sCaption, sCommand, nPosition, bTemporary ) );
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

