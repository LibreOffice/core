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

#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <tools/urlobj.hxx>
#include <unotools/charclass.hxx>
#include <stdlib.h>
#include <ctype.h>
#include <unotools/syslocale.hxx>

#include "global.hxx"
#include "rangeutl.hxx"
#include "rechead.hxx"
#include "compiler.hxx"
#include "paramisc.hxx"

#include "sc.hrc"
#include "globstr.hrc"

using ::std::vector;


// struct ScImportParam:

ScImportParam::ScImportParam() :
    nCol1(0),
    nRow1(0),
    nCol2(0),
    nRow2(0),
    bImport(false),
    bNative(false),
    bSql(true),
    nType(ScDbTable)
{
}

ScImportParam::ScImportParam( const ScImportParam& r ) :
    nCol1       (r.nCol1),
    nRow1       (r.nRow1),
    nCol2       (r.nCol2),
    nRow2       (r.nRow2),
    bImport     (r.bImport),
    aDBName     (r.aDBName),
    aStatement  (r.aStatement),
    bNative     (r.bNative),
    bSql        (r.bSql),
    nType       (r.nType)
{
}

ScImportParam::~ScImportParam()
{
}

ScImportParam& ScImportParam::operator=( const ScImportParam& r )
{
    nCol1           = r.nCol1;
    nRow1           = r.nRow1;
    nCol2           = r.nCol2;
    nRow2           = r.nRow2;
    bImport         = r.bImport;
    aDBName         = r.aDBName;
    aStatement      = r.aStatement;
    bNative         = r.bNative;
    bSql            = r.bSql;
    nType           = r.nType;

    return *this;
}

bool ScImportParam::operator==( const ScImportParam& rOther ) const
{
    return( nCol1       == rOther.nCol1 &&
            nRow1       == rOther.nRow1 &&
            nCol2       == rOther.nCol2 &&
            nRow2       == rOther.nRow2 &&
            bImport     == rOther.bImport &&
            aDBName     == rOther.aDBName &&
            aStatement  == rOther.aStatement &&
            bNative     == rOther.bNative &&
            bSql        == rOther.bSql &&
            nType       == rOther.nType );

    //! nQuerySh und pConnection sind gleich ?
}


// struct ScConsolidateParam:

ScConsolidateParam::ScConsolidateParam() :
    ppDataAreas( NULL )
{
    Clear();
}

ScConsolidateParam::ScConsolidateParam( const ScConsolidateParam& r ) :
        nCol(r.nCol),nRow(r.nRow),nTab(r.nTab),
        eFunction(r.eFunction),nDataAreaCount(0),
        ppDataAreas( NULL ),
        bByCol(r.bByCol),bByRow(r.bByRow),bReferenceData(r.bReferenceData)
{
    if ( r.nDataAreaCount > 0 )
    {
        nDataAreaCount = r.nDataAreaCount;
        ppDataAreas = new ScArea*[nDataAreaCount];
        for ( sal_uInt16 i=0; i<nDataAreaCount; i++ )
            ppDataAreas[i] = new ScArea( *(r.ppDataAreas[i]) );
    }
}

ScConsolidateParam::~ScConsolidateParam()
{
    ClearDataAreas();
}

void ScConsolidateParam::ClearDataAreas()
{
    if ( ppDataAreas )
    {
        for ( sal_uInt16 i=0; i<nDataAreaCount; i++ )
            delete ppDataAreas[i];
        delete [] ppDataAreas;
        ppDataAreas = NULL;
    }
    nDataAreaCount = 0;
}

void ScConsolidateParam::Clear()
{
    ClearDataAreas();

    nCol = 0;
    nRow = 0;
    nTab = 0;
    bByCol = bByRow = bReferenceData    = false;
    eFunction                           = SUBTOTAL_FUNC_SUM;
}

ScConsolidateParam& ScConsolidateParam::operator=( const ScConsolidateParam& r )
{
    nCol            = r.nCol;
    nRow            = r.nRow;
    nTab            = r.nTab;
    bByCol          = r.bByCol;
    bByRow          = r.bByRow;
    bReferenceData  = r.bReferenceData;
    eFunction       = r.eFunction;
    SetAreas( r.ppDataAreas, r.nDataAreaCount );

    return *this;
}

bool ScConsolidateParam::operator==( const ScConsolidateParam& r ) const
{
    bool bEqual =   (nCol           == r.nCol)
                 && (nRow           == r.nRow)
                 && (nTab           == r.nTab)
                 && (bByCol         == r.bByCol)
                 && (bByRow         == r.bByRow)
                 && (bReferenceData == r.bReferenceData)
                 && (nDataAreaCount == r.nDataAreaCount)
                 && (eFunction      == r.eFunction);

    if ( nDataAreaCount == 0 )
        bEqual = bEqual && (ppDataAreas == NULL) && (r.ppDataAreas == NULL);
    else
        bEqual = bEqual && (ppDataAreas != NULL) && (r.ppDataAreas != NULL);

    if ( bEqual && (nDataAreaCount > 0) )
        for ( sal_uInt16 i=0; i<nDataAreaCount && bEqual; i++ )
            bEqual = *(ppDataAreas[i]) == *(r.ppDataAreas[i]);

    return bEqual;
}

void ScConsolidateParam::SetAreas( ScArea* const* ppAreas, sal_uInt16 nCount )
{
    ClearDataAreas();
    if ( ppAreas && nCount > 0 )
    {
        ppDataAreas = new ScArea*[nCount];
        for ( sal_uInt16 i=0; i<nCount; i++ )
            ppDataAreas[i] = new ScArea( *(ppAreas[i]) );
        nDataAreaCount = nCount;
    }
}


