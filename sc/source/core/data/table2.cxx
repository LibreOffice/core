/*************************************************************************
 *
 *  $RCSfile: table2.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-23 09:49:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

// TOOLS
#define _BIGINT_HXX
#define _SFXMULTISEL_HXX
#define _STACK_HXX
#define _QUEUE_HXX
#define _DYNARR_HXX
#define _TREELIST_HXX
#define _CACHESTR_HXX
#define _NEW_HXX
#define _DYNARY_HXX
#define _CACHESTR_HXX
#define _SV_MULTISEL_HXX

//SV
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _FONTDLG_HXX
#define _PRVWIN_HXX
#define _HELP_HXX
#define _SPIN_HXX
#define _FILDLG_HXX
#define _COLDLG_HXX
#define _SOUND_HXX
#define _MENUBTN_HXX
#define _CLIP_HXX //*
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _FONTDLG_HXX
#define _PRVWIN_HXX
//#define _COLOR_HXX
//#define _PAL_HXX
//#define _BITMAP_HXX
//#define _GDIOBJ_HXX
//#define _POINTR_HXX
//#define _ICON_HXX
//#define _IMAGE_HXX
//#define _KEYCOD_HXX
//#define _EVENT_HXX
#define _HELP_HXX
//#define _APP_HXX
//#define _MDIAPP_HXX
//#define _TIMER_HXX
//#define _METRIC_HXX
//#define _REGION_HXX
//#define _OUTDEV_HXX
//#define _SYSTEM_HXX
//#define _VIRDEV_HXX
//#define _JOBSET_HXX
//#define _PRINT_HXX
//#define _WINDOW_HXX
//#define _SYSWIN_HXX
//#define _WRKWIN_HXX
#define _MDIWIN_HXX
//#define _FLOATWIN_HXX
//#define _DOCKWIN_HXX
//#define _CTRL_HXX
//#define _SCRBAR_HXX
//#define _BUTTON_HXX
//#define _IMAGEBTN_HXX
//#define _FIXED_HXX
//#define _GROUP_HXX
//#define _EDIT_HXX
//#define _COMBOBOX_HXX
//#define _LSTBOX_HXX
//#define _SELENG_HXX ***
//#define _SPLIT_HXX
#define _SPIN_HXX
//#define _FIELD_HXX
//#define _MOREBTN_HXX ***
//#define _TOOLBOX_HXX
//#define _STATUS_HXX ***
//#define _DIALOG_HXX
//#define _MSGBOX_HXX
//#define _SYSDLG_HXX
#define _FILDLG_HXX
//#define _PRNDLG_HXX
#define _COLDLG_HXX
//#define _TABDLG_HXX
//#define _MENU_HXX
//#define _GDIMTF_HXX
//#define _POLY_HXX
//#define _ACCEL_HXX
//#define _GRAPH_HXX
#define _SOUND_HXX


//svtools

#define _SCRWIN_HXX
#define _RULER_HXX
//#define _TABBAR_HXX
//#define _VALUESET_HXX
#define _STDMENU_HXX
//#define _STDCTRL_HXX
//#define _CTRLBOX_HXX
#define _CTRLTOOL_HXX
#define _EXTATTR_HXX
#define _FRM3D_HXX
#define _EXTATTR_HXX
//#define _SVTREELIST_HXX ***
#define _FILTER_HXX
//#define _SVLBOXITM_HXX ***
//#define _SVTREEBOX_HXX ***
#define _SVICNVW_HXX
#define _SVTABBX_HXX
#define _SCRWIN_HXX
#define _RULER_HXX
#define _STDMENU_HXX
#define _CTRLTOOL_HXX
#define _EXTATTR_HXX
#define _FRM3D_HXX
#define _EXTATTR_HXX
#define _FILTER_HXX
#define _SVICNVW_HXX
#define _SVTABBX_HXX

//sfxcore.hxx
#define _SFXGENLINK_HXX
#define _SFXHINTPOST_HXX
#define _SFXDOCINF_HXX
#define _SFXLINKHDL_HXX

//sfxsh.hxx
#define _SFXMACRO_HXX

// SFX
#define _SFX_SAVEOPT_HXX
#define _SFXPRNMON_HXX
#define _INTRO_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXFILEDLG_HXX
#define _PASSWD_HXX
#define _SFXTBXCTRL_HXX
#define _SFXSTBITEM_HXX
#define _SFXMNUITEM_HXX
#define _SFXIMGMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXSTBMGR_HXX
#define _SFX_MINFITEM_HXX
#define _SFXEVENT_HXX

//sfxdoc.hxx
//#define _SFX_OBJSH_HXX
//#define _SFX_CLIENTSH_HXX
//#define _SFXDOCINF_HXX
//#define _SFX_OBJFAC_HXX
#define _SFX_DOCFILT_HXX
//#define _SFXDOCFILE_HXX ***
//define _VIEWFAC_HXX
//#define _SFXVIEWFRM_HXX
//#define _SFXVIEWSH_HXX
//#define _MDIFRM_HXX ***
#define _SFX_IPFRM_HXX
//#define _SFX_INTERNO_HXX

//sfxdlg.hxx
//#define _SFXTABDLG_HXX
//#define _BASEDLGS_HXX ***
#define _SFX_DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
//#define _SFX_STYLEDLG_HXX
#define _NEWSTYLE_HXX
//#define _SFXDOCTEMPL_HXX ***
//#define _SFXDOCTDLG_HXX ***
//#define _SFX_TEMPLDLG_HXX ***
//#define _SFXNEW_HXX ***
#define _SFXDOCMAN_HXX
//#define _SFXDOCKWIN_HXX

//xout.hxx
//#define _XENUM_HXX
//#define _XPOLY_HXX
//#define _XATTR_HXX
//#define _XOUTX_HXX
//#define _XPOOL_HXX
//#define _XTABLE_HXX

//svdraw.hxx
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SDR_NOOBJECTS
//#define _SDR_NOVIEWS

//#define SI_NOITEMS //*
//#define SI_NODRW //*
#define _SI_NOSBXCONTROLS
//#define _VCATTR_HXX //*
#define _VCONT_HXX
//#define _VCSBX_HXX //*
#define _SI_NOOTHERFORMS
#define _VCTRLS_HXX
//#define _VCDRWOBJ_HXX //*
#define _SI_NOCONTROL
#define _SETBRW_HXX
#define _VCBRW_HXX
#define _SI_NOSBXCONTROLS
//#define _SIDLL_HXX //***

#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_THESDLG_HXX

//inet
#define _INETINDP_HXX
#define _INETIMP_HXX
#define _INETWRAP_HXX
#define _INETKEEP_HXX
#define _PLUGMGR_HXX
//#define _URLOBJ_HXX

#define SFX_NOCLOOKS

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/boxitem.hxx>
#include <tools/urlobj.hxx>
#include <svtools/poolcach.hxx>
#include <unotools/charclass.hxx>
#include <math.h>
#ifndef _SVTOOLS_PASSWORDHELPER_HXX
#include <svtools/PasswordHelper.hxx>
#endif
#include <unotools/collatorwrapper.hxx>

#include "patattr.hxx"
#include "docpool.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "olinetab.hxx"
#include "rechead.hxx"
#include "stlpool.hxx"
#include "attarray.hxx"     // Iterator
#include "markdata.hxx"
#include "progress.hxx"
#include "dociter.hxx"
#include "conditio.hxx"
#include "chartlis.hxx"
#include "globstr.hrc"

// STATIC DATA -----------------------------------------------------------

void lcl_LoadRange( SvStream& rStream, ScRange** ppRange );
void lcl_SaveRange( SvStream& rStream, ScRange* pRange );



BOOL ScTable::SetOutlineTable( const ScOutlineTable* pNewOutline )
{
    USHORT nOldSizeX = 0;
    USHORT nOldSizeY = 0;
    USHORT nNewSizeX = 0;
    USHORT nNewSizeY = 0;

    if (pOutlineTable)
    {
        nOldSizeX = pOutlineTable->GetColArray()->GetDepth();
        nOldSizeY = pOutlineTable->GetRowArray()->GetDepth();
        delete pOutlineTable;
    }

    if (pNewOutline)
    {
        pOutlineTable = new ScOutlineTable( *pNewOutline );
        nNewSizeX = pOutlineTable->GetColArray()->GetDepth();
        nNewSizeY = pOutlineTable->GetRowArray()->GetDepth();
    }
    else
        pOutlineTable = NULL;

    return ( nNewSizeX != nOldSizeX || nNewSizeY != nOldSizeY );        // Groesse geaendert ?
}


void ScTable::StartOutlineTable()
{
    if (!pOutlineTable)
        pOutlineTable = new ScOutlineTable;
}


BOOL ScTable::TestInsertRow( USHORT nStartCol, USHORT nEndCol, USHORT nSize )
{
    BOOL bTest = TRUE;

    if ( nStartCol==0 && nEndCol==MAXCOL && pOutlineTable )
        bTest = pOutlineTable->TestInsertRow(nSize);

    for (USHORT i=nStartCol; (i<=nEndCol) && bTest; i++)
        bTest = aCol[i].TestInsertRow( nSize );

    return bTest;
}


void ScTable::InsertRow( USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nSize )
{
    USHORT i;
    nRecalcLvl++;
    if (nStartCol==0 && nEndCol==MAXCOL)
    {
        if (pRowHeight && pRowFlags)
        {
            for (i=MAXROW; i>=nStartRow+nSize; i--)
            {
                pRowHeight[i] = pRowHeight[i-nSize];
                pRowFlags[i] = pRowFlags[i-nSize];
            }

            //  #67451# copy row height from row above
            USHORT nSourceRow = ( nStartRow > 0 ) ? ( nStartRow - 1 ) : 0;
            BYTE nNewFlags = pRowFlags[nSourceRow] & CR_MANUALSIZE;
            USHORT nNewHeight = pRowHeight[nSourceRow];
            for (i=nStartRow; i<nStartRow+nSize; i++)
            {
                pRowHeight[i] = nNewHeight;
                pRowFlags[i] = nNewFlags;
            }
        }
        if (pOutlineTable)
            pOutlineTable->InsertRow( nStartRow, nSize );
    }

    for (i=nStartCol; i<=nEndCol; i++)
        aCol[i].InsertRow( nStartRow, nSize );
    if( !--nRecalcLvl )
        SetDrawPageSize();
}


void ScTable::DeleteRow( USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nSize,
                            BOOL* pUndoOutline )
{
    USHORT i;
    nRecalcLvl++;
    if (nStartCol==0 && nEndCol==MAXCOL)
    {
        if (pRowHeight && pRowFlags)
            for (i=nStartRow; i+nSize<=MAXROW; i++)
            {
                pRowHeight[i] = pRowHeight[i+nSize];
                pRowFlags[i] = pRowFlags[i+nSize];
            }
        if (pOutlineTable)
            if (pOutlineTable->DeleteRow( nStartRow, nSize ))
                if (pUndoOutline)
                    *pUndoOutline = TRUE;
    }

    for (i=nStartCol; i<=nEndCol; i++)
        aCol[i].DeleteRow( nStartRow, nSize );
    if( !--nRecalcLvl )
        SetDrawPageSize();
}


BOOL ScTable::TestInsertCol( USHORT nStartRow, USHORT nEndRow, USHORT nSize )
{
    BOOL bTest = TRUE;

    if ( nStartRow==0 && nEndRow==MAXROW && pOutlineTable )
        bTest = pOutlineTable->TestInsertCol(nSize);

    if ( nSize > MAXCOL )
        bTest = FALSE;

    for (USHORT i=MAXCOL; (i+nSize>MAXCOL) && bTest; i--)
        bTest = aCol[i].TestInsertCol(nStartRow, nEndRow);

    return bTest;
}


void ScTable::InsertCol( USHORT nStartCol, USHORT nStartRow, USHORT nEndRow, USHORT nSize )
{
    USHORT i;
    nRecalcLvl++;
    if (nStartRow==0 && nEndRow==MAXROW)
    {
        if (pColWidth && pColFlags)
            for (i=MAXCOL; i>=nStartCol+nSize; i--)
            {
                pColWidth[i] = pColWidth[i-nSize];
                pColFlags[i] = pColFlags[i-nSize];
            }
        if (pOutlineTable)
            pOutlineTable->InsertCol( nStartCol, nSize );
    }


    if ((nStartRow == 0) && (nEndRow == MAXROW))
    {
        for (i=0; i < nSize; i++)
            for (USHORT nCol = MAXCOL; nCol > nStartCol; nCol--)
                aCol[nCol].SwapCol(aCol[nCol-1]);
    }
    else
    {
        for (i=0; i+nSize+nStartCol <= MAXCOL; i++)
            aCol[MAXCOL - nSize - i].MoveTo(nStartRow, nEndRow, aCol[MAXCOL - i]);
    }

    if (nStartCol>0)                        // alte Attribute kopieren
        for (i=0; i<nSize; i++)
        {
            aCol[nStartCol-1].CopyToColumn( nStartRow, nEndRow, IDF_ATTRIB,
                                                FALSE, aCol[nStartCol+i] );
            aCol[nStartCol+i].RemoveFlags( nStartRow, nEndRow,
                                                SC_MF_HOR | SC_MF_VER | SC_MF_AUTO );
        }
    if( !--nRecalcLvl )
        SetDrawPageSize();
}


void ScTable::DeleteCol( USHORT nStartCol, USHORT nStartRow, USHORT nEndRow, USHORT nSize,
                            BOOL* pUndoOutline )
{
    USHORT i;
    nRecalcLvl++;
    if (nStartRow==0 && nEndRow==MAXROW)
    {
        if (pColWidth && pColFlags)
            for (i=nStartCol; i+nSize<=MAXCOL; i++)
            {
                pColWidth[i] = pColWidth[i+nSize];
                pColFlags[i] = pColFlags[i+nSize];
            }
        if (pOutlineTable)
            if (pOutlineTable->DeleteCol( nStartCol, nSize ))
                if (pUndoOutline)
                    *pUndoOutline = TRUE;
    }


    for (i = 0; i < nSize; i++)
        aCol[nStartCol + i].DeleteArea(nStartRow, nEndRow, IDF_ALL);

    if ((nStartRow == 0) && (nEndRow == MAXROW))
    {
        for (i=0; i < nSize; i++)
            for (USHORT nCol = nStartCol; nCol < MAXCOL; nCol++)
                aCol[nCol].SwapCol(aCol[nCol+1]);
    }
    else
    {
        for (i=0; i+nSize+nStartCol <= MAXCOL; i++)
            aCol[nStartCol + nSize + i].MoveTo(nStartRow, nEndRow, aCol[nStartCol + i]);
    }
    if( !--nRecalcLvl )
        SetDrawPageSize();
}


void ScTable::DeleteArea(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nDelFlag)
{
    if (nCol2 > MAXCOL) nCol2 = MAXCOL;
    if (nRow2 > MAXROW) nRow2 = MAXROW;
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
    {
//      nRecalcLvl++;

        for (USHORT i = nCol1; i <= nCol2; i++)
            aCol[i].DeleteArea(nRow1, nRow2, nDelFlag);

            //
            // Zellschutz auf geschuetzter Tabelle nicht setzen
            //

        if ( bProtected && (nDelFlag & IDF_ATTRIB) )
        {
            ScPatternAttr aPattern(pDocument->GetPool());
            aPattern.GetItemSet().Put( ScProtectionAttr( FALSE ) );
            ApplyPatternArea( nCol1, nRow1, nCol2, nRow2, aPattern );
        }

/*      if( !--nRecalcLvl )
            SetDrawPageSize();
*/
    }
}


