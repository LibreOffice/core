/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pptxdumper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:41:43 $
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

