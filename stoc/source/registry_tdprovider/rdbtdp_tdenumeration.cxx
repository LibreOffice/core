/*************************************************************************
 *
 *  $RCSfile: rdbtdp_tdenumeration.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 09:07:44 $
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

//=========================================================================
// Todo:
//
// - closeKey() calls (according to JSC not really needed because XRegistry
//   implementation closes key in it's dtor.
//
//=========================================================================

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _REGISTRY_REFLREAD_HXX_
#include <registry/reflread.hxx>
#endif

#ifndef _STOC_RDBTDP_BASE_HXX
#include "base.hxx"
#endif
#ifndef _STOC_RDBTDP_TDENUMERATION_HXX
#include "rdbtdp_tdenumeration.hxx"
#endif

using namespace com::sun::star;

namespace stoc_rdbtdp
{

//=========================================================================
//=========================================================================
//
// TypeDescriptionEnumerationImpl Implementation.
//
//=========================================================================
//=========================================================================

// static
rtl::Reference< TypeDescriptionEnumerationImpl >
TypeDescriptionEnumerationImpl::createInstance(
        const uno::Reference< uno::XComponentContext > & xContext,
        const rtl::OUString & rModuleName,
        const uno::Sequence< uno::TypeClass > & rTypes,
        reflection::TypeDescriptionSearchDepth eDepth,
        const RegistryTypeReaderLoader & rLoader,
        const RegistryKeyList & rBaseKeys )
    throw ( reflection::NoSuchTypeNameException,
            reflection::InvalidTypeNameException,
            uno::RuntimeException )
{
    if ( rModuleName.getLength() == 0 )
    {
        // Enumeration for root requested.
        return rtl::Reference< TypeDescriptionEnumerationImpl >(
            new TypeDescriptionEnumerationImpl(
                xContext, rBaseKeys, rTypes, eDepth, rLoader ) );
    }

    RegistryKeyList aModuleKeys;

    if ( rLoader.isLoaded() )
    {
        rtl::OUString aKey( rModuleName.replace( '.', '/' ) );

        bool bOpenKeySucceeded = false;

        const RegistryKeyList::const_iterator end = rBaseKeys.end();
        RegistryKeyList::const_iterator it = rBaseKeys.begin();

        while ( it != end )
        {
            uno::Reference< registry::XRegistryKey > xKey;
            try
            {
                xKey = (*it)->openKey( aKey );
                if ( xKey.is() )
                {
                    // closes key in it's dtor (which is
                    // called even in case of exceptions).
                    RegistryKeyCloser aCloser( xKey );

                    if ( xKey->isValid() )
                    {
                        bOpenKeySucceeded = true;

                        if ( xKey->getValueType()
                                == registry::RegistryValueType_BINARY )
                        {
                            uno::Sequence< sal_Int8 > aBytes(
                                xKey->getBinaryValue() );

                            RegistryTypeReader aReader(
                                rLoader,
                                (const sal_uInt8 *)aBytes.getConstArray(),
                                aBytes.getLength(),
                                sal_False );

                            rtl::OUString aName(
                                aReader.getTypeName().replace( '/', '.' ) );

                            if ( aReader.getTypeClass() == RT_TYPE_MODULE )
                            {
                                // Do not close xKey!
                                aCloser.reset();

                                aModuleKeys.push_back( xKey );
                            }
                        }
                    }
                    else
                    {
                        OSL_ENSURE( sal_False,
                            "TypeDescriptionEnumerationImpl::createInstance "
                            "- Invalid registry key!" );
                    }
                }
            }
            catch ( registry::InvalidRegistryException const & )
            {
                // openKey, getValueType, getBinaryValue

                OSL_ENSURE( sal_False,
                            "TypeDescriptionEnumerationImpl::createInstance "
                            "- Caught InvalidRegistryException!" );
            }

            it++;
        }

        if ( !bOpenKeySucceeded )
            throw reflection::NoSuchTypeNameException();

        if ( aModuleKeys.size() == 0 )
            throw reflection::InvalidTypeNameException();
    }

    return rtl::Reference< TypeDescriptionEnumerationImpl >(
        new TypeDescriptionEnumerationImpl(
                xContext, aModuleKeys, rTypes, eDepth, rLoader ) );
}

//=========================================================================
TypeDescriptionEnumerationImpl::TypeDescriptionEnumerationImpl(
                            const uno::Reference<
                                uno::XComponentContext > & xContext,
                            const RegistryKeyList & rModuleKeys,
                            const uno::Sequence< uno::TypeClass > & rTypes,
                            reflection::TypeDescriptionSearchDepth eDepth,
                            const RegistryTypeReaderLoader & rLoader )
: m_aModuleKeys( rModuleKeys ),
  m_aTypes( rTypes ),
  m_eDepth( eDepth ),
  m_aLoader( rLoader ),
  m_xContext( xContext )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}

//=========================================================================
// virtual
TypeDescriptionEnumerationImpl::~TypeDescriptionEnumerationImpl()
{
    RegistryKeyList::const_iterator it = m_aModuleKeys.begin();
    RegistryKeyList::const_iterator end = m_aModuleKeys.end();
/*
   @@@ in case we enumerate root and queryMore was never called, then
       m_aModuleKeys contains open root keys which where passed from
       tdprov and must not be closed by us.

    while ( it != end )
    {
        try
        {
            if ( (*it)->isValid() )
                (*it)->closeKey();
        }
        catch (...)
        {
            // No exceptions from dtors, please!
            OSL_ENSURE( sal_False,
            "TypeDescriptionEnumerationImpl::~TypeDescriptionEnumerationImpl "
            "- Caught exception!" );
        }

        it++;
    }
*/
    it = m_aCurrentModuleSubKeys.begin();
    end = m_aCurrentModuleSubKeys.end();
    while ( it != end )
    {
        try
        {
            if ( (*it)->isValid() )
                (*it)->closeKey();
        }
        catch (Exception &)
        {
            // No exceptions from dtors, please!
            OSL_ENSURE( sal_False,
            "TypeDescriptionEnumerationImpl::~TypeDescriptionEnumerationImpl "
            "- Caught exception!" );
        }

        it++;
    }

    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