void ScTable::DeleteSelection( USHORT nDelFlag, const ScMarkData& rMark )
{
    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].DeleteSelection( nDelFlag, rMark );

        //
        // Zellschutz auf geschuetzter Tabelle nicht setzen
        //

    if ( bProtected && (nDelFlag & IDF_ATTRIB) )
    {
        ScDocumentPool* pPool = pDocument->GetPool();
        SfxItemSet aSet( *pPool, ATTR_PATTERN_START, ATTR_PATTERN_END );
        aSet.Put( ScProtectionAttr( FALSE ) );
        SfxItemPoolCache aCache( pPool, &aSet );
        ApplySelectionCache( &aCache, rMark );
    }
}


//  pTable = Clipboard
void ScTable::CopyToClip(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                        ScTable* pTable, BOOL bKeepScenarioFlags)
{
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
    {
        //  Inhalte kopieren

        for (USHORT i = nCol1; i <= nCol2; i++)
            aCol[i].CopyToClip(nRow1, nRow2, pTable->aCol[i], bKeepScenarioFlags);

        //  Zeilen-/Spaltenhoehen, und von Flags nur "ausgeblendet" und "manual"

        if (pColFlags && pTable->pColFlags && pColWidth && pTable->pColWidth)
            for (i=nCol1; i<=nCol2; i++)
            {
                pTable->pColFlags[i] = pColFlags[i] & CR_HIDDEN;
                pTable->pColWidth[i] = pColWidth[i];
            }

        if (pRowFlags && pTable->pRowFlags && pRowHeight && pTable->pRowHeight)
            for (i=nRow1; i<=nRow2; i++)
            {
                pTable->pRowFlags[i] = pRowFlags[i] & (CR_HIDDEN | CR_MANUALSIZE);
                pTable->pRowHeight[i] = pRowHeight[i];
            }


        //  ggf. Formeln durch Werte ersetzen

        if (bProtected)
            for (i = nCol1; i <= nCol2; i++)
                pTable->aCol[i].RemoveProtected(nRow1, nRow2);
    }
}


void ScTable::CopyFromClip(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                            short nDx, short nDy, USHORT nInsFlag, BOOL bAsLink, ScTable* pTable)
{
    if (nCol2 > MAXCOL) nCol2 = MAXCOL;
    if (nRow2 > MAXROW) nRow2 = MAXROW;
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
    {
        nRecalcLvl++;
        for (USHORT i = nCol1; i <= nCol2; i++)
            aCol[i].CopyFromClip(nRow1, nRow2, nDy, nInsFlag, bAsLink, pTable->aCol[i - nDx]);

        if ((nInsFlag & IDF_ATTRIB) != 0)
        {
            if (nRow1==0 && nRow2==MAXROW && pColWidth && pTable->pColWidth)
                for (i=nCol1; i<=nCol2; i++)
                    pColWidth[i] = pTable->pColWidth[i-nDx];

            if (nCol1==0 && nCol2==MAXCOL && pRowHeight && pTable->pRowHeight &&
                                             pRowFlags && pTable->pRowFlags)
                for (i=nRow1; i<=nRow2; i++)
                {
                    pRowHeight[i] = pTable->pRowHeight[i-nDy];
                    //  CR_MANUALSIZE Bit muss mitkopiert werden, sonst macht pRowHeight keinen Sinn
                    if ( pTable->pRowFlags[i-nDy] & CR_MANUALSIZE )
                        pRowFlags[i] |= CR_MANUALSIZE;
                    else
                        pRowFlags[i] &= ~CR_MANUALSIZE;
                }

                //
                // Zellschutz auf geschuetzter Tabelle nicht setzen
                //

            if ( bProtected && (nInsFlag & IDF_ATTRIB) )
            {
                ScPatternAttr aPattern(pDocument->GetPool());
                aPattern.GetItemSet().Put( ScProtectionAttr( FALSE ) );
                ApplyPatternArea( nCol1, nRow1, nCol2, nRow2, aPattern );
            }
        }
        if( !--nRecalcLvl )
            SetDrawPageSize();
    }
}


void ScTable::MixData( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                            USHORT nFunction, BOOL bSkipEmpty, ScTable* pSrcTab )
{
    for (USHORT i=nCol1; i<=nCol2; i++)
        aCol[i].MixData( nRow1, nRow2, nFunction, bSkipEmpty, pSrcTab->aCol[i] );
}


//  Markierung von diesem Dokument
void ScTable::MixMarked( const ScMarkData& rMark, USHORT nFunction,
                        BOOL bSkipEmpty, ScTable* pSrcTab )
{
    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].MixMarked( rMark, nFunction, bSkipEmpty, pSrcTab->aCol[i] );
}


void ScTable::TransposeClip( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                ScTable* pTransClip, USHORT nFlags, BOOL bAsLink )
{
    BOOL bWasCut = pDocument->IsCutMode();

    ScDocument* pDestDoc = pTransClip->pDocument;

    for (USHORT nCol=nCol1; nCol<=nCol2; nCol++)
    {
        USHORT nRow;
        ScBaseCell* pCell;

        if ( bAsLink && nFlags == IDF_ALL )
        {
            //  #68989# with IDF_ALL, also create links (formulas) for empty cells

            for ( nRow=nRow1; nRow<=nRow2; nRow++ )
            {
                //  create simple formula, as in ScColumn::CreateRefCell

                ScAddress aDestPos( nRow-nRow1, nCol-nCol1, pTransClip->nTab );
                SingleRefData aRef;
                aRef.nCol = nCol;
                aRef.nRow = nRow;
                aRef.nTab = nTab;
                aRef.InitFlags();                           // -> all absolute
                aRef.SetFlag3D(TRUE);
                aRef.CalcRelFromAbs( aDestPos );
                ScTokenArray aArr;
                aArr.AddSingleReference( aRef );

                ScBaseCell* pNew = new ScFormulaCell( pDestDoc, aDestPos, &aArr );
                pTransClip->PutCell( nRow-nRow1, nCol-nCol1, pNew );
            }
        }
        else
        {
            ScColumnIterator aIter( &aCol[nCol], nRow1, nRow2 );
            while (aIter.Next( nRow, pCell ))
            {
                ScBaseCell* pNew;
                if ( bAsLink )                  // Referenz erzeugen ?
                {
                    pNew = aCol[nCol].CreateRefCell( pDestDoc,
                                    ScAddress( nRow-nRow1, nCol-nCol1, pTransClip->nTab ),
                                    aIter.GetIndex(), nFlags );
                }
                else                            // kopieren
                {
                    if (pCell->GetCellType() == CELLTYPE_FORMULA)
                    {
                        pNew = ((ScFormulaCell*)pCell)->Clone( pDestDoc,
                                    ScAddress( nRow-nRow1, nCol-nCol1, nTab ) );

                        //  Referenzen drehen
                        //  bei Cut werden Referenzen spaeter per UpdateTranspose angepasst

                        if (!bWasCut)
                            ((ScFormulaCell*)pNew)->TransposeReference();
                    }
                    else
                        pNew = pCell->Clone( pDestDoc );
                }
                pTransClip->PutCell( nRow-nRow1, nCol-nCol1, pNew );
            }
        }

        //  Attribute

        USHORT nAttrRow1;
        USHORT nAttrRow2;
        const ScPatternAttr* pPattern;
        ScAttrIterator* pAttrIter = aCol[nCol].CreateAttrIterator( nRow1, nRow2 );
        while ( (pPattern = pAttrIter->Next( nAttrRow1, nAttrRow2 )) != 0 )
        {
            if ( !IsDefaultItem( pPattern ) )
            {
                const SfxItemSet& rSet = pPattern->GetItemSet();
                if ( rSet.GetItemState( ATTR_MERGE, FALSE ) == SFX_ITEM_DEFAULT &&
                     rSet.GetItemState( ATTR_MERGE_FLAG, FALSE ) == SFX_ITEM_DEFAULT &&
                     rSet.GetItemState( ATTR_BORDER, FALSE ) == SFX_ITEM_DEFAULT )
                {
                    // no borders or merge items involved - use pattern as-is
                    for (nRow = nAttrRow1; nRow<=nAttrRow2; nRow++)
                        pTransClip->SetPattern( nRow-nRow1, nCol-nCol1, *pPattern, TRUE );
                }
                else
                {
                    // transpose borders and merge values, remove merge flags (refreshed after pasting)
                    ScPatternAttr aNewPattern( *pPattern );
                    SfxItemSet& rNewSet = aNewPattern.GetItemSet();

                    const SvxBoxItem& rOldBox = (const SvxBoxItem&)rSet.Get(ATTR_BORDER);
                    if ( rOldBox.GetTop() || rOldBox.GetBottom() || rOldBox.GetLeft() || rOldBox.GetRight() )
                    {
                        SvxBoxItem aNew( ATTR_BORDER );
                        aNew.SetLine( rOldBox.GetLine( BOX_LINE_TOP ), BOX_LINE_LEFT );
                        aNew.SetLine( rOldBox.GetLine( BOX_LINE_LEFT ), BOX_LINE_TOP );
                        aNew.SetLine( rOldBox.GetLine( BOX_LINE_BOTTOM ), BOX_LINE_RIGHT );
                        aNew.SetLine( rOldBox.GetLine( BOX_LINE_RIGHT ), BOX_LINE_BOTTOM );
                        aNew.SetDistance( rOldBox.GetDistance( BOX_LINE_TOP ), BOX_LINE_LEFT );
                        aNew.SetDistance( rOldBox.GetDistance( BOX_LINE_LEFT ), BOX_LINE_TOP );
                        aNew.SetDistance( rOldBox.GetDistance( BOX_LINE_BOTTOM ), BOX_LINE_RIGHT );
                        aNew.SetDistance( rOldBox.GetDistance( BOX_LINE_RIGHT ), BOX_LINE_BOTTOM );
                        rNewSet.Put( aNew );
                    }

                    const ScMergeAttr& rOldMerge = (const ScMergeAttr&)rSet.Get(ATTR_MERGE);
                    if (rOldMerge.IsMerged())
                        rNewSet.Put( ScMergeAttr(
                            Min( rOldMerge.GetRowMerge(), (INT16)(MAXCOL+1 - (nAttrRow2-nRow1)) ),
                            Min( rOldMerge.GetColMerge(), (INT16)(MAXROW+1 - (nCol-nCol1)) ) ) );

                    const ScMergeFlagAttr& rOldFlag = (const ScMergeFlagAttr&)rSet.Get(ATTR_MERGE_FLAG);
                    if (rOldFlag.IsOverlapped())
                    {
                        INT16 nNewFlags = rOldFlag.GetValue() & ~( SC_MF_HOR | SC_MF_VER );
                        if ( nNewFlags )
                            rNewSet.Put( ScMergeFlagAttr( nNewFlags ) );
                        else
                            rNewSet.ClearItem( ATTR_MERGE_FLAG );
                    }

                    for (nRow = nAttrRow1; nRow<=nAttrRow2; nRow++)
                        pTransClip->SetPattern( nRow-nRow1, nCol-nCol1, aNewPattern, TRUE );
                }
            }
        }

        delete pAttrIter;
    }
}


