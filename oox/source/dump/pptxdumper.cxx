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

#include "oox/dump/pptxdumper.hxx"
#include "oox/helper/zipstorage.hxx"
#include "oox/ole/olestorage.hxx"
#include "oox/dump/biffdumper.hxx"
#include "oox/dump/oledumper.hxx"
#include "oox/dump/xlsbdumper.hxx"

#if OOX_INCLUDE_DUMPER

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::io::XInputStream;
using ::comphelper::MediaDescriptor;
using ::oox::core::FilterBase;

namespace oox {
namespace dump {
namespace pptx {

// ============================================================================

RootStorageObject::RootStorageObject( const DumperBase& rParent )
{
    StorageObjectBase::construct( rParent );
}

void RootStorageObject::implDumpStream( const BinaryInputStreamRef& rxStrm, const OUString& rStrgPath, const OUString& rStrmName, const OUString& rSysFileName )
{
    OUString aExt = InputOutputHelper::getFileNameExtension( rStrmName );
    Reference< XInputStream > xInStrm = InputOutputHelper::getXInputStream( *rxStrm );
    if( aExt.equalsIgnoreAsciiCaseAscii( "pptx" ) ||
        aExt.equalsIgnoreAsciiCaseAscii( "potx" ) )
    {
        Dumper( getFactory(), xInStrm, rSysFileName ).dump();
    }
    else if(
        aExt.equalsIgnoreAsciiCaseAscii( "xlsb" ) ||
        aExt.equalsIgnoreAsciiCaseAscii( "xlsm" ) ||
        aExt.equalsIgnoreAsciiCaseAscii( "xlsx" ) ||
        aExt.equalsIgnoreAsciiCaseAscii( "xltm" ) ||
        aExt.equalsIgnoreAsciiCaseAscii( "xltx" ) )
    {
        ::oox::dump::xlsb::Dumper( getFactory(), xInStrm, rSysFileName ).dump();
    }
    else if(
        aExt.equalsIgnoreAsciiCaseAscii( "xla" ) ||
        aExt.equalsIgnoreAsciiCaseAscii( "xlc" ) ||
        aExt.equalsIgnoreAsciiCaseAscii( "xlm" ) ||
        aExt.equalsIgnoreAsciiCaseAscii( "xls" ) ||
        aExt.equalsIgnoreAsciiCaseAscii( "xlt" ) ||
        aExt.equalsIgnoreAsciiCaseAscii( "xlw" ) )
    {
        ::oox::dump::biff::Dumper( getFactory(), xInStrm, rSysFileName ).dump();
    }
    else if(
        aExt.equalsIgnoreAsciiCaseAscii( "xml" ) ||
        aExt.equalsIgnoreAsciiCaseAscii( "vml" ) ||
        aExt.equalsIgnoreAsciiCaseAscii( "rels" ) )
    {
        XmlStreamObject( *this, rxStrm, rSysFileName ).dump();
    }
    else if( aExt.equalsIgnoreAsciiCaseAscii( "bin" ) )
    {
        if( rStrgPath.equalsAscii( "ppt" ) && rStrmName.equalsAscii( "vbaProject.bin" ) )
        {
            StorageRef xStrg( new ::oox::ole::OleStorage( getFactory(), xInStrm, false ) );
            VbaProjectStorageObject( *this, xStrg, rSysFileName ).dump();
        }
        else if( rStrgPath.equalsAscii( "ppt/embeddings" ) )
        {
            StorageRef xStrg( new ::oox::ole::OleStorage( getFactory(), xInStrm, false ) );
            OleStorageObject( *this, xStrg, rSysFileName ).dump();
        }
        else if( rStrgPath.equalsAscii( "ppt/activeX" ) )
        {
            StorageRef xStrg( new ::oox::ole::OleStorage( getFactory(), xInStrm, true ) );
            ActiveXStorageObject( *this, xStrg, rSysFileName ).dump();
        }
        else
        {
            BinaryStreamObject( *this, rxStrm, rSysFileName ).dump();
        }
    }
}

// ============================================================================

#define DUMP_PPTX_CONFIG_ENVVAR "OOO_PPTXDUMPER"

Dumper::Dumper( const FilterBase& rFilter )
{
    ConfigRef xCfg( new Config( DUMP_PPTX_CONFIG_ENVVAR, rFilter ) );
    DumperBase::construct( xCfg );
}

Dumper::Dumper( const Reference< XMultiServiceFactory >& rxFactory, const Reference< XInputStream >& rxInStrm, const OUString& rSysFileName )
{
    if( rxFactory.is() && rxInStrm.is() )
    {
        StorageRef xStrg( new ZipStorage( rxFactory, rxInStrm ) );
        MediaDescriptor aMediaDesc;
        ConfigRef xCfg( new Config( DUMP_PPTX_CONFIG_ENVVAR, rxFactory, xStrg, rSysFileName, aMediaDesc ) );
        DumperBase::construct( xCfg );
    }
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

