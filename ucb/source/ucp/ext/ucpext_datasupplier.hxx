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



#ifndef UCB_UCPEXT_DATA_SUPPLIER_HXX
#define UCB_UCPEXT_DATA_SUPPLIER_HXX

#include <rtl/ref.hxx>
#include <ucbhelper/resultset.hxx>

#include <boost/scoped_ptr.hpp>

//......................................................................................................................
namespace ucb { namespace ucp { namespace ext
{
//......................................................................................................................

    struct DataSupplier_Impl;
    class Content;

    //==================================================================================================================
    //= DataSupplier
    //==================================================================================================================
    typedef ::ucbhelper::ResultSetDataSupplier  DataSupplier_Base;
    class DataSupplier : public ::ucbhelper::ResultSetDataSupplier
    {
    public:
        DataSupplier(
            const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& i_rORB,
            const rtl::Reference< Content >& rContent,
            const sal_Int32 nOpenMode
        );

        void    fetchData();

    protected:
        virtual ~DataSupplier();

        virtual rtl::OUString queryContentIdentifierString( sal_uInt32 nIndex );
        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > queryContentIdentifier( sal_uInt32 nIndex );
        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent > queryContent( sal_uInt32 nIndex );

        virtual sal_Bool getResult( sal_uInt32 nIndex );

        virtual sal_uInt32 totalCount();
        virtual sal_uInt32 currentCount();
        virtual sal_Bool isCountFinal();

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XRow > queryPropertyValues( sal_uInt32 nIndex  );
        virtual void releasePropertyValues( sal_uInt32 nIndex );

        virtual void close();

        virtual void validate() throw( com::sun::star::ucb::ResultSetException );

    private:
        ::boost::scoped_ptr< DataSupplier_Impl >    m_pImpl;
    };

//......................................................................................................................
} } }   // namespace ucp::ext
//......................................................................................................................

#endif  // UCB_UCPEXT_DATA_SUPPLIER_HXX
