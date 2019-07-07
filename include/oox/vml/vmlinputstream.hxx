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

#ifndef INCLUDED_OOX_VML_VMLINPUTSTREAM_HXX
#define INCLUDED_OOX_VML_VMLINPUTSTREAM_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/string.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace io { class XTextInputStream2; }
    namespace uno { class XComponentContext; }
} } }

namespace oox {
namespace vml {


/** An input stream class for VML streams, implementing the UNO interface
    com.sun.star.io.XInputStream needed by the Expat XML parsers.

    This stream reads the data from the input stream passed to the constructor,
    and parses all XML elements for features unsupported by the current Expat
    XML parser:

    1)  All elements that have the form '<![inst]>' where 'inst' is any string
        not containing the characters '<' and '>' are stripped from the input
        stream.

    2)  Multiple occurrences of the same attribute in an element but the last
        are removed.

    3)  Line breaks represented by a single <br> element (without matching
        </br> element) are replaced by a literal LF character.
 */
class InputStream : public ::cppu::WeakImplHelper< css::io::XInputStream >
{
public:
    explicit            InputStream(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const css::uno::Reference< css::io::XInputStream >& rxInStrm );
    virtual             ~InputStream() override;

    virtual sal_Int32 SAL_CALL readBytes( css::uno::Sequence< sal_Int8 >& rData, sal_Int32 nBytesToRead ) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< sal_Int8 >& rData, sal_Int32 nMaxBytesToRead ) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) override;
    virtual sal_Int32 SAL_CALL available() override;
    virtual void SAL_CALL closeInput() override;

private:
    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    void                updateBuffer();
    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    OString      readToElementBegin();
    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    OString      readToElementEnd();

private:
    css::uno::Reference< css::io::XTextInputStream2 >
                        mxTextStrm;
    css::uno::Sequence< sal_Unicode > maOpeningBracket;
    css::uno::Sequence< sal_Unicode > maClosingBracket;
    OString             maBuffer;
    sal_Int32           mnBufferPos;
};


} // namespace vml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
