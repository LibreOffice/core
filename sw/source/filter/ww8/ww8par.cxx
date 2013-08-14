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


#include <com/sun/star/embed/ElementModes.hpp>

#include <i18nlangtag/languagetag.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <rtl/random.h>

#include <sfx2/docinf.hxx>
#include <sfx2/request.hxx>
#include <sfx2/frame.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>

#include <comphelper/docpasswordrequest.hxx>
#include <comphelper/string.hxx>

#include <editeng/brushitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/fontitem.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svxerr.hxx>
#include <filter/msfilter/mscodec.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xflclit.hxx>

#include <unotools/fltrcfg.hxx>
#include <fmtfld.hxx>
#include <fmturl.hxx>
#include <fmtinfmt.hxx>
#include <reffld.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <fmtcnct.hxx>
#include <fmtanchr.hxx>
#include <fmtpdsc.hxx>
#include <ftninfo.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <ndtxt.hxx>            // class SwTxtNode
#include <pagedesc.hxx>         // class SwPageDesc
#include <paratr.hxx>
#include <fmtclbl.hxx>
#include <section.hxx>
#include <docsh.hxx>
#include <docufld.hxx>
#include <swfltopt.hxx>
#include <viewsh.hxx>
#include <shellres.hxx>
#include <mdiexp.hxx>           // Progress
#include <statstr.hrc>          // ResId for Statusbar
#include <swerror.h>            // ERR_WW8_...
#include <swtable.hxx>          // class SwTableLines, ...
#include <fchrfmt.hxx>
#include <charfmt.hxx>


#include <comphelper/extract.hxx>
#include <fltini.hxx>

#include "writerwordglue.hxx"

#include "ndgrf.hxx"
#include <editeng/editids.hrc>
#include <txtflcnt.hxx>
#include <fmtflcnt.hxx>
#include <txatbase.hxx>

#include "ww8par2.hxx"          // class WW8RStyle, class WW8AnchorPara

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <svl/itemiter.hxx>     //SfxItemIter

#include <comphelper/processfactory.hxx>
#include <basic/basmgr.hxx>

#include "ww8toolbar.hxx"
#include <osl/file.hxx>

#include <breakit.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <iostream>
#include <dbgoutsw.hxx>
#endif
#include <unotools/localfilehelper.hxx>

#include "WW8Sttbf.hxx"
#include "WW8FibData.hxx"

using namespace ::com::sun::star;
using namespace sw::util;
using namespace sw::types;
using namespace nsHdFtFlags;

#include <com/sun/star/i18n/ScriptType.hpp>
#include <unotools/pathoptions.hxx>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>

#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <comphelper/mediadescriptor.hxx>
#include <oox/ole/vbaproject.hxx>
#include <oox/ole/olestorage.hxx>

using ::comphelper::MediaDescriptor;

class BasicProjImportHelper
{
    SwDocShell& mrDocShell;
    uno::Reference< uno::XComponentContext > mxCtx;
public:
    BasicProjImportHelper( SwDocShell& rShell ) : mrDocShell( rShell )
    {
        mxCtx = comphelper::getProcessComponentContext();
    }
    bool import( const uno::Reference< io::XInputStream >& rxIn );
    OUString getProjectName();
};

bool BasicProjImportHelper::import( const uno::Reference< io::XInputStream >& rxIn )
{
    bool bRet = false;
    try
    {
        oox::ole::OleStorage root( mxCtx, rxIn, false );
        oox::StorageRef vbaStg = root.openSubStorage( "Macros" , false );
        if ( vbaStg.get() )
        {
            oox::ole::VbaProject aVbaPrj( mxCtx, mrDocShell.GetModel(), OUString("Writer") );
            bRet = aVbaPrj.importVbaProject( *vbaStg );
        }
    }
    catch( const uno::Exception& )
    {
        bRet = false;
    }
    return bRet;
}

OUString BasicProjImportHelper::getProjectName()
{
    OUString sProjName( "Standard" );
    uno::Reference< beans::XPropertySet > xProps( mrDocShell.GetModel(), uno::UNO_QUERY );
    if ( xProps.is() )
    {
        try
        {
            uno::Reference< script::vba::XVBACompatibility > xVBA( xProps->getPropertyValue( "BasicLibraries" ), uno::UNO_QUERY_THROW  );
            sProjName = xVBA->getProjectName();

        }
        catch( const uno::Exception& )
        {
        }
    }
    return sProjName;
}


class Sttb : TBBase
{
struct SBBItem
{
    sal_uInt16 cchData;
    OUString data;
    SBBItem() : cchData(0){}
};
    sal_uInt16 fExtend;
    sal_uInt16 cData;
    sal_uInt16 cbExtra;

    std::vector< SBBItem > dataItems;

    Sttb(const Sttb&);
    Sttb& operator = ( const Sttb&);
public:
    Sttb();
    ~Sttb();
    bool Read(SvStream &rS);
    void Print( FILE* fp );
    OUString getStringAtIndex( sal_uInt32 );
};

Sttb::Sttb() : fExtend( 0 )
,cData( 0 )
,cbExtra( 0 )
{
}

Sttb::~Sttb()
{
}

bool Sttb::Read( SvStream& rS )
{
    OSL_TRACE("Sttb::Read() stream pos 0x%x", rS.Tell() );
    nOffSet = rS.Tell();
    rS >> fExtend >> cData >> cbExtra;
    if ( cData )
    {
        for ( sal_Int32 index = 0; index < cData; ++index )
        {
            SBBItem aItem;
            rS >> aItem.cchData;
            aItem.data = read_uInt16s_ToOUString(rS, aItem.cchData);
            dataItems.push_back( aItem );
        }
    }
    return true;
}

void Sttb::Print( FILE* fp )
{
    fprintf( fp, "[ 0x%" SAL_PRIxUINT32 " ] Sttb - dump\n", nOffSet);
    fprintf( fp, " fExtend 0x%x [expected 0xFFFF ]\n", fExtend );
    fprintf( fp, " cData no. or string data items %d (0x%x)\n", cData, cData );

    if ( cData )
    {
        for ( sal_Int32 index = 0; index < cData; ++index )
            fprintf(fp,"   string dataItem[ %d(0x%x) ] has name %s\n", static_cast< int >( index ), static_cast< unsigned int >( index ), OUStringToOString( dataItems[ index ].data, RTL_TEXTENCODING_UTF8 ).getStr() );
    }

}

OUString
Sttb::getStringAtIndex( sal_uInt32 index )
{
    OUString aRet;
    if ( index < dataItems.size() )
        aRet = dataItems[ index ].data;
    return aRet;
}

SwMSDffManager::SwMSDffManager( SwWW8ImplReader& rRdr )
    : SvxMSDffManager(*rRdr.pTableStream, rRdr.GetBaseURL(), rRdr.pWwFib->fcDggInfo,
        rRdr.pDataStream, 0, 0, COL_WHITE, 12, rRdr.pStrm),
    rReader(rRdr), pFallbackStream(0)
{
    SetSvxMSDffSettings( GetSvxMSDffSettings() );
    nSvxMSDffOLEConvFlags = SwMSDffManager::GetFilterFlags();
}

sal_uInt32 SwMSDffManager::GetFilterFlags()
{
    sal_uInt32 nFlags(0);
    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    if (rOpt.IsMathType2Math())
        nFlags |= OLE_MATHTYPE_2_STARMATH;
    if (rOpt.IsExcel2Calc())
        nFlags |= OLE_EXCEL_2_STARCALC;
    if (rOpt.IsPowerPoint2Impress())
        nFlags |= OLE_POWERPOINT_2_STARIMPRESS;
    if (rOpt.IsWinWord2Writer())
        nFlags |= OLE_WINWORD_2_STARWRITER;
    return nFlags;
}

/*
 * I would like to override the default OLE importing to add a test
 * and conversion of OCX controls from their native OLE type into our
 * native nonOLE Form Control Objects.
 *
 * cmc
 */
// #i32596# - consider new parameter <_nCalledByGroup>
SdrObject* SwMSDffManager::ImportOLE( long nOLEId,
                                      const Graphic& rGrf,
                                      const Rectangle& rBoundRect,
                                      const Rectangle& rVisArea,
                                      const int _nCalledByGroup,
                                      sal_Int64 nAspect ) const
{
    // #i32596# - no import of OLE object, if it's inside a group.
    // NOTE: This can be undone, if grouping of Writer fly frames is possible or
    // if drawing OLE objects are allowed in Writer.
    if ( _nCalledByGroup > 0 )
    {
        return 0L;
    }

    SdrObject* pRet = 0;
    OUString sStorageName;
    SotStorageRef xSrcStg;
    uno::Reference < embed::XStorage > xDstStg;
    if( GetOLEStorageName( nOLEId, sStorageName, xSrcStg, xDstStg ))
    {
        SvStorageRef xSrc = xSrcStg->OpenSotStorage( sStorageName,
            STREAM_READWRITE| STREAM_SHARE_DENYALL );
        OSL_ENSURE(rReader.pFormImpl, "No Form Implementation!");
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape;
        if ( (!(rReader.bIsHeader || rReader.bIsFooter)) &&
            rReader.pFormImpl->ReadOCXStream(xSrc,&xShape,true))
        {
            pRet = GetSdrObjectFromXShape(xShape);
        }
        else
        {
            ErrCode nError = ERRCODE_NONE;
            pRet = CreateSdrOLEFromStorage( sStorageName, xSrcStg, xDstStg,
                rGrf, rBoundRect, rVisArea, pStData, nError, nSvxMSDffOLEConvFlags, nAspect );
        }
    }
    return pRet;
}

void SwMSDffManager::DisableFallbackStream()
{
    OSL_ENSURE(!pFallbackStream,
        "if you're recursive, you're broken");
    pFallbackStream = pStData2;
    aOldEscherBlipCache = aEscherBlipCache;
    aEscherBlipCache.clear();
    pStData2 = 0;
}

void SwMSDffManager::EnableFallbackStream()
{
    pStData2 = pFallbackStream;
    aEscherBlipCache = aOldEscherBlipCache;
    aOldEscherBlipCache.clear();
    pFallbackStream = 0;
}

sal_uInt16 SwWW8ImplReader::GetToggleAttrFlags() const
{
    return pCtrlStck ? pCtrlStck->GetToggleAttrFlags() : 0;
}

sal_uInt16 SwWW8ImplReader::GetToggleBiDiAttrFlags() const
{
    return pCtrlStck ? pCtrlStck->GetToggleBiDiAttrFlags() : 0;
}

void SwWW8ImplReader::SetToggleAttrFlags(sal_uInt16 nFlags)
{
    if (pCtrlStck)
        pCtrlStck->SetToggleAttrFlags(nFlags);
}

void SwWW8ImplReader::SetToggleBiDiAttrFlags(sal_uInt16 nFlags)
{
    if (pCtrlStck)
        pCtrlStck->SetToggleBiDiAttrFlags(nFlags);
}


SdrObject* SwMSDffManager::ProcessObj(SvStream& rSt,
                                       DffObjData& rObjData,
                                       void* pData,
                                       Rectangle& rTextRect,
                                       SdrObject* pObj
                                       )
{
    if( !rTextRect.IsEmpty() )
    {
        SvxMSDffImportData& rImportData = *(SvxMSDffImportData*)pData;
        SvxMSDffImportRec* pImpRec = new SvxMSDffImportRec;

        // fill Import Record with data
        pImpRec->nShapeId   = rObjData.nShapeId;
        pImpRec->eShapeType = rObjData.eShapeType;

        rObjData.bClientAnchor = maShapeRecords.SeekToContent( rSt,
                                            DFF_msofbtClientAnchor,
                                            SEEK_FROM_CURRENT_AND_RESTART );
        if( rObjData.bClientAnchor )
            ProcessClientAnchor( rSt,
                    maShapeRecords.Current()->nRecLen,
                    pImpRec->pClientAnchorBuffer, pImpRec->nClientAnchorLen );

        rObjData.bClientData = maShapeRecords.SeekToContent( rSt,
                                            DFF_msofbtClientData,
                                            SEEK_FROM_CURRENT_AND_RESTART );
        if( rObjData.bClientData )
            ProcessClientData( rSt,
                    maShapeRecords.Current()->nRecLen,
                    pImpRec->pClientDataBuffer, pImpRec->nClientDataLen );


        // process user (== Winword) defined parameters in 0xF122 record
        // #i84783# - set special value to determine, if property is provided or not.
        pImpRec->nLayoutInTableCell = 0xFFFFFFFF;

        if(    maShapeRecords.SeekToContent( rSt,
                                             DFF_msofbtUDefProp,
                                             SEEK_FROM_CURRENT_AND_RESTART )
            && maShapeRecords.Current()->nRecLen )
        {
            sal_uInt32  nBytesLeft = maShapeRecords.Current()->nRecLen;
            sal_uInt32  nUDData;
            sal_uInt16  nPID;
            while( 5 < nBytesLeft )
            {
                rSt >> nPID;
                if ( rSt.GetError() != 0 )
                    break;
                rSt >> nUDData;
                switch( nPID )
                {
                    case 0x038F: pImpRec->nXAlign = nUDData; break;
                    case 0x0390:
                        delete pImpRec->pXRelTo;
                        pImpRec->pXRelTo = new sal_uInt32;
                        *(pImpRec->pXRelTo) = nUDData;
                        break;
                    case 0x0391: pImpRec->nYAlign = nUDData; break;
                    case 0x0392:
                        delete pImpRec->pYRelTo;
                        pImpRec->pYRelTo = new sal_uInt32;
                        *(pImpRec->pYRelTo) = nUDData;
                        break;
                    case 0x03BF: pImpRec->nLayoutInTableCell = nUDData; break;
                    case 0x0393:
                    // This seems to correspond to o:hrpct from .docx (even including
                    // the difference that it's in 0.1% even though the .docx spec
                    // says it's in 1%).
                        pImpRec->relativeHorizontalWidth = nUDData;
                        break;
                    case 0x0394:
                    // And this is really just a guess, but a mere presence of this
                    // flag makes a horizontal rule be as wide as the page (unless
                    // overriden by something), so it probably matches o:hr from .docx.
                        pImpRec->isHorizontalRule = true;
                        break;
                }
                if ( rSt.GetError() != 0 )
                    break;
                pImpRec->bHasUDefProp = sal_True;
                nBytesLeft  -= 6;
            }
        }

        // Text Frame also Title or Outline
        sal_uInt32 nTextId = GetPropertyValue( DFF_Prop_lTxid, 0 );
        if( nTextId )
        {
            SfxItemSet aSet( pSdrModel->GetItemPool() );

            // Originally anything that as a mso_sptTextBox was created as a
            // textbox, this was changed to be created as a simple
            // rect to keep impress happy. For the rest of us we'd like to turn
            // it back into a textbox again.
            bool bIsSimpleDrawingTextBox = (pImpRec->eShapeType == mso_sptTextBox);
            if (!bIsSimpleDrawingTextBox)
            {
                // Either
                // a) its a simple text object or
                // b) its a rectangle with text and square wrapping.
                bIsSimpleDrawingTextBox =
                (
                    (pImpRec->eShapeType == mso_sptTextSimple) ||
                    (
                        (pImpRec->eShapeType == mso_sptRectangle)
                        && ShapeHasText(pImpRec->nShapeId, rObjData.rSpHd.GetRecBegFilePos() )
                    )
                );
            }

            // Distance of Textbox to it's surrounding Autoshape
            sal_Int32 nTextLeft = GetPropertyValue( DFF_Prop_dxTextLeft, 91440L);
            sal_Int32 nTextRight = GetPropertyValue( DFF_Prop_dxTextRight, 91440L );
            sal_Int32 nTextTop = GetPropertyValue( DFF_Prop_dyTextTop, 45720L  );
            sal_Int32 nTextBottom = GetPropertyValue( DFF_Prop_dyTextBottom, 45720L );

            ScaleEmu( nTextLeft );
            ScaleEmu( nTextRight );
            ScaleEmu( nTextTop );
            ScaleEmu( nTextBottom );

            sal_Int32 nTextRotationAngle=0;
            bool bVerticalText = false;
            if ( IsProperty( DFF_Prop_txflTextFlow ) )
            {
                MSO_TextFlow eTextFlow = (MSO_TextFlow)(GetPropertyValue(
                    DFF_Prop_txflTextFlow) & 0xFFFF);
                switch( eTextFlow )
                {
                    case mso_txflBtoT:
                        nTextRotationAngle = 9000;
                    break;
                    case mso_txflVertN:
                    case mso_txflTtoBN:
                        nTextRotationAngle = 27000;
                        break;
                    case mso_txflTtoBA:
                        bVerticalText = true;
                    break;
                    case mso_txflHorzA:
                        bVerticalText = true;
                        nTextRotationAngle = 9000;
                    case mso_txflHorzN:
                    default :
                        break;
                }
            }

            if (nTextRotationAngle)
            {
                while (nTextRotationAngle > 360000)
                    nTextRotationAngle-=9000;
                switch (nTextRotationAngle)
                {
                    case 9000:
                        {
                            long nWidth = rTextRect.GetWidth();
                            rTextRect.Right() = rTextRect.Left() + rTextRect.GetHeight();
                            rTextRect.Bottom() = rTextRect.Top() + nWidth;

                            sal_Int32 nOldTextLeft = nTextLeft;
                            sal_Int32 nOldTextRight = nTextRight;
                            sal_Int32 nOldTextTop = nTextTop;
                            sal_Int32 nOldTextBottom = nTextBottom;

                            nTextLeft = nOldTextBottom;
                            nTextRight = nOldTextTop;
                            nTextTop = nOldTextLeft;
                            nTextBottom = nOldTextRight;
                        }
                        break;
                    case 27000:
                        {
                            long nWidth = rTextRect.GetWidth();
                            rTextRect.Right() = rTextRect.Left() + rTextRect.GetHeight();
                            rTextRect.Bottom() = rTextRect.Top() + nWidth;

                            sal_Int32 nOldTextLeft = nTextLeft;
                            sal_Int32 nOldTextRight = nTextRight;
                            sal_Int32 nOldTextTop = nTextTop;
                            sal_Int32 nOldTextBottom = nTextBottom;

                            nTextLeft = nOldTextTop;
                            nTextRight = nOldTextBottom;
                            nTextTop = nOldTextRight;
                            nTextBottom = nOldTextLeft;
                        }
                        break;
                    default:
                        break;
                }
            }

            if (bIsSimpleDrawingTextBox)
            {
                SdrObject::Free( pObj );
                pObj = new SdrRectObj(OBJ_TEXT, rTextRect);
            }

            // The vertical paragraph justification are contained within the
            // BoundRect so calculate it here
            Rectangle aNewRect(rTextRect);
            aNewRect.Bottom() -= nTextTop + nTextBottom;
            aNewRect.Right() -= nTextLeft + nTextRight;

            // Only if its a simple Textbox, Writer can replace the Object
            // with a Frame, else
            if( bIsSimpleDrawingTextBox )
            {
                ::boost::shared_ptr<SvxMSDffShapeInfo> const pTmpRec(
                        new SvxMSDffShapeInfo(0, pImpRec->nShapeId));

                SvxMSDffShapeInfos_ById::const_iterator const it =
                    GetShapeInfos()->find(pTmpRec);
                if (it != GetShapeInfos()->end())
                {
                    SvxMSDffShapeInfo& rInfo = **it;
                    pImpRec->bReplaceByFly   = rInfo.bReplaceByFly;
                    pImpRec->bLastBoxInChain = rInfo.bLastBoxInChain;
                }
            }

            if( bIsSimpleDrawingTextBox )
                ApplyAttributes( rSt, aSet, rObjData );

            if (GetPropertyValue(DFF_Prop_FitTextToShape) & 2)
            {
                aSet.Put( SdrTextAutoGrowHeightItem( sal_True ) );
                aSet.Put( SdrTextMinFrameHeightItem(
                    aNewRect.Bottom() - aNewRect.Top() ) );
                aSet.Put( SdrTextMinFrameWidthItem(
                    aNewRect.Right() - aNewRect.Left() ) );
            }
            else
            {
                aSet.Put( SdrTextAutoGrowHeightItem( sal_False ) );
                aSet.Put( SdrTextAutoGrowWidthItem( sal_False ) );
            }

            switch ( (MSO_WrapMode)
                GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare ) )
            {
                case mso_wrapNone :
                    aSet.Put( SdrTextAutoGrowWidthItem( sal_True ) );
                    pImpRec->bAutoWidth = true;
                break;
                case mso_wrapByPoints :
                    aSet.Put( SdrTextContourFrameItem( sal_True ) );
                break;
                default:
                    ;
            }

            // Set distances on Textbox's margins
            aSet.Put( SdrTextLeftDistItem( nTextLeft ) );
            aSet.Put( SdrTextRightDistItem( nTextRight ) );
            aSet.Put( SdrTextUpperDistItem( nTextTop ) );
            aSet.Put( SdrTextLowerDistItem( nTextBottom ) );
            pImpRec->nDxTextLeft    = nTextLeft;
            pImpRec->nDyTextTop     = nTextTop;
            pImpRec->nDxTextRight   = nTextRight;
            pImpRec->nDyTextBottom  = nTextBottom;

            // Taking the correct default (which is mso_anchorTop)
            MSO_Anchor eTextAnchor =
                (MSO_Anchor)GetPropertyValue( DFF_Prop_anchorText, mso_anchorTop );

            SdrTextVertAdjust eTVA = bVerticalText
                                     ? SDRTEXTVERTADJUST_BLOCK
                                     : SDRTEXTVERTADJUST_CENTER;
            SdrTextHorzAdjust eTHA = bVerticalText
                                     ? SDRTEXTHORZADJUST_CENTER
                                     : SDRTEXTHORZADJUST_BLOCK;

            switch( eTextAnchor )
            {
                case mso_anchorTop:
                {
                    if ( bVerticalText )
                        eTHA = SDRTEXTHORZADJUST_RIGHT;
                    else
                        eTVA = SDRTEXTVERTADJUST_TOP;
                }
                break;
                case mso_anchorTopCentered:
                {
                    if ( bVerticalText )
                        eTHA = SDRTEXTHORZADJUST_RIGHT;
                    else
                        eTVA = SDRTEXTVERTADJUST_TOP;
                }
                break;
                case mso_anchorMiddle:
                break;
                case mso_anchorMiddleCentered:
                break;
                case mso_anchorBottom:
                {
                    if ( bVerticalText )
                        eTHA = SDRTEXTHORZADJUST_LEFT;
                    else
                        eTVA = SDRTEXTVERTADJUST_BOTTOM;
                }
                break;
                case mso_anchorBottomCentered:
                {
                    if ( bVerticalText )
                        eTHA = SDRTEXTHORZADJUST_LEFT;
                    else
                        eTVA = SDRTEXTVERTADJUST_BOTTOM;
                }
                break;
                default:
                    ;
            }

            aSet.Put( SdrTextVertAdjustItem( eTVA ) );
            aSet.Put( SdrTextHorzAdjustItem( eTHA ) );

            if (pObj != NULL)
            {
                pObj->SetMergedItemSet(aSet);
                pObj->SetModel(pSdrModel);

                if (bVerticalText && dynamic_cast< SdrTextObj* >( pObj ) )
                    dynamic_cast< SdrTextObj* >( pObj )->SetVerticalWriting(sal_True);

                if ( bIsSimpleDrawingTextBox )
                {
                    if ( nTextRotationAngle )
                    {
                        long nMinWH = rTextRect.GetWidth() < rTextRect.GetHeight() ?
                            rTextRect.GetWidth() : rTextRect.GetHeight();
                        nMinWH /= 2;
                        Point aPivot(rTextRect.TopLeft());
                        aPivot.X() += nMinWH;
                        aPivot.Y() += nMinWH;
                        double a = nTextRotationAngle * nPi180;
                        pObj->NbcRotate(aPivot, nTextRotationAngle, sin(a), cos(a));
                    }
                }

                if ( ( ( rObjData.nSpFlags & SP_FFLIPV ) || mnFix16Angle || nTextRotationAngle ) && dynamic_cast< SdrObjCustomShape* >( pObj ) )
                {
                    SdrObjCustomShape* pCustomShape = dynamic_cast< SdrObjCustomShape* >( pObj );

                    double fExtraTextRotation = 0.0;
                    if ( mnFix16Angle && !( GetPropertyValue( DFF_Prop_FitTextToShape ) & 4 ) )
                    {   // text is already rotated, we have to take back the object rotation if DFF_Prop_RotateText is false
                        fExtraTextRotation = -mnFix16Angle;
                    }
                    if ( rObjData.nSpFlags & SP_FFLIPV )    // sj: in ppt the text is flipped, whereas in word the text
                    {                                       // remains unchanged, so we have to take back the flipping here
                        fExtraTextRotation += 18000.0;      // because our core will flip text if the shape is flipped.
                    }
                    fExtraTextRotation += nTextRotationAngle;
                    if ( !::basegfx::fTools::equalZero( fExtraTextRotation ) )
                    {
                        fExtraTextRotation /= 100.0;
                        SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                        const OUString sTextRotateAngle( "TextRotateAngle" );
                        com::sun::star::beans::PropertyValue aPropVal;
                        aPropVal.Name = sTextRotateAngle;
                        aPropVal.Value <<= fExtraTextRotation;
                        aGeometryItem.SetPropertyValue( aPropVal );
                        pCustomShape->SetMergedItem( aGeometryItem );
                    }
                }
                else if ( mnFix16Angle )
                {
                    // rotate text with shape ?
                    double a = mnFix16Angle * nPi180;
                    pObj->NbcRotate( rObjData.aBoundRect.Center(), mnFix16Angle,
                                     sin( a ), cos( a ) );
                }
            }
        }
        else if( !pObj )
        {
            // simple rectangular objects are ignored by ImportObj()  :-(
            // this is OK for Draw but not for Calc and Writer
            // cause here these objects have a default border
            pObj = new SdrRectObj(rTextRect);
            pObj->SetModel( pSdrModel );
            SfxItemSet aSet( pSdrModel->GetItemPool() );
            ApplyAttributes( rSt, aSet, rObjData );

            const SfxPoolItem* pPoolItem=NULL;
            SfxItemState eState = aSet.GetItemState( XATTR_FILLCOLOR,
                                                     sal_False, &pPoolItem );
            if( SFX_ITEM_DEFAULT == eState )
                aSet.Put( XFillColorItem( String(),
                          Color( mnDefaultColor ) ) );
            pObj->SetMergedItemSet(aSet);
        }

        // Means that fBehindDocument is set
        if (GetPropertyValue(DFF_Prop_fPrint) & 0x20)
            pImpRec->bDrawHell = sal_True;
        else
            pImpRec->bDrawHell = sal_False;
        if (GetPropertyValue(DFF_Prop_fPrint) & 0x02)
            pImpRec->bHidden = sal_True;
        pImpRec->nNextShapeId   = GetPropertyValue( DFF_Prop_hspNext, 0 );

        if ( nTextId )
        {
            pImpRec->aTextId.nTxBxS = (sal_uInt16)( nTextId >> 16 );
            pImpRec->aTextId.nSequence = (sal_uInt16)nTextId;
        }

        pImpRec->nDxWrapDistLeft = GetPropertyValue(
                                    DFF_Prop_dxWrapDistLeft, 114935L ) / 635L;
        pImpRec->nDyWrapDistTop = GetPropertyValue(
                                    DFF_Prop_dyWrapDistTop, 0 ) / 635L;
        pImpRec->nDxWrapDistRight = GetPropertyValue(
                                    DFF_Prop_dxWrapDistRight, 114935L ) / 635L;
        pImpRec->nDyWrapDistBottom = GetPropertyValue(
                                    DFF_Prop_dyWrapDistBottom, 0 ) / 635L;
        // 16.16 fraction times total image width or height, as appropriate.

        if (SeekToContent(DFF_Prop_pWrapPolygonVertices, rSt))
        {
            delete pImpRec->pWrapPolygon;
            pImpRec->pWrapPolygon = NULL;

            sal_uInt16 nNumElemVert, nNumElemMemVert, nElemSizeVert;
            rSt >> nNumElemVert >> nNumElemMemVert >> nElemSizeVert;
            if (nNumElemVert && ((nElemSizeVert == 8) || (nElemSizeVert == 4)))
            {
                pImpRec->pWrapPolygon = new Polygon(nNumElemVert);
                for (sal_uInt16 i = 0; i < nNumElemVert; ++i)
                {
                    sal_Int32 nX, nY;
                    if (nElemSizeVert == 8)
                        rSt >> nX >> nY;
                    else
                    {
                        sal_Int16 nSmallX, nSmallY;
                        rSt >> nSmallX >> nSmallY;
                        nX = nSmallX;
                        nY = nSmallY;
                    }
                    (*(pImpRec->pWrapPolygon))[i].X() = nX;
                    (*(pImpRec->pWrapPolygon))[i].Y() = nY;
                }
            }
        }

        pImpRec->nCropFromTop = GetPropertyValue(
                                    DFF_Prop_cropFromTop, 0 );
        pImpRec->nCropFromBottom = GetPropertyValue(
                                    DFF_Prop_cropFromBottom, 0 );
        pImpRec->nCropFromLeft = GetPropertyValue(
                                    DFF_Prop_cropFromLeft, 0 );
        pImpRec->nCropFromRight = GetPropertyValue(
                                    DFF_Prop_cropFromRight, 0 );

        sal_uInt32 nLineFlags = GetPropertyValue( DFF_Prop_fNoLineDrawDash );

        if ( !IsHardAttribute( DFF_Prop_fLine ) &&
             pImpRec->eShapeType == mso_sptPictureFrame )
        {
            nLineFlags &= ~0x08;
        }

        pImpRec->eLineStyle = (nLineFlags & 8)
                              ? (MSO_LineStyle)GetPropertyValue(
                                                    DFF_Prop_lineStyle,
                                                    mso_lineSimple )
                              : (MSO_LineStyle)USHRT_MAX;
        pImpRec->eLineDashing = (MSO_LineDashing)GetPropertyValue(
                                        DFF_Prop_lineDashing, mso_lineSolid );

        pImpRec->nFlags = rObjData.nSpFlags;

        if( pImpRec->nShapeId )
        {
            // Complement Import Record List
            pImpRec->pObj = pObj;
            rImportData.aRecords.insert( pImpRec );

            // Complement entry in Z Order List with a pointer to this Object
            // Only store objects which are not deep inside the tree
            if( ( rObjData.nCalledByGroup == 0 )
                ||
                ( (rObjData.nSpFlags & SP_FGROUP)
                 && (rObjData.nCalledByGroup < 2) )
              )
                StoreShapeOrder( pImpRec->nShapeId,
                                ( ( (sal_uLong)pImpRec->aTextId.nTxBxS ) << 16 )
                                    + pImpRec->aTextId.nSequence, pObj );
        }
        else
            delete pImpRec;
    }

    return pObj;
}

