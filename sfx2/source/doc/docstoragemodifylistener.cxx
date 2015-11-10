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

#include <sfx2/docstoragemodifylistener.hxx>
#include <comphelper/solarmutex.hxx>


namespace sfx2
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
    using ::com::sun::star::lang::EventObject;


    //=


    DocumentStorageModifyListener::DocumentStorageModifyListener( IModifiableDocument& _rDocument, comphelper::SolarMutex& _rMutex )
        :m_pDocument( &_rDocument )
        ,m_rMutex( _rMutex )
    {
    }


    DocumentStorageModifyListener::~DocumentStorageModifyListener()
    {
    }


    void DocumentStorageModifyListener::dispose()
    {
        ::osl::Guard< comphelper::SolarMutex > aGuard( m_rMutex );
        m_pDocument = nullptr;
    }


    void SAL_CALL DocumentStorageModifyListener::modified( const EventObject& /*aEvent*/ ) throw (RuntimeException, std::exception)
    {
        ::osl::Guard< comphelper::SolarMutex > aGuard( m_rMutex );
        // storageIsModified must not contain any locking!
        if ( m_pDocument )
            m_pDocument->storageIsModified();
    }


    void SAL_CALL DocumentStorageModifyListener::disposing( const EventObject& /*Source*/ ) throw (RuntimeException, std::exception)
    {
        // not interested in. In particular, we do *not* dispose ourself when a storage we're
        // listening at is disposed. The reason here is that this listener instance is *reused*
        // in case the document is re-based to another storage.
    }


} // namespace sfx2


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
