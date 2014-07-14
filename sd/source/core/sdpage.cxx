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
#include "precompiled_sd.hxx"

#include <algorithm>

#include <comphelper/classids.hxx>

#include <vcl/svapp.hxx>
#include "eetext.hxx"
#include <editeng/eeitem.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/editdata.hxx>
#include <svx/pageitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/bulitem.hxx>
#include <svx/svdpagv.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdopage.hxx>
#include <sfx2/printer.hxx>
#include <basic/basmgr.hxx>
#include <editeng/pbinitem.hxx>
#include <svx/svdundo.hxx>
#include <svl/smplhint.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/editobj.hxx>
#ifndef _SVX_SRIPTTYPEITEM_HXX
#include <editeng/scripttypeitem.hxx>
#endif
#include <svx/unopage.hxx>
#include <editeng/flditem.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/svditer.hxx>
#include <editeng/adjitem.hxx>
#include "../ui/inc/DrawDocShell.hxx"
#include "Outliner.hxx"
#include "app.hrc"
#include "misc.hxx"
#include "eetext.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "pglink.hxx"
#include "sdresid.hxx"
#include "stlsheet.hxx"
#include "glob.hrc"
#include "glob.hxx"
#include "helpids.h"
#include "anminfo.hxx"
#include "undo/undomanager.hxx"
#include "undo/undoobjects.hxx"
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/unoapi.hxx>
#include <set>
#include <svx/svdlegacy.hxx>
#include <svx/svdview.hxx>

using namespace ::sd;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// detect and hand back the connected SdPage for a SdrObject

