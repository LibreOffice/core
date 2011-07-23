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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "progresscapture.hxx"
#include "migrationprogress.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

//........................................................................
namespace dbmm
{
//........................................................................

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
    /** === end UNO using === **/

    //====================================================================
    //= ProgressCapture_Data
    //====================================================================
    struct ProgressCapture_Data
    {
        ProgressCapture_Data( const ::rtl::OUString& _rObjectName, IMigrationProgress& _rMasterProgress )
            :sObjectName( _rObjectName )
            ,rMasterProgress( _rMasterProgress )
            ,bDisposed( false )
        {
        }

        ::rtl::OUString     sObjectName;
        IMigrationProgress& rMasterProgress;
        bool                bDisposed;
    };

    //====================================================================
    //= ProgressCapture
    //====================================================================
    //--------------------------------------------------------------------
    ProgressCapture::ProgressCapture( const ::rtl::OUString& _rObjectName, IMigrationProgress& _rMasterProgress )
        :m_pData( new ProgressCapture_Data( _rObjectName, _rMasterProgress ) )
    {
    }

    //--------------------------------------------------------------------
    ProgressCapture::~ProgressCapture()
    {
    }

    //--------------------------------------------------------------------
    void ProgressCapture::dispose()
    {
        SolarMutexGuard aGuard;
        m_pData->bDisposed = true;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ProgressCapture::start( const ::rtl::OUString& _rText, ::sal_Int32 _nRange ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;
        if ( !m_pData->bDisposed )
            m_pData->rMasterProgress.startObject( m_pData->sObjectName, _rText, _nRange );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ProgressCapture::end(  ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;
        if ( !m_pData->bDisposed )
            m_pData->rMasterProgress.endObject();
    }

    //--------------------------------------------------------------------
    void SAL_CALL ProgressCapture::setText( const ::rtl::OUString& _rText ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;
        if ( !m_pData->bDisposed )
            m_pData->rMasterProgress.setObjectProgressText( _rText );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ProgressCapture::setValue( ::sal_Int32 _nValue ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;
        if ( !m_pData->bDisposed )
            m_pData->rMasterProgress.setObjectProgressValue( _nValue );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ProgressCapture::reset(  ) throw (RuntimeException)
    {
        OSL_FAIL( "ProgressCapture::reset: not implemented!" );
    }

//........................................................................
} // namespace dbmm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