//=========================================================================
//
// XEnumeration (base of XTypeDescriptionEnumeration) methods
//
//=========================================================================

// virtual
sal_Bool SAL_CALL TypeDescriptionEnumerationImpl::hasMoreElements()
    throw ( uno::RuntimeException )
{
    return queryMore();
}

//=========================================================================
// virtual
uno::Any SAL_CALL TypeDescriptionEnumerationImpl::nextElement()
    throw ( container::NoSuchElementException,
            lang::WrappedTargetException,
            uno::RuntimeException )
{
    return uno::Any( uno::makeAny( nextTypeDescription() ) );
}

//=========================================================================
//
// XTypeDescriptionEnumeration methods
//
//=========================================================================

// virtual
uno::Reference< reflection::XTypeDescription > SAL_CALL
TypeDescriptionEnumerationImpl::nextTypeDescription()
    throw ( container::NoSuchElementException,
            uno::RuntimeException )
{
    uno::Reference< reflection::XTypeDescription > xTD( queryNext() );

    if ( xTD.is() )
        return xTD;

    throw container::NoSuchElementException(
        rtl::OUString::createFromAscii(
            "No further elements in enumeration!" ),
        static_cast< cppu::OWeakObject * >( this  ) );
}

//=========================================================================
bool TypeDescriptionEnumerationImpl::queryMore()
{
    osl::MutexGuard aGuard( m_aMutex );

    for (;;)
    {
        if ( !m_aCurrentModuleSubKeys.empty() || !m_aTypeDescs.empty() )
        {
            // Okay, there is at least one more element.
            return true;
        }

        if ( m_aModuleKeys.empty() )
        {
            // No module keys (therefore no elements) left.
            return false;
        }

        // Note: m_aCurrentModuleSubKeys is always empty AND m_aModuleKeys is
        //       never empty when ariving here.
        //       ==> select new module key, fill m_aCurrentModuleSubKeys

        uno::Sequence< uno::Reference< registry::XRegistryKey > > aKeys;
        try
        {
            aKeys = m_aModuleKeys.front()->openKeys();
            for ( sal_Int32 n = 0; n < aKeys.getLength(); ++n )
            {
                uno::Reference< registry::XRegistryKey > xKey = aKeys[ n ];

                // closes key in it's dtor (which is
                // called even in case of exceptions).
                RegistryKeyCloser aCloser( xKey );

                try
                {
                    if ( xKey->isValid() )
                    {
                        if ( xKey->getValueType()
                                == registry::RegistryValueType_BINARY )
                        {
                            bool bIncludeIt = (m_aTypes.getLength() == 0);
                            bool bNeedTypeClass =
                                ((m_aTypes.getLength() > 0) ||
                                 (m_eDepth
                                    == reflection::TypeDescriptionSearchDepth_INFINITE));
                            if ( bNeedTypeClass )
                            {
                                uno::Sequence< sal_Int8 > aBytes(
                                    xKey->getBinaryValue() );

                                RegistryTypeReader aReader(
                                    m_aLoader,
                                    (const sal_uInt8 *)aBytes.getConstArray(),
                                    aBytes.getLength(),
                                    sal_False );

                                RTTypeClass eTypeClass = aReader.getTypeClass();

                                // Does key match requested types? Empty
                                // sequence means include all.
                                if ( m_aTypes.getLength() > 0 )
                                {
                                    for ( sal_Int32 m = 0;
                                          m < m_aTypes.getLength();
                                          ++m )
                                    {
                                        if ( m_aTypes[ m ] == eTypeClass )
                                        {
                                            bIncludeIt = true;
                                            break;
                                        }
                                    }
                                }

                                if ( m_eDepth ==
                                        reflection::TypeDescriptionSearchDepth_INFINITE )
                                {
                                    if ( eTypeClass == RT_TYPE_MODULE )
                                    {
                                        // Do not close xKey!
                                        aCloser.reset();

                                        // Remember new module key.
                                        m_aModuleKeys.push_back( xKey );
                                    }
                                }
                            }

                            if ( bIncludeIt )
                            {
                                // Do not close xKey!
                                aCloser.reset();

                                m_aCurrentModuleSubKeys.push_back( xKey );
                            }
                        }
                    }
                    else
                    {
                        OSL_ENSURE( sal_False,
                            "TypeDescriptionEnumerationImpl::queryMore "
                            "- Invalid registry key!" );
                    }

                }
                catch ( registry::InvalidRegistryException const & )
                {
                    // getValueType, getBinaryValue

                    OSL_ENSURE( sal_False,
                                "TypeDescriptionEnumerationImpl::queryMore "
                                "- Caught InvalidRegistryException!" );

                    // Don't stop iterating!
                }
            }
        }
        catch ( registry::InvalidRegistryException const & )
        {
            // openKeys

            for ( sal_Int32 n = 0; n < aKeys.getLength(); ++n )
            {
                try
                {
                    aKeys[ n ]->closeKey();
                }
                catch ( registry::InvalidRegistryException const & )
                {
                    OSL_ENSURE( sal_False,
                                "TypeDescriptionEnumerationImpl::queryMore "
                                "- Caught InvalidRegistryException!" );
                }
            }
        }

        /////////////////////////////////////////////////////////////////////
        // Special handling for constants contained directly in module.
        /////////////////////////////////////////////////////////////////////

        // Constants requested?
        bool bIncludeConstants = ( m_aTypes.getLength() == 0 );
        if ( !bIncludeConstants )
        {
            for ( sal_Int32 m = 0; m < m_aTypes.getLength(); ++m )
            {
                if ( m_aTypes[ m ] == uno::TypeClass_CONSTANT )
                {
                    bIncludeConstants = true;
                    break;
                }
            }

        }

        if ( bIncludeConstants )
        {
            if ( m_aModuleKeys.front()->getValueType()
                    == registry::RegistryValueType_BINARY )
            {
                try
                {
                    uno::Sequence< sal_Int8 > aBytes(
                        m_aModuleKeys.front()->getBinaryValue() );

                    RegistryTypeReader aReader(
                        m_aLoader, (const sal_uInt8 *)aBytes.getConstArray(),
                        aBytes.getLength(), sal_False );

                    if ( aReader.getTypeClass() == RT_TYPE_MODULE )
                    {
                        sal_uInt16 nFields
                            = (sal_uInt16)aReader.getFieldCount();
                        while ( nFields-- )
                        {
                            rtl::OUStringBuffer aName(
                                aReader.getTypeName().replace( '/', '.' ) );
                            aName.appendAscii( "." );
                            aName.append( aReader.getFieldName( nFields ) );

                            uno::Any aValue(
                                getRTValue(
                                    aReader.getFieldConstValue( nFields ) ) );

                            m_aTypeDescs.push_back(
                                new ConstantTypeDescriptionImpl(
                                        aName.makeStringAndClear(), aValue ) );
                        }
                    }
                }
                catch ( registry::InvalidRegistryException const & )
                {
                    // getBinaryValue

                    OSL_ENSURE( sal_False,
                                "TypeDescriptionEnumerationImpl::queryMore "
                                "- Caught InvalidRegistryException!" );
                }
            }
        }

        /////////////////////////////////////////////////////////////////////

/*
   @@@ m_aModuleKeys.front() may have open sub keys (may be contained in
       both m_aModuleKeys and m_aCurrentModuleSubKeys)!

        try
        {
           m_aModuleKeys.front()->closeKey();
        }
        catch ( registry::InvalidRegistryException const & )
        {
            OSL_ENSURE( sal_False,
                        "TypeDescriptionEnumerationImpl::queryMore "
                        "- Caught InvalidRegistryException!" );
        }
*/
        // We're done with this module key, even if there were errors.
        m_aModuleKeys.pop_front();
    }

    // unreachable
}

