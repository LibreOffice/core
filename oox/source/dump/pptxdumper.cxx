/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pptxdumper.cxx,v $
 * $Revision: 1.3 $
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

#include "oox/dump/pptxdumper.hxx"

#if OOX_INCLUDE_DUMPER

using ::rtl::OUString;
using ::oox::core::FilterBase;

namespace oox {
namespace dump {
namespace pptx {

// ============================================================================

RootStorageObject::RootStorageObject( const DumperBase& rParent )
{
    RootStorageObjectBase::construct( rParent );
}

void RootStorageObject::implDumpStream( BinaryInputStreamRef xStrm, const OUString& /*rStrgPath*/, const OUString& rStrmName, const OUString& rSystemFileName )
{
    OUString aExt = InputOutputHelper::getFileNameExtension( rStrmName );
    if( aExt.equalsIgnoreAsciiCaseAscii( "xml" ) ||
        aExt.equalsIgnoreAsciiCaseAscii( "vml" ) ||
        aExt.equalsIgnoreAsciiCaseAscii( "rels" ) )
    {
        XmlStreamObject( *this, rSystemFileName, xStrm ).dump();
    }
}

// ============================================================================

Dumper::Dumper( const FilterBase& rFilter )
{
    ConfigRef xCfg( new Config( "OOO_PPTXDUMPER" ) );
    DumperBase::construct( rFilter, xCfg );
}

void Dumper::implDump()
{
    RootStorageObject( *this ).dump();
}

// ============================================================================

} // namespace pptx
} // namespace dump
} // namespace oox

#endif