/**
 * Special FastSave - Attributes
 */
void SwWW8ImplReader::Read_StyleCode( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen < 0)
    {
        bCpxStyle = false;
        return;
    }
    sal_uInt16 nColl = 0;
    if (pWwFib->GetFIBVersion() <= ww::eWW2)
        nColl = *pData;
    else
        nColl = SVBT16ToShort(pData);
    if (nColl < vColl.size())
    {
        SetTxtFmtCollAndListLevel( *pPaM, vColl[nColl] );
        bCpxStyle = true;
    }
}

/**
 * Read_Majority is for Majority (103) and Majority50 (108)
 */
void SwWW8ImplReader::Read_Majority( sal_uInt16, const sal_uInt8* , short )
{
}

/**
 * Stack
 */
void SwWW8FltControlStack::NewAttr(const SwPosition& rPos,
    const SfxPoolItem& rAttr)
{
    OSL_ENSURE(RES_TXTATR_FIELD != rAttr.Which(), "probably don't want to put"
        "fields into the control stack");
    OSL_ENSURE(RES_FLTR_REDLINE != rAttr.Which(), "probably don't want to put"
        "redlines into the control stack");
    SwFltControlStack::NewAttr(rPos, rAttr);
}

SwFltStackEntry* SwWW8FltControlStack::SetAttr(const SwPosition& rPos, sal_uInt16 nAttrId,
    sal_Bool bTstEnde, long nHand, sal_Bool )
{
    SwFltStackEntry *pRet = NULL;
    // Doing a textbox, and using the control stack only as a temporary
    // collection point for properties which will are not to be set into
    // the real document
    if (rReader.pPlcxMan && rReader.pPlcxMan->GetDoingDrawTextBox())
    {
        size_t nCnt = size();
        for (size_t i=0; i < nCnt; ++i)
        {
            SwFltStackEntry& rEntry = (*this)[i];
            if (nAttrId == rEntry.pAttr->Which())
            {
                DeleteAndDestroy(i--);
                --nCnt;
            }
        }
    }
    else // Normal case, set the attribute into the document
        pRet = SwFltControlStack::SetAttr(rPos, nAttrId, bTstEnde, nHand);
    return pRet;
}

long GetListFirstLineIndent(const SwNumFmt &rFmt)
{
    OSL_ENSURE( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION,
            "<GetListFirstLineIndent> - misusage: position-and-space-mode does not equal LABEL_WIDTH_AND_POSITION" );

    SvxAdjust eAdj = rFmt.GetNumAdjust();
    long nReverseListIndented;
    if (eAdj == SVX_ADJUST_RIGHT)
        nReverseListIndented = -rFmt.GetCharTextDistance();
    else if (eAdj == SVX_ADJUST_CENTER)
        nReverseListIndented = rFmt.GetFirstLineOffset()/2;
    else
        nReverseListIndented = rFmt.GetFirstLineOffset();
    return nReverseListIndented;
}

static long lcl_GetTrueMargin(const SvxLRSpaceItem &rLR, const SwNumFmt &rFmt,
    long &rFirstLinePos)
{
    OSL_ENSURE( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION,
            "<lcl_GetTrueMargin> - misusage: position-and-space-mode does not equal LABEL_WIDTH_AND_POSITION" );

    const long nBodyIndent = rLR.GetTxtLeft();
    const long nFirstLineDiff = rLR.GetTxtFirstLineOfst();
    rFirstLinePos = nBodyIndent + nFirstLineDiff;

    const long nPseudoListBodyIndent = rFmt.GetAbsLSpace();
    const long nReverseListIndented = GetListFirstLineIndent(rFmt);
    long nExtraListIndent = nPseudoListBodyIndent + nReverseListIndented;

    return nExtraListIndent > 0 ? nExtraListIndent : 0;
}

// #i103711#
// #i105414#
void SyncIndentWithList( SvxLRSpaceItem &rLR,
                         const SwNumFmt &rFmt,
                         const bool bFirstLineOfstSet,
                         const bool bLeftIndentSet )
{
    if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        long nWantedFirstLinePos;
        long nExtraListIndent = lcl_GetTrueMargin(rLR, rFmt, nWantedFirstLinePos);
        rLR.SetTxtLeft(nWantedFirstLinePos - nExtraListIndent);
        rLR.SetTxtFirstLineOfst(0);
    }
    else if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        if ( !bFirstLineOfstSet && bLeftIndentSet &&
             rFmt.GetFirstLineIndent() != 0 )
        {
            rLR.SetTxtFirstLineOfst( rFmt.GetFirstLineIndent() );
        }
        else if ( bFirstLineOfstSet && !bLeftIndentSet &&
                  rFmt.GetIndentAt() != 0 )
        {
            rLR.SetTxtLeft( rFmt.GetIndentAt() );
        }
        else if (!bFirstLineOfstSet && !bLeftIndentSet )
        {
            if ( rFmt.GetFirstLineIndent() != 0 )
            {
                rLR.SetTxtFirstLineOfst( rFmt.GetFirstLineIndent() );
            }
            if ( rFmt.GetIndentAt() != 0 )
            {
                rLR.SetTxtLeft( rFmt.GetIndentAt() );
            }
        }
    }
}

const SwNumFmt* SwWW8FltControlStack::GetNumFmtFromStack(const SwPosition &rPos,
    const SwTxtNode &rTxtNode)
{
    const SwNumFmt *pRet = 0;
    const SfxPoolItem *pItem = GetStackAttr(rPos, RES_FLTR_NUMRULE);
    if (pItem && rTxtNode.GetNumRule())
    {
        String sName(((SfxStringItem*)pItem)->GetValue());
        if (rTxtNode.IsCountedInList())
        {
            const SwNumRule *pRule = pDoc->FindNumRulePtr(sName);

            pRet = GetNumFmtFromSwNumRuleLevel(*pRule, rTxtNode.GetActualListLevel());
        }
    }
    return pRet;
}

sal_Int32 SwWW8FltControlStack::GetCurrAttrCP() const
{
    return rReader.GetCurrAttrCP();
}

bool SwWW8FltControlStack::IsParaEndInCPs(sal_Int32 nStart,sal_Int32 nEnd,bool bSdOD) const
{
    return rReader.IsParaEndInCPs(nStart,nEnd,bSdOD);
}

/**
 * Clear the para end position recorded in reader intermittently
 * for the least impact on loading performance.
 */
void SwWW8FltControlStack::ClearParaEndPosition()
{
    if ( !empty() )
        return;

    rReader.ClearParaEndPosition();
}

bool SwWW8FltControlStack::CheckSdOD(sal_Int32 nStart,sal_Int32 nEnd)
{
    return rReader.IsParaEndInCPs(nStart,nEnd);
}

void SwWW8FltControlStack::SetAttrInDoc(const SwPosition& rTmpPos,
    SwFltStackEntry& rEntry)
{
    switch (rEntry.pAttr->Which())
    {
        case RES_LR_SPACE:
            {
                /*
                 Loop over the affected nodes and
                 a) convert the word style absolute indent to indent relative
                    to any numbering indent active on the nodes
                 b) adjust the writer style tabstops relative to the old
                    paragraph indent to be relative to the new paragraph indent
                */
                using namespace sw::util;
                SwPaM aRegion(rTmpPos);
                if (rEntry.MakeRegion(pDoc, aRegion, false))
                {
                    SvxLRSpaceItem aNewLR( *(SvxLRSpaceItem*)rEntry.pAttr );
                    sal_uLong nStart = aRegion.Start()->nNode.GetIndex();
                    sal_uLong nEnd   = aRegion.End()->nNode.GetIndex();
                    for(; nStart <= nEnd; ++nStart)
                    {
                        SwNode* pNode = pDoc->GetNodes()[ nStart ];
                        if (!pNode || !pNode->IsTxtNode())
                            continue;

                        SwCntntNode* pNd = (SwCntntNode*)pNode;
                        SvxLRSpaceItem aOldLR = (const SvxLRSpaceItem&)pNd->GetAttr(RES_LR_SPACE);

                        SwTxtNode *pTxtNode = (SwTxtNode*)pNode;

                        const SwNumFmt *pNum = 0;
                        pNum = GetNumFmtFromStack( *aRegion.GetPoint(), *pTxtNode );
                        if (!pNum)
                        {
                            pNum = GetNumFmtFromTxtNode(*pTxtNode);
                        }

                        if ( pNum )
                        {
                            // #i103711#
                            const bool bFirstLineIndentSet =
                                ( rReader.maTxtNodesHavingFirstLineOfstSet.end() !=
                                    rReader.maTxtNodesHavingFirstLineOfstSet.find( pNode ) );
                            // #i105414#
                            const bool bLeftIndentSet =
                                (  rReader.maTxtNodesHavingLeftIndentSet.end() !=
                                    rReader.maTxtNodesHavingLeftIndentSet.find( pNode ) );
                            SyncIndentWithList( aNewLR, *pNum,
                                                bFirstLineIndentSet,
                                                bLeftIndentSet );
                        }

                        if (aNewLR == aOldLR)
                            continue;

                        pNd->SetAttr(aNewLR);

                    }
                }
            }
            break;
        case RES_TXTATR_FIELD:
            OSL_ENSURE(!this, "What is a field doing in the control stack,"
                "probably should have been in the endstack");
            break;
        case RES_TXTATR_INETFMT:
            {
                SwPaM aRegion(rTmpPos);
                if (rEntry.MakeRegion(pDoc, aRegion, false))
                {
                    SwFrmFmt *pFrm;
                    // If we have just one single inline graphic then
                    // don't insert a field for the single frame, set
                    // the frames hyperlink field attribute directly.
                    if (0 != (pFrm = rReader.ContainsSingleInlineGraphic(aRegion)))
                    {
                        const SwFmtINetFmt *pAttr = (const SwFmtINetFmt *)
                            rEntry.pAttr;
                        SwFmtURL aURL;
                        aURL.SetURL(pAttr->GetValue(), false);
                        aURL.SetTargetFrameName(pAttr->GetTargetFrame());
                        pFrm->SetFmtAttr(aURL);
                    }
                    else
                    {
                        pDoc->InsertPoolItem(aRegion, *rEntry.pAttr, 0);
                    }
                }
            }
            break;
        default:
            SwFltControlStack::SetAttrInDoc(rTmpPos, rEntry);
            break;
    }
}

const SfxPoolItem* SwWW8FltControlStack::GetFmtAttr(const SwPosition& rPos,
    sal_uInt16 nWhich)
{
    const SfxPoolItem *pItem = GetStackAttr(rPos, nWhich);
    if (!pItem)
    {
        SwCntntNode const*const pNd = rPos.nNode.GetNode().GetCntntNode();
        if (!pNd)
            pItem = &pDoc->GetAttrPool().GetDefaultItem(nWhich);
        else
        {
            /*
            If we're hunting for the indent on a paragraph and need to use the
            parent style indent, then return the indent in msword format, and
            not writer format, because that's the style that the filter works
            in (naturally)
            */
            if (nWhich == RES_LR_SPACE)
            {
                SfxItemState eState = SFX_ITEM_DEFAULT;
                if (const SfxItemSet *pSet = pNd->GetpSwAttrSet())
                    eState = pSet->GetItemState(RES_LR_SPACE, false);
                if (eState != SFX_ITEM_SET && rReader.nAktColl < rReader.vColl.size())
                    pItem = &(rReader.vColl[rReader.nAktColl].maWordLR);
            }

            /*
            If we're hunting for a character property, try and exact position
            within the text node for lookup
            */
            if (pNd->IsTxtNode())
            {
                xub_StrLen nPos = rPos.nContent.GetIndex();
                SfxItemSet aSet(pDoc->GetAttrPool(), nWhich, nWhich);
                if (static_cast<const SwTxtNode*>(pNd)->GetAttr(aSet, nPos, nPos))
                    pItem = aSet.GetItem(nWhich);
            }

            if (!pItem)
                pItem = &pNd->GetAttr(nWhich);
        }
    }
    return pItem;
}

const SfxPoolItem* SwWW8FltControlStack::GetStackAttr(const SwPosition& rPos,
    sal_uInt16 nWhich)
{
    SwFltPosition aFltPos(rPos);

    size_t nSize = size();
    while (nSize)
    {
        const SwFltStackEntry& rEntry = (*this)[ --nSize ];
        if (rEntry.pAttr->Which() == nWhich)
        {
            if ( (rEntry.bOpen) ||
                 (
                  (rEntry.m_aMkPos.m_nNode <= aFltPos.m_nNode) &&
                  (rEntry.m_aPtPos.m_nNode >= aFltPos.m_nNode) &&
                  (rEntry.m_aMkPos.m_nCntnt <= aFltPos.m_nCntnt) &&
                  (rEntry.m_aPtPos.m_nCntnt > aFltPos.m_nCntnt)
                 )
               )
                /*
                 * e.g. half-open range [0-3) so asking for properties at 3
                 * means props that end at 3 are not included
                 */
            {
                return rEntry.pAttr;
            }
        }
    }
    return 0;
}

bool SwWW8FltRefStack::IsFtnEdnBkmField(const SwFmtFld& rFmtFld, sal_uInt16& rBkmNo)
{
    const SwField* pFld = rFmtFld.GetFld();
    sal_uInt16 nSubType;
    if(pFld && (RES_GETREFFLD == pFld->Which())
        && ((REF_FOOTNOTE == (nSubType = pFld->GetSubType())) || (REF_ENDNOTE  == nSubType))
        && !((SwGetRefField*)pFld)->GetSetRefName().isEmpty())
    {
        const IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
        IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->findMark(
            ((SwGetRefField*)pFld)->GetSetRefName());
        if(ppBkmk != pMarkAccess->getMarksEnd())
        {
            // find Sequence No of corresponding Foot-/Endnote
            rBkmNo = ppBkmk - pMarkAccess->getMarksBegin();
            return true;
        }
    }
    return false;
}

void SwWW8FltRefStack::SetAttrInDoc(const SwPosition& rTmpPos,
    SwFltStackEntry& rEntry)
{
    switch (rEntry.pAttr->Which())
    {
        /*
        Look up these in our lists of bookmarks that were changed to
        variables, and replace the ref field with a var field, otherwise
        do normal (?) strange stuff
        */
        case RES_TXTATR_FIELD:
        {
            SwNodeIndex aIdx(rEntry.m_aMkPos.m_nNode, 1);
            SwPaM aPaM(aIdx, rEntry.m_aMkPos.m_nCntnt);

            SwFmtFld& rFmtFld   = *(SwFmtFld*)rEntry.pAttr;
            SwField* pFld = rFmtFld.GetFld();

            if (!RefToVar(pFld, rEntry))
            {
                sal_uInt16 nBkmNo;
                if( IsFtnEdnBkmField(rFmtFld, nBkmNo) )
                {
                    ::sw::mark::IMark const * const pMark = (pDoc->getIDocumentMarkAccess()->getMarksBegin() + nBkmNo)->get();

                    const SwPosition& rBkMrkPos = pMark->GetMarkPos();

                    SwTxtNode* pTxt = rBkMrkPos.nNode.GetNode().GetTxtNode();
                    if( pTxt && rBkMrkPos.nContent.GetIndex() )
                    {
                        SwTxtAttr* const pFtn = pTxt->GetTxtAttrForCharAt(
                            rBkMrkPos.nContent.GetIndex()-1, RES_TXTATR_FTN );
                        if( pFtn )
                        {
                            sal_uInt16 nRefNo = ((SwTxtFtn*)pFtn)->GetSeqRefNo();

                            ((SwGetRefField*)pFld)->SetSeqNo( nRefNo );

                            if( pFtn->GetFtn().IsEndNote() )
                                ((SwGetRefField*)pFld)->SetSubType(REF_ENDNOTE);
                        }
                    }
                }
            }

            pDoc->InsertPoolItem(aPaM, *rEntry.pAttr, 0);
            MoveAttrs(*aPaM.GetPoint());
        }
        break;
        case RES_FLTR_TOX:
            SwFltEndStack::SetAttrInDoc(rTmpPos, rEntry);
            break;
        default:
        case RES_FLTR_BOOKMARK:
            OSL_ENSURE(!this, "EndStck used with non field, not what we want");
            SwFltEndStack::SetAttrInDoc(rTmpPos, rEntry);
            break;
    }
}

/*
 For styles we will do our tabstop arithmetic in word style and adjust them to
 writer style after all the styles have been finished and the dust settles as
 to what affects what.

 For explicit attributes we turn the adjusted writer tabstops back into 0 based
 word indexes and we'll turn them back into writer indexes when setting them
 into the document. If explicit left indent exist which affects them, then this
 is handled when the explicit left indent is set into the document
*/
void SwWW8ImplReader::Read_Tab(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    if (nLen < 0)
    {
        pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_PARATR_TABSTOP);
        return;
    }

    sal_uInt8 nDel = (nLen > 0) ? pData[0] : 0;
    const sal_uInt8* pDel = pData + 1;                   // Del - Array

    sal_uInt8 nIns = (nLen > nDel*2+1) ? pData[nDel*2+1] : 0;
    const sal_uInt8* pIns = pData + 2*nDel + 2;          // Ins - Array

    short nRequiredLength = 2 + 2*nDel + 2*nIns + 1*nIns;
    if (nRequiredLength > nLen)
    {
        // would require more data than available to describe!
        // discard invalid record
        nIns = 0;
        nDel = 0;
    }

    WW8_TBD* pTyp = (WW8_TBD*)(pData + 2*nDel + 2*nIns + 2); // Type Array

    SvxTabStopItem aAttr(0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP);

    const SwTxtFmtColl* pSty = 0;
    sal_uInt16 nTabBase;
    if (pAktColl && nAktColl < vColl.size()) // StyleDef
    {
        nTabBase = vColl[nAktColl].nBase;
        if (nTabBase < vColl.size())  // Based On
            pSty = (const SwTxtFmtColl*)vColl[nTabBase].pFmt;
    }
    else
    { // Text
        nTabBase = nAktColl;
        if (nAktColl < vColl.size())
            pSty = (const SwTxtFmtColl*)vColl[nAktColl].pFmt;
        //TODO: figure out else here
    }

    bool bFound = false;
    ::boost::unordered_set<size_t> aLoopWatch;
    while (pSty && !bFound)
    {
        const SfxPoolItem* pTabs;
        bFound = pSty->GetAttrSet().GetItemState(RES_PARATR_TABSTOP, false,
            &pTabs) == SFX_ITEM_SET;
        if( bFound )
            aAttr = *((const SvxTabStopItem*)pTabs);
        else
        {
            sal_uInt16 nOldTabBase = nTabBase;
            // If based on another
            if (nTabBase < vColl.size())
                nTabBase = vColl[nTabBase].nBase;

            if (
                    nTabBase < vColl.size() &&
                    nOldTabBase != nTabBase &&
                    nTabBase != ww::stiNil
               )
            {
                // #i61789: Stop searching when next style is the same as the
                // current one (prevent loop)
                aLoopWatch.insert(reinterpret_cast<size_t>(pSty));
                if (nTabBase < vColl.size())
                    pSty = (const SwTxtFmtColl*)vColl[nTabBase].pFmt;
                //TODO figure out the else branch

                if (aLoopWatch.find(reinterpret_cast<size_t>(pSty)) !=
                    aLoopWatch.end())
                    pSty = 0;
            }
            else
                pSty = 0; // Give up on the search
        }
    }

    SvxTabStop aTabStop;
    for (short i=0; i < nDel; ++i)
    {
        sal_uInt16 nPos = aAttr.GetPos(SVBT16ToShort(pDel + i*2));
        if( nPos != SVX_TAB_NOTFOUND )
            aAttr.Remove( nPos, 1 );
    }

    for (short i=0; i < nIns; ++i)
    {
        short nPos = SVBT16ToShort(pIns + i*2);
        aTabStop.GetTabPos() = nPos;
        switch( SVBT8ToByte( pTyp[i].aBits1 ) & 0x7 ) // pTyp[i].jc
        {
            case 0:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_LEFT;
                break;
            case 1:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_CENTER;
                break;
            case 2:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_RIGHT;
                break;
            case 3:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_DECIMAL;
                break;
            case 4:
                continue; // Ignore Bar
        }

        switch( SVBT8ToByte( pTyp[i].aBits1 ) >> 3 & 0x7 )
        {
            case 0:
                aTabStop.GetFill() = ' ';
                break;
            case 1:
                aTabStop.GetFill() = '.';
                break;
            case 2:
                aTabStop.GetFill() = '-';
                break;
            case 3:
            case 4:
                aTabStop.GetFill() = '_';
                break;
        }

        sal_uInt16 nPos2 = aAttr.GetPos( nPos );
        if (nPos2 != SVX_TAB_NOTFOUND)
            aAttr.Remove(nPos2, 1); // Or else Insert() refuses
        aAttr.Insert(aTabStop);
    }

    if (nIns || nDel)
        NewAttr(aAttr);
    else
    {
        // Here we have a tab definition which inserts no extra tabs, or deletes
        // no existing tabs. An older version of writer is probably the creater
        // of the document  :-( . So if we are importing a style we can just
        // ignore it. But if we are importing into text we cannot as during
        // text SwWW8ImplReader::Read_Tab is called at the begin and end of
        // the range the attrib affects, and ignoring it would upset the
        // balance
        if (!pAktColl) // not importing into a style
        {
            using namespace sw::util;
            SvxTabStopItem aOrig = pSty ?
            ItemGet<SvxTabStopItem>(*pSty, RES_PARATR_TABSTOP) :
            DefaultItemGet<SvxTabStopItem>(rDoc, RES_PARATR_TABSTOP);
            NewAttr(aOrig);
        }
    }
}