SdPage* findConnectionToSdrObject(const SdrObject* pSdrObject)
{
    if(pSdrObject)
    {
        if(pSdrObject->HasListeners())
        {
            const sal_uInt16 nListenerCount(pSdrObject->GetListenerCount());

            for(sal_uInt16 a(0); a < nListenerCount; a++)
            {
                SfxListener* pCandidate = pSdrObject->GetListener(a);

                if(pCandidate) // not all slots in a broadcaster have to be used
                {
                    SdPage* pRetval = dynamic_cast< SdPage* >(pCandidate);

                    if(pRetval)
                    {
                        return pRetval;
                    }
                }
            }
        }
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// connect to given SdrObject. All existing connections are deleted, a single
// new one gets established

void establishConnectionToSdrObject(SdrObject* pSdrObject, SdPage* pSdPage)
{
    if(pSdrObject)
    {
        // remove all SdPages
        resetConnectionToSdrObject(pSdrObject);

        if(pSdPage)
        {
            pSdPage->StartListening(*pSdrObject);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// all connections (should be a single one) to the SdrObject are removed

void resetConnectionToSdrObject(SdrObject* pSdrObject)
{
    if(pSdrObject)
    {
        SdPage* pFound = findConnectionToSdrObject(pSdrObject);

        while(pFound)
        {
            pFound->EndListening(*pSdrObject);
            pFound = findConnectionToSdrObject(pSdrObject);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdPage::SdPage(SdDrawDocument& rNewDoc, StarBASIC* pBasic, bool bMasterPage)
:   FmFormPage(rNewDoc, pBasic, bMasterPage)
,   SfxListener()
,   mePageKind(PK_STANDARD)
,   meAutoLayout(AUTOLAYOUT_NONE)
,   mbSelected(false)
,   mePresChange(PRESCHANGE_MANUAL)
,   mnTime(1)
,   mbSoundOn(false)
,   mbExcluded(false)
,   mbLoopSound(false)
,   mbStopSound(false)
,   mbScaleObjects(true)
,   mbBackgroundFullSize( false )
,   meCharSet(gsl_getSystemTextEncoding())
,   mnPaperBin(PAPERBIN_PRINTER_SETTINGS)
,   mpPageLink(NULL)
,   mpItems(NULL)
,   mnTransitionType(0)
,   mnTransitionSubtype(0)
,   mbTransitionDirection(sal_True)
,   mnTransitionFadeColor(0)
,   mfTransitionDuration(2.0)
,   mbIsPrecious(true)
{
    // Der Layoutname der Seite wird von SVDRAW benutzt, um die Praesentations-
    // vorlagen der Gliederungsobjekte zu ermitteln. Darum enthaelt er bereits
    // den Bezeichner fuer die Gliederung (STR_LAYOUT_OUTLINE).
    maLayoutName  = String(SdResId(STR_LAYOUT_DEFAULT_NAME));
    maLayoutName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
    maLayoutName += String(SdResId(STR_LAYOUT_OUTLINE));

    const basegfx::B2DVector& rPageScale = GetPageScale();

    if (rPageScale.getX() > rPageScale.getY())
    {
        meOrientation = ORIENTATION_LANDSCAPE;
    }
    else
    {
        meOrientation = ORIENTATION_PORTRAIT;
    }
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SdPage::~SdPage()
{
    DisconnectLink();

    EndListenOutlineText();

    if( mpItems )
        delete mpItems;
}

void SdPage::copyDataFromSdrPage(const SdrPage& rSource)
{
    if(this != &rSource)
    {
        const SdPage* pSource = dynamic_cast< const SdPage* >(&rSource);

        if(pSource)
        {
            // call parent
            FmFormPage::copyDataFromSdrPage(rSource);

            // no local data to copy
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrPage* SdPage::CloneSdrPage(SdrModel* pTargetModel) const
{
    SdDrawDocument* pSdDrawDocument = static_cast< SdDrawDocument* >(pTargetModel ? pTargetModel : &getSdrModelFromSdrPage());
    OSL_ENSURE(dynamic_cast< SdDrawDocument* >(pSdDrawDocument), "Wrong SdrModel type in SdPage clone (!)");
    SdPage* pClone = new SdPage(
        *pSdDrawDocument,
        GetBasic(),
        IsMasterPage());
    OSL_ENSURE(pClone, "CloneSdrPage error (!)");
    pClone->copyDataFromSdrPage(*this);

    return pClone;
}

struct OrdNumSorter
{
    bool operator()( SdrObject* p1, SdrObject* p2 )
    {
        return p1->GetNavigationPosition() < p2->GetNavigationPosition();
    }
};

/** returns the nIndex'th object from the given PresObjKind, index starts with 1 */
SdrObject* SdPage::GetPresObj(PresObjKind eObjKind, int nIndex, bool bFuzzySearch /* = false */ )
{
    // first sort all matching shapes with z-order
    SdrObjectVector aMatches;

    const SdrObject* pObj = 0;
    while( (pObj = maPresentationShapeList.getNextShape(pObj)) != 0 )
    {
        SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(*pObj);
        if( pInfo )
        {
            bool bFound = false;
            if( pInfo->mePresObjKind == eObjKind )
            {
                bFound = true;
            }
            else if( bFuzzySearch && (eObjKind == PRESOBJ_OUTLINE) )
            {
                switch( pInfo->mePresObjKind )
                {
                case PRESOBJ_GRAPHIC:
                case PRESOBJ_OBJECT:
                case PRESOBJ_CHART:
                case PRESOBJ_ORGCHART:
                case PRESOBJ_TABLE:
                case PRESOBJ_CALC:
                case PRESOBJ_IMAGE:
                case PRESOBJ_MEDIA:
                    bFound = true;
                    break;
                default:
                    break;
                }
            }
            if( bFound )
            {
                aMatches.push_back( const_cast< SdrObject* >(pObj) );
            }
        }
    }

    if( aMatches.size() > 1 )
    {
        OrdNumSorter aSortHelper;
        std::sort( aMatches.begin(), aMatches.end(), aSortHelper );
    }

    if( nIndex > 0 )
        nIndex--;

    if( (nIndex >= 0) && ( aMatches.size() > static_cast<unsigned int>(nIndex)) )
        return aMatches[nIndex];

    return 0;
}

/** create background properties */
void SdPage::EnsureMasterPageDefaultBackground()
{
    if(mbMaster)
    {
        // no hard attributes on MasterPage attributes
        getSdrPageProperties().ClearItem();
        SfxStyleSheet* pSheetForPresObj = GetStyleSheetForMasterPageBackground();

        if(pSheetForPresObj)
        {
            // set StyleSheet for background fill attributes
            getSdrPageProperties().SetStyleSheet(pSheetForPresObj);
        }
        else
        {
            // no style found, assert and set at least XFILL_NONE
            OSL_ENSURE(false, "No Style for MasterPageBackground fill found (!)");
            getSdrPageProperties().PutItem(XFillStyleItem(XFILL_NONE));
        }
    }
}

/** creates a presentation object with the given PresObjKind on this page. A user call will be set
*/
SdrObject* SdPage::CreatePresObj(PresObjKind eObjKind, bool bVertical, const basegfx::B2DRange& rRange, bool /* bInsert */ )
{
    ::svl::IUndoManager* pUndoManager = static_cast< SdDrawDocument* >(&getSdrModelFromSdrPage())->GetUndoManager();
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

    SdrObject* pSdrObj = NULL;
    bool bForceText = false;    // forces the shape text to be set even if its empty
    bool bEmptyPresObj = true;

    switch( eObjKind )
    {
        case PRESOBJ_TITLE:
        {
            pSdrObj = new SdrRectObj(
                getSdrModelFromSdrPage(),
                basegfx::B2DHomMatrix(),
                OBJ_TITLETEXT,
                true);

            if (mbMaster)
            {
                pSdrObj->SetNotVisibleAsMaster(true);
            }
        }
        break;

        case PRESOBJ_OUTLINE:
        {
            pSdrObj = new SdrRectObj(
                getSdrModelFromSdrPage(),
                basegfx::B2DHomMatrix(),
                OBJ_OUTLINETEXT,
                true);

            if (mbMaster)
            {
                pSdrObj->SetNotVisibleAsMaster(true);
            }
        }
        break;

        case PRESOBJ_NOTES:
        {
            pSdrObj = new SdrRectObj(
                getSdrModelFromSdrPage(),
                basegfx::B2DHomMatrix(),
                OBJ_TEXT,
                true);

            if (mbMaster)
            {
                pSdrObj->SetNotVisibleAsMaster(true);
            }
        }
        break;

        case PRESOBJ_TEXT:
        {
            pSdrObj = new SdrRectObj(
                getSdrModelFromSdrPage(),
                basegfx::B2DHomMatrix(),
                OBJ_TEXT,
                true);
        }
        break;

        case PRESOBJ_GRAPHIC:
        {
            BitmapEx aBmpEx( SdResId( BMP_PRESOBJ_GRAPHIC ) );
            Graphic  aGraphic( aBmpEx );
            OutputDevice &aOutDev = *Application::GetDefaultDevice();
            aOutDev.Push();

            aOutDev.SetMapMode( aGraphic.GetPrefMapMode() );
            Size aSizePix = aOutDev.LogicToPixel( aGraphic.GetPrefSize() );
            aOutDev.SetMapMode(MAP_100TH_MM);

            Size aSize = aOutDev.PixelToLogic(aSizePix);
            pSdrObj = new SdrGrafObj(
                getSdrModelFromSdrPage(),
                aGraphic,
                basegfx::tools::createScaleB2DHomMatrix(aSize.getWidth(), aSize.getHeight()));
            aOutDev.Pop();
        }
        break;

        case PRESOBJ_MEDIA:
        case PRESOBJ_OBJECT:
        {
            SdrOle2Obj* pNewSdrOle2Obj = new SdrOle2Obj(getSdrModelFromSdrPage());
            pSdrObj = pNewSdrOle2Obj;
            BitmapEx aBmpEx( SdResId( BMP_PRESOBJ_OBJECT ) );
            Graphic aGraphic( aBmpEx );
            pNewSdrOle2Obj->SetGraphic(&aGraphic);
        }
        break;

        case PRESOBJ_CHART:
        {
            SdrOle2Obj* pNewSdrOle2Obj = new SdrOle2Obj(getSdrModelFromSdrPage());
            pSdrObj = pNewSdrOle2Obj;
            pNewSdrOle2Obj->SetProgName( String( RTL_CONSTASCII_USTRINGPARAM( "StarChart" )));
            BitmapEx aBmpEx( SdResId( BMP_PRESOBJ_CHART ) );
            Graphic aGraphic( aBmpEx );
            pNewSdrOle2Obj->SetGraphic(&aGraphic);
        }
        break;

        case PRESOBJ_ORGCHART:
        {
            SdrOle2Obj* pNewSdrOle2Obj = new SdrOle2Obj(getSdrModelFromSdrPage());
            pSdrObj = pNewSdrOle2Obj;
            pNewSdrOle2Obj->SetProgName( String( RTL_CONSTASCII_USTRINGPARAM( "StarOrg" )));
            BitmapEx aBmpEx( SdResId( BMP_PRESOBJ_ORGCHART ) );
            Graphic aGraphic( aBmpEx );
            pNewSdrOle2Obj->SetGraphic(&aGraphic);
        }

        case PRESOBJ_TABLE:
        case PRESOBJ_CALC:
        {
            SdrOle2Obj* pNewSdrOle2Obj = new SdrOle2Obj(getSdrModelFromSdrPage());
            pSdrObj = pNewSdrOle2Obj;
            pNewSdrOle2Obj->SetProgName( String( RTL_CONSTASCII_USTRINGPARAM( "StarCalc" )));
            BitmapEx aBmpEx( SdResId( BMP_PRESOBJ_TABLE ) );
            Graphic aGraphic( aBmpEx );
            pNewSdrOle2Obj->SetGraphic(&aGraphic);
        }
        break;

        case PRESOBJ_HANDOUT:
        {
            //Erste Standardseite am SdrPageObj vermerken
            // #i105146# We want no content to be displayed for PK_HANDOUT,
            // so just never set a page as content
            pSdrObj = new SdrPageObj(getSdrModelFromSdrPage());
//          pSdrObj->SetResizeProtect(true);
        }
        break;

        case PRESOBJ_PAGE:
        {
            //Notizseite am SdrPageObj vermerken
            sal_uInt32 nDestPageNum(GetPageNumber());

            if(nDestPageNum)
            {
                // decrement only when != 0, else we get a SDRPAGE_NOTFOUND
                nDestPageNum -= 1;
            }

            if(nDestPageNum < getSdrModelFromSdrPage().GetPageCount())
            {
                pSdrObj = new SdrPageObj(
                    getSdrModelFromSdrPage(),
                    basegfx::B2DHomMatrix(),
                    getSdrModelFromSdrPage().GetPage(nDestPageNum));
            }
            else
            {
                pSdrObj = new SdrPageObj(getSdrModelFromSdrPage());
            }

            pSdrObj->SetResizeProtect(true);
        }
        break;

        case PRESOBJ_HEADER:
        case PRESOBJ_FOOTER:
        case PRESOBJ_DATETIME:
        case PRESOBJ_SLIDENUMBER:
        {
            pSdrObj = new SdrRectObj(
                getSdrModelFromSdrPage(),
                basegfx::B2DHomMatrix(),
                OBJ_TEXT,
                true);
            bEmptyPresObj = false;
            bForceText = true;
        }
        break;
        default:
            break;
    }

    if (pSdrObj)
    {
        pSdrObj->SetEmptyPresObj(bEmptyPresObj);
        sdr::legacy::SetLogicRange(*pSdrObj, rRange);
        InsertObjectToSdrObjList(*pSdrObj);
        SdrTextObj* pSdrTextObj = dynamic_cast< SdrTextObj* >(pSdrObj);

        if ( pSdrTextObj )
        {
            // #96243# Tell the object EARLY that it is vertical to have the
            // defaults for AutoGrowWidth/Height reversed
            if(bVertical)
                pSdrTextObj->SetVerticalWriting(true);

            SfxItemSet aTempAttr(pSdrTextObj->GetObjectItemPool());
            if( bVertical )
                aTempAttr.Put( SdrMetricItem(SDRATTR_TEXT_MINFRAMEWIDTH, basegfx::fround(rRange.getWidth()) ) );
            else
                aTempAttr.Put( SdrMetricItem(SDRATTR_TEXT_MINFRAMEHEIGHT, basegfx::fround(rRange.getHeight()) ) );

            if (mbMaster)
            {
                // Bei Praesentationsobjekten auf der MasterPage soll die
                // Groesse vom Benutzwer frei waehlbar sein

                // #96243# potential problem: This action was still NOT
                // adapted for vertical text. This sure needs to be done.
                if(bVertical)
                    aTempAttr.Put(SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, false));
                else
                    aTempAttr.Put(SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, false));
            }

            // check if we need another vertical adjustement than the default
            SdrTextVertAdjust eV = SDRTEXTVERTADJUST_TOP;

            if( (eObjKind == PRESOBJ_FOOTER) && (mePageKind != PK_STANDARD) )
            {
                eV = SDRTEXTVERTADJUST_BOTTOM;
            }
            else if( (eObjKind == PRESOBJ_SLIDENUMBER) && (mePageKind != PK_STANDARD) )
            {
                eV = SDRTEXTVERTADJUST_BOTTOM;
            }

            if( eV != SDRTEXTVERTADJUST_TOP )
                aTempAttr.Put(SdrTextVertAdjustItem(eV));

            pSdrTextObj->SetMergedItemSet(aTempAttr);

            sdr::legacy::SetLogicRange(*pSdrTextObj, rRange);
        }

        String aString = GetPresObjText(eObjKind);

        if( pSdrTextObj && (aString.Len() || bForceText) )
        {
            SdrOutliner* pOutliner = (static_cast< SdDrawDocument& >(getSdrModelFromSdrPage())).GetInternalOutliner();

            sal_uInt16 nOutlMode = pOutliner->GetMode();
            pOutliner->Init( OUTLINERMODE_TEXTOBJECT );
            pOutliner->SetStyleSheet( 0, NULL );
            pOutliner->SetVertical( bVertical );

            String aEmptyStr;
            SetObjText( pSdrTextObj, (SdrOutliner*)pOutliner, eObjKind, aString );

            pOutliner->Init( nOutlMode );
            pOutliner->SetStyleSheet( 0, NULL );
        }

        if( (eObjKind == PRESOBJ_HEADER) || (eObjKind == PRESOBJ_FOOTER) || (eObjKind == PRESOBJ_SLIDENUMBER) || (eObjKind == PRESOBJ_DATETIME) )
        {
            SfxItemSet aTempAttr(pSdrObj->GetObjectItemPool());
            aTempAttr.Put( SvxFontHeightItem( 493, 100, EE_CHAR_FONTHEIGHT ) );
            aTempAttr.Put( SvxFontHeightItem( 493, 100, EE_CHAR_FONTHEIGHT_CTL ) );
            aTempAttr.Put( SvxFontHeightItem( 493, 100, EE_CHAR_FONTHEIGHT_CJK ) );

            SvxAdjust eH = SVX_ADJUST_LEFT;

            if( (eObjKind == PRESOBJ_DATETIME) && (mePageKind != PK_STANDARD ) )
            {
                eH = SVX_ADJUST_RIGHT;
            }
            else if( (eObjKind == PRESOBJ_FOOTER) && (mePageKind == PK_STANDARD ) )
            {
                eH = SVX_ADJUST_CENTER;
            }
            else if( eObjKind == PRESOBJ_SLIDENUMBER )
            {
                eH = SVX_ADJUST_RIGHT;
            }

            if( eH != SVX_ADJUST_LEFT )
                aTempAttr.Put(SvxAdjustItem(eH, EE_PARA_JUST ));

            pSdrObj->SetMergedItemSet(aTempAttr);
        }

        if (mbMaster)
        {
            SdrLayerAdmin& rLayerAdmin = getSdrModelFromSdrPage().GetModelLayerAdmin();

            // Hintergrundobjekte der MasterPage
            pSdrObj->SetLayer( rLayerAdmin.
                GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), false) );
        }

        // Objekt am StyleSheet anmelden
        // #95114# Set style only when one was found (as in 5.2)
        // pSdrObj->SetStyleSheet( GetStyleSheetForPresObj(eObjKind), false );
        if( mePageKind != PK_HANDOUT )
        {
            SfxStyleSheet* pSheetForPresObj = GetStyleSheetForPresObj(eObjKind);
            if(pSheetForPresObj)
                pSdrObj->SetStyleSheet(pSheetForPresObj, false);
        }

        if (eObjKind == PRESOBJ_OUTLINE)
        {
            for (sal_uInt16 nLevel = 1; nLevel < 10; nLevel++)
            {
                String aName(maLayoutName);
                aName += sal_Unicode( ' ' );
                aName += String::CreateFromInt32( nLevel );
                SfxStyleSheet* pSheet = dynamic_cast< SfxStyleSheet* >(
                    getSdrModelFromSdrPage().GetStyleSheetPool()->Find(aName, SD_STYLE_FAMILY_MASTERPAGE));
                DBG_ASSERT(pSheet, "Vorlage fuer Gliederungsobjekt nicht gefunden");
                if (pSheet)
                    pSdrObj->StartListening(*pSheet);
            }
        }

        if ( eObjKind == PRESOBJ_OBJECT   ||
             eObjKind == PRESOBJ_CHART    ||
             eObjKind == PRESOBJ_ORGCHART ||
             eObjKind == PRESOBJ_CALC    ||
             eObjKind == PRESOBJ_GRAPHIC )
        {
            SfxItemSet aSet(pSdrObj->GetObjectItemPool());
            aSet.Put( SdrOnOffItem(SDRATTR_TEXT_CONTOURFRAME, true ) );
            aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ) );

            pSdrObj->SetMergedItemSet(aSet);
        }

        if( bUndo )
        {
            pUndoManager->AddUndoAction(getSdrModelFromSdrPage().GetSdrUndoFactory().CreateUndoNewObject(*pSdrObj));
        }

        if( bUndo )
        {
            pUndoManager->AddUndoAction( new UndoObjectPresentationKind( *pSdrObj ) );
            pUndoManager->AddUndoAction( new UndoConnectionToSdrObject(*pSdrObj) );
        }

        InsertPresObj(pSdrObj, eObjKind);
        establishConnectionToSdrObject(pSdrObj, this);
    }

    return(pSdrObj);
}

/*************************************************************************
|*
|* Es werden Praesentationsobjekte auf der Page erzeugt.
|* Alle Praesentationsobjekte erhalten einen UserCall auf die Page.
|*
\************************************************************************/

SfxStyleSheet* SdPage::GetStyleSheetForMasterPageBackground() const
{
    String aName(GetLayoutName());
    String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
    sal_uInt16 nPos = aName.Search(aSep);

    if (nPos != STRING_NOTFOUND)
    {
        nPos = nPos + aSep.Len();
        aName.Erase(nPos);
    }

    aName += String(SdResId(STR_LAYOUT_BACKGROUND));

    SfxStyleSheetBasePool* pStShPool = getSdrModelFromSdrPage().GetStyleSheetPool();
    SfxStyleSheetBase*     pResult   = pStShPool->Find(aName, SD_STYLE_FAMILY_MASTERPAGE);
    return (SfxStyleSheet*)pResult;
}

SfxStyleSheet* SdPage::GetStyleSheetForPresObj(PresObjKind eObjKind) const
{
    String aName(GetLayoutName());
    String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
    sal_uInt16 nPos = aName.Search(aSep);
    if (nPos != STRING_NOTFOUND)
    {
        nPos = nPos + aSep.Len();
        aName.Erase(nPos);
    }

    switch (eObjKind)
    {
        case PRESOBJ_OUTLINE:
        {
            aName = GetLayoutName();
            aName += sal_Unicode( ' ' );
            aName += String::CreateFromInt32( 1 );
        }
        break;

        case PRESOBJ_TITLE:
            aName += String(SdResId(STR_LAYOUT_TITLE));
            break;

        case PRESOBJ_NOTES:
            aName += String(SdResId(STR_LAYOUT_NOTES));
            break;

        case PRESOBJ_TEXT:
            aName += String(SdResId(STR_LAYOUT_SUBTITLE));
            break;

        case PRESOBJ_HEADER:
        case PRESOBJ_FOOTER:
        case PRESOBJ_DATETIME:
        case PRESOBJ_SLIDENUMBER:
            aName += String(SdResId(STR_LAYOUT_BACKGROUNDOBJECTS));
            break;

        default:
            break;
    }

    SfxStyleSheetBasePool* pStShPool = getSdrModelFromSdrPage().GetStyleSheetPool();
    SfxStyleSheetBase*     pResult   = pStShPool->Find(aName, SD_STYLE_FAMILY_MASTERPAGE);
    return (SfxStyleSheet*)pResult;
}

/** returns the presentation style with the given helpid from this masterpage or this
    slides masterpage */
SdStyleSheet* SdPage::getPresentationStyle( sal_uInt32 nHelpId ) const
{
    String aStyleName( /*GetPage()->*/GetLayoutName() );
    const String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
    aStyleName.Erase(aStyleName.Search(aSep) + aSep.Len());

    sal_uInt16 nNameId;
    switch( nHelpId )
    {
    case HID_PSEUDOSHEET_TITLE:             nNameId = STR_LAYOUT_TITLE;             break;
    case HID_PSEUDOSHEET_SUBTITLE:          nNameId = STR_LAYOUT_SUBTITLE;          break;
    case HID_PSEUDOSHEET_OUTLINE1:
    case HID_PSEUDOSHEET_OUTLINE2:
    case HID_PSEUDOSHEET_OUTLINE3:
    case HID_PSEUDOSHEET_OUTLINE4:
    case HID_PSEUDOSHEET_OUTLINE5:
    case HID_PSEUDOSHEET_OUTLINE6:
    case HID_PSEUDOSHEET_OUTLINE7:
    case HID_PSEUDOSHEET_OUTLINE8:
    case HID_PSEUDOSHEET_OUTLINE9:          nNameId = STR_LAYOUT_OUTLINE;           break;
    case HID_PSEUDOSHEET_BACKGROUNDOBJECTS: nNameId = STR_LAYOUT_BACKGROUNDOBJECTS; break;
    case HID_PSEUDOSHEET_BACKGROUND:        nNameId = STR_LAYOUT_BACKGROUND;        break;
    case HID_PSEUDOSHEET_NOTES:             nNameId = STR_LAYOUT_NOTES;             break;

    default:
        DBG_ERROR( "SdPage::getPresentationStyle(), illegal argument!" );
        return 0;
    }
    aStyleName.Append( String( SdResId( nNameId ) ) );
    if( nNameId == STR_LAYOUT_OUTLINE )
    {
        aStyleName.Append( sal_Unicode( ' ' ));
        aStyleName.Append( String::CreateFromInt32( sal_Int32( nHelpId - HID_PSEUDOSHEET_OUTLINE )));
    }

    SfxStyleSheetBasePool* pStShPool = getSdrModelFromSdrPage().GetStyleSheetPool();
    SfxStyleSheetBase*     pResult   = pStShPool->Find(aStyleName, SD_STYLE_FAMILY_MASTERPAGE);
    return dynamic_cast<SdStyleSheet*>(pResult);
}

/*************************************************************************
|*
|* Das Praesentationsobjekt rObj hat sich geaendert und wird nicht mehr
|* durch das Praesentationsobjekt der MasterPage referenziert.
|* Der UserCall wird geloescht.
|*
\************************************************************************/

void SdPage::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    const SdrBaseHint* pSdrBaseHint = dynamic_cast< const SdrBaseHint* >(&rHint);

    if(pSdrBaseHint)
    {
        const SdrObject* pObj = pSdrBaseHint->GetSdrHintObject();

        if(pObj)
        {
            HandleChanged(*pObj, pSdrBaseHint->GetSdrHintKind());
        }
    }
}

void SdPage::HandleChanged(
    const SdrObject& rObj,
    SdrHintKind eHint)
{
    if (!maLockAutoLayoutArrangement.isLocked())
    {
        switch (eHint)
        {
            case HINT_OBJCHG_MOVE:
            case HINT_OBJCHG_RESIZE:
            {
                if( getSdrModelFromSdrPage().isLocked() )
                    break;

                SdrObject* pObj = (SdrObject*) &rObj;

                if (pObj)
                {
                    if (!mbMaster)
                    {
                        if( findConnectionToSdrObject(pObj) )
                        {
                            ::svl::IUndoManager* pUndoManager = static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetUndoManager();
                            const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

                            if( bUndo )
                                pUndoManager->AddUndoAction( new UndoConnectionToSdrObject(*pObj) );

                            // Objekt was resized by user and does not listen to its slide anymore
                            resetConnectionToSdrObject(pObj);
                        }
                    }
                    else
                    {
                        // MasterPage-Objekt wurde veraendert, daher
                        // Objekte auf allen Seiten anpassen
                        SdDrawDocument& rDrawDocument = static_cast< SdDrawDocument& >(getSdrModelFromSdrPage());
                        sal_uInt32 nPageCount = rDrawDocument.GetSdPageCount(mePageKind);

                        for (sal_uInt32 i = 0; i < nPageCount; i++)
                        {
                            SdPage* pLoopPage = rDrawDocument.GetSdPage(i, mePageKind);

                            if (pLoopPage && this == &(pLoopPage->TRG_GetMasterPage()))
                            {
                                // Seite hoert auf diese MasterPage, daher
                                // AutoLayout anpassen
                                pLoopPage->SetAutoLayout(pLoopPage->GetAutoLayout());
                            }
                        }
                    }
                }
            }
            break;

            default:
                break;
        }
    }
}

/*************************************************************************
|*
|* Erzeugt auf einer MasterPage Hintergrund, Titel- und Layout-Bereich
|*
\************************************************************************/

void SdPage::CreateTitleAndLayout(bool bInit, bool bCreate )
{
    SdDrawDocument& rDrawDocument = static_cast< SdDrawDocument& >(getSdrModelFromSdrPage());
    ::svl::IUndoManager* pUndoManager = rDrawDocument.GetUndoManager();
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

    SdPage* pMasterPage = this;

    if (!mbMaster)
    {
        pMasterPage = (SdPage*)(&(TRG_GetMasterPage()));
    }

    if (!pMasterPage)
    {
        return;
    }

    /**************************************************************************
    * Hintergrund, Titel- und Layout-Bereich werden angelegt
    **************************************************************************/
    if( mePageKind == PK_STANDARD )
    {
        pMasterPage->EnsureMasterPageDefaultBackground();
    }

    if( DOCUMENT_TYPE_IMPRESS == rDrawDocument.GetDocumentType() )
    {
        if( mePageKind == PK_HANDOUT && bInit )
        {
            // handout template

            // delete all available handout presentation objects
            SdrObject *pObj=NULL;
            while( (pObj = pMasterPage->GetPresObj(PRESOBJ_HANDOUT)) != 0 )
            {
                pMasterPage->RemoveObjectFromSdrObjList(pObj->GetNavigationPosition());

                if(getSdrModelFromSdrPage().isLocked())
                {
                    // remove self when model is locked (during load), else the
                    // broadcast of object removal will do that
                    pMasterPage->RemovePresObj(pObj);
                }

                if( bUndo )
                {
                    pUndoManager->AddUndoAction(rDrawDocument.GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));
                }
                else
                {
                    deleteSdrObjectSafeAndClearPointer(pObj);  // memory leak i120050
                }
            }

            std::vector< basegfx::B2DRange > aAreas;
            CalculateHandoutAreas( rDrawDocument, pMasterPage->GetAutoLayout(), false, aAreas );

            const bool bSkip = pMasterPage->GetAutoLayout() == AUTOLAYOUT_HANDOUT3;
            std::vector< basegfx::B2DRange >::iterator iter( aAreas.begin() );

            while( iter != aAreas.end() )
            {
                SdrPageObj* pPageObj = static_cast<SdrPageObj*>(pMasterPage->CreatePresObj(PRESOBJ_HANDOUT, false, (*iter++), true) );
                // #i105146# We want no content to be displayed for PK_HANDOUT,
                // so just never set a page as content
                pPageObj->SetReferencedPage(0L);

                if( bSkip && iter != aAreas.end() )
                    iter++;
            }
        }

        if( mePageKind != PK_HANDOUT )
        {
            SdrObject* pMasterTitle = pMasterPage->GetPresObj( PRESOBJ_TITLE );
            if( pMasterTitle == NULL )
                pMasterPage->CreateDefaultPresObj(PRESOBJ_TITLE, true);

            SdrObject* pMasterOutline = pMasterPage->GetPresObj( mePageKind==PK_NOTES ? PRESOBJ_NOTES : PRESOBJ_OUTLINE );
            if( pMasterOutline == NULL )
                pMasterPage->CreateDefaultPresObj( mePageKind == PK_STANDARD ? PRESOBJ_OUTLINE : PRESOBJ_NOTES, true );
        }

        // create header&footer objects

        if( bCreate )
        {
            if( mePageKind != PK_STANDARD )
            {
                SdrObject* pHeader = pMasterPage->GetPresObj( PRESOBJ_HEADER );
                if( pHeader == NULL )
                    pMasterPage->CreateDefaultPresObj( PRESOBJ_HEADER, true );
            }

            SdrObject* pDate   = pMasterPage->GetPresObj( PRESOBJ_DATETIME );
            if( pDate == NULL )
                pMasterPage->CreateDefaultPresObj( PRESOBJ_DATETIME, true );

            SdrObject* pFooter = pMasterPage->GetPresObj( PRESOBJ_FOOTER );
            if( pFooter == NULL )
                pMasterPage->CreateDefaultPresObj( PRESOBJ_FOOTER, true );

            SdrObject* pNumber = pMasterPage->GetPresObj( PRESOBJ_SLIDENUMBER );
            if( pNumber == NULL )
                pMasterPage->CreateDefaultPresObj( PRESOBJ_SLIDENUMBER, true );
        }
    }
}

SdrObject* SdPage::CreateDefaultPresObj(PresObjKind eObjKind, bool bInsert)
{
    if( eObjKind == PRESOBJ_TITLE )
    {
        const basegfx::B2DRange aTitleRange( GetTitleRange() );
        return CreatePresObj(PRESOBJ_TITLE, false, aTitleRange, bInsert);
    }
    else if( eObjKind == PRESOBJ_OUTLINE )
    {
        const basegfx::B2DRange aLayoutRange( GetLayoutRange() );
        return CreatePresObj( PRESOBJ_OUTLINE, false, aLayoutRange, bInsert);
    }
    else if( eObjKind == PRESOBJ_NOTES )
    {
        const basegfx::B2DRange aLayoutRange( GetLayoutRange() );
        return CreatePresObj( PRESOBJ_NOTES, false, aLayoutRange, bInsert);
    }
    else if( (eObjKind == PRESOBJ_FOOTER) || (eObjKind == PRESOBJ_DATETIME) || (eObjKind == PRESOBJ_SLIDENUMBER) || (eObjKind == PRESOBJ_HEADER ) )
    {
        // create footer objects for standard master page
        if( mePageKind == PK_STANDARD )
        {
            const double fLeftBorder(GetLeftPageBorder());
            const double fTopBorder(GetTopPageBorder());
            const basegfx::B2DPoint aTitlePos(fLeftBorder, fTopBorder);
            const basegfx::B2DVector aPageSize(GetInnerPageScale());
            const double Y(fTopBorder + aPageSize.getY() * 0.911);
            const double W1(aPageSize.getX() * 0.233);
            const double W2(aPageSize.getX() * 0.317);
            const double H(aPageSize.getY() * 0.069);

            if(PRESOBJ_DATETIME == eObjKind)
            {
                const basegfx::B2DPoint aPos(fLeftBorder + (aPageSize.getX() * 0.05), Y);
                const basegfx::B2DRange aRange(aPos, aPos + basegfx::B2DTuple(W1, H));

                return CreatePresObj(PRESOBJ_DATETIME, false, aRange, bInsert);
            }
            else if(PRESOBJ_FOOTER == eObjKind)
            {
                const basegfx::B2DPoint aPos(fLeftBorder + (aPageSize.getX() * 0.342), Y);
                const basegfx::B2DRange aRange(aPos, aPos + basegfx::B2DTuple(W2, H));

                return CreatePresObj(PRESOBJ_FOOTER, false, aRange, bInsert);
            }
            else if( eObjKind == PRESOBJ_SLIDENUMBER )
            {
                const basegfx::B2DPoint aPos(fLeftBorder + (aPageSize.getX() * 0.717), Y);
                const basegfx::B2DRange aRange(aPos, aPos + basegfx::B2DTuple(W1, H));

                return CreatePresObj(PRESOBJ_SLIDENUMBER, false, aRange, bInsert);
            }
            else
            {
                DBG_ERROR( "SdPage::CreateDefaultPresObj() - can't create a header placeholder for a slide master" );
                return 0;
            }
        }
        else
        {
            // create header&footer objects for handout and notes master
            const basegfx::B2DPoint aTitlePos(GetLeftPageBorder(), GetTopPageBorder());
            const basegfx::B2DVector aPageSize(GetInnerPageScale());
            const double NOTES_HEADER_FOOTER_WIDTH(aPageSize.getX() * 0.434);
            const double NOTES_HEADER_FOOTER_HEIGHT(aPageSize.getY() * 0.05);
            const basegfx::B2DVector aSize(NOTES_HEADER_FOOTER_WIDTH, NOTES_HEADER_FOOTER_HEIGHT);
            const double X1(GetLeftPageBorder());
            const double X2(GetLeftPageBorder() + (aPageSize.getX() - NOTES_HEADER_FOOTER_WIDTH));
            const double Y1(GetTopPageBorder());
            const double Y2(GetTopPageBorder() + (aPageSize.getY() - NOTES_HEADER_FOOTER_HEIGHT));

            if(PRESOBJ_HEADER == eObjKind)
            {
                const basegfx::B2DPoint aPos(X1, Y1);
                const basegfx::B2DRange aRange(aPos, aPos + aSize);

                return CreatePresObj(PRESOBJ_HEADER, false, aRange, bInsert);
            }
            else if(PRESOBJ_DATETIME == eObjKind)
            {
                const basegfx::B2DPoint aPos(X2, Y1);
                const basegfx::B2DRange aRange(aPos, aPos + aSize);

                return CreatePresObj(PRESOBJ_DATETIME, false, aRange, bInsert);
            }
            else if(PRESOBJ_FOOTER == eObjKind)
            {
                const basegfx::B2DPoint aPos(X1, Y2);
                const basegfx::B2DRange aRange(aPos, aPos + aSize);

                return CreatePresObj(PRESOBJ_FOOTER, false, aRange, bInsert);
            }
            else if(PRESOBJ_SLIDENUMBER == eObjKind)
            {
                const basegfx::B2DPoint aPos(X2, Y2);
                const basegfx::B2DRange aRange(aPos, aPos + aSize);

                return CreatePresObj(PRESOBJ_SLIDENUMBER, false, aRange, bInsert);
            }

            DBG_ERROR("SdPage::CreateDefaultPresObj() - this should not happen!");
            return NULL;
        }
    }
    else
    {
        DBG_ERROR("SdPage::CreateDefaultPresObj() - unknown PRESOBJ kind" );
        return NULL;
    }
}

/*************************************************************************
|*
|* Titelbereich zurueckgeben
|*
\************************************************************************/

basegfx::B2DRange SdPage::GetTitleRange() const
{
    basegfx::B2DRange aTitleRange;

    if (mePageKind != PK_HANDOUT)
    {
        /******************************************************************
        * Standard- oder Notiz-Seite: Titelbereich
        ******************************************************************/
        basegfx::B2DPoint aTitlePos(GetLeftPageBorder(), GetTopPageBorder());
        basegfx::B2DVector aTitleSize(GetInnerPageScale());

        if (mePageKind == PK_STANDARD)
        {
            aTitlePos += aTitleSize * basegfx::B2DTuple(0.05, 0.0399);
            aTitleSize *= basegfx::B2DTuple(0.9, 0.167);
        }
        else if (mePageKind == PK_NOTES)
        {
            basegfx::B2DPoint aPos(aTitlePos.getX(), aTitlePos.getY() + (aTitleSize.getY() * 0.076));

            // Hoehe beschraenken
            aTitleSize.setY(aTitleSize.getY() * 0.375);

            basegfx::B2DVector aPartArea(aTitleSize);
            basegfx::B2DVector aSize;

            sal_uInt32 nDestPageNum(GetPageNumber());
            SdrPage* pRefPage = 0L;

            if(nDestPageNum)
            {
                // only decrement if != 0, else we get SDRPAGE_NOTFOUND
                nDestPageNum -= 1;
            }

            if(nDestPageNum < getSdrModelFromSdrPage().GetPageCount())
            {
                pRefPage = getSdrModelFromSdrPage().GetPage(nDestPageNum);
            }

            if ( pRefPage )
            {
                // tatsaechliche Seitengroesse in das Handout-Rechteck skalieren
                const basegfx::B2DVector aRefPageSize(pRefPage->GetPageScale());
                const basegfx::B2DVector aFactor(aPartArea / aRefPageSize);

                aSize = aRefPageSize * std::max(aFactor.getX(), aFactor.getY());
                aPos += (aPartArea - aSize) * 0.5;
            }

            aTitlePos = aPos;
            aTitleSize = aSize;
        }

        aTitleRange = basegfx::B2DRange(aTitlePos, aTitlePos + aTitleSize);
    }

    return aTitleRange;
}


/*************************************************************************
|*
|* Gliederungsbereich zurueckgeben
|*
\************************************************************************/

basegfx::B2DRange SdPage::GetLayoutRange() const
{
    basegfx::B2DRange aLayoutRange;

    if (mePageKind != PK_HANDOUT)
    {
        basegfx::B2DPoint aLayoutPos(GetLeftPageBorder(), GetTopPageBorder());
        basegfx::B2DVector aLayoutSize(GetInnerPageScale());

        if (mePageKind == PK_STANDARD)
        {
            aLayoutPos += aLayoutSize * basegfx::B2DTuple(0.05, 0.234);
            aLayoutSize *= basegfx::B2DTuple(0.9, 0.66);

            aLayoutRange = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);
        }
        else if (mePageKind == PK_NOTES)
        {
            aLayoutPos += aLayoutSize * basegfx::B2DTuple(0.1, 0.475);
            aLayoutSize *= basegfx::B2DTuple(0.8, 0.45);

            aLayoutRange = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);
        }
    }

    return aLayoutRange;
}


/**************************************************************************
|*
|* Diese Methode weist ein AutoLayout zu
|*
\*************************************************************************/

const int MAX_PRESOBJS = 7; // maximum number of presentation objects per layout
const int VERTICAL = 0x8000;

struct LayoutDescriptor
{
    int mnLayout;
    PresObjKind meKind[MAX_PRESOBJS];
    bool mbVertical[MAX_PRESOBJS];

    LayoutDescriptor( int nLayout, int k0 = 0, int k1 = 0, int k2 = 0, int k3 = 0, int k4 = 0, int k5 = 0, int k6 = 0 );
};

LayoutDescriptor::LayoutDescriptor( int nLayout, int k0, int k1, int k2, int k3, int k4, int k5, int k6 )
: mnLayout( nLayout )
{
    meKind[0] = static_cast<PresObjKind>(k0 & (~VERTICAL)); mbVertical[0] = (k0 & VERTICAL) == VERTICAL;
    meKind[1] = static_cast<PresObjKind>(k1 & (~VERTICAL)); mbVertical[1] = (k1 & VERTICAL) == VERTICAL;
    meKind[2] = static_cast<PresObjKind>(k2 & (~VERTICAL)); mbVertical[2] = (k2 & VERTICAL) == VERTICAL;
    meKind[3] = static_cast<PresObjKind>(k3 & (~VERTICAL)); mbVertical[3] = (k3 & VERTICAL) == VERTICAL;
    meKind[4] = static_cast<PresObjKind>(k4 & (~VERTICAL)); mbVertical[4] = (k4 & VERTICAL) == VERTICAL;
    meKind[5] = static_cast<PresObjKind>(k5 & (~VERTICAL)); mbVertical[5] = (k5 & VERTICAL) == VERTICAL;
    meKind[6] = static_cast<PresObjKind>(k6 & (~VERTICAL)); mbVertical[6] = (k6 & VERTICAL) == VERTICAL;
}

static const LayoutDescriptor& GetLayoutDescriptor( AutoLayout eLayout )
{
    static LayoutDescriptor aLayouts[AUTOLAYOUT__END-AUTOLAYOUT__START] =
    {
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_TEXT ),                                 // AUTOLAYOUT_TITLE
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_OUTLINE ),                              // AUTOLAYOUT_ENUM
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_OUTLINE ),                              // AUTOLAYOUT_CHART
        LayoutDescriptor( 1, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_2TEXT
        LayoutDescriptor( 1, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_TEXTCHART
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_OUTLINE ),                              // AUTOLAYOUT_ORG
        LayoutDescriptor( 1, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_TEXTCLbIP
        LayoutDescriptor( 1, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_CHARTTEXT
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_OUTLINE ),                              // AUTOLAYOUT_TAB
        LayoutDescriptor( 1, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_CLIPTEXT
        LayoutDescriptor( 1, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_TEXTOBJ
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_OBJECT ),                               // AUTOLAYOUT_OBJ
        LayoutDescriptor( 2, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),    // AUTOLAYOUT_TEXT2OBJ
        LayoutDescriptor( 1, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_TEXTOBJ
        LayoutDescriptor( 4, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_OBJOVERTEXT
        LayoutDescriptor( 3, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),    // AUTOLAYOUT_2OBJTEXT
        LayoutDescriptor( 5, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),    // AUTOLAYOUT_2OBJOVERTEXT
        LayoutDescriptor( 4, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_TEXTOVEROBJ
        LayoutDescriptor( 6, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE,                   // AUTOLAYOUT_4OBJ
            PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_NONE ),                                 // AUTOLAYOUT_ONLY_TITLE
        LayoutDescriptor( 0, PRESOBJ_NONE ),                                                // AUTOLAYOUT_NONE
        LayoutDescriptor( 0, PRESOBJ_PAGE, PRESOBJ_NOTES ),                                 // AUTOLAYOUT_NOTES
        LayoutDescriptor( 0 ),                                                              // AUTOLAYOUT_HANDOUT1
        LayoutDescriptor( 0 ),                                                              // AUTOLAYOUT_HANDOUT2
        LayoutDescriptor( 0 ),                                                              // AUTOLAYOUT_HANDOUT3
        LayoutDescriptor( 0 ),                                                              // AUTOLAYOUT_HANDOUT4
        LayoutDescriptor( 0 ),                                                              // AUTOLAYOUT_HANDOUT6
        LayoutDescriptor( 7, PRESOBJ_TITLE|VERTICAL, PRESOBJ_OUTLINE|VERTICAL, PRESOBJ_OUTLINE ),// AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART
        LayoutDescriptor( 8, PRESOBJ_TITLE|VERTICAL, PRESOBJ_OUTLINE|VERTICAL ),            // AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_OUTLINE|VERTICAL ),                     // AUTOLAYOUT_TITLE_VERTICAL_OUTLINE
        LayoutDescriptor( 9, PRESOBJ_TITLE, PRESOBJ_OUTLINE|VERTICAL, PRESOBJ_OUTLINE|VERTICAL ),   // AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART
        LayoutDescriptor( 0 ),                                                              // AUTOLAYOUT_HANDOUT9
        LayoutDescriptor( 10, PRESOBJ_TEXT, PRESOBJ_NONE ),                                 // AUTOLAYOUT_ONLY_TEXT
        LayoutDescriptor( 6, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE,               // AUTOLAYOUT_4CLIPART
            PRESOBJ_GRAPHIC, PRESOBJ_GRAPHIC ),
        LayoutDescriptor( 11, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE,              // AUTOLAYOUT_6CLIPART
            PRESOBJ_OUTLINE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE )
    };

    if( (eLayout < AUTOLAYOUT__START) || (eLayout >= AUTOLAYOUT__END) )
        eLayout = AUTOLAYOUT_NONE;

    return aLayouts[ eLayout - AUTOLAYOUT__START ];
}

static void CalcAutoLayoutRanges( SdPage& rPage, int nLayout, basegfx::B2DRange* rRanges )
{
    basegfx::B2DRange aTitleRange;
    basegfx::B2DRange aLayoutRange;

    if( rPage.GetPageKind() != PK_HANDOUT )
    {
        SdPage& rMasterPage = static_cast<SdPage&>(rPage.TRG_GetMasterPage());
        SdrObject* pMasterTitle = rMasterPage.GetPresObj( PRESOBJ_TITLE );
        SdrObject* pMasterSubTitle = rMasterPage.GetPresObj( PRESOBJ_TEXT );
        SdrObject* pMasterOutline = rMasterPage.GetPresObj( rPage.GetPageKind()==PK_NOTES ? PRESOBJ_NOTES : PRESOBJ_OUTLINE );

        if( pMasterTitle )
            aTitleRange = sdr::legacy::GetLogicRange(*pMasterTitle);

        if (aTitleRange.isEmpty() )
            aTitleRange = rPage.GetTitleRange();
        if( pMasterSubTitle )
            aLayoutRange = sdr::legacy::GetLogicRange(*pMasterSubTitle);
        else if( pMasterOutline )
            aLayoutRange = sdr::legacy::GetLogicRange(*pMasterOutline);

        if (aLayoutRange.isEmpty() )
            aLayoutRange = rPage.GetLayoutRange();
    }

    rRanges[0] = aTitleRange;

    int i;
    for( i = 1; i < MAX_PRESOBJS; i++ )
        rRanges[i] = aLayoutRange;

    basegfx::B2DVector aTitleSize(aTitleRange.getRange());
    basegfx::B2DPoint aTitlePos(aTitleRange.getMinimum());
    basegfx::B2DVector aLayoutSize(aLayoutRange.getRange());
    basegfx::B2DPoint aLayoutPos(aLayoutRange.getMinimum());
    basegfx::B2DVector aTempSize;
    basegfx::B2DPoint aTempPos;

    SdDrawDocument& rDrawDocument = static_cast< SdDrawDocument& >(rPage.getSdrModelFromSdrPage());
    const bool bRightToLeft(::com::sun::star::text::WritingMode_RL_TB == rDrawDocument.GetDefaultWritingMode());

    switch( nLayout )
    {
    case 0: // default layout using only the title and layout area
        break; // do nothing
    case 1: // title, 2 shapes
    case 9: // title, 2 vertical shapes
        aLayoutSize.setX(aLayoutSize.getX() * 0.488);
        rRanges[1] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos.setX(aLayoutPos.getX() + (aLayoutSize.getX() * 1.05));
        rRanges[2] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        if( bRightToLeft && (nLayout != 9) )
        {
            ::std::swap< basegfx::B2DRange >( rRanges[1], rRanges[2] );
        }
        break;
    case 2: // title, shape, 2 shapes
        aTempPos = aLayoutPos;
        aTempSize = aLayoutSize;

        aLayoutSize *= basegfx::B2DTuple(0.488, 0.477);
        aLayoutPos.setX(aLayoutPos.getX() + (aLayoutSize.getX() * 1.05));
        rRanges[2] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos.setY(aLayoutPos.getY() + (aLayoutSize.getY() * 1.095));
        rRanges[3] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos = aTempPos;
        aLayoutSize = aTempSize;

        aLayoutSize.setX(aLayoutSize.getX() * 0.488);
        rRanges[1] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        if( bRightToLeft )
        {
            // [2] nad [3] are on the right side, [1] is on the left.
            const double fDeltaX(rRanges[1].getMinX() - rRanges[2].getMinX());
            const basegfx::B2DHomMatrix aTransToLeft(basegfx::tools::createTranslateB2DHomMatrix(fDeltaX, 0.0));
            const basegfx::B2DHomMatrix aTransToRight(basegfx::tools::createTranslateB2DHomMatrix(-fDeltaX, 0.0));

            rRanges[1].transform(aTransToRight);
            rRanges[2].transform(aTransToLeft);
            rRanges[3].transform(aTransToLeft);
        }
        break;
    case 3: // title, 2 shapes, shape
        aTempPos = aLayoutPos;
        aTempSize = aLayoutSize;

        aLayoutSize *= basegfx::B2DTuple(0.488, 0.477);
        rRanges[1] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos.setY(aLayoutPos.getY() + (aLayoutSize.getY() * 1.095));
        rRanges[2] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos = aTempPos;
        aLayoutSize = aTempSize;

        aLayoutSize.setX(aLayoutSize.getX() * 0.488);
        aLayoutPos.setX(aLayoutPos.getX() + (aLayoutSize.getX() * 1.05));
        rRanges[3] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        if( bRightToLeft )
        {
            // [1] and [2] are left, [3] is right
            const double fDeltaX(rRanges[3].getMinX() - rRanges[1].getMinX());
            const basegfx::B2DHomMatrix aTransToRight(basegfx::tools::createTranslateB2DHomMatrix(fDeltaX, 0.0));
            const basegfx::B2DHomMatrix aTransToLeft(basegfx::tools::createTranslateB2DHomMatrix(-fDeltaX, 0.0));

            rRanges[1].transform(aTransToRight);
            rRanges[2].transform(aTransToRight);
            rRanges[3].transform(aTransToLeft);
        }
        break;
    case 4: // title, shape above shape
        aLayoutSize.setY(aLayoutSize.getY() * 0.477);
        rRanges[1] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos.setY(aLayoutPos.getY() + (aLayoutSize.getY() * 1.095));
        rRanges[2] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);
        break;

    case 5: // title, 2 shapes above shape
        aLayoutSize *= basegfx::B2DTuple(0.488, 0.477);
        rRanges[1] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aTempPos = aLayoutPos;
        aLayoutPos.setX(aLayoutPos.getX() + (aLayoutSize.getX() * 1.05));
        rRanges[2] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos.setX(aTempPos.getX());
        aLayoutPos.setY(aLayoutPos.getY() + (aLayoutSize.getY() * 1.095));
        aLayoutSize.setX(aLayoutSize.getX() / 0.488);
        rRanges[3] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);
        break;
    case 6: // title, 4 shapes
    {
        const double fX(aLayoutPos.getX());

        aLayoutSize *= basegfx::B2DTuple(0.488, 0.477);
        rRanges[1] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos.setX(fX + (aLayoutSize.getX() * 1.05));
        rRanges[2] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos.setY(aLayoutPos.getY() + (aLayoutSize.getY() * 1.095));
        rRanges[3] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos.setX(fX);
        rRanges[4] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);
        break;
    }
    case 7: // vertical title, shape above shape
    {
        basegfx::B2DVector aSize(rRanges[0].getHeight(), rRanges[1].getMaxY() - rRanges[0].getMinY());
        basegfx::B2DPoint aPos(aTitleRange.getMaxX() - aSize.getX(), aTitleRange.getMinY());

        rRanges[0] = basegfx::B2DRange(aPos, aPos + aSize);

        const basegfx::B2DVector aPageSize(rPage.GetPageScale().getX(), rPage.GetPageScale().getY() - (rPage.GetTopPageBorder() + rPage.GetBottomPageBorder()));

        aSize.setX(aPageSize.getX() * 0.7);
        aSize.setY(rRanges[0].getHeight() * 0.47);

        rRanges[1] = basegfx::B2DRange(aTitleRange.getMinimum(), aTitleRange.getMinimum() + aSize);

        aSize.setY(rRanges[0].getHeight());
        aPos = aTitleRange.getMinimum();
        aPos.setY(aPos.getY() + (aSize.getY() * 0.53));
        aSize.setY(rRanges[0].getHeight() * 0.47);

        rRanges[2] = basegfx::B2DRange(aPos, aPos + aSize);
        break;
    }
    case 8: // vertical title, shape
    {
        basegfx::B2DVector aSize(rRanges[0].getHeight(), rRanges[1].getMaxY() - rRanges[0].getMinY());
        const basegfx::B2DPoint aPos(aTitleRange.getMaxX() - aSize.getX(), aTitleRange.getMinY());

        rRanges[0] = basegfx::B2DRange(aPos, aPos + aSize);

        const basegfx::B2DVector aPageSize(rPage.GetPageScale().getX(), rPage.GetPageScale().getY() - (rPage.GetTopPageBorder() + rPage.GetBottomPageBorder()));

        aSize.setX(aPageSize.getX() * 0.7);
        aSize.setY(rRanges[0].getHeight());

        rRanges[1] = basegfx::B2DRange(aTitleRange.getMinimum(), aTitleRange.getMinimum() + aSize);
        break;
    }
    case 10: // onlytext
    {
        const basegfx::B2DVector aSize(rRanges[0].getWidth(), rRanges[1].getMaxY() - rRanges[0].getMinY());

        rRanges[0] = basegfx::B2DRange(aTitlePos, aTitlePos + aSize);
        break;
    }
    case 11: // title, 6 shapes
    {
        const double fX(aLayoutPos.getX());

        aLayoutSize *= basegfx::B2DTuple(0.322, 0.477);
        rRanges[1] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos.setX(fX + (aLayoutSize.getX() * 1.05));
        rRanges[2] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos.setX(fX + (aLayoutSize.getX() * (2.0 * 1.05)));
        rRanges[3] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos.setY(aLayoutPos.getY() + (aLayoutSize.getY() * 1.095));
        rRanges[4] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos.setX(fX + (aLayoutSize.getX() * 1.05));
        rRanges[5] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        aLayoutPos.setX(fX);
        rRanges[6] = basegfx::B2DRange(aLayoutPos, aLayoutPos + aLayoutSize);

        break;
    }

    }
}


void findAutoLayoutShapesImpl( SdPage& rPage, const LayoutDescriptor& rDescriptor, SdrObjectVector& rShapes, bool bInit, bool bSwitchLayout )
{
    int i;

    // init list of indexes for each presentation shape kind
    // this is used to find subsequent shapes with the same presentation shape kind
    int PresObjIndex[PRESOBJ_MAX];
    for( i = 0; i < PRESOBJ_MAX; i++ ) PresObjIndex[i] = 1;

    bool bMissing = false;

    // for each entry in the layoutdescriptor, arrange a presentation shape
    for (i = 0; (i < MAX_PRESOBJS) && (rDescriptor.meKind[i] != PRESOBJ_NONE); i++)
    {
        PresObjKind eKind = rDescriptor.meKind[i];
        SdrObject* pObj = 0;
        while( (pObj = rPage.GetPresObj( eKind, PresObjIndex[eKind], true )) != 0 )
        {
            PresObjIndex[eKind]++; // on next search for eKind, find next shape with same eKind

            if( !bSwitchLayout || !pObj->IsEmptyPresObj() )
            {
                rShapes[i] = pObj;
                break;
            }
        }

        if( !pObj )
            bMissing = true;
    }

    if( bMissing && bInit )
    {
        // for each entry in the layoutdescriptor, look for an alternative shape
        for (i = 0; (i < MAX_PRESOBJS) && (rDescriptor.meKind[i] != PRESOBJ_NONE); i++)
        {
            if( rShapes[i] )
                continue;

            PresObjKind eKind = rDescriptor.meKind[i];

            SdrObject* pObj = 0;
            bool bFound = false;

            const int nShapeCount = rPage.GetObjCount();
            int nShapeIndex = 0;
            while((nShapeIndex < nShapeCount) && !bFound )
            {
                pObj = rPage.GetObj(nShapeIndex++);

                if( pObj->IsEmptyPresObj() )
                    continue;

                if( pObj->GetObjInventor() != SdrInventor )
                    continue;

                // do not reuse shapes that are already part of the layout
                if( std::find( rShapes.begin(), rShapes.end(), pObj ) != rShapes.end() )
                    continue;

                bool bPresStyle = pObj->GetStyleSheet() && (pObj->GetStyleSheet()->GetFamily() == SD_STYLE_FAMILY_MASTERPAGE);
                SdrObjKind eSdrObjKind = static_cast< SdrObjKind >( pObj->GetObjIdentifier() );

                switch( eKind )
                {
                case PRESOBJ_TITLE:
                    bFound = eSdrObjKind == OBJ_TITLETEXT;
                    break;
                case PRESOBJ_TABLE:
                    bFound = eSdrObjKind == OBJ_TABLE;
                    break;
                case PRESOBJ_MEDIA:
                    bFound = eSdrObjKind == OBJ_MEDIA;
                    break;
                case PRESOBJ_OUTLINE:
                    bFound = (eSdrObjKind == OBJ_OUTLINETEXT) ||
                             ((eSdrObjKind == OBJ_TEXT) && bPresStyle) ||
                             (eSdrObjKind == OBJ_TABLE) || (eSdrObjKind == OBJ_MEDIA) || (eSdrObjKind == OBJ_GRAF) || (eSdrObjKind == OBJ_OLE2);
                    break;
                case PRESOBJ_GRAPHIC:
                    bFound = eSdrObjKind == OBJ_GRAF;
                    break;
                case PRESOBJ_OBJECT:
                    if( eSdrObjKind == OBJ_OLE2 )
                    {
                        SdrOle2Obj* pOle2 = dynamic_cast< SdrOle2Obj* >( pObj );
                        if( pOle2 )
                        {
                            if( pOle2->IsEmpty() )
                            {
                                bFound = true;
                            }
                            else
                            {
                                SdDrawDocument& rDrawDocument = static_cast< SdDrawDocument& >(rPage.getSdrModelFromSdrPage());
                                ::comphelper::IEmbeddedHelper *pPersist = rDrawDocument.GetPersist();
                                if( pPersist )
                                {
                                    uno::Reference < embed::XEmbeddedObject > xObject = pPersist->getEmbeddedObjectContainer().
                                            GetEmbeddedObject( static_cast< SdrOle2Obj* >( pObj )->GetPersistName() );

                                    // TODO CL->KA: Why is this not working anymore?
                                    if( xObject.is() )
                                    {
                                        SvGlobalName aClassId( xObject->getClassID() );

                                        const SvGlobalName aAppletClassId( SO3_APPLET_CLASSID );
                                        const SvGlobalName aPluginClassId( SO3_PLUGIN_CLASSID );
                                        const SvGlobalName aIFrameClassId( SO3_IFRAME_CLASSID );

                                        if( aPluginClassId != aClassId && aAppletClassId != aClassId && aIFrameClassId != aClassId )
                                        {
                                            bFound = true;
                                        }
                                    }
                                }
                             }
                         }
                    }
                    break;
                case PRESOBJ_CHART:
                case PRESOBJ_CALC:
                    if( eSdrObjKind == OBJ_OLE2 )
                    {
                        SdrOle2Obj* pOle2 = dynamic_cast< SdrOle2Obj* >( pObj );
                        if( pOle2 )
                        {
                            if(
                                ((eKind == PRESOBJ_CHART) &&
                                    ( pOle2->GetProgName().EqualsAscii( "StarChart" ) || pOle2->IsChart() ) )
                                ||
                                ((eKind == PRESOBJ_CALC) &&
                                    ( pOle2->GetProgName().EqualsAscii( "StarCalc" ) || pOle2->IsCalc() ) ) )
                            {
                                bFound = true;
                            }
                        }
                        break;
                    }
                    else if( eSdrObjKind == OBJ_TABLE )
                    {
                        bFound = true;
                    }
                    break;
                case PRESOBJ_PAGE:
                case PRESOBJ_HANDOUT:
                    bFound = eSdrObjKind == OBJ_PAGE;
                    break;
                case PRESOBJ_NOTES:
                case PRESOBJ_TEXT:
                    bFound = (bPresStyle && (eSdrObjKind == OBJ_TEXT)) || (eSdrObjKind == OBJ_OUTLINETEXT);
                    break;
                default:
                    break;
                }
            }

            if( bFound )
                rShapes[i] = pObj;
        }
    }
}

void SdPage::SetAutoLayout(AutoLayout eLayout, bool bInit, bool bCreate )
{
    sd::ScopeLockGuard aGuard( maLockAutoLayoutArrangement );

    const bool bSwitchLayout = eLayout != GetAutoLayout();
    ::svl::IUndoManager* pUndoManager = static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetUndoManager();
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

    meAutoLayout = eLayout;

    // if needed, creates and initialises the presentation shapes on this slides master page
    CreateTitleAndLayout(bInit, bCreate);

    if((meAutoLayout == AUTOLAYOUT_NONE && maPresentationShapeList.isEmpty()) || mbMaster)
    {
        // MasterPage or no layout and no presentation shapes available, noting to do
        return;
    }

    basegfx::B2DRange aRanges[MAX_PRESOBJS];
    const LayoutDescriptor& aDescriptor = GetLayoutDescriptor( meAutoLayout );
    CalcAutoLayoutRanges( *this, aDescriptor.mnLayout, aRanges );

    std::set< SdrObject* > aUsedPresentationObjects;


    SdrObjectVector aLayoutShapes(PRESOBJ_MAX, 0);
    findAutoLayoutShapesImpl( *this, aDescriptor, aLayoutShapes, bInit, bSwitchLayout );

    int i;

    // for each entry in the layoutdescriptor, arrange a presentation shape
    for (i = 0; (i < MAX_PRESOBJS) && (aDescriptor.meKind[i] != PRESOBJ_NONE); i++)
    {
        PresObjKind eKind = aDescriptor.meKind[i];
        SdrObject* pObj = InsertAutoLayoutShape( aLayoutShapes[i], eKind, aDescriptor.mbVertical[i], aRanges[i], bInit );
        if( pObj )
            aUsedPresentationObjects.insert(pObj); // remember that we used this empty shape
    }

    // now delete all empty presentation objects that are no longer used by the new layout
    if( bInit )
    {
        SdrObject* pObj = const_cast< SdrObject* >(maPresentationShapeList.getNextShape(0));

        while( pObj )
        {
            SdrObject* pNext = const_cast< SdrObject* >(maPresentationShapeList.getNextShape(pObj));

            if( aUsedPresentationObjects.count(pObj) == 0 )
            {

                if( pObj->IsEmptyPresObj() )
                {
                    if( bUndo )
                        pUndoManager->AddUndoAction(getSdrModelFromSdrPage().GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));

                    RemoveObjectFromSdrObjList( pObj->GetNavigationPosition() );

                    if( !bUndo )
                    {
                        deleteSdrObjectSafeAndClearPointer( pObj );
                    }
                }
/* #i108541# keep non empty pres obj as pres obj even if they are not part of the current layout
                else
                {
                    if( bUndo )
                    {
                        pUndoManager->AddUndoAction( new UndoObjectPresentationKind( *pObj ) );
                        if( findConnectionToSdrObject(pObj) )
                            pUndoManager->AddUndoAction( new UndoConnectionToSdrObject( *pObj ) );
                    }
                    maPresentationShapeList.removeShape( *pObj );
                    resetConnectionToSdrObject(pObj);
                }
*/
            }
            pObj = pNext;
        }
    }
}

// #95876# Also overload ReplaceObject methods to realize when
// objects are removed with this mechanism instead of RemoveObject
// -------------------------------------------------------------------------
// called after a shape is removed/replaced inserted from this slide

void SdPage::handleContentChange(const SfxHint& rHint)
{
    const SdrBaseHint* pSdrBaseHint = dynamic_cast< const SdrBaseHint* >(&rHint);

    if(pSdrBaseHint)
    {
        switch(pSdrBaseHint->GetSdrHintKind())
        {
            case HINT_OBJINSERTED:
            {
                onInsertObject(pSdrBaseHint->GetSdrHintObject());
                break;
            }
            case HINT_OBJREMOVED:
            {
                onRemoveObject(pSdrBaseHint->GetSdrHintObject());
                break;
            }
            default:
            {
                break;
            }
        }
    }
}

void SdPage::onInsertObject(const SdrObject* pObject)
{
    if(pObject)
    {
        static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).InsertObject(pObject);
        SdrLayerID nId = pObject->GetLayer();

        if( mbMaster )
        {
            if( nId == 0 )
            {
                // wrong layer. corrected to BackgroundObj layer
                const_cast< SdrObject* >(pObject)->SetLayer( 2 );
            }
        }
        else
        {
            if( nId == 2 )
            {
                // wrong layer. corrected to layout layer
                const_cast< SdrObject* >(pObject)->SetLayer( 0 );
            }
        }
    }
}

