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

#include "i18nlangtag/mslangid.hxx"

#include <unotools/configmgr.hxx>
#include <vcl/virdev.hxx>
#include <vcl/print.hxx>
#include <vcl/outdev.hxx>
#include <vcl/edit.hxx>
#include <vcl/settings.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/fontcharmap.hxx>

#include "sallayout.hxx"
#include "svdata.hxx"

#include "impfont.hxx"
#include "outdata.hxx"
#include "fontinstance.hxx"
#include "fontattributes.hxx"

#include "outdev.h"
#include "window.h"

#include "PhysicalFontCollection.hxx"
#include "PhysicalFontFace.hxx"
#include "PhysicalFontFamily.hxx"

#include "svids.hrc"

#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include "graphite_features.hxx"
#endif

#include "../gdi/pdfwriter_impl.hxx"

#include <boost/functional/hash.hpp>
#include <cmath>
#include <cstring>
#include <memory>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::utl;

// extend std namespace to add custom hash needed for LogicalFontInstance

namespace std
{
    template <> struct hash< pair< sal_UCS4, FontWeight > >
    {
        size_t operator()(const pair< sal_UCS4, FontWeight >& rData) const
        {
            size_t h1 = hash<sal_UCS4>()(rData.first);
            size_t h2 = hash<int>()(rData.second);
            return h1 ^ h2;
        }
    };
}


LogicalFontInstance::LogicalFontInstance( const FontSelectPattern& rFontSelData )
    : mpFontCache(nullptr)
    , maFontSelData( rFontSelData )
    , maFontAttributes( rFontSelData )
    , mpConversion( nullptr )
    , mnLineHeight( 0 )
    , mnRefCount( 1 )
    , mnSetFontFlags( 0 )
    , mnOwnOrientation( 0 )
    , mnOrientation( 0 )
    , mbInit( false )
    , mpUnicodeFallbackList( nullptr )
{
    maFontSelData.mpFontInstance = this;
}

LogicalFontInstance::~LogicalFontInstance()
{
    delete mpUnicodeFallbackList;
    mpFontCache = nullptr;
}

void LogicalFontInstance::AddFallbackForUnicode( sal_UCS4 cChar, FontWeight eWeight, const OUString& rFontName )
{
    if( !mpUnicodeFallbackList )
        mpUnicodeFallbackList = new UnicodeFallbackList;
    (*mpUnicodeFallbackList)[ std::pair< sal_UCS4, FontWeight >(cChar,eWeight) ] = rFontName;
}

bool LogicalFontInstance::GetFallbackForUnicode( sal_UCS4 cChar, FontWeight eWeight, OUString* pFontName ) const
{
    if( !mpUnicodeFallbackList )
        return false;

    UnicodeFallbackList::const_iterator it = mpUnicodeFallbackList->find( std::pair< sal_UCS4, FontWeight >(cChar,eWeight) );
    if( it == mpUnicodeFallbackList->end() )
        return false;

    *pFontName = (*it).second;
    return true;
}

void LogicalFontInstance::IgnoreFallbackForUnicode( sal_UCS4 cChar, FontWeight eWeight, const OUString& rFontName )
{
    UnicodeFallbackList::iterator it = mpUnicodeFallbackList->find( std::pair< sal_UCS4,FontWeight >(cChar,eWeight) );
    if( it == mpUnicodeFallbackList->end() )
        return;
    if( (*it).second == rFontName )
        mpUnicodeFallbackList->erase( it );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
