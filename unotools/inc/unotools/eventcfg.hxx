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
#ifndef _EVENTCFG_HXX
#define _EVENTCFG_HXX

#include "unotools/unotoolsdllapi.h"
#include <unotools/configitem.hxx>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase2.hxx>
#include <hash_map>
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

typedef ::std::hash_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > EventBindingHash;
typedef ::std::vector< ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XFrame > > FrameVector;
typedef ::std::vector< ::rtl::OUString > SupportedEventsVector;

class GlobalEventConfig_Impl : public utl::ConfigItem
{
    EventBindingHash m_eventBindingHash;
    FrameVector m_lFrames;
    SupportedEventsVector m_supportedEvents;

    void initBindingInfo();

public:
    GlobalEventConfig_Impl( );
    ~GlobalEventConfig_Impl( );

    void EstablishFrameCallback(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame);
    void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    void            Commit();

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException);
    ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
    ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);
    ::rtl::OUString GetEventName( sal_Int32 nID );
};

class UNOTOOLS_DLLPUBLIC GlobalEventConfig:
        public ::cppu::WeakImplHelper2 < ::com::sun::star::document::XEventsSupplier, ::com::sun::star::container::XNameReplace >
{
    public:
        GlobalEventConfig( );
        ~GlobalEventConfig( );
        static ::osl::Mutex& GetOwnStaticMutex();

        void EstablishFrameCallback(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame);
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw (::com::sun::star::uno::RuntimeException);
        void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException);
        ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
        ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::rtl::OUString GetEventName( sal_Int32 nID );

    private:
        static GlobalEventConfig_Impl* m_pImpl;
        static sal_Int32 m_nRefCount;
};

#endif // _EVENTCFG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
