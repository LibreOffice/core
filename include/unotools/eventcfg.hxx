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
#ifndef INCLUDED_UNOTOOLS_EVENTCFG_HXX
#define INCLUDED_UNOTOOLS_EVENTCFG_HXX

#include <unotools/unotoolsdllapi.h>
#include <unotools/configitem.hxx>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase2.hxx>
#include <unordered_map>
#include <vector>

enum class GlobalEventId
{
    STARTAPP,
    CLOSEAPP,
    DOCCREATED,
    CREATEDOC,
    LOADFINISHED,
    OPENDOC,
    PREPARECLOSEDOC,
    CLOSEDOC,
    SAVEDOC,
    SAVEDOCDONE,
    SAVEDOCFAILED,
    SAVEASDOC,
    SAVEASDOCDONE,
    SAVEASDOCFAILED,
    SAVETODOC,
    SAVETODOCDONE,
    SAVETODOCFAILED,
    ACTIVATEDOC,
    DEACTIVATEDOC,
    PRINTDOC,
    VIEWCREATED,
    PREPARECLOSEVIEW,
    CLOSEVIEW,
    MODIFYCHANGED,
    TITLECHANGED,
    VISAREACHANGED,
    MODECHANGED,
    STORAGECHANGED,
    LAST = STORAGECHANGED
};

class GlobalEventConfig_Impl;

class UNOTOOLS_DLLPUBLIC GlobalEventConfig:
        public ::cppu::WeakImplHelper2 < ::com::sun::star::document::XEventsSupplier, ::com::sun::star::container::XNameReplace >
{
    public:
        GlobalEventConfig( );
        virtual ~GlobalEventConfig( );
        static ::osl::Mutex& GetOwnStaticMutex();

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        void SAL_CALL replaceByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        static OUString GetEventName( GlobalEventId nID );

    private:
        static GlobalEventConfig_Impl* m_pImpl;
        static sal_Int32 m_nRefCount;
};

#endif // INCLUDED_UNOTOOLS_EVENTCFG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
