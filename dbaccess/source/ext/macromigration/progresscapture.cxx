/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbmm.hxx"

#include "progresscapture.hxx"
#include "migrationprogress.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

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
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        m_pData->bDisposed = true;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ProgressCapture::start( const ::rtl::OUString& _rText, ::sal_Int32 _nRange ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        if ( !m_pData->bDisposed )
            m_pData->rMasterProgress.startObject( m_pData->sObjectName, _rText, _nRange );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ProgressCapture::end(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        if ( !m_pData->bDisposed )
            m_pData->rMasterProgress.endObject();
    }

    //--------------------------------------------------------------------
    void SAL_CALL ProgressCapture::setText( const ::rtl::OUString& _rText ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        if ( !m_pData->bDisposed )
            m_pData->rMasterProgress.setObjectProgressText( _rText );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ProgressCapture::setValue( ::sal_Int32 _nValue ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        if ( !m_pData->bDisposed )
            m_pData->rMasterProgress.setObjectProgressValue( _nValue );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ProgressCapture::reset(  ) throw (RuntimeException)
    {
        OSL_ENSURE( false, "ProgressCapture::reset: not implemented!" );
    }

//........................................................................
} // namespace dbmm
//........................................................................