/**
 * DOP
*/
void SwWW8ImplReader::ImportDop()
{
    // correct the LastPrinted date in DocumentProperties
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        mpDocShell->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocuProps(
        xDPS->getDocumentProperties());
    OSL_ENSURE(xDocuProps.is(), "DocumentProperties is null");
    if (xDocuProps.is())
    {
        DateTime aLastPrinted(
            msfilter::util::DTTM2DateTime(pWDop->dttmLastPrint));
       ::util::DateTime uDT(aLastPrinted.GetNanoSec(),
            aLastPrinted.GetSec(), aLastPrinted.GetMin(),
            aLastPrinted.GetHour(), aLastPrinted.GetDay(),
            aLastPrinted.GetMonth(), aLastPrinted.GetYear(), false);
        xDocuProps->setPrintDate(uDT);
    }

    //
    // COMPATIBILITY FLAGS START
    //

    // #i78951# - remember the unknown compatability options
    // so as to export them out
    rDoc.Setn32DummyCompatabilityOptions1( pWDop->GetCompatabilityOptions());
    rDoc.Setn32DummyCompatabilityOptions2( pWDop->GetCompatabilityOptions2());

    // The distance between two paragraphs is the sum of the bottom distance of
    // the first paragraph and the top distance of the second one
    rDoc.set(IDocumentSettingAccess::PARA_SPACE_MAX, pWDop->fDontUseHTMLAutoSpacing);
    rDoc.set(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES, true );
    // move tabs on alignment
    rDoc.set(IDocumentSettingAccess::TAB_COMPAT, true);
    // #i24363# tab stops relative to indent
    rDoc.set(IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT, false);

    // Import Default Tabs
    long nDefTabSiz = pWDop->dxaTab;
    if( nDefTabSiz < 56 )
        nDefTabSiz = 709;

    // We want exactly one DefaultTab
    SvxTabStopItem aNewTab( 1, sal_uInt16(nDefTabSiz), SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
    ((SvxTabStop&)aNewTab[0]).GetAdjustment() = SVX_TAB_ADJUST_DEFAULT;

    rDoc.GetAttrPool().SetPoolDefaultItem( aNewTab );

    // Import zoom factor
    if (pWDop->wScaleSaved)
    {
        uno::Sequence<beans::PropertyValue> aViewProps(3);
        aViewProps[0].Name = "ZoomFactor";
        aViewProps[0].Value <<= sal_Int16(pWDop->wScaleSaved);
        aViewProps[1].Name = "VisibleBottom";
        aViewProps[1].Value <<= sal_Int32(0);
        aViewProps[2].Name = "ZoomType";
        aViewProps[2].Value <<= sal_Int16(0);

        uno::Reference< uno::XComponentContext > xComponentContext(comphelper::getProcessComponentContext());
        uno::Reference<container::XIndexContainer> xBox = document::IndexedPropertyValues::create(xComponentContext);
        xBox->insertByIndex(sal_Int32(0), uno::makeAny(aViewProps));
        uno::Reference<container::XIndexAccess> xIndexAccess(xBox, uno::UNO_QUERY);
        uno::Reference<document::XViewDataSupplier> xViewDataSupplier(mpDocShell->GetModel(), uno::UNO_QUERY);
        xViewDataSupplier->setViewData(xIndexAccess);
    }

    rDoc.set(IDocumentSettingAccess::USE_VIRTUAL_DEVICE, !pWDop->fUsePrinterMetrics);
    rDoc.set(IDocumentSettingAccess::USE_HIRES_VIRTUAL_DEVICE, true);
    rDoc.set(IDocumentSettingAccess::ADD_FLY_OFFSETS, true );
    rDoc.set(IDocumentSettingAccess::ADD_EXT_LEADING, !pWDop->fNoLeading);
    rDoc.set(IDocumentSettingAccess::OLD_NUMBERING, false);
    rDoc.set(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING, false); // #i47448#
    rDoc.set(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK, !pWDop->fExpShRtn); // #i49277#, #i56856#
    rDoc.set(IDocumentSettingAccess::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT, false);  // #i53199#
    rDoc.set(IDocumentSettingAccess::OLD_LINE_SPACING, false);

    // #i25901# - set new compatibility option
    //      'Add paragraph and table spacing at bottom of table cells'
    rDoc.set(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS, true);

    // #i11860# - set new compatibility option
    //      'Use former object positioning' to <sal_False>
    rDoc.set(IDocumentSettingAccess::USE_FORMER_OBJECT_POS, false);

    // #i27767# - set new compatibility option
    //      'Conder Wrapping mode when positioning object' to <sal_True>
    rDoc.set(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION, true);

    rDoc.set(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING, false); // #i13832#, #i24135#

    rDoc.set(IDocumentSettingAccess::TABLE_ROW_KEEP, true); //SetTableRowKeep( true );

    rDoc.set(IDocumentSettingAccess::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION, true); // #i3952#

    rDoc.set(IDocumentSettingAccess::INVERT_BORDER_SPACING, true);
    rDoc.set(IDocumentSettingAccess::COLLAPSE_EMPTY_CELL_PARA, true);
    rDoc.set(IDocumentSettingAccess::TAB_OVERFLOW, true);
    rDoc.set(IDocumentSettingAccess::UNBREAKABLE_NUMBERINGS, true);
    rDoc.set(IDocumentSettingAccess::CLIPPED_PICTURES, true);
    rDoc.set(IDocumentSettingAccess::TAB_OVER_MARGIN, true);
    rDoc.set(IDocumentSettingAccess::SURROUND_TEXT_WRAP_SMALL, true);

    //
    // COMPATIBILITY FLAGS END
    //

    // Import magic doptypography information, if its there
    if (pWwFib->nFib > 105)
        ImportDopTypography(pWDop->doptypography);

    // disable form design mode to be able to use imported controls directly
    // #i31239# always disable form design mode, not only in protected docs
    {
        using namespace com::sun::star;

        uno::Reference<lang::XComponent> xModelComp(mpDocShell->GetModel(),
           uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xDocProps(xModelComp,
           uno::UNO_QUERY);
        if (xDocProps.is())
        {
            uno::Reference<beans::XPropertySetInfo> xInfo =
                xDocProps->getPropertySetInfo();
            sal_Bool bValue = false;
            if (xInfo.is())
            {
                if (xInfo->hasPropertyByName("ApplyFormDesignMode"))
                {
                    xDocProps->setPropertyValue("ApplyFormDesignMode", cppu::bool2any(bValue));
                }
            }
        }
    }

    mpDocShell->SetModifyPasswordHash(pWDop->lKeyProtDoc);

    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    if (rOpt.IsUseEnhancedFields())
        rDoc.set(IDocumentSettingAccess::PROTECT_FORM, pWDop->fProtEnabled );
}

void SwWW8ImplReader::ImportDopTypography(const WW8DopTypography &rTypo)
{
    using namespace com::sun::star;
    switch (rTypo.iLevelOfKinsoku)
    {
        case 2: // custom
            {
                i18n::ForbiddenCharacters aForbidden(rTypo.rgxchFPunct,
                    rTypo.rgxchLPunct);
                rDoc.setForbiddenCharacters(rTypo.GetConvertedLang(),
                        aForbidden);
                // Obviously cannot set the standard level 1 for japanese, so
                // bail out now while we can.
                if (rTypo.GetConvertedLang() == LANGUAGE_JAPANESE)
                    return;
            }
            break;
        default:
            break;
    }

    /*
    This MS hack means that level 2 of japanese is not in operation, so we put
    in what we know are the MS defaults, there is a complementary reverse
    hack in the writer. Its our default as well, but we can set it anyway
    as a flag for later.
    */
    if (!rTypo.reserved2)
    {
        i18n::ForbiddenCharacters aForbidden(rTypo.GetJapanNotBeginLevel1(),
            rTypo.GetJapanNotEndLevel1());
        rDoc.setForbiddenCharacters(LANGUAGE_JAPANESE,aForbidden);
    }

    rDoc.set(IDocumentSettingAccess::KERN_ASIAN_PUNCTUATION, rTypo.fKerningPunct);
    rDoc.setCharacterCompressionType(static_cast<SwCharCompressType>(rTypo.iJustification));
}

/**
 * Footnotes and Endnotes
 */
WW8ReaderSave::WW8ReaderSave(SwWW8ImplReader* pRdr ,WW8_CP nStartCp) :
    maTmpPos(*pRdr->pPaM->GetPoint()),
    mpOldStck(pRdr->pCtrlStck),
    mpOldAnchorStck(pRdr->pAnchorStck),
    mpOldRedlines(pRdr->mpRedlineStack),
    mpOldPlcxMan(pRdr->pPlcxMan),
    mpWFlyPara(pRdr->pWFlyPara),
    mpSFlyPara(pRdr->pSFlyPara),
    mpPreviousNumPaM(pRdr->pPreviousNumPaM),
    mpPrevNumRule(pRdr->pPrevNumRule),
    mpTableDesc(pRdr->pTableDesc),
    mnInTable(pRdr->nInTable),
    mnAktColl(pRdr->nAktColl),
    mcSymbol(pRdr->cSymbol),
    mbIgnoreText(pRdr->bIgnoreText),
    mbSymbol(pRdr->bSymbol),
    mbHdFtFtnEdn(pRdr->bHdFtFtnEdn),
    mbTxbxFlySection(pRdr->bTxbxFlySection),
    mbAnl(pRdr->bAnl),
    mbInHyperlink(pRdr->bInHyperlink),
    mbPgSecBreak(pRdr->bPgSecBreak),
    mbWasParaEnd(pRdr->bWasParaEnd),
    mbHasBorder(pRdr->bHasBorder),
    mbFirstPara(pRdr->bFirstPara)
{
    pRdr->bSymbol = false;
    pRdr->bHdFtFtnEdn = true;
    pRdr->bTxbxFlySection = pRdr->bAnl = pRdr->bPgSecBreak = pRdr->bWasParaEnd
        = pRdr->bHasBorder = false;
    pRdr->bFirstPara = true;
    pRdr->nInTable = 0;
    pRdr->pWFlyPara = 0;
    pRdr->pSFlyPara = 0;
    pRdr->pPreviousNumPaM = 0;
    pRdr->pPrevNumRule = 0;
    pRdr->pTableDesc = 0;
    pRdr->nAktColl = 0;


    pRdr->pCtrlStck = new SwWW8FltControlStack(&pRdr->rDoc, pRdr->nFieldFlags,
        *pRdr);

    pRdr->mpRedlineStack = new sw::util::RedlineStack(pRdr->rDoc);

    pRdr->pAnchorStck = new SwWW8FltAnchorStack(&pRdr->rDoc, pRdr->nFieldFlags);

    // Save the attribute manager: we need this as the newly created PLCFx Manager
    // access the same FKPs as the old one and their Start-End position changes.
    if (pRdr->pPlcxMan)
        pRdr->pPlcxMan->SaveAllPLCFx(maPLCFxSave);

    if (nStartCp != -1)
    {
        pRdr->pPlcxMan = new WW8PLCFMan(pRdr->pSBase,
            mpOldPlcxMan->GetManType(), nStartCp);
    }

    maOldApos.push_back(false);
    maOldApos.swap(pRdr->maApos);
    maOldFieldStack.swap(pRdr->maFieldStack);
}

void WW8ReaderSave::Restore( SwWW8ImplReader* pRdr )
{
    pRdr->pWFlyPara = mpWFlyPara;
    pRdr->pSFlyPara = mpSFlyPara;
    pRdr->pPreviousNumPaM = mpPreviousNumPaM;
    pRdr->pPrevNumRule = mpPrevNumRule;
    pRdr->pTableDesc = mpTableDesc;
    pRdr->cSymbol = mcSymbol;
    pRdr->bSymbol = mbSymbol;
    pRdr->bIgnoreText = mbIgnoreText;
    pRdr->bHdFtFtnEdn = mbHdFtFtnEdn;
    pRdr->bTxbxFlySection = mbTxbxFlySection;
    pRdr->nInTable = mnInTable;
    pRdr->bAnl = mbAnl;
    pRdr->bInHyperlink = mbInHyperlink;
    pRdr->bWasParaEnd = mbWasParaEnd;
    pRdr->bPgSecBreak = mbPgSecBreak;
    pRdr->nAktColl = mnAktColl;
    pRdr->bHasBorder = mbHasBorder;
    pRdr->bFirstPara = mbFirstPara;

    // Close all attributes as attributes could be created that extend the Fly
    pRdr->DeleteCtrlStk();
    pRdr->pCtrlStck = mpOldStck;

    pRdr->mpRedlineStack->closeall(*pRdr->pPaM->GetPoint());
    delete pRdr->mpRedlineStack;
    pRdr->mpRedlineStack = mpOldRedlines;

    pRdr->DeleteAnchorStk();
    pRdr->pAnchorStck = mpOldAnchorStck;

    *pRdr->pPaM->GetPoint() = maTmpPos;

    if (mpOldPlcxMan != pRdr->pPlcxMan)
    {
        delete pRdr->pPlcxMan;
        pRdr->pPlcxMan = mpOldPlcxMan;
    }
    if (pRdr->pPlcxMan)
        pRdr->pPlcxMan->RestoreAllPLCFx(maPLCFxSave);
    pRdr->maApos.swap(maOldApos);
    pRdr->maFieldStack.swap(maOldFieldStack);
}

void SwWW8ImplReader::Read_HdFtFtnText( const SwNodeIndex* pSttIdx,
    long nStartCp, long nLen, ManTypes nType )
{
    // Saves Flags (amongst other things) and resets them
    WW8ReaderSave aSave( this );

    pPaM->GetPoint()->nNode = pSttIdx->GetIndex() + 1;
    pPaM->GetPoint()->nContent.Assign( pPaM->GetCntntNode(), 0 );

    // Read Text for Header, Footer or Footnote
    ReadText( nStartCp, nLen, nType ); // Ignore Sepx when doing so
    aSave.Restore( this );
}

/**
 * Use authornames, if not available fall back to initials.
 */
long SwWW8ImplReader::Read_And(WW8PLCFManResult* pRes)
{
    WW8PLCFx_SubDoc* pSD = pPlcxMan->GetAtn();
    if( !pSD )
        return 0;

    String sAuthor;
    String sInitials;
    String sName;
    if( bVer67 )
    {
        const WW67_ATRD* pDescri = (const WW67_ATRD*)pSD->GetData();
        const String* pA = GetAnnotationAuthor(SVBT16ToShort(pDescri->ibst));
        if (pA)
            sAuthor = *pA;
        else
            sAuthor = String(pDescri->xstUsrInitl + 1, pDescri->xstUsrInitl[0],
                RTL_TEXTENCODING_MS_1252);
    }
    else
    {
        const WW8_ATRD* pDescri = (const WW8_ATRD*)pSD->GetData();

        {
            sal_uInt16 nLen = SVBT16ToShort(pDescri->xstUsrInitl[0]);
            for(sal_uInt16 nIdx = 1; nIdx <= nLen; ++nIdx)
                sInitials += SVBT16ToShort(pDescri->xstUsrInitl[nIdx]);
        }

        if (const String* pA = GetAnnotationAuthor(SVBT16ToShort(pDescri->ibst)))
            sAuthor = *pA;
        else
            sAuthor = sInitials;

        // If there is a bookmark tag, a text range should be commented.
        sal_uInt32 nTagBkmk = SVBT32ToUInt32(pDescri->ITagBkmk);
        if (nTagBkmk != 0xFFFFFFFF)
        {
            sName = OUString::valueOf(sal_Int32(nTagBkmk));
            int nAtnIndex = GetAnnotationIndex(nTagBkmk);
            if (nAtnIndex != -1)
            {
                WW8_CP nStart = GetAnnotationStart(nAtnIndex);
                WW8_CP nEnd = GetAnnotationEnd(nAtnIndex);
                sal_Int32 nLen = nEnd - nStart;
                // Don't support ranges affecting multiple SwTxtNode for now.
                if (nLen && pPaM->GetPoint()->nContent.GetIndex() >= nLen)
                {
                    pPaM->SetMark();
                    pPaM->GetPoint()->nContent -= nLen;
                }
            }
        }
    }

    sal_uInt32 nDateTime = 0;

    if (sal_uInt8 * pExtended = pPlcxMan->GetExtendedAtrds()) // Word < 2002 has no date data for comments
    {
        sal_uLong nIndex = pSD->GetIdx() & 0xFFFF; // Index is (stupidly) multiplexed for WW8PLCFx_SubDocs
        if (pWwFib->lcbAtrdExtra/18 > nIndex)
            nDateTime = SVBT32ToUInt32(*(SVBT32*)(pExtended+(nIndex*18)));
    }

    DateTime aDate = msfilter::util::DTTM2DateTime(nDateTime);

    String sTxt;
    OutlinerParaObject *pOutliner = ImportAsOutliner( sTxt, pRes->nCp2OrIdx,
        pRes->nCp2OrIdx + pRes->nMemLen, MAN_AND );

    this->pFmtOfJustInsertedApo = 0;
    SwPostItField aPostIt(
        (SwPostItFieldType*)rDoc.GetSysFldType(RES_POSTITFLD), sAuthor,
        sTxt, sInitials, sName, aDate );
    aPostIt.SetTextObject(pOutliner);

    // If this is a range, create the associated fieldmark.
    if (pPaM->HasMark())
    {
        IDocumentMarkAccess* pMarksAccess = rDoc.getIDocumentMarkAccess();
        pMarksAccess->makeFieldBookmark(*pPaM, aPostIt.GetName(), ODF_COMMENTRANGE);
        pPaM->Exchange();
        pPaM->DeleteMark();
    }

    pCtrlStck->NewAttr(*pPaM->GetPoint(), SvxCharHiddenItem(false, RES_CHRATR_HIDDEN));
    rDoc.InsertPoolItem(*pPaM, SwFmtFld(aPostIt), 0);
    pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_CHRATR_HIDDEN);

    return 0;
}

void SwWW8ImplReader::Read_HdFtTextAsHackedFrame(long nStart, long nLen,
    SwFrmFmt &rHdFtFmt, sal_uInt16 nPageWidth)
{
    const SwNodeIndex* pSttIdx = rHdFtFmt.GetCntnt().GetCntntIdx();
    OSL_ENSURE(pSttIdx, "impossible");
    if (!pSttIdx)
        return;

    SwPosition aTmpPos(*pPaM->GetPoint());

    pPaM->GetPoint()->nNode = pSttIdx->GetIndex() + 1;
    pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);

    SwFlyFrmFmt *pFrame = rDoc.MakeFlySection(FLY_AT_PARA, pPaM->GetPoint());

    SwFmtAnchor aAnch( pFrame->GetAnchor() );
    aAnch.SetType( FLY_AT_PARA );
    pFrame->SetFmtAttr( aAnch );
    SwFmtFrmSize aSz(ATT_MIN_SIZE, nPageWidth, MINLAY);
    SwFrmSize eFrmSize = ATT_MIN_SIZE;
    if( eFrmSize != aSz.GetWidthSizeType() )
        aSz.SetWidthSizeType( eFrmSize );
    pFrame->SetFmtAttr(aSz);
    pFrame->SetFmtAttr(SwFmtSurround(SURROUND_THROUGHT));
    pFrame->SetFmtAttr(SwFmtHoriOrient(0, text::HoriOrientation::LEFT)); //iFOO

    // #i43427# - send frame for header/footer into background.
    pFrame->SetFmtAttr( SvxOpaqueItem( RES_OPAQUE, false ) );
    SdrObject* pFrmObj = CreateContactObject( pFrame );
    OSL_ENSURE( pFrmObj,
            "<SwWW8ImplReader::Read_HdFtTextAsHackedFrame(..)> - missing SdrObject instance" );
    if ( pFrmObj )
    {
        pFrmObj->SetOrdNum( 0L );
    }
    MoveInsideFly(pFrame);

    const SwNodeIndex* pHackIdx = pFrame->GetCntnt().GetCntntIdx();

    Read_HdFtFtnText(pHackIdx, nStart, nLen - 1, MAN_HDFT);

    MoveOutsideFly(pFrame, aTmpPos);
}

void SwWW8ImplReader::Read_HdFtText(long nStart, long nLen, SwFrmFmt* pHdFtFmt)
{
    const SwNodeIndex* pSttIdx = pHdFtFmt->GetCntnt().GetCntntIdx();
    if (!pSttIdx)
        return;

    SwPosition aTmpPos( *pPaM->GetPoint() ); // Remember old cursor position

    Read_HdFtFtnText(pSttIdx, nStart, nLen - 1, MAN_HDFT);

    *pPaM->GetPoint() = aTmpPos;
}


bool SwWW8ImplReader::isValid_HdFt_CP(WW8_CP nHeaderCP) const
{
    // Each CP of Plcfhdd MUST be less than FibRgLw97.ccpHdd
    return (nHeaderCP < pWwFib->ccpHdr) ? true : false;
}

bool SwWW8ImplReader::HasOwnHeaderFooter(sal_uInt8 nWhichItems, sal_uInt8 grpfIhdt,
    int nSect)
{
    if (pHdFt)
    {
        WW8_CP start;
        long nLen;
        sal_uInt8 nNumber = 5;

        for( sal_uInt8 nI = 0x20; nI; nI >>= 1, nNumber-- )
        {
            if (nI & nWhichItems)
            {
                bool bOk = true;
                if( bVer67 )
                    bOk = ( pHdFt->GetTextPos(grpfIhdt, nI, start, nLen ) && nLen >= 2 );
                else
                {
                    pHdFt->GetTextPosExact( static_cast< short >(nNumber + (nSect+1)*6), start, nLen);
                    bOk = ( 2 <= nLen ) && isValid_HdFt_CP(start);
                }

                if (bOk)
                    return true;
            }
        }
    }
    return false;
}

void SwWW8ImplReader::Read_HdFt(int nSect, const SwPageDesc *pPrev,
    const wwSection &rSection)
{
    sal_uInt8 grpfIhdt = rSection.maSep.grpfIhdt;
    SwPageDesc *pPD = rSection.mpPage;

    if( pHdFt )
    {
        WW8_CP start;
        long nLen;
        sal_uInt8 nNumber = 5;

        for( sal_uInt8 nI = 0x20; nI; nI >>= 1, nNumber-- )
        {
            if (nI & grpfIhdt)
            {
                bool bOk = true;
                if( bVer67 )
                    bOk = ( pHdFt->GetTextPos(grpfIhdt, nI, start, nLen ) && nLen >= 2 );
                else
                {
                    pHdFt->GetTextPosExact( static_cast< short >(nNumber + (nSect+1)*6), start, nLen);
                    bOk = ( 2 <= nLen ) && isValid_HdFt_CP(start);
                }

                bool bUseLeft
                    = (nI & ( WW8_HEADER_EVEN | WW8_FOOTER_EVEN )) ? true: false;
                bool bUseFirst
                    = (nI & ( WW8_HEADER_FIRST | WW8_FOOTER_FIRST )) ? true: false;
                bool bFooter
                    = (nI & ( WW8_FOOTER_EVEN | WW8_FOOTER_ODD | WW8_FOOTER_FIRST )) ? true: false;

                SwFrmFmt& rFmt = bUseLeft ? pPD->GetLeft()
                    : bUseFirst ? pPD->GetFirst()
                    : pPD->GetMaster();

                SwFrmFmt* pHdFtFmt;
                // If we have empty first page header and footer.
                bool bNoFirst = !(grpfIhdt & WW8_HEADER_FIRST) && !(grpfIhdt & WW8_FOOTER_FIRST);
                if (bFooter)
                {
                    bIsFooter = true;
                    //#i17196# Cannot have left without right
                    if (!pPD->GetMaster().GetFooter().GetFooterFmt())
                        pPD->GetMaster().SetFmtAttr(SwFmtFooter(true));
                    if (bUseLeft)
                        pPD->GetLeft().SetFmtAttr(SwFmtFooter(true));
                    if (bUseFirst || (rSection.maSep.fTitlePage && bNoFirst))
                        pPD->GetFirst().SetFmtAttr(SwFmtFooter(true));
                    pHdFtFmt = const_cast<SwFrmFmt*>(rFmt.GetFooter().GetFooterFmt());
                }
                else
                {
                    bIsHeader = true;
                    //#i17196# Cannot have left without right
                    if (!pPD->GetMaster().GetHeader().GetHeaderFmt())
                        pPD->GetMaster().SetFmtAttr(SwFmtHeader(true));
                    if (bUseLeft)
                        pPD->GetLeft().SetFmtAttr(SwFmtHeader(true));
                    if (bUseFirst || (rSection.maSep.fTitlePage && bNoFirst))
                        pPD->GetFirst().SetFmtAttr(SwFmtHeader(true));
                    pHdFtFmt = const_cast<SwFrmFmt*>(rFmt.GetHeader().GetHeaderFmt());
                }

                if (bOk)
                {
                    bool bHackRequired = false;
                    if (bIsHeader && rSection.IsFixedHeightHeader())
                        bHackRequired = true;
                    else if (bIsFooter && rSection.IsFixedHeightFooter())
                        bHackRequired = true;

                    if (bHackRequired)
                    {
                        Read_HdFtTextAsHackedFrame(start, nLen, *pHdFtFmt,
                            static_cast< sal_uInt16 >(rSection.GetTextAreaWidth()) );
                    }
                    else
                        Read_HdFtText(start, nLen, pHdFtFmt);
                }
                else if (!bOk && pPrev)
                    CopyPageDescHdFt(pPrev, pPD, nI);

                bIsHeader = bIsFooter = false;
            }
        }
    }
}

bool wwSectionManager::SectionIsProtected(const wwSection &rSection) const
{
    return (mrReader.pWwFib->fReadOnlyRecommended && !rSection.IsNotProtected());
}

void wwSectionManager::SetHdFt(wwSection &rSection, int nSect,
    const wwSection *pPrevious)
{
    // Header/Footer not present
    if (!rSection.maSep.grpfIhdt)
        return;

    OSL_ENSURE(rSection.mpPage, "makes no sense to call with a main page");
    if (rSection.mpPage)
    {
        mrReader.Read_HdFt(nSect, pPrevious ? pPrevious->mpPage : 0,
                rSection);
    }

    // Header/Footer - Update Index
    // So that the index is still valid later on
    if (mrReader.pHdFt)
        mrReader.pHdFt->UpdateIndex(rSection.maSep.grpfIhdt);

}

class AttribHere : public std::unary_function<const xub_StrLen*, bool>
{
private:
    xub_StrLen nPosition;
public:
    AttribHere(xub_StrLen nPos) : nPosition(nPos) {}
    bool operator()(const xub_StrLen *pPosition) const
    {
        return (*pPosition >= nPosition);
    }
};

void SwWW8ImplReader::AppendTxtNode(SwPosition& rPos)
{
    SwTxtNode* pTxt = pPaM->GetNode()->GetTxtNode();

    const SwNumRule* pRule = NULL;

    if (pTxt != NULL)
        pRule = sw::util::GetNumRuleFromTxtNode(*pTxt);

    if (
         pRule && !pWDop->fDontUseHTMLAutoSpacing &&
         (bParaAutoBefore || bParaAutoAfter)
       )
    {
        // If after spacing is set to auto, set the after space to 0
        if (bParaAutoAfter)
            SetLowerSpacing(*pPaM, 0);

        // If the previous textnode had numbering and
        // and before spacing is set to auto, set before space to 0
        if(pPrevNumRule && bParaAutoBefore)
            SetUpperSpacing(*pPaM, 0);

        // If the previous numbering rule was different we need
        // to insert a space after the previous paragraph
        if((pRule != pPrevNumRule) && pPreviousNumPaM)
            SetLowerSpacing(*pPreviousNumPaM, GetParagraphAutoSpace(pWDop->fDontUseHTMLAutoSpacing));

        // cache current paragraph
        if(pPreviousNumPaM)
            delete pPreviousNumPaM, pPreviousNumPaM = 0;

        pPreviousNumPaM = new SwPaM(*pPaM);
        pPrevNumRule = pRule;
    }
    else if(!pRule && pPreviousNumPaM)
    {
        // If the previous paragraph has numbering but the current one does not
        // we need to add a space after the previous paragraph
        SetLowerSpacing(*pPreviousNumPaM, GetParagraphAutoSpace(pWDop->fDontUseHTMLAutoSpacing));
        delete pPreviousNumPaM, pPreviousNumPaM = 0;
        pPrevNumRule = 0;
    }
    else
    {
        // clear paragraph cache
        if(pPreviousNumPaM)
            delete pPreviousNumPaM, pPreviousNumPaM = 0;
        pPrevNumRule = pRule;
    }

    // If this is the first paragraph in the document and
    // Auto-spacing before paragraph is set,
    // set the upper spacing value to 0
    if(bParaAutoBefore && bFirstPara && !pWDop->fDontUseHTMLAutoSpacing)
        SetUpperSpacing(*pPaM, 0);

    bFirstPara = false;

    rDoc.AppendTxtNode(rPos);

    // We can flush all anchored graphics at the end of a paragraph.
    pAnchorStck->Flush();
}

bool SwWW8ImplReader::SetSpacing(SwPaM &rMyPam, int nSpace, bool bIsUpper )
{
        bool bRet = false;
        const SwPosition* pSpacingPos = rMyPam.GetPoint();

        const SvxULSpaceItem* pULSpaceItem = (const SvxULSpaceItem*)pCtrlStck->GetFmtAttr(*pSpacingPos, RES_UL_SPACE);

        if(pULSpaceItem != 0)
        {
            SvxULSpaceItem aUL(*pULSpaceItem);

            if(bIsUpper)
                aUL.SetUpper( static_cast< sal_uInt16 >(nSpace) );
            else
                aUL.SetLower( static_cast< sal_uInt16 >(nSpace) );

            xub_StrLen nEnd = pSpacingPos->nContent.GetIndex();
            rMyPam.GetPoint()->nContent.Assign(rMyPam.GetCntntNode(), 0);
            pCtrlStck->NewAttr(*pSpacingPos, aUL);
            rMyPam.GetPoint()->nContent.Assign(rMyPam.GetCntntNode(), nEnd);
            pCtrlStck->SetAttr(*pSpacingPos, RES_UL_SPACE);
            bRet = true;
        }
        return bRet;
}

