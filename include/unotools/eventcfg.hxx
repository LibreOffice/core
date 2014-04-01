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
#include <boost/unordered_map.hpp>
#include <vector>

#define STR_EVENT_STARTAPP                   0
#define STR_EVENT_CLOSEAPP                   1
#define STR_EVENT_DOCCREATED                 2
#define STR_EVENT_CREATEDOC                  3
#define STR_EVENT_LOADFINISHED               4
#define STR_EVENT_OPENDOC                    5
#define STR_EVENT_PREPARECLOSEDOC            6
#define STR_EVENT_CLOSEDOC                   7
#define STR_EVENT_SAVEDOC                    8
#define STR_EVENT_SAVEDOCDONE                9
#define STR_EVENT_SAVEDOCFAILED             10
#define STR_EVENT_SAVEASDOC                 11
#define STR_EVENT_SAVEASDOCDONE             12
#define STR_EVENT_SAVEASDOCFAILED           13
#define STR_EVENT_SAVETODOC                 14
#define STR_EVENT_SAVETODOCDONE             15
#define STR_EVENT_SAVETODOCFAILED           16
#define STR_EVENT_ACTIVATEDOC               17
#define STR_EVENT_DEACTIVATEDOC             18
#define STR_EVENT_PRINTDOC                  19
#define STR_EVENT_VIEWCREATED               20
#define STR_EVENT_PREPARECLOSEVIEW          21
#define STR_EVENT_CLOSEVIEW                 22
#define STR_EVENT_MODIFYCHANGED             23
#define STR_EVENT_TITLECHANGED              24
#define STR_EVENT_VISAREACHANGED            25
#define STR_EVENT_MODECHANGED               26
#define STR_EVENT_STORAGECHANGED            27

typedef ::boost::unordered_map< OUString, OUString, OUStringHash, ::std::equal_to< OUString > > EventBindingHash;
typedef ::std::vector< ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XFrame > > FrameVector;
typedef ::std::vector< OUString > SupportedEventsVector;

class GlobalEventConfig_Impl : public utl::ConfigItem
{
    EventBindingHash m_eventBindingHash;
    FrameVector m_lFrames;
    SupportedEventsVector m_supportedEvents;

    void initBindingInfo();

public:
    GlobalEventConfig_Impl( );
    virtual ~GlobalEventConfig_Impl( );

    void            Notify( const com::sun::star::uno::Sequence<OUString>& aPropertyNames) SAL_OVERRIDE;
    void            Commit() SAL_OVERRIDE;

    void SAL_CALL replaceByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException);
    bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
    bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);
    OUString GetEventName( sal_Int32 nID );
};

class UNOTOOLS_DLLPUBLIC GlobalEventConfig:
        public ::cppu::WeakImplHelper2 < ::com::sun::star::document::XEventsSupplier, ::com::sun::star::container::XNameReplace >
{
    public:
        GlobalEventConfig( );
        virtual ~GlobalEventConfig( );
        static ::osl::Mutex& GetOwnStaticMutex();

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        void SAL_CALL replaceByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        ::sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        static OUString GetEventName( sal_Int32 nID );

    private:
        static GlobalEventConfig_Impl* m_pImpl;
        static sal_Int32 m_nRefCount;
};

#endif // INCLUDED_UNOTOOLS_EVENTCFG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