//=========================================================================
uno::Reference< reflection::XTypeDescription >
TypeDescriptionEnumerationImpl::queryNext()
{
    osl::MutexGuard aGuard( m_aMutex );

    for (;;)
    {
        if ( !queryMore() )
            return uno::Reference< reflection::XTypeDescription >();

        uno::Reference< reflection::XTypeDescription > xTD;

        if ( !m_aTypeDescs.empty() )
        {
            xTD = m_aTypeDescs.front();
            m_aTypeDescs.pop_front();
            return xTD;
        }

        // Note: xKey is already opened.
        uno::Reference< registry::XRegistryKey >
            xKey( m_aCurrentModuleSubKeys.front() );
/*
   @@@ xKey may still be contained in m_aModuleKeys, too

        // closes key in it's dtor (which is
        // called even in case of exceptions).
        RegistryKeyCloser aCloser( xKey );
*/
        try
        {
            {
                if ( xKey->isValid() )
                {
                    if ( xKey->getValueType()
                            == registry::RegistryValueType_BINARY )
                    {
                        uno::Sequence< sal_Int8 > aBytes(
                            xKey->getBinaryValue() );

                        xTD = createTypeDescription( m_aLoader,
                                                     aBytes,
                                                     getTDMgr(),
                                                     false );
                        OSL_ENSURE( xTD.is(),
                            "TypeDescriptionEnumerationImpl::queryNext "
                            "- No XTypeDescription created!" );
                    }
                }
                else
                {
                    OSL_ENSURE( sal_False,
                        "TypeDescriptionEnumerationImpl::queryNext "
                        "- Invalid registry key!" );
                }
            }
        }
        catch ( registry::InvalidRegistryException const & )
        {
            // getValueType, getBinaryValue

            OSL_ENSURE( sal_False,
                        "TypeDescriptionEnumerationImpl::queryNext "
                        "- Caught InvalidRegistryException!" );
        }

        // We're done with this key, even if there were errors.
        m_aCurrentModuleSubKeys.pop_front();

        if ( xTD.is() )
            return xTD;

        // next try...

    } // for (;;)

    return uno::Reference< reflection::XTypeDescription >();
}

//=========================================================================
uno::Reference< container::XHierarchicalNameAccess >
TypeDescriptionEnumerationImpl::getTDMgr()
{
    if ( !m_xTDMgr.is() )
    {
        uno::Reference< container::XHierarchicalNameAccess > xTDMgr;
        m_xContext->getValueByName(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "/singletons/com.sun.star.reflection.theTypeDescriptionManager" ) ) )
            >>= xTDMgr;

        OSL_ENSURE( xTDMgr.is(),
            "TypeDescriptionEnumerationImpl::getTDMgr "
            "Cannot get singleton \"TypeDescriptionManager\" from context!" );
        {
            MutexGuard guard( m_aMutex );
            if ( !m_xTDMgr.is() )
                m_xTDMgr = xTDMgr;
        }
    }
    return m_xTDMgr;
}

} // namespace stoc_rdbtdp

