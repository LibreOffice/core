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



#ifndef DBACORE_SDBCORETOOLS_HXX
#define DBACORE_SDBCORETOOLS_HXX

/** === begin UNO includes === **/
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
/** === end UNO includes === **/

namespace comphelper
{
    class ComponentContext;
}

//.........................................................................
namespace dbaccess
{
//.........................................................................

    // -----------------------------------------------------------------------------
    void notifyDataSourceModified(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxObject,sal_Bool _bModified);

    // -----------------------------------------------------------------------------
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        getDataSource( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxDependentObject );

    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    /** retrieves a to-be-displayed string for a given caught exception;
    */
    ::rtl::OUString extractExceptionMessage( const ::comphelper::ComponentContext& _rContext, const ::com::sun::star::uno::Any& _rError );

    namespace tools
    {
        namespace stor
        {
            bool    storageIsWritable_nothrow(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxStorage
                    );

            /// commits a given storage if it's not readonly
            bool    commitStorageIfWriteable(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxStorage
                    )
                    SAL_THROW((
                        ::com::sun::star::io::IOException,
                        ::com::sun::star::lang::WrappedTargetException
                ));
        }

    }

}   // namespace dbaccess
//.........................................................................

#endif // DBACORE_SDBCORETOOLS_HXX