bool SwWW8ImplReader::SetLowerSpacing(SwPaM &rMyPam, int nSpace)
{
    return SetSpacing(rMyPam, nSpace, false);
}

bool SwWW8ImplReader::SetUpperSpacing(SwPaM &rMyPam, int nSpace)
{
    return SetSpacing(rMyPam, nSpace, true);
}

sal_uInt16 SwWW8ImplReader::TabRowSprm(int nLevel) const
{
    if (bVer67)
        return 25;
    return nLevel ? 0x244C : 0x2417;
}

void SwWW8ImplReader::EndSpecial()
{
    // Frame/Table/Anl
    if (bAnl)
        StopAllAnl(); // -> bAnl = false

    while(maApos.size() > 1)
    {
        StopTable();
        maApos.pop_back();
        --nInTable;
        if (maApos[nInTable] == true)
            StopApo();
    }

    if (maApos[0] == true)
        StopApo();

    OSL_ENSURE(!nInTable, "unclosed table!");
}

bool SwWW8ImplReader::ProcessSpecial(bool &rbReSync, WW8_CP nStartCp)
{
    // Frame/Table/Anl
    if (bInHyperlink)
        return false;

    rbReSync = false;

    OSL_ENSURE(nInTable >= 0,"nInTable < 0!");

    // TabRowEnd
    bool bTableRowEnd = (pPlcxMan->HasParaSprm(bVer67 ? 25 : 0x2417) != 0 );

// Unfortunately, for every paragraph we need to check first whether
// they contain a sprm 29 (0x261B), which starts an APO.
// All other sprms then refer to that APO and not to the normal text
// surrounding it.
// The same holds true for a Table (sprm 24 (0x2416)) and Anls (sprm 13).
//
// WW: Table in APO is possible (Both Start-Ends occur at the same time)
// WW: APO in Table not possible
//
// This mean that of a Table is the content of a APO, the APO start needs
// to be edited first, so that the Table remains in the APO and not the
// other way around.
// At the End, however, we need to edit the Table End first as the APO
// must end after that Table (or else we never find the APO End).
//
// The same holds true for Fly / Anl, Tab / Anl, Fly / Tab / Anl.
//
// If the Table is within an APO the TabRowEnd Area misses the
// APO settings.
// To not end the APO there, we do not call ProcessApo

// KHZ: When there is a table inside the Apo the Apo-flags are also
//      missing for the 2nd, 3rd... paragraphs of each cell.

//  1st look for in-table flag, for 2000+ there is a subtable flag to
//  be considered, the sprm 6649 gives the level of the table
    sal_uInt8 nCellLevel = 0;

    if (bVer67)
        nCellLevel = 0 != pPlcxMan->HasParaSprm(24);
    else
    {
        nCellLevel = 0 != pPlcxMan->HasParaSprm(0x2416);
        if (!nCellLevel)
            nCellLevel = 0 != pPlcxMan->HasParaSprm(0x244B);
    }
    do
    {
        WW8_TablePos *pTabPos=0;
        WW8_TablePos aTabPos;
        if(nCellLevel && !bVer67)
        {
            WW8PLCFxSave1 aSave;
            pPlcxMan->GetPap()->Save( aSave );
            rbReSync = true;
            WW8PLCFx_Cp_FKP* pPap = pPlcxMan->GetPapPLCF();
            WW8_CP nMyStartCp=nStartCp;

            if (const sal_uInt8 *pLevel = pPlcxMan->HasParaSprm(0x6649))
                nCellLevel = *pLevel;

            bool bHasRowEnd = SearchRowEnd(pPap, nMyStartCp, (nInTable<nCellLevel?nInTable:nCellLevel-1));

            // Bad Table, remain unchanged in level, e.g. #i19667#
            if (!bHasRowEnd)
                nCellLevel = static_cast< sal_uInt8 >(nInTable);

            if (bHasRowEnd && ParseTabPos(&aTabPos,pPap))
                pTabPos = &aTabPos;

            pPlcxMan->GetPap()->Restore( aSave );
        }

        // Then look if we are in an Apo

        ApoTestResults aApo = TestApo(nCellLevel, bTableRowEnd, pTabPos);

        // Look to see if we are in a Table, but Table in foot/end note not allowed
        bool bStartTab = (nInTable < nCellLevel) && !bFtnEdn;

        bool bStopTab = bWasTabRowEnd && (nInTable > nCellLevel) && !bFtnEdn;

        bWasTabRowEnd = false;  // must be deactivated right here to prevent next
                                // WW8TabDesc::TableCellEnd() from making nonsense

        if (nInTable && !bTableRowEnd && !bStopTab && (nInTable == nCellLevel && aApo.HasStartStop()))
            bStopTab = bStartTab = true; // Required to stop and start table

        //  Test for Anl (Numbering) and process all events in the right order
        if( bAnl && !bTableRowEnd )
        {
            const sal_uInt8* pSprm13 = pPlcxMan->HasParaSprm( 13 );
            if( pSprm13 )
            {   // Still Anl left?
                sal_uInt8 nT = static_cast< sal_uInt8 >(GetNumType( *pSprm13 ));
                if( ( nT != WW8_Pause && nT != nWwNumType ) // Anl change
                    || aApo.HasStartStop()                  // Forced Anl end
                    || bStopTab || bStartTab )
                {
                    StopAnlToRestart(nT);  // Anl-Restart (= change) over sprms
                }
                else
                {
                    NextAnlLine( pSprm13 ); // Next Anl Line
                }
            }
            else
            {   // Regular Anl end
                StopAllAnl(); // Actual end
            }
        }
        if (bStopTab)
        {
            StopTable();
            maApos.pop_back();
            --nInTable;
        }
        if (aApo.mbStopApo)
        {
            StopApo();
            maApos[nInTable] = false;
        }

        if (aApo.mbStartApo)
        {
            maApos[nInTable] = StartApo(aApo, pTabPos);
            // We need an ReSync after StartApo
            // (actually only if the Apo extends past a FKP border)
            rbReSync = true;
        }
        if (bStartTab)
        {
            WW8PLCFxSave1 aSave;
            pPlcxMan->GetPap()->Save( aSave );

           // Numbering for cell borders causes a crash -> no Anls in Tables
           if (bAnl)
               StopAllAnl();

            if(nInTable < nCellLevel)
            {
                if (StartTable(nStartCp))
                    ++nInTable;
                else
                    break;
                maApos.push_back(false);
            }

            if(nInTable >= nCellLevel)
            {
                // We need an ReSync after StartTable
                // (actually only if the Apo extends past a FKP border)
                rbReSync = true;
                pPlcxMan->GetPap()->Restore( aSave );
            }
        }
    } while (nInTable < nCellLevel);
    return bTableRowEnd;
}

CharSet SwWW8ImplReader::GetCurrentCharSet()
{
    /*
    #i2015
    If the hard charset is set use it, if not see if there is an open
    character run that has set the charset, if not then fallback to the
    current underlying paragraph style.
    */
    CharSet eSrcCharSet = eHardCharSet;
    if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
    {
        if (!maFontSrcCharSets.empty())
            eSrcCharSet = maFontSrcCharSets.top();
        if ((eSrcCharSet == RTL_TEXTENCODING_DONTKNOW) && nCharFmt >= 0 && (size_t)nCharFmt < vColl.size() )
            eSrcCharSet = vColl[nCharFmt].GetCharSet();
        if ((eSrcCharSet == RTL_TEXTENCODING_DONTKNOW) && StyleExists(nAktColl) && nAktColl < vColl.size())
            eSrcCharSet = vColl[nAktColl].GetCharSet();
        if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
        {
            /*
             #i22206#/#i52786#
             The (default) character set used for a run of text is the default
             character set for the version of Word that last saved the document.

             This is a bit tentative, more might be required if the concept is correct.
             When later version of word write older 6/95 documents the charset is
             correctly set in the character runs involved, so its hard to reproduce
             documents that require this to be sure of the process involved.
            */
            const SvxLanguageItem *pLang = (const SvxLanguageItem*)GetFmtAttr(RES_CHRATR_LANGUAGE);
            LanguageType eLang = pLang ? pLang->GetLanguage() : LANGUAGE_SYSTEM;
            ::com::sun::star::lang::Locale aLocale(LanguageTag::convertToLocale(eLang));
            eSrcCharSet = msfilter::util::getBestTextEncodingFromLocale(aLocale);
        }
    }
    return eSrcCharSet;
}

//Takashi Ono for CJK
CharSet SwWW8ImplReader::GetCurrentCJKCharSet()
{
    /*
    #i2015
    If the hard charset is set use it, if not see if there is an open
    character run that has set the charset, if not then fallback to the
    current underlying paragraph style.
    */
    CharSet eSrcCharSet = eHardCharSet;
    if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
    {
        if (!maFontSrcCJKCharSets.empty())
            eSrcCharSet = maFontSrcCJKCharSets.top();
        if (!vColl.empty())
        {
            if ((eSrcCharSet == RTL_TEXTENCODING_DONTKNOW) && nCharFmt >= 0 && (size_t)nCharFmt < vColl.size() )
                eSrcCharSet = vColl[nCharFmt].GetCJKCharSet();
            if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW && nAktColl < vColl.size())
                eSrcCharSet = vColl[nAktColl].GetCJKCharSet();
        }
        if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
        { // patch from cmc for #i52786#
            /*
             #i22206#/#i52786#
             The (default) character set used for a run of text is the default
             character set for the version of Word that last saved the document.

             This is a bit tentative, more might be required if the concept is correct.
             When later version of word write older 6/95 documents the charset is
             correctly set in the character runs involved, so its hard to reproduce
             documents that require this to be sure of the process involved.
            */
            const SvxLanguageItem *pLang =
                (const SvxLanguageItem*)GetFmtAttr(RES_CHRATR_LANGUAGE);
            if (pLang)
            {
                switch (pLang->GetLanguage())
                {
                    case LANGUAGE_CZECH:
                        eSrcCharSet = RTL_TEXTENCODING_MS_1250;
                        break;
                    default:
                        eSrcCharSet = RTL_TEXTENCODING_MS_1252;
                        break;
                }
            }
        }
    }
    return eSrcCharSet;
}

void SwWW8ImplReader::PostProcessAttrs()
{
    if (mpPostProcessAttrsInfo != NULL)
    {
        SfxItemIter aIter(mpPostProcessAttrsInfo->mItemSet);

        const SfxPoolItem * pItem = aIter.GetCurItem();
        if (pItem != NULL)
        {
            do
            {
                pCtrlStck->NewAttr(*mpPostProcessAttrsInfo->mPaM.GetPoint(),
                                   *pItem);
                pCtrlStck->SetAttr(*mpPostProcessAttrsInfo->mPaM.GetMark(),
                                   pItem->Which(), true);
            }
            while (!aIter.IsAtEnd() && 0 != (pItem = aIter.NextItem()));
        }

        delete mpPostProcessAttrsInfo;
        mpPostProcessAttrsInfo = NULL;
    }
}

/*
 #i9241#
 It appears that some documents that are in a baltic 8 bit encoding which has
 some undefined characters can have use made of those characters, in which
 case they default to CP1252. If not then its perhaps that the font encoding
 is only in use for 6/7 and for 8+ if we are in 8bit mode then the encoding
 is always 1252.

 So a encoding converter that on an undefined character attempts to
 convert from 1252 on the undefined character
*/
sal_Size Custom8BitToUnicode(rtl_TextToUnicodeConverter hConverter,
    sal_Char *pIn, sal_Size nInLen, sal_Unicode *pOut, sal_Size nOutLen)
{
    const sal_uInt32 nFlags =
        RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR |
        RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR |
        RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE |
        RTL_TEXTTOUNICODE_FLAGS_FLUSH;

    const sal_uInt32 nFlags2 =
        RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE |
        RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_IGNORE |
        RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE |
        RTL_TEXTTOUNICODE_FLAGS_FLUSH;

    sal_Size nDestChars=0;
    sal_Size nConverted=0;

    do
    {
        sal_uInt32 nInfo = 0;
        sal_Size nThisConverted=0;

        nDestChars += rtl_convertTextToUnicode(hConverter, 0,
            pIn+nConverted, nInLen-nConverted,
            pOut+nDestChars, nOutLen-nDestChars,
            nFlags, &nInfo, &nThisConverted);

        OSL_ENSURE(nInfo == 0, "A character conversion failed!");

        nConverted += nThisConverted;

        if (
            nInfo & RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR ||
            nInfo & RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
           )
        {
            sal_Size nOtherConverted;
            rtl_TextToUnicodeConverter hCP1252Converter =
                rtl_createTextToUnicodeConverter(RTL_TEXTENCODING_MS_1252);
            nDestChars += rtl_convertTextToUnicode(hCP1252Converter, 0,
                pIn+nConverted, 1,
                pOut+nDestChars, nOutLen-nDestChars,
                nFlags2, &nInfo, &nOtherConverted);
            rtl_destroyTextToUnicodeConverter(hCP1252Converter);
            nConverted+=1;
        }
    } while (nConverted < nInLen);

    return nDestChars;
}

bool SwWW8ImplReader::LangUsesHindiNumbers(sal_uInt16 nLang)
{
    bool bResult = false;

    switch (nLang)
    {
        case 0x1401: // Arabic(Algeria)
        case 0x3c01: // Arabic(Bahrain)
        case 0xc01: // Arabic(Egypt)
        case 0x801: // Arabic(Iraq)
        case 0x2c01: // Arabic (Jordan)
        case 0x3401: // Arabic(Kuwait)
        case 0x3001: // Arabic(Lebanon)
        case 0x1001: // Arabic(Libya)
        case 0x1801: // Arabic(Morocco)
        case 0x2001: // Arabic(Oman)
        case 0x4001: // Arabic(Qatar)
        case 0x401: // Arabic(Saudi Arabia)
        case 0x2801: // Arabic(Syria)
        case 0x1c01: // Arabic(Tunisia)
        case 0x3801: // Arabic(U.A.E)
        case 0x2401: // Arabic(Yemen)
            bResult = true;
            break;
        default:
            break;
    }

    return bResult;
}

sal_Unicode SwWW8ImplReader::TranslateToHindiNumbers(sal_Unicode nChar)
{
    if (nChar >= 0x0030 && nChar <= 0x0039)
        return nChar + 0x0630;

    return nChar;
}

/**
 * Return value: true for non special chars
 */
bool SwWW8ImplReader::ReadPlainChars(WW8_CP& rPos, long nEnd, long nCpOfs)
{
    sal_Size nRequestedStrLen = nEnd - rPos;

    OSL_ENSURE(nRequestedStrLen, "String is 0");
    if (!nRequestedStrLen)
        return true;

    sal_Size nRequestedPos = pSBase->WW8Cp2Fc(nCpOfs+rPos, &bIsUnicode);
    bool bValidPos = checkSeek(*pStrm, nRequestedPos);
    OSL_ENSURE(bValidPos, "Document claimed to have more text than available");
    if (!bValidPos)
    {
        // Swallow missing range, e.g. #i95550#
        rPos+=nRequestedStrLen;
        return true;
    }

    sal_Size nAvailableStrLen = pStrm->remainingSize() / (bIsUnicode ? 2 : 1);
    OSL_ENSURE(nAvailableStrLen, "Document claimed to have more text than available");
    if (!nAvailableStrLen)
    {
        // Swallow missing range, e.g. #i95550#
        rPos+=nRequestedStrLen;
        return true;
    }

    sal_Size nValidStrLen = std::min(nRequestedStrLen, nAvailableStrLen);

    // Reset Unicode flag and correct FilePos if needed.
    // Note: Seek is not expensive, as we're checking inline whether or not
    // the correct FilePos has already been reached.
    xub_StrLen nStrLen;
    if (nValidStrLen <= (STRING_MAXLEN-1))
        nStrLen = writer_cast<xub_StrLen>(nValidStrLen);
    else
        nStrLen = STRING_MAXLEN-1;

    const CharSet eSrcCharSet = bVer67 ? GetCurrentCharSet() :
        RTL_TEXTENCODING_MS_1252;
    const CharSet eSrcCJKCharSet = bVer67 ? GetCurrentCJKCharSet() :
        RTL_TEXTENCODING_MS_1252;

    // allocate unicode string data
    rtl_uString *pStr = rtl_uString_alloc(nStrLen);
    sal_Unicode* pBuffer = pStr->buffer;
    sal_Unicode* pWork = pBuffer;

    sal_Char* p8Bits = NULL;

    rtl_TextToUnicodeConverter hConverter = 0;
    if (!bIsUnicode || bVer67)
        hConverter = rtl_createTextToUnicodeConverter(eSrcCharSet);

    if (!bIsUnicode)
        p8Bits = new sal_Char[nStrLen];

    // read the stream data
    sal_uInt8   nBCode = 0;
    sal_uInt16 nUCode;
    xub_StrLen nL2;

    sal_uInt16 nCTLLang = 0;
    const SfxPoolItem * pItem = GetFmtAttr(RES_CHRATR_CTL_LANGUAGE);
    if (pItem != NULL)
        nCTLLang = dynamic_cast<const SvxLanguageItem *>(pItem)->GetLanguage();

    for( nL2 = 0; nL2 < nStrLen; ++nL2, ++pWork )
    {
        if (bIsUnicode)
            *pStrm >> nUCode; // unicode  --> read 2 bytes
        else
        {
            *pStrm >> nBCode; // old code --> read 1 byte
            nUCode = nBCode;
        }

        if (pStrm->GetError())
        {
            rPos = WW8_CP_MAX-10; // -> eof or other error
            rtl_freeMemory(pStr);
            delete [] p8Bits;
            return true;
        }

        if ((32 > nUCode) || (0xa0 == nUCode))
        {
            pStrm->SeekRel( bIsUnicode ? -2 : -1 );
            break; // Special character < 32, == 0xa0 found
        }

        if (bIsUnicode)
        {
            if (!bVer67)
                *pWork = nUCode;
            else
            {
                if (nUCode >= 0x3000) //0x8000 ?
                {
                    sal_Char aTest[2];
                    aTest[0] = static_cast< sal_Char >((nUCode & 0xFF00) >> 8);
                    aTest[1] = static_cast< sal_Char >(nUCode & 0x00FF);
                    String aTemp(aTest, 2, eSrcCJKCharSet);
                    OSL_ENSURE(aTemp.Len() == 1, "so much for that theory");
                    *pWork = aTemp.GetChar(0);
                }
                else
                {
                    sal_Char cTest = static_cast< sal_Char >(nUCode & 0x00FF);
                    Custom8BitToUnicode(hConverter, &cTest, 1, pWork, 1);
                }
            }
        }
        else
            p8Bits[nL2] = nBCode;
    }

    if (nL2)
    {
        xub_StrLen nEndUsed = nL2;

        if (!bIsUnicode)
            nEndUsed = Custom8BitToUnicode(hConverter, p8Bits, nL2, pBuffer, nStrLen);

        for( xub_StrLen nI = 0; nI < nStrLen; ++nI, ++pBuffer )
            if (m_bRegardHindiDigits && bBidi && LangUsesHindiNumbers(nCTLLang))
                *pBuffer = TranslateToHindiNumbers(*pBuffer);

        pStr->buffer[nEndUsed] = 0;
        pStr->length = nEndUsed;

        emulateMSWordAddTextToParagraph(OUString(pStr, SAL_NO_ACQUIRE));
        pStr = NULL;
        rPos += nL2;
        if (!maApos.back()) // a para end in apo doesn't count
            bWasParaEnd = false; // No CR
    }

    if (hConverter)
        rtl_destroyTextToUnicodeConverter(hConverter);
    if (pStr)
        rtl_uString_release(pStr);
    delete [] p8Bits;
    return nL2 >= nStrLen;
}

#define MSASCII SAL_MAX_INT16

namespace
{
    // We want to force weak chars inside 0x0020 to 0x007F to LATIN
    sal_Int16 lcl_getScriptType(
        const uno::Reference<i18n::XBreakIterator>& rBI,
        const OUString &rString, sal_Int32 nPos)
    {
        sal_Int16 nScript = rBI->getScriptType(rString, nPos);
        if (nScript == i18n::ScriptType::WEAK && rString[nPos] >= 0x0020 && rString[nPos] <= 0x007F)
            nScript = MSASCII;
        return nScript;
    }

    // We want to know about WEAK segments, so endOfScript isn't
    // useful, and see lcl_getScriptType anyway
    sal_Int32 lcl_endOfScript(
        const uno::Reference<i18n::XBreakIterator>& rBI,
        const OUString &rString, sal_Int32 nPos, sal_Int16 nScript)
    {
        while (nPos < rString.getLength())
        {
            sal_Int16 nNewScript = lcl_getScriptType(rBI, rString, nPos);
            if (nScript != nNewScript)
                break;
            ++nPos;
        }
        return nPos;
    }

    sal_Int32 lcl_getWriterScriptType(
        const uno::Reference<i18n::XBreakIterator>& rBI,
        const OUString &rString, sal_Int32 nPos)
    {
        sal_Int16 nScript = i18n::ScriptType::WEAK;

        if (rString.isEmpty())
            return nScript;

        while (nPos >= 0)
        {
            nScript = rBI->getScriptType(rString, nPos);
            if (nScript != i18n::ScriptType::WEAK)
                break;
            --nPos;
        }

        return nScript;
    }

    bool samePitchIgnoreUnknown(FontPitch eA, FontPitch eB)
    {
        return (eA == eB || eA == PITCH_DONTKNOW || eB == PITCH_DONTKNOW);
    }

    bool sameFontIgnoringIrrelevantFields(const SvxFontItem &rA, const SvxFontItem &rB)
    {
        // Ignoring CharSet, and ignoring unknown pitch
        return rA.GetFamilyName() == rB.GetFamilyName() &&
            rA.GetStyleName() == rB.GetStyleName() &&
            rA.GetFamily() == rB.GetFamily() &&
            samePitchIgnoreUnknown(rA.GetPitch(), rB.GetPitch());
    }
}

// In writer we categorize text into CJK, CTL and "Western" for everything else.
// Microsoft Word basically categorizes text into East Asian, Complex, ASCII,
// NonEastAsian/HighAnsi, with some shared characters and some properties to
// hint as to which way to bias those shared characters.
//
// That's four categories, we however have three categories. Given that problem
// here we would ideally find out "what would word do" to see what font/language
// word would assign to characters based on the unicode range they fall into and
// hack the word one onto the range we use. However it's unclear what word's
// categorization is. So we don't do that here yet.
//
// Additional to the categorization, when word encounters weak text for ambigious
// chars it uses idcthint to indicate which way to bias. We don't have a idcthint
// feature in writer.
//
// So what we currently do here then is to split our text into non-weak/weak
// sections and uses word's idcthint to determine what font it would use and
// force that on for the segment. Following what we *do* know about word's
// categorization, we know that the range 0x0020 and 0x007F is sprmCRgFtc0 in
// word, something we map to LATIN, so we consider all weaks chars in that range
// to auto-bias to LATIN.
//
// See https://bugs.freedesktop.org/show_bug.cgi?id=34319 for an example
void SwWW8ImplReader::emulateMSWordAddTextToParagraph(const OUString& rAddString)
{
    if (rAddString.isEmpty())
        return;

    uno::Reference<i18n::XBreakIterator> xBI(g_pBreakIt->GetBreakIter());
    if (!xBI.is())
    {
        simpleAddTextToParagraph(rAddString);
        return;
    }

    sal_Int16 nScript = lcl_getScriptType(xBI, rAddString, 0);
    sal_Int32 nLen = rAddString.getLength();

    OUString sParagraphText;
    const SwCntntNode *pCntNd = pPaM->GetCntntNode();
    const SwTxtNode* pNd = pCntNd ? pCntNd->GetTxtNode() : NULL;
    if (pNd)
        sParagraphText = pNd->GetTxt();
    sal_Int32 nParaOffset = sParagraphText.getLength();
    sParagraphText = sParagraphText + rAddString;

    sal_Int32 nPos = 0;
    while (nPos < nLen)
    {
        sal_Int32 nEnd = lcl_endOfScript(xBI, rAddString, nPos, nScript);
        if (nEnd < 0)
            break;

        OUString sChunk(rAddString.copy(nPos, nEnd-nPos));
        const sal_uInt16 aIds[] = {RES_CHRATR_FONT, RES_CHRATR_CJK_FONT, RES_CHRATR_CTL_FONT};
        const SvxFontItem *pOverriddenItems[] = {NULL, NULL, NULL};
        bool aForced[] = {false, false, false};

        int nLclIdctHint = 0xFF;
        if (nScript == i18n::ScriptType::WEAK)
            nLclIdctHint = nIdctHint;
        else if (nScript == MSASCII) // Force weak chars in ascii range to use LATIN font
            nLclIdctHint = 0;

        sal_uInt16 nForceFromFontId = 0;
        if (nLclIdctHint != 0xFF)
        {
            switch (nLclIdctHint)
            {
                case 0:
                    nForceFromFontId = RES_CHRATR_FONT;
                    break;
                case 1:
                    nForceFromFontId = RES_CHRATR_CJK_FONT;
                    break;
                case 2:
                    nForceFromFontId = RES_CHRATR_CTL_FONT;
                    break;
                default:
                    break;
            }
        }

        if (nForceFromFontId != 0)
        {
            // Now we know that word would use the nForceFromFontId font for this range
            // Try and determine what script writer would assign this range to

            sal_Int32 nWriterScript = lcl_getWriterScriptType(xBI, sParagraphText,
                nPos + nParaOffset);

            bool bWriterWillUseSameFontAsWordAutomatically = false;

            if (nWriterScript != i18n::ScriptType::WEAK)
            {
                if (
                     (nWriterScript == i18n::ScriptType::ASIAN && nForceFromFontId == RES_CHRATR_CJK_FONT) ||
                     (nWriterScript == i18n::ScriptType::COMPLEX && nForceFromFontId == RES_CHRATR_CTL_FONT) ||
                     (nWriterScript == i18n::ScriptType::LATIN && nForceFromFontId == RES_CHRATR_FONT)
                   )
                {
                    bWriterWillUseSameFontAsWordAutomatically = true;
                }
                else
                {
                    const SvxFontItem *pSourceFont = (const SvxFontItem*)GetFmtAttr(nForceFromFontId);
                    sal_uInt16 nDestId = aIds[nWriterScript-1];
                    const SvxFontItem *pDestFont = (const SvxFontItem*)GetFmtAttr(nDestId);
                    bWriterWillUseSameFontAsWordAutomatically = sameFontIgnoringIrrelevantFields(*pSourceFont, *pDestFont);
                }
            }

            // Writer won't use the same font as word, so force the issue
            if (!bWriterWillUseSameFontAsWordAutomatically)
            {
                const SvxFontItem *pSourceFont = (const SvxFontItem*)GetFmtAttr(nForceFromFontId);

                for (size_t i = 0; i < SAL_N_ELEMENTS(aIds); ++i)
                {
                    const SvxFontItem *pDestFont = (const SvxFontItem*)GetFmtAttr(aIds[i]);
                    aForced[i] = aIds[i] != nForceFromFontId && *pSourceFont != *pDestFont;
                    if (aForced[i])
                    {
                        pOverriddenItems[i] =
                            (const SvxFontItem*)pCtrlStck->GetStackAttr(*pPaM->GetPoint(), aIds[i]);

                        SvxFontItem aForceFont(*pSourceFont);
                        aForceFont.SetWhich(aIds[i]);
                        pCtrlStck->NewAttr(*pPaM->GetPoint(), aForceFont);
                    }
                }
            }
        }

        simpleAddTextToParagraph(sChunk);

        for (size_t i = 0; i < SAL_N_ELEMENTS(aIds); ++i)
        {
            if (aForced[i])
            {
                pCtrlStck->SetAttr(*pPaM->GetPoint(), aIds[i]);
                if (pOverriddenItems[i])
                    pCtrlStck->NewAttr(*pPaM->GetPoint(), *(pOverriddenItems[i]));
            }
        }

        nPos = nEnd;
        if (nPos < nLen)
            nScript = lcl_getScriptType(xBI, rAddString, nPos);
    }
}