void SdPage::onRemoveObject(const SdrObject* pObject)
{
    if(pObject)
    {
        RemovePresObj(pObject);
        static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).RemoveObject(pObject);
        removeAnimations( pObject );
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdPage::SetPageScale(const basegfx::B2DVector& aNewScale)
{
    const basegfx::B2DVector aOldScale(GetPageScale());

    if(!aNewScale.equal(aOldScale))
    {
        FmFormPage::SetPageScale(aNewScale);

        if(aOldScale.getX() == 10.0 && aOldScale.getY() == 10.0)
        {
            // Die Seite bekommt erstmalig eine gueltige Groesse gesetzt,
            // daher wird nun die Orientation initialisiert
            if (aNewScale.getX() > aNewScale.getY())
            {
                meOrientation = ORIENTATION_LANDSCAPE;
            }
            else
            {
                meOrientation = ORIENTATION_PORTRAIT;
            }
        }
    }
}


/*************************************************************************
|*
|* Setzt BackgroundFullSize und ruft dann AdjustBackground auf
|*
\************************************************************************/

void SdPage::SetBackgroundFullSize( bool bIn )
{
    if( bIn != mbBackgroundFullSize )
    {
        mbBackgroundFullSize = bIn;
    }
}

/*************************************************************************
|*
|* Alle Objekte an neue Seitengroesse anpassen
|*
|* bScaleAllObj: Alle Objekte werden in die neue Flaeche innerhalb der
|* Seitenraender skaliert. Dabei werden die Position, Groesse und bei
|* Praesentationsobjekten auf der MasterPage auch die Schrifthoehe der
|* Praesentationsvorlagen skaliert.
|*
\************************************************************************/

void SdPage::ScaleObjects(const basegfx::B2DVector& rNewPageSize, double fLeft, double fTop, double fRight, double fBottom, bool bScaleAllObj)
{
    sd::ScopeLockGuard aGuard( maLockAutoLayoutArrangement );

    mbScaleObjects = bScaleAllObj;
    SdrObject* pObj = NULL;
    basegfx::B2DPoint aRefPoint(0.0, 0.0);
    basegfx::B2DVector aNewPageSize(rNewPageSize);

    // Negative Werte stehen fuer nicht zu aendernde Werte
    // -> aktuelle Werte verwenden
    if (aNewPageSize.getX() < 0.0)
    {
        aNewPageSize.setX(GetPageScale().getX());
    }
    if (aNewPageSize.getY() < 0.0)
    {
        aNewPageSize.setY(GetPageScale().getY());
    }
    if (fLeft < 0)
    {
        fLeft = GetLeftPageBorder();
    }
    if (fRight < 0)
    {
        fRight = GetRightPageBorder();
    }
    if (fTop < 0)
    {
        fTop = GetTopPageBorder();
    }
    if (fBottom < 0)
    {
        fBottom = GetBottomPageBorder();
    }

    if (mbScaleObjects)
    {
        aNewPageSize.setX(aNewPageSize.getX() - (fLeft  + fRight));
        aNewPageSize.setY(aNewPageSize.getY() - (fTop + fBottom));
    }

    const basegfx::B2DVector aFactor(
        aNewPageSize.getX() / (basegfx::fTools::equalZero(GetInnerPageScale().getX()) ? 1.0 : GetInnerPageScale().getX()),
        aNewPageSize.getY() / (basegfx::fTools::equalZero(GetInnerPageScale().getY()) ? 1.0 : GetInnerPageScale().getY()));
    const sal_uInt32 nObjCnt(mbScaleObjects ? GetObjCount() : 0);

    for (sal_uInt32 nObj = 0; nObj < nObjCnt; nObj++)
    {
        bool bIsPresObjOnMaster = false;

        // Alle Objekte
        pObj = GetObj(nObj);

        if (mbMaster && IsPresObj(pObj))
        {
            // Es ist ein Praesentationsobjekt auf der MasterPage
            bIsPresObjOnMaster = true;
        }

        if (pObj)
        {
            // #88084# remember aTopLeft as original TopLeft
            const basegfx::B2DPoint aTopLeft(pObj->getObjectRange(0).getMinimum());

            if (!dynamic_cast< SdrEdgeObj* >(pObj))
            {
                /**************************************************************
                * Objekt skalieren
                **************************************************************/
                if (mbScaleObjects)
                {
                    // #88084# use aTopLeft as original TopLeft
                    aRefPoint = aTopLeft;
                }

                {
                    basegfx::B2DHomMatrix aTransform;

                    aTransform.translate(-aRefPoint);
                    aTransform.scale(aFactor);
                    aTransform.translate(aRefPoint);

                    sdr::legacy::transformSdrObject(*pObj, aTransform);
                }

                if (mbScaleObjects)
                {
                    SdrObjKind eObjKind = (SdrObjKind) pObj->GetObjIdentifier();

                    if (bIsPresObjOnMaster)
                    {
                        /**********************************************************
                        * Praesentationsvorlage: Texthoehe anpassen
                        **********************************************************/
                        sal_uInt32 nIndexTitle = 0;
                        sal_uInt32 nIndexOutline = 0;
                        sal_uInt32 nIndexNotes = 0;

                        if (pObj == GetPresObj(PRESOBJ_TITLE, nIndexTitle))
                        {
                            SfxStyleSheet* pTitleSheet = GetStyleSheetForPresObj(PRESOBJ_TITLE);

                            if (pTitleSheet)
                            {
                                SfxItemSet& rSet = pTitleSheet->GetItemSet();

                                SvxFontHeightItem& rOldHgt = (SvxFontHeightItem&) rSet.Get(EE_CHAR_FONTHEIGHT);
                                sal_uInt32 nFontHeight = rOldHgt.GetHeight();
                                nFontHeight = long(nFontHeight * aFactor.getY());
                                rSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT));

                                if( SFX_ITEM_AVAILABLE == rSet.GetItemState( EE_CHAR_FONTHEIGHT_CJK ) )
                                {
                                    rOldHgt = (SvxFontHeightItem&) rSet.Get(EE_CHAR_FONTHEIGHT_CJK);
                                    nFontHeight = rOldHgt.GetHeight();
                                    nFontHeight = long(nFontHeight * aFactor.getY());
                                    rSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CJK));
                                }

                                if( SFX_ITEM_AVAILABLE == rSet.GetItemState( EE_CHAR_FONTHEIGHT_CTL ) )
                                {
                                    rOldHgt = (SvxFontHeightItem&) rSet.Get(EE_CHAR_FONTHEIGHT_CTL);
                                    nFontHeight = rOldHgt.GetHeight();
                                    nFontHeight = long(nFontHeight * aFactor.getY());
                                    rSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CTL));
                                }

                                pTitleSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                            }
                        }
                        else if (pObj == GetPresObj(PRESOBJ_OUTLINE, nIndexOutline))
                        {
                            String aName(GetLayoutName());
                            aName += sal_Unicode( ' ' );

                            for (sal_uInt32 i=1; i<=9; i++)
                            {
                                String sLayoutName(aName);
                                sLayoutName += String::CreateFromInt32( i );
                                SfxStyleSheet* pOutlineSheet = dynamic_cast< SfxStyleSheet* >(
                                    static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetStyleSheetPool()->Find(sLayoutName, SD_STYLE_FAMILY_MASTERPAGE));

                                if (pOutlineSheet)
                                {
                                    // Neue Fonthoehe berechnen
                                    SfxItemSet aTempSet(pOutlineSheet->GetItemSet());

                                    SvxFontHeightItem& rOldHgt = (SvxFontHeightItem&) aTempSet.Get(EE_CHAR_FONTHEIGHT);
                                    sal_uInt32 nFontHeight = rOldHgt.GetHeight();
                                    nFontHeight = long(nFontHeight * aFactor.getY());
                                    aTempSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT));

                                    if( SFX_ITEM_AVAILABLE == aTempSet.GetItemState( EE_CHAR_FONTHEIGHT_CJK ) )
                                    {
                                        rOldHgt = (SvxFontHeightItem&) aTempSet.Get(EE_CHAR_FONTHEIGHT_CJK);
                                        nFontHeight = rOldHgt.GetHeight();
                                        nFontHeight = long(nFontHeight * aFactor.getY());
                                        aTempSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CJK));
                                    }

                                    if( SFX_ITEM_AVAILABLE == aTempSet.GetItemState( EE_CHAR_FONTHEIGHT_CTL ) )
                                    {
                                        rOldHgt = (SvxFontHeightItem&) aTempSet.Get(EE_CHAR_FONTHEIGHT_CTL);
                                        nFontHeight = rOldHgt.GetHeight();
                                        nFontHeight = long(nFontHeight * aFactor.getY());
                                        aTempSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CTL));
                                    }

                                    // Bullet anpassen
                                    ((SdStyleSheet*) pOutlineSheet)->AdjustToFontHeight(aTempSet, false);

                                    // Sonderbehandlung: die INVALIDS auf NULL-Pointer
                                    // zurueckgesetzen (sonst landen INVALIDs oder
                                    // Pointer auf die DefaultItems in der Vorlage;
                                    // beides wuerde die Attribut-Vererbung unterbinden)
                                    aTempSet.ClearInvalidItems();

                                    // Sonderbehandlung: nur die gueltigen Anteile des
                                    // BulletItems
                                    if (aTempSet.GetItemState(EE_PARA_BULLET) == SFX_ITEM_AVAILABLE)
                                    {
                                        SvxBulletItem aOldBulItem((SvxBulletItem&) pOutlineSheet->GetItemSet().Get(EE_PARA_BULLET));
                                        SvxBulletItem& rNewBulItem = (SvxBulletItem&) aTempSet.Get(EE_PARA_BULLET);
                                        aOldBulItem.CopyValidProperties(rNewBulItem);
                                        aTempSet.Put(aOldBulItem);
                                    }

                                    pOutlineSheet->GetItemSet().Put(aTempSet);
                                    pOutlineSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                                }
                            }
                        }
                        else if (pObj == GetPresObj(PRESOBJ_NOTES, nIndexNotes))
                        {
                            SfxStyleSheet* pNotesSheet = GetStyleSheetForPresObj(PRESOBJ_NOTES);

                            if (pNotesSheet)
                            {
                                sal_uInt32 nHeight = sdr::legacy::GetLogicRect(*pObj).GetSize().Height();
                                sal_uInt32 nFontHeight = (sal_uInt32) (nHeight * 0.0741);
                                SfxItemSet& rSet = pNotesSheet->GetItemSet();
                                rSet.Put( SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT ));
                                rSet.Put( SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CJK ));
                                rSet.Put( SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CTL ));
                                pNotesSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                            }
                        }
                    }
                    else if ( eObjKind != OBJ_TITLETEXT   &&
                              eObjKind != OBJ_OUTLINETEXT &&
                              dynamic_cast< SdrTextObj* >(pObj) &&
                              pObj->GetOutlinerParaObject() )
                    {
                        /******************************************************
                        * Normales Textobjekt: Texthoehe anpassen
                        ******************************************************/
                        sal_uInt32 nScriptType = pObj->GetOutlinerParaObject()->GetTextObject().GetScriptType();
                        sal_uInt16 nWhich = EE_CHAR_FONTHEIGHT;
                        if ( nScriptType == SCRIPTTYPE_ASIAN )
                            nWhich = EE_CHAR_FONTHEIGHT_CJK;
                        else if ( nScriptType == SCRIPTTYPE_COMPLEX )
                            nWhich = EE_CHAR_FONTHEIGHT_CTL;

                        // #88084# use more modern method to scale the text height
                        sal_uInt32 nFontHeight = ((SvxFontHeightItem&)pObj->GetMergedItem(nWhich)).GetHeight();
                        sal_uInt32 nNewFontHeight = sal_uInt32((double)nFontHeight * aFactor.getY());

                        pObj->SetMergedItem(SvxFontHeightItem(nNewFontHeight, 100, nWhich));
                    }
                }
            }

            if (mbScaleObjects && !dynamic_cast< SdrEdgeObj* >(pObj))
            {
                // scale object position
                const basegfx::B2DPoint aNewPos(
                    ((aTopLeft.getX() - GetLeftPageBorder()) * aFactor.getX()) + fLeft,
                    ((aTopLeft.getY() - GetTopPageBorder()) * aFactor.getY()) + fTop);

                sdr::legacy::transformSdrObject(*pObj, basegfx::tools::createTranslateB2DHomMatrix(aNewPos - aTopLeft));
            }
        }
    }
}

