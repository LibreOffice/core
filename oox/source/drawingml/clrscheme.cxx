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

#include <osl/diagnose.h>
#include "oox/drawingml/clrscheme.hxx"
#include "oox/token/tokens.hxx"

namespace oox { namespace drawingml {

bool ClrMap::getColorMap( sal_Int32& nClrToken )
{
    sal_Int32 nMapped = 0;
    std::map < sal_Int32, sal_Int32 >::const_iterator aIter( maClrMap.find( nClrToken ) );
    if ( aIter != maClrMap.end() )
        nMapped = (*aIter).second;
    if ( nMapped )
    {
        nClrToken = nMapped;
        return true;
    }
    else
        return false;
}

void ClrMap::setColorMap( sal_Int32 nClrToken, sal_Int32 nMappedClrToken )
{
    maClrMap[ nClrToken ] = nMappedClrToken;
}



ClrScheme::ClrScheme()
{
}
ClrScheme::~ClrScheme()
{
}

bool ClrScheme::getColor( sal_Int32 nSchemeClrToken, sal_Int32& rColor ) const
{
    OSL_ASSERT((nSchemeClrToken & sal_Int32(0xFFFF0000))==0);
    switch( nSchemeClrToken )
    {
        case XML_bg1 : nSchemeClrToken = XML_lt1; break;
        case XML_bg2 : nSchemeClrToken = XML_lt2; break;
        case XML_tx1 : nSchemeClrToken = XML_dk1; break;
        case XML_tx2 : nSchemeClrToken = XML_dk2; break;
    }
    std::map < sal_Int32, sal_Int32 >::const_iterator aIter( maClrScheme.find( nSchemeClrToken ) );
    if ( aIter != maClrScheme.end() )
        rColor = (*aIter).second;
    return aIter != maClrScheme.end();
}

void ClrScheme::setColor( sal_Int32 nSchemeClrToken, sal_Int32 nColor )
{
    maClrScheme[ nSchemeClrToken ] = nColor;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
