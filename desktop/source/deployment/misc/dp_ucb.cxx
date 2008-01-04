/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_ucb.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-04 14:34:05 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_misc.hrc"
#include "dp_misc.h"
#include "dp_ucb.h"
#include "rtl/uri.hxx"
#include "rtl/ustrbuf.hxx"
#include "ucbhelper/content.hxx"
#include "xmlscript/xml_helper.hxx"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/ucb/CommandFailedException.hpp"
#include "com/sun/star/ucb/XContentCreator.hpp"
#include "com/sun/star/ucb/ContentInfo.hpp"
#include "com/sun/star/ucb/ContentInfoAttribute.hpp"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_misc
{

const OUString StrTitle::operator () ()
{
    return OUSTR("Title");
}

//==============================================================================
bool create_ucb_content(
    ::ucbhelper::Content * ret_ucbContent, OUString const & url,
    Reference<XCommandEnvironment> const & xCmdEnv,
    bool throw_exc )
{
    try {
        // Existense check...
        // content ctor/isFolder() will throw exception in case the resource
        // does not exist.

        // dilemma: no chance to use the given iahandler here, because it would
        //          raise no such file dialogs, else no interaction for
        //          passwords, ...? xxx todo
        ::ucbhelper::Content ucbContent(
            url, Reference<XCommandEnvironment>() );

        ucbContent.isFolder();

        if (ret_ucbContent != 0)
        {
            ucbContent.setCommandEnvironment( xCmdEnv );
            *ret_ucbContent = ucbContent;
        }
        return true;
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (Exception &) {
        if (throw_exc)
            throw;
    }
    return false;
}

//==============================================================================
bool create_folder(
    ::ucbhelper::Content * ret_ucb_content, OUString const & url_,
    Reference<XCommandEnvironment> const & xCmdEnv, bool throw_exc )
{
    ::ucbhelper::Content ucb_content;
    if (create_ucb_content(
            &ucb_content, url_, xCmdEnv, false /* no throw */ ))
    {
        if (ucb_content.isFolder()) {
            if (ret_ucb_content != 0)
                *ret_ucb_content = ucb_content;
            return true;
        }
    }

    OUString url( url_ );
    // xxx todo: find parent
    sal_Int32 slash = url.lastIndexOf( '/' );
    if (slash < 0) {
        // fallback:
        url = expandUnoRcUrl( url );
        slash = url.lastIndexOf( '/' );
    }
    if (slash < 0) {
        // invalid: has to be at least "auth:/..."
        if (throw_exc)
            throw ContentCreationException(
                OUSTR("Cannot create folder (invalid path): ") + url,
                Reference<XInterface>(), ContentCreationError_UNKNOWN );
        return false;
    }
    ::ucbhelper::Content parentContent;
    if (! create_folder(
            &parentContent, url.copy( 0, slash ), xCmdEnv, throw_exc ))
        return false;
    Reference<XContentCreator> xCreator( parentContent.get(), UNO_QUERY );
    if (xCreator.is())
    {
        const Any title( ::rtl::Uri::decode( url.copy( slash + 1 ),
                                             rtl_UriDecodeWithCharset,
                                             RTL_TEXTENCODING_UTF8 ) );
        const Sequence<ContentInfo> infos(
            xCreator->queryCreatableContentsInfo() );
        for ( sal_Int32 pos = 0; pos < infos.getLength(); ++pos )
        {
            // look KIND_FOLDER:
            ContentInfo const & info = infos[ pos ];
            if ((info.Attributes & ContentInfoAttribute::KIND_FOLDER) != 0)
            {
                // make sure the only required bootstrap property is "Title":
                Sequence<beans::Property> const & rProps = info.Properties;
                if (rProps.getLength() != 1 ||
                    !rProps[ 0 ].Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM("Title") ))
                    continue;

                try {
                    if (parentContent.insertNewContent(
                            info.Type,
                            Sequence<OUString>( &StrTitle::get(), 1 ),
                            Sequence<Any>( &title, 1 ),
                            ucb_content )) {
                        if (ret_ucb_content != 0)
                            *ret_ucb_content = ucb_content;
                        return true;
                    }
                }
                catch (RuntimeException &) {
                    throw;
                }
                catch (CommandFailedException &) {
                    // Interaction Handler already handled the error
                    // that has occured...
                }
                catch (Exception &) {
                    if (throw_exc)
                        throw;
                    return false;
                }
            }
        }
    }
    if (throw_exc)
        throw ContentCreationException(
            OUSTR("Cannot create folder: ") + url,
            Reference<XInterface>(), ContentCreationError_UNKNOWN );
    return false;
}

//==============================================================================
bool erase_path( OUString const & url,
                 Reference<XCommandEnvironment> const & xCmdEnv,
                 bool throw_exc )
{
    ::ucbhelper::Content ucb_content;
    if (create_ucb_content( &ucb_content, url, xCmdEnv, false /* no throw */ ))
    {
        try {
            ucb_content.executeCommand(
                OUSTR("delete"), Any( true /* delete physically */ ) );
        }
        catch (RuntimeException &) {
            throw;
        }
        catch (Exception &) {
            if (throw_exc)
                throw;
            return false;
        }
    }
    return true;
}

//==============================================================================
::rtl::ByteSequence readFile( ::ucbhelper::Content & ucb_content )
{
    ::rtl::ByteSequence bytes;
    Reference<io::XOutputStream> xStream(
        ::xmlscript::createOutputStream( &bytes ) );
    if (! ucb_content.openStream( xStream ))
        throw RuntimeException(
            OUSTR(
                "::ucbhelper::Content::openStream( XOutputStream ) failed!"),
            0 );
    return bytes;
}

//==============================================================================
bool readLine( OUString * res, OUString const & startingWith,
               ::ucbhelper::Content & ucb_content, rtl_TextEncoding textenc )
{
    // read whole file:
    ::rtl::ByteSequence bytes( readFile( ucb_content ) );
    OUString file( reinterpret_cast<sal_Char const *>(bytes.getConstArray()),
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
                pos = file.indexOf( LF, pos );
                if (pos < 0) { // EOF
                    buf.append( file.copy( start ) );
                }
                else
                {
                    if (pos > 0 && file[ pos - 1 ] == CR)
                    {
                        // consume extra CR
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
                        buf.append( static_cast<sal_Unicode>(' ') );
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
        sal_Int32 next_lf = file.indexOf( LF, pos );
        if (next_lf < 0) // EOF
            break;
        pos = next_lf + 1;
    }
    return false;
}

}
