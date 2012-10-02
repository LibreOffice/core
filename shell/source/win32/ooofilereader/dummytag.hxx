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


#ifndef DUMMYTAG_HXX_INCLUDED
#define DUMMYTAG_HXX_INCLUDED

#include "itag.hxx"

/***************************   dummy tag readers   ***************************/

/** Implements the ITag interface but does
    nothing (Null object pattern), may be used for
    tags we are not interessted in to avoid if-else
    branches.
*/
class CDummyTag : public ITag
{
    public:
        virtual void startTag(){};

        virtual void endTag(){};

        virtual void addCharacters(const std::wstring& /*characters*/){};

        virtual void addAttributes(const XmlTagAttributes_t& /*attributes*/){};

        virtual std::wstring getTagContent( void )
        {
            return EMPTY_STRING;
        };

        virtual ::std::wstring const getTagAttribute( ::std::wstring  const & /*attrname*/ ){ return ::std::wstring(EMPTY_STRING); };
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
