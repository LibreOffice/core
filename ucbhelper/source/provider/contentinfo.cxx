/*************************************************************************
 *
 *  $RCSfile: contentinfo.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
                        ContentImplHelper* pContent )
: m_xSMgr( rxSMgr ),
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
    vos::OGuard aGuard( m_aMutex );

    if ( !m_pProps )
    {
        m_pProps = new Sequence< Property >( 128 );
        Property* pProps = m_pProps->getArray();
        sal_Int32 nPos  = 0;
        sal_Int32 nSize = m_pProps->getLength();

        //////////////////////////////////////////////////////////////////
        // Get info for core ( native) properties.
        //////////////////////////////////////////////////////////////////

        const ::ucb::PropertyInfoTableEntry& rCoreProps
                                    = m_pContent->getPropertyInfoTable();
        const ::ucb::PropertyInfoTableEntry* pCurr = &rCoreProps;
        while ( pCurr->pName )
        {
            if ( nSize <= nPos )
            {
                m_pProps->realloc( 128 );
                nSize += 128;
            }

            Property& rProp = pProps[ nPos ];

            VOS_ENSURE( pCurr->pType,
                        "PropertySetInfo::getProperties - No type!" );

            rProp.Name       = OUString::createFromAscii( pCurr->pName );
            rProp.Handle     = pCurr->nHandle;
            rProp.Type       = *pCurr->pType;
            rProp.Attributes = pCurr->nAttributes;

            nPos++;
            pCurr++;
        }

        if ( nPos > 0 )
        {
            m_pProps->realloc( nPos );
            nSize = m_pProps->getLength();
        }

        //////////////////////////////////////////////////////////////////
        // Get info for additional properties.
        //////////////////////////////////////////////////////////////////

        Reference< XPersistentPropertySet > xSet (
                        m_pContent->getAdditionalPropertySet( sal_False ) );

        if ( xSet.is() )
        {
            // Get property set info.
            Reference< XPropertySetInfo > xInfo( xSet->getPropertySetInfo() );
            if ( xInfo.is() )
            {
                const Sequence< Property >& rAddProps = xInfo->getProperties();
                sal_Int32 nAddProps = rAddProps.getLength();
                if ( nAddProps > 0 )
                {
                    m_pProps->realloc( nSize + nAddProps );
                    pProps = m_pProps->getArray();

                    const Property* pAddProps = rAddProps.getConstArray();
                    for ( sal_Int32 n = 0; n < nAddProps; ++n, ++nPos )
                        pProps[ nPos ] = pAddProps[ n ];
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
                        ContentImplHelper* pContent )
: m_xSMgr( rxSMgr ),
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
    vos::OGuard aGuard( m_aMutex );

    if ( !m_pCommands )
    {
        m_pCommands = new Sequence< CommandInfo >( 128 );
        CommandInfo* pCommands = m_pCommands->getArray();
        sal_Int32 nPos  = 0;
        sal_Int32 nSize = m_pCommands->getLength();

        //////////////////////////////////////////////////////////////////
        // Get info for commands.
        //////////////////////////////////////////////////////////////////

        const ::ucb::CommandInfoTableEntry& rCommands
                                    = m_pContent->getCommandInfoTable();
        const ::ucb::CommandInfoTableEntry* pCurr = &rCommands;
        while ( pCurr->pName )
        {
            if ( nSize <= nPos )
            {
                m_pCommands->realloc( 128 );
                nSize += 128;
            }

            CommandInfo& rCommand = pCommands[ nPos ];

            rCommand.Name   = OUString::createFromAscii( pCurr->pName );
            rCommand.Handle = pCurr->nHandle;
            if ( pCurr->pArgType )
                rCommand.ArgType = *pCurr->pArgType;

            nPos++;
            pCurr++;
        }

        if ( nPos > 0 )
            m_pCommands->realloc( nPos );
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

