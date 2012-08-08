/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
