/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdmgr_tdenumeration.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 13:05:13 $
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
#include "precompiled_stoc.hxx"

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _STOC_TDMGR_COMMON_HXX
#include "tdmgr_common.hxx"
#endif
#ifndef _STOC_TDMGR_TDENUMERATION_HXX
#include "tdmgr_tdenumeration.hxx"
#endif

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

