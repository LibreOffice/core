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

#include "documentevents.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/namedvaluecollection.hxx>

#include <algorithm>
#include <functional>
#include <o3tl/compat_functional.hxx>

namespace dbaccess
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;

    // DocumentEvents_Data
    struct DocumentEvents_Data : public ::boost::noncopyable
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
    };

    // helper
    struct DocumentEventData
    {
        const sal_Char* pAsciiEventName;
        bool            bNeedsSyncNotify;
    };

    namespace
    {
        static const DocumentEventData* lcl_getDocumentEventData()
        {
            static const DocumentEventData s_aData[] = {
                { "OnCreate",               true  },
                { "OnLoadFinished",         true  },
                { "OnNew",                  false },    // compatibility, see http://www.openoffice.org/issues/show_bug.cgi?id=46484
                { "OnLoad",                 false },    // compatibility, see http://www.openoffice.org/issues/show_bug.cgi?id=46484
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
                { NULL, false }
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
            DocumentEventsData::iterator existingPos = m_pData->rEventsData.find( sEventName );
            if ( existingPos == m_pData->rEventsData.end() )
                m_pData->rEventsData[ sEventName ] = Sequence< PropertyValue >();
            ++pEventData;
        }
    }

    DocumentEvents::~DocumentEvents()
    {
    }

    void SAL_CALL DocumentEvents::acquire() throw()
    {
        m_pData->rParent.acquire();
    }

    void SAL_CALL DocumentEvents::release() throw()
    {
        m_pData->rParent.release();
    }

    bool DocumentEvents::needsSynchronousNotification( const OUString& _rEventName )
    {
        const DocumentEventData* pEventData = lcl_getDocumentEventData();
        while ( pEventData->pAsciiEventName )
        {
            if ( _rEventName.compareToAscii( pEventData->pAsciiEventName ) == 0 )
                return pEventData->bNeedsSyncNotify;
            ++pEventData;
        }

        // this is an unknown event ... assume async notification
        return false;
    }

    void SAL_CALL DocumentEvents::replaceByName( const OUString& _Name, const Any& _Element ) throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_pData->rMutex );

        DocumentEventsData::iterator elementPos = m_pData->rEventsData.find( _Name );
        if ( elementPos == m_pData->rEventsData.end() )
            throw NoSuchElementException( _Name, *this );

        Sequence< PropertyValue > aEventDescriptor;
        if ( _Element.hasValue() && !( _Element >>= aEventDescriptor ) )
            throw IllegalArgumentException( _Element.getValueTypeName(), *this, 2 );

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

    Any SAL_CALL DocumentEvents::getByName( const OUString& _Name ) throw (NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_pData->rMutex );

        DocumentEventsData::const_iterator elementPos = m_pData->rEventsData.find( _Name );
        if ( elementPos == m_pData->rEventsData.end() )
            throw NoSuchElementException( _Name, *this );

        Any aReturn;
        const Sequence< PropertyValue >& rEventDesc( elementPos->second );
        if ( rEventDesc.getLength() > 0 )
            aReturn <<= rEventDesc;
        return aReturn;
    }

    Sequence< OUString > SAL_CALL DocumentEvents::getElementNames(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_pData->rMutex );

        Sequence< OUString > aNames( m_pData->rEventsData.size() );
        ::std::transform(
            m_pData->rEventsData.begin(),
            m_pData->rEventsData.end(),
            aNames.getArray(),
            ::o3tl::select1st< DocumentEventsData::value_type >()
        );
        return aNames;
    }

    ::sal_Bool SAL_CALL DocumentEvents::hasByName( const OUString& _Name ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_pData->rMutex );

        return m_pData->rEventsData.find( _Name ) != m_pData->rEventsData.end();
    }

    Type SAL_CALL DocumentEvents::getElementType(  ) throw (RuntimeException)
    {
        return ::cppu::UnoType< Sequence< PropertyValue > >::get();
    }

    ::sal_Bool SAL_CALL DocumentEvents::hasElements(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_pData->rMutex );
        return !m_pData->rEventsData.empty();
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