void ScTable::StartAllListeners()
{
    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].StartAllListeners();
}


void ScTable::StartRelNameListeners()
{
    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].StartRelNameListeners();
}


void ScTable::BroadcastInArea( USHORT nCol1, USHORT nRow1,
        USHORT nCol2, USHORT nRow2 )
{
    if (nCol2 > MAXCOL) nCol2 = MAXCOL;
    if (nRow2 > MAXROW) nRow2 = MAXROW;
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
        for (USHORT i = nCol1; i <= nCol2; i++)
            aCol[i].BroadcastInArea( nRow1, nRow2 );
}


void ScTable::StartListeningInArea( USHORT nCol1, USHORT nRow1,
        USHORT nCol2, USHORT nRow2 )
{
    if (nCol2 > MAXCOL) nCol2 = MAXCOL;
    if (nRow2 > MAXROW) nRow2 = MAXROW;
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
        for (USHORT i = nCol1; i <= nCol2; i++)
            aCol[i].StartListeningInArea( nRow1, nRow2 );
}


void ScTable::CopyToTable(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                            USHORT nFlags, BOOL bMarked, ScTable* pDestTab,
                            const ScMarkData* pMarkData,
                            BOOL bAsLink, BOOL bColRowFlags)
{
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
    {
        USHORT i;

        if (nFlags)
            for (i = nCol1; i <= nCol2; i++)
                aCol[i].CopyToColumn(nRow1, nRow2, nFlags, bMarked,
                                pDestTab->aCol[i], pMarkData, bAsLink);

        if (bColRowFlags)       // Spaltenbreiten/Zeilenhoehen/Flags
        {
            //  Charts muessen beim Ein-/Ausblenden angepasst werden
            ScChartListenerCollection* pCharts = pDestTab->pDocument->GetChartListenerCollection();
            if ( pCharts && !pCharts->GetCount() )
                pCharts = NULL;

            if (nRow1==0 && nRow2==MAXROW && pColWidth && pDestTab->pColWidth)
                for (i=nCol1; i<=nCol2; i++)
                {
                    BOOL bChange = pCharts &&
                        ( pDestTab->pColFlags[i] & CR_HIDDEN ) != ( pColFlags[i] & CR_HIDDEN );
                    pDestTab->pColWidth[i] = pColWidth[i];
                    pDestTab->pColFlags[i] = pColFlags[i];
                    //! Aenderungen zusammenfassen?
                    if (bChange)
                        pCharts->SetRangeDirty(ScRange( i, 0, nTab, i, MAXROW, nTab ));
                }

            if (nCol1==0 && nCol2==MAXCOL && pRowHeight && pDestTab->pRowHeight)
                for (i=nRow1; i<=nRow2; i++)
                {
                    BOOL bChange = pCharts &&
                        ( pDestTab->pRowFlags[i] & CR_HIDDEN ) != ( pRowFlags[i] & CR_HIDDEN );
                    pDestTab->pRowHeight[i] = pRowHeight[i];
                    pDestTab->pRowFlags[i]  = pRowFlags[i];
                    //! Aenderungen zusammenfassen?
                    if (bChange)
                        pCharts->SetRangeDirty(ScRange( 0, i, nTab, MAXCOL, i, nTab ));
                }

            pDestTab->SetOutlineTable( pOutlineTable );     // auch nur wenn bColRowFlags
        }
    }
}


void ScTable::UndoToTable(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                            USHORT nFlags, BOOL bMarked, ScTable* pDestTab,
                            const ScMarkData* pMarkData)
{
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
    {
        BOOL bWidth  = (nRow1==0 && nRow2==MAXROW && pColWidth && pDestTab->pColWidth);
        BOOL bHeight = (nCol1==0 && nCol2==MAXCOL && pRowHeight && pDestTab->pRowHeight);

        if (bWidth||bHeight)
            nRecalcLvl++;

        for (USHORT i=0; i<=MAXCOL; i++)
        {
            if ( i >= nCol1 && i <= nCol2 )
                aCol[i].UndoToColumn(nRow1, nRow2, nFlags, bMarked, pDestTab->aCol[i],
                                        pMarkData);
            else
                aCol[i].CopyToColumn(0, MAXROW, IDF_FORMULA, FALSE, pDestTab->aCol[i]);
        }

        if (bWidth||bHeight)
        {
            if (bWidth)
                for (i=nCol1; i<=nCol2; i++)
                    pDestTab->pColWidth[i] = pColWidth[i];
            if (bHeight)
                for (i=nRow1; i<=nRow2; i++)
                    pDestTab->pRowHeight[i] = pRowHeight[i];
            if( !--nRecalcLvl )
                SetDrawPageSize();
        }
    }
}


void ScTable::CopyUpdated( const ScTable* pPosTab, ScTable* pDestTab ) const
{
    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].CopyUpdated( pPosTab->aCol[i], pDestTab->aCol[i] );
}

void ScTable::CopyScenarioTo( ScTable* pDestTab ) const
{
    DBG_ASSERT( bScenario, "bScenario == FALSE" );

    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].CopyScenarioTo( pDestTab->aCol[i] );
}

void ScTable::CopyScenarioFrom( const ScTable* pSrcTab )
{
    DBG_ASSERT( bScenario, "bScenario == FALSE" );

    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].CopyScenarioFrom( pSrcTab->aCol[i] );
}

void ScTable::MarkScenarioIn( ScMarkData& rDestMark, USHORT nNeededBits ) const
{
    DBG_ASSERT( bScenario, "bScenario == FALSE" );

    if ( ( nScenarioFlags & nNeededBits ) != nNeededBits )  // alle Bits gesetzt?
        return;

    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].MarkScenarioIn( rDestMark );
}

BOOL ScTable::HasScenarioRange( const ScRange& rRange ) const
{
    DBG_ASSERT( bScenario, "bScenario == FALSE" );

//  ScMarkData aMark;
//  MarkScenarioIn( aMark, 0 );             //! Bits als Parameter von HasScenarioRange?
//  return aMark.IsAllMarked( rRange );

    ScRange aTabRange = rRange;
    aTabRange.aStart.SetTab( nTab );
    aTabRange.aEnd.SetTab( nTab );

    const ScRangeList* pList = GetScenarioRanges();
//  return ( pList && pList->Find( aTabRange ) );

    if (pList)
    {
        ULONG nCount = pList->Count();
        for ( ULONG j = 0; j < nCount; j++ )
        {
            ScRange* pR = pList->GetObject( j );
            if ( pR->Intersects( aTabRange ) )
                return TRUE;
        }
    }

    return FALSE;
}

void ScTable::InvalidateScenarioRanges()
{
    delete pScenarioRanges;
    pScenarioRanges = NULL;
}

const ScRangeList* ScTable::GetScenarioRanges() const
{
    DBG_ASSERT( bScenario, "bScenario == FALSE" );

    if (!pScenarioRanges)
    {
        ((ScTable*)this)->pScenarioRanges = new ScRangeList;
        ScMarkData aMark;
        MarkScenarioIn( aMark, 0 );     // immer
        aMark.FillRangeListWithMarks( pScenarioRanges, FALSE );
    }
    return pScenarioRanges;
}

BOOL ScTable::TestCopyScenarioTo( const ScTable* pDestTab ) const
{
    DBG_ASSERT( bScenario, "bScenario == FALSE" );

    if (!pDestTab->IsProtected())
        return TRUE;

    BOOL bOk = TRUE;
    for (USHORT i=0; i<=MAXCOL && bOk; i++)
        bOk = aCol[i].TestCopyScenarioTo( pDestTab->aCol[i] );
    return bOk;
}

void ScTable::PutCell( USHORT nCol, USHORT nRow, ScBaseCell* pCell )
{
    if (ValidColRow(nCol,nRow))
    {
        if (pCell)
            aCol[nCol].Insert( nRow, pCell );
        else
            aCol[nCol].Delete( nRow );
    }
}


void ScTable::PutCell( USHORT nCol, USHORT nRow, ULONG nFormatIndex, ScBaseCell* pCell )
{
    if (ValidColRow(nCol,nRow))
    {
        if (pCell)
            aCol[nCol].Insert( nRow, nFormatIndex, pCell );
        else
            aCol[nCol].Delete( nRow );
    }
}


void ScTable::PutCell( const ScAddress& rPos, ScBaseCell* pCell )
{
    if (pCell)
        aCol[rPos.Col()].Insert( rPos.Row(), pCell );
    else
        aCol[rPos.Col()].Delete( rPos.Row() );
}


void ScTable::PutCell( const ScAddress& rPos, ULONG nFormatIndex, ScBaseCell* pCell )
{
    if (pCell)
        aCol[rPos.Col()].Insert( rPos.Row(), nFormatIndex, pCell );
    else
        aCol[rPos.Col()].Delete( rPos.Row() );
}


BOOL ScTable::SetString( USHORT nCol, USHORT nRow, USHORT nTab, const String& rString )
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].SetString( nRow, nTab, rString );
    else
        return FALSE;
}


void ScTable::SetValue( USHORT nCol, USHORT nRow, const double& rVal )
{
    if (ValidColRow(nCol, nRow))
        aCol[nCol].SetValue( nRow, rVal );
}


void ScTable::SetNote( USHORT nCol, USHORT nRow, const ScPostIt& rNote)
{
    if (ValidColRow(nCol, nRow))
        aCol[nCol].SetNote(nRow, rNote);
}


void ScTable::GetString( USHORT nCol, USHORT nRow, String& rString )
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].GetString( nRow, rString );
    else
        rString.Erase();
}


void ScTable::GetInputString( USHORT nCol, USHORT nRow, String& rString )
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].GetInputString( nRow, rString );
    else
        rString.Erase();
}


double ScTable::GetValue( USHORT nCol, USHORT nRow )
{
    if (ValidColRow( nCol, nRow ))
        return aCol[nCol].GetValue( nRow );
    return 0.0;
}


void ScTable::GetFormula( USHORT nCol, USHORT nRow, String& rFormula,
                          BOOL bAsciiExport )
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].GetFormula( nRow, rFormula, bAsciiExport );
    else
        rFormula.Erase();
}


BOOL ScTable::GetNote( USHORT nCol, USHORT nRow, ScPostIt& rNote)
{
    BOOL bHasNote = FALSE;

    if (ValidColRow(nCol,nRow))
        bHasNote = aCol[nCol].GetNote( nRow, rNote );
    else
        rNote.Clear();

    return bHasNote;
}


CellType ScTable::GetCellType( USHORT nCol, USHORT nRow ) const
{
    if (ValidColRow( nCol, nRow ))
        return aCol[nCol].GetCellType( nRow );
    return CELLTYPE_NONE;
}


ScBaseCell* ScTable::GetCell( USHORT nCol, USHORT nRow ) const
{
    if (ValidColRow( nCol, nRow ))
        return aCol[nCol].GetCell( nRow );

    DBG_ERROR("GetCell ausserhalb");
    return NULL;
}


void ScTable::GetLastDataPos(USHORT& rCol, USHORT& rRow) const
{
    rCol = MAXCOL;
    rRow = 0;
    while (aCol[rCol].IsEmptyData() && (rCol > 0))
        rCol--;
    USHORT nCol = rCol;
    while ((short)nCol >= 0)
    {
        rRow = Max(rRow, aCol[nCol].GetLastDataPos());
        nCol--;
    }
}


BOOL ScTable::HasData( USHORT nCol, USHORT nRow )
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].HasDataAt( nRow );
    else
        return FALSE;
}


BOOL ScTable::HasStringData( USHORT nCol, USHORT nRow )
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].HasStringData( nRow );
    else
        return FALSE;
}


BOOL ScTable::HasValueData( USHORT nCol, USHORT nRow )
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].HasValueData( nRow );
    else
        return FALSE;
}


BOOL ScTable::HasStringCells( USHORT nStartCol, USHORT nStartRow,
                                USHORT nEndCol, USHORT nEndRow ) const
{
    if ( nEndCol <= MAXCOL )
        for ( USHORT nCol=nStartCol; nCol<=nEndCol; nCol++ )
            if (aCol[nCol].HasStringCells(nStartRow, nEndRow))
                return TRUE;

    return FALSE;
}


USHORT ScTable::GetErrCode( USHORT nCol, USHORT nRow ) const
{
    if (ValidColRow( nCol, nRow ))
        return aCol[nCol].GetErrCode( nRow );
    return 0;
}


