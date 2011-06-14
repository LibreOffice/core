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
#include "precompiled_sc.hxx"



#ifndef PCH
#include "global.hxx"
#include "document.hxx"
#include "filter.hxx"
#endif
#include "editutil.hxx"
#include "rtfimp.hxx"
#include "rtfparse.hxx"
#include "ftools.hxx"


FltError ScFormatFilterPluginImpl::ScImportRTF( SvStream &rStream, const String& rBaseURL, ScDocument *pDoc, ScRange& rRange )
{
    ScRTFImport aImp( pDoc, rRange );
    FltError nErr = (FltError) aImp.Read( rStream, rBaseURL );
    ScRange aR = aImp.GetRange();
    rRange.aEnd = aR.aEnd;
    aImp.WriteToDocument();
    return nErr;
}

ScEEAbsImport *ScFormatFilterPluginImpl::CreateRTFImport( ScDocument* pDoc, const ScRange& rRange )
{
    return new ScRTFImport( pDoc, rRange );
}


ScRTFImport::ScRTFImport( ScDocument* pDocP, const ScRange& rRange ) :
    ScEEImport( pDocP, rRange )
{
    mpParser = new ScRTFParser( mpEngine );
}


ScRTFImport::~ScRTFImport()
{
    // Reihenfolge wichtig, sonst knallt's irgendwann irgendwo in irgendeinem Dtor!
    // Ist gewaehrleistet, da ScEEImport Basisklasse ist
    delete (ScRTFParser*) mpParser;     // vor EditEngine!
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