SdrObject* convertPresentationObjectImpl( SdPage& rPage, SdrObject* pSourceObj, PresObjKind& o_eObjKind, bool bVertical, const basegfx::B2DRange& rRange)
{
    SdDrawDocument& rLocalDrawModel = static_cast< SdDrawDocument& >(rPage.getSdrModelFromSdrPage());
    if( !pSourceObj )
        return pSourceObj;

    ::svl::IUndoManager* pUndoManager = rLocalDrawModel.GetUndoManager();
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && rPage.IsInserted();

    SdrObject* pNewObj = pSourceObj;
    if((o_eObjKind == PRESOBJ_OUTLINE) && (pSourceObj->GetObjIdentifier() == OBJ_TEXT) )
    {
        pNewObj = rPage.CreatePresObj(PRESOBJ_OUTLINE, bVertical, rRange);

        // Text des Untertitels in das PRESOBJ_OUTLINE setzen
        OutlinerParaObject* pOutlParaObj = pSourceObj->GetOutlinerParaObject();

        if(pOutlParaObj)
        {
            // Text umsetzen
            ::sd::Outliner* pOutl = rLocalDrawModel.GetInternalOutliner( true );
            pOutl->Clear();
            pOutl->SetText( *pOutlParaObj );
            pOutlParaObj = pOutl->CreateParaObject();
            pNewObj->SetOutlinerParaObject( pOutlParaObj );
            pOutl->Clear();
            pNewObj->SetEmptyPresObj(false);

            for (sal_uInt16 nLevel = 1; nLevel < 10; nLevel++)
            {
                // Neue Vorlage zuweisen
                String aName(rPage.GetLayoutName());
                aName += sal_Unicode( ' ' );
                aName += String::CreateFromInt32( nLevel );
                SfxStyleSheet* pSheet = static_cast< SfxStyleSheet* >(
                    rLocalDrawModel.GetStyleSheetPool()->Find(aName, SD_STYLE_FAMILY_MASTERPAGE));

                if (pSheet)
                {
                    if (nLevel == 1)
                    {
                        SfxStyleSheet* pSubtitleSheet = rPage.GetStyleSheetForPresObj(PRESOBJ_TEXT);

                        if (pSubtitleSheet)
                            pOutlParaObj->ChangeStyleSheetName(SD_STYLE_FAMILY_MASTERPAGE, pSubtitleSheet->GetName(), pSheet->GetName());
                    }

                    pNewObj->StartListening(*pSheet);
                }
            }

            // LRSpace-Item loeschen
            SfxItemSet aSet(pNewObj->GetObjectItemPool());

            aSet.Put(pNewObj->GetMergedItemSet());
            aSet.ClearItem(EE_PARA_LRSPACE);

            pNewObj->SetMergedItemSet(aSet);

            if( bUndo )
                pUndoManager->AddUndoAction(rLocalDrawModel.GetSdrUndoFactory().CreateUndoDeleteObject(*pSourceObj) );

            // Remove outline shape from page
            rPage.RemoveObjectFromSdrObjList( pSourceObj->GetNavigationPosition() );

            if( !bUndo )
            {
                deleteSdrObjectSafeAndClearPointer( pSourceObj );
        }
    }
    }
    else if((o_eObjKind == PRESOBJ_TEXT) && (pSourceObj->GetObjIdentifier() == OBJ_OUTLINETEXT) )
    {
        // is there an outline shape we can use to replace empty subtitle shape?
        pNewObj = rPage.CreatePresObj(PRESOBJ_TEXT, bVertical, rRange);

        // Text des Gliederungsobjekts in das PRESOBJ_TITLE setzen
        OutlinerParaObject* pOutlParaObj = pSourceObj->GetOutlinerParaObject();

        if(pOutlParaObj)
        {
            // Text umsetzen
            ::sd::Outliner* pOutl = rLocalDrawModel.GetInternalOutliner();
            pOutl->Clear();
            pOutl->SetText( *pOutlParaObj );
            pOutlParaObj = pOutl->CreateParaObject();
            pNewObj->SetOutlinerParaObject( pOutlParaObj );
            pOutl->Clear();
            pNewObj->SetEmptyPresObj(false);

            // Linken Einzug zuruecksetzen
            SfxItemSet aSet(pNewObj->GetObjectItemPool(), EE_PARA_LRSPACE, EE_PARA_LRSPACE );

            aSet.Put(pNewObj->GetMergedItemSet());

            const SvxLRSpaceItem& rLRItem = (const SvxLRSpaceItem&) aSet.Get(EE_PARA_LRSPACE);
            SvxLRSpaceItem aNewLRItem(rLRItem);
            aNewLRItem.SetTxtLeft(0);
            aSet.Put(aNewLRItem);

            pNewObj->SetMergedItemSet(aSet);

            SfxStyleSheet* pSheet = rPage.GetStyleSheetForPresObj(PRESOBJ_TEXT);
            if (pSheet)
                pNewObj->SetStyleSheet(pSheet, true);

            // Remove subtitle shape from page
            if( bUndo )
                pUndoManager->AddUndoAction(rLocalDrawModel.GetSdrUndoFactory().CreateUndoDeleteObject(*pSourceObj));

            rPage.RemoveObjectFromSdrObjList( pSourceObj->GetNavigationPosition() );

            if( !bUndo )
            {
                deleteSdrObjectSafeAndClearPointer( pSourceObj );
            }
        }
    }
    else if((o_eObjKind == PRESOBJ_OUTLINE) && (pSourceObj->GetObjIdentifier() != OBJ_OUTLINETEXT) )
    {
        switch( pSourceObj->GetObjIdentifier() )
        {
        case OBJ_TABLE: o_eObjKind = PRESOBJ_TABLE; break;
        case OBJ_MEDIA: o_eObjKind = PRESOBJ_MEDIA; break;
        case OBJ_GRAF: o_eObjKind = PRESOBJ_GRAPHIC; break;
        case OBJ_OLE2: o_eObjKind = PRESOBJ_OBJECT; break;
        }
    }

    return pNewObj;
}

