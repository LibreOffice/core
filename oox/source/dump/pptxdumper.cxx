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

#include <oox/dump/pptxdumper.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <oox/dump/oledumper.hxx>
#include <oox/dump/xlsbdumper.hxx>
#include <oox/helper/zipstorage.hxx>
#include <oox/ole/olestorage.hxx>
#include <o3tl/string_view.hxx>

#ifdef DBG_UTIL

namespace oox::dump::pptx {

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
    std::u16string_view aExt( InputOutputHelper::getFileNameExtension( rStrmName ) );
    if( o3tl::equalsIgnoreAsciiCase(aExt, u"pptx") ||
        o3tl::equalsIgnoreAsciiCase(aExt, u"potx") )
    {
        Dumper( getContext(), rxStrm, rSysFileName ).dump();
    }
#ifdef FIXME
    else if(
        o3tl::equalsIgnoreAsciiCase(aExt, u"xlsb") ||
        o3tl::equalsIgnoreAsciiCase(aExt, u"xlsm") ||
        o3tl::equalsIgnoreAsciiCase(aExt, u"xlsx") ||
        o3tl::equalsIgnoreAsciiCase(aExt, u"xltm") ||
        o3tl::equalsIgnoreAsciiCase(aExt, u"xltx") )
    {
        ::oox::dump::xlsb::Dumper( getContext(), rxStrm, rSysFileName ).dump();
    }
    else if(
        o3tl::equalsIgnoreAsciiCase(aExt, u"xla") ||
        o3tl::equalsIgnoreAsciiCase(aExt, u"xlc") ||
        o3tl::equalsIgnoreAsciiCase(aExt, u"xlm") ||
        o3tl::equalsIgnoreAsciiCase(aExt, u"xls") ||
        o3tl::equalsIgnoreAsciiCase(aExt, u"xlt") ||
        o3tl::equalsIgnoreAsciiCase(aExt, u"xlw") )
    {
        ::oox::dump::biff::Dumper( getContext(), rxStrm, rSysFileName ).dump();
    }
#endif
    else if(
        o3tl::equalsIgnoreAsciiCase(aExt, u"xml") ||
        o3tl::equalsIgnoreAsciiCase(aExt, u"vml") ||
        o3tl::equalsIgnoreAsciiCase(aExt, u"rels") )
    {
        XmlStreamObject( *this, rxStrm, rSysFileName ).dump();
    }
    else if( o3tl::equalsIgnoreAsciiCase(aExt, u"bin") )
    {
        if( rStrgPath == "ppt" && rStrmName == "vbaProject.bin" )
        {
            StorageRef xStrg = std::make_shared<::oox::ole::OleStorage>( getContext(), rxStrm, false );
            VbaProjectStorageObject( *this, xStrg, rSysFileName ).dump();
        }
        else if ( rStrgPath == "ppt/embeddings" )
        {
            StorageRef xStrg = std::make_shared<::oox::ole::OleStorage>( getContext(), rxStrm, false );
            OleStorageObject( *this, xStrg, rSysFileName ).dump();
        }
        else if ( rStrgPath == "ppt/activeX" )
        {
            StorageRef xStrg = std::make_shared<::oox::ole::OleStorage>( getContext(), rxStrm, true );
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
    ConfigRef xCfg = std::make_shared<Config>( DUMP_PPTX_CONFIG_ENVVAR, rFilter );
    DumperBase::construct( xCfg );
}

Dumper::Dumper( const Reference< XComponentContext >& rxContext, const Reference< XInputStream >& rxInStrm, const OUString& rSysFileName )
{
    if( rxContext.is() && rxInStrm.is() )
    {
        StorageRef xStrg = std::make_shared<ZipStorage>( rxContext, rxInStrm, false );
        ConfigRef xCfg = std::make_shared<Config>( DUMP_PPTX_CONFIG_ENVVAR, rxContext, xStrg, rSysFileName );
        DumperBase::construct( xCfg );
    }
}

void Dumper::implDump()
{
    RootStorageObject( *this ).dump();
}

} // namespace oox::dump::pptx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