void SwWW8ImplReader::simpleAddTextToParagraph(const String& rAddString)
{
    if (!rAddString.Len())
        return;

#if OSL_DEBUG_LEVEL > 1
        {
            OString sText(OUStringToOString(rAddString, RTL_TEXTENCODING_UTF8));
            SAL_INFO("sw.ww8", "<addTextToParagraph>" << sText.getStr() << "</addTextToParagraph>");
        }
#endif

    const SwCntntNode *pCntNd = pPaM->GetCntntNode();
    const SwTxtNode* pNd = pCntNd ? pCntNd->GetTxtNode() : NULL;

    OSL_ENSURE(pNd, "What the hell, where's my text node");

    if (!pNd)
        return;

    if ((pNd->GetTxt().getLength() + rAddString.Len()) < STRING_MAXLEN-1)
    {
        rDoc.InsertString(*pPaM, rAddString);
    }
    else
    {

        if (pNd->GetTxt().getLength() < STRING_MAXLEN -1)
        {
            String sTempStr (rAddString,0,
                STRING_MAXLEN - pNd->GetTxt().getLength() -1);
            rDoc.InsertString(*pPaM, sTempStr);
            sTempStr = rAddString.Copy(sTempStr.Len(),
                rAddString.Len() - sTempStr.Len());
            AppendTxtNode(*pPaM->GetPoint());
            rDoc.InsertString(*pPaM, sTempStr);
        }
        else
        {
            AppendTxtNode(*pPaM->GetPoint());
            rDoc.InsertString(*pPaM, rAddString);
        }
    }

    bReadTable = false;
}

/**
 * Return value: true for para end
 */
bool SwWW8ImplReader::ReadChars(WW8_CP& rPos, WW8_CP nNextAttr, long nTextEnd,
    long nCpOfs)
{
    long nEnd = ( nNextAttr < nTextEnd ) ? nNextAttr : nTextEnd;

    if (bSymbol || bIgnoreText)
    {
        if( bSymbol ) // Insert special chars
        {
            for(sal_uInt16 nCh = 0; nCh < nEnd - rPos; ++nCh)
            {
                rDoc.InsertString( *pPaM, OUString(cSymbol) );
            }
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_FONT );
        }
        pStrm->SeekRel( nEnd- rPos );
        rPos = nEnd; // Ignore until attribute end
        return false;
    }

    while (true)
    {
        if (ReadPlainChars(rPos, nEnd, nCpOfs))
            return false; // Done

        bool bStartLine = ReadChar(rPos, nCpOfs);
        rPos++;
        if (bPgSecBreak || bStartLine || rPos == nEnd) // CR or Done
        {
            return bStartLine;
        }
    }
}

bool SwWW8ImplReader::HandlePageBreakChar()
{
    bool bParaEndAdded = false;
    // #i1909# section/page breaks should not occur in tables, word
    // itself ignores them in this case.
    if (!nInTable)
    {
        bool IsTemp=true;
        SwTxtNode* pTemp = pPaM->GetNode()->GetTxtNode();
        if (pTemp && pTemp->GetTxt().isEmpty()
                && (bFirstPara || bFirstParaOfPage))
        {
            IsTemp = false;
            AppendTxtNode(*pPaM->GetPoint());
            pTemp->SetAttr(*GetDfltAttr(RES_PARATR_NUMRULE));
        }

        bPgSecBreak = true;
        pCtrlStck->KillUnlockedAttrs(*pPaM->GetPoint());
        /*
        If its a 0x0c without a paragraph end before it, act like a
        paragraph end, but nevertheless, numbering (and perhaps other
        similar constructs) do not exist on the para.
        */
        if (!bWasParaEnd && IsTemp)
        {
            bParaEndAdded = true;
            if (0 >= pPaM->GetPoint()->nContent.GetIndex())
            {
                if (SwTxtNode* pTxtNode = pPaM->GetNode()->GetTxtNode())
                {
                    pTxtNode->SetAttr(
                        *GetDfltAttr(RES_PARATR_NUMRULE));
                }
            }
        }
    }
    return bParaEndAdded;
}

bool SwWW8ImplReader::ReadChar(long nPosCp, long nCpOfs)
{
    bool bNewParaEnd = false;
    // Reset Unicode flag and correct FilePos if needed.
    // Note: Seek is not expensive, as we're checking inline whether or not
    // the correct FilePos has already been reached.
    sal_Size nRequestedPos = pSBase->WW8Cp2Fc(nCpOfs+nPosCp, &bIsUnicode);
    if (!checkSeek(*pStrm, nRequestedPos))
        return false;

    sal_uInt8 nBCode(0);
    sal_uInt16 nWCharVal(0);
    if( bIsUnicode )
        *pStrm >> nWCharVal; // unicode  --> read 2 bytes
    else
    {
        *pStrm  >>  nBCode; // old code --> read 1 byte
        nWCharVal = nBCode;
    }

    sal_Unicode cInsert = '\x0';
    bool bRet = false;

    if ( 0xc != nWCharVal )
        bFirstParaOfPage = false;

    switch (nWCharVal)
    {
        case 0:
            {
                // Page number
                SwPageNumberField aFld(
                    (SwPageNumberFieldType*)rDoc.GetSysFldType(
                    RES_PAGENUMBERFLD ), PG_RANDOM, SVX_NUM_ARABIC);
                rDoc.InsertPoolItem(*pPaM, SwFmtFld(aFld), 0);
            }
            break;
        case 0xe:
            // if there is only one column word treats a column break like a pagebreak.
            if (maSectionManager.CurrentSectionColCount() < 2)
                bRet = HandlePageBreakChar();
            else if (!nInTable)
            {
                // Always insert a txtnode for a column break, e.g. ##
                SwCntntNode *pCntNd=pPaM->GetCntntNode();
                if (pCntNd!=NULL && pCntNd->Len()>0) // if par is empty not break is needed
                    AppendTxtNode(*pPaM->GetPoint());
                rDoc.InsertPoolItem(*pPaM, SvxFmtBreakItem(SVX_BREAK_COLUMN_BEFORE, RES_BREAK), 0);
            }
            break;
        case 0x7:
            bNewParaEnd = true;
            TabCellEnd();       // Table cell end (query flags!)
            break;
        case 0xf:
            if( !bSpec )        // "Satellite"
                cInsert = '\xa4';
            break;
        case 0x14:
            if( !bSpec )        // "Para End" char
                cInsert = '\xb5';
            break;
        case 0x15:
            if( !bSpec )        // Section sign
                cInsert = '\xa7';
            break;
        case 0x9:
            cInsert = '\x9';    // Tab
            break;
        case 0xb:
            cInsert = '\xa';    // Hard NewLine
            break;
        case 0xc:
            bRet = HandlePageBreakChar();
            break;
        case 0x1e:              // Non-breaking hyphen
            rDoc.InsertString( *pPaM, OUString(CHAR_HARDHYPHEN) );
            break;
        case 0x1f:              // Non-required hyphens
            rDoc.InsertString( *pPaM, OUString(CHAR_SOFTHYPHEN) );
            break;
        case 0xa0:              // Non-breaking spaces
            rDoc.InsertString( *pPaM, OUString(CHAR_HARDBLANK)  );
            break;
        case 0x1:
            /*
            Current thinking is that if bObj is set then we have a
            straightforward "traditional" ole object, otherwise we have a
            graphic preview of an associated ole2 object (or a simple
            graphic of course)

            normally in the canvas field, the code is 0x8 0x1.
            in a special case, the code is 0x1 0x1, which yields a simple picture
            */
            {
                bool bReadObj = IsInlineEscherHack();
                if( bReadObj )
                {
                    long nCurPos = pStrm->Tell();
                    sal_uInt16 nWordCode(0);

                    if( bIsUnicode )
                        *pStrm >> nWordCode;
                    else
                    {
                        sal_uInt8 nByteCode(0);
                        *pStrm >> nByteCode;
                        nWordCode = nByteCode;
                    }
                    if( nWordCode == 0x1 )
                        bReadObj = false;
                    pStrm->Seek( nCurPos );
                }
                if( !bReadObj )
                {
                    SwFrmFmt *pResult = 0;
                    if (bObj)
                        pResult = ImportOle();
                    else if (bSpec)
                        pResult = ImportGraf();

                    // If we have a bad 0x1 insert a space instead.
                    if (!pResult)
                    {
                        cInsert = ' ';
                        OSL_ENSURE(!bObj && !bEmbeddObj && !nObjLocFc,
                            "WW8: Please report this document, it may have a "
                            "missing graphic");
                    }
                    else
                    {
                        // reset the flags.
                        bObj = bEmbeddObj = false;
                        nObjLocFc = 0;
                    }
                }
            }
            break;
        case 0x8:
            if( !bObj )
                Read_GrafLayer( nPosCp );
            break;
        case 0xd:
            bNewParaEnd = bRet = true;
            if (nInTable > 1)
            {
                /*
                #i9666#/#i23161#
                Yes complex, if there is an entry in the undocumented PLCF
                which I believe to be a record of cell and row boundaries
                see if the magic bit which I believe to mean cell end is
                set. I also think btw that the third byte of the 4 byte
                value is the level of the cell
                */
                WW8PLCFspecial* pTest = pPlcxMan->GetMagicTables();
                if (pTest && pTest->SeekPosExact(nPosCp+1+nCpOfs) &&
                    pTest->Where() == nPosCp+1+nCpOfs)
                {
                    WW8_FC nPos;
                    void *pData;
                    pTest->Get(nPos, pData);
                    sal_uInt32 nData = SVBT32ToUInt32(*(SVBT32*)pData);
                    if (nData & 0x2) // Might be how it works
                    {
                        TabCellEnd();
                        bRet = false;
                    }
                }
                else if (bWasTabCellEnd)
                {
                    TabCellEnd();
                    bRet = false;
                }
            }

            bWasTabCellEnd = false;

            break;              // line end
        case 0x5:               // Annotation reference
        case 0x13:
            break;
        case 0x2:               // TODO: Auto-Footnote-Number, should be replaced by SwWW8ImplReader::End_Ftn later
            if (!maFtnStack.empty())
                cInsert = 0x2;
            break;
        default:
            SAL_INFO( "sw.ww8.level2", "<unknownValue val=\"" << nWCharVal << "\">" );
            break;
    }

    if( '\x0' != cInsert )
    {
        OUString sInsert(cInsert);
        emulateMSWordAddTextToParagraph(sInsert);
    }
    if (!maApos.back()) // a para end in apo doesn't count
        bWasParaEnd = bNewParaEnd;
    return bRet;
}

void SwWW8ImplReader::ProcessAktCollChange(WW8PLCFManResult& rRes,
    bool* pStartAttr, bool bCallProcessSpecial)
{
    sal_uInt16 nOldColl = nAktColl;
    nAktColl = pPlcxMan->GetColl();

    // Invalid Style-Id
    if (nAktColl >= vColl.size() || !vColl[nAktColl].pFmt || !vColl[nAktColl].bColl)
    {
        nAktColl = 0;
        bParaAutoBefore = false;
        bParaAutoAfter = false;
    }
    else
    {
        bParaAutoBefore = vColl[nAktColl].bParaAutoBefore;
        bParaAutoAfter = vColl[nAktColl].bParaAutoAfter;
    }

    if (nOldColl >= vColl.size())
        nOldColl = 0; // guess! TODO make sure this is what we want

    bool bTabRowEnd = false;
    if( pStartAttr && bCallProcessSpecial && !bInHyperlink )
    {
        bool bReSync;
        // Frame/Table/Autonumbering List Level
        bTabRowEnd = ProcessSpecial(bReSync, rRes.nAktCp+pPlcxMan->GetCpOfs());
        if( bReSync )
            *pStartAttr = pPlcxMan->Get( &rRes ); // Get Attribut-Pos again
    }

    if (!bTabRowEnd && StyleExists(nAktColl))
    {
        SetTxtFmtCollAndListLevel( *pPaM, vColl[ nAktColl ]);
        ChkToggleAttr(vColl[ nOldColl ].n81Flags, vColl[ nAktColl ].n81Flags);
        ChkToggleBiDiAttr(vColl[nOldColl].n81BiDiFlags,
            vColl[nAktColl].n81BiDiFlags);
    }
}

