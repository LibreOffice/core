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

    // DocumentEvents_Data
    struct DocumentEvents_Data
    {
        ::cppu::OWeakObject&    rParent;
        ::osl::Mutex&           rMutex;
        DocumentEventsData&     rEventsData;

        DocumentEvents_Data( ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex, DocumentEventsData& _rEventsData )
            :rParent( _rParent )
            ,rMutex( _rMutex )
            ,rEventsData( _rEventsData )
        {
        }
        DocumentEvents_Data(const DocumentEvents_Data&) = delete;
        const DocumentEvents_Data& operator=(const DocumentEvents_Data&) = delete;
    };

    namespace {

        // helper
        struct DocumentEventData
        {
            const char* pAsciiEventName;
            bool        bNeedsSyncNotify;
        };

        const DocumentEventData* lcl_getDocumentEventData()
        {
            static const DocumentEventData s_aData[] = {
                { "OnCreate",               true  },
                { "OnLoadFinished",         true  },
                { "OnNew",                  false },    // compatibility, see https://bz.apache.org/ooo/show_bug.cgi?id=46484
                { "OnLoad",                 false },    // compatibility, see https://bz.apache.org/ooo/show_bug.cgi?id=46484
                { "OnSaveAs",               true  },
                { "OnSaveAsDone",           false },
                { "OnSaveAsFailed",         false },
                { "OnSave",                 true  },
                { "OnSaveDone",             false },
                { "OnSaveFailed",           false },
                { "OnSaveTo",               true  },
                { "OnSaveToDone",           false },
                { "OnSaveToFailed",         false },
                { "OnPrepareUnload",        true  },
                { "OnUnload",               true  },
                { "OnFocus",                false },
                { "OnUnfocus",              false },
                { "OnModifyChanged",        false },
                { "OnViewCreated",          false },
                { "OnPrepareViewClosing",   true  },
                { "OnViewClosed",           false },
                { "OnTitleChanged",         false },
                { "OnSubComponentOpened",   false },
                { "OnSubComponentClosed",   false },
                { nullptr, false }
            };
            return s_aData;
        }
    }

    // DocumentEvents
    DocumentEvents::DocumentEvents( ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex, DocumentEventsData& _rEventsData )
        :m_pData( new DocumentEvents_Data( _rParent, _rMutex, _rEventsData ) )
    {
        const DocumentEventData* pEventData = lcl_getDocumentEventData();
        while ( pEventData->pAsciiEventName )
        {
            OUString sEventName = OUString::createFromAscii( pEventData->pAsciiEventName );
            DocumentEventsData::const_iterator existingPos = m_pData->rEventsData.find( sEventName );
            if ( existingPos == m_pData->rEventsData.end() )
                m_pData->rEventsData[ sEventName ] = Sequence< PropertyValue >();
            ++pEventData;
        }
    }

    DocumentEvents::~DocumentEvents()
    {
    }

    void SAL_CALL DocumentEvents::acquire() noexcept
    {
        m_pData->rParent.acquire();
    }

    void SAL_CALL DocumentEvents::release() noexcept
    {
        m_pData->rParent.release();
    }

    bool DocumentEvents::needsSynchronousNotification( const OUString& _rEventName )
    {
        const DocumentEventData* pEventData = lcl_getDocumentEventData();
        while ( pEventData->pAsciiEventName )
        {
            if ( _rEventName.equalsAscii( pEventData->pAsciiEventName ) )
                return pEventData->bNeedsSyncNotify;
            ++pEventData;
        }

        // this is an unknown event ... assume async notification
        return false;
    }

    void SAL_CALL DocumentEvents::replaceByName( const OUString& Name, const Any& Element )
    {
        ::osl::MutexGuard aGuard( m_pData->rMutex );

        DocumentEventsData::iterator elementPos = m_pData->rEventsData.find( Name );
        if ( elementPos == m_pData->rEventsData.end() )
            throw NoSuchElementException( Name, *this );

        Sequence< PropertyValue > aEventDescriptor;
        if ( Element.hasValue() && !( Element >>= aEventDescriptor ) )
            throw IllegalArgumentException( Element.getValueTypeName(), *this, 2 );

        // Weird enough, the event assignment UI has (well: had) the idea of using an empty "EventType"/"Script"
        // to indicate the event descriptor should be reset, instead of just passing an empty event descriptor.
        ::comphelper::NamedValueCollection aCheck( aEventDescriptor );
        if ( aCheck.has( "EventType" ) )
        {
            OUString sEventType = aCheck.getOrDefault( "EventType", OUString() );
            OSL_ENSURE( !sEventType.isEmpty(), "DocumentEvents::replaceByName: doing a reset via an empty EventType is weird!" );
            if ( sEventType.isEmpty() )
                aEventDescriptor.realloc( 0 );
        }
        if ( aCheck.has( "Script" ) )
        {
            OUString sScript = aCheck.getOrDefault( "Script", OUString() );
            OSL_ENSURE( !sScript.isEmpty(), "DocumentEvents::replaceByName: doing a reset via an empty Script is weird!" );
            if ( sScript.isEmpty() )
                aEventDescriptor.realloc( 0 );
        }

        elementPos->second = aEventDescriptor;
    }

    Any SAL_CALL DocumentEvents::getByName( const OUString& Name )
    {
        ::osl::MutexGuard aGuard( m_pData->rMutex );

        DocumentEventsData::const_iterator elementPos = m_pData->rEventsData.find( Name );
        if ( elementPos == m_pData->rEventsData.end() )
            throw NoSuchElementException( Name, *this );

        Any aReturn;
        const Sequence< PropertyValue >& rEventDesc( elementPos->second );
        if ( rEventDesc.hasElements() )
            aReturn <<= rEventDesc;
        return aReturn;
    }

    Sequence< OUString > SAL_CALL DocumentEvents::getElementNames(  )
    {
        ::osl::MutexGuard aGuard( m_pData->rMutex );

        return comphelper::mapKeysToSequence( m_pData->rEventsData );
    }

    sal_Bool SAL_CALL DocumentEvents::hasByName( const OUString& Name )
    {
        ::osl::MutexGuard aGuard( m_pData->rMutex );

        return m_pData->rEventsData.find( Name ) != m_pData->rEventsData.end();
    }

    Type SAL_CALL DocumentEvents::getElementType(  )
    {
        return ::cppu::UnoType< Sequence< PropertyValue > >::get();
    }

    sal_Bool SAL_CALL DocumentEvents::hasElements(  )
    {
        ::osl::MutexGuard aGuard( m_pData->rMutex );
        return !m_pData->rEventsData.empty();
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
