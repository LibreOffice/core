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


#include "deployment.hrc"
#include "dp_misc.h"
#include "dp_ucb.h"
#include "rtl/uri.hxx"
#include "rtl/ustrbuf.hxx"
#include "ucbhelper/content.hxx"
#include "xmlscript/xml_helper.hxx"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/ucb/CommandFailedException.hpp"
#include "com/sun/star/ucb/ContentInfo.hpp"
#include "com/sun/star/ucb/ContentInfoAttribute.hpp"
#include "comphelper/processfactory.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_misc
{

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
            url, Reference<XCommandEnvironment>(),
            comphelper::getProcessComponentContext() );

        ucbContent.isFolder();

        if (ret_ucbContent != 0)
        {
            ucbContent.setCommandEnvironment( xCmdEnv );
            *ret_ucbContent = ucbContent;
        }
        return true;
    }
    catch (const RuntimeException &) {
        throw;
    }
    catch (const Exception &) {
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
    const Any title( ::rtl::Uri::decode( url.copy( slash + 1 ),
                                         rtl_UriDecodeWithCharset,
                                         RTL_TEXTENCODING_UTF8 ) );
    const Sequence<ContentInfo> infos(
        parentContent.queryCreatableContentsInfo() );
    for ( sal_Int32 pos = 0; pos < infos.getLength(); ++pos )
    {
        // look KIND_FOLDER:
        ContentInfo const & info = infos[ pos ];
        if ((info.Attributes & ContentInfoAttribute::KIND_FOLDER) != 0)
        {
            // make sure the only required bootstrap property is "Title":
            Sequence<beans::Property> const & rProps = info.Properties;
            if ( rProps.getLength() != 1 || rProps[ 0 ].Name != "Title" )
                continue;

            try {
                if (parentContent.insertNewContent(
                        info.Type,
                        StrTitle::getTitleSequence(),
                        Sequence<Any>( &title, 1 ),
                        ucb_content )) {
                    if (ret_ucb_content != 0)
                        *ret_ucb_content = ucb_content;
                    return true;
                }
            }
            catch (const RuntimeException &) {
                throw;
            }
            catch (const CommandFailedException &) {
                // Interaction Handler already handled the error
                // that has occurred...
            }
            catch (const Exception &) {
                if (throw_exc)
                    throw;
                return false;
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
        catch (const RuntimeException &) {
            throw;
        }
        catch (const Exception &) {
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

bool readProperties( ::std::list< ::std::pair< ::rtl::OUString, ::rtl::OUString> > & out_result,
                     ::ucbhelper::Content & ucb_content )
{
    // read whole file:
    ::rtl::ByteSequence bytes( readFile( ucb_content ) );
    OUString file( reinterpret_cast<sal_Char const *>(bytes.getConstArray()),
                   bytes.getLength(), RTL_TEXTENCODING_UTF8);
    sal_Int32 pos = 0;

    for (;;)
    {

        ::rtl::OUStringBuffer buf;
        sal_Int32 start = pos;

        bool bEOF = false;
        pos = file.indexOf( LF, pos );
        if (pos < 0) { // EOF
            buf.append( file.copy( start ) );
            bEOF = true;
        }
        else
        {
            if (pos > 0 && file[ pos - 1 ] == CR)
                // consume extra CR
                buf.append( file.copy( start, pos - start - 1 ) );
            else
                buf.append( file.copy( start, pos - start ) );
            pos++;
        }
        OUString aLine = buf.makeStringAndClear();

        sal_Int32 posEqual = aLine.indexOf('=');
        if (posEqual > 0 && (posEqual + 1) <  aLine.getLength())
        {
            OUString name = aLine.copy(0, posEqual);
            OUString value = aLine.copy(posEqual + 1);
            out_result.push_back(::std::make_pair(name, value));
        }

        if (bEOF)
            break;
    }
    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
