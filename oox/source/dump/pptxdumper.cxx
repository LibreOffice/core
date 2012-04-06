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

#include "oox/dump/pptxdumper.hxx"

#include "oox/dump/oledumper.hxx"
#include "oox/dump/xlsbdumper.hxx"
#include "oox/helper/zipstorage.hxx"
#include "oox/ole/olestorage.hxx"

#if OOX_INCLUDE_DUMPER

namespace oox {
namespace dump {
namespace pptx {

// ============================================================================

using namespace ::com::sun::star::io;
//using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

using ::comphelper::MediaDescriptor;
using ::oox::core::FilterBase;
using ::rtl::OUString;

// ============================================================================

RootStorageObject::RootStorageObject( const DumperBase& rParent )
{
    StorageObjectBase::construct( rParent );
}

void RootStorageObject::implDumpStream( const Reference< XInputStream >& rxStrm, const OUString& rStrgPath, const OUString& rStrmName, const OUString& rSysFileName )
{
    OUString aExt = InputOutputHelper::getFileNameExtension( rStrmName );
    if( aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("pptx")) ||
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("potx")) )
    {
        Dumper( getContext(), rxStrm, rSysFileName ).dump();
    }
#if FIXME
    else if(
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("xlsb")) ||
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("xlsm")) ||
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("xlsx")) ||
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("xltm")) ||
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("xltx")) )
    {
        ::oox::dump::xlsb::Dumper( getContext(), rxStrm, rSysFileName ).dump();
    }
    else if(
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("xla")) ||
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("xlc")) ||
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("xlm")) ||
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("xls")) ||
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("xlt")) ||
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("xlw")) )
    {
        ::oox::dump::biff::Dumper( getContext(), rxStrm, rSysFileName ).dump();
    }
#endif
    else if(
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("xml")) ||
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("vml")) ||
        aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("rels")) )
    {
        XmlStreamObject( *this, rxStrm, rSysFileName ).dump();
    }
    else if( aExt.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("bin")) )
    {
        if( rStrgPath.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ppt" ) ) && rStrmName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "vbaProject.bin" ) ) )
        {
            StorageRef xStrg( new ::oox::ole::OleStorage( getContext(), rxStrm, false ) );
            VbaProjectStorageObject( *this, xStrg, rSysFileName ).dump();
        }
        else if ( rStrgPath == "ppt/embeddings" )
        {
            StorageRef xStrg( new ::oox::ole::OleStorage( getContext(), rxStrm, false ) );
            OleStorageObject( *this, xStrg, rSysFileName ).dump();
        }
        else if ( rStrgPath == "ppt/activeX" )
        {
            StorageRef xStrg( new ::oox::ole::OleStorage( getContext(), rxStrm, true ) );
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

Dumper::Dumper( const Reference< XComponentContext >& rxContext, const Reference< XInputStream >& rxInStrm, const OUString& rSysFileName )
{
    if( rxContext.is() && rxInStrm.is() )
    {
        StorageRef xStrg( new ZipStorage( rxContext, rxInStrm ) );
        MediaDescriptor aMediaDesc;
        ConfigRef xCfg( new Config( DUMP_PPTX_CONFIG_ENVVAR, rxContext, xStrg, rSysFileName, aMediaDesc ) );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
