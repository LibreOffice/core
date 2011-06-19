/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("No further elements in enumeration!") ),
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
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("No further elements in enumeration!") ),
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
            OSL_FAIL( "TypeDescriptionEnumerationImpl::queryCurrentChildEnumeration "
               "- Caught NoSuchTypeNameException!" );
        }
        catch ( reflection::InvalidTypeNameException const & )
        {
            OSL_FAIL( "TypeDescriptionEnumerationImpl::queryCurrentChildEnumeration "
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
