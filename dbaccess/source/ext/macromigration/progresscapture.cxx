/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: progresscapture.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:06:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

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
