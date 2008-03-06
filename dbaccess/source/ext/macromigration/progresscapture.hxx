/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: progresscapture.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:06:16 $
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

#ifndef DBACCESS_PROGRESSCAPTURE_HXX
#define DBACCESS_PROGRESSCAPTURE_HXX

/** === begin UNO includes === **/
#include <com/sun/star/task/XStatusIndicator.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

#include <memory>

//........................................................................
namespace dbmm
{
//........................................................................

    class IMigrationProgress;

    //====================================================================
    //= ProgressCapture
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::task::XStatusIndicator
                                    >   ProgressCapture_Base;

    struct ProgressCapture_Data;

    class ProgressCapture : public ProgressCapture_Base
    {
    public:
        ProgressCapture( const ::rtl::OUString& _rObjectName, IMigrationProgress& _rMasterProgress );

        void    dispose();

        // XStatusIndicator
        virtual void SAL_CALL start( const ::rtl::OUString& Text, ::sal_Int32 Range ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL end(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setText( const ::rtl::OUString& Text ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setValue( ::sal_Int32 Value ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL reset(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~ProgressCapture();

    private:
        ::std::auto_ptr< ProgressCapture_Data > m_pData;
    };

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_PROGRESSCAPTURE_HXX
