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
