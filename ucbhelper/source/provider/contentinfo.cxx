/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contentinfo.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 17:21:59 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"
/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROPERTYSETREGISTRY_HPP_
#include <com/sun/star/ucb/XPropertySetRegistry.hpp>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _UCBHELPER_CONTENTHELPER_HXX
#include <ucbhelper/contenthelper.hxx>
#endif
#ifndef _UCBHELPER_CONTENTINFO_HXX
#include <ucbhelper/contentinfo.hxx>
#endif

using namespace rtl;
using namespace vos;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::ucb;
using namespace ucb;

//=========================================================================
//=========================================================================
//
// PropertySetInfo Implementation.
//
//=========================================================================
//=========================================================================

PropertySetInfo::PropertySetInfo(
                        const Reference< XMultiServiceFactory >& rxSMgr,
                        const Reference< XCommandEnvironment >& rxEnv,
                        ContentImplHelper* pContent )
: m_xSMgr( rxSMgr ),
  m_xEnv( rxEnv ),
  m_pProps( 0 ),
  m_pContent( pContent )
{
}

//=========================================================================
// virtual
PropertySetInfo::~PropertySetInfo()
{
    delete m_pProps;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_2( PropertySetInfo,
                   XTypeProvider,
                   XPropertySetInfo );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( PropertySetInfo,
                          XTypeProvider,
                          XPropertySetInfo );

//=========================================================================
//
// XPropertySetInfo methods.
//
//=========================================================================

// virtual
Sequence< Property > SAL_CALL PropertySetInfo::getProperties()
    throw( RuntimeException )
{
    if ( !m_pProps )
    {
        vos::OGuard aGuard( m_aMutex );
        if ( !m_pProps )
        {
            //////////////////////////////////////////////////////////////
            // Get info for core ( native) properties.
            //////////////////////////////////////////////////////////////

            try
            {
                Sequence< Property > aProps
                    = m_pContent->getProperties( m_xEnv );
                m_pProps = new Sequence< Property >( aProps );
            }
            catch ( RuntimeException const & )
            {
                throw;
            }
            catch ( Exception const & )
            {
                m_pProps = new Sequence< Property >( 0 );
            }

            //////////////////////////////////////////////////////////////
            // Get info for additional properties.
            //////////////////////////////////////////////////////////////

            Reference< XPersistentPropertySet > xSet (
                        m_pContent->getAdditionalPropertySet( sal_False ) );

            if ( xSet.is() )
            {
                // Get property set info.
                Reference< XPropertySetInfo > xInfo(
                                        xSet->getPropertySetInfo() );
                if ( xInfo.is() )
                {
                    const Sequence< Property >& rAddProps
                        = xInfo->getProperties();
                    sal_Int32 nAddProps = rAddProps.getLength();
                    if ( nAddProps > 0 )
                    {
                        sal_Int32 nPos = m_pProps->getLength();
                        m_pProps->realloc( nPos + nAddProps );

                        Property* pProps = m_pProps->getArray();
                        const Property* pAddProps = rAddProps.getConstArray();

                        for ( sal_Int32 n = 0; n < nAddProps; ++n, ++nPos )
                            pProps[ nPos ] = pAddProps[ n ];
                    }
                }
            }
        }
    }
    return *m_pProps;
}

//=========================================================================
// virtual
Property SAL_CALL PropertySetInfo::getPropertyByName( const OUString& aName )
    throw( UnknownPropertyException, RuntimeException )
{
    Property aProp;
    if ( queryProperty( aName, aProp ) )
        return aProp;

    throw UnknownPropertyException();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL PropertySetInfo::hasPropertyByName( const OUString& Name )
    throw( RuntimeException )
{
    Property aProp;
    return queryProperty( Name, aProp );
}

//=========================================================================
//
// Non-Interface methods.
//
//=========================================================================

void PropertySetInfo::reset()
{
    vos::OGuard aGuard( m_aMutex );
    delete m_pProps;
    m_pProps = 0;
}

//=========================================================================
sal_Bool PropertySetInfo::queryProperty(
                                const OUString& rName, Property& rProp )
{
    vos::OGuard aGuard( m_aMutex );

    getProperties();

    const Property* pProps = m_pProps->getConstArray();
    sal_Int32 nCount = m_pProps->getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const Property& rCurrProp = pProps[ n ];
        if ( rCurrProp.Name == rName )
        {
            rProp = rCurrProp;
            return sal_True;
        }
    }

    return sal_False;
}

//=========================================================================
//=========================================================================
//
// CommandProcessorInfo Implementation.
//
//=========================================================================
//=========================================================================

CommandProcessorInfo::CommandProcessorInfo(
                        const Reference< XMultiServiceFactory >& rxSMgr,
                        const Reference< XCommandEnvironment >& rxEnv,
                        ContentImplHelper* pContent )
: m_xSMgr( rxSMgr ),
  m_xEnv( rxEnv ),
  m_pCommands( 0 ),
  m_pContent( pContent )
{
}

//=========================================================================
// virtual
CommandProcessorInfo::~CommandProcessorInfo()
{
    delete m_pCommands;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_2( CommandProcessorInfo,
                   XTypeProvider,
                   XCommandInfo );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( CommandProcessorInfo,
                         XTypeProvider,
                         XCommandInfo );

//=========================================================================
//
// XCommandInfo methods.
//
//=========================================================================

// virtual
Sequence< CommandInfo > SAL_CALL CommandProcessorInfo::getCommands()
    throw( RuntimeException )
{
    if ( !m_pCommands )
    {
        vos::OGuard aGuard( m_aMutex );
        if ( !m_pCommands )
        {
            //////////////////////////////////////////////////////////////
            // Get info for commands.
            //////////////////////////////////////////////////////////////

            try
            {
                Sequence< CommandInfo > aCmds
                    = m_pContent->getCommands( m_xEnv );
                m_pCommands = new Sequence< CommandInfo >( aCmds );
            }
            catch ( RuntimeException const & )
            {
                throw;
            }
            catch ( Exception const & )
            {
                m_pCommands = new Sequence< CommandInfo >( 0 );
            }
        }
    }
    return *m_pCommands;
}

//=========================================================================
// virtual
CommandInfo SAL_CALL CommandProcessorInfo::getCommandInfoByName(
                                                        const OUString& Name )
    throw( UnsupportedCommandException, RuntimeException )
{
    CommandInfo aInfo;
    if ( queryCommand( Name, aInfo ) )
        return aInfo;

    throw UnsupportedCommandException();
}

//=========================================================================
// virtual
CommandInfo SAL_CALL CommandProcessorInfo::getCommandInfoByHandle(
                                                        sal_Int32 Handle )
    throw( UnsupportedCommandException, RuntimeException )
{
    CommandInfo aInfo;
    if ( queryCommand( Handle, aInfo ) )
        return aInfo;

    throw UnsupportedCommandException();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL CommandProcessorInfo::hasCommandByName( const OUString& Name )
    throw( RuntimeException )
{
    CommandInfo aInfo;
    return queryCommand( Name, aInfo );
}

//=========================================================================
// virtual
sal_Bool SAL_CALL CommandProcessorInfo::hasCommandByHandle( sal_Int32 Handle )
    throw( RuntimeException )
{
    CommandInfo aInfo;
    return queryCommand( Handle, aInfo );
}

//=========================================================================
//
// Non-Interface methods.
//
//=========================================================================

void CommandProcessorInfo::reset()
{
    vos::OGuard aGuard( m_aMutex );
    delete m_pCommands;
    m_pCommands = 0;
}


//=========================================================================
sal_Bool CommandProcessorInfo::queryCommand( const OUString& rName,
                                             CommandInfo& rCommand )
{
    vos::OGuard aGuard( m_aMutex );

    getCommands();

    const CommandInfo* pCommands = m_pCommands->getConstArray();
    sal_Int32 nCount = m_pCommands->getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const CommandInfo& rCurrCommand = pCommands[ n ];
        if ( rCurrCommand.Name == rName )
        {
            rCommand = rCurrCommand;
            return sal_True;
        }
    }

    return sal_False;
}

//=========================================================================
sal_Bool CommandProcessorInfo::queryCommand( sal_Int32 nHandle,
                                             CommandInfo& rCommand )
{
    vos::OGuard aGuard( m_aMutex );

    getCommands();

    const CommandInfo* pCommands = m_pCommands->getConstArray();
    sal_Int32 nCount = m_pCommands->getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const CommandInfo& rCurrCommand = pCommands[ n ];
        if ( rCurrCommand.Handle == nHandle )
        {
            rCommand = rCurrCommand;
            return sal_True;
        }
    }

    return sal_False;
}

