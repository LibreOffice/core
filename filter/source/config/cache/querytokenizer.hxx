/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _FILTER_CONFIG_QUERYTOKENIZER_HXX_
#define _FILTER_CONFIG_QUERYTOKENIZER_HXX_

#include <boost/unordered_map.hpp>
#include <rtl/ustring.hxx>


namespace filter{
    namespace config{


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

    @attention  This class is not threadsafe implemented. Because its not neccessary.
                But you have to make shure, that ist not used as such :-)
 */
class QueryTokenizer : public ::boost::unordered_map< ::rtl::OUString                    ,
                                               ::rtl::OUString                    ,
                                               ::rtl::OUStringHash                ,
                                               ::std::equal_to< ::rtl::OUString > >
{
    //-------------------------------------------
    // member

    private:

        /** @short  Because the given query can contain errors,
                    it should be checked outside.

            TODO    May its a good idea to describe the real problem
                    more detailed ...
         */
        sal_Bool m_bValid;

    //-------------------------------------------
    // interface

    public:

        /** @short  create a new tokenizer instance with a
                    a new query.

            @descr  The given query is immidiatly analyzed
                    and seperated into its token, which can
                    be access by some specialized method later.

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
