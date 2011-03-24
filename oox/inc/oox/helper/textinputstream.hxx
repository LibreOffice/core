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

#ifndef OOX_HELPER_TEXTINPUTSTREAM_HXX
#define OOX_HELPER_TEXTINPUTSTREAM_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star {
    namespace io { class XInputStream; }
    namespace io { class XTextInputStream; }
    namespace uno { class XComponentContext; }
} } }

namespace oox {

class BinaryInputStream;

// ============================================================================

class TextInputStream
{
public:
    explicit            TextInputStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStrm,
                            rtl_TextEncoding eTextEnc );

    explicit            TextInputStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            BinaryInputStream& rInStrm,
                            rtl_TextEncoding eTextEnc );

                        ~TextInputStream();

    /** Returns true, if no more text is available in the stream.
     */
    bool                isEof() const;

    /** Reads a text line from the stream.

        If the last line in the stream is not terminated with line-end
        character(s), the stream will immediately go into EOF state and return
        the text line. Otherwise, if the last character in the stream is a
        line-end character, the next call to this function will turn the stream
        into EOF state and return an empty string.
     */
    ::rtl::OUString     readLine();

    /** Reads a text portion from the stream until the specified character is
        found.

        If the end of the stream is not terminated with the specified
        character, the stream will immediately go into EOF state and return the
        remaining text portion. Otherwise, if the last character in the stream
        is the specified character (and caller specifies to read and return it,
        see parameter bIncludeChar), the next call to this function will turn
        the stream into EOF state and return an empty string.

        @param cChar
            The separator character to be read to.

        @param bIncludeChar
            True = if found, the specified character will be read from stream
                and included in the returned string.
            False = the specified character will neither be read from the
                stream nor included in the returned string, but will be
                returned as first character in the next call of this function
                or readLine().
     */
    ::rtl::OUString     readToChar( sal_Unicode cChar, bool bIncludeChar );

    // ------------------------------------------------------------------------

    /** Creates a UNO text input stream object from the passed UNO input stream.
     */
    static ::com::sun::star::uno::Reference< ::com::sun::star::io::XTextInputStream >
                        createXTextInputStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStrm,
                            rtl_TextEncoding eTextEnc );

    // ------------------------------------------------------------------------
private:
    void                init(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStrm,
                            rtl_TextEncoding eTextEnc );

    /** Adds the pending character in front of the passed string, if existing. */
    ::rtl::OUString     createFinalString( const ::rtl::OUString& rString );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XTextInputStream >
                        mxTextStrm;
    sal_Unicode         mcPendingChar;
};

// ============================================================================

} // namespace oox

#endif