long SwWW8ImplReader::ReadTextAttr(WW8_CP& rTxtPos, bool& rbStartLine)
{
    long nSkipChars = 0;
    WW8PLCFManResult aRes;

    OSL_ENSURE(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
    bool bStartAttr = pPlcxMan->Get(&aRes); // Get Attribute position again
    aRes.nAktCp = rTxtPos;                  // Current Cp position

    bool bNewSection = (aRes.nFlags & MAN_MASK_NEW_SEP) && !bIgnoreText;
    if ( bNewSection ) // New Section
    {
        OSL_ENSURE(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
        // Create PageDesc and fill it
        maSectionManager.CreateSep(rTxtPos, bPgSecBreak);
        // -> 0xc was a Sectionbreak, but not a Pagebreak;
        // Create PageDesc and fill it
        bPgSecBreak = false;
        OSL_ENSURE(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
    }

    // New paragraph over Plcx.Fkp.papx
    if ( (aRes.nFlags & MAN_MASK_NEW_PAP)|| rbStartLine )
    {
        ProcessAktCollChange( aRes, &bStartAttr,
            MAN_MASK_NEW_PAP == (aRes.nFlags & MAN_MASK_NEW_PAP) &&
            !bIgnoreText );
        rbStartLine = false;
    }

    // position of last CP that's to be ignored
    long nSkipPos = -1;

    if( 0 < aRes.nSprmId ) // Ignore empty Attrs
    {
        if( ( eFTN > aRes.nSprmId ) || ( 0x0800 <= aRes.nSprmId ) )
        {
            if( bStartAttr ) // WW attributes
            {
                if( aRes.nMemLen >= 0 )
                    ImportSprm(aRes.pMemPos, aRes.nSprmId);
            }
            else
                EndSprm( aRes.nSprmId ); // Switch off Attr
        }
        else if( aRes.nSprmId < 0x800 ) // Own helper attributes
        {
            if (bStartAttr)
            {
                nSkipChars = ImportExtSprm(&aRes);
                if (
                    (aRes.nSprmId == eFTN) || (aRes.nSprmId == eEDN) ||
                    (aRes.nSprmId == eFLD) || (aRes.nSprmId == eAND)
                   )
                {
                    // Skip Field/Ftn-/End-Note here
                    rTxtPos += nSkipChars;
                    nSkipPos = rTxtPos-1;
                }
            }
            else
                EndExtSprm( aRes.nSprmId );
        }
    }

    pStrm->Seek(pSBase->WW8Cp2Fc( pPlcxMan->GetCpOfs() + rTxtPos, &bIsUnicode));

    // Find next Attr position (and Skip attributes of field contents if needed)
    if (nSkipChars && !bIgnoreText)
        pCtrlStck->MarkAllAttrsOld();
    bool bOldIgnoreText = bIgnoreText;
    bIgnoreText = true;
    sal_uInt16 nOldColl = nAktColl;
    bool bDoPlcxManPlusPLus = true;
    long nNext;
    do
    {
        if( bDoPlcxManPlusPLus )
            pPlcxMan->advance();
        nNext = pPlcxMan->Where();

        if (mpPostProcessAttrsInfo &&
            mpPostProcessAttrsInfo->mnCpStart == nNext)
        {
            mpPostProcessAttrsInfo->mbCopy = true;
        }

        if( (0 <= nNext) && (nSkipPos >= nNext) )
        {
            nNext = ReadTextAttr( rTxtPos, rbStartLine );
            bDoPlcxManPlusPLus = false;
            bIgnoreText = true;
        }

        if (mpPostProcessAttrsInfo &&
            nNext > mpPostProcessAttrsInfo->mnCpEnd)
        {
            mpPostProcessAttrsInfo->mbCopy = false;
        }
    }
    while( nSkipPos >= nNext );
    bIgnoreText    = bOldIgnoreText;
    if( nSkipChars )
    {
        pCtrlStck->KillUnlockedAttrs( *pPaM->GetPoint() );
        if( nOldColl != pPlcxMan->GetColl() )
            ProcessAktCollChange(aRes, 0, false);
    }

    return nNext;
}

//Revised 2012.8.16 for the complex attribute presentation of 0x0D in MS
bool SwWW8ImplReader::IsParaEndInCPs(sal_Int32 nStart, sal_Int32 nEnd,bool bSdOD) const
{
    //Revised for performance consideration
    if (nStart == -1 || nEnd == -1 || nEnd < nStart )
        return false;

    for (cp_vector::const_reverse_iterator aItr = maEndParaPos.rbegin(); aItr!= maEndParaPos.rend(); ++aItr)
    {
        //Revised 2012.8.16,to the 0x0D,the attribute will have two situations
        //*********within***********exact******//
        //*********but also sample with only left and the position of 0x0d is the edge of the right side***********//
        if ( bSdOD && ( (nStart < *aItr && nEnd > *aItr) || ( nStart == nEnd && *aItr == nStart)) )
            return true;
        else if ( !bSdOD &&  (nStart < *aItr && nEnd >= *aItr) )
            return true;
    }

    return false;
}

//Clear the para end position recorded in reader intermittently for the least impact on loading performance
void SwWW8ImplReader::ClearParaEndPosition()
{
    if ( maEndParaPos.size() > 0 )
        maEndParaPos.clear();
}

void SwWW8ImplReader::ReadAttrs(WW8_CP& rNext, WW8_CP& rTxtPos, bool& rbStartLine)
{
    // Dow we have attributes?
    if( rTxtPos >= rNext )
    {
        do
        {
            maCurrAttrCP = rTxtPos;
            rNext = ReadTextAttr( rTxtPos, rbStartLine );
        }
        while( rTxtPos >= rNext );

    }
    else if ( rbStartLine )
    {
    /* No attributes, but still a new line.
     * If a line ends with a line break and paragraph attributes or paragraph templates
     * do NOT change the line end was not added to the Plcx.Fkp.papx i.e. (nFlags & MAN_MASK_NEW_PAP)
     * is false.
     * Due to this we need to set the template here as a kind of special treatment.
     */
    if (!bCpxStyle && nAktColl < vColl.size())
            SetTxtFmtCollAndListLevel(*pPaM, vColl[nAktColl]);
        rbStartLine = false;
    }
}

/**
 * CloseAttrEnds to only read the attribute ends at the end of a text or a
 * text area (Header, Footnote, ...).
 * We ignore attribute starts and fields.
 */
void SwWW8ImplReader::CloseAttrEnds()
{
    // If there are any unclosed sprms then copy them to
    // another stack and close the ones that must be closed
    std::stack<sal_uInt16> aStack;
    pPlcxMan->TransferOpenSprms(aStack);

    while (!aStack.empty())
    {
        sal_uInt16 nSprmId = aStack.top();
        if ((0 < nSprmId) && (( eFTN > nSprmId) || (0x0800 <= nSprmId)))
            EndSprm(nSprmId);
        aStack.pop();
    }

    EndSpecial();
}

bool SwWW8ImplReader::ReadText(long nStartCp, long nTextLen, ManTypes nType)
{
    bool bJoined=false;

    bool bStartLine = true;
    short nCrCount = 0;
    short nDistance = 0;

    bWasParaEnd = false;
    nAktColl    =  0;
    pAktItemSet =  0;
    nCharFmt    = -1;
    bSpec = false;
    bPgSecBreak = false;

    pPlcxMan = new WW8PLCFMan( pSBase, nType, nStartCp );
    long nCpOfs = pPlcxMan->GetCpOfs(); // Offset for Header/Footer, Footnote

    WW8_CP nNext = pPlcxMan->Where();
    SwTxtNode* pPreviousNode = 0;
    sal_uInt8 nDropLines = 0;
    SwCharFmt* pNewSwCharFmt = 0;
    const SwCharFmt* pFmt = 0;
    pStrm->Seek( pSBase->WW8Cp2Fc( nStartCp + nCpOfs, &bIsUnicode ) );

    WW8_CP l = nStartCp;
    while ( l<nStartCp+nTextLen )
    {
        ReadAttrs( nNext, l, bStartLine );// Takes SectionBreaks into account, too
        OSL_ENSURE(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");

        if (mpPostProcessAttrsInfo != NULL)
            PostProcessAttrs();

        if( l>= nStartCp + nTextLen )
            break;

        bStartLine = ReadChars(l, nNext, nStartCp+nTextLen, nCpOfs);

        // If the previous paragraph was a dropcap then do not
        // create a new txtnode and join the two paragraphs together
        if (bStartLine && !pPreviousNode) // Line end
        {
            // We will record the CP of a paragraph end ('0x0D'), if current loading contents is from main stream;
            if (mbOnLoadingMain)
                maEndParaPos.push_back(l-1);
            AppendTxtNode(*pPaM->GetPoint());
        }

        if (pPreviousNode && bStartLine)
        {
            SwTxtNode* pEndNd = pPaM->GetNode()->GetTxtNode();
            const xub_StrLen nDropCapLen = pPreviousNode->GetTxt().getLength();

            // Need to reset the font size and text position for the dropcap
            {
                SwPaM aTmp(*pEndNd, 0, *pEndNd, nDropCapLen+1);
                pCtrlStck->Delete(aTmp);
            }

            // Get the default document dropcap which we can use as our template
            const SwFmtDrop* defaultDrop =
                (const SwFmtDrop*) GetFmtAttr(RES_PARATR_DROP);
            SwFmtDrop aDrop(*defaultDrop);

            aDrop.GetLines() = nDropLines;
            aDrop.GetDistance() = nDistance;
            aDrop.GetChars() = writer_cast<sal_uInt8>(nDropCapLen);
            // Word has no concept of a "whole word dropcap"
            aDrop.GetWholeWord() = false;

            if (pFmt)
                aDrop.SetCharFmt(const_cast<SwCharFmt*>(pFmt));
            else if(pNewSwCharFmt)
                aDrop.SetCharFmt(const_cast<SwCharFmt*>(pNewSwCharFmt));

            SwPosition aStart(*pEndNd);
            pCtrlStck->NewAttr(aStart, aDrop);
            pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_PARATR_DROP);
            pPreviousNode = 0;
        }
        else if (bDropCap)
        {
            // If we have found a dropcap store the textnode
            pPreviousNode = pPaM->GetNode()->GetTxtNode();

            const sal_uInt8 *pDCS;

            if (bVer67)
                pDCS = pPlcxMan->GetPapPLCF()->HasSprm(46);
            else
                pDCS = pPlcxMan->GetPapPLCF()->HasSprm(0x442C);

            if (pDCS)
                nDropLines = (*pDCS) >> 3;
            else    // There is no Drop Cap Specifier hence no dropcap
                pPreviousNode = 0;

            if (const sal_uInt8 *pDistance = pPlcxMan->GetPapPLCF()->HasSprm(0x842F))
                nDistance = SVBT16ToShort( pDistance );
            else
                nDistance = 0;

            const SwFmtCharFmt *pSwFmtCharFmt = 0;

            if(pAktItemSet)
                pSwFmtCharFmt = &(ItemGet<SwFmtCharFmt>(*pAktItemSet, RES_TXTATR_CHARFMT));

            if(pSwFmtCharFmt)
                pFmt = pSwFmtCharFmt->GetCharFmt();

            if(pAktItemSet && !pFmt)
            {
                OUString sPrefix(OUStringBuffer("WW8Dropcap").append(nDropCap++).makeStringAndClear());
                pNewSwCharFmt = rDoc.MakeCharFmt(sPrefix, (SwCharFmt*)rDoc.GetDfltCharFmt());
                 pAktItemSet->ClearItem(RES_CHRATR_ESCAPEMENT);
                pNewSwCharFmt->SetFmtAttr( *pAktItemSet );
            }

            delete pAktItemSet;
            pAktItemSet = 0;
            bDropCap=false;
        }

        if (bStartLine || bWasTabRowEnd)
        {
            // Call all 64 CRs; not for Header and the like
            if ((nCrCount++ & 0x40) == 0 && nType == MAN_MAINTEXT)
            {
                nProgress = (sal_uInt16)( l * 100 / nTextLen );
                ::SetProgressState(nProgress, mpDocShell); // Update
            }
        }

        // If we have encountered a 0x0c which indicates either section of
        // pagebreak then look it up to see if it is a section break, and
        // if it is not then insert a page break. If it is a section break
        // it will be handled as such in the ReadAttrs of the next loop
        if (bPgSecBreak)
        {
            // We need only to see if a section is ending at this cp,
            // the plcf will already be sitting on the correct location
            // if it is there.
            WW8PLCFxDesc aTemp;
            aTemp.nStartPos = aTemp.nEndPos = WW8_CP_MAX;
            if (pPlcxMan->GetSepPLCF())
                pPlcxMan->GetSepPLCF()->GetSprms(&aTemp);
            if ((aTemp.nStartPos != l) && (aTemp.nEndPos != l))
            {
                // #i39251# - insert text node for page break, if no one inserted.
                // #i43118# - refine condition: the anchor
                // control stack has to have entries, otherwise it's not needed
                // to insert a text node.
                if (!bStartLine && !pAnchorStck->empty())
                {
                    AppendTxtNode(*pPaM->GetPoint());
                }
                rDoc.InsertPoolItem(*pPaM,
                    SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE, RES_BREAK), 0);
                bFirstParaOfPage = true;
                bPgSecBreak = false;
            }
        }
    }

    if (pPaM->GetPoint()->nContent.GetIndex())
        AppendTxtNode(*pPaM->GetPoint());

    if (!bInHyperlink)
        bJoined = JoinNode(*pPaM);

    CloseAttrEnds();

    delete pPlcxMan, pPlcxMan = 0;
    return bJoined;
}

/**
 * class SwWW8ImplReader
 */
SwWW8ImplReader::SwWW8ImplReader(sal_uInt8 nVersionPara, SvStorage* pStorage,
    SvStream* pSt, SwDoc& rD, const String& rBaseURL, bool bNewDoc) :
    mpDocShell(rD.GetDocShell()),
    pStg(pStorage),
    pStrm(pSt),
    pTableStream(0),
    pDataStream(0),
    rDoc(rD),
    maSectionManager(*this),
    m_aExtraneousParas(rD),
    maInsertedTables(rD),
    maSectionNameGenerator(rD, OUString("WW")),
    maGrfNameGenerator(bNewDoc, OUString('G')),
    maParaStyleMapper(rD),
    maCharStyleMapper(rD),
    maTxtNodesHavingFirstLineOfstSet(), // #i103711#
    maTxtNodesHavingLeftIndentSet(), // #i105414#
    pMSDffManager(0),
    mpAtnNames(0),
    sBaseURL(rBaseURL),
    m_bRegardHindiDigits( false ),
    mbNewDoc(bNewDoc),
    nDropCap(0),
    nIdctHint(0),
    bBidi(false),
    bReadTable(false),
    maCurrAttrCP(-1),
    mbOnLoadingMain(false)
{
    pStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    nWantedVersion = nVersionPara;
    pCtrlStck   = 0;
    mpRedlineStack = 0;
    pReffedStck = 0;
    pReffingStck = 0;
    pAnchorStck = 0;
    pFonts = 0;
    pSBase = 0;
    pPlcxMan = 0;
    pStyles = 0;
    pAktColl = 0;
    pLstManager = 0;
    pAktItemSet = 0;
    pDfltTxtFmtColl = 0;
    pStandardFmtColl = 0;
    pHdFt = 0;
    pWFlyPara = 0;
    pSFlyPara = 0;
    pFlyFmtOfJustInsertedGraphic   = 0;
    pFmtOfJustInsertedApo = 0;
    pPreviousNumPaM = 0;
    pPrevNumRule = 0;
    nAktColl = 0;
    nObjLocFc = nPicLocFc = 0;
    nInTable=0;
    bReadNoTbl = bPgSecBreak = bSpec = bObj = bTxbxFlySection
               = bHasBorder = bSymbol = bIgnoreText
               = bWasTabRowEnd = bWasTabCellEnd = false;
    bShdTxtCol = bCharShdTxtCol = bAnl = bHdFtFtnEdn = bFtnEdn
               = bIsHeader = bIsFooter = bIsUnicode = bCpxStyle = bStyNormal =
                 bWWBugNormal  = false;

    mpPostProcessAttrsInfo = 0;

    bNoAttrImport = bEmbeddObj = false;
    bAktAND_fNumberAcross = false;
    bNoLnNumYet = true;
    bInHyperlink = false;
    bWasParaEnd = false;
    bDropCap = false;
    bFirstPara = true;
    bFirstParaOfPage = false;
    bParaAutoBefore = false;
    bParaAutoAfter = false;
    nProgress = 0;
    nSwNumLevel = nWwNumType = 0xff;
    pTableDesc = 0;
    pNumOlst = 0;
    pNode_FLY_AT_PARA = 0;
    pDrawModel = 0;
    pDrawPg = 0;
    mpDrawEditEngine = 0;
    pWWZOrder = 0;
    pFormImpl = 0;
    mpChosenOutlineNumRule = 0;
    pNumFldType = 0;
    nFldNum = 0;

    nLFOPosition = USHRT_MAX;
    nListLevel = WW8ListManager::nMaxLevel;
    eHardCharSet = RTL_TEXTENCODING_DONTKNOW;

    nPgChpDelim = nPgChpLevel = 0;

    maApos.push_back(false);
}

void SwWW8ImplReader::DeleteStk(SwFltControlStack* pStck)
{
    if( pStck )
    {
        pStck->SetAttr( *pPaM->GetPoint(), 0, false);
        pStck->SetAttr( *pPaM->GetPoint(), 0, false);
        delete pStck;
    }
    else
    {
        OSL_ENSURE( !this, "WW-Stack bereits geloescht" );
    }
}

void wwSectionManager::SetSegmentToPageDesc(const wwSection &rSection,
    bool bIgnoreCols)
{
    SwPageDesc &rPage = *rSection.mpPage;

    SetNumberingType(rSection, rPage);

    SwFrmFmt &rFmt = rPage.GetMaster();

    if(mrReader.pWDop->fUseBackGroundInAllmodes) // #i56806# Make sure mrReader is initialized
        mrReader.GrafikCtor();


    if (mrReader.pWDop->fUseBackGroundInAllmodes && mrReader.pMSDffManager)
    {
        Rectangle aRect(0, 0, 100, 100); // A dummy, we don't care about the size
        SvxMSDffImportData aData(aRect);
        SdrObject* pObject = 0;
        if (mrReader.pMSDffManager->GetShape(0x401, pObject, aData))
        {
            // Only handle shape if it is a background shape
            if ((aData.begin()->nFlags & 0x400) != 0)
            {
                SfxItemSet aSet(rFmt.GetAttrSet());
                mrReader.MatchSdrItemsIntoFlySet(pObject, aSet, mso_lineSimple,
                                                 mso_lineSolid, mso_sptRectangle, aRect);
                rFmt.SetFmtAttr(aSet.Get(RES_BACKGROUND));
            }
        }
    }
    wwULSpaceData aULData;
    GetPageULData(rSection, aULData);
    SetPageULSpaceItems(rFmt, aULData, rSection);

    SetPage(rPage, rFmt, rSection, bIgnoreCols);

    if (!(rSection.maSep.pgbApplyTo & 1))
        mrReader.SetPageBorder(rFmt, rSection);
    if (!(rSection.maSep.pgbApplyTo & 2))
        mrReader.SetPageBorder(rPage.GetFirst(), rSection);

    mrReader.SetDocumentGrid(rFmt, rSection);
}

void wwSectionManager::SetUseOn(wwSection &rSection)
{
    bool bEven = (rSection.maSep.grpfIhdt & (WW8_HEADER_EVEN|WW8_FOOTER_EVEN)) ?
        true : false;

    bool bMirror = mrReader.pWDop->fMirrorMargins ||
        mrReader.pWDop->doptypography.f2on1;

    UseOnPage eUseBase = bMirror ? nsUseOnPage::PD_MIRROR : nsUseOnPage::PD_ALL;
    UseOnPage eUse = eUseBase;
    if (!bEven)
        eUse = (UseOnPage)(eUse | nsUseOnPage::PD_HEADERSHARE | nsUseOnPage::PD_FOOTERSHARE);
    if (!rSection.HasTitlePage())
        eUse = (UseOnPage)(eUse | nsUseOnPage::PD_FIRSTSHARE);

    OSL_ENSURE(rSection.mpPage, "Makes no sense to call me with no pages to set");
    if (rSection.mpPage)
        rSection.mpPage->WriteUseOn(eUse);
}

/**
 * Set the page descriptor on this node, handle the different cases for a text
 * node or a table
 */
void GiveNodePageDesc(SwNodeIndex &rIdx, const SwFmtPageDesc &rPgDesc,
    SwDoc &rDoc)
{
    /*
    If its a table here, apply the pagebreak to the table
    properties, otherwise we add it to the para at this
    position
    */
    if (rIdx.GetNode().IsTableNode())
    {
        SwTable& rTable =
            rIdx.GetNode().GetTableNode()->GetTable();
        SwFrmFmt* pApply = rTable.GetFrmFmt();
        OSL_ENSURE(pApply, "impossible");
        if (pApply)
            pApply->SetFmtAttr(rPgDesc);
    }
    else
    {
        SwPosition aPamStart(rIdx);
        aPamStart.nContent.Assign(
            rIdx.GetNode().GetCntntNode(), 0);
        SwPaM aPage(aPamStart);

        rDoc.InsertPoolItem(aPage, rPgDesc, 0);
    }
}

/**
 * Map a word section to a writer page descriptor
 */
SwFmtPageDesc wwSectionManager::SetSwFmtPageDesc(mySegIter &rIter,
    mySegIter &rStart, bool bIgnoreCols)
{
    if (IsNewDoc() && rIter == rStart)
    {
        rIter->mpPage =
            mrReader.rDoc.GetPageDescFromPool(RES_POOLPAGE_STANDARD);
    }
    else
    {
        sal_uInt16 nPos = mrReader.rDoc.MakePageDesc(
            ViewShell::GetShellRes()->GetPageDescName(mnDesc, ShellResource::NORMAL_PAGE),
            0, false);
        rIter->mpPage = &mrReader.rDoc.GetPageDesc(nPos);
    }
    OSL_ENSURE(rIter->mpPage, "no page!");
    if (!rIter->mpPage)
        return SwFmtPageDesc();

    // Set page before hd/ft
    const wwSection *pPrevious = 0;
    if (rIter != rStart)
        pPrevious = &(*(rIter-1));
    SetHdFt(*rIter, std::distance(rStart, rIter), pPrevious);
    SetUseOn(*rIter);

    // Set hd/ft after set page
    SetSegmentToPageDesc(*rIter, bIgnoreCols);

    SwFmtPageDesc aRet(rIter->mpPage);

    rIter->mpPage->SetFollow(rIter->mpPage);

    if (rIter->PageRestartNo())
        aRet.SetNumOffset(rIter->PageStartAt());

    ++mnDesc;
    return aRet;
}

bool wwSectionManager::IsNewDoc() const
{
    return mrReader.mbNewDoc;
}

void wwSectionManager::InsertSegments()
{
    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    sal_Bool bUseEnhFields = rOpt.IsUseEnhancedFields();
    mySegIter aEnd = maSegments.end();
    mySegIter aStart = maSegments.begin();
    for (mySegIter aIter = aStart; aIter != aEnd; ++aIter)
    {
        // If the section is of type "New column" (0x01), then simply insert a column break.
        // But only if there actually are columns on the page, otherwise a column break
        // seems to be handled like a page break by MSO.
        if ( aIter->maSep.bkc == 1 && aIter->maSep.ccolM1 > 0 )
        {
            SwPaM start( aIter->maStart );
            mrReader.rDoc.InsertPoolItem( start, SvxFmtBreakItem(SVX_BREAK_COLUMN_BEFORE, RES_BREAK), 0);
            continue;
        }

        mySegIter aNext = aIter+1;
        mySegIter aPrev = (aIter == aStart) ? aIter : aIter-1;

        // If two following sections are different in following properties, Word will interprete a continuous
        // section break between them as if it was a section break next page.
        bool bThisAndPreviousAreCompatible = ((aIter->GetPageWidth() == aPrev->GetPageWidth()) &&
            (aIter->GetPageHeight() == aPrev->GetPageHeight()) && (aIter->IsLandScape() == aPrev->IsLandScape()));

        bool bInsertSection = (aIter != aStart) ? (aIter->IsContinuous() &&  bThisAndPreviousAreCompatible): false;
        bool bInsertPageDesc = !bInsertSection;
        bool bProtected = SectionIsProtected(*aIter); // do we really  need this ?? I guess I have a different logic in editshell which disables this...
        if (bUseEnhFields && mrReader.pWDop->fProtEnabled && aIter->IsNotProtected())
        {
            // here we have the special case that the whole document is protected, with the execption of this section.
            // I want to address this when I do the section rework, so for the moment we disable the overall protection then...
            mrReader.rDoc.set(IDocumentSettingAccess::PROTECT_FORM, false );
        }


        if (bInsertPageDesc)
        {
            /*
             If a cont section follows this section then we won't be
             creating a page desc with 2+ cols as we cannot host a one
             col section in a 2+ col pagedesc and make it look like
             word. But if the current section actually has columns then
             we are forced to insert a section here as well as a page
             descriptor.
            */

            bool bIgnoreCols = false;
            bool bThisAndNextAreCompatible = (aNext != aEnd) ? ((aIter->GetPageWidth() == aNext->GetPageWidth()) &&
                (aIter->GetPageHeight() == aNext->GetPageHeight()) && (aIter->IsLandScape() == aNext->IsLandScape())) : true;

            if (((aNext != aEnd && aNext->IsContinuous() && bThisAndNextAreCompatible) || bProtected))
            {
                bIgnoreCols = true;
                if ((aIter->NoCols() > 1) || bProtected)
                    bInsertSection = true;
            }

            SwFmtPageDesc aDesc(SetSwFmtPageDesc(aIter, aStart, bIgnoreCols));
            if (!aDesc.GetPageDesc())
                continue;

            // special case handling for odd/even section break
            // a) as before create a new page style for the section break
            // b) set Layout of generated page style to right/left ( according
            //    to section break odd/even )
            // c) create a new style to follow the break page style
            if ( aIter->maSep.bkc == 3 || aIter->maSep.bkc == 4 )
            {
                // SetSwFmtPageDesc calls some methods that could
                // modify aIter (e.g. wwSection ).
                // Since  we call SetSwFmtPageDesc below to generate the
                // 'Following' style of the Break style, it is safer
                // to take  a copy of the contents of aIter.
                wwSection aTmpSection = *aIter;
                // create a new following page style
                SwFmtPageDesc aFollow(SetSwFmtPageDesc(aIter, aStart, bIgnoreCols));
                // restore any contents of aIter trashed by SetSwFmtPageDesc
                *aIter = aTmpSection;

                // Handle the section break
                UseOnPage eUseOnPage = nsUseOnPage::PD_LEFT;
                if ( aIter->maSep.bkc == 4 ) // Odd ( right ) Section break
                    eUseOnPage = nsUseOnPage::PD_RIGHT;

                aDesc.GetPageDesc()->WriteUseOn( eUseOnPage );
                aDesc.GetPageDesc()->SetFollow( aFollow.GetPageDesc() );
            }

            GiveNodePageDesc(aIter->maStart, aDesc, mrReader.rDoc);
        }

        SwTxtNode* pTxtNd = 0;
        if (bInsertSection)
        {
            // Start getting the bounds of this section
            SwPaM aSectPaM(*mrReader.pPaM);
            SwNodeIndex aAnchor(aSectPaM.GetPoint()->nNode);
            if (aNext != aEnd)
            {
                aAnchor = aNext->maStart;
                aSectPaM.GetPoint()->nNode = aAnchor;
                aSectPaM.GetPoint()->nContent.Assign(
                    aNext->maStart.GetNode().GetCntntNode(), 0);
                aSectPaM.Move(fnMoveBackward);
            }

            const SwPosition* pPos  = aSectPaM.GetPoint();
            SwTxtNode const*const pSttNd = pPos->nNode.GetNode().GetTxtNode();
            const SwTableNode* pTableNd = pSttNd ? pSttNd->FindTableNode() : 0;
            if (pTableNd)
            {
                pTxtNd =
                    mrReader.rDoc.GetNodes().MakeTxtNode(aAnchor,
                    mrReader.rDoc.GetTxtCollFromPool( RES_POOLCOLL_TEXT ));

                aSectPaM.GetPoint()->nNode = SwNodeIndex(*pTxtNd);
                aSectPaM.GetPoint()->nContent.Assign(
                    aSectPaM.GetCntntNode(), 0);
            }

            aSectPaM.SetMark();

            aSectPaM.GetPoint()->nNode = aIter->maStart;
            aSectPaM.GetPoint()->nContent.Assign(
                aSectPaM.GetCntntNode(), 0);

            // End getting the bounds of this section, quite a job eh?
            SwSectionFmt *pRet = InsertSection(aSectPaM, *aIter);
            // The last section if continous is always unbalanced
            if (pRet)
            {
                // Set the columns to be UnBalanced if that compatability option is set
                if (mrReader.pWDop->fNoColumnBalance)
                    pRet->SetFmtAttr(SwFmtNoBalancedColumns(true));
                else
                {
                    // Otherwise set to unbalanced if the following section is
                    // not continuous, (which also means that the last section
                    // is unbalanced)
                    if (aNext == aEnd || !aNext->IsContinuous())
                        pRet->SetFmtAttr(SwFmtNoBalancedColumns(true));
                }
            }

            bool bHasOwnHdFt = false;
            /*
             In this nightmare scenario the continuous section has its own
             headers and footers so we will try and find a hard page break
             between here and the end of the section and put the headers and
             footers there.
            */
            if (!bInsertPageDesc)
            {
               bHasOwnHdFt =
                mrReader.HasOwnHeaderFooter(
                 aIter->maSep.grpfIhdt & ~(WW8_HEADER_FIRST | WW8_FOOTER_FIRST),
                 aIter->maSep.grpfIhdt, std::distance(aStart, aIter)
                );
            }
            if (bHasOwnHdFt)
            {
                // #i40766# Need to cache the page descriptor in case there is
                // no page break in the section
                SwPageDesc *pOrig = aIter->mpPage;
                bool bFailed = true;
                SwFmtPageDesc aDesc(SetSwFmtPageDesc(aIter, aStart, true));
                if (aDesc.GetPageDesc())
                {
                    sal_uLong nStart = aSectPaM.Start()->nNode.GetIndex();
                    sal_uLong nEnd   = aSectPaM.End()->nNode.GetIndex();
                    for(; nStart <= nEnd; ++nStart)
                    {
                        SwNode* pNode = mrReader.rDoc.GetNodes()[nStart];
                        if (!pNode)
                            continue;
                        if (sw::util::HasPageBreak(*pNode))
                        {
                            SwNodeIndex aIdx(*pNode);
                            GiveNodePageDesc(aIdx, aDesc, mrReader.rDoc);
                            bFailed = false;
                            break;
                        }
                    }
                }
                if(bFailed)
                {
                    aIter->mpPage = pOrig;
                }
            }
        }

        if (pTxtNd)
        {
            SwNodeIndex aIdx(*pTxtNd);
            SwPaM aTest(aIdx);
            mrReader.rDoc.DelFullPara(aTest);
            pTxtNd = 0;
        }
    }
}

void wwExtraneousParas::delete_all_from_doc()
{
    typedef std::vector<SwTxtNode*>::iterator myParaIter;
    myParaIter aEnd = m_aTxtNodes.end();
    for (myParaIter aI = m_aTxtNodes.begin(); aI != aEnd; ++aI)
    {
        SwTxtNode *pTxtNode = *aI;
        SwNodeIndex aIdx(*pTxtNode);
        SwPaM aTest(aIdx);
        m_rDoc.DelFullPara(aTest);
    }
    m_aTxtNodes.clear();
}

void SwWW8ImplReader::StoreMacroCmds()
{
    if (pWwFib->lcbCmds)
    {
        pTableStream->Seek(pWwFib->fcCmds);

        uno::Reference < embed::XStorage > xRoot(mpDocShell->GetStorage());

        if (!xRoot.is())
            return;

        try
        {
            uno::Reference < io::XStream > xStream =
                    xRoot->openStreamElement( OUString(SL::aMSMacroCmds), embed::ElementModes::READWRITE );
            SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( xStream );

            sal_uInt8 *pBuffer = new sal_uInt8[pWwFib->lcbCmds];
            pWwFib->lcbCmds = pTableStream->Read(pBuffer, pWwFib->lcbCmds);
            pStream->Write(pBuffer, pWwFib->lcbCmds);
            delete[] pBuffer;
            delete pStream;
        }
        catch ( const uno::Exception& )
        {
        }
    }
}

void SwWW8ImplReader::ReadDocVars()
{
    std::vector<String> aDocVarStrings;
    std::vector<ww::bytes> aDocVarStringIds;
    std::vector<String> aDocValueStrings;
    WW8ReadSTTBF(!bVer67, *pTableStream, pWwFib->fcStwUser,
        pWwFib->lcbStwUser, bVer67 ? 2 : 0, eStructCharSet,
        aDocVarStrings, &aDocVarStringIds, &aDocValueStrings);
    if (!bVer67) {
        using namespace ::com::sun::star;

        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            mpDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps(
            xDPS->getDocumentProperties());
        OSL_ENSURE(xDocProps.is(), "DocumentProperties is null");
        uno::Reference<beans::XPropertyContainer> xUserDefinedProps =
            xDocProps->getUserDefinedProperties();
        OSL_ENSURE(xUserDefinedProps.is(), "UserDefinedProperties is null");

        for(size_t i=0; i<aDocVarStrings.size(); i++)
        {
            uno::Any aDefaultValue;
            OUString name(aDocVarStrings[i]);
            uno::Any aValue;
            aValue <<= OUString(aDocValueStrings[i]);
            try {
                xUserDefinedProps->addProperty( name,
                    beans::PropertyAttribute::REMOVABLE,
                    aValue );
            } catch (const uno::Exception &) {
                // ignore
            }
        }
    }
}

/**
 * Document Info
 */
void SwWW8ImplReader::ReadDocInfo()
{
    if( pStg )
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            mpDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps(
            xDPS->getDocumentProperties());
        OSL_ENSURE(xDocProps.is(), "DocumentProperties is null");

        if (xDocProps.is()) {
            if ( pWwFib->fDot )
            {
                OUString sTemplateURL;
                SfxMedium* pMedium = mpDocShell->GetMedium();
                if ( pMedium )
                {
                    OUString aName = pMedium->GetName();
                    INetURLObject aURL( aName );
                    sTemplateURL = aURL.GetMainURL(INetURLObject::DECODE_TO_IURI);
                    if ( !sTemplateURL.isEmpty() )
                        xDocProps->setTemplateURL( sTemplateURL );
                }
            }
            else if (pWwFib->lcbSttbfAssoc) // not a template, and has a SttbfAssoc
            {
                long nCur = pTableStream->Tell();
                Sttb aSttb;
                pTableStream->Seek( pWwFib->fcSttbfAssoc ); // point at tgc record
                if (!aSttb.Read( *pTableStream ) )
                    OSL_TRACE("** Read of SttbAssoc data failed!!!! ");
                pTableStream->Seek( nCur ); // return to previous position, is that necessary?
#if OSL_DEBUG_LEVEL > 1
                aSttb.Print( stderr );
#endif
                String sPath = aSttb.getStringAtIndex( 0x1 );
                OUString aURL;
                // attempt to convert to url (won't work for obvious reasons on linux)
                if ( sPath.Len() )
                    ::utl::LocalFileHelper::ConvertPhysicalNameToURL( sPath, aURL );
                if (aURL.isEmpty())
                    xDocProps->setTemplateURL( aURL );
                else
                    xDocProps->setTemplateURL( sPath );

            }
            sfx2::LoadOlePropertySet(xDocProps, pStg);
        }
    }
}

static void lcl_createTemplateToProjectEntry( const uno::Reference< container::XNameContainer >& xPrjNameCache, const OUString& sTemplatePathOrURL, const OUString& sVBAProjName )
{
    if ( xPrjNameCache.is() )
    {
        INetURLObject aObj;
        aObj.SetURL( sTemplatePathOrURL );
        bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
        OUString aURL;
        if ( bIsURL )
            aURL = sTemplatePathOrURL;
        else
        {
            osl::FileBase::getFileURLFromSystemPath( sTemplatePathOrURL, aURL );
            aObj.SetURL( aURL );
        }
        try
        {
            OUString templateNameWithExt = aObj.GetLastName();
            OUString templateName;
            sal_Int32 nIndex =  templateNameWithExt.lastIndexOf( '.' );
            if ( nIndex != -1 )
            {
                templateName = templateNameWithExt.copy( 0, nIndex );
                xPrjNameCache->insertByName( templateName, uno::makeAny( sVBAProjName ) );
            }
        }
        catch( const uno::Exception& )
        {
        }
    }
}

class WW8Customizations
{
    SvStream* mpTableStream;
    WW8Fib mWw8Fib;
public:
    WW8Customizations( SvStream*, WW8Fib& );
    bool  Import( SwDocShell* pShell );
};

WW8Customizations::WW8Customizations( SvStream* pTableStream, WW8Fib& rFib ) : mpTableStream(pTableStream), mWw8Fib( rFib )
{
}

bool WW8Customizations::Import( SwDocShell* pShell )
{
    if ( mWw8Fib.lcbCmds == 0 || !IsEightPlus(mWw8Fib.GetFIBVersion()) )
        return false;
    try
    {
        Tcg aTCG;
        long nCur = mpTableStream->Tell();
        mpTableStream->Seek( mWw8Fib.fcCmds ); // point at tgc record
        bool bReadResult = aTCG.Read( *mpTableStream );
        mpTableStream->Seek( nCur ); // return to previous position, is that necessary?
        if ( !bReadResult )
        {
            SAL_WARN("sw.ww8", "** Read of Customization data failed!!!! ");
            return false;
        }
#if OSL_DEBUG_LEVEL > 1
        aTCG.Print( stderr );
#endif
        return aTCG.ImportCustomToolBar( *pShell );
    }
    catch(...)
    {
        SAL_WARN("sw.ww8", "** Read of Customization data failed!!!! epically");
        return false;
    }
}

bool SwWW8ImplReader::ReadGlobalTemplateSettings( const OUString& sCreatedFrom, const uno::Reference< container::XNameContainer >& xPrjNameCache )
{
    SvtPathOptions aPathOpt;
    String aAddinPath = aPathOpt.GetAddinPath();
    uno::Sequence< OUString > sGlobalTemplates;

    // first get the autoload addins in the directory STARTUP
    uno::Reference<ucb::XSimpleFileAccess3> xSFA(ucb::SimpleFileAccess::create(::comphelper::getProcessComponentContext()));

    if( xSFA->isFolder( aAddinPath ) )
        sGlobalTemplates = xSFA->getFolderContents( aAddinPath, sal_False );

    sal_Int32 nEntries = sGlobalTemplates.getLength();
    bool bRes = true;
    for ( sal_Int32 i=0; i<nEntries; ++i )
    {
        INetURLObject aObj;
        aObj.SetURL( sGlobalTemplates[ i ] );
        bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
        OUString aURL;
        if ( bIsURL )
                aURL = sGlobalTemplates[ i ];
        else
                osl::FileBase::getFileURLFromSystemPath( sGlobalTemplates[ i ], aURL );
        if ( !aURL.endsWithIgnoreAsciiCase( ".dot" ) || ( !sCreatedFrom.isEmpty() && sCreatedFrom.equals( aURL ) ) )
            continue; // don't try and read the same document as ourselves

        SotStorageRef rRoot = new SotStorage( aURL, STREAM_STD_READWRITE, STORAGE_TRANSACTED );

        BasicProjImportHelper aBasicImporter( *mpDocShell );
        // Import vba via oox filter
        aBasicImporter.import( mpDocShell->GetMedium()->GetInputStream() );
        lcl_createTemplateToProjectEntry( xPrjNameCache, aURL, aBasicImporter.getProjectName() );
        // Read toolbars & menus
        SvStorageStreamRef refMainStream = rRoot->OpenSotStream( OUString( "WordDocument" ));
        refMainStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
        WW8Fib aWwFib( *refMainStream, 8 );
        SvStorageStreamRef xTableStream = rRoot->OpenSotStream(OUString::createFromAscii( aWwFib.fWhichTblStm ? SL::a1Table : SL::a0Table), STREAM_STD_READ);

        if (xTableStream.Is() && SVSTREAM_OK == xTableStream->GetError())
        {
            xTableStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
            WW8Customizations aGblCustomisations( xTableStream, aWwFib );
            aGblCustomisations.Import( mpDocShell );
        }
    }
    return bRes;
}

sal_uLong SwWW8ImplReader::CoreLoad(WW8Glossary *pGloss, const SwPosition &rPos)
{
    sal_uLong nErrRet = 0;

    if (mbNewDoc && pStg && !pGloss)
        ReadDocInfo();

    ::ww8::WW8FibData * pFibData = new ::ww8::WW8FibData();

    if (pWwFib->fReadOnlyRecommended)
        pFibData->setReadOnlyRecommended(true);
    else
        pFibData->setReadOnlyRecommended(false);

    if (pWwFib->fWriteReservation)
        pFibData->setWriteReservation(true);
    else
        pFibData->setWriteReservation(false);

    ::sw::tExternalDataPointer pExternalFibData(pFibData);

    rDoc.setExternalData(::sw::FIB, pExternalFibData);

    ::sw::tExternalDataPointer pSttbfAsoc
          (new ::ww8::WW8Sttb<ww8::WW8Struct>(*pTableStream, pWwFib->fcSttbfAssoc, pWwFib->lcbSttbfAssoc));

    rDoc.setExternalData(::sw::STTBF_ASSOC, pSttbfAsoc);

    if (pWwFib->fWriteReservation || pWwFib->fReadOnlyRecommended)
    {
        SwDocShell * pDocShell = rDoc.GetDocShell();
        if (pDocShell)
            pDocShell->SetReadOnlyUI(sal_True);
    }

    pPaM = new SwPaM(rPos);

    pCtrlStck = new SwWW8FltControlStack( &rDoc, nFieldFlags, *this );

    mpRedlineStack = new sw::util::RedlineStack(rDoc);

    /*
        RefFldStck: Keeps track of bookmarks which may be inserted as
        variables intstead.
    */
    pReffedStck = new SwFltEndStack(&rDoc, nFieldFlags);
    pReffingStck = new SwWW8FltRefStack(&rDoc, nFieldFlags);

    pAnchorStck = new SwWW8FltAnchorStack(&rDoc, nFieldFlags);

    sal_uInt16 nPageDescOffset = rDoc.GetPageDescCnt();

    SwNodeIndex aSttNdIdx( rDoc.GetNodes() );
    SwRelNumRuleSpaces aRelNumRule(rDoc, mbNewDoc);

    sal_uInt16 eMode = nsRedlineMode_t::REDLINE_SHOW_INSERT;

    mpSprmParser = new wwSprmParser(pWwFib->GetFIBVersion());

    // Set handy helper variables
    bVer6  = (6 == pWwFib->nVersion);
    bVer7  = (7 == pWwFib->nVersion);
    bVer67 = bVer6 || bVer7;
    bVer8  = (8 == pWwFib->nVersion);

    eTextCharSet = WW8Fib::GetFIBCharset(pWwFib->chse);
    eStructCharSet = WW8Fib::GetFIBCharset(pWwFib->chseTables);

    bWWBugNormal = pWwFib->nProduct == 0xc03d;

    if (!mbNewDoc)
        aSttNdIdx = pPaM->GetPoint()->nNode;

    ::StartProgress(STR_STATSTR_W4WREAD, 0, 100, mpDocShell);

    // read Font Table
    pFonts = new WW8Fonts( *pTableStream, *pWwFib );

    // Document Properties
    pWDop = new WW8Dop( *pTableStream, pWwFib->nFib, pWwFib->fcDop,
        pWwFib->lcbDop );

    if (mbNewDoc)
        ImportDop();

    /*
        Import revisioning data: author names
    */
    if( pWwFib->lcbSttbfRMark )
    {
        ReadRevMarkAuthorStrTabl( *pTableStream,
                                    pWwFib->fcSttbfRMark,
                                    pWwFib->lcbSttbfRMark, rDoc );
    }

    // M.M. Initialize our String/ID map for Linked Sections
    std::vector<String> aLinkStrings;
    std::vector<ww::bytes> aStringIds;

    WW8ReadSTTBF(!bVer67, *pTableStream, pWwFib->fcSttbFnm,
        pWwFib->lcbSttbFnm, bVer67 ? 2 : 0, eStructCharSet,
        aLinkStrings, &aStringIds);

    for (size_t i=0; i < aLinkStrings.size() && i < aStringIds.size(); ++i)
    {
        ww::bytes stringId = aStringIds[i];
        WW8_STRINGID *stringIdStruct = (WW8_STRINGID*)(&stringId[0]);
        aLinkStringMap[SVBT16ToShort(stringIdStruct->nStringId)] =
            aLinkStrings[i];
    }

    ReadDocVars(); // import document variables as meta information.

    ::SetProgressState(nProgress, mpDocShell);    // Update

    pLstManager = new WW8ListManager( *pTableStream, *this );

    /*
        zuerst(!) alle Styles importieren   (siehe WW8PAR2.CXX)
            VOR dem Import der Listen !!
    */
    ::SetProgressState(nProgress, mpDocShell);    // Update
    pStyles = new WW8RStyle( *pWwFib, this );     // Styles
    pStyles->Import();

    /*
        In the end: (also see WW8PAR3.CXX)

        Go through all Styles and attach respective List Format
        AFTER we imported the Styles and AFTER we imported the Lists!
    */
    ::SetProgressState(nProgress, mpDocShell); // Update
    pStyles->PostProcessStyles();

    if (!vColl.empty())
        SetOutLineStyles();

    pSBase = new WW8ScannerBase(pStrm,pTableStream,pDataStream,pWwFib);

    static const SvxExtNumType eNumTA[16] =
    {
        SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
        SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC
    };

    if (pSBase->AreThereFootnotes())
    {
        static const SwFtnNum eNumA[4] =
        {
            FTNNUM_DOC, FTNNUM_CHAPTER, FTNNUM_PAGE, FTNNUM_DOC
        };

        SwFtnInfo aInfo;
        aInfo = rDoc.GetFtnInfo(); // Copy-Ctor private

        aInfo.ePos = FTNPOS_PAGE;
        aInfo.eNum = eNumA[pWDop->rncFtn];
        aInfo.aFmt.SetNumberingType( static_cast< sal_uInt16 >(eNumTA[pWDop->nfcFtnRef]) );
        if( pWDop->nFtn )
            aInfo.nFtnOffset = pWDop->nFtn - 1;
        rDoc.SetFtnInfo( aInfo );
    }
    if( pSBase->AreThereEndnotes() )
    {
        SwEndNoteInfo aInfo;
        aInfo = rDoc.GetEndNoteInfo(); // Same as for Ftn

        aInfo.aFmt.SetNumberingType( static_cast< sal_uInt16 >(eNumTA[pWDop->nfcEdnRef]) );
        if( pWDop->nEdn )
            aInfo.nFtnOffset = pWDop->nEdn - 1;
        rDoc.SetEndNoteInfo( aInfo );
    }

    if( pWwFib->lcbPlcfhdd )
        pHdFt = new WW8PLCF_HdFt( pTableStream, *pWwFib, *pWDop );

    if (!mbNewDoc)
    {
        // inserting into an existing document:
        // As only complete paragraphs are inserted, the current one
        // needs to be splitted - once or even twice.
        const SwPosition* pPos = pPaM->GetPoint();

        // split current paragraph to get new paragraph for the insertion
        rDoc.SplitNode( *pPos, false );

        // another split, if insertion position was not at the end of the current paragraph.
        SwTxtNode const*const pTxtNd = pPos->nNode.GetNode().GetTxtNode();
        if ( pTxtNd->GetTxt().getLength() )
        {
            rDoc.SplitNode( *pPos, false );
            // move PaM back to the newly empty paragraph
            pPaM->Move( fnMoveBackward );
        }

        // suppress insertion of tables inside footnotes.
        const sal_uLong nNd = pPos->nNode.GetIndex();
        bReadNoTbl = ( nNd < rDoc.GetNodes().GetEndOfInserts().GetIndex() &&
                       rDoc.GetNodes().GetEndOfInserts().StartOfSectionIndex() < nNd );

    }

    ::SetProgressState(nProgress, mpDocShell); // Update

    // loop for each glossary entry and add dummy section node
    if (pGloss)
    {
        WW8PLCF aPlc(*pTableStream, pWwFib->fcPlcfglsy, pWwFib->lcbPlcfglsy, 0);

        WW8_CP nStart, nEnd;
        void* pDummy;

        for (int i = 0; i < pGloss->GetNoStrings(); ++i, aPlc.advance())
        {
            SwNodeIndex aIdx( rDoc.GetNodes().GetEndOfContent());
            SwTxtFmtColl* pColl =
                rDoc.GetTxtCollFromPool(RES_POOLCOLL_STANDARD,
                false);
            SwStartNode *pNode =
                rDoc.GetNodes().MakeTextSection(aIdx,
                SwNormalStartNode,pColl);
            pPaM->GetPoint()->nNode = pNode->GetIndex()+1;
            pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(),0);
            aPlc.Get( nStart, nEnd, pDummy );
            ReadText(nStart,nEnd-nStart-1,MAN_MAINTEXT);
        }
    }
    else // ordinary case
    {
        if (mbNewDoc && pStg && !pGloss) /*meaningless for a glossary, cmc*/
        {
            mpDocShell->SetIsTemplate( pWwFib->fDot ); // point at tgc record
            uno::Reference<document::XDocumentPropertiesSupplier> const
                xDocPropSupp(mpDocShell->GetModel(), uno::UNO_QUERY_THROW);
            uno::Reference< document::XDocumentProperties > xDocProps( xDocPropSupp->getDocumentProperties(), uno::UNO_QUERY_THROW );

            OUString sCreatedFrom = xDocProps->getTemplateURL();
            uno::Reference< container::XNameContainer > xPrjNameCache;
            uno::Reference< lang::XMultiServiceFactory> xSF(mpDocShell->GetModel(), uno::UNO_QUERY);
            if ( xSF.is() )
                xPrjNameCache.set( xSF->createInstance( "ooo.vba.VBAProjectNameProvider" ), uno::UNO_QUERY );

            // Read Global templates
            ReadGlobalTemplateSettings( sCreatedFrom, xPrjNameCache );

            // Create and insert Word vba Globals
            uno::Any aGlobs;
            uno::Sequence< uno::Any > aArgs(1);
            aArgs[ 0 ] <<= mpDocShell->GetModel();
            aGlobs <<= ::comphelper::getProcessServiceFactory()->createInstanceWithArguments( "ooo.vba.word.Globals", aArgs );

#ifndef DISABLE_SCRIPTING
            BasicManager *pBasicMan = mpDocShell->GetBasicManager();
            if (pBasicMan)
                pBasicMan->SetGlobalUNOConstant( "VBAGlobals", aGlobs );
#endif
            BasicProjImportHelper aBasicImporter( *mpDocShell );
            // Import vba via oox filter
            bool bRet = aBasicImporter.import( mpDocShell->GetMedium()->GetInputStream() );

            lcl_createTemplateToProjectEntry( xPrjNameCache, sCreatedFrom, aBasicImporter.getProjectName() );
            WW8Customizations aCustomisations( pTableStream, *pWwFib );
            aCustomisations.Import( mpDocShell );

            if( bRet )
                rDoc.SetContainsMSVBasic(true);

            StoreMacroCmds();
        }
        mbOnLoadingMain = true;
        ReadText(0, pWwFib->ccpText, MAN_MAINTEXT);
        mbOnLoadingMain = false;

    }

    ::SetProgressState(nProgress, mpDocShell); // Update

    if (pDrawPg && pMSDffManager && pMSDffManager->GetShapeOrders())
    {
        // Helper array to chain the inserted frames (instead of SdrTxtObj)
        SvxMSDffShapeTxBxSort aTxBxSort;

        // Ensure correct z-order of read Escher objects
        sal_uInt16 nShapeCount = pMSDffManager->GetShapeOrders()->size();

        for (sal_uInt16 nShapeNum=0; nShapeNum < nShapeCount; nShapeNum++)
        {
            SvxMSDffShapeOrder *pOrder =
                (*pMSDffManager->GetShapeOrders())[nShapeNum];
            // Insert Pointer into new Sort array
            if (pOrder->nTxBxComp && pOrder->pFly)
                aTxBxSort.insert(pOrder);
        }
        // Chain Frames
        if( !aTxBxSort.empty() )
        {
            SwFmtChain aChain;
            for( SvxMSDffShapeTxBxSort::iterator it = aTxBxSort.begin(); it != aTxBxSort.end(); ++it )
            {
                SvxMSDffShapeOrder *pOrder = *it;

                // Initialize FlyFrame Formats
                SwFlyFrmFmt* pFlyFmt     = pOrder->pFly;
                SwFlyFrmFmt* pNextFlyFmt = 0;
                SwFlyFrmFmt* pPrevFlyFmt = 0;

                // Determine successor, if we can
                SvxMSDffShapeTxBxSort::iterator tmpIter1 = it;
                ++tmpIter1;
                if( tmpIter1 != aTxBxSort.end() )
                {
                    SvxMSDffShapeOrder *pNextOrder = *tmpIter1;
                    if ((0xFFFF0000 & pOrder->nTxBxComp)
                           == (0xFFFF0000 & pNextOrder->nTxBxComp))
                        pNextFlyFmt = pNextOrder->pFly;
                }
                // Determine precessor, if we can
                if( it != aTxBxSort.begin() )
                {
                    SvxMSDffShapeTxBxSort::iterator tmpIter2 = it;
                    --tmpIter2;
                    SvxMSDffShapeOrder *pPrevOrder = *tmpIter2;
                    if ((0xFFFF0000 & pOrder->nTxBxComp)
                           == (0xFFFF0000 & pPrevOrder->nTxBxComp))
                        pPrevFlyFmt = pPrevOrder->pFly;
                }
                // If successor or predecessor present, insert the
                // chain at the FlyFrame Format
                if (pNextFlyFmt || pPrevFlyFmt)
                {
                    aChain.SetNext( pNextFlyFmt );
                    aChain.SetPrev( pPrevFlyFmt );
                    pFlyFmt->SetFmtAttr( aChain );
                }
            }

        }

    }

    if (mbNewDoc)
    {
        if( pWDop->fRevMarking )
            eMode |= nsRedlineMode_t::REDLINE_ON;
        if( pWDop->fRMView )
            eMode |= nsRedlineMode_t::REDLINE_SHOW_DELETE;
    }

    maInsertedTables.DelAndMakeTblFrms();
    maSectionManager.InsertSegments();

    vColl.clear();

    DELETEZ( pStyles );

    if( pFormImpl )
        DeleteFormImpl();
    GrafikDtor();
    DELETEZ( pMSDffManager );
    DELETEZ( pHdFt );
    DELETEZ( pSBase );
    delete pWDop;
    DELETEZ( pFonts );
    delete mpAtnNames;
    delete mpSprmParser;
    ::EndProgress(mpDocShell);

    pDataStream = 0;
    pTableStream = 0;

    DeleteCtrlStk();
    mpRedlineStack->closeall(*pPaM->GetPoint());
    delete mpRedlineStack;
    DeleteAnchorStk();
    DeleteRefStks();
    // For i120928,achieve the graphics from the special bookmark with is for graphic bullet
    {
        std::vector<const SwGrfNode*> vecBulletGrf;
        std::vector<SwFrmFmt*> vecFrmFmt;

        IDocumentMarkAccess* const pMarkAccess = rDoc.getIDocumentMarkAccess();
        if ( pMarkAccess )
        {
            IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->findBookmark( "_PictureBullets" );
            if ( ppBkmk != pMarkAccess->getBookmarksEnd() &&
                       IDocumentMarkAccess::GetType( *(ppBkmk->get()) ) == IDocumentMarkAccess::BOOKMARK )
            {
                SwTxtNode* pTxtNode = ppBkmk->get()->GetMarkStart().nNode.GetNode().GetTxtNode();

                if ( pTxtNode )
                {
                    const SwpHints *pHints = pTxtNode->GetpSwpHints();
                    for(int nHintPos = 0; pHints && nHintPos < pHints->Count(); ++nHintPos)
                    {
                        const SwTxtAttr *pHt = (*pHints)[nHintPos];
                        xub_StrLen st = *(pHt->GetStart());
                        if(pHt && pHt->Which() == RES_TXTATR_FLYCNT && (st >= ppBkmk->get()->GetMarkStart().nContent.GetIndex()))
                        {
                            SwFrmFmt *pFrmFmt = pHt->GetFlyCnt().GetFrmFmt();
                            const SwNodeIndex *pNdIdx = pFrmFmt->GetCntnt().GetCntntIdx();
                            if (pNdIdx)
                            {
                                const SwNodes &nos = pNdIdx->GetNodes();
                                const SwGrfNode *pGrf = dynamic_cast<const SwGrfNode*>(nos[pNdIdx->GetIndex() + 1]);
                                if (pGrf)
                                {
                                    vecBulletGrf.push_back(pGrf);
                                    vecFrmFmt.push_back(pFrmFmt);
                                }
                            }
                        }
                    }
                    // update graphic bullet information
                    size_t nCount = pLstManager->GetWW8LSTInfoNum();
                    for (size_t i = 0; i < nCount; ++i)
                    {
                        SwNumRule* pRule = pLstManager->GetNumRule(i);
                        for (int j = 0; j < MAXLEVEL; ++j)
                        {
                            SwNumFmt aNumFmt(pRule->Get(j));
                            sal_Int16 nType = aNumFmt.GetNumberingType();
                            sal_uInt16 nGrfBulletCP = aNumFmt.GetGrfBulletCP();
                            if (nType == SVX_NUM_BITMAP && vecBulletGrf.size() > nGrfBulletCP)
                            {
                                Graphic aGraphic = vecBulletGrf[nGrfBulletCP]->GetGrf();
                                SvxBrushItem aBrush(aGraphic, GPOS_AREA, SID_ATTR_BRUSH);
                                Font aFont = numfunc::GetDefBulletFont();
                                int nHeight = aFont.GetHeight() * 12;//20;
                                Size aPrefSize( aGraphic.GetPrefSize());
                                if (aPrefSize.Height() * aPrefSize.Width() != 0 )
                                {
                                    int nWidth = (nHeight * aPrefSize.Width()) / aPrefSize.Height();
                                    Size aSize(nWidth, nHeight);
                                    aNumFmt.SetGraphicBrush(&aBrush, &aSize);
                                }
                                else
                                {
                                    aNumFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                                    aNumFmt.SetBulletChar(0x2190);
                                }
                                pRule->Set( j, aNumFmt );
                            }
                        }
                    }
                    // Remove additional pictures
                    for (sal_uInt16 i = 0; i < vecFrmFmt.size(); ++i)
                    {
                        rDoc.DelLayoutFmt(vecFrmFmt[i]);
                    }
                }
            }
        }
        DELETEZ( pLstManager );
    }

    // remove extra paragraphs after attribute ctrl
    // stacks etc. are destroyed, and before fields
    // are updated
    m_aExtraneousParas.delete_all_from_doc();

    UpdateFields();

    // delete the pam before the call for hide all redlines (Bug 73683)
    if (mbNewDoc)
      rDoc.SetRedlineMode((RedlineMode_t)( eMode ));

    UpdatePageDescs(rDoc, nPageDescOffset);

    delete pPaM, pPaM = 0;
    return nErrRet;
}