/** reuses or creates a presentation shape for an auto layout that fits the given parameter

    @param  eObjKind
        The kind of presentation shape we like to have
    @param  nIndex
        If > 1 we skip the first nIndex-1 shapes with the presentation shape kind eObjKind while
        looking for an existing presentation shape
    @param  bVertical
        If true, the shape is created vertical if bInit is true
    @param  rRange
        The rectangle that should be used to transform the shape
    @param  bInit
        If true the shape is created if not found
    @returns
        A presentation shape that was either found or created with the given parameters
*/
SdrObject* SdPage::InsertAutoLayoutShape( SdrObject* pObj, PresObjKind eObjKind, bool bVertical, const basegfx::B2DRange& rRange, bool bInit )
{
    ::svl::IUndoManager* pUndoManager = static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetUndoManager();
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

    if (!pObj && bInit)
    {
        pObj = CreatePresObj(eObjKind, bVertical, rRange);
    }
    else if ( pObj && (findConnectionToSdrObject(pObj) || bInit) )
    {
        // convert object if shape type does not match kind (f.e. converting outline text to subtitle text)
        if( bInit )
        {
            // CAUTION! This call may change eObjKind which is given by reference
            pObj = convertPresentationObjectImpl( *this, pObj, eObjKind, bVertical, rRange );
        }

        if( bUndo )
        {
            pUndoManager->AddUndoAction( getSdrModelFromSdrPage().GetSdrUndoFactory().CreateUndoGeoObject( *pObj ) );
            pUndoManager->AddUndoAction( getSdrModelFromSdrPage().GetSdrUndoFactory().CreateUndoAttrObject( *pObj, true, true ) );
            pUndoManager->AddUndoAction( new UndoConnectionToSdrObject( *pObj ) );
        }

        pObj->AdjustToMaxRange( rRange );

        establishConnectionToSdrObject(pObj, this);

        SdrTextObj* pTextObject = dynamic_cast< SdrTextObj* >(pObj);
        if( pTextObject )
        {
            if( pTextObject->IsVerticalWriting() != (bVertical ? sal_True : sal_False) )
            {
                pTextObject->SetVerticalWriting( bVertical );

                // #94826# here make sure the correct anchoring is used when the object
                // is re-used but orientation is changed
                if(PRESOBJ_OUTLINE == eObjKind)
                    pTextObject->SetMergedItem(SdrTextHorzAdjustItem( bVertical ? SDRTEXTHORZADJUST_RIGHT : SDRTEXTHORZADJUST_BLOCK ));
            }

            if( !mbMaster && (pTextObject->GetObjIdentifier() != OBJ_TABLE) )
            {
                if ( pTextObject->IsAutoGrowHeight() )
                {
                    // switch off AutoGrowHeight, set new MinHeight
                    SfxItemSet aTempAttr(pTextObject->GetObjectItemPool());
                    SdrMetricItem aMinHeight(SDRATTR_TEXT_MINFRAMEHEIGHT, basegfx::fround(rRange.getHeight()) );
                    aTempAttr.Put( aMinHeight );
                    aTempAttr.Put( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, false) );
                    pTextObject->SetMergedItemSet(aTempAttr);
                    sdr::legacy::SetLogicRange(*pTextObject, rRange);

                    // switch on AutoGrowHeight
                    SfxItemSet aAttr(pTextObject->GetObjectItemPool());
                    aAttr.Put( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, true) );

                    pTextObject->SetMergedItemSet(aAttr);
                }

                if ( pTextObject->IsAutoGrowWidth() )
                {
                    // switch off AutoGrowWidth , set new MinWidth
                    SfxItemSet aTempAttr(pTextObject->GetObjectItemPool());
                    SdrMetricItem aMinWidth(SDRATTR_TEXT_MINFRAMEWIDTH, basegfx::fround(rRange.getWidth()));
                    aTempAttr.Put( aMinWidth );
                    aTempAttr.Put( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, false) );
                    pTextObject->SetMergedItemSet(aTempAttr);
                    sdr::legacy::SetLogicRange(*pTextObject, rRange);

                    // switch on AutoGrowWidth
                    SfxItemSet aAttr(pTextObject->GetObjectItemPool());
                    aAttr.Put( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, true) );
                    pTextObject->SetMergedItemSet(aAttr);
                }
            }
        }
    }

    if(pObj && bInit )
    {
        if( !IsPresObj( pObj ) )
        {
            if( bUndo )
                pUndoManager->AddUndoAction( new UndoObjectPresentationKind( *pObj ) );

            InsertPresObj( pObj, eObjKind );
        }

        // make adjustments for vertical title and outline shapes
        if( bVertical && (( eObjKind == PRESOBJ_TITLE) || (eObjKind == PRESOBJ_OUTLINE)))
        {
            SfxItemSet aNewSet(pObj->GetMergedItemSet());
            aNewSet.Put( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, true) );
            aNewSet.Put( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, false) );
            if( eObjKind == PRESOBJ_OUTLINE )
            {
                aNewSet.Put( SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP) );
                aNewSet.Put( SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT) );
            }
            pObj->SetMergedItemSet(aNewSet);
        }
    }

    if(pObj && (pObj->IsEmptyPresObj() || !dynamic_cast< SdrGrafObj* >(pObj)))
    {
        if(bInit || findConnectionToSdrObject(pObj))
        {
            pObj->AdjustToMaxRange( rRange );
        }
    }

    return pObj;
}


