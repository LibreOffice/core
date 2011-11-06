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



#ifndef DBACCESS_SOURCE_SDBTOOLS_INC_OBJECTNAMES_HXX
#define DBACCESS_SOURCE_SDBTOOLS_INC_OBJECTNAMES_HXX

#include "connectiondependent.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/sdb/tools/XObjectNames.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <cppuhelper/implbase1.hxx>

#include <memory>

//........................................................................
namespace sdbtools
{
//........................................................................

    //====================================================================
    //= ObjectNames
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::sdb::tools::XObjectNames
                                    >   ObjectNames_Base;
    struct ObjectNames_Impl;
    /** default implementation for XObjectNames
    */
    class ObjectNames   :public ObjectNames_Base
                        ,public ConnectionDependentComponent
    {
    private:
        ::std::auto_ptr< ObjectNames_Impl >   m_pImpl;

    public:
        /** constructs the instance

            @param _rContext
                the component's context
            @param  _rxConnection
                the connection to work with. Will be held weak. Must not be <NULL/>.

            @throws ::com::sun::star::lang::NullPointerException
                if _rxConnection is <NULL/>
        */
        ObjectNames(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
        );

        // XObjectNames
        virtual ::rtl::OUString SAL_CALL suggestName( ::sal_Int32 CommandType, const ::rtl::OUString& BaseName ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL convertToSQLName( const ::rtl::OUString& Name ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isNameUsed( ::sal_Int32 CommandType, const ::rtl::OUString& Name ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isNameValid( ::sal_Int32 CommandType, const ::rtl::OUString& Name ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL checkNameForCreate( ::sal_Int32 CommandType, const ::rtl::OUString& Name ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~ObjectNames();

    private:
        ObjectNames();                                  // never implemented
        ObjectNames( const ObjectNames& );              // never implemented
        ObjectNames& operator=( const ObjectNames& );   // never implemented
    };

//........................................................................
} // namespace sdbtools
//........................................................................

#endif // DBACCESS_SOURCE_SDBTOOLS_INC_OBJECTNAMES_HXX

