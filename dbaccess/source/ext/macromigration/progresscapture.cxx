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

#include "progresscapture.hxx"
#include "migrationprogress.hxx"

#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

namespace dbmm
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

    // ProgressCapture_Data
    struct ProgressCapture_Data
    {
        ProgressCapture_Data( const OUString& _rObjectName, IMigrationProgress& _rMasterProgress )
            :sObjectName( _rObjectName )
            ,rMasterProgress( _rMasterProgress )
            ,bDisposed( false )
        {
        }

        OUString     sObjectName;
        IMigrationProgress& rMasterProgress;
        bool                bDisposed;
    };

    // ProgressCapture
    ProgressCapture::ProgressCapture( const OUString& _rObjectName, IMigrationProgress& _rMasterProgress )
        :m_pData( new ProgressCapture_Data( _rObjectName, _rMasterProgress ) )
    {
    }

    ProgressCapture::~ProgressCapture()
    {
    }

    void ProgressCapture::dispose()
    {
        SolarMutexGuard aGuard;
        m_pData->bDisposed = true;
    }

    void SAL_CALL ProgressCapture::start( const OUString& _rText, ::sal_Int32 _nRange ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;
        if ( !m_pData->bDisposed )
            m_pData->rMasterProgress.startObject( m_pData->sObjectName, _rText, _nRange );
    }

    void SAL_CALL ProgressCapture::end(  ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;
        if ( !m_pData->bDisposed )
            m_pData->rMasterProgress.endObject();
    }

    void SAL_CALL ProgressCapture::setText( const OUString& _rText ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;
        if ( !m_pData->bDisposed )
            m_pData->rMasterProgress.setObjectProgressText( _rText );
    }

    void SAL_CALL ProgressCapture::setValue( ::sal_Int32 _nValue ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;
        if ( !m_pData->bDisposed )
            m_pData->rMasterProgress.setObjectProgressValue( _nValue );
    }

    void SAL_CALL ProgressCapture::reset(  ) throw (RuntimeException)
    {
        OSL_FAIL( "ProgressCapture::reset: not implemented!" );
    }

} // namespace dbmm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
