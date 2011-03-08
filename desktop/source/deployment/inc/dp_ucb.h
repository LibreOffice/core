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

#if ! defined INCLUDED_DP_UCB_H
#define INCLUDED_DP_UCB_H

#include <list>
#include "rtl/byteseq.hxx"
#include "rtl/instance.hxx"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "dp_misc_api.hxx"

namespace ucbhelper
{
class Content;
}

namespace css = ::com::sun::star;

namespace dp_misc {

struct DESKTOP_DEPLOYMENTMISC_DLLPUBLIC StrTitle :
    public rtl::StaticWithInit<const rtl::OUString, StrTitle>
{
    const rtl::OUString operator () ();
};

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC bool create_ucb_content(
    ::ucbhelper::Content * ucb_content,
    ::rtl::OUString const & url,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    bool throw_exc = true );

//==============================================================================
/** @return true if previously non-existing folder has been created
 */
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC bool create_folder(
    ::ucbhelper::Content * ucb_content,
    ::rtl::OUString const & url,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    bool throw_exc = true );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC bool erase_path(
    ::rtl::OUString const & url,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    bool throw_exc = true );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
::rtl::ByteSequence readFile( ::ucbhelper::Content & ucb_content );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
bool readLine( ::rtl::OUString * res, ::rtl::OUString const & startingWith,
               ::ucbhelper::Content & ucb_content, rtl_TextEncoding textenc );

DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
bool readProperties( ::std::list< ::std::pair< ::rtl::OUString, ::rtl::OUString> > & out_result,
                ::ucbhelper::Content & ucb_content);



}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
