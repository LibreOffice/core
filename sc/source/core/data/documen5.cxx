/*************************************************************************
 *
 *  $RCSfile: documen5.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:14 $
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

#pragma optimize("",off)
#pragma optimize("q",off) // p-code off

//------------------------------------------------------------------------

#define _SV_NOXSOUND

#define _BASDLG_HXX
#define _CACHESTR_HXX
#define _CURSOR_HXX
#define _CTRLTOOL_HXX
#define _DLGCFG_HXX
#define _EXTATTR_HXX
#define _FILDLG_HXX
#define _FRM3D_HXX
#define _INTRO_HXX
#define _ISETBWR_HXX
#define _NO_SVRTF_PARSER_HXX
#define _MACRODLG_HXX
#define _MODALDLG_HXX
#define _MOREBUTTON_HXX
#define _OUTLINER_HXX
#define _PASSWD_HXX

#define _PVRWIN_HXX

//#define _PRNDLG_HXX
//#define _POLY_HXX
#define _QUEUE_HXX
#define _RULER_HXX
#define _SCRWIN_HXX
#define _SETBRW_HXX
//#define _STATUS_HXX
#define _STDMENU_HXX
#define _TABBAR_HXX

// TOOLS
#define _BIGINT_HXX
#define _SFXMULTISEL_HXX
#define _STACK_HXX
#define _QUEUE_HXX
#define _DYNARR_HXX
//#define _TREELIST_HXX
#define _CACHESTR_HXX

//SV
#define _CLIP_HXX
//#define _CONFIG_HXX
#define _CURSOR_HXX
#define _FONTDLG_HXX
#define _PRVWIN_HXX

#ifndef OS2
#define _MENUBTN_HXX
#endif


#define _SFX_DOCFILE_HXX
#define _SFX_DOCFILT_HXX
#define _SFX_DOCINF_HXX
#define _SFX_DOCSH_HXX
#define _SFX_INTERNO_HXX
#define _SFX_PRNMON_HXX
#define _SFX_RESMGR_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXAPPWIN_HXX
#define _SFXBASIC_HXX
#define _SFXCTRLITEM
#define _SFXDLGCFG_HXX
#define _SFXDISPATCH_HXX
#define _SFXDOCFILE_HXX
#define _SFXDOCMAN_HXX
#define _SFXDOCMGR_HXX
#define _SFXDOCTDLG_HXX
#define _SFXFILEDLG_HXX
#define _SFXIMGMGR_HXX
#define _SFXIPFRM_HXX
#define _SFX_MACRO_HXX
#define _SFXMNUITEM_HXX
#define _SFXMNUMGR_HXX
#define _SFXMULTISEL_HXX
#define _SFXMSG_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFX_MINFITEM_HXX
#define _SFXOBJFACE_HXX
#define _SFXOBJFAC_HXX
#define _SFX_SAVEOPT_HXX
#define _SFXSTBITEM_HXX
//#define _SFXSTBMGR_HXX
#define _SFXTBXCTRL_HXX
#define _SFXTBXMGR_HXX

#define _SFXGENLINK_HXX
#define _SFXHINTPOST_HXX
#define _SFXLINKHDL_HXX
#define _XTABLE_HXX
#define SFX_NOCLOOKS

#define _SIDLL_HXX
#define _SI_NOITEMS_HXX
#define _SI_NODRW_HXX
#define _SI_NOCONTROL_HXX
#define _SI_NOOTHERFORMS_HXX
#define _SI_NOSBXCONTROLS_HXX
#define _VCBRW_HXX
#define _VCTRLS_HXX
//#define _VCSBX_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX

#define _SVBOXITM_HXX
#define _SVCONTNR_HXX
//#define _SVDATTR_HXX
#define _SVDXOUT_HXX
#define _SVDEC_HXX
#define _SVDIALDLL_HXX
#define _SVDIO_HXX //*
//#define _SVDLAYER_HXX ***
#define _SVDRAG_HXX
#define _SVINCVW_HXX
#define _SV_MULTISEL_HXX
#define _SVRTV_HXX
#define _SVTABBX_HXX
#define _SVTREEBOX_HXX
#define _SVTREELIST_HXX


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

//svdraw.hxx
#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
#define _SDR_NOXOUTDEV
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM        // Transformationen, selten verwendet
#define _SDR_NOTOUCH            // Hit-Tests, selten verwendet

#define _SDR_NOEXTDEV           // ExtOutputDevice
//#define   _SDR_NOUNDO             // Undo-Objekte
#define _SDR_NOSURROGATEOBJ     // SdrObjSurrogate
#define _SDR_NOPAGEOBJ          // SdrPageObj
#define _SDR_NOVIRTOBJ          // SdrVirtObj
#define _SDR_NOGROUPOBJ         // SdrGroupObj
#define _SDR_NOTEXTOBJ          // SdrTextObj
#define _SDR_NOPATHOBJ          // SdrPathObj
#define _SDR_NOEDGEOBJ          // SdrEdgeObj
#define _SDR_NORECTOBJ          // SdrRectObj
#define _SDR_NOCAPTIONOBJ       // SdrCaptionObj
#define _SDR_NOCIRCLEOBJ        // SdrCircleObj
#define _SDR_NOGRAFOBJ          // SdrGrafObj
#define _SDR_NOOLE2OBJ          // SdrOle2Obj




// INCLUDE ---------------------------------------------------------------

#include <sfx2/objsh.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <sch/schdll.hxx>
#include <sch/memchrt.hxx>
#include <sch/schdll0.hxx>
#include <so3/ipobj.hxx>

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif

#include "document.hxx"
#include "drwlayer.hxx"
#include "chartarr.hxx"
#include "chartlis.hxx"
#include "refupdat.hxx"

// -----------------------------------------------------------------------

        // Charts aus altem Dokument updaten

void ScDocument::UpdateAllCharts(BOOL bDoUpdate)
{
    if (!pDrawLayer)
        return;

    USHORT nDataCount = pChartCollection->GetCount();
    if ( !nDataCount )
        return ;        // nothing to do

    USHORT nPos;

    for (USHORT nTab=0; nTab<=MAXTAB; nTab++)
    {
        if (pTab[nTab])
        {
            SdrPage* pPage = pDrawLayer->GetPage(nTab);
            DBG_ASSERT(pPage,"Page ?");

            ScRange aRange;
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
                {
                    SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                    if (aIPObj.Is())
                    {
                        // String aIPName = aIPObj->GetName()->GetName();

                        SvInfoObject* pInfoObj = pShell->Find( aIPObj );
                        String aIPName;

                        if ( pInfoObj )
                            aIPName = pInfoObj->GetObjName();

                        for (nPos=0; nPos<nDataCount; nPos++)
                        {
                            ScChartArray* pChartObj = (*pChartCollection)[nPos];
                            if (pChartObj->GetName() == aIPName)
                            {
                                if (bDoUpdate)
                                {
                                    SchMemChart* pMemChart = pChartObj->CreateMemChart();
                                    SchDLL::Update( aIPObj, pMemChart );
                                    delete pMemChart;
                                }
                                else        // nur Position uebernehmen
                                {
                                    SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
                                    if (pChartData)
                                    {
                                        pChartObj->SetExtraStrings(*pChartData);
//                                      aIPObj->SetModified( TRUE );
                                    }
                                }
                                ScChartListener* pCL = new ScChartListener(
                                    aIPName, this, pChartObj->GetRangeList() );
                                pChartListenerCollection->Insert( pCL );
                                pCL->StartListeningTo();
                            }
                        }
                    }
                }
                pObject = aIter.Next();
            }
        }
    }

    pChartCollection->FreeAll();
}

BOOL ScDocument::HasChartAtPoint( USHORT nTab, const Point& rPos, String* pName )
{
    if (pDrawLayer && pTab[nTab])
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                 pObject->GetBoundRect().IsInside(rPos) )
            {
                        // auch Chart-Objekte die nicht in der Collection sind

                if (IsChart(pObject))
                {
                    if (pName)
                        *pName = ((SdrOle2Obj*)pObject)->GetName();
                    return TRUE;
                }
            }
            pObject = aIter.Next();
        }
    }

    if (pName)
        pName->Erase();
    return FALSE;                   // nix gefunden
}

void ScDocument::UpdateChartArea( const String& rChartName,
            const ScRange& rNewArea, BOOL bColHeaders, BOOL bRowHeaders,
            BOOL bAdd, Window* pWindow )
{
    ScRangeListRef aRLR( new ScRangeList );
    aRLR->Append( rNewArea );
    UpdateChartArea( rChartName, aRLR, bColHeaders, bRowHeaders, bAdd, pWindow );
}

void ScDocument::UpdateChartArea( const String& rChartName,
            const ScRangeListRef& rNewList, BOOL bColHeaders, BOOL bRowHeaders,
            BOOL bAdd, Window* pWindow )
{
    if (!pDrawLayer)
        return;

    for (USHORT nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    ((SdrOle2Obj*)pObject)->GetName() == rChartName )
            {
                SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if (aIPObj.Is())
                {
                    const SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
                    if ( pChartData )
                    {
                        ScChartArray aArray( this, *pChartData );
                        if ( bAdd )
                        {
                            // bei bAdd werden Header-Angaben ignoriert
                            aArray.AddToRangeList( rNewList );
                        }
                        else
                        {
                            aArray.SetRangeList( rNewList );
                            aArray.SetHeaders( bColHeaders, bRowHeaders );
                        }
                        pChartListenerCollection->ChangeListening(
                            rChartName, aArray.GetRangeList() );


                        SchMemChart* pMemChart = aArray.CreateMemChart();
                        ScChartArray::CopySettings( *pMemChart, *pChartData );

                        SchDLL::Update( aIPObj, pMemChart, pWindow );
                        delete pMemChart;

                        // Dies veranlaesst Chart zum sofortigen Update
                        SvData aEmpty;
                        aIPObj->SendDataChanged( aEmpty );
                        aIPObj->SendViewChanged();
                        pObject->SendRepaintBroadcast();

                        return;         // nicht weitersuchen
                    }
                }
            }
            pObject = aIter.Next();
        }
    }
}

void ScDocument::UpdateChart( const String& rChartName, Window* pWindow )
{
    if (!pDrawLayer || bInDtorClear)
        return;

    for (USHORT nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    ((SdrOle2Obj*)pObject)->GetName() == rChartName )
            {
                SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if (aIPObj.Is())
                {
                    const SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
                    if ( pChartData )
                    {
                        ScChartArray aArray( this, *pChartData );

                        SchMemChart* pMemChart = aArray.CreateMemChart();
                        ScChartArray::CopySettings( *pMemChart, *pChartData );

                        //  #57655# Chart-Update ohne geaenderte Einstellungen (MemChart)
                        //  soll das Dokument nicht auf modified setzen (z.B. in frisch
                        //  geladenem Dokument durch initiales Recalc)

                        //  #72576# disable SetModified for readonly documents only

                        BOOL bEnabled = ( pShell && pShell->IsReadOnly() &&
                                            aIPObj->IsEnableSetModified() );
                        if (bEnabled)
                            aIPObj->EnableSetModified(FALSE);

                        SchDLL::Update( aIPObj, pMemChart, pWindow );
                        delete pMemChart;

                        // Dies veranlaesst Chart zum sofortigen Update
                        SvData aEmpty;
                        aIPObj->SendDataChanged( aEmpty );
                        aIPObj->SendViewChanged();
                        pObject->SendRepaintBroadcast();

                        if (bEnabled)
                            aIPObj->EnableSetModified(TRUE);

                        return;         // nicht weitersuchen
                    }
                }
            }
            pObject = aIter.Next();
        }
    }
}

void ScDocument::UpdateChartRef( UpdateRefMode eUpdateRefMode,
                                    USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                    USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                    short nDx, short nDy, short nDz )
{
    if (!pDrawLayer)
        return;

    USHORT nChartCount = pChartListenerCollection->GetCount();
    for ( USHORT nIndex = 0; nIndex < nChartCount; nIndex++ )
    {
        ScChartListener* pChartListener =
            (ScChartListener*) (pChartListenerCollection->At(nIndex));
        ScRangeListRef aRLR( pChartListener->GetRangeList() );
        ScRangeListRef aNewRLR( new ScRangeList );
        BOOL bChanged = FALSE;
        BOOL bDataChanged = FALSE;
        for ( ScRangePtr pR = aRLR->First(); pR; pR = aRLR->Next() )
        {
            USHORT theCol1 = pR->aStart.Col();
            USHORT theRow1 = pR->aStart.Row();
            USHORT theTab1 = pR->aStart.Tab();
            USHORT theCol2 = pR->aEnd.Col();
            USHORT theRow2 = pR->aEnd.Row();
            USHORT theTab2 = pR->aEnd.Tab();
            ScRefUpdateRes eRes = ScRefUpdate::Update(
                this, eUpdateRefMode,
                nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
                nDx,nDy,nDz,
                theCol1,theRow1,theTab1,
                theCol2,theRow2,theTab2 );
            if ( eRes != UR_NOTHING )
            {
                bChanged = TRUE;
                aNewRLR->Append( ScRange(
                    theCol1, theRow1, theTab1,
                    theCol2, theRow2, theTab2 ));
                if ( eUpdateRefMode == URM_INSDEL
                    && !bDataChanged
                    && (eRes == UR_INVALID ||
                        ((pR->aEnd.Col() - pR->aStart.Col()
                        != theCol2 - theCol1)
                    || (pR->aEnd.Row() - pR->aStart.Row()
                        != theRow2 - theRow1)
                    || (pR->aEnd.Tab() - pR->aStart.Tab()
                        != theTab2 - theTab1))) )
                {
                    bDataChanged = TRUE;
                }
            }
            else
                aNewRLR->Append( *pR );
        }
        if ( bChanged )
        {
            SetChartRangeList( pChartListener->GetString(), aNewRLR );
            pChartListener->ChangeListening( aNewRLR, bDataChanged );
        }
    }
}


void ScDocument::SetChartRangeList( const String& rChartName,
            const ScRangeListRef& rNewRangeListRef )
{
    SchMemChart* pChartData = FindChartData( rChartName, TRUE );
    if ( pChartData )
    {
        ScChartArray aArray( this, *pChartData );
        aArray.SetRangeList( rNewRangeListRef );
        aArray.SetExtraStrings( *pChartData );
    }
}


BOOL ScDocument::HasData( USHORT nCol, USHORT nRow, USHORT nTab )
{
    if (pTab[nTab])
        return pTab[nTab]->HasData( nCol, nRow );
    else
        return FALSE;
}

SchMemChart* ScDocument::FindChartData(const String& rName, BOOL bForModify)
{
    if (!pDrawLayer)
        return NULL;

    //  die Seiten hier vom Draw-Layer nehmen,
    //  weil sie evtl. nicht mit den Tabellen uebereinstimmen
    //  (z.B. Redo von Tabelle loeschen, Draw-Redo passiert vor DeleteTab).

    USHORT nCount = pDrawLayer->GetPageCount();
    for (USHORT nTab=0; nTab<nCount; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    ((SdrOle2Obj*)pObject)->GetName() == rName )
            {
                SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if ( aIPObj.Is() )
                {
                    if (bForModify)
                        aIPObj->SetModified( TRUE );
                    return SchDLL::GetChartData( aIPObj );
                }
            }
            pObject = aIter.Next();
        }
    }

    return NULL;                            // nix
}


void ScDocument::UpdateChartListenerCollection()
{
    bChartListenerCollectionNeedsUpdate = FALSE;
    if (!pDrawLayer)
        return;
    else
    {
        ScRange aRange;
        // Range fuer Suche unwichtig
        ScChartListener aCLSearcher( EMPTY_STRING, this, aRange );
        for (USHORT nTab=0; nTab<=MAXTAB; nTab++)
        {
            if (pTab[nTab])
            {
                SdrPage* pPage = pDrawLayer->GetPage(nTab);
                DBG_ASSERT(pPage,"Page ?");

                SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
                    {
                        aCLSearcher.SetString( ((SdrOle2Obj*)pObject)->GetName() );
                        USHORT nIndex;
                        if ( pChartListenerCollection->Search( &aCLSearcher, nIndex ) )
                        {
                            ((ScChartListener*) (pChartListenerCollection->
                                At( nIndex )))->SetUsed( TRUE );
                        }
                        else
                        {
                            //  SchDLL::GetChartData zieht sofort die Chart-DLL an,
                            //  darum vorher SchModuleDummy::HasID testen!

                            SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                            if ( aIPObj.Is() && SchModuleDummy::HasID( *aIPObj->GetSvFactory() ) )
                            {
                                SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
                                if ( pChartData )
                                {
                                    ScChartArray aArray( this, *pChartData );
                                    ScChartListener* pCL = new ScChartListener(
                                        ((SdrOle2Obj*)pObject)->GetName(),
                                        this, aArray.GetRangeList() );
                                    pChartListenerCollection->Insert( pCL );
                                    pCL->StartListeningTo();
                                    pCL->SetUsed( TRUE );

                                    //  Set ReadOnly flag at MemChart, so Chart knows
                                    //  about the external data in a freshly loaded document.
                                    //  #73642# only if the chart really has external data
                                    if ( aArray.IsValid() )
                                        pChartData->SetReadOnly( TRUE );

#if 1
// #74046# initially loaded charts need the number formatter standard precision
                                    BOOL bEnabled = aIPObj->IsEnableSetModified();
                                    if (bEnabled)
                                        aIPObj->EnableSetModified(FALSE);
                                    pChartData->SetNumberFormatter( GetFormatTable() );
                                    SchDLL::Update( aIPObj, pChartData );
                                    //! pChartData got deleted, don't use it anymore
                                    if (bEnabled)
                                        aIPObj->EnableSetModified(TRUE);
#ifndef PRODUCT
                                    static BOOL bShown74046 = 0;
                                    if ( !bShown74046 && SOFFICE_FILEFORMAT_NOW > SOFFICE_FILEFORMAT_50 )
                                    {
                                        bShown74046 = 1;
                                        DBG_ERRORFILE( "on incompatible file format save number formatter standard precision in chart" );
                                    }
#endif
#endif
                                }
                            }
                        }
                    }
                    pObject = aIter.Next();
                }
            }
        }
        // alle nicht auf SetUsed gesetzten loeschen
        pChartListenerCollection->FreeUnused();
    }
}