void ScTable::SetDirtyVar()
{
    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].SetDirtyVar();
}


void ScTable::SetDirty()
{
    BOOL bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( FALSE );    // Mehrfachberechnungen vermeiden
    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].SetDirty();
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScTable::SetDirty( const ScRange& rRange )
{
    BOOL bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( FALSE );    // Mehrfachberechnungen vermeiden
    USHORT nCol2 = rRange.aEnd.Col();
    for (USHORT i=rRange.aStart.Col(); i<=nCol2; i++)
        aCol[i].SetDirty( rRange );
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScTable::SetTableOpDirty( const ScRange& rRange )
{
    BOOL bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( FALSE );    // no multiple recalculation
    USHORT nCol2 = rRange.aEnd.Col();
    for (USHORT i=rRange.aStart.Col(); i<=nCol2; i++)
        aCol[i].SetTableOpDirty( rRange );
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScTable::SetDirtyAfterLoad()
{
    BOOL bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( FALSE );    // Mehrfachberechnungen vermeiden
    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].SetDirtyAfterLoad();
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScTable::SetRelNameDirty()
{
    BOOL bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( FALSE );    // Mehrfachberechnungen vermeiden
    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].SetRelNameDirty();
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScTable::CalcAll()
{
    for (USHORT i=0; i<=MAXCOL; i++) aCol[i].CalcAll();
}


void ScTable::CompileAll()
{
    for (USHORT i=0; i <= MAXCOL; i++) aCol[i].CompileAll();
}


void ScTable::CompileXML()
{
    for (USHORT i=0; i <= MAXCOL; i++)
    {
        aCol[i].CompileXML();
    }
}


void ScTable::CalcAfterLoad()
{
    for (USHORT i=0; i <= MAXCOL; i++) aCol[i].CalcAfterLoad();
}


void ScTable::ResetChanged( const ScRange& rRange )
{
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();

    for (USHORT nCol=nStartCol; nCol<=nEndCol; nCol++)
        aCol[nCol].ResetChanged(nStartRow, nEndRow);
}

//  Attribute

const SfxPoolItem* ScTable::GetAttr( USHORT nCol, USHORT nRow, USHORT nWhich ) const
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].GetAttr( nRow, nWhich );
    else
        return NULL;
}


ULONG ScTable::GetNumberFormat( USHORT nCol, USHORT nRow ) const
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].GetNumberFormat( nRow );
    else
        return 0;
}


const ScPatternAttr* ScTable::GetPattern( USHORT nCol, USHORT nRow ) const
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].GetPattern( nRow );
    else
        return NULL;
}


BOOL ScTable::HasAttrib( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nMask ) const
{
    BOOL bFound=FALSE;
    for (USHORT i=nCol1; i<=nCol2 && !bFound; i++)
        bFound |= aCol[i].HasAttrib( nRow1, nRow2, nMask );
    return bFound;
}


BOOL ScTable::HasLines( const ScRange& rRange, Rectangle& rSizes ) const
{
    USHORT nCol1 = rRange.aStart.Col();
    USHORT nRow1 = rRange.aStart.Row();
    USHORT nCol2 = rRange.aEnd.Col();
    USHORT nRow2 = rRange.aEnd.Row();
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );

    BOOL bFound = FALSE;
    for (USHORT i=nCol1; i<=nCol2; i++)
        if (aCol[i].HasLines( nRow1, nRow2, rSizes, (i==nCol1), (i==nCol2) ))
            bFound = TRUE;

    return bFound;
}


BOOL ScTable::HasAttribSelection( const ScMarkData& rMark, USHORT nMask ) const
{
    BOOL bFound=FALSE;
    for (USHORT i=0; i<=MAXCOL && !bFound; i++)
        bFound |= aCol[i].HasAttribSelection( rMark, nMask );
    return bFound;
}


BOOL ScTable::ExtendMerge( USHORT nStartCol, USHORT nStartRow,
                           USHORT& rEndCol, USHORT& rEndRow,
                           BOOL bRefresh, BOOL bAttrs )
{
    BOOL bFound=FALSE;
    USHORT nOldEndX = rEndCol;
    USHORT nOldEndY = rEndRow;
    for (USHORT i=nStartCol; i<=nOldEndX; i++)
        bFound |= aCol[i].ExtendMerge( i, nStartRow, nOldEndY, rEndCol, rEndRow, bRefresh, bAttrs );
    return bFound;
}


BOOL ScTable::IsBlockEmpty( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 ) const
{
    BOOL bEmpty = TRUE;
    for (USHORT i=nCol1; i<=nCol2 && bEmpty; i++)
        bEmpty = aCol[i].IsEmptyBlock( nRow1, nRow2 );
    return bEmpty;
}

USHORT ScTable::FillMaxRot( RowInfo* pRowInfo, USHORT nArrCount, USHORT nX1, USHORT nX2,
                            USHORT nCol, USHORT nAttrRow1, USHORT nAttrRow2, USHORT nArrY,
                            const ScPatternAttr* pPattern, const SfxItemSet* pCondSet ) const
{
    //  Rueckgabe = neues nArrY

    BYTE nRotDir = pPattern->GetRotateDir( pCondSet );
    if ( nRotDir != SC_ROTDIR_NONE )
    {
        BOOL bHit = TRUE;
        if ( nCol+1 < nX1 )                             // links
            bHit = ( nRotDir != SC_ROTDIR_LEFT );
        else if ( nCol > nX2+1 )                        // rechts
            bHit = ( nRotDir == SC_ROTDIR_LEFT );

        if ( bHit )
        {
            double nFactor = 0.0;
            if ( nCol > nX2+1 )
            {
                long nRotVal = ((const SfxInt32Item&) pPattern->
                        GetItem( ATTR_ROTATE_VALUE, pCondSet )).GetValue();
                double nRealOrient = nRotVal * F_PI18000;   // 1/100 Grad
                double nCos = cos( nRealOrient );
                double nSin = sin( nRealOrient );
                //! begrenzen !!!
                //! zusaetzlich Faktor fuer unterschiedliche PPT X/Y !!!

                //  bei SC_ROTDIR_LEFT kommt immer ein negativer Wert heraus,
                //  wenn der Modus beruecksichtigt wird
                nFactor = -fabs( nCos / nSin );
            }

            for ( USHORT nRow = nAttrRow1; nRow <= nAttrRow2; nRow++ )
            {
                if ( !(pRowFlags[nRow] & CR_HIDDEN) )
                {
                    BOOL bHitOne = TRUE;
                    if ( nCol > nX2+1 )
                    {
                        // reicht die gedrehte Zelle bis in den sichtbaren Bereich?

                        USHORT nTouchedCol = nCol;
                        long nWidth = (long) ( pRowHeight[nRow] * nFactor );
                        DBG_ASSERT(nWidth <= 0, "Richtung falsch");
                        while ( nWidth < 0 && nTouchedCol > 0 )
                        {
                            --nTouchedCol;
                            nWidth += GetColWidth( nTouchedCol );
                        }
                        if ( nTouchedCol > nX2 )
                            bHitOne = FALSE;
                    }

                    if (bHitOne)
                    {
                        while ( nArrY<nArrCount && pRowInfo[nArrY].nRowNo < nRow )
                            ++nArrY;
                        if ( nArrY<nArrCount && pRowInfo[nArrY].nRowNo == nRow )
                            pRowInfo[nArrY].nRotMaxCol = nCol;
                    }
                }
            }
        }
    }

    return nArrY;
}

void ScTable::FindMaxRotCol( RowInfo* pRowInfo, USHORT nArrCount, USHORT nX1, USHORT nX2 ) const
{
    if ( !pColWidth || !pRowHeight || !pColFlags || !pRowFlags )
    {
        DBG_ERROR( "Spalten-/Zeileninfo fehlt" );
        return;
    }

    //  nRotMaxCol ist auf SC_ROTMAX_NONE initialisiert, nRowNo ist schon gesetzt

    USHORT nY1 = pRowInfo[0].nRowNo;
    USHORT nY2 = pRowInfo[nArrCount-1].nRowNo;

    for (USHORT nCol=0; nCol<=MAXCOL; nCol++)
    {
        if ( !(pColFlags[nCol] & CR_HIDDEN) )
        {
            USHORT nArrY = 0;
            ScDocAttrIterator aIter( pDocument, nTab, nCol, nY1, nCol, nY2 );
            USHORT nAttrCol, nAttrRow1, nAttrRow2;
            const ScPatternAttr* pPattern = aIter.GetNext( nAttrCol, nAttrRow1, nAttrRow2 );
            while ( pPattern )
            {
                const SfxPoolItem* pCondItem;
                if ( pPattern->GetItemSet().GetItemState( ATTR_CONDITIONAL, TRUE, &pCondItem )
                        == SFX_ITEM_SET )
                {
                    //  alle Formate durchgehen, damit die Zellen nicht einzeln
                    //  angeschaut werden muessen

                    ULONG nIndex = ((const SfxUInt32Item*)pCondItem)->GetValue();
                    ScConditionalFormatList* pList = pDocument->GetCondFormList();
                    ScStyleSheetPool* pStylePool = pDocument->GetStyleSheetPool();
                    if (pList && pStylePool)
                    {
                        const ScConditionalFormat* pFormat = pList->GetFormat(nIndex);
                        if ( pFormat )
                        {
                            USHORT nEntryCount = pFormat->Count();
                            for (USHORT nEntry=0; nEntry<nEntryCount; nEntry++)
                            {
                                String aName = pFormat->GetEntry(nEntry)->GetStyle();
                                if (aName.Len())
                                {
                                    SfxStyleSheetBase* pStyleSheet =
                                            pStylePool->Find( aName, SFX_STYLE_FAMILY_PARA );
                                    if ( pStyleSheet )
                                    {
                                        FillMaxRot( pRowInfo, nArrCount, nX1, nX2,
                                                    nCol, nAttrRow1, nAttrRow2,
                                                    nArrY, pPattern, &pStyleSheet->GetItemSet() );
                                        //  nArrY nicht veraendern
                                    }
                                }
                            }
                        }
                    }
                }

                nArrY = FillMaxRot( pRowInfo, nArrCount, nX1, nX2,
                                    nCol, nAttrRow1, nAttrRow2,
                                    nArrY, pPattern, NULL );

                pPattern = aIter.GetNext( nAttrCol, nAttrRow1, nAttrRow2 );
            }
        }
    }
}

BOOL ScTable::HasBlockMatrixFragment( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 ) const
{
    // nix:0, mitte:1, unten:2, links:4, oben:8, rechts:16, offen:32
    USHORT nEdges;

    if ( nCol1 == nCol2 )
    {   // linke und rechte Spalte
        const USHORT n = 4 | 16;
        nEdges = aCol[nCol1].GetBlockMatrixEdges( nRow1, nRow2, n );
        // nicht (4 und 16) oder 1 oder 32
        if ( nEdges && (((nEdges & n) != n) || (nEdges & 33)) )
            return TRUE;        // linke oder rechte Kante fehlt oder offen
    }
    else
    {   // linke Spalte
        nEdges = aCol[nCol1].GetBlockMatrixEdges( nRow1, nRow2, 4 );
        // nicht 4 oder 1 oder 32
        if ( nEdges && (((nEdges & 4) != 4) || (nEdges & 33)) )
            return TRUE;        // linke Kante fehlt oder offen
        // rechte Spalte
        nEdges = aCol[nCol2].GetBlockMatrixEdges( nRow1, nRow2, 16 );
        // nicht 16 oder 1 oder 32
        if ( nEdges && (((nEdges & 16) != 16) || (nEdges & 33)) )
            return TRUE;        // rechte Kante fehlt oder offen
    }

    if ( nRow1 == nRow2 )
    {   // obere und untere Zeile
        USHORT i;
        BOOL bOpen = FALSE;
        const USHORT n = 2 | 8;
        for ( i=nCol1; i<=nCol2; i++)
        {
            nEdges = aCol[i].GetBlockMatrixEdges( nRow1, nRow1, n );
            if ( nEdges )
            {
                if ( (nEdges & n) != n )
                    return TRUE;        // obere oder untere Kante fehlt
                if ( nEdges & 4 )
                    bOpen = TRUE;       // linke Kante oeffnet, weitersehen
                else if ( !bOpen )
                    return TRUE;        // es gibt was, was nicht geoeffnet wurde
                if ( nEdges & 16 )
                    bOpen = FALSE;      // rechte Kante schliesst
            }
        }
        if ( bOpen )
            return TRUE;                // es geht noch weiter
    }
    else
    {
        USHORT j, nR, n, i;
        // erst obere Zeile, dann untere Zeile
        for ( j=0, nR=nRow1, n=8; j<2; j++, nR=nRow2, n=2 )
        {
            BOOL bOpen = FALSE;
            for ( i=nCol1; i<=nCol2; i++)
            {
                nEdges = aCol[i].GetBlockMatrixEdges( nR, nR, n );
                if ( nEdges )
                {
                    // in oberere Zeile keine obere Kante bzw.
                    // in unterer Zeile keine untere Kante
                    if ( (nEdges & n) != n )
                        return TRUE;
                    if ( nEdges & 4 )
                        bOpen = TRUE;       // linke Kante oeffnet, weitersehen
                    else if ( !bOpen )
                        return TRUE;        // es gibt was, was nicht geoeffnet wurde
                    if ( nEdges & 16 )
                        bOpen = FALSE;      // rechte Kante schliesst
                }
            }
            if ( bOpen )
                return TRUE;                // es geht noch weiter
        }
    }
    return FALSE;
}