sal_uLong SwWW8ImplReader::SetSubStreams(SvStorageStreamRef &rTableStream,
    SvStorageStreamRef &rDataStream)
{
    sal_uLong nErrRet = 0;
    // 6 stands for "6 OR 7", 7 stands for "ONLY 7"
    switch (pWwFib->nVersion)
    {
        case 6:
        case 7:
            pTableStream = pStrm;
            pDataStream = pStrm;
            break;
        case 8:
            if(!pStg)
            {
                OSL_ENSURE( pStg, "Version 8 always needs to have a Storage!!" );
                nErrRet = ERR_SWG_READ_ERROR;
                break;
            }

            rTableStream = pStg->OpenSotStream( OUString::createFromAscii(
                pWwFib->fWhichTblStm ? SL::a1Table : SL::a0Table),
                STREAM_STD_READ);

            pTableStream = &rTableStream;
            pTableStream->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

            rDataStream = pStg->OpenSotStream(OUString(SL::aData),
                STREAM_STD_READ | STREAM_NOCREATE );

            if (rDataStream.Is() && SVSTREAM_OK == rDataStream->GetError())
            {
                pDataStream = &rDataStream;
                pDataStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
            }
            else
                pDataStream = pStrm;
            break;
        default:
            // Program error!
            OSL_ENSURE( !this, "We forgot to encode nVersion!" );
            nErrRet = ERR_SWG_READ_ERROR;
            break;
    }
    return nErrRet;
}

namespace
{
    utl::TempFile *MakeTemp(SvFileStream &rSt)
    {
        utl::TempFile *pT = new utl::TempFile;
        pT->EnableKillingFile();
        rSt.Open(pT->GetFileName(), STREAM_READWRITE | STREAM_SHARE_DENYWRITE);
        return pT;
    }

#define WW_BLOCKSIZE 0x200

    void DecryptRC4(msfilter::MSCodec_Std97& rCtx, SvStream &rIn, SvStream &rOut)
    {
        rIn.Seek(STREAM_SEEK_TO_END);
        const sal_Size nLen = rIn.Tell();
        rIn.Seek(0);

        sal_uInt8 in[WW_BLOCKSIZE];
        for (sal_Size nI = 0, nBlock = 0; nI < nLen; nI += WW_BLOCKSIZE, ++nBlock)
        {
            sal_Size nBS = (nLen - nI > WW_BLOCKSIZE) ? WW_BLOCKSIZE : nLen - nI;
            nBS = rIn.Read(in, nBS);
            rCtx.InitCipher(nBlock);
            rCtx.Decode(in, nBS, in, nBS);
            rOut.Write(in, nBS);
        }
    }

    void DecryptXOR(msfilter::MSCodec_XorWord95 &rCtx, SvStream &rIn, SvStream &rOut)
    {
        sal_Size nSt = rIn.Tell();
        rIn.Seek(STREAM_SEEK_TO_END);
        sal_Size nLen = rIn.Tell();
        rIn.Seek(nSt);

        rCtx.InitCipher();
        rCtx.Skip(nSt);

        sal_uInt8 in[0x4096];
        for (sal_Size nI = nSt; nI < nLen; nI += 0x4096)
        {
            sal_Size nBS = (nLen - nI > 0x4096 ) ? 0x4096 : nLen - nI;
            nBS = rIn.Read(in, nBS);
            rCtx.Decode(in, nBS);
            rOut.Write(in, nBS);
        }
    }

    // moan, copy and paste :-(
    String QueryPasswordForMedium(SfxMedium& rMedium)
    {
        String aPassw;

        using namespace com::sun::star;

        const SfxItemSet* pSet = rMedium.GetItemSet();
        const SfxPoolItem *pPasswordItem;

        if(pSet && SFX_ITEM_SET == pSet->GetItemState(SID_PASSWORD, sal_True, &pPasswordItem))
            aPassw = ((const SfxStringItem *)pPasswordItem)->GetValue();
        else
        {
            try
            {
                uno::Reference< task::XInteractionHandler > xHandler( rMedium.GetInteractionHandler() );
                if( xHandler.is() )
                {
                    ::comphelper::DocPasswordRequest* pRequest = new ::comphelper::DocPasswordRequest(
                        ::comphelper::DocPasswordRequestType_MS, task::PasswordRequestMode_PASSWORD_ENTER,
                        INetURLObject( rMedium.GetOrigURL() ).GetName( INetURLObject::DECODE_WITH_CHARSET ) );
                    uno::Reference< task::XInteractionRequest > xRequest( pRequest );

                    xHandler->handle( xRequest );

                    if( pRequest->isPassword() )
                        aPassw = pRequest->getPassword();
                }
            }
            catch( const uno::Exception& )
            {
            }
        }

        return aPassw;
    }

    uno::Sequence< beans::NamedValue > InitXorWord95Codec( ::msfilter::MSCodec_XorWord95& rCodec, SfxMedium& rMedium, WW8Fib* pWwFib )
    {
        uno::Sequence< beans::NamedValue > aEncryptionData;
        SFX_ITEMSET_ARG( rMedium.GetItemSet(), pEncryptionData, SfxUnoAnyItem, SID_ENCRYPTIONDATA, sal_False );
        if ( pEncryptionData && ( pEncryptionData->GetValue() >>= aEncryptionData ) && !rCodec.InitCodec( aEncryptionData ) )
            aEncryptionData.realloc( 0 );

        if ( !aEncryptionData.getLength() )
        {
            String sUniPassword = QueryPasswordForMedium( rMedium );

            OString sPassword(OUStringToOString(sUniPassword,
                WW8Fib::GetFIBCharset(pWwFib->chseTables)));

            sal_Int32 nLen = sPassword.getLength();
            if( nLen <= 15 )
            {
                sal_uInt8 pPassword[16];
                memcpy(pPassword, sPassword.getStr(), nLen);
                memset(pPassword+nLen, 0, sizeof(pPassword)-nLen);

                rCodec.InitKey( pPassword );
                aEncryptionData = rCodec.GetEncryptionData();

                // the export supports RC4 algorithm only, so we have to
                // generate the related EncryptionData as well, so that Save
                // can export the document without asking for a password;
                // as result there will be EncryptionData for both algorithms
                // in the MediaDescriptor
                ::msfilter::MSCodec_Std97 aCodec97;

                // Generate random number with a seed of time as salt.
                TimeValue aTime;
                osl_getSystemTime( &aTime );
                rtlRandomPool aRandomPool = rtl_random_createPool();
                rtl_random_addBytes ( aRandomPool, &aTime, 8 );

                sal_uInt8 pDocId[ 16 ];
                rtl_random_getBytes( aRandomPool, pDocId, 16 );

                rtl_random_destroyPool( aRandomPool );

                sal_uInt16 pStd97Pass[16];
                memset( pStd97Pass, 0, sizeof( pStd97Pass ) );
                for (xub_StrLen nChar = 0; nChar < nLen; ++nChar )
                    pStd97Pass[nChar] = sUniPassword.GetChar(nChar);

                aCodec97.InitKey( pStd97Pass, pDocId );

                // merge the EncryptionData, there should be no conflicts
                ::comphelper::SequenceAsHashMap aEncryptionHash( aEncryptionData );
                aEncryptionHash.update( ::comphelper::SequenceAsHashMap( aCodec97.GetEncryptionData() ) );
                aEncryptionHash >> aEncryptionData;
            }
        }

        return aEncryptionData;
    }

    uno::Sequence< beans::NamedValue > InitStd97Codec( ::msfilter::MSCodec_Std97& rCodec, sal_uInt8 pDocId[16], SfxMedium& rMedium )
    {
        uno::Sequence< beans::NamedValue > aEncryptionData;
        SFX_ITEMSET_ARG( rMedium.GetItemSet(), pEncryptionData, SfxUnoAnyItem, SID_ENCRYPTIONDATA, sal_False );
        if ( pEncryptionData && ( pEncryptionData->GetValue() >>= aEncryptionData ) && !rCodec.InitCodec( aEncryptionData ) )
            aEncryptionData.realloc( 0 );

        if ( !aEncryptionData.getLength() )
        {
            String sUniPassword = QueryPasswordForMedium( rMedium );

            xub_StrLen nLen = sUniPassword.Len();
            if ( nLen <= 15 )
            {
                sal_Unicode pPassword[16];
                memset( pPassword, 0, sizeof( pPassword ) );
                for (xub_StrLen nChar = 0; nChar < nLen; ++nChar )
                    pPassword[nChar] = sUniPassword.GetChar(nChar);

                rCodec.InitKey( pPassword, pDocId );
                aEncryptionData = rCodec.GetEncryptionData();
            }
        }

        return aEncryptionData;
    }
}

