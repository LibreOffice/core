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



#include "hints.hxx"

// -----------------------------------------------------------------------
//      ScPaintHint - Angabe, was neu gezeichnet werden muss
// -----------------------------------------------------------------------

ScPaintHint::ScPaintHint( const ScRange& rRng, sal_uInt16 nPaint ) :
    aRange( rRng ),
    nParts( nPaint ),
    bPrint( sal_True )
{
}

ScPaintHint::~ScPaintHint()
{
}

// -----------------------------------------------------------------------
//      ScUpdateRefHint - Referenz-Updaterei
// -----------------------------------------------------------------------

ScUpdateRefHint::ScUpdateRefHint( UpdateRefMode eMode, const ScRange& rR,
                                    SCsCOL nX, SCsROW nY, SCsTAB nZ ) :
    eUpdateRefMode( eMode ),
    aRange( rR ),
    nDx( nX ),
    nDy( nY ),
    nDz( nZ )
{
}

ScUpdateRefHint::~ScUpdateRefHint()
{
}

// -----------------------------------------------------------------------
//      ScPointerChangedHint - Pointer ist ungueltig geworden
// -----------------------------------------------------------------------

//UNUSED2008-05  ScPointerChangedHint::ScPointerChangedHint( sal_uInt16 nF ) :
//UNUSED2008-05      nFlags( nF )
//UNUSED2008-05  {
//UNUSED2008-05  }

ScPointerChangedHint::~ScPointerChangedHint()
{
}

// -----------------------------------------------------------------------
//      ScLinkRefreshedHint - a link has been refreshed
// -----------------------------------------------------------------------

ScLinkRefreshedHint::ScLinkRefreshedHint() :
    nLinkType( SC_LINKREFTYPE_NONE ),
    nDdeMode( 0 )
{
}

ScLinkRefreshedHint::~ScLinkRefreshedHint()
{
}

void ScLinkRefreshedHint::SetSheetLink( const String& rSourceUrl )
{
    nLinkType = SC_LINKREFTYPE_SHEET;
    aUrl = rSourceUrl;
}

void ScLinkRefreshedHint::SetDdeLink(
            const String& rA, const String& rT, const String& rI, sal_uInt8 nM )
{
    nLinkType = SC_LINKREFTYPE_DDE;
    aDdeAppl  = rA;
    aDdeTopic = rT;
    aDdeItem  = rI;
    nDdeMode  = nM;
}

void ScLinkRefreshedHint::SetAreaLink( const ScAddress& rPos )
{
    nLinkType = SC_LINKREFTYPE_AREA;
    aDestPos = rPos;
}

// -----------------------------------------------------------------------
//      ScAutoStyleHint - STYLE() function has been called
// -----------------------------------------------------------------------

ScAutoStyleHint::ScAutoStyleHint( const ScRange& rR, const String& rSt1,
                                        sal_uLong nT, const String& rSt2 ) :
    aRange( rR ),
    aStyle1( rSt1 ),
    aStyle2( rSt2 ),
    nTimeout( nT )
{
}

ScAutoStyleHint::~ScAutoStyleHint()
{
}


ScDBRangeRefreshedHint::ScDBRangeRefreshedHint( const ScImportParam& rP )
    : aParam(rP)
{
}
ScDBRangeRefreshedHint::~ScDBRangeRefreshedHint()
{
}


ScDataPilotModifiedHint::ScDataPilotModifiedHint( const String& rName )
    : maName(rName)
{
}
ScDataPilotModifiedHint::~ScDataPilotModifiedHint()
{
}


