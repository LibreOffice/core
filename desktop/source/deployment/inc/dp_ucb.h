/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_ucb.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:19:41 $
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


namespace ucb
{
class Content;
}

namespace css = ::com::sun::star;

namespace dp_misc {

struct StrTitle : public rtl::StaticWithInit<const rtl::OUString, StrTitle> {
    const rtl::OUString operator () ();
};

//==============================================================================
bool create_ucb_content(
    ::ucb::Content * ucb_content,
    ::rtl::OUString const & url,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    bool throw_exc = true );

//==============================================================================
/** @return true if previously non-existing folder has been created
 */
bool create_folder(
    ::ucb::Content * ucb_content,
    ::rtl::OUString const & url,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    bool throw_exc = true );

//==============================================================================
bool erase_path(
    ::rtl::OUString const & url,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    bool throw_exc = true );

//==============================================================================
::rtl::ByteSequence readFile( ::ucb::Content & ucb_content );

//==============================================================================
bool readLine( ::rtl::OUString * res, ::rtl::OUString const & startingWith,
               ::ucb::Content & ucb_content, rtl_TextEncoding textenc );

}

#endif
