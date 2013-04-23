/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef OOX_VML_VMLINPUTSTREAM_HXX
#define OOX_VML_VMLINPUTSTREAM_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase1.hxx>
#include <rtl/string.hxx>

namespace com { namespace sun { namespace star {
    namespace io { class XTextInputStream2; }
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
    OString      readToElementBegin() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    OString      readToElementEnd() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XTextInputStream2 >
                        mxTextStrm;
    ::com::sun::star::uno::Sequence< sal_Unicode > maOpeningBracket;
    ::com::sun::star::uno::Sequence< sal_Unicode > maClosingBracket;
    const OString maOpeningCData;
    const OString maClosingCData;
    OString      maBuffer;
    sal_Int32           mnBufferPos;
};

// ============================================================================

} // namespace vml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
