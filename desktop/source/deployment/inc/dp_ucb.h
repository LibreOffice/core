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

#include <vector>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include "dp_misc_api.hxx"
#include <ucbhelper/content.hxx>

namespace ucbhelper
{
class Content;
}

namespace dp_misc {

struct DESKTOP_DEPLOYMENTMISC_DLLPUBLIC StrTitle
{
    static css::uno::Sequence< OUString > getTitleSequence()
    {
        css::uno::Sequence<OUString> aSeq { u"Title"_ustr };
        return aSeq;
    }
    static OUString getTitle( ::ucbhelper::Content &rContent )
    {
        return rContent.getPropertyValue(u"Title"_ustr).get<OUString>();
    }
    // just return titles - the ucbhelper should have a simpler API for this [!]
    static css::uno::Reference< css::sdbc::XResultSet >
        createCursor( ::ucbhelper::Content &rContent,
                      ucbhelper::ResultSetInclude eInclude )
    {
        return rContent.createCursor( StrTitle::getTitleSequence(), eInclude );
    }
};


DESKTOP_DEPLOYMENTMISC_DLLPUBLIC bool create_ucb_content(
    ::ucbhelper::Content * ucb_content,
    OUString const & url,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    bool throw_exc = true );


/** @return true if previously non-existing folder has been created
 */
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC bool create_folder(
    ::ucbhelper::Content * ucb_content,
    OUString const & url,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    bool throw_exc = true );


DESKTOP_DEPLOYMENTMISC_DLLPUBLIC bool erase_path(
    OUString const & url,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    bool throw_exc = true );


DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
std::vector<sal_Int8> readFile( ::ucbhelper::Content & ucb_content );


DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
bool readLine( OUString * res, std::u16string_view startingWith,
               ::ucbhelper::Content & ucb_content, rtl_TextEncoding textenc );

DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
bool readProperties( std::vector< std::pair< OUString, OUString> > & out_result,
                ::ucbhelper::Content & ucb_content);



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
