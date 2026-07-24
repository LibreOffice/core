/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <cppuhelper/implbase.hxx>

#include <map>

namespace dbaccess
{

    typedef std::map< OUString, cpo::uno::Sequence< css::beans::PropertyValue > >
            DocumentEventsData;

    typedef ::cppu::WeakImplHelper<   css::container::XNameReplace
                                  >   DocumentEvents_Base;

    class DocumentEvents    :public DocumentEvents_Base
    {
    public:
        DocumentEvents( ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex, DocumentEventsData& _rEventsData );
        virtual ~DocumentEvents() override;

        DocumentEvents(const DocumentEvents&) = delete;
        const DocumentEvents& operator=(const DocumentEvents&) = delete;

        static bool needsSynchronousNotification( std::u16string_view _rEventName );

        // XInterface
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;

        // XNameReplace
        virtual void replaceByName( const OUString& aName, const cpo::uno::Any& aElement ) override;

        // XNameAccess
        virtual cpo::uno::Any getByName( const OUString& aName ) override;
        virtual cpo::uno::Sequence< OUString > getElementNames(  ) override;
        virtual bool hasByName( const OUString& aName ) override;

        // XElementAccess
        virtual cpo::uno::Type getElementType(  ) override;
        virtual bool hasElements(  ) override;

    private:
        ::cppu::OWeakObject&    mrParent;
        ::osl::Mutex&           mrMutex;
        DocumentEventsData&     mrEventsData;
    };

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