sal_uLong SwWW8ImplReader::LoadThroughDecryption(SwPaM& rPaM ,WW8Glossary *pGloss)
{
    sal_uLong nErrRet = 0;
    if (pGloss)
        pWwFib = pGloss->GetFib();
    else
        pWwFib = new WW8Fib(*pStrm, nWantedVersion);

    if (pWwFib->nFibError)
        nErrRet = ERR_SWG_READ_ERROR;

    SvStorageStreamRef xTableStream, xDataStream;

    if (!nErrRet)
        nErrRet = SetSubStreams(xTableStream, xDataStream);

    utl::TempFile *pTempMain = 0;
    utl::TempFile *pTempTable = 0;
    utl::TempFile *pTempData = 0;
    SvFileStream aDecryptMain;
    SvFileStream aDecryptTable;
    SvFileStream aDecryptData;

    bool bDecrypt = false;
    enum {RC4, XOR, Other} eAlgo = Other;
    if (pWwFib->fEncrypted && !nErrRet)
    {
        if (!pGloss)
        {
            bDecrypt = true;
            if (8 != pWwFib->nVersion)
                eAlgo = XOR;
            else
            {
                if (pWwFib->nKey != 0)
                    eAlgo = XOR;
                else
                {
                    pTableStream->Seek(0);
                    sal_uInt32 nEncType;
                    *pTableStream >> nEncType;
                    if (nEncType == 0x10001)
                        eAlgo = RC4;
                }
            }
        }
    }

    if (bDecrypt)
    {
        nErrRet = ERRCODE_SVX_WRONGPASS;
        SfxMedium* pMedium = mpDocShell->GetMedium();

        if ( pMedium )
        {
            switch (eAlgo)
            {
                default:
                    nErrRet = ERRCODE_SVX_READ_FILTER_CRYPT;
                    break;
                case XOR:
                {
                    msfilter::MSCodec_XorWord95 aCtx;
                    uno::Sequence< beans::NamedValue > aEncryptionData = InitXorWord95Codec( aCtx, *pMedium, pWwFib );

                    // if initialization has failed the EncryptionData should be empty
                    if ( aEncryptionData.getLength() && aCtx.VerifyKey( pWwFib->nKey, pWwFib->nHash ) )
                    {
                        nErrRet = 0;
                        pTempMain = MakeTemp(aDecryptMain);

                        pStrm->Seek(0);
                        size_t nUnencryptedHdr =
                            (8 == pWwFib->nVersion) ? 0x44 : 0x34;
                        sal_uInt8 *pIn = new sal_uInt8[nUnencryptedHdr];
                        nUnencryptedHdr = pStrm->Read(pIn, nUnencryptedHdr);
                        aDecryptMain.Write(pIn, nUnencryptedHdr);
                        delete [] pIn;

                        DecryptXOR(aCtx, *pStrm, aDecryptMain);

                        if (!pTableStream || pTableStream == pStrm)
                            pTableStream = &aDecryptMain;
                        else
                        {
                            pTempTable = MakeTemp(aDecryptTable);
                            DecryptXOR(aCtx, *pTableStream, aDecryptTable);
                            pTableStream = &aDecryptTable;
                        }

                        if (!pDataStream || pDataStream == pStrm)
                            pDataStream = &aDecryptMain;
                        else
                        {
                            pTempData = MakeTemp(aDecryptData);
                            DecryptXOR(aCtx, *pDataStream, aDecryptData);
                            pDataStream = &aDecryptData;
                        }

                        pMedium->GetItemSet()->ClearItem( SID_PASSWORD );
                        pMedium->GetItemSet()->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData ) ) );
                    }
                }
                break;
                case RC4:
                {
                    sal_uInt8 aDocId[ 16 ];
                    sal_uInt8 aSaltData[ 16 ];
                    sal_uInt8 aSaltHash[ 16 ];

                    bool bCouldReadHeaders =
                        checkRead(*pTableStream, aDocId, 16) &&
                        checkRead(*pTableStream, aSaltData, 16) &&
                        checkRead(*pTableStream, aSaltHash, 16);

                    msfilter::MSCodec_Std97 aCtx;
                    // if initialization has failed the EncryptionData should be empty
                    uno::Sequence< beans::NamedValue > aEncryptionData;
                    if (bCouldReadHeaders)
                        aEncryptionData = InitStd97Codec( aCtx, aDocId, *pMedium );
                    if ( aEncryptionData.getLength() && aCtx.VerifyKey( aSaltData, aSaltHash ) )
                    {
                        nErrRet = 0;

                        pTempMain = MakeTemp(aDecryptMain);

                        pStrm->Seek(0);
                        sal_Size nUnencryptedHdr = 0x44;
                        sal_uInt8 *pIn = new sal_uInt8[nUnencryptedHdr];
                        nUnencryptedHdr = pStrm->Read(pIn, nUnencryptedHdr);

                        DecryptRC4(aCtx, *pStrm, aDecryptMain);

                        aDecryptMain.Seek(0);
                        aDecryptMain.Write(pIn, nUnencryptedHdr);
                        delete [] pIn;


                        pTempTable = MakeTemp(aDecryptTable);
                        DecryptRC4(aCtx, *pTableStream, aDecryptTable);
                        pTableStream = &aDecryptTable;

                        if (!pDataStream || pDataStream == pStrm)
                            pDataStream = &aDecryptMain;
                        else
                        {
                            pTempData = MakeTemp(aDecryptData);
                            DecryptRC4(aCtx, *pDataStream, aDecryptData);
                            pDataStream = &aDecryptData;
                        }

                        pMedium->GetItemSet()->ClearItem( SID_PASSWORD );
                        pMedium->GetItemSet()->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData ) ) );
                    }
                }
                break;
            }
        }

        if (nErrRet == 0)
        {
            pStrm = &aDecryptMain;

            delete pWwFib;
            pWwFib = new WW8Fib(*pStrm, nWantedVersion);
            if (pWwFib->nFibError)
                nErrRet = ERR_SWG_READ_ERROR;
        }
    }

    if (!nErrRet)
        nErrRet = CoreLoad(pGloss, *rPaM.GetPoint());

    delete pTempMain;
    delete pTempTable;
    delete pTempData;

    if (!pGloss)
        delete pWwFib;
    return nErrRet;
}

class outlineeq : public std::unary_function<const SwTxtFmtColl*, bool>
{
private:
    sal_uInt8 mnNum;
public:
    outlineeq(sal_uInt8 nNum) : mnNum(nNum) {}
    bool operator()(const SwTxtFmtColl *pTest) const
    {
        return pTest->IsAssignedToListLevelOfOutlineStyle() && pTest->GetAssignedOutlineStyleLevel() == mnNum;  //<-end,zhaojianwei
    }
};

void SwWW8ImplReader::SetOutLineStyles()
{
    /*
    #i3674# - Load new document and insert document cases.
    */
    SwNumRule aOutlineRule(*rDoc.GetOutlineNumRule());
    // #i53044,i53213#
    // <mpChosenOutlineNumRule> has to be set to point to local variable
    // <aOutlineRule>, because its used below to be compared this <&aOutlineRule>.
    // But at the end of the method <mpChosenOutlineNumRule> has to be set to
    // <rDoc.GetOutlineNumRule()>, because <aOutlineRule> will be destroyed.
    mpChosenOutlineNumRule = &aOutlineRule;

    sw::ParaStyles aOutLined(sw::util::GetParaStyles(rDoc));
    // #i98791# - sorting algorithm adjusted
    sw::util::SortByAssignedOutlineStyleListLevel(aOutLined);

    typedef sw::ParaStyleIter myParaStyleIter;
    /*
    If we are inserted into a document then don't clobber existing existing
    levels.
    */
    sal_uInt16 nFlagsStyleOutlLevel = 0;
    if (!mbNewDoc)
    {
        // #i70748# - backward iteration needed due to the outline level attribute
        sw::ParaStyles::reverse_iterator aEnd = aOutLined.rend();
        for ( sw::ParaStyles::reverse_iterator aIter = aOutLined.rbegin(); aIter < aEnd; ++aIter)
        {
            if ((*aIter)->IsAssignedToListLevelOfOutlineStyle())
                nFlagsStyleOutlLevel |= 1 << (*aIter)->GetAssignedOutlineStyleLevel();
            else
                break;
        }
    }
    else
    {
        /*
        Only import *one* of the possible multiple outline numbering rules, so
        pick the one that affects most styles. If we're not importing a new
        document, we got to stick with what is already there.
        */
        // use index in text format collection array <vColl>
        // as key of the outline numbering map <aRuleMap>
        // instead of the memory pointer of the outline numbering rule
        // to assure that, if two outline numbering rule affect the same
        // count of text formats, always the same outline numbering rule is chosen.
        std::map<sal_uInt16, int>aRuleMap;
        typedef std::map<sal_uInt16, int>::iterator myIter;
        for (sal_uInt16 nI = 0; nI < vColl.size(); ++nI)
        {
            SwWW8StyInf& rSI = vColl[ nI ];
            if (
                (MAXLEVEL > rSI.nOutlineLevel) && rSI.pOutlineNumrule &&
                rSI.pFmt
               )
            {
                myIter aIter = aRuleMap.find(nI);
                if (aIter == aRuleMap.end())
                {
                    aRuleMap[nI] = 1;
                }
                else
                    ++(aIter->second);
            }
        }

        int nMax = 0;
        myIter aEnd2 = aRuleMap.end();
        for (myIter aIter = aRuleMap.begin(); aIter != aEnd2; ++aIter)
        {
            if (aIter->second > nMax)
            {
                nMax = aIter->second;
                if(aIter->first < vColl.size())
                    mpChosenOutlineNumRule = vColl[ aIter->first ].pOutlineNumrule;
                else
                    mpChosenOutlineNumRule = 0; //TODO make sure this is what we want
            }
        }

        OSL_ENSURE(mpChosenOutlineNumRule, "Impossible");
        if (mpChosenOutlineNumRule)
            aOutlineRule = *mpChosenOutlineNumRule;

        if (mpChosenOutlineNumRule != &aOutlineRule)
        {
            // #i70748# - backward iteration needed due to the outline level attribute
            sw::ParaStyles::reverse_iterator aEnd = aOutLined.rend();
            for ( sw::ParaStyles::reverse_iterator aIter = aOutLined.rbegin(); aIter < aEnd; ++aIter)
            {
                if((*aIter)->IsAssignedToListLevelOfOutlineStyle())
                    (*aIter)->DeleteAssignmentToListLevelOfOutlineStyle();

                else
                    break;
            }
        }
    }

    sal_uInt16 nOldFlags = nFlagsStyleOutlLevel;

    for (sal_uInt16 nI = 0; nI < vColl.size(); ++nI)
    {
        SwWW8StyInf& rSI = vColl[nI];

        if (rSI.IsOutlineNumbered())
        {
            sal_uInt16 nAktFlags = 1 << rSI.nOutlineLevel;
            if (
                 (nAktFlags & nFlagsStyleOutlLevel) ||
                 (rSI.pOutlineNumrule != mpChosenOutlineNumRule)
               )
            {
                /*
                If our spot is already taken by something we can't replace
                then don't insert and remove our outline level.
                */
                rSI.pFmt->SetFmtAttr(
                        SwNumRuleItem( rSI.pOutlineNumrule->GetName() ) );
                ((SwTxtFmtColl*)rSI.pFmt)->DeleteAssignmentToListLevelOfOutlineStyle(); // #outline level
            }
            else
            {
                /*
                If there is a style already set for this outline
                numbering level and its not a style set by us already
                then we can remove it outline numbering.
                (its one of the default headings in a new document
                so we can clobber it)
                Of course if we are being inserted into a document that
                already has some set we can't do this, thats covered by
                the list of level in nFlagsStyleOutlLevel to ignore.
                */
                outlineeq aCmp(rSI.nOutlineLevel);
                myParaStyleIter aResult = std::find_if(aOutLined.begin(),
                    aOutLined.end(), aCmp);

                myParaStyleIter aEnd = aOutLined.end();
                while (aResult != aEnd  && aCmp(*aResult))
                {
                    (*aResult)->DeleteAssignmentToListLevelOfOutlineStyle();
                    ++aResult;
                }

                /*
                #i1886#
                I believe that when a list is registered onto a winword
                style which is an outline numbering style (i.e.
                nOutlineLevel is set) that the style of numbering is for
                the level is indexed by the *list* level that was
                registered on that style, and not the outlinenumbering
                level, which is probably a logical sequencing, and not a
                physical mapping into the list style reged on that outline
                style.
                */
                sal_uInt8 nFromLevel = rSI.nListLevel;
                sal_uInt8 nToLevel = rSI.nOutlineLevel;
                const SwNumFmt& rRule=rSI.pOutlineNumrule->Get(nFromLevel);
                aOutlineRule.Set(nToLevel, rRule);
                ((SwTxtFmtColl*)rSI.pFmt)->AssignToListLevelOfOutlineStyle(nToLevel);
                // If there are more styles on this level ignore them
                nFlagsStyleOutlLevel |= nAktFlags;
            }
        }
    }
    if (nOldFlags != nFlagsStyleOutlLevel)
        rDoc.SetOutlineNumRule(aOutlineRule);
    // #i53044,i53213#
    if ( mpChosenOutlineNumRule == &aOutlineRule )
    {
        mpChosenOutlineNumRule = rDoc.GetOutlineNumRule();
    }
}

const String* SwWW8ImplReader::GetAnnotationAuthor(sal_uInt16 nIdx)
{
    if (!mpAtnNames && pWwFib->lcbGrpStAtnOwners)
    {
        // Determine authors: can be found in the TableStream
        mpAtnNames = new ::std::vector<String>;
        SvStream& rStrm = *pTableStream;

        long nOldPos = rStrm.Tell();
        rStrm.Seek( pWwFib->fcGrpStAtnOwners );

        long nRead = 0, nCount = pWwFib->lcbGrpStAtnOwners;
        while (nRead < nCount)
        {
            if( bVer67 )
            {
                mpAtnNames->push_back(read_uInt8_PascalString(rStrm,
                    RTL_TEXTENCODING_MS_1252));
                nRead += mpAtnNames->rbegin()->Len() + 1; // Length + sal_uInt8 count
            }
            else
            {
                mpAtnNames->push_back(read_uInt16_PascalString(rStrm));
                // Unicode: double the length + sal_uInt16 count
                nRead += mpAtnNames->rbegin()->Len() * 2 + 2;
            }
        }
        rStrm.Seek( nOldPos );
    }

    const String *pRet = 0;
    if (mpAtnNames && nIdx < mpAtnNames->size())
        pRet = &((*mpAtnNames)[nIdx]);
    return pRet;
}

int SwWW8ImplReader::GetAnnotationIndex(sal_uInt32 nTag)
{
    if (!mpAtnIndexes.get() && pWwFib->lcbSttbfAtnbkmk)
    {
        mpAtnIndexes.reset(new std::map<sal_uInt32, int>());
        std::vector<String> aStrings;
        std::vector<ww::bytes> aEntries;
        WW8ReadSTTBF(!bVer67, *pTableStream, pWwFib->fcSttbfAtnbkmk, pWwFib->lcbSttbfAtnbkmk, sizeof(struct WW8_ATNBE), eStructCharSet, aStrings, &aEntries);
        for (size_t i = 0; i < aStrings.size() && i < aEntries.size(); ++i)
        {
            ww::bytes aEntry = aEntries[i];
            WW8_ATNBE* pAtnbeStruct = (WW8_ATNBE*)(&aEntry[0]);
            mpAtnIndexes->insert(std::pair<sal_uInt32, int>(SVBT32ToUInt32(pAtnbeStruct->nTag), i));
        }
    }
    if (mpAtnIndexes.get())
    {
        std::map<sal_uInt32, int>::iterator it = mpAtnIndexes->find(nTag);
        if (it != mpAtnIndexes->end())
            return it->second;
    }
    return -1;
}

WW8_CP SwWW8ImplReader::GetAnnotationStart(int nIndex)
{
    if (!mpAtnStarts.get() && pWwFib->lcbPlcfAtnbkf)
        // A PLCFBKF is a PLC whose data elements are FBKF structures (4 bytes each).
        mpAtnStarts.reset(new WW8PLCFspecial(pTableStream, pWwFib->fcPlcfAtnbkf, pWwFib->lcbPlcfAtnbkf, 4));

    if (mpAtnStarts.get())
        return mpAtnStarts->GetPos(nIndex);
    else
        return SAL_MAX_INT32;
}

WW8_CP SwWW8ImplReader::GetAnnotationEnd(int nIndex)
{
    if (!mpAtnEnds.get() && pWwFib->lcbPlcfAtnbkl)
        // The Plcfbkl structure is a PLC that contains only CPs and no additional data.
        mpAtnEnds.reset(new WW8PLCFspecial(pTableStream, pWwFib->fcPlcfAtnbkl, pWwFib->lcbPlcfAtnbkl, 0));

    if (mpAtnEnds.get())
        return mpAtnEnds->GetPos(nIndex);
    else
        return SAL_MAX_INT32;
}

sal_uLong SwWW8ImplReader::LoadDoc( SwPaM& rPaM,WW8Glossary *pGloss)
{
    sal_uLong nErrRet = 0;

    {
        static const sal_Char* aNames[ 13 ] = {
            "WinWord/WW", "WinWord/WW8", "WinWord/WWFT",
            "WinWord/WWFLX", "WinWord/WWFLY",
            "WinWord/WWF",
            "WinWord/WWFA0", "WinWord/WWFA1", "WinWord/WWFA2",
            "WinWord/WWFB0", "WinWord/WWFB1", "WinWord/WWFB2",
            "WinWord/RegardHindiDigits"
        };
        sal_uInt32 aVal[ 13 ];

        SwFilterOptions aOpt( 13, aNames, aVal );

        nIniFlags = aVal[ 0 ];
        nIniFlags1= aVal[ 1 ];
        // Moves Flys by x twips to the right or left
        nIniFlyDx = aVal[ 3 ];
        nIniFlyDy = aVal[ 4 ];

        nFieldFlags = aVal[ 5 ];
        nFieldTagAlways[0] = aVal[ 6 ];
        nFieldTagAlways[1] = aVal[ 7 ];
        nFieldTagAlways[2] = aVal[ 8 ];
        nFieldTagBad[0] = aVal[ 9 ];
        nFieldTagBad[1] = aVal[ 10 ];
        nFieldTagBad[2] = aVal[ 11 ];
        m_bRegardHindiDigits = aVal[ 12 ] > 0;
    }

    sal_uInt16 nMagic(0);
    *pStrm >> nMagic;

    // Remember: 6 means "6 OR 7", 7 means "JUST 7"
    switch (nWantedVersion)
    {
        case 6:
        case 7:
            if (
                (0xa5dc != nMagic && 0xa5db != nMagic) &&
                (nMagic < 0xa697 || nMagic > 0xa699)
               )
            {
                // Test for own 97 fake!
                if (pStg && 0xa5ec == nMagic)
                {
                    sal_uLong nCurPos = pStrm->Tell();
                    if (pStrm->Seek(nCurPos + 22))
                    {
                        sal_uInt32 nfcMin;
                        *pStrm >> nfcMin;
                        if (0x300 != nfcMin)
                            nErrRet = ERR_WW6_NO_WW6_FILE_ERR;
                    }
                    pStrm->Seek( nCurPos );
                }
                else
                    nErrRet = ERR_WW6_NO_WW6_FILE_ERR;
            }
            break;
        case 8:
            if (0xa5ec != nMagic)
                nErrRet = ERR_WW8_NO_WW8_FILE_ERR;
            break;
        default:
            nErrRet = ERR_WW8_NO_WW8_FILE_ERR;
            OSL_ENSURE( !this, "We forgot to encode nVersion!" );
            break;
    }

    if (!nErrRet)
        nErrRet = LoadThroughDecryption(rPaM ,pGloss);

    rDoc.PropagateOutlineRule();

    return nErrRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT Reader* SAL_CALL ImportDOC()
{
    return new WW8Reader();
}

sal_uLong WW8Reader::OpenMainStream( SvStorageStreamRef& rRef, sal_uInt16& rBuffSize )
{
    sal_uLong nRet = ERR_SWG_READ_ERROR;
    OSL_ENSURE( pStg, "Where is my Storage?" );
    rRef = pStg->OpenSotStream( OUString("WordDocument"), STREAM_READ | STREAM_SHARE_DENYALL);

    if( rRef.Is() )
    {
        if( SVSTREAM_OK == rRef->GetError() )
        {
            sal_uInt16 nOld = rRef->GetBufferSize();
            rRef->SetBufferSize( rBuffSize );
            rBuffSize = nOld;
            nRet = 0;
        }
        else
            nRet = rRef->GetError();
    }
    return nRet;
}

sal_uLong WW8Reader::Read(SwDoc &rDoc, const OUString& rBaseURL, SwPaM &rPam, const OUString & /* FileName */)
{
    sal_uInt16 nOldBuffSize = 32768;
    bool bNew = !bInsertMode; // New Doc (no inserting)


    SvStorageStreamRef refStrm; // So that no one else can steal the Stream
    SvStream* pIn = pStrm;

    sal_uLong nRet = 0;
    sal_uInt8 nVersion = 8;

    const OUString sFltName = GetFltName();
    if ( sFltName=="WW6" )
    {
        if (pStrm)
            nVersion = 6;
        else
        {
            OSL_ENSURE(!this, "WinWord 95 Reader-Read without Stream");
            nRet = ERR_SWG_READ_ERROR;
        }
    }
    else
    {
        if ( sFltName=="CWW6" )
            nVersion = 6;
        else if ( sFltName=="CWW7" )
            nVersion = 7;

        if( pStg )
        {
            nRet = OpenMainStream( refStrm, nOldBuffSize );
            pIn = &refStrm;
        }
        else
        {
            OSL_ENSURE(!this, "WinWord 95/97 Reader-Read without Storage");
            nRet = ERR_SWG_READ_ERROR;
        }
    }

    if( !nRet )
    {
        if (bNew)
        {
            // Remove Frame and offsets from Frame Template
            Reader::ResetFrmFmts( rDoc );
        }
        SwWW8ImplReader* pRdr = new SwWW8ImplReader(nVersion, pStg, pIn, rDoc,
            rBaseURL, bNew);
        try
        {
            nRet = pRdr->LoadDoc( rPam );
        }
        catch( const std::exception& )
        {
            nRet = ERR_WW8_NO_WW8_FILE_ERR;
        }
        delete pRdr;

        if( refStrm.Is() )
        {
            refStrm->SetBufferSize( nOldBuffSize );
            refStrm.Clear();
        }
        else if (pIn)
            pIn->ResetError();

    }
    return nRet;
}

int WW8Reader::GetReaderType()
{
    return SW_STORAGE_READER | SW_STREAM_READER;
}

sal_Bool WW8Reader::HasGlossaries() const
{
    return true;
}

sal_Bool WW8Reader::ReadGlossaries(SwTextBlocks& rBlocks, sal_Bool bSaveRelFiles) const
{
    bool bRet=false;

    WW8Reader *pThis = const_cast<WW8Reader *>(this);

    sal_uInt16 nOldBuffSize = 32768;
    SvStorageStreamRef refStrm;
    if (!pThis->OpenMainStream(refStrm, nOldBuffSize))
    {
        WW8Glossary aGloss( refStrm, 8, pStg );
        bRet = aGloss.Load( rBlocks, bSaveRelFiles ? true : false);
    }
    return bRet ? true : false;
}

sal_Bool SwMSDffManager::GetOLEStorageName(long nOLEId, OUString& rStorageName,
    SvStorageRef& rSrcStorage, uno::Reference < embed::XStorage >& rDestStorage) const
{
    bool bRet = false;

    sal_Int32 nPictureId = 0;
    if (rReader.pStg)
    {
        // Via the TextBox-PLCF we get the right char Start-End positions
        // We should then find the EmbedField and the corresponding Sprms
        // in that Area.
        // We only need the Sprm for the Picture Id.
        long nOldPos = rReader.pStrm->Tell();
        {
            // #i32596# - consider return value of method
            // <rReader.GetTxbxTextSttEndCp(..)>. If it returns false, method
            // wasn't successful. Thus, continue in this case.
            // Note: Ask MM for initialization of <nStartCp> and <nEndCp>.
            // Note: Ask MM about assertions in method <rReader.GetTxbxTextSttEndCp(..)>.
            WW8_CP nStartCp, nEndCp;
            if ( rReader.GetTxbxTextSttEndCp(nStartCp, nEndCp,
                            static_cast<sal_uInt16>((nOLEId >> 16) & 0xFFFF),
                            static_cast<sal_uInt16>(nOLEId & 0xFFFF)) )
            {
                WW8PLCFxSaveAll aSave;
                memset( &aSave, 0, sizeof( aSave ) );
                rReader.pPlcxMan->SaveAllPLCFx( aSave );

                nStartCp += rReader.nDrawCpO;
                nEndCp   += rReader.nDrawCpO;
                WW8PLCFx_Cp_FKP* pChp = rReader.pPlcxMan->GetChpPLCF();
                wwSprmParser aSprmParser(rReader.pWwFib->GetFIBVersion());
                while (nStartCp <= nEndCp && !nPictureId)
                {
                    WW8PLCFxDesc aDesc;
                    pChp->SeekPos( nStartCp );
                    pChp->GetSprms( &aDesc );

                    if (aDesc.nSprmsLen && aDesc.pMemPos) // Attributes present
                    {
                        long nLen = aDesc.nSprmsLen;
                        const sal_uInt8* pSprm = aDesc.pMemPos;

                        while (nLen >= 2 && !nPictureId)
                        {
                            sal_uInt16 nId = aSprmParser.GetSprmId(pSprm);
                            sal_uInt16 nSL = aSprmParser.GetSprmSize(nId, pSprm);

                            if( nLen < nSL )
                                break; // Not enough Bytes left

                            if( 0x6A03 == nId && 0 < nLen )
                            {
                                nPictureId = SVBT32ToUInt32(pSprm +
                                    aSprmParser.DistanceToData(nId));
                                bRet = true;
                            }
                            pSprm += nSL;
                            nLen -= nSL;
                        }
                    }
                    nStartCp = aDesc.nEndPos;
                }

                rReader.pPlcxMan->RestoreAllPLCFx( aSave );
            }
        }
        rReader.pStrm->Seek( nOldPos );
    }

    if( bRet )
    {
        rStorageName = OUString('_');
        rStorageName += OUString::valueOf(nPictureId);
        rSrcStorage = rReader.pStg->OpenSotStorage(OUString(
            SL::aObjectPool));
        if (!rReader.mpDocShell)
            bRet=false;
        else
            rDestStorage = rReader.mpDocShell->GetStorage();
    }
    return bRet;
}

/**
 * When reading a single Box (which possibly is part of a group), we do
 * not yet have enough information to decide whether we need it as a TextField
 * or not.
 * So convert all of them as a precaution.
 * FIXME: Actually implement this!
 */
sal_Bool SwMSDffManager::ShapeHasText(sal_uLong, sal_uLong) const
{
    return true;
}

bool SwWW8ImplReader::InEqualOrHigherApo(int nLvl) const
{
    if (nLvl)
        --nLvl;
    // #i60827# - check size of <maApos> to assure that <maApos.begin() + nLvl> can be performed.
    if ( sal::static_int_cast< sal_Int32>(nLvl) >= sal::static_int_cast< sal_Int32>(maApos.size()) )
    {
        return false;
    }
    mycApoIter aIter = std::find(maApos.begin() + nLvl, maApos.end(), true);
    if (aIter != maApos.end())
        return true;
    else
        return false;
}

bool SwWW8ImplReader::InEqualApo(int nLvl) const
{
    // If we are in a table, see if an apo was inserted at the level below the table.
    if (nLvl)
        --nLvl;
    if (nLvl < 0 || static_cast<size_t>(nLvl) >= maApos.size())
        return false;
    return maApos[nLvl];
}

namespace sw
{
    namespace hack
    {
        Position::Position(const SwPosition &rPos)
            : maPtNode(rPos.nNode), mnPtCntnt(rPos.nContent.GetIndex())
        {
        }

        Position::Position(const Position &rPos)
            : maPtNode(rPos.maPtNode), mnPtCntnt(rPos.mnPtCntnt)
        {
        }

        Position::operator SwPosition() const
        {
            SwPosition aRet(maPtNode);
            aRet.nContent.Assign(maPtNode.GetNode().GetCntntNode(), mnPtCntnt);
            return aRet;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