BOOL ScTable::HasSelectionMatrixFragment( const ScMarkData& rMark ) const
{
    BOOL bFound=FALSE;
    for (USHORT i=0; i<=MAXCOL && !bFound; i++)
        bFound |= aCol[i].HasSelectionMatrixFragment(rMark);
    return bFound;
}


BOOL ScTable::IsBlockEditable( USHORT nCol1, USHORT nRow1, USHORT nCol2,
            USHORT nRow2, BOOL* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
{
    BOOL bIsEditable;
    if ( nLockCount )
        bIsEditable = FALSE;
    else if ( bProtected )
        bIsEditable = !HasAttrib( nCol1, nRow1, nCol2, nRow2, HASATTR_PROTECTED );
    else
        bIsEditable = TRUE;
    if ( bIsEditable )
    {
        if ( HasBlockMatrixFragment( nCol1, nRow1, nCol2, nRow2 ) )
        {
            bIsEditable = FALSE;
            if ( pOnlyNotBecauseOfMatrix )
                *pOnlyNotBecauseOfMatrix = TRUE;
        }
        else if ( pOnlyNotBecauseOfMatrix )
            *pOnlyNotBecauseOfMatrix = FALSE;
    }
    else if ( pOnlyNotBecauseOfMatrix )
        *pOnlyNotBecauseOfMatrix = FALSE;
    return bIsEditable;
}


BOOL ScTable::IsSelectionEditable( const ScMarkData& rMark,
            BOOL* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
{
    BOOL bIsEditable;
    if ( nLockCount )
        bIsEditable = FALSE;
    else if ( bProtected )
         bIsEditable = !HasAttribSelection( rMark, HASATTR_PROTECTED );
    else
        bIsEditable = TRUE;
    if ( bIsEditable )
    {
        if ( HasSelectionMatrixFragment( rMark ) )
        {
            bIsEditable = FALSE;
            if ( pOnlyNotBecauseOfMatrix )
                *pOnlyNotBecauseOfMatrix = TRUE;
        }
        else if ( pOnlyNotBecauseOfMatrix )
            *pOnlyNotBecauseOfMatrix = FALSE;
    }
    else if ( pOnlyNotBecauseOfMatrix )
        *pOnlyNotBecauseOfMatrix = FALSE;
    return bIsEditable;
}



void ScTable::LockTable()
{
    ++nLockCount;
}


void ScTable::UnlockTable()
{
    if (nLockCount)
        --nLockCount;
    else
        DBG_ERROR("UnlockTable ohne LockTable");
}


void ScTable::MergeSelectionPattern( SfxItemSet** ppSet, const ScMarkData& rMark, BOOL bDeep ) const
{
    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].MergeSelectionPattern( ppSet, rMark, bDeep );
}


void ScTable::MergePatternArea( SfxItemSet** ppSet, USHORT nCol1, USHORT nRow1,
                                                    USHORT nCol2, USHORT nRow2, BOOL bDeep ) const
{
    for (USHORT i=nCol1; i<=nCol2; i++)
        aCol[i].MergePatternArea( ppSet, nRow1, nRow2, bDeep );
}


void ScTable::MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner, ScLineFlags& rFlags,
                    USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow ) const
{
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
    {
        PutInOrder(nStartCol, nEndCol);
        PutInOrder(nStartRow, nEndRow);
        for (USHORT i=nStartCol; i<=nEndCol; i++)
            aCol[i].MergeBlockFrame( pLineOuter, pLineInner, rFlags,
                                    nStartRow, nEndRow, (i==nStartCol), nEndCol-i );
    }
}


void ScTable::ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                    USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow )
{
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
    {
        PutInOrder(nStartCol, nEndCol);
        PutInOrder(nStartRow, nEndRow);
        for (USHORT i=nStartCol; i<=nEndCol; i++)
            aCol[i].ApplyBlockFrame( pLineOuter, pLineInner,
                                    nStartRow, nEndRow, (i==nStartCol), nEndCol-i );
    }
}


void ScTable::ApplyPattern( USHORT nCol, USHORT nRow, const ScPatternAttr& rAttr )
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].ApplyPattern( nRow, rAttr );
}


void ScTable::ApplyPatternArea( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                     const ScPatternAttr& rAttr )
{
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
    {
        PutInOrder(nStartCol, nEndCol);
        PutInOrder(nStartRow, nEndRow);
        for (USHORT i = nStartCol; i <= nEndCol; i++)
            aCol[i].ApplyPatternArea(nStartRow, nEndRow, rAttr);
    }
}

void ScTable::ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
        const ScPatternAttr& rPattern, short nNewType )
{
    USHORT nEndCol = rRange.aEnd.Col();
    for ( USHORT nCol = rRange.aStart.Col(); nCol <= nEndCol; nCol++ )
    {
        aCol[nCol].ApplyPatternIfNumberformatIncompatible( rRange, rPattern, nNewType );
    }
}



void ScTable::ApplyStyle( USHORT nCol, USHORT nRow, const ScStyleSheet& rStyle )
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].ApplyStyle( nRow, rStyle );
}


void ScTable::ApplyStyleArea( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow, const ScStyleSheet& rStyle )
{
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
    {
        PutInOrder(nStartCol, nEndCol);
        PutInOrder(nStartRow, nEndRow);
        for (USHORT i = nStartCol; i <= nEndCol; i++)
            aCol[i].ApplyStyleArea(nStartRow, nEndRow, rStyle);
    }
}


void ScTable::ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark)
{
    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].ApplySelectionStyle( rStyle, rMark );
}


void ScTable::ApplySelectionLineStyle( const ScMarkData& rMark,
                            const SvxBorderLine* pLine, BOOL bColorOnly )
{
    if ( bColorOnly && !pLine )
        return;

    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].ApplySelectionLineStyle( rMark, pLine, bColorOnly );
}


const ScStyleSheet* ScTable::GetStyle( USHORT nCol, USHORT nRow ) const
{
    if (ValidColRow(nCol, nRow))
        return aCol[nCol].GetStyle(nRow);
    else
        return NULL;
}


const ScStyleSheet* ScTable::GetSelectionStyle( const ScMarkData& rMark, BOOL& rFound ) const
{
    rFound = FALSE;

    BOOL    bEqual = TRUE;
    BOOL    bColFound;
    USHORT  i;

    const ScStyleSheet* pStyle = NULL;
    const ScStyleSheet* pNewStyle;

    for (i=0; i<=MAXCOL && bEqual; i++)
        if (rMark.HasMultiMarks(i))
        {
            pNewStyle = aCol[i].GetSelectionStyle( rMark, bColFound );
            if (bColFound)
            {
                rFound = TRUE;
                if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
                    bEqual = FALSE;                                             // unterschiedliche
                pStyle = pNewStyle;
            }
        }

    return bEqual ? pStyle : NULL;
}


const ScStyleSheet* ScTable::GetAreaStyle( BOOL& rFound, USHORT nCol1, USHORT nRow1,
                                                    USHORT nCol2, USHORT nRow2 ) const
{
    rFound = FALSE;

    BOOL    bEqual = TRUE;
    BOOL    bColFound;
    USHORT  i;

    const ScStyleSheet* pStyle = NULL;
    const ScStyleSheet* pNewStyle;

    for (i=nCol1; i<=nCol2 && bEqual; i++)
    {
        pNewStyle = aCol[i].GetAreaStyle(bColFound, nRow1, nRow2);
        if (bColFound)
        {
            rFound = TRUE;
            if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
                bEqual = FALSE;                                             // unterschiedliche
            pStyle = pNewStyle;
        }
    }

    return bEqual ? pStyle : NULL;
}


BOOL ScTable::IsStyleSheetUsed( const SfxStyleSheetBase& rStyle ) const
{
    BOOL bIsUsed = FALSE;

    for ( USHORT i=0; (i<=MAXCOL) && !bIsUsed; i++ )
        bIsUsed = aCol[i].IsStyleSheetUsed( rStyle );

    return bIsUsed;
}


void ScTable::StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, BOOL bRemoved,
                                OutputDevice* pDev,
                                double nPPTX, double nPPTY,
                                const Fraction& rZoomX, const Fraction& rZoomY )
{
    BOOL* pUsed = new BOOL[MAXROW+1];
    USHORT i;
    for (i=0; i<=MAXROW; i++)
        pUsed[i] = FALSE;

    USHORT nCol;
    for (nCol=0; nCol<=MAXCOL; nCol++)
        aCol[nCol].FindStyleSheet( pStyleSheet, pUsed, bRemoved );

    BOOL bFound = FALSE;
    USHORT nStart;
    USHORT nEnd;
    for (i=0; i<=MAXROW; i++)
    {
        if (pUsed[i])
        {
            if (!bFound)
            {
                nStart = i;
                bFound = TRUE;
            }
            nEnd = i;
        }
        else if (bFound)
        {
            SetOptimalHeight( nStart, nEnd, 0, pDev, nPPTX, nPPTY, rZoomX, rZoomY, FALSE );
            bFound = FALSE;
        }
    }
    if (bFound)
        SetOptimalHeight( nStart, nEnd, 0, pDev, nPPTX, nPPTY, rZoomX, rZoomY, FALSE );

    delete[] pUsed;
}


BOOL ScTable::ApplyFlags( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    INT16 nFlags )
{
    BOOL bChanged = FALSE;
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
        for (USHORT i = nStartCol; i <= nEndCol; i++)
            bChanged |= aCol[i].ApplyFlags(nStartRow, nEndRow, nFlags);
    return bChanged;
}


BOOL ScTable::RemoveFlags( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    INT16 nFlags )
{
    BOOL bChanged = FALSE;
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
        for (USHORT i = nStartCol; i <= nEndCol; i++)
            bChanged |= aCol[i].RemoveFlags(nStartRow, nEndRow, nFlags);
    return bChanged;
}


void ScTable::SetPattern( USHORT nCol, USHORT nRow, const ScPatternAttr& rAttr, BOOL bPutToPool )
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].SetPattern( nRow, rAttr, bPutToPool );
}


void ScTable::ApplyAttr( USHORT nCol, USHORT nRow, const SfxPoolItem& rAttr )
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].ApplyAttr( nRow, rAttr );
}


void ScTable::ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark )
{
    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].ApplySelectionCache( pCache, rMark );
}


void ScTable::ChangeSelectionIndent( BOOL bIncrement, const ScMarkData& rMark )
{
    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].ChangeSelectionIndent( bIncrement, rMark );
}


void ScTable::ClearSelectionItems( const USHORT* pWhich, const ScMarkData& rMark )
{
    for (USHORT i=0; i<=MAXCOL; i++)
        aCol[i].ClearSelectionItems( pWhich, rMark );
}


//  Spaltenbreiten / Zeilenhoehen

void ScTable::SetColWidth( USHORT nCol, USHORT nNewWidth )
{
    if (VALIDCOL(nCol) && pColWidth)
    {
        if (!nNewWidth)
        {
//          DBG_ERROR("Spaltenbreite 0 in SetColWidth");
            nNewWidth = STD_COL_WIDTH;
        }

        if ( nNewWidth != pColWidth[nCol] )
        {
            nRecalcLvl++;
            ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
            if (pDrawLayer)
                pDrawLayer->WidthChanged( nTab, nCol, ((long) nNewWidth) - (long) pColWidth[nCol] );
            pColWidth[nCol] = nNewWidth;
            if( !--nRecalcLvl )
                SetDrawPageSize();
        }
    }
    else
        DBG_ERROR("Falsche Spaltennummer oder keine Breiten");
}


void ScTable::SetRowHeight( USHORT nRow, USHORT nNewHeight )
{
    if (VALIDROW(nRow) && pRowHeight)
    {
        if (!nNewHeight)
        {
            DBG_ERROR("Zeilenhoehe 0 in SetRowHeight");
            nNewHeight = ScGlobal::nStdRowHeight;
        }

        if ( nNewHeight != pRowHeight[nRow] )
        {
            nRecalcLvl++;
            ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
            if (pDrawLayer)
                pDrawLayer->HeightChanged( nTab, nRow, ((long) nNewHeight) - (long) pRowHeight[nRow] );
            pRowHeight[nRow] = nNewHeight;
            if( !--nRecalcLvl )
                SetDrawPageSize();
        }
    }
    else
        DBG_ERROR("Falsche Zeilennummer oder keine Hoehen");
}


