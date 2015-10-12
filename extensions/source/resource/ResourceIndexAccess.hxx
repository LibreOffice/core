/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_EXTENSIONS_SOURCE_RESOURCE_RESOURCEINDEXACCESS_HXX
#define INCLUDED_EXTENSIONS_SOURCE_RESOURCE_RESOURCEINDEXACCESS_HXX


#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppuhelper/implbase.hxx>
#include <memory>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }
class ResMgr;

namespace extensions { namespace resource
{
    /** This class provides access to tools library text resources */
    class ResourceIndexAccess : public cppu::WeakImplHelper< css::container::XNameAccess>
    {
        public:
            /** The ctor takes a sequence with one element: the name of the resource, e.g. svt */
            ResourceIndexAccess(css::uno::Sequence< css::uno::Any> const& rArgs, css::uno::Reference< css::uno::XComponentContext> const&);
            // XNameAccess
            // The XNameAccess provides access to two named elements:
            //    "String" returns a XIndexAccess to String resources
            //    "StringList" returns a XIndexAccess to StringList/StringArray resources
            virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (css::uno::RuntimeException, std::exception) override;
            // XElementAccess
            virtual css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException, std::exception) override
                { return cppu::UnoType<css::uno::XInterface>::get(); };
            virtual sal_Bool SAL_CALL hasElements(  ) throw (css::uno::RuntimeException, std::exception) override
                { return static_cast<bool>(m_pResMgr.get()); };

        private:
            // m_pResMgr should never be NULL
            const std::shared_ptr<ResMgr> m_pResMgr;
    };
}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