/*************************************************************************
|*
|* Liefert den PresObjKind eines Objektes zurueck
|*
\************************************************************************/

PresObjKind SdPage::GetPresObjKind(const SdrObject* pObj) const
{
    PresObjKind eKind = PRESOBJ_NONE;

    if( pObj && maPresentationShapeList.hasShape(*pObj) )
    {
        SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(*pObj);
        if( pInfo )
            eKind = pInfo->mePresObjKind;
    }

    return eKind;
}

bool SdPage::IsPresObj(const SdrObject* pObj)
{
    return pObj && maPresentationShapeList.hasShape( const_cast<SdrObject&>(*pObj) );
}

void SdPage::RemovePresObj(const SdrObject* pObj)
{
    if( pObj && maPresentationShapeList.hasShape(const_cast<SdrObject&>(*pObj)) )
    {
        SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(const_cast<SdrObject&>(*pObj));
        if( pInfo )
            pInfo->mePresObjKind = PRESOBJ_NONE;
        maPresentationShapeList.removeShape(const_cast<SdrObject&>(*pObj));
    }
}

void SdPage::InsertPresObj(SdrObject* pObj, PresObjKind eKind )
{
    DBG_ASSERT( pObj, "sd::SdPage::InsertPresObj(), invalid presentation object inserted!" );
    DBG_ASSERT( !IsPresObj(pObj), "sd::SdPage::InsertPresObj(), presentation object inserted twice!" );
    if( pObj )
    {
        SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(*pObj, true);
        if( pInfo )
            pInfo->mePresObjKind = eKind;
        maPresentationShapeList.addShape(*pObj);
    }
}

