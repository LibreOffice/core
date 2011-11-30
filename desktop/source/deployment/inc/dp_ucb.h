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
