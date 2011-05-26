/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *      Bjoern Michaelsen <bjoern.michaelsen@canonical.com>
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Major Contributor(s):
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef EXTENSIONS_RESOURCE_RESOURCESTRINGINDEXACCESS_HXX
#define EXTENSIONS_RESOURCE_RESOURCESTRINGINDEXACCESS_HXX

#include "precompiled_extensions.hxx"

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppuhelper/implbase1.hxx>
#include <memory>

class ResMgr;

namespace extensions { namespace resource
{
    class ResourceStringIndexAccess : public cppu::WeakImplHelper1< ::com::sun::star::container::XIndexAccess>
    {
        public:
            ResourceStringIndexAccess(::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> const& rArgs, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> const&);
            // XIndexAccess
            virtual ::sal_Int32 SAL_CALL getCount(  ) throw (::com::sun::star::uno::RuntimeException)
                { return m_pResMgr.get() ? SAL_MAX_UINT16 : 0; };
            virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
            // XElementAccess
            virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException)
                { return ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)); };
            virtual ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException)
                { return static_cast<bool>(m_pResMgr.get()); };

        private:
            // m_pResMgr should never be NULL, see initResourceStringIndexAccess
            const ::std::auto_ptr<ResMgr> m_pResMgr;
    };

}}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> initResourceStringIndexAccess(::extensions::resource::ResourceStringIndexAccess*);

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