/*************************************************************************
|*
|* Text des Objektes setzen
|*
\************************************************************************/

void SdPage::SetObjText(SdrTextObj* pObj, SdrOutliner* pOutliner, PresObjKind eObjKind, const String& rString )
{
    if ( pObj )
    {
        DBG_ASSERT( dynamic_cast< SdrTextObj* >(pObj), "SetObjText: Kein SdrTextObj!" );
        ::Outliner* pOutl = pOutliner;

        if (!pOutliner)
        {
            SdDrawDocument& rSdDrawDocument = static_cast< SdDrawDocument& >(getSdrModelFromSdrPage());
            SfxItemPool* pPool = rSdDrawDocument.GetDrawOutliner().GetEmptyItemSet().GetPool();
            pOutl = new ::Outliner( pPool, OUTLINERMODE_OUTLINEOBJECT );
            pOutl->SetRefDevice( SD_MOD()->GetRefDevice( *rSdDrawDocument.GetDocSh() ) );
            pOutl->SetEditTextObjectPool(pPool);
            pOutl->SetStyleSheetPool(dynamic_cast< SfxStyleSheetPool* >(rSdDrawDocument.GetStyleSheetPool()));
            pOutl->EnableUndo(false);
            pOutl->SetUpdateMode( false );
        }

        sal_uInt16 nOutlMode = pOutl->GetMode();
        Size aPaperSize = pOutl->GetPaperSize();
        bool bUpdateMode = pOutl->GetUpdateMode();
        pOutl->SetUpdateMode(false);
        pOutl->SetParaAttribs( 0, pOutl->GetEmptyItemSet() );

        // #95114# Always set the object's StyleSheet at the Outliner to
        // use the current objects StyleSheet. Thus it's the same as in
        // SetText(...).
        // #95114# Moved this implementation from where SetObjText(...) was called
        // to inside this method to work even when outliner is fetched here.
        pOutl->SetStyleSheet(0, pObj->GetStyleSheet());

        String aString;

        switch( eObjKind )
        {
            case PRESOBJ_OUTLINE:
            {
                pOutl->Init( OUTLINERMODE_OUTLINEOBJECT );

                aString += sal_Unicode( '\t' );
                aString += rString;

                if (mbMaster)
                {
                    pOutl->SetStyleSheet( 0, GetStyleSheetForPresObj(eObjKind) );
                    aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t" ));
                    aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER2 ) );

                    aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t\t" ));
                    aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER3 ) );

                    aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t\t\t" ));
                    aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER4 ) );

                    aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t\t\t\t" ));
                    aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER5 ) );

                    aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t\t\t\t\t" ));
                    aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER6 ) );

                    aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t\t\t\t\t\t" ));
                    aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER7 ) );

                    aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t\t\t\t\t\t\t" ));
                    aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER8 ) );

                    aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t\t\t\t\t\t\t\t" ));
                    aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER9 ) );
                }
            }
            break;

            case PRESOBJ_TITLE:
            {
                pOutl->Init( OUTLINERMODE_TITLEOBJECT );
                aString += rString;
            }
            break;

            default:
            {
                pOutl->Init( OUTLINERMODE_TEXTOBJECT );
                aString += rString;

                // check if we need to add a text field
                SvxFieldData* pData = NULL;

                switch( eObjKind )
                {
                case PRESOBJ_HEADER:
                    pData = new SvxHeaderField();
                    break;
                case PRESOBJ_FOOTER:
                    pData = new SvxFooterField();
                    break;
                case PRESOBJ_SLIDENUMBER:
                    pData = new SvxPageField();
                    break;
                case PRESOBJ_DATETIME:
                    pData = new SvxDateTimeField();
                    break;
                default:
                    break;
                }

                if( pData )
                {
                    ESelection e;
                    SvxFieldItem aField( *pData, EE_FEATURE_FIELD );
                    pOutl->QuickInsertField(aField,e);
                    delete pData;
                }
            }
            break;
        }

        pOutl->SetPaperSize( sdr::legacy::GetLogicRect(*pObj).GetSize() );

        if( aString.Len() )
            pOutl->SetText( aString, pOutl->GetParagraph( 0 ) );

        ( (SdrTextObj*) pObj)->SetOutlinerParaObject( pOutl->CreateParaObject() );

        if (!pOutliner)
        {
            delete pOutl;
            pOutl = NULL;
        }
        else
        {
            // Outliner restaurieren
            pOutl->Init( nOutlMode );
            pOutl->SetParaAttribs( 0, pOutl->GetEmptyItemSet() );
            pOutl->SetUpdateMode( bUpdateMode );
            pOutl->SetPaperSize( aPaperSize );
        }
    }
}

/*************************************************************************
|*
|* Link & Daten von einem VControl empfangen
|*
\************************************************************************/

void SdPage::SetLinkData(const String&, const String& )
{
}

/*************************************************************************
|*
|* Layoutname setzen
|*
\************************************************************************/
void SdPage::SetLayoutName(String aName)
{
    maLayoutName = aName;

    if( mbMaster )
    {
        String aSep( RTL_CONSTASCII_USTRINGPARAM(SD_LT_SEPARATOR) );
        sal_uInt16 nPos = maLayoutName.Search( aSep );
        if ( nPos != STRING_NOTFOUND )
        {
            FmFormPage::SetName(maLayoutName.Copy(0, nPos));
        }
    }
}


/*************************************************************************
|*
|* Seitenname zurueckgeben und ggf. generieren
|*
\************************************************************************/

const String& SdPage::GetName() const
{
    String aCreatedPageName( maCreatedPageName );
    if (GetRealName().Len() == 0)
    {
        if ((mePageKind == PK_STANDARD || mePageKind == PK_NOTES) && !mbMaster)
        {
            // default name for handout pages
            sal_uInt32 nNum = (GetPageNumber() + 1) / 2;

            aCreatedPageName = String(SdResId(STR_PAGE));
            aCreatedPageName += sal_Unicode( ' ' );

            if( SVX_NUMBER_NONE == getSdrModelFromSdrPage().GetPageNumType() )
            {
                // if the document has number none as a formating
                // for page numbers we still default to arabic numbering
                // to keep the default page names unique
                aCreatedPageName += String::CreateFromInt32( (sal_Int32)nNum );
            }
            else
            {
                aCreatedPageName += static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).CreatePageNumValue(nNum);
            }
        }
        else
        {
            /******************************************************************
            * Defaultname fuer Handzettelseiten
            ******************************************************************/
            aCreatedPageName = String(SdResId(STR_LAYOUT_DEFAULT_NAME));
        }
    }
    else
    {
        aCreatedPageName = GetRealName();
    }

    if (mePageKind == PK_NOTES)
    {
        aCreatedPageName += sal_Unicode( ' ' );
        aCreatedPageName += String(SdResId(STR_NOTES));
    }
    else if (mePageKind == PK_HANDOUT && mbMaster)
    {
        aCreatedPageName += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( " (" ));
        aCreatedPageName += String(SdResId(STR_HANDOUT));
        aCreatedPageName += sal_Unicode( ')' );
    }

    const_cast< SdPage* >(this)->maCreatedPageName = aCreatedPageName;
    return maCreatedPageName;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdPage::SetOrientation( Orientation eOrient)
{
    meOrientation = eOrient;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

Orientation SdPage::GetOrientation() const
{
    return meOrientation;
}

/*************************************************************************
|*
|* Liefert den Default-Text eines PresObjektes zurueck
|*
\************************************************************************/

String SdPage::GetPresObjText(PresObjKind eObjKind) const
{
    String aString;

    if (eObjKind == PRESOBJ_TITLE)
    {
        if (mbMaster)
        {
            if (mePageKind != PK_NOTES)
            {
                aString = String ( SdResId( STR_PRESOBJ_MPTITLE ) );
            }
            else
            {
                aString = String ( SdResId( STR_PRESOBJ_MPNOTESTITLE ) );
            }
        }
        else
        {
            aString = String ( SdResId( STR_PRESOBJ_TITLE ) );
        }
    }
    else if (eObjKind == PRESOBJ_OUTLINE)
    {
        if (mbMaster)
        {
            aString = String ( SdResId( STR_PRESOBJ_MPOUTLINE ) );
        }
        else
        {
            aString = String ( SdResId( STR_PRESOBJ_OUTLINE ) );
        }
    }
    else if (eObjKind == PRESOBJ_NOTES)
    {
        if (mbMaster)
        {
            aString = String ( SdResId( STR_PRESOBJ_MPNOTESTEXT ) );
        }
        else
        {
            aString = String ( SdResId( STR_PRESOBJ_NOTESTEXT ) );
        }
    }
    else if (eObjKind == PRESOBJ_TEXT)
    {
        aString = String ( SdResId( STR_PRESOBJ_TEXT ) );
    }
    else if (eObjKind == PRESOBJ_GRAPHIC)
    {
        aString = String ( SdResId( STR_PRESOBJ_GRAPHIC ) );
    }
    else if (eObjKind == PRESOBJ_OBJECT)
    {
        aString = String ( SdResId( STR_PRESOBJ_OBJECT ) );
    }
    else if (eObjKind == PRESOBJ_CHART)
    {
        aString = String ( SdResId( STR_PRESOBJ_CHART ) );
    }
    else if (eObjKind == PRESOBJ_ORGCHART)
    {
        aString = String ( SdResId( STR_PRESOBJ_ORGCHART ) );
    }
    else if (eObjKind == PRESOBJ_CALC)
    {
        aString = String ( SdResId( STR_PRESOBJ_TABLE ) );
    }

    return(aString);
}

extern uno::Reference< uno::XInterface > createUnoPageImpl( SdPage* pPage );

uno::Reference< uno::XInterface > SdPage::createUnoPage()
{
    return createUnoPageImpl( this );
}

/** returns the SdPage implementation for the given XDrawPage or 0 if not available */
SdPage* SdPage::getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage )
{
    try
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUnoTunnel( xPage, ::com::sun::star::uno::UNO_QUERY );
        if( xUnoTunnel.is() )
        {
            SvxDrawPage* pUnoPage = reinterpret_cast<SvxDrawPage*>(sal::static_int_cast<sal_uIntPtr>(xUnoTunnel->getSomething( SvxDrawPage::getUnoTunnelId()) ) );
            if( pUnoPage )
                return static_cast< SdPage* >( pUnoPage->GetSdrPage() );
        }
    }
    catch( ::com::sun::star::uno::Exception& e )
    {
        (void)e;
        DBG_ERROR("sd::SdPage::getImplementation(), exception cathced!" );
    }

    return 0;
}

void SdPage::SetName (const String& rName)
{
    String aOldName = GetName();
    FmFormPage::SetName (rName);
    static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).UpdatePageRelativeURLs(aOldName, rName);
    ActionChanged();
}

const HeaderFooterSettings& SdPage::getHeaderFooterSettings() const
{
    if( mePageKind == PK_HANDOUT && !mbMaster )
    {
        return (((SdPage&)TRG_GetMasterPage()).maHeaderFooterSettings);
    }
    else
    {
        return maHeaderFooterSettings;
    }
}

void SdPage::setHeaderFooterSettings( const sd::HeaderFooterSettings& rNewSettings )
{
    if( mePageKind == PK_HANDOUT && !mbMaster )
    {
        (((SdPage&)TRG_GetMasterPage()).maHeaderFooterSettings) = rNewSettings;
    }
    else
    {
        maHeaderFooterSettings = rNewSettings;
    }

    SetChanged();

    if(TRG_HasMasterPage())
    {
        TRG_GetMasterPageDescriptorViewContact().ActionChanged();

        // #119056# For HeaderFooterSettings SdrObjects are used, but the properties
        // used are not part of their model data, but kept in SD. This data is applied
        // using a 'backdoor' on primitive creation. Thus, the normal mechanism to detect
        // object changes does not work here. It is necessary to trigger updates here
        // directly. BroadcastObjectChange used for PagePreview invalidations,
        // flushViewObjectContacts used to invalidate and flush all visualizations in
        // edit views.
        SdPage* pMasterPage = dynamic_cast< SdPage* >(&TRG_GetMasterPage());

        if(pMasterPage)
        {
            SdrObject* pCandidate = 0;

            pCandidate = pMasterPage->GetPresObj( PRESOBJ_HEADER );

            if(pCandidate)
            {
                const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pCandidate);
                pCandidate->GetViewContact().flushViewObjectContacts();
            }

            pCandidate = pMasterPage->GetPresObj( PRESOBJ_DATETIME );

            if(pCandidate)
            {
                const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pCandidate);
                pCandidate->GetViewContact().flushViewObjectContacts();
            }

            pCandidate = pMasterPage->GetPresObj( PRESOBJ_FOOTER );

            if(pCandidate)
            {
                const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pCandidate);
                pCandidate->GetViewContact().flushViewObjectContacts();
            }

            pCandidate = pMasterPage->GetPresObj( PRESOBJ_SLIDENUMBER );

            if(pCandidate)
            {
                const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pCandidate);
                pCandidate->GetViewContact().flushViewObjectContacts();
            }
        }
    }
}