BOOL ScTable::SetRowHeightRange( USHORT nStartRow, USHORT nEndRow, USHORT nNewHeight,
                                    double nPPTX,double nPPTY )
{
    BOOL bChanged = FALSE;
    if (VALIDROW(nStartRow) && VALIDROW(nEndRow) && pRowHeight)
    {
        nRecalcLvl++;
        if (!nNewHeight)
        {
            DBG_ERROR("Zeilenhoehe 0 in SetRowHeight");
            nNewHeight = ScGlobal::nStdRowHeight;
        }

        long nNewPix = (long) ( nNewHeight * nPPTY );

        BOOL bSingle = FALSE;
        ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
        if (pDrawLayer)
            if (pDrawLayer->HasObjectsInRows( nTab, nStartRow, nEndRow ))
                bSingle = TRUE;

        USHORT nRow;
        if (bSingle)
        {
            BOOL bDiff = FALSE;
            for (nRow=nStartRow; nRow<=nEndRow && !bDiff; nRow++)
                bDiff = ( pRowHeight[nRow] != nNewHeight );
            if (!bDiff)
                bSingle = FALSE;
        }
        if (bSingle)
        {
            if (nEndRow-nStartRow < 20)
                for (nRow=nStartRow; nRow<=nEndRow; nRow++)
                {
                    if (!bChanged)
                        if ( pRowHeight[nRow] != nNewHeight )
                            bChanged = ( nNewPix != (long) ( pRowHeight[nRow] * nPPTY ) );
                    SetRowHeight( nRow, nNewHeight );
                }
            else
            {
                USHORT nMid = (nStartRow+nEndRow) / 2;
                if (SetRowHeightRange( nStartRow, nMid, nNewHeight, 1.0, 1.0 ))
                    bChanged = TRUE;
                if (SetRowHeightRange( nMid+1, nEndRow, nNewHeight, 1.0, 1.0 ))
                    bChanged = TRUE;
            }
        }
        else
        {
            if (pDrawLayer)
            {
                long nHeightDif = 0;
                for (nRow=nStartRow; nRow<=nEndRow; nRow++)
                    nHeightDif += ((long) nNewHeight) - (long) pRowHeight[nRow];
                pDrawLayer->HeightChanged( nTab, nEndRow, nHeightDif );
            }
            for (nRow=nStartRow; nRow<=nEndRow; nRow++)
            {
                if (!bChanged)
                    if ( pRowHeight[nRow] != nNewHeight )
                        bChanged = ( nNewPix != (long) ( pRowHeight[nRow] * nPPTY ) );
                pRowHeight[nRow] = nNewHeight;
            }
        }
        if( !--nRecalcLvl )
            SetDrawPageSize();
    }
    else
        DBG_ERROR("Falsche Zeilennummer oder keine Hoehen");

    return bChanged;
}


void ScTable::SetManualHeight( USHORT nStartRow, USHORT nEndRow, BOOL bManual )
{
    if (VALIDROW(nStartRow) && VALIDROW(nEndRow) && pRowFlags)
    {
        USHORT nRow;
        if (bManual)
            for (nRow=nStartRow; nRow<=nEndRow; nRow++)
                pRowFlags[nRow] |= CR_MANUALSIZE;
        else
            for (nRow=nStartRow; nRow<=nEndRow; nRow++)
                pRowFlags[nRow] &= ~CR_MANUALSIZE;
    }
    else
        DBG_ERROR("Falsche Zeilennummer oder keine Zeilenflags");
}


USHORT ScTable::GetColWidth( USHORT nCol ) const
{
    DBG_ASSERT(VALIDCOL(nCol),"Falsche Spaltennummer");

    if (VALIDCOL(nCol) && pColFlags && pColWidth)
    {
        if ( pColFlags[nCol] & CR_HIDDEN )
            return 0;
        else
            return pColWidth[nCol];
    }
    else
        return (USHORT) STD_COL_WIDTH;
}


USHORT ScTable::GetOriginalWidth( USHORT nCol ) const       // immer die eingestellte
{
    DBG_ASSERT(VALIDCOL(nCol),"Falsche Spaltennummer");

    if (VALIDCOL(nCol) && pColWidth)
        return pColWidth[nCol];
    else
        return (USHORT) STD_COL_WIDTH;
}


USHORT ScTable::GetRowHeight( USHORT nRow ) const
{
    DBG_ASSERT(VALIDROW(nRow),"Falsche Zeilennummer");

    if (VALIDROW(nRow) && pRowFlags && pRowHeight)
    {
        if ( pRowFlags[nRow] & CR_HIDDEN )
            return 0;
        else
            return pRowHeight[nRow];
    }
    else
        return (USHORT) ScGlobal::nStdRowHeight;
}


USHORT ScTable::GetOriginalHeight( USHORT nRow ) const      // non-0 even if hidden
{
    DBG_ASSERT(VALIDROW(nRow),"wrong row number");

    if (VALIDROW(nRow) && pRowHeight)
        return pRowHeight[nRow];
    else
        return (USHORT) ScGlobal::nStdRowHeight;
}


//  Spalten-/Zeilen-Flags


USHORT ScTable::GetHiddenRowCount( USHORT nRow ) const
{
    USHORT nEndRow = nRow;
    if ( pRowFlags )
    {
        while ( nEndRow <= MAXROW && ( pRowFlags[nEndRow] & CR_HIDDEN ) )
            ++nEndRow;
    }
    return nEndRow - nRow;
}


//!     ShowRows / DBShowRows zusammenfassen

void ScTable::ShowCol(USHORT nCol, BOOL bShow)
{
    if (VALIDCOL(nCol) && pColFlags)
    {
        BOOL bWasVis = ( pColFlags[nCol] & CR_HIDDEN ) == 0;
        if (bWasVis != bShow)
        {
            nRecalcLvl++;
            ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
            if (pDrawLayer)
            {
                if (bShow)
                    pDrawLayer->WidthChanged( nTab, nCol, (long) pColWidth[nCol] );
                else
                    pDrawLayer->WidthChanged( nTab, nCol, -(long) pColWidth[nCol] );
            }

            if (bShow)
                pColFlags[nCol] &= ~CR_HIDDEN;
            else
                pColFlags[nCol] |= CR_HIDDEN;
            if( !--nRecalcLvl )
                SetDrawPageSize();

            ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
            if ( pCharts && pCharts->GetCount() )
                pCharts->SetRangeDirty(ScRange( nCol, 0, nTab, nCol, MAXROW, nTab ));
        }
    }
    else
        DBG_ERROR("Falsche Spaltennummer oder keine Flags");
}


void ScTable::ShowRow(USHORT nRow, BOOL bShow)
{
    if (VALIDROW(nRow) && pRowFlags)
    {
        BOOL bWasVis = ( pRowFlags[nRow] & CR_HIDDEN ) == 0;
        if (bWasVis != bShow)
        {
            nRecalcLvl++;
            ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
            if (pDrawLayer)
            {
                if (bShow)
                    pDrawLayer->HeightChanged( nTab, nRow, (long) pRowHeight[nRow] );
                else
                    pDrawLayer->HeightChanged( nTab, nRow, -(long) pRowHeight[nRow] );
            }

            if (bShow)
                pRowFlags[nRow] &= ~(CR_HIDDEN | CR_FILTERED);
            else
                pRowFlags[nRow] |= CR_HIDDEN;
            if( !--nRecalcLvl )
                SetDrawPageSize();

            ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
            if ( pCharts && pCharts->GetCount() )
                pCharts->SetRangeDirty(ScRange( 0, nRow, nTab, MAXCOL, nRow, nTab ));
        }
    }
    else
        DBG_ERROR("Falsche Zeilennummer oder keine Flags");
}


void ScTable::DBShowRow(USHORT nRow, BOOL bShow)
{
    if (VALIDROW(nRow) && pRowFlags)
    {
        BOOL bWasVis = ( pRowFlags[nRow] & CR_HIDDEN ) == 0;
        nRecalcLvl++;
        if (bWasVis != bShow)
        {
            ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
            if (pDrawLayer)
            {
                if (bShow)
                    pDrawLayer->HeightChanged( nTab, nRow, (long) pRowHeight[nRow] );
                else
                    pDrawLayer->HeightChanged( nTab, nRow, -(long) pRowHeight[nRow] );
            }
        }

        //  Filter-Flag immer setzen, auch wenn Hidden unveraendert
        if (bShow)
            pRowFlags[nRow] &= ~(CR_HIDDEN | CR_FILTERED);
        else
            pRowFlags[nRow] |= (CR_HIDDEN | CR_FILTERED);
        if( !--nRecalcLvl )
            SetDrawPageSize();

        if (bWasVis != bShow)
        {
            ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
            if ( pCharts && pCharts->GetCount() )
                pCharts->SetRangeDirty(ScRange( 0, nRow, nTab, MAXCOL, nRow, nTab ));
        }
    }
    else
        DBG_ERROR("Falsche Zeilennummer oder keine Flags");
}


void ScTable::DBShowRows(USHORT nRow1, USHORT nRow2, BOOL bShow)
{
    USHORT i;
    USHORT nStartRow = nRow1;
    nRecalcLvl++;
    while (nStartRow <= nRow2)
    {
        USHORT nEndRow = nStartRow;
        BYTE nOldFlag = pRowFlags[nStartRow] & CR_HIDDEN;
        while ( nEndRow < nRow2 && (pRowFlags[nEndRow+1] & CR_HIDDEN) == nOldFlag )
            ++nEndRow;

        BOOL bWasVis = ( nOldFlag == 0 );
        BOOL bChanged = ( bWasVis != bShow );
        if ( bChanged )
        {
            ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
            if (pDrawLayer)
            {
                long nHeight = 0;
                for (i=nStartRow; i<=nEndRow; i++)
                    nHeight += pRowHeight[i];

                if (bShow)
                    pDrawLayer->HeightChanged( nTab, nStartRow, nHeight );
                else
                    pDrawLayer->HeightChanged( nTab, nStartRow, -nHeight );
            }
        }

        if (bShow)
            for (i=nStartRow; i<=nEndRow; i++)
                pRowFlags[i] &= ~(CR_HIDDEN | CR_FILTERED);
        else
            for (i=nStartRow; i<=nEndRow; i++)
                pRowFlags[i] |= (CR_HIDDEN | CR_FILTERED);

        if ( bChanged )
        {
            ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
            if ( pCharts && pCharts->GetCount() )
                pCharts->SetRangeDirty(ScRange( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab ));
        }

        nStartRow = nEndRow + 1;
    }
    if( !--nRecalcLvl )
        SetDrawPageSize();
}


void ScTable::ShowRows(USHORT nRow1, USHORT nRow2, BOOL bShow)
{
    USHORT i;
    USHORT nStartRow = nRow1;
    nRecalcLvl++;
    while (nStartRow <= nRow2)
    {
        USHORT nEndRow = nStartRow;
        BYTE nOldFlag = pRowFlags[nStartRow] & CR_HIDDEN;
        while ( nEndRow < nRow2 && (pRowFlags[nEndRow+1] & CR_HIDDEN) == nOldFlag )
            ++nEndRow;

        BOOL bWasVis = ( nOldFlag == 0 );
        BOOL bChanged = ( bWasVis != bShow );
        if ( bChanged )
        {
            ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
            if (pDrawLayer)
            {
                long nHeight = 0;
                for (i=nStartRow; i<=nEndRow; i++)
                    nHeight += pRowHeight[i];

                if (bShow)
                    pDrawLayer->HeightChanged( nTab, nStartRow, nHeight );
                else
                    pDrawLayer->HeightChanged( nTab, nStartRow, -nHeight );
            }
        }

        if (bShow)
            for (i=nStartRow; i<=nEndRow; i++)
                pRowFlags[i] &= ~(CR_HIDDEN | CR_FILTERED);
        else
            for (i=nStartRow; i<=nEndRow; i++)
                pRowFlags[i] |= CR_HIDDEN;

        if ( bChanged )
        {
            ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
            if ( pCharts && pCharts->GetCount() )
                pCharts->SetRangeDirty(ScRange( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab ));
        }

        nStartRow = nEndRow + 1;
    }
    if( !--nRecalcLvl )
        SetDrawPageSize();
}


BOOL ScTable::IsFiltered(USHORT nRow) const
{
    if (VALIDROW(nRow) && pRowFlags)
        return ( pRowFlags[nRow] & CR_FILTERED ) != 0;

    DBG_ERROR("Falsche Zeilennummer oder keine Flags");
    return FALSE;
}


void ScTable::SetColFlags( USHORT nCol, BYTE nNewFlags )
{
    if (VALIDCOL(nCol) && pColFlags)
        pColFlags[nCol] = nNewFlags;
    else
        DBG_ERROR("Falsche Spaltennummer oder keine Flags");
}


void ScTable::SetRowFlags( USHORT nRow, BYTE nNewFlags )
{
    if (VALIDROW(nRow) && pRowFlags)
        pRowFlags[nRow] = nNewFlags;
    else
        DBG_ERROR("Falsche Zeilennummer oder keine Flags");
}


BYTE ScTable::GetColFlags( USHORT nCol ) const
{
    if (VALIDCOL(nCol) && pColFlags)
        return pColFlags[nCol];
    else
        return 0;
}


BYTE ScTable::GetRowFlags( USHORT nRow ) const
{
    if (VALIDROW(nRow) && pRowFlags)
        return pRowFlags[nRow];
    else
        return 0;
}


