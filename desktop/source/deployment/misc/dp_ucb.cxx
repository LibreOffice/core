/*************************************************************************
 *
 *  $RCSfile: dp_ucb.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:09:14 $
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

//==============================================================================
bool create_ucb_content(
    ::ucb::Content * ret_ucbContent, OUString const & url,
    Reference<XCommandEnvironment> const & xCmdEnv,
    bool throw_exc )
{
    try {
        // dilemma: no chance to use the given iahandler here, because it would
        //          raise no such file dialogs, else no interaction for
        //          passwords, ...? xxx todo
        ::ucb::Content ucbContent( url, Reference<XCommandEnvironment>() );
        if (! ucbContent.isFolder())
            ucbContent.openStream()->closeInput();
        if (ret_ucbContent != 0)
            *ret_ucbContent = ::ucb::Content( url, xCmdEnv );
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
    ::ucb::Content * ret_ucb_content, OUString const & url_,
    Reference<XCommandEnvironment> const & xCmdEnv, bool throw_exc )
{
    ::ucb::Content ucb_content;
    if (create_ucb_content(
            &ucb_content, url_, xCmdEnv, false /* no throw */ ))
    {
        if (ucb_content.isFolder())
        {
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
        url = expand_url( url );
        slash = url.lastIndexOf( '/' );
    }
    ::ucb::Content parentContent;
    if (! create_folder(
            &parentContent, url.copy( 0, slash ), xCmdEnv, throw_exc ))
        return false;
    Reference<XContentCreator> xCreator( parentContent.get(), UNO_QUERY );
    if (xCreator.is())
    {
        OUString strTitle( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );
        Any title( makeAny( ::rtl::Uri::decode( url.copy( slash + 1 ),
                                                rtl_UriDecodeWithCharset,
                                                RTL_TEXTENCODING_UTF8 ) ) );

        Sequence<ContentInfo> infos( xCreator->queryCreatableContentsInfo() );
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
                            Sequence<OUString>( &strTitle, 1 ),
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
    ::ucb::Content ucb_content;
    if (create_ucb_content( &ucb_content, url, xCmdEnv, false /* no throw */ ))
    {
        try {
            ucb_content.executeCommand(
                OUSTR("delete"), makeAny( true /* delete physically */ ) );
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
::rtl::ByteSequence readFile( ::ucb::Content & ucb_content )
{
    ::rtl::ByteSequence bytes;
    Reference<io::XOutputStream> xStream(
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
