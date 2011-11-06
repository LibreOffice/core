/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"
#include <osl/diagnose.h>
#include "tdmgr_common.hxx"
#include "tdmgr_tdenumeration.hxx"

using namespace com::sun::star;

extern rtl_StandardModuleCount g_moduleCount;

namespace stoc_tdmgr
{

//=========================================================================
//=========================================================================
//
// TypeDescriptionEnumerationImpl Implementation.
//
//=========================================================================
//=========================================================================

TypeDescriptionEnumerationImpl::TypeDescriptionEnumerationImpl(
        const rtl::OUString & rModuleName,
        const com::sun::star::uno::Sequence<
            com::sun::star::uno::TypeClass > & rTypes,
        com::sun::star::reflection::TypeDescriptionSearchDepth eDepth,
        const TDEnumerationAccessStack & rTDEAS )
: m_aModuleName( rModuleName ),
  m_aTypes( rTypes ),
  m_eDepth( eDepth ),
  m_aChildren( rTDEAS )
{
    ::g_moduleCount.modCnt.acquire( &::g_moduleCount.modCnt );
}

//=========================================================================
// virtual
TypeDescriptionEnumerationImpl::~TypeDescriptionEnumerationImpl()
{
    ::g_moduleCount.modCnt.release( &::g_moduleCount.modCnt );
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
    uno::Reference< reflection::XTypeDescriptionEnumeration > xEnum
        = queryCurrentChildEnumeration();
    if ( xEnum.is() )
        return xEnum->hasMoreElements();

    return sal_False;
}

//=========================================================================
// virtual
uno::Any SAL_CALL TypeDescriptionEnumerationImpl::nextElement()
    throw ( container::NoSuchElementException,
            lang::WrappedTargetException,
            uno::RuntimeException )
{
    uno::Reference< reflection::XTypeDescriptionEnumeration > xEnum
        = queryCurrentChildEnumeration();
    if ( xEnum.is() )
        return xEnum->nextElement();

    throw container::NoSuchElementException(
        rtl::OUString::createFromAscii(
            "No further elements in enumeration!" ),
        static_cast< cppu::OWeakObject * >( this  ) );
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
    uno::Reference< reflection::XTypeDescriptionEnumeration > xEnum
        = queryCurrentChildEnumeration();
    if ( xEnum.is() )
        return xEnum->nextTypeDescription();

    throw container::NoSuchElementException(
        rtl::OUString::createFromAscii(
            "No further elements in enumeration!" ),
        static_cast< cppu::OWeakObject * >( this  ) );
}

//=========================================================================
uno::Reference< reflection::XTypeDescriptionEnumeration >
TypeDescriptionEnumerationImpl::queryCurrentChildEnumeration()
{
    osl::MutexGuard aGuard( m_aMutex );

    for (;;)
    {
        if ( m_xEnum.is() )
        {
            if ( m_xEnum->hasMoreElements() )
            {
                return m_xEnum;
            }
            else
            {
                // Forget about enumeration without further elements. Try next.
                m_xEnum.clear();
            }
        }

        // Note: m_xEnum is always null here.

        if ( m_aChildren.empty() )
        {
            // No child enumerations left.
            return m_xEnum;
        }

        try
        {
            m_xEnum =
                m_aChildren.top()->createTypeDescriptionEnumeration(
                    m_aModuleName, m_aTypes, m_eDepth );
        }
        catch ( reflection::NoSuchTypeNameException const & )
        {
            OSL_ENSURE( sal_False,
               "TypeDescriptionEnumerationImpl::queryCurrentChildEnumeration "
               "- Caught NoSuchTypeNameException!" );
        }
        catch ( reflection::InvalidTypeNameException const & )
        {
            OSL_ENSURE( sal_False,
               "TypeDescriptionEnumerationImpl::queryCurrentChildEnumeration "
               "- Caught InvalidTypeNameException!" );
        }

        // We're done with this enumeration access in any case (Either
        // enumeration was successfully created or creation failed for some
        // reason).
        m_aChildren.pop();
    }

    // unreachable
}

} // namespace stoc_tdmgr

