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

#include <cppuhelper/implbase1.hxx>

#include <memory>
#include <map>
#include <boost/noncopyable.hpp>

namespace dbaccess
{

    typedef ::std::map< OUString, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >
            DocumentEventsData;

    // DocumentEvents
    struct DocumentEvents_Data;

    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::container::XNameReplace
                                    >   DocumentEvents_Base;

    class DocumentEvents    :public DocumentEvents_Base
                            ,public ::boost::noncopyable
    {
    public:
        DocumentEvents( ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex, DocumentEventsData& _rEventsData );
        virtual ~DocumentEvents();

        static bool needsSynchronousNotification( const OUString& _rEventName );

        // XInterface
        virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;
        virtual void SAL_CALL release() throw() SAL_OVERRIDE;

        // XNameReplace
        virtual void SAL_CALL replaceByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    private:
        ::std::unique_ptr< DocumentEvents_Data >  m_pData;
    };

}

#endif // INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTEVENTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
