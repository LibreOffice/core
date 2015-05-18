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

#include "oox/dump/pptxdumper.hxx"

#include "oox/dump/oledumper.hxx"
#include "oox/dump/xlsbdumper.hxx"
#include "oox/helper/zipstorage.hxx"
#include "oox/ole/olestorage.hxx"

#if OOX_INCLUDE_DUMPER

namespace oox {
namespace dump {
namespace pptx {

using namespace ::com::sun::star::io;
//using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

using ::oox::core::FilterBase;

RootStorageObject::RootStorageObject( const DumperBase& rParent )
{
    StorageObjectBase::construct( rParent );
}

void RootStorageObject::implDumpStream( const Reference< XInputStream >& rxStrm, const OUString& rStrgPath, const OUString& rStrmName, const OUString& rSysFileName )
{
    OUString aExt = InputOutputHelper::getFileNameExtension( rStrmName );
    if( aExt.equalsIgnoreAsciiCase("pptx") ||
        aExt.equalsIgnoreAsciiCase("potx") )
    {
        Dumper( getContext(), rxStrm, rSysFileName ).dump();
    }
#ifdef FIXME
    else if(
        aExt.equalsIgnoreAsciiCase("xlsb") ||
        aExt.equalsIgnoreAsciiCase("xlsm") ||
        aExt.equalsIgnoreAsciiCase("xlsx") ||
        aExt.equalsIgnoreAsciiCase("xltm") ||
        aExt.equalsIgnoreAsciiCase("xltx") )
    {
        ::oox::dump::xlsb::Dumper( getContext(), rxStrm, rSysFileName ).dump();
    }
    else if(
        aExt.equalsIgnoreAsciiCase("xla") ||
        aExt.equalsIgnoreAsciiCase("xlc") ||
        aExt.equalsIgnoreAsciiCase("xlm") ||
        aExt.equalsIgnoreAsciiCase("xls") ||
        aExt.equalsIgnoreAsciiCase("xlt") ||
        aExt.equalsIgnoreAsciiCase("xlw") )
    {
        ::oox::dump::biff::Dumper( getContext(), rxStrm, rSysFileName ).dump();
    }
#endif
    else if(
        aExt.equalsIgnoreAsciiCase("xml") ||
        aExt.equalsIgnoreAsciiCase("vml") ||
        aExt.equalsIgnoreAsciiCase("rels") )
    {
        XmlStreamObject( *this, rxStrm, rSysFileName ).dump();
    }
    else if( aExt.equalsIgnoreAsciiCase("bin") )
    {
        if( rStrgPath == "ppt" && rStrmName == "vbaProject.bin" )
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
        ConfigRef xCfg( new Config( DUMP_PPTX_CONFIG_ENVVAR, rxContext, xStrg, rSysFileName ) );
        DumperBase::construct( xCfg );
    }
}

void Dumper::implDump()
{
    RootStorageObject( *this ).dump();
}

}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