// struct ScSolveParam

ScSolveParam::ScSolveParam()
    :   pStrTargetVal( NULL )
{
}

ScSolveParam::ScSolveParam( const ScSolveParam& r )
    :   aRefFormulaCell ( r.aRefFormulaCell ),
        aRefVariableCell( r.aRefVariableCell ),
        pStrTargetVal   ( r.pStrTargetVal
                            ? new OUString(*r.pStrTargetVal)
                            : NULL )
{
}

ScSolveParam::ScSolveParam( const ScAddress& rFormulaCell,
                            const ScAddress& rVariableCell,
                            const OUString&   rTargetValStr )
    :   aRefFormulaCell ( rFormulaCell ),
        aRefVariableCell( rVariableCell ),
        pStrTargetVal   ( new OUString(rTargetValStr) )
{
}

ScSolveParam::~ScSolveParam()
{
    delete pStrTargetVal;
}

ScSolveParam& ScSolveParam::operator=( const ScSolveParam& r )
{
    delete pStrTargetVal;

    aRefFormulaCell  = r.aRefFormulaCell;
    aRefVariableCell = r.aRefVariableCell;
    pStrTargetVal    = r.pStrTargetVal
                            ? new OUString(*r.pStrTargetVal)
                            : NULL;
    return *this;
}

bool ScSolveParam::operator==( const ScSolveParam& r ) const
{
    bool bEqual =   (aRefFormulaCell  == r.aRefFormulaCell)
                 && (aRefVariableCell == r.aRefVariableCell);

    if ( bEqual )
    {
        if ( !pStrTargetVal && !r.pStrTargetVal )
            bEqual = true;
        else if ( !pStrTargetVal || !r.pStrTargetVal )
            bEqual = false;
        else if ( pStrTargetVal && r.pStrTargetVal )
            bEqual = ( *pStrTargetVal == *(r.pStrTargetVal) );
    }

    return bEqual;
}


// struct ScTabOpParam

ScTabOpParam::ScTabOpParam() : meMode(Column) {}

ScTabOpParam::ScTabOpParam( const ScTabOpParam& r )
    :   aRefFormulaCell ( r.aRefFormulaCell ),
        aRefFormulaEnd  ( r.aRefFormulaEnd ),
        aRefRowCell     ( r.aRefRowCell ),
        aRefColCell     ( r.aRefColCell ),
    meMode(r.meMode)
{
}

ScTabOpParam::ScTabOpParam( const ScRefAddress& rFormulaCell,
                            const ScRefAddress& rFormulaEnd,
                            const ScRefAddress& rRowCell,
                            const ScRefAddress& rColCell,
                            Mode eMode )
    :   aRefFormulaCell ( rFormulaCell ),
        aRefFormulaEnd  ( rFormulaEnd ),
        aRefRowCell     ( rRowCell ),
        aRefColCell     ( rColCell ),
    meMode(eMode)
{
}

ScTabOpParam& ScTabOpParam::operator=( const ScTabOpParam& r )
{
    aRefFormulaCell  = r.aRefFormulaCell;
    aRefFormulaEnd   = r.aRefFormulaEnd;
    aRefRowCell      = r.aRefRowCell;
    aRefColCell      = r.aRefColCell;
    meMode = r.meMode;
    return *this;
}

bool ScTabOpParam::operator==( const ScTabOpParam& r ) const
{
    return (        (aRefFormulaCell == r.aRefFormulaCell)
                 && (aRefFormulaEnd  == r.aRefFormulaEnd)
                 && (aRefRowCell     == r.aRefRowCell)
                 && (aRefColCell     == r.aRefColCell)
                 && (meMode == r.meMode) );
}

OUString ScGlobal::GetAbsDocName( const OUString& rFileName,
                                SfxObjectShell* pShell )
{
    OUString aAbsName;
    if ( !pShell->HasName() )
    {   // maybe relative to document path working directory
        INetURLObject aObj;
        SvtPathOptions aPathOpt;
        aObj.SetSmartURL( aPathOpt.GetWorkPath() );
        aObj.setFinalSlash();       // it IS a path
        bool bWasAbs = true;
        aAbsName = aObj.smartRel2Abs( rFileName, bWasAbs ).GetMainURL(INetURLObject::NO_DECODE);
        //  returned string must be encoded because it's used directly to create SfxMedium
    }
    else
    {
        const SfxMedium* pMedium = pShell->GetMedium();
        if ( pMedium )
        {
            bool bWasAbs = true;
            aAbsName = pMedium->GetURLObject().smartRel2Abs( rFileName, bWasAbs ).GetMainURL(INetURLObject::NO_DECODE);
        }
        else
        {   // This can't happen, but ...
            // just to be sure to have the same encoding
            INetURLObject aObj;
            aObj.SetSmartURL( aAbsName );
            aAbsName = aObj.GetMainURL(INetURLObject::NO_DECODE);
        }
    }
    return aAbsName;
}

OUString ScGlobal::GetDocTabName( const OUString& rFileName,
                                const OUString& rTabName )
{
    OUString  aDocTab('\'');
    aDocTab += rFileName;
    sal_Int32 nPos = 1;
    while( (nPos = aDocTab.indexOf( '\'', nPos )) != -1 )
    {   // escape Quotes
        aDocTab = aDocTab.replaceAt( nPos, 0, "\\" );
        nPos += 2;
    }
    aDocTab += "'";
    aDocTab += OUString(SC_COMPILER_FILE_TAB_SEP);
    aDocTab += rTabName;    // "'Doc'#Tab"
    return aDocTab;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
