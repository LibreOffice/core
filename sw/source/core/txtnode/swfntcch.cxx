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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <viewsh.hxx>
#include "swfntcch.hxx"
#include "fmtcol.hxx"
#include "swfont.hxx"

// aus atrstck.cxx
extern const sal_uInt8 StackPos[];

// globale Variablen, werden in SwFntCch.Hxx bekanntgegeben
// Der FontCache wird in TxtInit.Cxx _TXTINIT erzeugt und in _TXTEXIT geloescht
SwFontCache *pSwFontCache = NULL;

/*************************************************************************
|*
|*  SwFontObj::SwFontObj(), ~SwFontObj()
|*
|*************************************************************************/

SwFontObj::SwFontObj( const void *pOwn, ViewShell *pSh ) :
    SwCacheObj( (void*)pOwn ),
    aSwFont( &((SwTxtFmtColl *)pOwn)->GetAttrSet(), pSh ? pSh->getIDocumentSettingAccess() : 0 )
{
    aSwFont.GoMagic( pSh, aSwFont.GetActual() );
    const SwAttrSet& rAttrSet = ((SwTxtFmtColl *)pOwn)->GetAttrSet();
    for (sal_uInt16 i = RES_CHRATR_BEGIN; i < RES_CHRATR_END; i++)
        pDefaultArray[ StackPos[ i ] ] = &rAttrSet.Get( i, sal_True );
}

SwFontObj::~SwFontObj()
{
}

/*************************************************************************
|*
|*  SwFontAccess::SwFontAccess()
|*
|*************************************************************************/

SwFontAccess::SwFontAccess( const void *pOwn, ViewShell *pSh ) :
    SwCacheAccess( *pSwFontCache, pOwn,
            ((SwTxtFmtColl*)pOwn)->IsInSwFntCache() ),
    pShell( pSh )
{
}

SwFontObj *SwFontAccess::Get( )
{
    return (SwFontObj *) SwCacheAccess::Get( );
}

SwCacheObj *SwFontAccess::NewObj( )
{
    ((SwTxtFmtColl*)pOwner)->SetInSwFntCache( sal_True );
    return new SwFontObj( pOwner, pShell );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
