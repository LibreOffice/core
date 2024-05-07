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

#include <documentevents.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/sequence.hxx>

namespace dbaccess
{

    using ::com::sun::star::uno::Any;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;

    namespace {

        // helper
        struct DocumentEventData
        {
            OUString    aAsciiEventName;
            bool        bNeedsSyncNotify;
        };

        constexpr DocumentEventData s_DocumentEventData[] {
            { u"OnCreate"_ustr,               true  },
            { u"OnLoadFinished"_ustr,         true  },
            { u"OnNew"_ustr,                  false },    // compatibility, see https://bz.apache.org/ooo/show_bug.cgi?id=46484
            { u"OnLoad"_ustr,                 false },    // compatibility, see https://bz.apache.org/ooo/show_bug.cgi?id=46484
            { u"OnSaveAs"_ustr,               true  },
            { u"OnSaveAsDone"_ustr,           false },
            { u"OnSaveAsFailed"_ustr,         false },
            { u"OnSave"_ustr,                 true  },
            { u"OnSaveDone"_ustr,             false },
            { u"OnSaveFailed"_ustr,           false },
            { u"OnSaveTo"_ustr,               true  },
            { u"OnSaveToDone"_ustr,           false },
            { u"OnSaveToFailed"_ustr,         false },
            { u"OnPrepareUnload"_ustr,        true  },
            { u"OnUnload"_ustr,               true  },
            { u"OnFocus"_ustr,                false },
            { u"OnUnfocus"_ustr,              false },
            { u"OnModifyChanged"_ustr,        false },
            { u"OnViewCreated"_ustr,          false },
            { u"OnPrepareViewClosing"_ustr,   true  },
            { u"OnViewClosed"_ustr,           false },
            { u"OnTitleChanged"_ustr,         false },
            { u"OnSubComponentOpened"_ustr,   false },
            { u"OnSubComponentClosed"_ustr,   false },
        };
    }

    // DocumentEvents
    DocumentEvents::DocumentEvents( ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex, DocumentEventsData& _rEventsData )
        :mrParent(_rParent), mrMutex(_rMutex), mrEventsData(_rEventsData)
    {
        for (const auto & rEventData : s_DocumentEventData)
        {
            OUString sEventName = rEventData.aAsciiEventName;
            DocumentEventsData::const_iterator existingPos = mrEventsData.find( sEventName );
            if ( existingPos == mrEventsData.end() )
                mrEventsData[ sEventName ] = Sequence< PropertyValue >();
        }
    }

    DocumentEvents::~DocumentEvents()
    {
    }

    void SAL_CALL DocumentEvents::acquire() noexcept
    {
        mrParent.acquire();
    }

    void SAL_CALL DocumentEvents::release() noexcept
    {
        mrParent.release();
    }

    bool DocumentEvents::needsSynchronousNotification( std::u16string_view _rEventName )
    {
        for (const auto & rEventData : s_DocumentEventData)
        {
            if ( _rEventName == rEventData.aAsciiEventName )
                return rEventData.bNeedsSyncNotify;
        }

        // this is an unknown event ... assume async notification
        return false;
    }

    void SAL_CALL DocumentEvents::replaceByName( const OUString& Name, const Any& Element )
    {
        ::osl::MutexGuard aGuard( mrMutex );

        DocumentEventsData::iterator elementPos = mrEventsData.find( Name );
        if ( elementPos == mrEventsData.end() )
            throw NoSuchElementException( Name, *this );

        Sequence< PropertyValue > aEventDescriptor;
        if ( Element.hasValue() && !( Element >>= aEventDescriptor ) )
            throw IllegalArgumentException( Element.getValueTypeName(), *this, 2 );

        // Weird enough, the event assignment UI has (well: had) the idea of using an empty "EventType"/"Script"
        // to indicate the event descriptor should be reset, instead of just passing an empty event descriptor.
        ::comphelper::NamedValueCollection aCheck( aEventDescriptor );
        if ( aCheck.has( u"EventType"_ustr ) )
        {
            OUString sEventType = aCheck.getOrDefault( u"EventType"_ustr, OUString() );
            OSL_ENSURE( !sEventType.isEmpty(), "DocumentEvents::replaceByName: doing a reset via an empty EventType is weird!" );
            if ( sEventType.isEmpty() )
                aEventDescriptor.realloc( 0 );
        }
        if ( aCheck.has( u"Script"_ustr ) )
        {
            OUString sScript = aCheck.getOrDefault( u"Script"_ustr, OUString() );
            OSL_ENSURE( !sScript.isEmpty(), "DocumentEvents::replaceByName: doing a reset via an empty Script is weird!" );
            if ( sScript.isEmpty() )
                aEventDescriptor.realloc( 0 );
        }

        elementPos->second = aEventDescriptor;
    }

    Any SAL_CALL DocumentEvents::getByName( const OUString& Name )
    {
        ::osl::MutexGuard aGuard( mrMutex );

        DocumentEventsData::const_iterator elementPos = mrEventsData.find( Name );
        if ( elementPos == mrEventsData.end() )
            throw NoSuchElementException( Name, *this );

        Any aReturn;
        const Sequence< PropertyValue >& rEventDesc( elementPos->second );
        if ( rEventDesc.hasElements() )
            aReturn <<= rEventDesc;
        return aReturn;
    }

    Sequence< OUString > SAL_CALL DocumentEvents::getElementNames(  )
    {
        ::osl::MutexGuard aGuard( mrMutex );

        return comphelper::mapKeysToSequence( mrEventsData );
    }

    sal_Bool SAL_CALL DocumentEvents::hasByName( const OUString& Name )
    {
        ::osl::MutexGuard aGuard( mrMutex );

        return mrEventsData.find( Name ) != mrEventsData.end();
    }

    Type SAL_CALL DocumentEvents::getElementType(  )
    {
        return ::cppu::UnoType< Sequence< PropertyValue > >::get();
    }

    sal_Bool SAL_CALL DocumentEvents::hasElements(  )
    {
        ::osl::MutexGuard aGuard( mrMutex );
        return !mrEventsData.empty();
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