bool SdPage::checkVisibility(
    const sdr::contact::ViewObjectContact& rOriginal,
    const sdr::contact::DisplayInfo& rDisplayInfo,
    bool bEdit )
{
    if( !FmFormPage::checkVisibility( rOriginal, rDisplayInfo, bEdit ) )
        return false;

    SdrObject* pObj = rOriginal.GetViewContact().TryToGetSdrObject();
    if( pObj == NULL )
        return false;

    const SdrPage* pVisualizedPage = GetSdrPageFromXDrawPage(rOriginal.GetObjectContact().getViewInformation2D().getVisualizedPage());
    const bool bIsPrinting(rOriginal.GetObjectContact().isOutputToPrinter() || rOriginal.GetObjectContact().isOutputToPDFFile());
    const SdrView* pSdrView = rOriginal.GetObjectContact().TryToGetSdrView();
    const bool bIsInsidePageObj(pSdrView && pSdrView->GetSdrPageView() && &pSdrView->GetSdrPageView()->getSdrPageFromSdrPageView() != pVisualizedPage);

    // empty presentation objects only visible during edit mode
    if( (bIsPrinting || !bEdit || bIsInsidePageObj ) && pObj->IsEmptyPresObj() )
    {
        if( (pObj->GetObjInventor() != SdrInventor) || ( (pObj->GetObjIdentifier() != OBJ_RECT) && (pObj->GetObjIdentifier() != OBJ_PAGE) ) )
            return false;
    }

    if( ( pObj->GetObjInventor() == SdrInventor ) && ( pObj->GetObjIdentifier() == OBJ_TEXT ) )
    {
           const SdPage* pCheckPage = dynamic_cast< const SdPage* >(pObj->getSdrPageFromSdrObject());

        if( pCheckPage )
        {
            PresObjKind eKind = pCheckPage->GetPresObjKind(pObj);

            if((eKind == PRESOBJ_FOOTER) || (eKind == PRESOBJ_HEADER) || (eKind == PRESOBJ_DATETIME) || (eKind == PRESOBJ_SLIDENUMBER) )
            {
                const bool bSubContentProcessing(rDisplayInfo.GetSubContentActive());

                if( bSubContentProcessing || ( pCheckPage->GetPageKind() == PK_HANDOUT && bIsPrinting ) )
                {
                    // use the page that is currently processed
                    const SdPage* pVisualizedSdPage = dynamic_cast< const SdPage* >(pVisualizedPage);

                    if( pVisualizedSdPage )
                    {
                        // if we are not on a masterpage, see if we have to draw this header&footer object at all
                        const sd::HeaderFooterSettings& rSettings = pVisualizedSdPage->getHeaderFooterSettings();

                        switch( eKind )
                        {
                        case PRESOBJ_FOOTER:
                            return rSettings.mbFooterVisible;
                        case PRESOBJ_HEADER:
                            return rSettings.mbHeaderVisible;
                        case PRESOBJ_DATETIME:
                            return rSettings.mbDateTimeVisible;
                        case PRESOBJ_SLIDENUMBER:
                            return rSettings.mbSlideNumberVisible;
                        default:
                            break;
                        }
                    }
                }
            } // check for placeholders on master
            else if( (eKind != PRESOBJ_NONE) && pCheckPage->IsMasterPage() && ( pVisualizedPage != pCheckPage ) )
            {
                // presentation objects on master slide are always invisible if slide is shown.
                return false;
            }
        }
    }

    // i63977, do not print SdrpageObjs from master pages
    if( ( pObj->GetObjInventor() == SdrInventor ) && ( pObj->GetObjIdentifier() == OBJ_PAGE ) )
    {
        if( pObj->getSdrPageFromSdrObject() && pObj->getSdrPageFromSdrObject()->IsMasterPage() )
            return false;
    }

    return true;
}

bool SdPage::RestoreDefaultText( SdrObject* pObj )
{
    bool bRet = false;

    SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );

    if( pTextObj )
    {
        PresObjKind ePresObjKind = GetPresObjKind(pTextObj);

        if (ePresObjKind == PRESOBJ_TITLE   ||
            ePresObjKind == PRESOBJ_OUTLINE ||
            ePresObjKind == PRESOBJ_NOTES   ||
            ePresObjKind == PRESOBJ_TEXT)
        {
            String aString( GetPresObjText(ePresObjKind) );

            if (aString.Len())
            {
                bool bVertical = false;
                OutlinerParaObject* pOldPara = pTextObj->GetOutlinerParaObject();
                if( pOldPara )
                    bVertical = pOldPara->IsVertical();  // is old para object vertical?

                SetObjText( pTextObj, 0, ePresObjKind, aString );

                if( pOldPara )
                {
                    //pTextObj->SetVerticalWriting( bVertical );
                    //
                    // #94826# Here, only the vertical flag for the
                    // OutlinerParaObjects needs to be changed. The
                    // AutoGrowWidth/Height items still exist in the
                    // not changed object.
                    if(pTextObj
                        && pTextObj->GetOutlinerParaObject()
                        && pTextObj->GetOutlinerParaObject()->IsVertical() != (bool)bVertical)
                    {
                        const Rectangle aObjectRect(sdr::legacy::GetSnapRect(*pTextObj));
                        pTextObj->GetOutlinerParaObject()->SetVertical(bVertical);
                        sdr::legacy::SetSnapRect(*pTextObj, aObjectRect);
                    }
                }

                pTextObj->SetTextEditOutliner( NULL );  // to make stylesheet settings work
                pTextObj->SetStyleSheet( GetStyleSheetForPresObj(ePresObjKind), true );
                pTextObj->SetEmptyPresObj(true);
                bRet = true;
            }
        }
    }
    return bRet;
}

void SdPage::CalculateHandoutAreas( SdDrawDocument& rModel, AutoLayout eLayout, bool bHorizontal, std::vector< basegfx::B2DRange >& rAreas )
{
    SdPage& rHandoutMaster = *rModel.GetMasterSdPage( 0, PK_HANDOUT );

    if(AUTOLAYOUT_NONE == eLayout)
    {
        // use layout from handout master
        SdrObjListIter aShapeIter (rHandoutMaster);

        while (aShapeIter.IsMore())
        {
            SdrPageObj* pPageObj = dynamic_cast<SdrPageObj*>(aShapeIter.Next());

            if (pPageObj)
            {
                rAreas.push_back(pPageObj->getObjectRange(0));
            }
        }
    }
    else
    {
        const double fGapW(1000.0); // gap is 1cm
        const double fGapH(1000.0);
        const double fLeftBorder(rHandoutMaster.GetLeftPageBorder());
        const double fRightBorder(rHandoutMaster.GetRightPageBorder());
        double fTopBorder(rHandoutMaster.GetTopPageBorder());
        double fBottomBorder(rHandoutMaster.GetBottomPageBorder());
        basegfx::B2DVector aArea(rHandoutMaster.GetPageScale());
        const double fHeaderFooterHeight((aArea.getY() - fTopBorder - fLeftBorder) * 0.05);

        fTopBorder += fHeaderFooterHeight;
        fBottomBorder += fHeaderFooterHeight;

        double fX(fGapW + fLeftBorder);
        double fY(fGapH + fTopBorder);

        aArea.setX(aArea.getX() - (fGapW * 2.0 + fLeftBorder + fRightBorder));
        aArea.setY(aArea.getY() - (fGapH * 2.0 + fTopBorder + fBottomBorder));

        const bool bLandscape(aArea.getX() > aArea.getY());

        static sal_uInt16 aOffsets[5][9] =
        {
            { 0, 1, 2, 3, 4, 5, 6, 7, 8 }, // AUTOLAYOUT_HANDOUT9, Portrait, Horizontal order
            { 0, 2, 4, 1, 3, 5, 0, 0, 0 }, // AUTOLAYOUT_HANDOUT3, Landscape, Vertical
            { 0, 2, 1, 3, 0, 0, 0, 0, 0 }, // AUTOLAYOUT_HANDOUT4, Landscape, Vertical
            { 0, 3, 1, 4, 2, 5, 0, 0, 0 }, // AUTOLAYOUT_HANDOUT4, Portrait, Vertical
            { 0, 3, 6, 1, 4, 7, 2, 5, 8 }, // AUTOLAYOUT_HANDOUT9, Landscape, Vertical
        };

        sal_uInt16* pOffsets = aOffsets[0];
        sal_uInt16 nColCnt(0);
        sal_uInt16 nRowCnt(0);

        switch ( eLayout )
        {
            case AUTOLAYOUT_HANDOUT1:
                nColCnt = 1; nRowCnt = 1;
                break;

            case AUTOLAYOUT_HANDOUT2:
                if( bLandscape )
                {
                    nColCnt = 2; nRowCnt = 1;
                }
                else
                {
                    nColCnt = 1; nRowCnt = 2;
                }
                break;

            case AUTOLAYOUT_HANDOUT3:
                if( bLandscape )
                {
                    nColCnt = 3; nRowCnt = 2;
                }
                else
                {
                    nColCnt = 2; nRowCnt = 3;
                }
                pOffsets = aOffsets[ bLandscape ? 1 : 0 ];
                break;

            case AUTOLAYOUT_HANDOUT4:
                nColCnt = 2; nRowCnt = 2;
                pOffsets = aOffsets[ bHorizontal ? 0 : 2 ];
                break;

            case AUTOLAYOUT_HANDOUT6:
                if( bLandscape )
                {
                    nColCnt = 3; nRowCnt = 2;
                }
                else
                {
                    nColCnt = 2; nRowCnt = 3;
                }
                if( !bHorizontal )
                    pOffsets = aOffsets[ bLandscape ? 1 : 3 ];
                break;

            default:
            case AUTOLAYOUT_HANDOUT9:
                nColCnt = 3; nRowCnt = 3;

                if( !bHorizontal )
                    pOffsets = aOffsets[4];
                break;
        }

        const basegfx::B2DVector aPartArea(
            (aArea.getX()  - ((nColCnt - 1) * fGapW) ) / nColCnt,
            (aArea.getY() - ((nRowCnt - 1) * fGapH) ) / nRowCnt);
        basegfx::B2DVector aSize(0.0, 0.0);
        SdrPage* pFirstPage = rModel.GetMasterSdPage(0, PK_STANDARD);

        if ( pFirstPage )
        {
            // scale actual size into handout rect
            double fScale = aPartArea.getX() / pFirstPage->GetPageScale().getX();

            aSize.setY(fScale * pFirstPage->GetPageScale().getY());

            if( aSize.getY() > aPartArea.getY() )
            {
                fScale = aPartArea.getY() / pFirstPage->GetPageScale().getY();
                aSize.setY(aPartArea.getY());
                aSize.setX(fScale * pFirstPage->GetPageScale().getX());
            }
            else
            {
                aSize.setX(aPartArea.getX());
            }

            fX += (aPartArea.getX() - aSize.getX()) * 0.5;
            fY += (aPartArea.getY()- aSize.getY()) * 0.5;
        }
        else
        {
            aSize = aPartArea;
        }

        basegfx::B2DPoint aPos( fX, fY );
        const bool bRTL(rModel.GetDefaultWritingMode() == ::com::sun::star::text::WritingMode_RL_TB);
        const double fOffsetX((aPartArea.getX() + fGapW) * (bRTL ? -1.0 : 1.0));
        const double fOffsetY(aPartArea.getY() + fGapH);
        const double fStartX(bRTL ? fOffsetX * (1.0 - nColCnt) - fX : fX);

        rAreas.resize( nColCnt * nRowCnt );

        for(sal_uInt16 nRow = 0; nRow < nRowCnt; nRow++)
        {
            aPos.setX(fStartX);

            for(sal_uInt16 nCol = 0; nCol < nColCnt; nCol++)
            {
                rAreas[*pOffsets++] = basegfx::B2DRange(aPos, aPos + aSize);
                aPos.setX(aPos.getX() + fOffsetX);
            }

            aPos.setY(aPos.getY() + fOffsetY);
        }
    }
}




void SdPage::SetPrecious (const bool bIsPrecious)
{
    mbIsPrecious = bIsPrecious;
}




bool SdPage::IsPrecious (void) const
{
    return mbIsPrecious;
}



void SdPage::SetTime(sal_uInt32 nNewTime)
{
    mnTime = nNewTime;
}

void SdPage::SetPresChange(PresChange eChange)
{
    mePresChange = eChange;
}

AutoLayout SdPage::GetAutoLayout() const
{
    return meAutoLayout;
}

PageKind SdPage::GetPageKind() const
{
    return mePageKind;
}



HeaderFooterSettings::HeaderFooterSettings()
{
    mbHeaderVisible = true;
    mbFooterVisible = true;
    mbSlideNumberVisible = false;
    mbDateTimeVisible = true;
    mbDateTimeIsFixed = true;
    meDateTimeFormat = SVXDATEFORMAT_A;
}


HeaderFooterSettings::HeaderFooterSettings(const HeaderFooterSettings& rSource)
{
    *this = rSource;
}

HeaderFooterSettings::~HeaderFooterSettings()
{
}

bool HeaderFooterSettings::operator==( const HeaderFooterSettings& rSettings ) const
{
    return (mbHeaderVisible == rSettings.mbHeaderVisible) &&
           (maHeaderText == rSettings.maHeaderText) &&
           (mbFooterVisible == rSettings.mbFooterVisible) &&
           (maFooterText == rSettings.maFooterText) &&
           (mbSlideNumberVisible == rSettings.mbSlideNumberVisible) &&
           (mbDateTimeVisible == rSettings.mbDateTimeVisible) &&
           (mbDateTimeIsFixed == rSettings.mbDateTimeIsFixed) &&
           (meDateTimeFormat == rSettings.meDateTimeFormat) &&
           (maDateTimeText == rSettings.maDateTimeText);
}

HeaderFooterSettings& HeaderFooterSettings::operator=(const HeaderFooterSettings& rHeaderFooter)
{
    mbHeaderVisible = rHeaderFooter.mbHeaderVisible;
    maHeaderText = rHeaderFooter.maHeaderText;
    mbFooterVisible = rHeaderFooter.mbFooterVisible;
    maFooterText = rHeaderFooter.maFooterText;
    mbSlideNumberVisible = rHeaderFooter.mbSlideNumberVisible;
    mbDateTimeVisible = rHeaderFooter.mbDateTimeVisible;
    mbDateTimeIsFixed = rHeaderFooter.mbDateTimeIsFixed;
    maDateTimeText = rHeaderFooter.maDateTimeText;
    meDateTimeFormat = rHeaderFooter.meDateTimeFormat;

    return *this;
}

// eof
