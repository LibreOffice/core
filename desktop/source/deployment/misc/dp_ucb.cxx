/*************************************************************************
 *
 *  $RCSfile: dp_ucb.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-13 12:07:31 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "dp_misc.h"
#include "dp_ucb.h"
#include "rtl/uri.hxx"
#include "rtl/ustrbuf.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/implbase1.hxx"
#include "ucbhelper/content.hxx"
#include "xmlscript/xml_helper.hxx"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/ucb/CommandFailedException.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_misc
{

template< typename iface >
class InteractionContinuationImpl : public ::cppu::WeakImplHelper1< iface >
{
    bool * m_pselect;

public:
    inline InteractionContinuationImpl( bool * pselect )
        : m_pselect( pselect )
        {}

    // XInteractionContinuation
    inline virtual void SAL_CALL select() throw (RuntimeException)
        { *m_pselect = true; }
};

//==============================================================================
class InteractionRequestImpl
    : public ::cppu::WeakImplHelper1< task::XInteractionRequest >
{
    Any m_request;
    Sequence< Reference< task::XInteractionContinuation > > m_conts;

public:
    inline InteractionRequestImpl(
        Any const & request,
        Sequence< Reference< task::XInteractionContinuation > > const & conts )
        : m_request( request ),
          m_conts( conts )
        {}

    // XInteractionRequest
    virtual Any SAL_CALL getRequest() throw (RuntimeException);
    virtual Sequence< Reference< task::XInteractionContinuation > > SAL_CALL
    getContinuations() throw (RuntimeException);
};

// XInteractionRequest
//______________________________________________________________________________
Any InteractionRequestImpl::getRequest() throw (RuntimeException)
{
    return m_request;
}

//______________________________________________________________________________
Sequence< Reference< task::XInteractionContinuation > >
InteractionRequestImpl::getContinuations() throw (RuntimeException)
{
    return m_conts;
}

/** @returns true if abort is wanted */
static bool interact_error_nothrow(
    Any const & exc,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    OSL_ASSERT( exc.getValueTypeClass() == TypeClass_EXCEPTION );
    bool abort = true; // default
    if (xCmdEnv.is())
    {
        Reference< task::XInteractionHandler > xInteractionHandler(
            xCmdEnv->getInteractionHandler() );
        if (xInteractionHandler.is())
        {
            bool abort_ = false;
            Sequence< Reference< task::XInteractionContinuation > > conts( 1 );
            conts[ 0 ] = new InteractionContinuationImpl<
                task::XInteractionAbort >( &abort_ );
            xInteractionHandler->handle(
                new InteractionRequestImpl( exc, conts ) );
            abort = abort_;
        }
    }
    return abort;
}

static void log_error( Reference< XCommandEnvironment > const & xCmdEnv,
                       Any const & exc )
{
    if (xCmdEnv.is())
    {
        Reference< XProgressHandler > xProgressHandler(
            xCmdEnv->getProgressHandler() );
        if (xProgressHandler.is())
            xProgressHandler->update( exc );
    }
}

//==============================================================================
void interact_error(
    Any const & exc,
    Reference< XCommandEnvironment > const & xCmdEnv, bool log )
{
    if (log)
        log_error( xCmdEnv, exc );
    if (interact_error_nothrow( exc, xCmdEnv ))
        ::cppu::throwException( exc );
}

//==============================================================================
void handle_error(
    deployment::DeploymentException const & exc,
    Reference< XCommandEnvironment > const & xCmdEnv, bool log )
{
    OSL_ASSERT( exc.Cause.getValueTypeClass() == TypeClass_EXCEPTION );

    // don't handle RuntimeExceptions, rethrow them:
    RuntimeException rt_exc;
    if (exc.Cause >>= rt_exc)
    {
        OSL_ENSURE( 0, "### missing RuntimeException rethrow?" );
        ::cppu::throwException( exc.Cause );
    }
    // don't handle DeploymentExceptions, rethrow them:
    deployment::DeploymentException depl_exc;
    if (exc.Cause >>= depl_exc)
    {
        OSL_ENSURE( 0, "### missing DeploymentException rethrow?" );
        ::cppu::throwException( exc.Cause );
    }

    // unwrap already negotiated (handled) CommandFailedExceptions:
    CommandFailedException cf_exc;
    if (exc.Cause >>= cf_exc)
    {
        throw deployment::DeploymentException(
            exc.Message, exc.Context, cf_exc.Reason );
    }

    if (log)
        log_error( xCmdEnv, makeAny(exc) );
    if (interact_error_nothrow( exc.Cause, xCmdEnv ))
        throw exc;
}

//==============================================================================
bool create_ucb_content(
    ::ucb::Content * ret_ucb_content, OUString const & url,
    Reference< XCommandEnvironment > const & xCmdEnv,
    bool throw_exc )
{
    bool success = false;
    ::ucb::Content ucb_content;
    if (::ucb::Content::create( url, Reference< XCommandEnvironment >(),
                                ucb_content ))
    {
        try
        {
            success = ucb_content.isFolder();
            if (! success)
            {
                Reference< io::XInputStream > xStream(
                    ucb_content.openStream() );
                if (xStream.is())
                {
                    xStream->closeInput();
                    success = true;
                }
            }
        }
        catch (RuntimeException &)
        {
            throw;
        }
        catch (Exception &)
        {
        }
    }

    if (success)
    {
        if (ret_ucb_content != 0)
        {
            if (xCmdEnv.is())
                *ret_ucb_content = ::ucb::Content( url, xCmdEnv );
            else
                *ret_ucb_content = ucb_content;
        }
    }
    else if (throw_exc)
    {
        throw ContentCreationException(
            OUSTR("cannot open ") + url, Reference< XInterface >(),
            ContentCreationError_UNKNOWN );
    }

    return success;
}

