/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "documentevents.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/PropertyValue.hpp>
/** === end UNO includes === **/

#include <comphelper/namedvaluecollection.hxx>

#include <algorithm>
#include <functional>
#include <o3tl/compat_functional.hxx>

namespace dbaccess
{

    /** === begin UNO using === **/
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
    /** === end UNO using === **/

    //====================================================================
    //= DocumentEvents_Data
    //====================================================================
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

    //====================================================================
    //= helper
    //====================================================================
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

    //====================================================================
    //= DocumentEvents
    //====================================================================
    DocumentEvents::DocumentEvents( ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex, DocumentEventsData& _rEventsData )
        :m_pData( new DocumentEvents_Data( _rParent, _rMutex, _rEventsData ) )
    {
        const DocumentEventData* pEventData = lcl_getDocumentEventData();
        while ( pEventData->pAsciiEventName )
        {
            ::rtl::OUString sEventName = ::rtl::OUString::createFromAscii( pEventData->pAsciiEventName );
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

    bool DocumentEvents::needsSynchronousNotification( const ::rtl::OUString& _rEventName )
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

    void SAL_CALL DocumentEvents::replaceByName( const ::rtl::OUString& _Name, const Any& _Element ) throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
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
            ::rtl::OUString sEventType = aCheck.getOrDefault( "EventType", ::rtl::OUString() );
            OSL_ENSURE( sEventType.getLength(), "DocumentEvents::replaceByName: doing a reset via an empty EventType is weird!" );
            if ( !sEventType.getLength() )
                aEventDescriptor.realloc( 0 );
        }
        if ( aCheck.has( "Script" ) )
        {
            ::rtl::OUString sScript = aCheck.getOrDefault( "Script", ::rtl::OUString() );
            OSL_ENSURE( sScript.getLength(), "DocumentEvents::replaceByName: doing a reset via an empty Script is weird!" );
            if ( !sScript.getLength() )
                aEventDescriptor.realloc( 0 );
        }

        elementPos->second = aEventDescriptor;
    }

    Any SAL_CALL DocumentEvents::getByName( const ::rtl::OUString& _Name ) throw (NoSuchElementException, WrappedTargetException, RuntimeException)
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

    Sequence< ::rtl::OUString > SAL_CALL DocumentEvents::getElementNames(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_pData->rMutex );

        Sequence< ::rtl::OUString > aNames( m_pData->rEventsData.size() );
        ::std::transform(
            m_pData->rEventsData.begin(),
            m_pData->rEventsData.end(),
            aNames.getArray(),
            ::o3tl::select1st< DocumentEventsData::value_type >()
        );
        return aNames;
    }

    ::sal_Bool SAL_CALL DocumentEvents::hasByName( const ::rtl::OUString& _Name ) throw (RuntimeException)
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
