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

#ifndef OOX_VML_VMLINPUTSTREAM_HXX
#define OOX_VML_VMLINPUTSTREAM_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase1.hxx>
#include <rtl/string.hxx>

namespace com { namespace sun { namespace star {
    namespace io { class XTextInputStream; }
    namespace uno { class XComponentContext; }
} } }

namespace oox {
namespace vml {

// ============================================================================

typedef ::cppu::WeakImplHelper1< ::com::sun::star::io::XInputStream > InputStream_BASE;

/** An input stream class for VML streams, implementing the UNO interface
    com.sun.star.io.XInputStream needed by the Expat XML parsers.

    This stream reads the data from the input stream passed to the constructor,
    and parses all XML elements for features unsupported by the current Expat
    XML parser:

    1)  All elements that have the form '<![inst]>' where 'inst' is any string
        not containing the characters '<' and '>' are stripped from the input
        stream.

    2)  Multiple occurences of the same attribute in an element but the last
        are removed.

    3)  Line breaks represented by a single <br> element (without matching
        </br> element) are replaced by a literal LF character.
 */
class InputStream : public InputStream_BASE
{
public:
    explicit            InputStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStrm );
    virtual             ~InputStream();

    virtual sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& rData, sal_Int32 nBytesToRead )
                        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& rData, sal_Int32 nMaxBytesToRead )
                        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
                        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL available()
                        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeInput()
                        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

private:
    void                updateBuffer() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    ::rtl::OString      readToElementBegin() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    ::rtl::OString      readToElementEnd() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XTextInputStream >
                        mxTextStrm;
    ::com::sun::star::uno::Sequence< sal_Unicode > maOpeningBracket;
    ::com::sun::star::uno::Sequence< sal_Unicode > maClosingBracket;
    const ::rtl::OString maOpeningCData;
    const ::rtl::OString maClosingCData;
    ::rtl::OString      maBuffer;
    sal_Int32           mnBufferPos;
};

// ============================================================================

} // namespace vml
} // namespace oox

#endif
