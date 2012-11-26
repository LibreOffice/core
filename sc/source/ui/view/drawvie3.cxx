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



// INCLUDE ---------------------------------------------------------------

#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>

#include "drawview.hxx"
#include "drwlayer.hxx"
#include "viewdata.hxx"
#include "dbfunc.hxx"
#include "document.hxx"
#include "userdat.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"

void ScIMapDlgSet( const Graphic& rGraphic, const ImageMap* pImageMap,
                    const TargetList* pTargetList, void* pEditingObj );     // imapwrap
sal_uInt16 ScIMapChildWindowId();

// STATIC DATA -----------------------------------------------------------

ScDrawView::ScDrawView( OutputDevice* pOut, ScViewData* pData )
:   FmFormView( *pData->GetDocument()->GetDrawLayer(), pOut ),
    pViewData( pData ),
    pDev( pOut ),
    pDoc( pData->GetDocument() ),
    nTab( pData->GetTabNo() ),
    pDropMarker( NULL ),
    pDropMarkObj( NULL ),
    bInConstruct( sal_True )
{
    // #i73602# Use default from the configuration
    SetBufferedOverlayAllowed(getOptionsDrawinglayer().IsOverlayBuffer_Calc());

    // #i74769#, #i75172# Use default from the configuration
    SetBufferedOutputAllowed(getOptionsDrawinglayer().IsPaintBuffer_Calc());

    Construct();
}

// Verankerung setzen

void ScDrawView::SetAnchor( ScAnchorType eType )
{
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

    if( aSelection.size() )
    {
        for( sal_uInt32 i(0); i < aSelection.size(); i++ )
        {
            ScDrawLayer::SetAnchor( aSelection[i], eType );
        }

        if ( pViewData )
            pViewData->GetDocShell()->SetDrawModified();
    }
}

ScAnchorType ScDrawView::GetAnchor() const
{
    sal_Bool bPage = sal_False;
    sal_Bool bCell = sal_False;

    if( areSdrObjectsSelected() )
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for( sal_uInt32 i(0); i < aSelection.size(); i++ )
        {
            if( ScDrawLayer::GetAnchor( aSelection[i] ) == SCA_CELL )
                bCell =sal_True;
            else
                bPage = sal_True;
        }
    }

    if( bPage && !bCell )
        return SCA_PAGE;
    if( !bPage && bCell )
        return SCA_CELL;
    return SCA_DONTKNOW;
}

void __EXPORT ScDrawView::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const ScTabDeletedHint* pScTabDeletedHint = dynamic_cast< const ScTabDeletedHint* >(&rHint);

    if (pScTabDeletedHint)                      // Tabelle geloescht
    {
        SCTAB nDelTab = const_cast< ScTabDeletedHint* >(pScTabDeletedHint)->GetTab();
        if (ValidTab(nDelTab))
        {
            // used to be: HidePagePgNum(nDelTab) - hide only if the deleted sheet is shown here
            if ( nDelTab == nTab )
                HideSdrPage();
        }
    }
    else
    {
        const ScTabSizeChangedHint* pScTabSizeChangedHint = dynamic_cast< const ScTabSizeChangedHint* >(&rHint);

        if (pScTabSizeChangedHint)              // Groesse geaendert
        {
            if ( nTab == const_cast< ScTabSizeChangedHint* >(pScTabSizeChangedHint)->GetTab() )
            UpdateWorkArea();
        }
        else
        {
            FmFormView::Notify( rBC,rHint );
        }
    }
}

void ScDrawView::UpdateIMap( SdrObject* pObj )
{
    if ( pViewData &&
         pViewData->GetViewShell()->GetViewFrame()->HasChildWindow( ScIMapChildWindowId() ) &&
         pObj )
    {
        SdrGrafObj* pSdrGrafObj = dynamic_cast< SdrGrafObj* >(pObj);
        SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >(pObj);

        if( pSdrGrafObj || pSdrOle2Obj )
        {
            Graphic     aGraphic;
            TargetList  aTargetList;
            ScIMapInfo* pIMapInfo = ScDrawLayer::GetIMapInfo( pObj );
            const ImageMap* pImageMap = NULL;
            if ( pIMapInfo )
                pImageMap = &pIMapInfo->GetImageMap();

            // Target-Liste besorgen
            pViewData->GetViewShell()->GetViewFrame()->GetTargetList( aTargetList );

            // Grafik vom Objekt besorgen
            if ( pSdrGrafObj )
            {
                aGraphic = pSdrGrafObj->GetGraphic();
            }
            else
            {
                    Graphic* pGraphic = pSdrOle2Obj->GetGraphic();
                if ( pGraphic )
                    aGraphic = *pGraphic;
            }

            ScIMapDlgSet( aGraphic, pImageMap, &aTargetList, pObj );    // aus imapwrap

            // TargetListe kann von uns wieder geloescht werden
            String* pEntry = aTargetList.First();
            while( pEntry )
            {
                delete pEntry;
                pEntry = aTargetList.Next();
            }
        }
    }
}