USHORT ScTable::GetLastFlaggedCol() const
{
    if ( !pColFlags )
        return 0;

    USHORT nLastFound = 0;
    for (USHORT nCol = 1; nCol <= MAXCOL; nCol++)
        if ((pColFlags[nCol] & ~CR_PAGEBREAK) || (pColWidth[nCol] != STD_COL_WIDTH))
            nLastFound = nCol;

    return nLastFound;
}


USHORT ScTable::GetLastFlaggedRow() const
{
    if ( !pRowFlags )
        return 0;

    USHORT nLastFound = 0;
    for (USHORT nRow = 1; nRow <= MAXROW; nRow++)
        if ((pRowFlags[nRow] & ~CR_PAGEBREAK) || (pRowHeight[nRow] != ScGlobal::nStdRowHeight))
            nLastFound = nRow;

    return nLastFound;
}


BOOL ScTable::UpdateOutlineCol( USHORT nStartCol, USHORT nEndCol, BOOL bShow )
{
    if (pOutlineTable && pColFlags)
        return pOutlineTable->GetColArray()->ManualAction( nStartCol, nEndCol, bShow, pColFlags );
    else
        return FALSE;
}


BOOL ScTable::UpdateOutlineRow( USHORT nStartRow, USHORT nEndRow, BOOL bShow )
{
    if (pOutlineTable && pRowFlags)
        return pOutlineTable->GetRowArray()->ManualAction( nStartRow, nEndRow, bShow, pRowFlags );
    else
        return FALSE;
}


void ScTable::ExtendHidden( USHORT& rX1, USHORT& rY1, USHORT& rX2, USHORT& rY2 )
{
    if (pColFlags)
    {
        while ( rX1>0 ? (pColFlags[rX1-1] & CR_HIDDEN) : FALSE )
            --rX1;
        while ( rX2<MAXCOL ? (pColFlags[rX2+1] & CR_HIDDEN) : FALSE )
            ++rX2;
    }
    if (pRowFlags)
    {
        while ( rY1>0 ? (pRowFlags[rY1-1] & CR_HIDDEN) : FALSE )
            --rY1;
        while ( rY2<MAXROW ? (pRowFlags[rY2+1] & CR_HIDDEN) : FALSE )
            ++rY2;
    }
}


void ScTable::StripHidden( USHORT& rX1, USHORT& rY1, USHORT& rX2, USHORT& rY2 )
{
    if (pColFlags)
    {
        while ( rX2>rX1 && (pColFlags[rX2] & CR_HIDDEN) )
            --rX2;
        while ( rX2>rX1 && (pColFlags[rX1] & CR_HIDDEN) )
            ++rX1;
    }
    if (pRowFlags)
    {
        while ( rY2>rY1 && (pRowFlags[rY2] & CR_HIDDEN) )
            --rY2;
        while ( rY2>rY1 && (pRowFlags[rY1] & CR_HIDDEN) )
            ++rY1;
    }
}


//  Auto-Outline

inline short DiffSign( USHORT a, USHORT b )
{
    return (a<b) ? -1 :
            (a>b) ? 1 : 0;
}


void ScTable::DoAutoOutline( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow )
{
    BOOL bSizeChanged = FALSE;
    BOOL bMissed      = FALSE;

    USHORT nCol;
    USHORT nRow;
    USHORT i;
    BOOL bFound;
    ScOutlineArray* pArray;
    ScBaseCell* pCell;
    ScRange aRef;
/*  ScPatternAttr aBoldPattern( pDocument->GetPool() );             //! spezielle Format-Vorlage
    aBoldPattern.GetItemSet().Put( SvxWeightItem( WEIGHT_BOLD ) );
*/

    StartOutlineTable();

                            // Zeilen

    USHORT  nCount = nEndRow-nStartRow+1;
    BOOL*   pUsed = new BOOL[nCount];
    for (i=0; i<nCount; i++)
        pUsed[i] = FALSE;
    for (nCol=nStartCol; nCol<=nEndCol; nCol++)
        if (!aCol[nCol].IsEmptyData())
            aCol[nCol].FindUsed( nStartRow, nEndRow, pUsed );

    pArray = pOutlineTable->GetRowArray();
    for (nRow=nStartRow; nRow<=nEndRow; nRow++)
        if (pUsed[nRow-nStartRow])
        {
            bFound = FALSE;
            for (nCol=nStartCol; nCol<=nEndCol && !bFound; nCol++)
                if (!aCol[nCol].IsEmptyData())
                {
                    pCell = aCol[nCol].GetCell( nRow );
                    if (pCell)
                        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                            if (((ScFormulaCell*)pCell)->HasOneReference( aRef ))
                                if ( aRef.aStart.Col() == nCol && aRef.aEnd.Col() == nCol &&
                                     aRef.aStart.Tab() == nTab && aRef.aEnd.Tab() == nTab &&
                                     DiffSign( aRef.aStart.Row(), nRow ) ==
                                        DiffSign( aRef.aEnd.Row(), nRow ) )
                                {
                                    if (pArray->Insert( aRef.aStart.Row(), aRef.aEnd.Row(), bSizeChanged ))
                                    {
//                                      ApplyPatternArea( nStartCol, nRow, nEndCol, nRow, aBoldPattern );
                                        bFound = TRUE;
                                    }
                                    else
                                        bMissed = TRUE;
                                }
                }
        }

    delete[] pUsed;

                            // Spalten

    pArray = pOutlineTable->GetColArray();
    for (nCol=nStartCol; nCol<=nEndCol; nCol++)
    {
        if (!aCol[nCol].IsEmptyData())
        {
            bFound = FALSE;
            ScColumnIterator aIter( &aCol[nCol], nStartRow, nEndRow );
            while ( aIter.Next( nRow, pCell ) && !bFound )
            {
                if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                    if (((ScFormulaCell*)pCell)->HasOneReference( aRef ))
                        if ( aRef.aStart.Row() == nRow && aRef.aEnd.Row() == nRow &&
                             aRef.aStart.Tab() == nTab && aRef.aEnd.Tab() == nTab &&
                             DiffSign( aRef.aStart.Col(), nCol ) ==
                                DiffSign( aRef.aEnd.Col(), nCol ) )
                        {
                            if (pArray->Insert( aRef.aStart.Col(), aRef.aEnd.Col(), bSizeChanged ))
                            {
//                              ApplyPatternArea( nCol, nStartRow, nCol, nEndRow, aBoldPattern );
                                bFound = TRUE;
                            }
                            else
                                bMissed = TRUE;
                        }
            }
        }
    }
}

//
//  Datei-Operationen
//

//  Laden

BOOL ScTable::Load( SvStream& rStream, USHORT nVersion, ScProgress* pProgress )
{
    ScReadHeader aHdr( rStream );
    USHORT i;

    while (aHdr.BytesLeft() && rStream.GetError() == SVSTREAM_OK)
    {
        USHORT nID;
        rStream >> nID;
        switch (nID)
        {
            case SCID_COLUMNS:                      //  Spalten (Daten)
                {
                    ScMultipleReadHeader aColHdr( rStream );
                    if( nVersion >= SC_DATABYTES )
                      while( aColHdr.BytesLeft() )
                    {
                        BYTE nCol;
                        rStream >> nCol;
                        aCol[ nCol ].Load(rStream, aColHdr);
                        if (pProgress)
                            pProgress->SetState( rStream.Tell() );
                    }
                    else
                      for (i=0; i<=MAXCOL; i++)
                    {
                        aCol[i].Load(rStream, aColHdr);
                        if (pProgress)
                            pProgress->SetState( rStream.Tell() );
                    }
                }
                break;
            case SCID_COLROWFLAGS:                  //  Spalten-/Zeilenflags
                {
                    ScReadHeader aFlagsHdr( rStream );

                    BYTE    nFlags;
                    USHORT  nVal;
                    short   nRep;

                    i=0;
                    while (i<=MAXCOL)
                    {
                        rStream >> nRep;
                        rStream >> nVal;
                        while (nRep-- && i<=MAXCOL)
                            pColWidth[i++] = nVal;
                    }
                    DBG_ASSERT(nRep==-1, "Fehler bei Spaltenbreiten");
                    i=0;
                    while (i<=MAXCOL)
                    {
                        rStream >> nRep;
                        rStream >> nFlags;
                        while (nRep-- && i<=MAXCOL)
                            pColFlags[i++] = nFlags;
                    }
                    DBG_ASSERT(nRep==-1, "Fehler bei Spaltenflags");
                    if (pProgress)
                        pProgress->SetState( rStream.Tell() );

                    //  aus der Datei immer soviele Zeilen laden, wie gespeichert wurden
                    USHORT nSrcMaxRow = pDocument->GetSrcMaxRow();

                    i=0;
                    while (i<=nSrcMaxRow)       // nSrcMaxRow und MAXROW evtl. unterschiedlich
                    {
                        rStream >> nRep;
                        rStream >> nVal;
                        while (nRep--)
                        {
                            if (i<=MAXROW)
                                pRowHeight[i] = nVal;
                            ++i;
                        }
                    }
                    DBG_ASSERT(i==nSrcMaxRow+1, "Fehler bei Zeilenhoehen");
                    if (pProgress)
                        pProgress->SetState( rStream.Tell() );
                    i=0;
                    while (i<=nSrcMaxRow)       // nSrcMaxRow und MAXROW evtl. unterschiedlich
                    {
                        rStream >> nRep;
                        rStream >> nFlags;
                        while (nRep--)
                        {
                            if (i<=MAXROW)
                                pRowFlags[i] = nFlags;
                            ++i;
                        }
                    }
                    DBG_ASSERT(i==nSrcMaxRow+1, "Fehler bei Zeilenflags");
                    if (pProgress)
                        pProgress->SetState( rStream.Tell() );
                }
                break;
            case SCID_TABOPTIONS:                   //  einzelne Einstellungen
                {
                    ScReadHeader aFlagsHdr( rStream );

                    rStream.ReadByteString( aName, rStream.GetStreamCharSet() );

                    rStream >> bScenario;
                    rStream.ReadByteString( aComment, rStream.GetStreamCharSet() );

                    rStream >> bProtected;
                    String aPass;
                    rStream.ReadByteString( aPass, rStream.GetStreamCharSet() );
                    if (aPass.Len())
                        SvPasswordHelper::GetHashPassword(aProtectPass, aPass);

                    BOOL bOutline;
                    rStream >> bOutline;
                    if (bOutline)
                    {
                        StartOutlineTable();
                        pOutlineTable->Load( rStream );
                    }

                    if ( aFlagsHdr.BytesLeft() )
                    {
                        SfxStyleSheetBasePool* pStylePool =
                                pDocument->GetStyleSheetPool();

                        rStream.ReadByteString( aPageStyle, rStream.GetStreamCharSet() );

                        if ( !pStylePool->Find( aPageStyle, SFX_STYLE_FAMILY_PAGE ) )
                        {
                            DBG_TRACE( "PageStyle not found. Using Standard." );
                            aPageStyle = ScGlobal::GetRscString(STR_STYLENAME_STANDARD);
                        }
                    }

                    if ( aFlagsHdr.BytesLeft() )
                    {
                        BOOL bOneRange;                     // einzelner Druckbereich ?
                        rStream >> bOneRange;
                        if ( bOneRange )
                        {
                            ScRange aRange;
                            rStream >> aRange;
                            SetPrintRangeCount( 1 );
                            SetPrintRange( 0, aRange );
                        }

                        lcl_LoadRange( rStream, &pRepeatColRange );
                        lcl_LoadRange( rStream, &pRepeatRowRange );
                    }

                    if ( aFlagsHdr.BytesLeft() )
                        rStream >> bVisible;

                    if ( aFlagsHdr.BytesLeft() )            // Druckbereiche ab Version 314c
                    {
                        USHORT nNewCount;
                        rStream >> nNewCount;
                        if ( nNewCount )
                        {
                            ScRange aTmp;
                            SetPrintRangeCount( nNewCount );
                            for (i=0; i<nNewCount; i++)
                            {
                                rStream >> aTmp;
                                SetPrintRange( i, aTmp );
                            }
                        }
                    }

                    if ( aFlagsHdr.BytesLeft() )            // erweiterte Szenario-Flags ab 5.0
                    {
                        rStream >> aScenarioColor;
                        rStream >> nScenarioFlags;
                        rStream >> bActiveScenario;
                    }
                    else if ( bScenario )
                    {
                        //  Default fuer Szenarien aus alten Dateien??
                        //  Wenn die alten Szenarien wirklich benutzt wurden,
                        //  wuerde der Rahmen wahrscheinlich stoeren.

                        nScenarioFlags = SC_SCENARIO_COPYALL;
                    }

                    if (pProgress)
                        pProgress->SetState( rStream.Tell() );
                }
                break;
            case SCID_TABLINK:                      //  Verknuepfung
                {
                    ScReadHeader aLinkHdr( rStream );

                    rStream >> nLinkMode;
                    rStream.ReadByteString( aLinkDoc, rStream.GetStreamCharSet() );
                    aLinkDoc = INetURLObject::RelToAbs( aLinkDoc );
                    rStream.ReadByteString( aLinkFlt, rStream.GetStreamCharSet() );
                    rStream.ReadByteString( aLinkTab, rStream.GetStreamCharSet() );

                    BOOL bRelURL;
                    if ( aLinkHdr.BytesLeft() )
                        rStream >> bRelURL;
                    else
                        bRelURL = FALSE;
                    // externer Tabellenname relativ zu absolut
                    if ( nLinkMode == SC_LINK_VALUE && bRelURL )
                        aName = ScGlobal::GetDocTabName( aLinkDoc, aLinkTab );

                    if ( aLinkHdr.BytesLeft() )     // ab 336 auch Filter-Optionen
                        rStream.ReadByteString( aLinkOpt, rStream.GetStreamCharSet() );
                }
                break;
            default:
                {
                    DBG_ERROR("unbekannter Sub-Record in ScTable::Load");
                    ScReadHeader aDummyHeader( rStream );
                }
        }
    }

    //  Neuberechnungen

    ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
    if (pDrawLayer)
        pDrawLayer->ScRenamePage( nTab, aName );

    return TRUE;
}

