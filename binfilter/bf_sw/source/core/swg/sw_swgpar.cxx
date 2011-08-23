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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "swtypes.hxx"

#include "rdswg.hxx"
#include "swgpar.hxx"
namespace binfilter {

/////////////////////////////////////////////////////////////////////////

SwSwgParser::SwSwgParser( SwDoc *pSwDoc, const SwPaM* pSwPaM,
                 SvStream *pIstream, const String& rFileName, BOOL bNewDoc )
{
    pRdr = new SwSwgReader( pSwDoc, pSwPaM, *pIstream, rFileName, bNewDoc );
    pRdr->LoadFileHeader();
}

SwSwgParser::SwSwgParser( SvStream *pIstream )
{
    pRdr = new SwSwgReader( NULL, NULL, *pIstream, aEmptyStr, TRUE );
    pRdr->LoadFileHeader();
}

SwSwgParser::~SwSwgParser()
{
    delete pRdr;
}


ULONG SwSwgParser::CallParser( USHORT nOptions )
{
    return pRdr->Read( nOptions );
}

BOOL SwSwgParser::NeedsPasswd()
{
    return pRdr->GetError()
        ? FALSE
        : BOOL( ( pRdr->aFile.nFlags & SWGF_HAS_PASSWD ) != 0 );
}

BOOL SwSwgParser::CheckPasswd( const String& rStr )
{
    return pRdr->GetError() ? FALSE : pRdr->CheckPasswd( rStr );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
