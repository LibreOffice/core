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



#ifndef _FILTER_CONFIG_QUERYTOKENIZER_HXX_
#define _FILTER_CONFIG_QUERYTOKENIZER_HXX_

//_______________________________________________
// includes

#include <hash_map>
#include <rtl/ustring.hxx>

//_______________________________________________
// namespace

namespace filter{
    namespace config{

//_______________________________________________
// definitions

/** @short      It can be used to split any query string (which can be used at the
                related interface <type scope="com::sun::star::container">XContainerQuery</type>)
                into its different tokens using a fix schema.

    @descr      All queries implemented of the services
                <ul>
                    <li><type scope="com::sun::star::document">TypeDetection</type></li>
                    <li><type scope="com::sun::star::document">FilterFactory</type></li>
                    <li><type scope="com::sun::star::document">ExtendedTypeDetectionFactory</type></li>
                    <li><type scope="com::sun::star::frame">FrameLoaderFactory</type></li>
                    <li><type scope="com::sun::star::frame">ContentHandlerFactory</type></li>
                </ul>
                uses this schema.

    @attention  This class is not threadsafe implemented. Because it's not necessary.
                But you have to make sure, that it's not used as such :-)
 */
class QueryTokenizer : public ::std::hash_map< ::rtl::OUString                    ,
                                               ::rtl::OUString                    ,
                                               ::rtl::OUStringHash                ,
                                               ::std::equal_to< ::rtl::OUString > >
{
    //-------------------------------------------
    // member

    private:

        /** @short  Because the given query can contain errors,
                    it should be checked outside.

            TODO    Maybe it's a good idea to describe the real problem
                    more detailed ...
         */
        sal_Bool m_bValid;

    //-------------------------------------------
    // interface

    public:

        /** @short  create a new tokenizer instance with a
                    a new query.

            @descr  The given query is immediately analyzed
                    and separated into its token, which can
                    be accessed by some specialized method later.

            @param  sQuery
                    the query string.
         */
        QueryTokenizer(const ::rtl::OUString& sQuery);

        //---------------------------------------

        /** @short  destruct an instance of this class.
         */
        virtual ~QueryTokenizer();

        //---------------------------------------

        /** @short  can be used to check if analyzing of given query
                    was successfully or not.
         */
        virtual sal_Bool valid() const;
};

    } // namespace config
} // namespace filter

#endif // _FILTER_CONFIG_QUERYTOKENIZER_HXX_