//  Speichern


void lcl_SaveValue( SvStream& rStream, USHORT* pValue, USHORT nEnd )
{
    USHORT nPos = 0;
    while (nPos<=nEnd)
    {
        USHORT nVal = pValue[nPos];
        USHORT nNextPos = nPos+1;
        while (nNextPos<=nEnd && pValue[nNextPos]==nVal)
            ++nNextPos;
        rStream << (USHORT)( nNextPos - nPos );
        rStream << nVal;
        nPos = nNextPos;
    }
}


void lcl_SaveFlags( SvStream& rStream, BYTE* pValue, USHORT nEnd )
{
    USHORT nPos = 0;
    while (nPos<=nEnd)
    {
        BYTE nVal = pValue[nPos] & CR_SAVEMASK;
        USHORT nNextPos = nPos+1;
        while (nNextPos<=nEnd && (pValue[nNextPos] & CR_SAVEMASK)==nVal)
            ++nNextPos;
        rStream << (USHORT)( nNextPos - nPos );
        rStream << nVal;
        nPos = nNextPos;
    }
}


void lcl_LoadRange( SvStream& rStream, ScRange** ppRange )
{
    BOOL bIsSet = FALSE;

    rStream >> bIsSet;

    if ( bIsSet )
    {
        *ppRange = new ScRange;
        rStream >> **ppRange;
    }
    else
        *ppRange = NULL;
}


void lcl_SaveRange( SvStream& rStream, ScRange* pRange )
{
    if ( pRange )
    {
        rStream << (BOOL)TRUE;
        rStream << *pRange;
    }
    else
        rStream << (BOOL)FALSE;
}


BOOL ScTable::Save( SvStream& rStream, long& rSavedDocCells, ScProgress* pProgress ) const
{
    ScWriteHeader aHdr( rStream );
    USHORT i;

    //  Spalten (Daten)

    {
        rStream << (USHORT) SCID_COLUMNS;
        ScMultipleWriteHeader aColHdr( rStream );

        for (i=0; i<=MAXCOL; i++)
        {
            const ScColumn* pCol = &aCol[ i ];
            if( !pCol->IsEmptyData() || pCol->NoteCount() || !pCol->IsEmptyAttr())
            {
                rStream << (BYTE) i;
                aCol[i].Save(rStream, aColHdr);
                rSavedDocCells += aCol[i].GetWeightedCount();
                if (pProgress)
                    pProgress->SetState( rSavedDocCells );
            }
        }
    }

    //  Spalten-/Zeilenflags

    {
        rStream << (USHORT) SCID_COLROWFLAGS;
        ScWriteHeader aFlagsHdr( rStream );

        USHORT nSaveMaxRow = pDocument->GetSrcMaxRow();

        lcl_SaveValue( rStream, pColWidth, MAXCOL );
        lcl_SaveFlags( rStream, pColFlags, MAXCOL );
        lcl_SaveValue( rStream, pRowHeight,nSaveMaxRow );
        lcl_SaveFlags( rStream, pRowFlags, nSaveMaxRow );

        //  wenn dabei weniger Zeilenhoehen als vorhanden gespeichert wurden,
        //  gibt das noch keine Warnung wegen Datenverlust.
    }

    BOOL bRelURL = FALSE;
    String aLinkDocSaveName( aLinkDoc );
    String aSaveName( aName );
    if ( nLinkMode )
    {
        aLinkDocSaveName = INetURLObject::AbsToRel( aLinkDocSaveName );
        aLinkDocSaveName = INetURLObject::decode( aLinkDocSaveName,
            INET_HEX_ESCAPE, INetURLObject::DECODE_UNAMBIGUOUS );
        if ( ScGlobal::pCollator->compareString(
                aLinkDocSaveName,
                INetURLObject::decode( aLinkDoc, INET_HEX_ESCAPE,
                INetURLObject::DECODE_UNAMBIGUOUS ) ) == COMPARE_EQUAL )
        {
            aSaveName = INetURLObject::decode( aSaveName,
                INET_HEX_ESCAPE, INetURLObject::DECODE_UNAMBIGUOUS );
        }
        else
        {
            bRelURL = TRUE;
            // Reference to external sheet, only the sheet name is stored
            // instead of the absolute DocTabName, will be reconcatenated upon
            // load time.
            if ( nLinkMode == SC_LINK_VALUE )
                aSaveName = aLinkTab;
        }
    }

    //  einzelne Einstellungen

    {
        rStream << (USHORT) SCID_TABOPTIONS;
        ScWriteHeader aFlagsHdr( rStream );

        rStream.WriteByteString( aSaveName, rStream.GetStreamCharSet() );

        rStream << bScenario;
        rStream.WriteByteString( aComment, rStream.GetStreamCharSet() );

        rStream << bProtected;
        String aPass;
        //rStream.WriteByteString( aProtectPass, rStream.GetStreamCharSet() );
        rStream.WriteByteString( aPass, rStream.GetStreamCharSet() );

        BOOL bOutline = ( pOutlineTable != NULL );
        rStream << bOutline;
        if (bOutline)
            pOutlineTable->Store( rStream );

        rStream.WriteByteString( aPageStyle, rStream.GetStreamCharSet() );

        if ( pPrintRanges && nPrintRangeCount == 1 )        // kompatibel zu alten Versionen
            lcl_SaveRange( rStream, pPrintRanges );         // (nur wenn genau ein Bereich)
        else
            lcl_SaveRange( rStream, NULL );
        lcl_SaveRange( rStream, pRepeatColRange );
        lcl_SaveRange( rStream, pRepeatRowRange );

        rStream << bVisible;

        if ( pPrintRanges && nPrintRangeCount>1 )           // einzelner Bereich schon oben
        {
            rStream << nPrintRangeCount;                    // ab Version 314c
            if ( nPrintRangeCount > 1 )
                for ( i=0; i<nPrintRangeCount; i++)
                    rStream << pPrintRanges[i];
        }
        else
            rStream << (USHORT) 0;

        if( rStream.GetVersion() > SOFFICE_FILEFORMAT_40 )  // erweiterte Szenario-Flags ab 5.0
        {
            rStream << aScenarioColor;
            rStream << nScenarioFlags;
            rStream << bActiveScenario;
        }
    }

    //  Verknuepfung

    if (nLinkMode)
    {
        rStream << (USHORT) SCID_TABLINK;
        ScWriteHeader aLinkHdr( rStream );

        //  Hack, um verknuepfte Tabellen mit der 3.1 laden zu koennen (#35242#)
        //  bei 3.1 Export Filter von "StarCalc 4.0" auf "StarCalc 3.0" umsetzen
        //  (4.0 Dateien koennen vom Calc 3.1 gelesen werden)
        String aSaveFlt = aLinkFlt;
        if ( rStream.GetVersion() == SOFFICE_FILEFORMAT_31 && aSaveFlt.EqualsAscii("StarCalc 4.0") )
            aSaveFlt.AssignAscii(RTL_CONSTASCII_STRINGPARAM("StarCalc 3.0"));

        rStream << nLinkMode;
        rStream.WriteByteString( aLinkDocSaveName, rStream.GetStreamCharSet() );
        rStream.WriteByteString( aSaveFlt, rStream.GetStreamCharSet() );
        rStream.WriteByteString( aLinkTab, rStream.GetStreamCharSet() );
        rStream << bRelURL;
        rStream.WriteByteString( aLinkOpt, rStream.GetStreamCharSet() );
    }

    return TRUE;
}


                                    //  CopyData - fuer Query in anderen Bereich

void ScTable::CopyData( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                            USHORT nDestCol, USHORT nDestRow, USHORT nDestTab )
{
    //!     wenn fuer mehrere Zeilen benutzt, nach Spalten optimieren!

    ScAddress aSrc( nStartCol, nStartRow, nTab );
    ScAddress aDest( nDestCol, nDestRow, nDestTab );
    ScRange aRange( aSrc, aDest );
    BOOL bThisTab = ( nDestTab == nTab );
    USHORT nDestY = nDestRow;
    for (USHORT nRow=nStartRow; nRow<=nEndRow; nRow++)
    {
        aSrc.SetRow( nRow );
        aDest.SetRow( nDestY );
        USHORT nDestX = nDestCol;
        for (USHORT nCol=nStartCol; nCol<=nEndCol; nCol++)
        {
            aSrc.SetCol( nCol );
            aDest.SetCol( nDestX );
            ScBaseCell* pCell = GetCell( nCol, nRow );
            if (pCell)
            {
                pCell = pCell->Clone(pDocument);
                if (pCell->GetCellType() == CELLTYPE_FORMULA)
                {
                    ((ScFormulaCell*)pCell)->UpdateReference( URM_COPY, aRange,
                                    ((short) nDestCol) - ((short) nStartCol),
                                    ((short) nDestRow) - ((short) nStartRow),
                                    ((short) nDestTab) - ((short) nTab) );
                    ((ScFormulaCell*)pCell)->aPos = aDest;
                }
            }
            if (bThisTab)
            {
                PutCell( nDestX, nDestY, pCell );
                SetPattern( nDestX, nDestY, *GetPattern( nCol, nRow ), TRUE );
            }
            else
            {
                pDocument->PutCell( aDest, pCell );
                pDocument->SetPattern( aDest, *GetPattern( nCol, nRow ), TRUE );
            }

            ++nDestX;
        }
        ++nDestY;
    }
}


BOOL ScTable::RefVisible(ScFormulaCell* pCell)
{
    ScRange aRef;

    if (pCell->HasOneReference(aRef))
    {
        if (aRef.aStart.Col()==aRef.aEnd.Col() && aRef.aStart.Tab()==aRef.aEnd.Tab() && pRowFlags)
        {
            for (USHORT nRow=aRef.aStart.Row(); nRow<=aRef.aEnd.Row(); nRow++)
                if ( (pRowFlags[nRow] & CR_FILTERED) == 0 )
                    return TRUE;                                        // Teil sichtbar
            return FALSE;                                               // alles unsichtbar
        }
    }

    return TRUE;                        // irgendwie anders
}


void ScTable::GetUpperCellString(USHORT nCol, USHORT nRow, String& rStr)
{
    GetInputString(nCol, nRow, rStr);
    rStr.EraseTrailingChars();
    rStr.EraseLeadingChars();
    ScGlobal::pCharClass->toUpper(rStr);
}


// Berechnen der Groesse der Tabelle und setzen der Groesse an der DrawPage

void ScTable::SetDrawPageSize()
{
    ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
    if( pDrawLayer )
    {
        ULONG x = GetColOffset( MAXCOL + 1 );
        ULONG y = GetRowOffset( MAXROW + 1 );
        x = (ULONG) ((double) x * HMM_PER_TWIPS);
        y = (ULONG) ((double) y * HMM_PER_TWIPS);
        pDrawLayer->SetPageSize( nTab, Size( x, y ) );
    }
}


ULONG ScTable::GetRowOffset( USHORT nRow ) const
{
    ULONG n = 0;
    if ( pRowFlags && pRowHeight )
    {
        USHORT i;
        BYTE* pFlags = pRowFlags;
        USHORT* pHeight = pRowHeight;
        for( i = 0; i < nRow; i++, pFlags++, pHeight++ )
            if( !( *pFlags & CR_HIDDEN ) )
                n += *pHeight;
    }
    else
        DBG_ERROR("GetRowOffset: Daten fehlen");
    return n;
}


ULONG ScTable::GetColOffset( USHORT nCol ) const
{
    ULONG n = 0;
    if ( pColFlags && pColWidth )
    {
        USHORT i;
        BYTE* pFlags = pColFlags;
        USHORT* pWidth = pColWidth;
        for( i = 0; i < nCol; i++, pFlags++, pWidth++ )
            if( !( *pFlags & CR_HIDDEN ) )
                n += *pWidth;
    }
    else
        DBG_ERROR("GetColumnOffset: Daten fehlen");
    return n;
}