//==============================================================================
bool create_folder(
    ::ucb::Content * ret_ucb_content, OUString const & url,
    Reference< XCommandEnvironment > const & xCmdEnv,
    bool throw_exc )
{
    ::ucb::Content ucb_content;
    if (create_ucb_content( &ucb_content, url, xCmdEnv, false /* no throw */ ))
    {
        if (ucb_content.isFolder())
        {
            if (ret_ucb_content != 0)
                *ret_ucb_content = ucb_content;
            return true;
        }
        else if (throw_exc)
            throw ContentCreationException(
                OUSTR("there is already an item which is no folder: ") +
                url, Reference< XInterface >(), ContentCreationError_UNKNOWN );
        return false;
    }

    // xxx todo: find parent
    sal_Int32 slash = url.lastIndexOf( '/' );
    if (slash < 0)
    {
        if (throw_exc)
            throw ContentCreationException(
                OUSTR("cannot determine parent folder URL of ") + url,
                Reference< XInterface >(), ContentCreationError_UNKNOWN );
        return false;
    }
    ::ucb::Content parent_content;
    if (! create_folder(
            &parent_content, url.copy( 0, slash ), xCmdEnv, throw_exc ))
        return false;

    OUString str_title( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );
    Any value( makeAny( ::rtl::Uri::decode( url.copy( slash + 1 ),
                                            rtl_UriDecodeWithCharset,
                                            RTL_TEXTENCODING_UTF8 ) ) );
    if (parent_content.insertNewContent(
            // xxx todo: currently file UCP specific
            OUSTR("application/vnd.sun.staroffice.fsys-folder"),
            Sequence< OUString >( &str_title, 1 ),
            Sequence< Any >( &value, 1 ),
            ucb_content ))
    {
        if (ret_ucb_content != 0)
            *ret_ucb_content = ucb_content;
        return true;
    }
    else if (throw_exc)
        throw ContentCreationException(
            OUSTR("inserting new content failed: ") + url,
            Reference< XInterface >(), ContentCreationError_UNKNOWN );
    return false;
}

//==============================================================================
void erase_path(
    OUString const & url, Reference< XCommandEnvironment > const & xCmdEnv )
{
    Reference< XProgressHandler > xProgressHandler;
    if (xCmdEnv.is())
        xProgressHandler = xCmdEnv->getProgressHandler();
    if (xProgressHandler.is())
    {
        xProgressHandler->update(
            makeAny( OUSTR("erasing path ") + url ) );
    }
    ::ucb::Content ucb_content;
    if (create_ucb_content( &ucb_content, url, xCmdEnv, false /* no throw */ ))
    {
        ucb_content.executeCommand(
            OUSTR("delete"), makeAny( true /* delete physically */ ) );
    }
}

//==============================================================================
OUString make_url( OUString const & base_url, OUString const & url )
{
    // xxx todo: wait for SB's api
    ::rtl::OUStringBuffer buf;
    buf.append( base_url );
    if (base_url.getLength() > 0 &&
        base_url[ base_url.getLength() - 1 ] != '/')
    {
        buf.append( static_cast< sal_Unicode >('/') );
    }
    if (url.getLength() > 0 && url[ 0 ] == '/')
        buf.append( url.copy( 1 ) );
    else
        buf.append( url );
    return buf.makeStringAndClear();
}

//==============================================================================
::rtl::ByteSequence readFile( ::ucb::Content & ucb_content )
{
    ::rtl::ByteSequence bytes;
    Reference< io::XOutputStream > xStream(
        ::xmlscript::createOutputStream( &bytes ) );
    if (! ucb_content.openStream( xStream ))
        throw RuntimeException(
            OUSTR("::ucb::Content::openStream( XOutputStream ) failed!"), 0 );
    return bytes;
}

//==============================================================================
bool readLine( OUString * res, OUString const & startingWith,
               ::ucb::Content & ucb_content, rtl_TextEncoding textenc )
{
    // read whole file:
    ::rtl::ByteSequence bytes( readFile( ucb_content ) );
    OUString file( reinterpret_cast< sal_Char const * >(bytes.getConstArray()),
                   bytes.getLength(), textenc );
    sal_Int32 pos = 0;
    for (;;)
    {
        if (file.match( startingWith, pos ))
        {
            ::rtl::OUStringBuffer buf;
            sal_Int32 start = pos;
            pos += startingWith.getLength();
            for (;;)
            {
                pos = file.indexOf( '\n', pos );
                if (pos < 0) // EOF
                {
                    buf.append( file.copy( start ) );
                }
                else
                {
                    if (file[ pos - 1 ] == '\r') // consume extra CR
                    {
                        buf.append( file.copy( start, pos - start - 1 ) );
                        ++pos;
                    }
                    else
                        buf.append( file.copy( start, pos - start ) );
                    ++pos; // consume LF
                    // check next line:
                    if (pos < file.getLength() &&
                        (file[ pos ] == ' ' || file[ pos ] == '\t'))
                    {
                        buf.append( static_cast< sal_Unicode >(' ') );
                        ++pos;
                        start = pos;
                        continue;
                    }
                }
                break;
            }
            *res = buf.makeStringAndClear();
            return true;
        }
        // next line:
        sal_Int32 next_lf = file.indexOf( '\n', pos );
        if (next_lf < 0) // EOF
            break;
        pos = next_lf + 1;
    }
    return false;
}

}
