/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_ucb.h,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 14:27:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#if ! defined INCLUDED_DP_UCB_H
#define INCLUDED_DP_UCB_H

#include "rtl/byteseq.hxx"
#include "rtl/instance.hxx"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_MISC_API_HXX
#include "dp_misc_api.hxx"
#endif

namespace ucb
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
    ::ucb::Content * ucb_content,
    ::rtl::OUString const & url,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    bool throw_exc = true );

//==============================================================================
/** @return true if previously non-existing folder has been created
 */
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC bool create_folder(
    ::ucb::Content * ucb_content,
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
::rtl::ByteSequence readFile( ::ucb::Content & ucb_content );

//==============================================================================
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
bool readLine( ::rtl::OUString * res, ::rtl::OUString const & startingWith,
               ::ucb::Content & ucb_content, rtl_TextEncoding textenc );

}

#endif
