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



#ifndef SVX_SQLPARSERCLIENT_HXX
#define SVX_SQLPARSERCLIENT_HXX

#include "svx/dbtoolsclient.hxx"
#include "svx/ParseContext.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= OSQLParserClient
    //====================================================================
    class SVX_DLLPUBLIC OSQLParserClient : public ODbtoolsClient
                            ,public ::svxform::OParseContextClient
    {
    private:
        //add by BerryJia for fixing Bug97420 Time:2002-9-12-11:00(PRC time)
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xORB;

    protected:
        mutable ::rtl::Reference< ::connectivity::simple::ISQLParser >  m_xParser;

    protected:
        OSQLParserClient(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
        virtual bool ensureLoaded() const;

    protected:
        inline ::rtl::Reference< ::connectivity::simple::ISQLParseNode > predicateTree(
                ::rtl::OUString& _rErrorMessage,
                const ::rtl::OUString& _rStatement,
                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxField
            ) const
        {
            ::rtl::Reference< ::connectivity::simple::ISQLParseNode > xReturn;
            if ( ensureLoaded() )
                xReturn = m_xParser->predicateTree(_rErrorMessage, _rStatement, _rxFormatter, _rxField);
            return xReturn;
        }
    };

//........................................................................
}   // namespace svxform
//........................................................................

#endif // SVX_SQLPARSERCLIENT_HXX


