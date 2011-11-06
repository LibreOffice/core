/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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



