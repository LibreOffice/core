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

#include <comphelper/seqstream.hxx>

namespace oox {
namespace vml {

// ============================================================================

struct StreamDataContainer
{
    ::comphelper::ByteSequence maDataSeq;

    explicit            StreamDataContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStrm );
};

// ============================================================================

/** An input stream class for VML streams.

    This stream reads the entire data from the input stream passed to the
    constructor, and parses all XML elements for features unsupported by the
    current Expat parser.

    All elements that have the form '<![inst]>' where 'inst' is any string not
    containing the characters '<' and '>' are stripped from the input stream.
 */
class InputStream : private StreamDataContainer, public ::comphelper::SequenceInputStream
{
public:
    explicit            InputStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStrm );
    virtual             ~InputStream();
};

// ============================================================================

} // namespace vml
} // namespace oox

#endif

