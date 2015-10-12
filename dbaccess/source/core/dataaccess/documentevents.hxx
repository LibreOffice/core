/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTEVENTS_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTEVENTS_HXX

#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <cppuhelper/implbase.hxx>

#include <memory>
#include <map>
#include <boost/noncopyable.hpp>

namespace dbaccess
{

    typedef ::std::map< OUString, css::uno::Sequence< css::beans::PropertyValue > >
            DocumentEventsData;

    // DocumentEvents
    struct DocumentEvents_Data;

    typedef ::cppu::WeakImplHelper<   css::container::XNameReplace
                                  >   DocumentEvents_Base;

    class DocumentEvents    :public DocumentEvents_Base
                            ,public ::boost::noncopyable
    {
    public:
        DocumentEvents( ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex, DocumentEventsData& _rEventsData );
        virtual ~DocumentEvents();

        static bool needsSynchronousNotification( const OUString& _rEventName );

        // XInterface
        virtual void SAL_CALL acquire() throw() override;
        virtual void SAL_CALL release() throw() override;

        // XNameReplace
        virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XNameAccess
        virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (css::uno::RuntimeException, std::exception) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasElements(  ) throw (css::uno::RuntimeException, std::exception) override;

    private:
        ::std::unique_ptr< DocumentEvents_Data >  m_pData;
    };

} // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTEVENTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
