/*************************************************************************
 *
 *  $RCSfile: dp_ucb.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-13 12:06:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if ! defined INCLUDED_DP_UCB_H
#define INCLUDED_DP_UCB_H

#include "rtl/byteseq.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/deployment/DeploymentException.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"


namespace ucb
{
class Content;
}

namespace css = ::com::sun::star;

namespace dp_misc
{

//==============================================================================
class ProgressLevel
{
    css::uno::Reference< css::ucb::XProgressHandler > m_xProgressHandler;

public:
    inline ProgressLevel(
        css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv );
    inline ProgressLevel(
        css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv,
        ::rtl::OUString const & status );
    inline ~ProgressLevel();

    inline void update( ::rtl::OUString const & status );
};

//______________________________________________________________________________
inline ProgressLevel::ProgressLevel(
    css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv )
{
    if (xCmdEnv.is())
        m_xProgressHandler = xCmdEnv->getProgressHandler();
    if (m_xProgressHandler.is())
        m_xProgressHandler->push( css::uno::Any() );
}

//______________________________________________________________________________
inline ProgressLevel::ProgressLevel(
    css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv,
    ::rtl::OUString const & status )
{
    if (xCmdEnv.is())
        m_xProgressHandler = xCmdEnv->getProgressHandler();
    if (m_xProgressHandler.is())
        m_xProgressHandler->push( css::uno::makeAny(status) );
}

//______________________________________________________________________________
inline ProgressLevel::~ProgressLevel()
{
    if (m_xProgressHandler.is())
        m_xProgressHandler->pop();
}

//______________________________________________________________________________
inline void ProgressLevel::update( ::rtl::OUString const & status )
{
    if (m_xProgressHandler.is())
        m_xProgressHandler->update( css::uno::makeAny(status) );
}

//==============================================================================
void handle_error(
    css::deployment::DeploymentException const & exc,
    css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv,
    bool log = true );

//==============================================================================
void interact_error(
    css::uno::Any const & exc,
    css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv,
    bool log = true );


//==============================================================================
bool create_ucb_content(
    ::ucb::Content * ucb_content,
    ::rtl::OUString const & url,
    css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv =
    css::uno::Reference< css::ucb::XCommandEnvironment >(),
    bool throw_exc = true );

//==============================================================================
/** @return true if previously non-existing folder has been created
 */
bool create_folder(
    ::ucb::Content * ucb_content,
    ::rtl::OUString const & url,
    css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv =
    css::uno::Reference< css::ucb::XCommandEnvironment >(),
    bool throw_exc = true );

//==============================================================================
void erase_path(
    ::rtl::OUString const & url,
    css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv =
    css::uno::Reference< css::ucb::XCommandEnvironment >() );

//==============================================================================
::rtl::OUString make_url(
    ::rtl::OUString const & base_url, ::rtl::OUString const & url );

//==============================================================================
::rtl::ByteSequence readFile( ::ucb::Content & ucb_content );

//==============================================================================
bool readLine( ::rtl::OUString * res, ::rtl::OUString const & startingWith,
               ::ucb::Content & ucb_content, rtl_TextEncoding textenc );

}

#endif
