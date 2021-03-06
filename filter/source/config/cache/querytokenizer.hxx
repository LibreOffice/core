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

#pragma once

#include <rtl/ustring.hxx>
#include <unordered_map>


namespace filter::config {


/** @short      It can be used to split any query string (which can be used at the
                related interface <type scope="css::container">XContainerQuery</type>)
                into its different tokens using a fix schema.

    @descr      All queries implemented of the services
                <ul>
                    <li><type scope="css::document">TypeDetection</type></li>
                    <li><type scope="css::document">FilterFactory</type></li>
                    <li><type scope="css::document">ExtendedTypeDetectionFactory</type></li>
                    <li><type scope="css::frame">FrameLoaderFactory</type></li>
                    <li><type scope="css::frame">ContentHandlerFactory</type></li>
                </ul>
                uses this schema.

    @attention  This class is not threadsafe implemented. Because it's not necessary.
                But you have to make sure that it's not used as such :-)
 */
class QueryTokenizer : public std::unordered_map< OUString, OUString >
{

    // member

    private:

        /** @short  Because the given query can contain errors,
                    it should be checked outside.

            TODO    May it's a good idea to describe the real problem
                    more detailed ...
         */
        bool m_bValid;


    // interface

    public:

        /** @short  create a new tokenizer instance with a
                    a new query.

            @descr  The given query is immediately analyzed
                    and separated into its token, which can
                    be access by some specialized method later.

            @param  sQuery
                    the query string.
         */
        explicit QueryTokenizer(const OUString& sQuery);


        /** @short  destruct an instance of this class.
         */
        virtual ~QueryTokenizer();


        /** @short  can be used to check if analyzing of given query
                    was successful or not.
         */
        bool valid() const;
};

} // namespace filter::config

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
