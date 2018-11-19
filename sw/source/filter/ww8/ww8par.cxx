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

#include <config_features.h>

#include <sal/config.h>
#include <sal/log.hxx>

#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/ElementModes.hpp>

#include <i18nlangtag/languagetag.hxx>

#include <unotools/configmgr.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <rtl/random.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <sfx2/docinf.hxx>
#include <sfx2/request.hxx>
#include <sfx2/frame.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>

#include <comphelper/docpasswordrequest.hxx>
#include <comphelper/propertysequence.hxx>

#include <editeng/outlobj.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/editeng.hxx>
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
#include <ndtxt.hxx>
#include <pagedesc.hxx>
#include <paratr.hxx>
#include <fmtclbl.hxx>
#include <section.hxx>
#include <docsh.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentExternalData.hxx>
#include <docufld.hxx>
#include <swfltopt.hxx>
#include <viewsh.hxx>
#include <viewopt.hxx>
#include <shellres.hxx>
#include <mdiexp.hxx>
#include <swerror.h>
#include <swtable.hxx>
#include <fchrfmt.hxx>
#include <charfmt.hxx>
#include <unocrsr.hxx>
#include <IDocumentSettingAccess.hxx>
#include "sprmids.hxx"

#include <fltini.hxx>

#include "writerwordglue.hxx"

#include <ndgrf.hxx>
#include <editeng/editids.hrc>
#include <txtflcnt.hxx>
#include <fmtflcnt.hxx>
#include <txatbase.hxx>

#include "ww8par2.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <svl/itemiter.hxx>

#include <comphelper/processfactory.hxx>
#include <basic/basmgr.hxx>

#include "ww8toolbar.hxx"
#include <osl/file.hxx>

#include <breakit.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <iostream>
#include <dbgoutsw.hxx>
#endif

#include <svx/hlnkitem.hxx>
#include <sfx2/docfile.hxx>
#include <swdll.hxx>
#include "WW8Sttbf.hxx"
#include "WW8FibData.hxx"
#include <unordered_set>
#include <memory>

using namespace ::com::sun::star;
using namespace sw::util;
using namespace sw::types;
using namespace nsHdFtFlags;

#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <unotools/pathoptions.hxx>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>

#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <comphelper/sequenceashashmap.hxx>
#include <oox/ole/vbaproject.hxx>
#include <oox/ole/olestorage.hxx>
#include <comphelper/storagehelper.hxx>
#include <sfx2/DocumentMetadataAccess.hxx>
#include <tools/diagnose_ex.h>

static SwMacroInfo* GetMacroInfo( SdrObject* pObj )
{
    if ( pObj )
    {
        sal_uInt16 nCount = pObj->GetUserDataCount();
        for( sal_uInt16 i = 0; i < nCount; i++ )
        {
            SdrObjUserData* pData = pObj->GetUserData( i );
            if( pData && pData->GetInventor() == SdrInventor::ScOrSwDraw
                && pData->GetId() == SW_UD_IMAPDATA)
            {
                return dynamic_cast<SwMacroInfo*>(pData);
            }
        }
        SwMacroInfo* pData = new SwMacroInfo;
        pObj->AppendUserData(std::unique_ptr<SdrObjUserData>(pData));
        return pData;
    }

    return nullptr;
};

static void lclGetAbsPath(OUString& rPath, sal_uInt16 nLevel, SwDocShell const * pDocShell)
{
    OUStringBuffer aTmpStr;
    while( nLevel )
    {
        aTmpStr.append("../");
        --nLevel;
    }
    if (!aTmpStr.isEmpty())
        aTmpStr.append(rPath);
    else
        aTmpStr = rPath;

    if (!aTmpStr.isEmpty())
    {
        bool bWasAbs = false;
        rPath = pDocShell->GetMedium()->GetURLObject().smartRel2Abs( aTmpStr.makeStringAndClear(), bWasAbs ).GetMainURL( INetURLObject::DecodeMechanism::NONE );
        // full path as stored in SvxURLField must be encoded
    }
}

namespace
{
    void lclIgnoreUString32(SvStream& rStrm)
    {
        sal_uInt32 nChars(0);
        rStrm.ReadUInt32(nChars);
        nChars *= 2;
        rStrm.SeekRel(nChars);
    }
}

void SwWW8ImplReader::ReadEmbeddedData(SvStream& rStrm, SwDocShell const * pDocShell, struct HyperLinksTable& hlStr)
{
    // (0x01B8) HLINK
    // const sal_uInt16 WW8_ID_HLINK               = 0x01B8;
    const sal_uInt32 WW8_HLINK_BODY             = 0x00000001;   /// Contains file link or URL.
    const sal_uInt32 WW8_HLINK_ABS              = 0x00000002;   /// Absolute path.
    const sal_uInt32 WW8_HLINK_DESCR            = 0x00000014;   /// Description.
    const sal_uInt32 WW8_HLINK_MARK             = 0x00000008;   /// Text mark.
    const sal_uInt32 WW8_HLINK_FRAME            = 0x00000080;   /// Target frame.
    const sal_uInt32 WW8_HLINK_UNC              = 0x00000100;   /// UNC path.

    //sal_uInt8 maGuidStdLink[ 16 ] ={
    //    0xD0, 0xC9, 0xEA, 0x79, 0xF9, 0xBA, 0xCE, 0x11, 0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9, 0x0B };

    sal_uInt8 const aGuidUrlMoniker[ 16 ] = {
        0xE0, 0xC9, 0xEA, 0x79, 0xF9, 0xBA, 0xCE, 0x11, 0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9, 0x0B };

    sal_uInt8 const aGuidFileMoniker[ 16 ] = {
        0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 };

    sal_uInt8 aGuid[16];
    sal_uInt32 nFlags(0);

    rStrm.ReadBytes(aGuid, 16);
    rStrm.SeekRel( 4 );
    rStrm.ReadUInt32( nFlags );

    sal_uInt16 nLevel = 0;                  // counter for level to climb down in path
    std::unique_ptr< OUString > xLongName;    // link / file name
    std::unique_ptr< OUString > xShortName;   // 8.3-representation of file name
    std::unique_ptr< OUString > xTextMark;    // text mark

    // description (ignore)
    if( ::get_flag( nFlags, WW8_HLINK_DESCR ) )
        lclIgnoreUString32( rStrm );

    // target frame
    if( ::get_flag( nFlags, WW8_HLINK_FRAME ) )
    {
        hlStr.tarFrame = read_uInt32_lenPrefixed_uInt16s_ToOUString(rStrm);
    }

        // UNC path
    if( ::get_flag( nFlags, WW8_HLINK_UNC ) )
    {
        // MS-OSHARED: An unsigned integer that specifies the number of Unicode characters in the
        // string field, including the null-terminating character.
        sal_uInt32 nStrLen(0);
        rStrm.ReadUInt32(nStrLen);
        if (nStrLen)
        {
            xLongName.reset(new OUString(read_uInt16s_ToOUString(rStrm, nStrLen - 1)));
            rStrm.SeekRel(sizeof(sal_Unicode)); // skip null-byte at end
        }
        lclGetAbsPath( *xLongName, 0 , pDocShell);
    }
    // file link or URL
    else if( ::get_flag( nFlags, WW8_HLINK_BODY ) )
    {
        rStrm.ReadBytes(aGuid, 16);

        if( memcmp(aGuid, aGuidFileMoniker, 16) == 0 )
        {
            rStrm.ReadUInt16( nLevel );
            // MS-OSHARED: An unsigned integer that specifies the number of
            // ANSI characters in ansiPath, including the terminating NULL character
            sal_uInt32 nUnits = 0;
            rStrm.ReadUInt32(nUnits);
            if (nUnits)
            {
                OString sStr(read_uInt8s_ToOString(rStrm, nUnits - 1));
                rStrm.SeekRel(sizeof(sal_uInt8)); // skip null-byte at end
                xShortName.reset(new OUString(sStr.getStr(), sStr.getLength(), GetCharSetFromLanguage()));
            }
            rStrm.SeekRel( 24 );

            sal_uInt32 nStrLen(0);
            rStrm.ReadUInt32( nStrLen );
            if( nStrLen )
            {
                nStrLen = 0;
                rStrm.ReadUInt32( nStrLen );
                nStrLen /= 2;
                rStrm.SeekRel( 2 );
                // MS-OSHARED: This array MUST not include a terminating NULL character.
                xLongName.reset(new OUString(read_uInt16s_ToOUString(rStrm, nStrLen)));
                lclGetAbsPath( *xLongName, nLevel, pDocShell);
            }
            else
                lclGetAbsPath( *xShortName, nLevel, pDocShell);
        }
        else if( memcmp(aGuid, aGuidUrlMoniker, 16) == 0 )
        {
            // MS-OSHARED: An unsigned integer that specifies the size of this
            // structure in bytes, excluding the size of the length field. The
            // value of this field MUST be ... the byte size of the url
            // field (including the terminating NULL character)
            sal_uInt32 nStrLen(0);
            rStrm.ReadUInt32( nStrLen );
            nStrLen /= 2;
            if (nStrLen)
            {
                xLongName.reset(new OUString(read_uInt16s_ToOUString(rStrm, nStrLen - 1)));
                rStrm.SeekRel(sizeof(sal_Unicode)); // skip null-byte at end
            }
            if( !::get_flag( nFlags, WW8_HLINK_ABS ) )
                lclGetAbsPath( *xLongName, 0 ,pDocShell);
        }
        else
        {
            SAL_INFO("sw.ww8", "WW8Hyperlink::ReadEmbeddedData - unknown content GUID");
        }
    }

    // text mark
    if( ::get_flag( nFlags, WW8_HLINK_MARK ) )
    {
        xTextMark.reset(new OUString(read_uInt32_lenPrefixed_uInt16s_ToOUString(rStrm)));
    }

    if (!xLongName && xShortName.get())
    {
        xLongName.reset( new OUString );
        *xLongName += *xShortName;
    }
    else if (!xLongName && xTextMark.get())
        xLongName.reset( new OUString );

    if (xLongName)
    {
        if (xTextMark)
        {
            if (xLongName->isEmpty())
                *xTextMark = xTextMark->replace('!', '.');
            *xLongName += "#" + *xTextMark;
        }
        hlStr.hLinkAddr = *xLongName;
    }
}

class BasicProjImportHelper
{
    SwDocShell& mrDocShell;
    uno::Reference< uno::XComponentContext > mxCtx;
public:
    explicit BasicProjImportHelper( SwDocShell& rShell ) : mrDocShell( rShell )
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
            oox::ole::VbaProject aVbaPrj( mxCtx, mrDocShell.GetModel(), "Writer" );
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

class Sttb : public TBBase
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

    Sttb(Sttb const&) = delete;
    Sttb& operator=(Sttb const&) = delete;

public:
    Sttb();

    bool Read(SvStream &rS) override;
    OUString getStringAtIndex( sal_uInt32 );
};

Sttb::Sttb()
    : fExtend(0)
    , cData(0)
    , cbExtra(0)
{
}

bool Sttb::Read( SvStream& rS )
{
    SAL_INFO("sw.ww8", "stream pos " << rS.Tell());
    nOffSet = rS.Tell();
    rS.ReadUInt16( fExtend ).ReadUInt16( cData ).ReadUInt16( cbExtra );
    if ( cData )
    {
        //if they are all going to be empty strings, how many could there be
        const size_t nMaxPossibleRecords = rS.remainingSize() / sizeof(sal_uInt16);
        if (cData > nMaxPossibleRecords)
            return false;
        for ( sal_Int32 index = 0; index < cData; ++index )
        {
            SBBItem aItem;
            rS.ReadUInt16( aItem.cchData );
            aItem.data = read_uInt16s_ToOUString(rS, aItem.cchData);
            dataItems.push_back( aItem );
        }
    }
    return true;
}

OUString
Sttb::getStringAtIndex( sal_uInt32 index )
{
    OUString aRet;
    if ( index < dataItems.size() )
        aRet = dataItems[ index ].data;
    return aRet;

}

SwMSDffManager::SwMSDffManager( SwWW8ImplReader& rRdr, bool bSkipImages )
    : SvxMSDffManager(*rRdr.m_pTableStream, rRdr.GetBaseURL(), rRdr.m_xWwFib->m_fcDggInfo,
        rRdr.m_pDataStream, nullptr, 0, COL_WHITE, rRdr.m_pStrm, bSkipImages),
    rReader(rRdr), pFallbackStream(nullptr)
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
 */
// #i32596# - consider new parameter <_nCalledByGroup>
SdrObject* SwMSDffManager::ImportOLE( sal_uInt32 nOLEId,
                                      const Graphic& rGrf,
                                      const tools::Rectangle& rBoundRect,
                                      const tools::Rectangle& rVisArea,
                                      const int _nCalledByGroup ) const
{
    // #i32596# - no import of OLE object, if it's inside a group.
    // NOTE: This can be undone, if grouping of Writer fly frames is possible or
    // if drawing OLE objects are allowed in Writer.
    if ( _nCalledByGroup > 0 )
    {
        return nullptr;
    }

    SdrObject* pRet = nullptr;
    OUString sStorageName;
    tools::SvRef<SotStorage> xSrcStg;
    uno::Reference < embed::XStorage > xDstStg;
    if( GetOLEStorageName( nOLEId, sStorageName, xSrcStg, xDstStg ))
    {
        tools::SvRef<SotStorage> xSrc = xSrcStg->OpenSotStorage( sStorageName );
        OSL_ENSURE(rReader.m_xFormImpl, "No Form Implementation!");
        css::uno::Reference< css::drawing::XShape > xShape;
        if ( (!(rReader.m_bIsHeader || rReader.m_bIsFooter)) &&
            rReader.m_xFormImpl->ReadOCXStream(xSrc,&xShape,true))
        {
            pRet = GetSdrObjectFromXShape(xShape);
        }
        else
        {
            ErrCode nError = ERRCODE_NONE;
            pRet = CreateSdrOLEFromStorage(
                *pSdrModel,
                sStorageName,
                xSrcStg,
                xDstStg,
                rGrf,
                rBoundRect,
                rVisArea,
                pStData,
                nError,
                nSvxMSDffOLEConvFlags,
                css::embed::Aspects::MSOLE_CONTENT,
                rReader.GetBaseURL());
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
    pStData2 = nullptr;
}

void SwMSDffManager::EnableFallbackStream()
{
    pStData2 = pFallbackStream;
    aEscherBlipCache = aOldEscherBlipCache;
    aOldEscherBlipCache.clear();
    pFallbackStream = nullptr;
}

sal_uInt16 SwWW8ImplReader::GetToggleAttrFlags() const
{
    return m_xCtrlStck ? m_xCtrlStck->GetToggleAttrFlags() : 0;
}

sal_uInt16 SwWW8ImplReader::GetToggleBiDiAttrFlags() const
{
    return m_xCtrlStck ? m_xCtrlStck->GetToggleBiDiAttrFlags() : 0;
}

void SwWW8ImplReader::SetToggleAttrFlags(sal_uInt16 nFlags)
{
    if (m_xCtrlStck)
        m_xCtrlStck->SetToggleAttrFlags(nFlags);
}

void SwWW8ImplReader::SetToggleBiDiAttrFlags(sal_uInt16 nFlags)
{
    if (m_xCtrlStck)
        m_xCtrlStck->SetToggleBiDiAttrFlags(nFlags);
}

SdrObject* SwMSDffManager::ProcessObj(SvStream& rSt,
                                       DffObjData& rObjData,
                                       SvxMSDffClientData& rData,
                                       tools::Rectangle& rTextRect,
                                       SdrObject* pObj
                                       )
{
    if( !rTextRect.IsEmpty() )
    {
        SvxMSDffImportData& rImportData = static_cast<SvxMSDffImportData&>(rData);
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
            sal_uInt32 nBytesLeft = maShapeRecords.Current()->nRecLen;
            auto nAvailableBytes = rSt.remainingSize();
            if (nBytesLeft > nAvailableBytes)
            {
                SAL_WARN("sw.ww8", "Document claimed to have shape record of " << nBytesLeft << " bytes, but only " << nAvailableBytes << " available");
                nBytesLeft = nAvailableBytes;
            }
            while( 5 < nBytesLeft )
            {
                sal_uInt16 nPID(0);
                rSt.ReadUInt16(nPID);
                sal_uInt32 nUDData(0);
                rSt.ReadUInt32(nUDData);
                if (!rSt.good())
                    break;
                switch (nPID)
                {
                    case 0x038F: pImpRec->nXAlign = nUDData; break;
                    case 0x0390:
                        pImpRec->nXRelTo = nUDData;
                        break;
                    case 0x0391: pImpRec->nYAlign = nUDData; break;
                    case 0x0392:
                        pImpRec->nYRelTo = nUDData;
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
                    // overridden by something), so it probably matches o:hr from .docx.
                        pImpRec->isHorizontalRule = true;
                        break;
                }
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
                // a) it's a simple text object or
                // b) it's a rectangle with text and square wrapping.
                bIsSimpleDrawingTextBox =
                (
                    (pImpRec->eShapeType == mso_sptTextSimple) ||
                    (
                        (pImpRec->eShapeType == mso_sptRectangle)
                        && ShapeHasText(pImpRec->nShapeId, rObjData.rSpHd.GetRecBegFilePos() )
                    )
                );
            }

            // Distance of Textbox to its surrounding Autoshape
            sal_Int32 nTextLeft = GetPropertyValue( DFF_Prop_dxTextLeft, 91440);
            sal_Int32 nTextRight = GetPropertyValue( DFF_Prop_dxTextRight, 91440 );
            sal_Int32 nTextTop = GetPropertyValue( DFF_Prop_dyTextTop, 45720 );
            sal_Int32 nTextBottom = GetPropertyValue( DFF_Prop_dyTextBottom, 45720 );

            ScaleEmu( nTextLeft );
            ScaleEmu( nTextRight );
            ScaleEmu( nTextTop );
            ScaleEmu( nTextBottom );

            sal_Int32 nTextRotationAngle=0;
            bool bVerticalText = false;
            if ( IsProperty( DFF_Prop_txflTextFlow ) )
            {
                MSO_TextFlow eTextFlow = static_cast<MSO_TextFlow>(GetPropertyValue(
                    DFF_Prop_txflTextFlow, 0) & 0xFFFF);
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
                    break;
                    case mso_txflHorzN:
                    default :
                        break;
                }
            }

            if (nTextRotationAngle)
            {
                if (nTextRotationAngle == 9000)
                {
                    long nWidth = rTextRect.GetWidth();
                    rTextRect.SetRight( rTextRect.Left() + rTextRect.GetHeight() );
                    rTextRect.SetBottom( rTextRect.Top() + nWidth );

                    sal_Int32 nOldTextLeft = nTextLeft;
                    sal_Int32 nOldTextRight = nTextRight;
                    sal_Int32 nOldTextTop = nTextTop;
                    sal_Int32 nOldTextBottom = nTextBottom;

                    nTextLeft = nOldTextBottom;
                    nTextRight = nOldTextTop;
                    nTextTop = nOldTextLeft;
                    nTextBottom = nOldTextRight;
                }
                else if (nTextRotationAngle == 27000)
                {
                    long nWidth = rTextRect.GetWidth();
                    rTextRect.SetRight( rTextRect.Left() + rTextRect.GetHeight() );
                    rTextRect.SetBottom( rTextRect.Top() + nWidth );

                    sal_Int32 nOldTextLeft = nTextLeft;
                    sal_Int32 nOldTextRight = nTextRight;
                    sal_Int32 nOldTextTop = nTextTop;
                    sal_Int32 nOldTextBottom = nTextBottom;

                    nTextLeft = nOldTextTop;
                    nTextRight = nOldTextBottom;
                    nTextTop = nOldTextRight;
                    nTextBottom = nOldTextLeft;
                }
            }

            if (bIsSimpleDrawingTextBox)
            {
                SdrObject::Free( pObj );
                pObj = new SdrRectObj(
                    *pSdrModel,
                    OBJ_TEXT,
                    rTextRect);
            }

            // The vertical paragraph justification are contained within the
            // BoundRect so calculate it here
            tools::Rectangle aNewRect(rTextRect);
            aNewRect.AdjustBottom( -(nTextTop + nTextBottom) );
            aNewRect.AdjustRight( -(nTextLeft + nTextRight) );

            // Only if it's a simple Textbox, Writer can replace the Object
            // with a Frame, else
            if( bIsSimpleDrawingTextBox )
            {
                std::shared_ptr<SvxMSDffShapeInfo> const xTmpRec(
                        new SvxMSDffShapeInfo(0, pImpRec->nShapeId));

                SvxMSDffShapeInfos_ById::const_iterator const it =
                    GetShapeInfos()->find(xTmpRec);
                if (it != GetShapeInfos()->end())
                {
                    SvxMSDffShapeInfo& rInfo = **it;
                    pImpRec->bReplaceByFly   = rInfo.bReplaceByFly;
                }
            }

            if( bIsSimpleDrawingTextBox )
                ApplyAttributes( rSt, aSet, rObjData );

            if (GetPropertyValue(DFF_Prop_FitTextToShape, 0) & 2)
            {
                aSet.Put( makeSdrTextAutoGrowHeightItem( true ) );
                aSet.Put( makeSdrTextMinFrameHeightItem(
                    aNewRect.Bottom() - aNewRect.Top() ) );
                aSet.Put( makeSdrTextMinFrameWidthItem(
                    aNewRect.Right() - aNewRect.Left() ) );
            }
            else
            {
                aSet.Put( makeSdrTextAutoGrowHeightItem( false ) );
                aSet.Put( makeSdrTextAutoGrowWidthItem( false ) );
            }

            switch ( static_cast<MSO_WrapMode>(GetPropertyValue( DFF_Prop_WrapText, mso_wrapSquare )) )
            {
                case mso_wrapNone :
                    aSet.Put( makeSdrTextAutoGrowWidthItem( true ) );
                    pImpRec->bAutoWidth = true;
                break;
                case mso_wrapByPoints :
                    aSet.Put( makeSdrTextContourFrameItem( true ) );
                break;
                default:
                    ;
            }

            // Set distances on Textbox's margins
            aSet.Put( makeSdrTextLeftDistItem( nTextLeft ) );
            aSet.Put( makeSdrTextRightDistItem( nTextRight ) );
            aSet.Put( makeSdrTextUpperDistItem( nTextTop ) );
            aSet.Put( makeSdrTextLowerDistItem( nTextBottom ) );
            pImpRec->nDxTextLeft    = nTextLeft;
            pImpRec->nDyTextTop     = nTextTop;
            pImpRec->nDxTextRight   = nTextRight;
            pImpRec->nDyTextBottom  = nTextBottom;

            // Taking the correct default (which is mso_anchorTop)
            sal_uInt32 eTextAnchor =
                GetPropertyValue( DFF_Prop_anchorText, mso_anchorTop );

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

            if (pObj != nullptr)
            {
                pObj->SetMergedItemSet(aSet);

                if (bVerticalText)
                {
                    SdrTextObj *pTextObj = dynamic_cast< SdrTextObj* >(pObj);
                    if (pTextObj)
                        pTextObj->SetVerticalWriting(true);
                }

                if ( bIsSimpleDrawingTextBox )
                {
                    if ( nTextRotationAngle )
                    {
                        long nMinWH = rTextRect.GetWidth() < rTextRect.GetHeight() ?
                            rTextRect.GetWidth() : rTextRect.GetHeight();
                        nMinWH /= 2;
                        Point aPivot(rTextRect.TopLeft());
                        aPivot.AdjustX(nMinWH );
                        aPivot.AdjustY(nMinWH );
                        double a = nTextRotationAngle * nPi180;
                        pObj->NbcRotate(aPivot, nTextRotationAngle, sin(a), cos(a));
                    }
                }

                if ( ( ( rObjData.nSpFlags & ShapeFlag::FlipV ) || mnFix16Angle || nTextRotationAngle ) && dynamic_cast< SdrObjCustomShape* >( pObj ) )
                {
                    SdrObjCustomShape* pCustomShape = dynamic_cast< SdrObjCustomShape* >( pObj );
                    if (pCustomShape)
                    {
                        double fExtraTextRotation = 0.0;
                        if ( mnFix16Angle && !( GetPropertyValue( DFF_Prop_FitTextToShape, 0 ) & 4 ) )
                        {   // text is already rotated, we have to take back the object rotation if DFF_Prop_RotateText is false
                            fExtraTextRotation = -mnFix16Angle;
                        }
                        if ( rObjData.nSpFlags & ShapeFlag::FlipV )    // sj: in ppt the text is flipped, whereas in word the text
                        {                                       // remains unchanged, so we have to take back the flipping here
                            fExtraTextRotation += 18000.0;      // because our core will flip text if the shape is flipped.
                        }
                        fExtraTextRotation += nTextRotationAngle;
                        if ( !::basegfx::fTools::equalZero( fExtraTextRotation ) )
                        {
                            fExtraTextRotation /= 100.0;
                            SdrCustomShapeGeometryItem aGeometryItem( pCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                            const OUString sTextRotateAngle( "TextRotateAngle" );
                            css::beans::PropertyValue aPropVal;
                            aPropVal.Name = sTextRotateAngle;
                            aPropVal.Value <<= fExtraTextRotation;
                            aGeometryItem.SetPropertyValue( aPropVal );
                            pCustomShape->SetMergedItem( aGeometryItem );
                        }
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
            pObj = new SdrRectObj(
                *pSdrModel,
                rTextRect);

            SfxItemSet aSet( pSdrModel->GetItemPool() );
            ApplyAttributes( rSt, aSet, rObjData );

            const SfxPoolItem* pPoolItem=nullptr;
            SfxItemState eState = aSet.GetItemState( XATTR_FILLCOLOR,
                                                     false, &pPoolItem );
            if( SfxItemState::DEFAULT == eState )
                aSet.Put( XFillColorItem( OUString(), mnDefaultColor ) );
            pObj->SetMergedItemSet(aSet);
        }

        // Means that fBehindDocument is set
        if (GetPropertyValue(DFF_Prop_fPrint, 0) & 0x20)
            pImpRec->bDrawHell = true;
        else
            pImpRec->bDrawHell = false;
        if (GetPropertyValue(DFF_Prop_fPrint, 0) & 0x02)
            pImpRec->bHidden = true;
        pImpRec->nNextShapeId   = GetPropertyValue( DFF_Prop_hspNext, 0 );

        if ( nTextId )
        {
            pImpRec->aTextId.nTxBxS = static_cast<sal_uInt16>( nTextId >> 16 );
            pImpRec->aTextId.nSequence = static_cast<sal_uInt16>(nTextId);
        }

        pImpRec->nDxWrapDistLeft = GetPropertyValue(
                                    DFF_Prop_dxWrapDistLeft, 114935 ) / 635;
        pImpRec->nDyWrapDistTop = GetPropertyValue(
                                    DFF_Prop_dyWrapDistTop, 0 ) / 635;
        pImpRec->nDxWrapDistRight = GetPropertyValue(
                                    DFF_Prop_dxWrapDistRight, 114935 ) / 635;
        pImpRec->nDyWrapDistBottom = GetPropertyValue(
                                    DFF_Prop_dyWrapDistBottom, 0 ) / 635;
        // 16.16 fraction times total image width or height, as appropriate.

        if (SeekToContent(DFF_Prop_pWrapPolygonVertices, rSt))
        {
            pImpRec->pWrapPolygon.reset();

            sal_uInt16 nNumElemVert(0), nNumElemMemVert(0), nElemSizeVert(0);
            rSt.ReadUInt16( nNumElemVert ).ReadUInt16( nNumElemMemVert ).ReadUInt16( nElemSizeVert );
            bool bOk = false;
            if (nNumElemVert && ((nElemSizeVert == 8) || (nElemSizeVert == 4)))
            {
                //check if there is enough data in the file to make the
                //record sane
                bOk = rSt.remainingSize() / nElemSizeVert >= nNumElemVert;
            }
            if (bOk)
            {
                pImpRec->pWrapPolygon.reset( new tools::Polygon(nNumElemVert) );
                for (sal_uInt16 i = 0; i < nNumElemVert; ++i)
                {
                    sal_Int32 nX(0), nY(0);
                    if (nElemSizeVert == 8)
                        rSt.ReadInt32( nX ).ReadInt32( nY );
                    else
                    {
                        sal_Int16 nSmallX(0), nSmallY(0);
                        rSt.ReadInt16( nSmallX ).ReadInt16( nSmallY );
                        nX = nSmallX;
                        nY = nSmallY;
                    }
                    (*(pImpRec->pWrapPolygon))[i].setX( nX );
                    (*(pImpRec->pWrapPolygon))[i].setY( nY );
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

        sal_uInt32 nLineFlags = GetPropertyValue( DFF_Prop_fNoLineDrawDash, 0 );

        if ( !IsHardAttribute( DFF_Prop_fLine ) &&
             pImpRec->eShapeType == mso_sptPictureFrame )
        {
            nLineFlags &= ~0x08;
        }

        pImpRec->eLineStyle = (nLineFlags & 8)
                              ? static_cast<MSO_LineStyle>(GetPropertyValue(
                                                    DFF_Prop_lineStyle,
                                                    mso_lineSimple ))
                              : MSO_LineStyle(USHRT_MAX);
        pImpRec->eLineDashing = static_cast<MSO_LineDashing>(GetPropertyValue(
                                        DFF_Prop_lineDashing, mso_lineSolid ));

        pImpRec->nFlags = rObjData.nSpFlags;

        if( pImpRec->nShapeId )
        {
            // Complement Import Record List
            pImpRec->pObj = pObj;
            rImportData.insert(pImpRec);

            // Complement entry in Z Order List with a pointer to this Object
            // Only store objects which are not deep inside the tree
            if( ( rObjData.nCalledByGroup == 0 )
                ||
                ( (rObjData.nSpFlags & ShapeFlag::Group)
                 && (rObjData.nCalledByGroup < 2) )
              )
                StoreShapeOrder( pImpRec->nShapeId,
                                ( static_cast<sal_uLong>(pImpRec->aTextId.nTxBxS) << 16 )
                                    + pImpRec->aTextId.nSequence, pObj );
        }
        else
            delete pImpRec;
    }

    sal_uInt32 nBufferSize = GetPropertyValue( DFF_Prop_pihlShape, 0 );
    if( (0 < nBufferSize) && (nBufferSize <= 0xFFFF) && SeekToContent( DFF_Prop_pihlShape, rSt ) )
    {
        SvMemoryStream aMemStream;
        struct HyperLinksTable hlStr;
        sal_uInt16 nRawRecId,nRawRecSize;
        aMemStream.WriteUInt16( 0 ).WriteUInt16( nBufferSize );

        // copy from DFF stream to memory stream
        std::vector< sal_uInt8 > aBuffer( nBufferSize );
        if (rSt.ReadBytes(aBuffer.data(), nBufferSize) == nBufferSize)
        {
            aMemStream.WriteBytes(aBuffer.data(), nBufferSize);
            sal_uInt8 nStreamSize = aMemStream.TellEnd();
            aMemStream.Seek( STREAM_SEEK_TO_BEGIN );
            bool bRet = 4 <= nStreamSize;
            if( bRet )
                aMemStream.ReadUInt16( nRawRecId ).ReadUInt16( nRawRecSize );
            SwDocShell* pDocShell = rReader.m_pDocShell;
            if (pDocShell)
            {
                rReader.ReadEmbeddedData(aMemStream, pDocShell, hlStr);
            }
        }

        if (pObj && !hlStr.hLinkAddr.isEmpty())
        {
            SwMacroInfo* pInfo = GetMacroInfo( pObj );
            if( pInfo )
            {
                pInfo->SetShapeId( rObjData.nShapeId );
                pInfo->SetHlink( hlStr.hLinkAddr );
                if (!hlStr.tarFrame.isEmpty())
                    pInfo->SetTarFrame( hlStr.tarFrame );
                OUString aNameStr = GetPropertyString( DFF_Prop_wzName, rSt );
                if (!aNameStr.isEmpty())
                    pInfo->SetName( aNameStr );
            }
        }
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
        m_bCpxStyle = false;
        return;
    }
    sal_uInt16 nColl = 0;
    if (m_xWwFib->GetFIBVersion() <= ww::eWW2)
        nColl = *pData;
    else
        nColl = SVBT16ToShort(pData);
    if (nColl < m_vColl.size())
    {
        SetTextFormatCollAndListLevel( *m_pPaM, m_vColl[nColl] );
        m_bCpxStyle = true;
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
    OSL_ENSURE(RES_TXTATR_INPUTFIELD != rAttr.Which(), "probably don't want to put"
        "input fields into the control stack");
    OSL_ENSURE(RES_TXTATR_ANNOTATION != rAttr.Which(), "probably don't want to put"
        "annotations into the control stack");
    OSL_ENSURE(RES_FLTR_REDLINE != rAttr.Which(), "probably don't want to put"
        "redlines into the control stack");
    SwFltControlStack::NewAttr(rPos, rAttr);
}

SwFltStackEntry* SwWW8FltControlStack::SetAttr(const SwPosition& rPos, sal_uInt16 nAttrId,
    bool bTstEnde, long nHand, bool )
{
    SwFltStackEntry *pRet = nullptr;
    // Doing a textbox, and using the control stack only as a temporary
    // collection point for properties which will are not to be set into
    // the real document
    if (rReader.m_xPlcxMan && rReader.m_xPlcxMan->GetDoingDrawTextBox())
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

long GetListFirstLineIndent(const SwNumFormat &rFormat)
{
    OSL_ENSURE( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION,
            "<GetListFirstLineIndent> - misusage: position-and-space-mode does not equal LABEL_WIDTH_AND_POSITION" );

    SvxAdjust eAdj = rFormat.GetNumAdjust();
    long nReverseListIndented;
    if (eAdj == SvxAdjust::Right)
        nReverseListIndented = -rFormat.GetCharTextDistance();
    else if (eAdj == SvxAdjust::Center)
        nReverseListIndented = rFormat.GetFirstLineOffset()/2;
    else
        nReverseListIndented = rFormat.GetFirstLineOffset();
    return nReverseListIndented;
}

static long lcl_GetTrueMargin(const SvxLRSpaceItem &rLR, const SwNumFormat &rFormat,
    long &rFirstLinePos)
{
    OSL_ENSURE( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION,
            "<lcl_GetTrueMargin> - misusage: position-and-space-mode does not equal LABEL_WIDTH_AND_POSITION" );

    const long nBodyIndent = rLR.GetTextLeft();
    const long nFirstLineDiff = rLR.GetTextFirstLineOfst();
    rFirstLinePos = nBodyIndent + nFirstLineDiff;

    const auto nPseudoListBodyIndent = rFormat.GetAbsLSpace();
    const long nReverseListIndented = GetListFirstLineIndent(rFormat);
    long nExtraListIndent = nPseudoListBodyIndent + nReverseListIndented;

    return std::max<long>(nExtraListIndent, 0);
}

// #i103711#
// #i105414#
void SyncIndentWithList( SvxLRSpaceItem &rLR,
                         const SwNumFormat &rFormat,
                         const bool bFirstLineOfstSet,
                         const bool bLeftIndentSet )
{
    if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        long nWantedFirstLinePos;
        long nExtraListIndent = lcl_GetTrueMargin(rLR, rFormat, nWantedFirstLinePos);
        rLR.SetTextLeft(nWantedFirstLinePos - nExtraListIndent);
        rLR.SetTextFirstLineOfst(0);
    }
    else if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        if ( !bFirstLineOfstSet && bLeftIndentSet &&
             rFormat.GetFirstLineIndent() != 0 )
        {
            rLR.SetTextFirstLineOfst( rFormat.GetFirstLineIndent() );
        }
        else if ( bFirstLineOfstSet && !bLeftIndentSet &&
                  rFormat.GetIndentAt() != 0 )
        {
            rLR.SetTextLeft( rFormat.GetIndentAt() );
        }
        else if (!bFirstLineOfstSet && !bLeftIndentSet )
        {
            if ( rFormat.GetFirstLineIndent() != 0 )
            {
                rLR.SetTextFirstLineOfst( rFormat.GetFirstLineIndent() );
            }
            if ( rFormat.GetIndentAt() != 0 )
            {
                rLR.SetTextLeft( rFormat.GetIndentAt() );
            }
        }
    }
}

const SwNumFormat* SwWW8FltControlStack::GetNumFormatFromStack(const SwPosition &rPos,
    const SwTextNode &rTextNode)
{
    const SwNumFormat *pRet = nullptr;
    const SfxPoolItem *pItem = GetStackAttr(rPos, RES_FLTR_NUMRULE);
    if (pItem && rTextNode.GetNumRule())
    {
        if (rTextNode.IsCountedInList())
        {
            OUString sName(static_cast<const SfxStringItem*>(pItem)->GetValue());
            const SwNumRule *pRule = pDoc->FindNumRulePtr(sName);
            if (pRule)
                pRet = GetNumFormatFromSwNumRuleLevel(*pRule, rTextNode.GetActualListLevel());
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

void SwWW8ReferencedFltEndStack::SetAttrInDoc( const SwPosition& rTmpPos,
                                               SwFltStackEntry& rEntry )
{
    switch( rEntry.pAttr->Which() )
    {
    case RES_FLTR_BOOKMARK:
        {
            // suppress insertion of bookmark, which is recognized as an internal bookmark used for table-of-content
            // and which is not referenced.
            bool bInsertBookmarkIntoDoc = true;

            SwFltBookmark* pFltBookmark = dynamic_cast<SwFltBookmark*>(rEntry.pAttr.get());
            if ( pFltBookmark != nullptr && pFltBookmark->IsTOCBookmark() )
            {
                const OUString& rName = pFltBookmark->GetName();
                std::set< OUString, SwWW8::ltstr >::const_iterator aResult = aReferencedTOCBookmarks.find(rName);
                if ( aResult == aReferencedTOCBookmarks.end() )
                {
                    bInsertBookmarkIntoDoc = false;
                }
            }
            if ( bInsertBookmarkIntoDoc )
            {
                SwFltEndStack::SetAttrInDoc( rTmpPos, rEntry );
            }
            break;
        }
    default:
        SwFltEndStack::SetAttrInDoc( rTmpPos, rEntry );
        break;
    }

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
                SwPaM aRegion(rTmpPos);
                if (rEntry.MakeRegion(pDoc, aRegion, false))
                {
                    SvxLRSpaceItem aNewLR( *static_cast<SvxLRSpaceItem*>(rEntry.pAttr.get()) );
                    sal_uLong nStart = aRegion.Start()->nNode.GetIndex();
                    sal_uLong nEnd   = aRegion.End()->nNode.GetIndex();
                    for(; nStart <= nEnd; ++nStart)
                    {
                        SwNode* pNode = pDoc->GetNodes()[ nStart ];
                        if (!pNode || !pNode->IsTextNode())
                            continue;

                        SwContentNode* pNd = static_cast<SwContentNode*>(pNode);
                        SvxLRSpaceItem aOldLR = static_cast<const SvxLRSpaceItem&>(pNd->GetAttr(RES_LR_SPACE));

                        SwTextNode *pTextNode = static_cast<SwTextNode*>(pNode);

                        const SwNumFormat* pNum
                            = GetNumFormatFromStack(*aRegion.GetPoint(), *pTextNode);
                        if (!pNum)
                        {
                            pNum = GetNumFormatFromTextNode(*pTextNode);
                        }

                        if ( pNum )
                        {
                            // #i103711#
                            const bool bFirstLineIndentSet =
                                ( rReader.m_aTextNodesHavingFirstLineOfstSet.end() !=
                                    rReader.m_aTextNodesHavingFirstLineOfstSet.find( pNode ) );
                            // #i105414#
                            const bool bLeftIndentSet =
                                (  rReader.m_aTextNodesHavingLeftIndentSet.end() !=
                                    rReader.m_aTextNodesHavingLeftIndentSet.find( pNode ) );
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
            OSL_ENSURE(false, "What is a field doing in the control stack,"
                "probably should have been in the endstack");
            break;

        case RES_TXTATR_ANNOTATION:
            OSL_ENSURE(false, "What is a annotation doing in the control stack,"
                "probably should have been in the endstack");
            break;

        case RES_TXTATR_INPUTFIELD:
            OSL_ENSURE(false, "What is a input field doing in the control stack,"
                "probably should have been in the endstack");
            break;

        case RES_TXTATR_INETFMT:
            {
                SwPaM aRegion(rTmpPos);
                if (rEntry.MakeRegion(pDoc, aRegion, false))
                {
                    SwFrameFormat *pFrame;
                    // If we have just one single inline graphic then
                    // don't insert a field for the single frame, set
                    // the frames hyperlink field attribute directly.
                    if (nullptr != (pFrame = SwWW8ImplReader::ContainsSingleInlineGraphic(aRegion)))
                    {
                        const SwFormatINetFormat *pAttr = static_cast<const SwFormatINetFormat *>(
                            rEntry.pAttr.get());
                        SwFormatURL aURL;
                        aURL.SetURL(pAttr->GetValue(), false);
                        aURL.SetTargetFrameName(pAttr->GetTargetFrame());
                        pFrame->SetFormatAttr(aURL);
                    }
                    else
                    {
                        pDoc->getIDocumentContentOperations().InsertPoolItem(aRegion, *rEntry.pAttr);
                    }
                }
            }
            break;
        default:
            SwFltControlStack::SetAttrInDoc(rTmpPos, rEntry);
            break;
    }
}

const SfxPoolItem* SwWW8FltControlStack::GetFormatAttr(const SwPosition& rPos,
    sal_uInt16 nWhich)
{
    const SfxPoolItem *pItem = GetStackAttr(rPos, nWhich);
    if (!pItem)
    {
        SwContentNode const*const pNd = rPos.nNode.GetNode().GetContentNode();
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
                SfxItemState eState = SfxItemState::DEFAULT;
                if (const SfxItemSet *pSet = pNd->GetpSwAttrSet())
                    eState = pSet->GetItemState(RES_LR_SPACE, false);
                if (eState != SfxItemState::SET && rReader.m_nCurrentColl < rReader.m_vColl.size())
                    pItem = &(rReader.m_vColl[rReader.m_nCurrentColl].maWordLR);
            }

            /*
            If we're hunting for a character property, try and exact position
            within the text node for lookup
            */
            if (pNd->IsTextNode())
            {
                const sal_Int32 nPos = rPos.nContent.GetIndex();
                m_xScratchSet.reset(new SfxItemSet(pDoc->GetAttrPool(), {{nWhich, nWhich}}));
                if (pNd->GetTextNode()->GetParaAttr(*m_xScratchSet, nPos, nPos))
                    pItem = m_xScratchSet->GetItem(nWhich);
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
                  (rEntry.m_aMkPos.m_nContent <= aFltPos.m_nContent) &&
                  (rEntry.m_aPtPos.m_nContent > aFltPos.m_nContent)
                 )
               )
                /*
                 * e.g. half-open range [0-3) so asking for properties at 3
                 * means props that end at 3 are not included
                 */
            {
                return rEntry.pAttr.get();
            }
        }
    }
    return nullptr;
}

bool SwWW8FltRefStack::IsFootnoteEdnBkmField(
    const SwFormatField& rFormatField,
    sal_uInt16& rBkmNo)
{
    const SwField* pField = rFormatField.GetField();
    sal_uInt16 nSubType;
    if(pField && (SwFieldIds::GetRef == pField->Which())
        && ((REF_FOOTNOTE == (nSubType = pField->GetSubType())) || (REF_ENDNOTE  == nSubType))
        && !static_cast<const SwGetRefField*>(pField)->GetSetRefName().isEmpty())
    {
        const IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
        IDocumentMarkAccess::const_iterator_t ppBkmk =
            pMarkAccess->findMark( static_cast<const SwGetRefField*>(pField)->GetSetRefName() );
        if(ppBkmk != pMarkAccess->getAllMarksEnd())
        {
            // find Sequence No of corresponding Foot-/Endnote
            rBkmNo = ppBkmk - pMarkAccess->getAllMarksBegin();
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
        case RES_TXTATR_ANNOTATION:
        case RES_TXTATR_INPUTFIELD:
        {
            SwNodeIndex aIdx(rEntry.m_aMkPos.m_nNode, 1);
            SwPaM aPaM(aIdx, rEntry.m_aMkPos.m_nContent);

            SwFormatField& rFormatField   = *static_cast<SwFormatField*>(rEntry.pAttr.get());
            SwField* pField = rFormatField.GetField();

            if (!RefToVar(pField, rEntry))
            {
                sal_uInt16 nBkmNo;
                if( IsFootnoteEdnBkmField(rFormatField, nBkmNo) )
                {
                    ::sw::mark::IMark const * const pMark = (pDoc->getIDocumentMarkAccess()->getAllMarksBegin() + nBkmNo)->get();

                    const SwPosition& rBkMrkPos = pMark->GetMarkPos();

                    SwTextNode* pText = rBkMrkPos.nNode.GetNode().GetTextNode();
                    if( pText && rBkMrkPos.nContent.GetIndex() )
                    {
                        SwTextAttr* const pFootnote = pText->GetTextAttrForCharAt(
                            rBkMrkPos.nContent.GetIndex()-1, RES_TXTATR_FTN );
                        if( pFootnote )
                        {
                            sal_uInt16 nRefNo = static_cast<SwTextFootnote*>(pFootnote)->GetSeqRefNo();

                            static_cast<SwGetRefField*>(pField)->SetSeqNo( nRefNo );

                            if( pFootnote->GetFootnote().IsEndNote() )
                                static_cast<SwGetRefField*>(pField)->SetSubType(REF_ENDNOTE);
                        }
                    }
                }
            }

            pDoc->getIDocumentContentOperations().InsertPoolItem(aPaM, *rEntry.pAttr);
            MoveAttrs(*aPaM.GetPoint());
        }
        break;
        case RES_FLTR_TOX:
            SwFltEndStack::SetAttrInDoc(rTmpPos, rEntry);
            break;
        default:
        case RES_FLTR_BOOKMARK:
            OSL_ENSURE(false, "EndStck used with non field, not what we want");
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
        m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_PARATR_TABSTOP);
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

    WW8_TBD const * pTyp = reinterpret_cast<WW8_TBD const *>(pData + 2*nDel + 2*nIns + 2); // Type Array

    SvxTabStopItem aAttr(0, 0, SvxTabAdjust::Default, RES_PARATR_TABSTOP);

    const SwFormat * pSty = nullptr;
    sal_uInt16 nTabBase;
    if (m_pCurrentColl && m_nCurrentColl < m_vColl.size()) // StyleDef
    {
        nTabBase = m_vColl[m_nCurrentColl].m_nBase;
        if (nTabBase < m_vColl.size())  // Based On
            pSty = m_vColl[nTabBase].m_pFormat;
    }
    else
    { // Text
        nTabBase = m_nCurrentColl;
        if (m_nCurrentColl < m_vColl.size())
            pSty = m_vColl[m_nCurrentColl].m_pFormat;
        //TODO: figure out else here
    }

    bool bFound = false;
    std::unordered_set<size_t> aLoopWatch;
    while (pSty && !bFound)
    {
        const SfxPoolItem* pTabs;
        bFound = pSty->GetAttrSet().GetItemState(RES_PARATR_TABSTOP, false,
            &pTabs) == SfxItemState::SET;
        if( bFound )
            aAttr = *static_cast<const SvxTabStopItem*>(pTabs);
        else
        {
            sal_uInt16 nOldTabBase = nTabBase;
            // If based on another
            if (nTabBase < m_vColl.size())
                nTabBase = m_vColl[nTabBase].m_nBase;

            if (
                    nTabBase < m_vColl.size() &&
                    nOldTabBase != nTabBase &&
                    nTabBase != ww::stiNil
               )
            {
                // #i61789: Stop searching when next style is the same as the
                // current one (prevent loop)
                aLoopWatch.insert(reinterpret_cast<size_t>(pSty));
                if (nTabBase < m_vColl.size())
                    pSty = m_vColl[nTabBase].m_pFormat;
                //TODO figure out the else branch

                if (aLoopWatch.find(reinterpret_cast<size_t>(pSty)) !=
                    aLoopWatch.end())
                    pSty = nullptr;
            }
            else
                pSty = nullptr; // Give up on the search
        }
    }

    SvxTabStop aTabStop;
    for (short i=0; i < nDel; ++i)
    {
        sal_uInt16 nPos = aAttr.GetPos(SVBT16ToShort(pDel + i*2));
        if( nPos != SVX_TAB_NOTFOUND )
            aAttr.Remove( nPos );
    }

    for (short i=0; i < nIns; ++i)
    {
        short nPos = SVBT16ToShort(pIns + i*2);
        aTabStop.GetTabPos() = nPos;
        switch( pTyp[i].aBits1 & 0x7 ) // pTyp[i].jc
        {
            case 0:
                aTabStop.GetAdjustment() = SvxTabAdjust::Left;
                break;
            case 1:
                aTabStop.GetAdjustment() = SvxTabAdjust::Center;
                break;
            case 2:
                aTabStop.GetAdjustment() = SvxTabAdjust::Right;
                break;
            case 3:
                aTabStop.GetAdjustment() = SvxTabAdjust::Decimal;
                break;
            case 4:
                continue; // Ignore Bar
        }

        switch( pTyp[i].aBits1 >> 3 & 0x7 )
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
            aAttr.Remove(nPos2); // Or else Insert() refuses
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
        if (!m_pCurrentColl) // not importing into a style
        {
            SvxTabStopItem aOrig = pSty ?
            ItemGet<SvxTabStopItem>(*pSty, RES_PARATR_TABSTOP) :
            DefaultItemGet<SvxTabStopItem>(m_rDoc, RES_PARATR_TABSTOP);
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
        m_pDocShell->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocuProps(
        xDPS->getDocumentProperties());
    OSL_ENSURE(xDocuProps.is(), "DocumentProperties is null");
    if (xDocuProps.is())
    {
        DateTime aLastPrinted(
            msfilter::util::DTTM2DateTime(m_xWDop->dttmLastPrint));
        ::util::DateTime uDT = aLastPrinted.GetUNODateTime();
        xDocuProps->setPrintDate(uDT);
    }

    // COMPATIBILITY FLAGS START

    // #i78951# - remember the unknown compatibility options
    // so as to export them out
    m_rDoc.getIDocumentSettingAccess().Setn32DummyCompatibilityOptions1(m_xWDop->GetCompatibilityOptions());
    m_rDoc.getIDocumentSettingAccess().Setn32DummyCompatibilityOptions2(m_xWDop->GetCompatibilityOptions2());

    // The distance between two paragraphs is the sum of the bottom distance of
    // the first paragraph and the top distance of the second one
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::PARA_SPACE_MAX, m_xWDop->fDontUseHTMLAutoSpacing);
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::PARA_SPACE_MAX_AT_PAGES, true );
    // move tabs on alignment
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::TAB_COMPAT, true);
    // #i24363# tab stops relative to indent
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::TABS_RELATIVE_TO_INDENT, false);
    // tdf#117923
    m_rDoc.getIDocumentSettingAccess().set(
        DocumentSettingId::APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING, true);

    // Import Default Tabs
    long nDefTabSiz = m_xWDop->dxaTab;
    if( nDefTabSiz < 56 )
        nDefTabSiz = 709;

    // We want exactly one DefaultTab
    SvxTabStopItem aNewTab( 1, sal_uInt16(nDefTabSiz), SvxTabAdjust::Default, RES_PARATR_TABSTOP );
    const_cast<SvxTabStop&>(aNewTab[0]).GetAdjustment() = SvxTabAdjust::Default;

    m_rDoc.GetAttrPool().SetPoolDefaultItem( aNewTab );

    // Import zoom factor
    if (m_xWDop->wScaleSaved)
    {
        //Import zoom type
        sal_Int16 nZoomType;
        switch (m_xWDop->zkSaved) {
            case 1:  nZoomType = sal_Int16(SvxZoomType::WHOLEPAGE); break;
            case 2:  nZoomType = sal_Int16(SvxZoomType::PAGEWIDTH); break;
            case 3:  nZoomType = sal_Int16(SvxZoomType::OPTIMAL);   break;
            default: nZoomType = sal_Int16(SvxZoomType::PERCENT);   break;
        }
        uno::Sequence<beans::PropertyValue> aViewProps( comphelper::InitPropertySequence({
                { "ZoomFactor", uno::Any(sal_Int16(m_xWDop->wScaleSaved)) },
                { "VisibleBottom", uno::Any(sal_Int32(0)) },
                { "ZoomType", uno::Any(nZoomType) }
            }));

        uno::Reference< uno::XComponentContext > xComponentContext(comphelper::getProcessComponentContext());
        uno::Reference<container::XIndexContainer> xBox = document::IndexedPropertyValues::create(xComponentContext);
        xBox->insertByIndex(sal_Int32(0), uno::makeAny(aViewProps));
        uno::Reference<container::XIndexAccess> xIndexAccess(xBox, uno::UNO_QUERY);
        uno::Reference<document::XViewDataSupplier> xViewDataSupplier(m_pDocShell->GetModel(), uno::UNO_QUERY);
        xViewDataSupplier->setViewData(xIndexAccess);
    }

    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::USE_VIRTUAL_DEVICE, !m_xWDop->fUsePrinterMetrics);
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::USE_HIRES_VIRTUAL_DEVICE, true);
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::ADD_FLY_OFFSETS, true );
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::ADD_EXT_LEADING, !m_xWDop->fNoLeading);
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::OLD_NUMBERING, false);
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING, false); // #i47448#
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK, !m_xWDop->fExpShRtn); // #i49277#, #i56856#
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT, false);  // #i53199#
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::OLD_LINE_SPACING, false);

    // #i25901# - set new compatibility option
    //      'Add paragraph and table spacing at bottom of table cells'
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS, true);

    // #i11860# - set new compatibility option
    //      'Use former object positioning' to <false>
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::USE_FORMER_OBJECT_POS, false);

    // #i27767# - set new compatibility option
    //      'Consider Wrapping mode when positioning object' to <true>
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION, true);

    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::USE_FORMER_TEXT_WRAPPING, false); // #i13832#, #i24135#

    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::TABLE_ROW_KEEP, true); //SetTableRowKeep( true );

    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION, true); // #i3952#

    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::INVERT_BORDER_SPACING, true);
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::COLLAPSE_EMPTY_CELL_PARA, true);
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::TAB_OVERFLOW, true);
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::UNBREAKABLE_NUMBERINGS, true);
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::CLIPPED_PICTURES, true);
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::TAB_OVER_MARGIN, true);
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::SURROUND_TEXT_WRAP_SMALL, true);
    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::PROP_LINE_SPACING_SHRINKS_FIRST_LINE, true);

    // COMPATIBILITY FLAGS END

    // Import magic doptypography information, if its there
    if (m_xWwFib->m_nFib > 105)
        ImportDopTypography(m_xWDop->doptypography);

    // disable form design mode to be able to use imported controls directly
    // #i31239# always disable form design mode, not only in protected docs
    uno::Reference<beans::XPropertySet> xDocProps(m_pDocShell->GetModel(), uno::UNO_QUERY);
    if (xDocProps.is())
    {
        uno::Reference<beans::XPropertySetInfo> xInfo = xDocProps->getPropertySetInfo();
        if (xInfo.is())
        {
            if (xInfo->hasPropertyByName("ApplyFormDesignMode"))
                xDocProps->setPropertyValue("ApplyFormDesignMode", css::uno::makeAny(false));
        }
    }

    // Still allow editing of form fields.
    if (!m_xWDop->fProtEnabled)
        m_pDocShell->SetModifyPasswordHash(m_xWDop->lKeyProtDoc);
    else if ( xDocProps.is() )
    {
        comphelper::SequenceAsHashMap aGrabBag(xDocProps->getPropertyValue("InteropGrabBag"));
        aGrabBag["FormPasswordHash"] <<= m_xWDop->lKeyProtDoc;
        xDocProps->setPropertyValue("InteropGrabBag", uno::Any(aGrabBag.getAsConstPropertyValueList()));
    }

    const SvtFilterOptions& rOpt = SvtFilterOptions::Get();
    if (rOpt.IsUseEnhancedFields())
        m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::PROTECT_FORM, m_xWDop->fProtEnabled );
}

void SwWW8ImplReader::ImportDopTypography(const WW8DopTypography &rTypo)
{
    switch (rTypo.iLevelOfKinsoku)
    {
        case 2: // custom
            {
                i18n::ForbiddenCharacters aForbidden(rTypo.rgxchFPunct,
                    rTypo.rgxchLPunct);
                m_rDoc.getIDocumentSettingAccess().setForbiddenCharacters(rTypo.GetConvertedLang(),
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
        i18n::ForbiddenCharacters aForbidden(WW8DopTypography::GetJapanNotBeginLevel1(),
            WW8DopTypography::GetJapanNotEndLevel1());
        m_rDoc.getIDocumentSettingAccess().setForbiddenCharacters(LANGUAGE_JAPANESE,aForbidden);
    }

    m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::KERN_ASIAN_PUNCTUATION, bool(rTypo.fKerningPunct));
    m_rDoc.getIDocumentSettingAccess().setCharacterCompressionType(static_cast<CharCompressType>(rTypo.iJustification));
}

/**
 * Footnotes and Endnotes
 */
WW8ReaderSave::WW8ReaderSave(SwWW8ImplReader* pRdr ,WW8_CP nStartCp) :
    maTmpPos(*pRdr->m_pPaM->GetPoint()),
    mxOldStck(std::move(pRdr->m_xCtrlStck)),
    mxOldAnchorStck(std::move(pRdr->m_xAnchorStck)),
    mxOldRedlines(std::move(pRdr->m_xRedlineStack)),
    mxOldPlcxMan(pRdr->m_xPlcxMan),
    mpWFlyPara(std::move(pRdr->m_xWFlyPara)),
    mpSFlyPara(std::move(pRdr->m_xSFlyPara)),
    mpPreviousNumPaM(pRdr->m_pPreviousNumPaM),
    mpPrevNumRule(pRdr->m_pPrevNumRule),
    mxTableDesc(std::move(pRdr->m_xTableDesc)),
    mnInTable(pRdr->m_nInTable),
    mnCurrentColl(pRdr->m_nCurrentColl),
    mcSymbol(pRdr->m_cSymbol),
    mbIgnoreText(pRdr->m_bIgnoreText),
    mbSymbol(pRdr->m_bSymbol),
    mbHdFtFootnoteEdn(pRdr->m_bHdFtFootnoteEdn),
    mbTxbxFlySection(pRdr->m_bTxbxFlySection),
    mbAnl(pRdr->m_bAnl),
    mbInHyperlink(pRdr->m_bInHyperlink),
    mbPgSecBreak(pRdr->m_bPgSecBreak),
    mbWasParaEnd(pRdr->m_bWasParaEnd),
    mbHasBorder(pRdr->m_bHasBorder),
    mbFirstPara(pRdr->m_bFirstPara)
{
    pRdr->m_bSymbol = false;
    pRdr->m_bHdFtFootnoteEdn = true;
    pRdr->m_bTxbxFlySection = pRdr->m_bAnl = pRdr->m_bPgSecBreak = pRdr->m_bWasParaEnd
        = pRdr->m_bHasBorder = false;
    pRdr->m_bFirstPara = true;
    pRdr->m_nInTable = 0;
    pRdr->m_pPreviousNumPaM = nullptr;
    pRdr->m_pPrevNumRule = nullptr;
    pRdr->m_nCurrentColl = 0;

    pRdr->m_xCtrlStck.reset(new SwWW8FltControlStack(&pRdr->m_rDoc, pRdr->m_nFieldFlags,
        *pRdr));

    pRdr->m_xRedlineStack.reset(new sw::util::RedlineStack(pRdr->m_rDoc));

    pRdr->m_xAnchorStck.reset(new SwWW8FltAnchorStack(&pRdr->m_rDoc, pRdr->m_nFieldFlags));

    // Save the attribute manager: we need this as the newly created PLCFx Manager
    // access the same FKPs as the old one and their Start-End position changes.
    if (pRdr->m_xPlcxMan)
        pRdr->m_xPlcxMan->SaveAllPLCFx(maPLCFxSave);

    if (nStartCp != -1)
    {
        pRdr->m_xPlcxMan.reset(new WW8PLCFMan(pRdr->m_xSBase.get(),
            mxOldPlcxMan->GetManType(), nStartCp));
    }

    maOldApos.push_back(false);
    maOldApos.swap(pRdr->m_aApos);
    maOldFieldStack.swap(pRdr->m_aFieldStack);
}

void WW8ReaderSave::Restore( SwWW8ImplReader* pRdr )
{
    pRdr->m_xWFlyPara = std::move(mpWFlyPara);
    pRdr->m_xSFlyPara = std::move(mpSFlyPara);
    pRdr->m_pPreviousNumPaM = mpPreviousNumPaM;
    pRdr->m_pPrevNumRule = mpPrevNumRule;
    pRdr->m_xTableDesc = std::move(mxTableDesc);
    pRdr->m_cSymbol = mcSymbol;
    pRdr->m_bSymbol = mbSymbol;
    pRdr->m_bIgnoreText = mbIgnoreText;
    pRdr->m_bHdFtFootnoteEdn = mbHdFtFootnoteEdn;
    pRdr->m_bTxbxFlySection = mbTxbxFlySection;
    pRdr->m_nInTable = mnInTable;
    pRdr->m_bAnl = mbAnl;
    pRdr->m_bInHyperlink = mbInHyperlink;
    pRdr->m_bWasParaEnd = mbWasParaEnd;
    pRdr->m_bPgSecBreak = mbPgSecBreak;
    pRdr->m_nCurrentColl = mnCurrentColl;
    pRdr->m_bHasBorder = mbHasBorder;
    pRdr->m_bFirstPara = mbFirstPara;

    // Close all attributes as attributes could be created that extend the Fly
    pRdr->DeleteCtrlStack();
    pRdr->m_xCtrlStck = std::move(mxOldStck);

    pRdr->m_xRedlineStack->closeall(*pRdr->m_pPaM->GetPoint());
    pRdr->m_aFrameRedlines.emplace(std::move(pRdr->m_xRedlineStack));
    pRdr->m_xRedlineStack = std::move(mxOldRedlines);

    pRdr->DeleteAnchorStack();
    pRdr->m_xAnchorStck = std::move(mxOldAnchorStck);

    *pRdr->m_pPaM->GetPoint() = maTmpPos;

    if (mxOldPlcxMan != pRdr->m_xPlcxMan)
        pRdr->m_xPlcxMan = mxOldPlcxMan;
    if (pRdr->m_xPlcxMan)
        pRdr->m_xPlcxMan->RestoreAllPLCFx(maPLCFxSave);
    pRdr->m_aApos.swap(maOldApos);
    pRdr->m_aFieldStack.swap(maOldFieldStack);
}

void SwWW8ImplReader::Read_HdFtFootnoteText( const SwNodeIndex* pSttIdx,
    WW8_CP nStartCp, WW8_CP nLen, ManTypes nType )
{
    if (nStartCp < 0 || nLen < 0)
        return;

    // Saves Flags (amongst other things) and resets them
    WW8ReaderSave aSave( this );

    m_pPaM->GetPoint()->nNode = pSttIdx->GetIndex() + 1;
    m_pPaM->GetPoint()->nContent.Assign( m_pPaM->GetContentNode(), 0 );

    // Read Text for Header, Footer or Footnote
    ReadText( nStartCp, nLen, nType ); // Ignore Sepx when doing so
    aSave.Restore( this );
}

/**
 * Use authornames, if not available fall back to initials.
 */
long SwWW8ImplReader::Read_And(WW8PLCFManResult* pRes)
{
    WW8PLCFx_SubDoc* pSD = m_xPlcxMan->GetAtn();
    if (!pSD)
        return 0;

    const void* pData = pSD->GetData();
    if (!pData)
        return 0;

    OUString sAuthor;
    OUString sInitials;
    if( m_bVer67 )
    {
        const WW67_ATRD* pDescri = static_cast<const WW67_ATRD*>(pData);
        const OUString* pA = GetAnnotationAuthor(SVBT16ToShort(pDescri->ibst));
        if (pA)
            sAuthor = *pA;
        else
        {
            const sal_uInt8 nLen = std::min<sal_uInt8>(pDescri->xstUsrInitl[0],
                                                       SAL_N_ELEMENTS(pDescri->xstUsrInitl)-1);
            sAuthor = OUString(pDescri->xstUsrInitl + 1, nLen, RTL_TEXTENCODING_MS_1252);
        }
    }
    else
    {
        const WW8_ATRD* pDescri = static_cast<const WW8_ATRD*>(pData);
        {
            const sal_uInt16 nLen = std::min<sal_uInt16>(SVBT16ToShort(pDescri->xstUsrInitl[0]),
                                                         SAL_N_ELEMENTS(pDescri->xstUsrInitl)-1);
            OUStringBuffer aBuf;
            aBuf.setLength(nLen);
            for(sal_uInt16 nIdx = 1; nIdx <= nLen; ++nIdx)
                aBuf[nIdx-1] = SVBT16ToShort(pDescri->xstUsrInitl[nIdx]);
            sInitials = aBuf.makeStringAndClear();
        }

        if (const OUString* pA = GetAnnotationAuthor(SVBT16ToShort(pDescri->ibst)))
            sAuthor = *pA;
        else
            sAuthor = sInitials;
    }

    sal_uInt32 nDateTime = 0;

    if (sal_uInt8 * pExtended = m_xPlcxMan->GetExtendedAtrds()) // Word < 2002 has no date data for comments
    {
        sal_uLong nIndex = pSD->GetIdx() & 0xFFFF; // Index is (stupidly) multiplexed for WW8PLCFx_SubDocs
        if (m_xWwFib->m_lcbAtrdExtra/18 > nIndex)
            nDateTime = SVBT32ToUInt32(*reinterpret_cast<SVBT32*>(pExtended+(nIndex*18)));
    }

    DateTime aDate = msfilter::util::DTTM2DateTime(nDateTime);

    OUString sText;
    std::unique_ptr<OutlinerParaObject> pOutliner = ImportAsOutliner( sText, pRes->nCp2OrIdx,
        pRes->nCp2OrIdx + pRes->nMemLen, MAN_AND );

    m_pFormatOfJustInsertedApo = nullptr;
    SwPostItField aPostIt(
        static_cast<SwPostItFieldType*>(m_rDoc.getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::Postit)), sAuthor,
        sText, sInitials, OUString(), aDate );
    aPostIt.SetTextObject(std::move(pOutliner));

    SwPaM aEnd(*m_pPaM->End(), *m_pPaM->End());
    m_xCtrlStck->NewAttr(*aEnd.GetPoint(), SvxCharHiddenItem(false, RES_CHRATR_HIDDEN));
    m_rDoc.getIDocumentContentOperations().InsertPoolItem(aEnd, SwFormatField(aPostIt));
    m_xCtrlStck->SetAttr(*aEnd.GetPoint(), RES_CHRATR_HIDDEN);
    // If this is a range, make sure that it ends after the just inserted character, not before it.
    m_xReffedStck->MoveAttrs(*aEnd.GetPoint(), SwFltControlStack::MoveAttrsMode::POSTIT_INSERTED);

    return 0;
}

void SwWW8ImplReader::Read_HdFtTextAsHackedFrame(WW8_CP nStart, WW8_CP nLen,
    SwFrameFormat const &rHdFtFormat, sal_uInt16 nPageWidth)
{
    const SwNodeIndex* pSttIdx = rHdFtFormat.GetContent().GetContentIdx();
    OSL_ENSURE(pSttIdx, "impossible");
    if (!pSttIdx)
        return;

    SwPosition aTmpPos(*m_pPaM->GetPoint());

    m_pPaM->GetPoint()->nNode = pSttIdx->GetIndex() + 1;
    m_pPaM->GetPoint()->nContent.Assign(m_pPaM->GetContentNode(), 0);

    SwFlyFrameFormat *pFrame = m_rDoc.MakeFlySection(RndStdIds::FLY_AT_PARA, m_pPaM->GetPoint());

    SwFormatAnchor aAnch( pFrame->GetAnchor() );
    aAnch.SetType( RndStdIds::FLY_AT_PARA );
    pFrame->SetFormatAttr( aAnch );
    SwFormatFrameSize aSz(ATT_MIN_SIZE, nPageWidth, MINLAY);
    SwFrameSize eFrameSize = ATT_MIN_SIZE;
    if( eFrameSize != aSz.GetWidthSizeType() )
        aSz.SetWidthSizeType( eFrameSize );
    pFrame->SetFormatAttr(aSz);
    pFrame->SetFormatAttr(SwFormatSurround(css::text::WrapTextMode_THROUGH));
    pFrame->SetFormatAttr(SwFormatHoriOrient(0, text::HoriOrientation::LEFT)); //iFOO

    // #i43427# - send frame for header/footer into background.
    pFrame->SetFormatAttr( SvxOpaqueItem( RES_OPAQUE, false ) );
    SdrObject* pFrameObj = CreateContactObject( pFrame );
    OSL_ENSURE( pFrameObj,
            "<SwWW8ImplReader::Read_HdFtTextAsHackedFrame(..)> - missing SdrObject instance" );
    if ( pFrameObj )
    {
        pFrameObj->SetOrdNum( 0 );
    }
    MoveInsideFly(pFrame);

    const SwNodeIndex* pHackIdx = pFrame->GetContent().GetContentIdx();

    Read_HdFtFootnoteText(pHackIdx, nStart, nLen - 1, MAN_HDFT);

    MoveOutsideFly(pFrame, aTmpPos);
}

void SwWW8ImplReader::Read_HdFtText(WW8_CP nStart, WW8_CP nLen, SwFrameFormat const * pHdFtFormat)
{
    const SwNodeIndex* pSttIdx = pHdFtFormat->GetContent().GetContentIdx();
    if (!pSttIdx)
        return;

    SwPosition aTmpPos( *m_pPaM->GetPoint() ); // Remember old cursor position

    Read_HdFtFootnoteText(pSttIdx, nStart, nLen - 1, MAN_HDFT);

    *m_pPaM->GetPoint() = aTmpPos;
}

bool SwWW8ImplReader::isValid_HdFt_CP(WW8_CP nHeaderCP) const
{
    // Each CP of Plcfhdd MUST be less than FibRgLw97.ccpHdd
    return (nHeaderCP < m_xWwFib->m_ccpHdr && nHeaderCP >= 0);
}

bool SwWW8ImplReader::HasOwnHeaderFooter(sal_uInt8 nWhichItems, sal_uInt8 grpfIhdt,
    int nSect)
{
    if (m_xHdFt)
    {
        WW8_CP nStart, nLen;
        sal_uInt8 nNumber = 5;

        for( sal_uInt8 nI = 0x20; nI; nI >>= 1, nNumber-- )
        {
            if (nI & nWhichItems)
            {
                bool bOk = true;
                if( m_bVer67 )
                    bOk = ( m_xHdFt->GetTextPos(grpfIhdt, nI, nStart, nLen ) && nStart >= 0 && nLen >= 2 );
                else
                {
                    m_xHdFt->GetTextPosExact( static_cast< short >(nNumber + (nSect+1)*6), nStart, nLen);
                    bOk = ( 2 <= nLen ) && isValid_HdFt_CP(nStart);
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

    if( m_xHdFt )
    {
        WW8_CP nStart, nLen;
        sal_uInt8 nNumber = 5;

        // This loops through the 6 flags WW8_{FOOTER,HEADER}_{ODD,EVEN,FIRST}
        // corresponding to bit fields in grpfIhdt indicating which
        // header/footer(s) are present in this section
        for( sal_uInt8 nI = 0x20; nI; nI >>= 1, nNumber-- )
        {
            if (nI & grpfIhdt)
            {
                bool bOk = true;
                if( m_bVer67 )
                    bOk = ( m_xHdFt->GetTextPos(grpfIhdt, nI, nStart, nLen ) && nLen >= 2 );
                else
                {
                    m_xHdFt->GetTextPosExact( static_cast< short >(nNumber + (nSect+1)*6), nStart, nLen);
                    bOk = ( 2 <= nLen ) && isValid_HdFt_CP(nStart);
                }

                bool bUseLeft
                    = (nI & ( WW8_HEADER_EVEN | WW8_FOOTER_EVEN )) != 0;
                bool bUseFirst
                    = (nI & ( WW8_HEADER_FIRST | WW8_FOOTER_FIRST )) != 0;

                // If we are loading a first-page header/footer which is not
                // actually enabled in this section (it still needs to be
                // loaded as it may be inherited by a later section)
                bool bDisabledFirst = bUseFirst && !rSection.HasTitlePage();

                bool bFooter
                    = (nI & ( WW8_FOOTER_EVEN | WW8_FOOTER_ODD | WW8_FOOTER_FIRST )) != 0;

                SwFrameFormat& rFormat = bUseLeft ? pPD->GetLeft()
                    : bUseFirst ? pPD->GetFirstMaster()
                    : pPD->GetMaster();

                SwFrameFormat* pHdFtFormat;
                // If we have empty first page header and footer.
                bool bNoFirst = !(grpfIhdt & WW8_HEADER_FIRST) && !(grpfIhdt & WW8_FOOTER_FIRST);
                if (bFooter)
                {
                    m_bIsFooter = true;
                    //#i17196# Cannot have left without right
                    if (!bDisabledFirst
                            && !pPD->GetMaster().GetFooter().GetFooterFormat())
                        pPD->GetMaster().SetFormatAttr(SwFormatFooter(true));
                    if (bUseLeft)
                        pPD->GetLeft().SetFormatAttr(SwFormatFooter(true));
                    if (bUseFirst || (rSection.maSep.fTitlePage && bNoFirst))
                        pPD->GetFirstMaster().SetFormatAttr(SwFormatFooter(true));
                    pHdFtFormat = const_cast<SwFrameFormat*>(rFormat.GetFooter().GetFooterFormat());
                }
                else
                {
                    m_bIsHeader = true;
                    //#i17196# Cannot have left without right
                    if (!bDisabledFirst
                            && !pPD->GetMaster().GetHeader().GetHeaderFormat())
                        pPD->GetMaster().SetFormatAttr(SwFormatHeader(true));
                    if (bUseLeft)
                        pPD->GetLeft().SetFormatAttr(SwFormatHeader(true));
                    if (bUseFirst || (rSection.maSep.fTitlePage && bNoFirst))
                        pPD->GetFirstMaster().SetFormatAttr(SwFormatHeader(true));
                    pHdFtFormat = const_cast<SwFrameFormat*>(rFormat.GetHeader().GetHeaderFormat());
                }

                if (bOk)
                {
                    bool bHackRequired = false;
                    if (m_bIsHeader && rSection.IsFixedHeightHeader())
                        bHackRequired = true;
                    else if (m_bIsFooter && rSection.IsFixedHeightFooter())
                        bHackRequired = true;

                    if (bHackRequired)
                    {
                        Read_HdFtTextAsHackedFrame(nStart, nLen, *pHdFtFormat,
                            static_cast< sal_uInt16 >(rSection.GetTextAreaWidth()) );
                    }
                    else
                        Read_HdFtText(nStart, nLen, pHdFtFormat);
                }
                else if (!bOk && pPrev)
                    CopyPageDescHdFt(pPrev, pPD, nI);

                m_bIsHeader = m_bIsFooter = false;
            }
        }
    }
}

bool wwSectionManager::SectionIsProtected(const wwSection &rSection) const
{
    return ( mrReader.m_xWDop->fProtEnabled && !rSection.IsNotProtected() );
}

void wwSectionManager::SetHdFt(wwSection const &rSection, int nSect,
    const wwSection *pPrevious)
{
    // Header/Footer not present
    if (!rSection.maSep.grpfIhdt)
        return;

    OSL_ENSURE(rSection.mpPage, "makes no sense to call with a main page");
    if (rSection.mpPage)
    {
        mrReader.Read_HdFt(nSect, pPrevious ? pPrevious->mpPage : nullptr,
                rSection);
    }

    // Header/Footer - Update Index
    // So that the index is still valid later on
    if (mrReader.m_xHdFt)
        mrReader.m_xHdFt->UpdateIndex(rSection.maSep.grpfIhdt);

}

void SwWW8ImplReader::AppendTextNode(SwPosition& rPos)
{
    SwTextNode* pText = m_pPaM->GetNode().GetTextNode();

    const SwNumRule* pRule = nullptr;

    if (pText != nullptr)
        pRule = sw::util::GetNumRuleFromTextNode(*pText);

    if (
         pRule && !m_xWDop->fDontUseHTMLAutoSpacing &&
         (m_bParaAutoBefore || m_bParaAutoAfter)
       )
    {
        // If after spacing is set to auto, set the after space to 0
        if (m_bParaAutoAfter)
            SetLowerSpacing(*m_pPaM, 0);

        // If the previous textnode had numbering and
        // and before spacing is set to auto, set before space to 0
        if(m_pPrevNumRule && m_bParaAutoBefore)
            SetUpperSpacing(*m_pPaM, 0);

        // If the previous numbering rule was different we need
        // to insert a space after the previous paragraph
        if((pRule != m_pPrevNumRule) && m_pPreviousNumPaM)
            SetLowerSpacing(*m_pPreviousNumPaM, GetParagraphAutoSpace(m_xWDop->fDontUseHTMLAutoSpacing));

        // cache current paragraph
        if(m_pPreviousNumPaM)
        {
            delete m_pPreviousNumPaM;
            m_pPreviousNumPaM = nullptr;
        }

        m_pPreviousNumPaM = new SwPaM(*m_pPaM, m_pPaM);
        m_pPrevNumRule = pRule;
    }
    else if(!pRule && m_pPreviousNumPaM)
    {
        // If the previous paragraph has numbering but the current one does not
        // we need to add a space after the previous paragraph
        SetLowerSpacing(*m_pPreviousNumPaM, GetParagraphAutoSpace(m_xWDop->fDontUseHTMLAutoSpacing));
        delete m_pPreviousNumPaM;
        m_pPreviousNumPaM = nullptr;
        m_pPrevNumRule = nullptr;
    }
    else
    {
        // clear paragraph cache
        if(m_pPreviousNumPaM)
        {
            delete m_pPreviousNumPaM;
            m_pPreviousNumPaM = nullptr;
        }
        m_pPrevNumRule = pRule;
    }

    // If this is the first paragraph in the document and
    // Auto-spacing before paragraph is set,
    // set the upper spacing value to 0
    if(m_bParaAutoBefore && m_bFirstPara && !m_xWDop->fDontUseHTMLAutoSpacing)
        SetUpperSpacing(*m_pPaM, 0);

    m_bFirstPara = false;

    m_rDoc.getIDocumentContentOperations().AppendTextNode(rPos);

    // We can flush all anchored graphics at the end of a paragraph.
    m_xAnchorStck->Flush();
}

bool SwWW8ImplReader::SetSpacing(SwPaM &rMyPam, int nSpace, bool bIsUpper )
{
        bool bRet = false;
        const SwPosition* pSpacingPos = rMyPam.GetPoint();

        const SvxULSpaceItem* pULSpaceItem = m_xCtrlStck->GetFormatAttr(*pSpacingPos, RES_UL_SPACE);

        if(pULSpaceItem != nullptr)
        {
            SvxULSpaceItem aUL(*pULSpaceItem);

            if(bIsUpper)
                aUL.SetUpper( static_cast< sal_uInt16 >(nSpace) );
            else
                aUL.SetLower( static_cast< sal_uInt16 >(nSpace) );

            const sal_Int32 nEnd = pSpacingPos->nContent.GetIndex();
            rMyPam.GetPoint()->nContent.Assign(rMyPam.GetContentNode(), 0);
            m_xCtrlStck->NewAttr(*pSpacingPos, aUL);
            rMyPam.GetPoint()->nContent.Assign(rMyPam.GetContentNode(), nEnd);
            m_xCtrlStck->SetAttr(*pSpacingPos, RES_UL_SPACE);
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
    if (m_bVer67)
        return 25;
    return nLevel ? 0x244C : 0x2417;
}

void SwWW8ImplReader::EndSpecial()
{
    // Frame/Table/Anl
    if (m_bAnl)
        StopAllAnl(); // -> bAnl = false

    while(m_aApos.size() > 1)
    {
        StopTable();
        m_aApos.pop_back();
        --m_nInTable;
        if (m_aApos[m_nInTable])
            StopApo();
    }

    if (m_aApos[0])
        StopApo();

    OSL_ENSURE(!m_nInTable, "unclosed table!");
}

bool SwWW8ImplReader::FloatingTableConversion(WW8PLCFx_Cp_FKP* pPap)
{
    // This is ww8 version of the code deciding if the table needs to be
    // in a floating frame.
    // For OOXML code, see SectionPropertyMap::FloatingTableConversion in
    // writerfilter/source/dmapper/PropertyMap.cxx
    // The two should do ~same, so if you make changes here, please check
    // that the other is in sync.

    // Note that this is just a list of heuristics till sw core can have a
    // table that is floating and can span over multiple pages at the same
    // time.

    bool bResult = true;

    SprmResult aRes = pPap->HasSprm(NS_sprm::sprmTDefTable);
    if (nullptr != aRes.pSprm)
    {
        bResult = false;
        WW8TabBandDesc aDesc;
        aDesc.ReadDef(false, aRes.pSprm, aRes.nRemainingData);
        int nTextAreaWidth = m_aSectionManager.GetTextAreaWidth();
        int nTableWidth = aDesc.nCenter[aDesc.nWwCols] - aDesc.nCenter[0];

        // It seems Word has a limit here, so that in case the table width is quite
        // close to the text area width, then it won't perform a wrapping, even in
        // case the content (e.g. an empty paragraph) would fit. The magic constant
        // here represents this limit.
        const int nMagicNumber = 469;

        // If the table is wider than the text area, then don't create a fly
        // for the table: no wrapping will be performed anyway, but multi-page
        // tables will be broken.
        if ((nTableWidth + nMagicNumber) < nTextAreaWidth)
            bResult = true;

        // If there are columns, do create a fly, as the flow of the columns
        // would otherwise restrict the table.
        if (!bResult && (m_aSectionManager.CurrentSectionColCount() >= 2))
            bResult = true;
    }

    if (bResult)
    {
        WW8PLCFxSave1 aSave;
        pPap->Save(aSave);
        if (SearchTableEnd(pPap))
        {
            // Table is considered to be imported into a fly frame and we
            // know where the end of the table is.
            bool bIsUnicode;
            WW8_FC nFc = m_xSBase->WW8Cp2Fc(pPap->Where(), &bIsUnicode);
            sal_uInt64 nPos = m_pStrm->Tell();
            m_pStrm->Seek(nFc);
            sal_uInt16 nUChar = 0;
            if (bIsUnicode)
                m_pStrm->ReadUInt16(nUChar);
            else
            {
                sal_uInt8 nChar = 0;
                m_pStrm->ReadUChar(nChar);
                nUChar = nChar;
            }
            m_pStrm->Seek(nPos);
            if (nUChar == 0xc)
                // The pap after the table starts with a page break, so
                // there will be no wrapping around the float-table.
                // Request no fly in this case, so the table can properly
                // be a multi-page one if necessary.
                bResult = false;
        }
        pPap->Restore(aSave);
    }

    return bResult;
}

bool SwWW8ImplReader::ProcessSpecial(bool &rbReSync, WW8_CP nStartCp)
{
    // Frame/Table/Anl
    if (m_bInHyperlink)
        return false;

    rbReSync = false;

    OSL_ENSURE(m_nInTable >= 0,"nInTable < 0!");

    // TabRowEnd
    bool bTableRowEnd = (m_xPlcxMan->HasParaSprm(m_bVer67 ? 25 : 0x2417).pSprm != nullptr);

// Unfortunately, for every paragraph we need to check first whether
// they contain a sprm 29 (0x261B), which starts an APO.
// All other sprms then refer to that APO and not to the normal text
// surrounding it.
// The same holds true for a Table (sprm 24 (0x2416)) and Anls (sprm 13).

// WW: Table in APO is possible (Both Start-Ends occur at the same time)
// WW: APO in Table not possible

// This mean that of a Table is the content of a APO, the APO start needs
// to be edited first, so that the Table remains in the APO and not the
// other way around.
// At the End, however, we need to edit the Table End first as the APO
// must end after that Table (or else we never find the APO End).

// The same holds true for Fly / Anl, Tab / Anl, Fly / Tab / Anl.

// If the Table is within an APO the TabRowEnd Area misses the
// APO settings.
// To not end the APO there, we do not call ProcessApo

// KHZ: When there is a table inside the Apo the Apo-flags are also
//      missing for the 2nd, 3rd... paragraphs of each cell.

//  1st look for in-table flag, for 2000+ there is a subtable flag to
//  be considered, the sprm 6649 gives the level of the table
    sal_uInt8 nCellLevel = 0;

    if (m_bVer67)
        nCellLevel = int(nullptr != m_xPlcxMan->HasParaSprm(24).pSprm);
    else
    {
        nCellLevel = int(nullptr != m_xPlcxMan->HasParaSprm(0x2416).pSprm);
        if (!nCellLevel)
            nCellLevel = int(nullptr != m_xPlcxMan->HasParaSprm(0x244B).pSprm);
    }
    do
    {
        WW8_TablePos *pTabPos=nullptr;
        WW8_TablePos aTabPos;
        if(nCellLevel && !m_bVer67)
        {
            WW8PLCFxSave1 aSave;
            m_xPlcxMan->GetPap()->Save( aSave );
            rbReSync = true;
            WW8PLCFx_Cp_FKP* pPap = m_xPlcxMan->GetPapPLCF();
            WW8_CP nMyStartCp=nStartCp;

            SprmResult aLevel = m_xPlcxMan->HasParaSprm(0x6649);
            if (aLevel.pSprm && aLevel.nRemainingData >= 1)
                nCellLevel = *aLevel.pSprm;

            bool bHasRowEnd = SearchRowEnd(pPap, nMyStartCp, (m_nInTable<nCellLevel?m_nInTable:nCellLevel-1));

            // Bad Table, remain unchanged in level, e.g. #i19667#
            if (!bHasRowEnd)
                nCellLevel = static_cast< sal_uInt8 >(m_nInTable);

            if (bHasRowEnd && ParseTabPos(&aTabPos,pPap))
                pTabPos = &aTabPos;

            m_xPlcxMan->GetPap()->Restore( aSave );
        }

        // Then look if we are in an Apo

        ApoTestResults aApo = TestApo(nCellLevel, bTableRowEnd, pTabPos);

        // Look to see if we are in a Table, but Table in foot/end note not allowed
        bool bStartTab = (m_nInTable < nCellLevel) && !m_bFootnoteEdn;

        bool bStopTab = m_bWasTabRowEnd && (m_nInTable > nCellLevel) && !m_bFootnoteEdn;

        m_bWasTabRowEnd = false;  // must be deactivated right here to prevent next
                                // WW8TabDesc::TableCellEnd() from making nonsense

        if (m_nInTable && !bTableRowEnd && !bStopTab && (m_nInTable == nCellLevel && aApo.HasStartStop()))
            bStopTab = bStartTab = true; // Required to stop and start table

        //  Test for Anl (Numbering) and process all events in the right order
        if( m_bAnl && !bTableRowEnd )
        {
            SprmResult aSprm13 = m_xPlcxMan->HasParaSprm(13);
            const sal_uInt8* pSprm13 = aSprm13.pSprm;
            if (pSprm13 && aSprm13.nRemainingData >= 1)
            {   // Still Anl left?
                sal_uInt8 nT = static_cast< sal_uInt8 >(GetNumType( *pSprm13 ));
                if( ( nT != WW8_Pause && nT != m_nWwNumType ) // Anl change
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
            m_aApos.pop_back();
            --m_nInTable;
        }
        if (aApo.mbStopApo)
        {
            StopApo();
            m_aApos[m_nInTable] = false;
        }

        if (aApo.mbStartApo)
        {
            m_aApos[m_nInTable] = StartApo(aApo, pTabPos);
            // We need an ReSync after StartApo
            // (actually only if the Apo extends past a FKP border)
            rbReSync = true;
        }
        if (bStartTab)
        {
            WW8PLCFxSave1 aSave;
            m_xPlcxMan->GetPap()->Save( aSave );

           // Numbering for cell borders causes a crash -> no Anls in Tables
           if (m_bAnl)
               StopAllAnl();

            if(m_nInTable < nCellLevel)
            {
                if (StartTable(nStartCp))
                    ++m_nInTable;
                else
                    break;
                m_aApos.push_back(false);
            }

            if(m_nInTable >= nCellLevel)
            {
                // We need an ReSync after StartTable
                // (actually only if the Apo extends past a FKP border)
                rbReSync = true;
                m_xPlcxMan->GetPap()->Restore( aSave );
            }
        }
    } while (!m_bFootnoteEdn && (m_nInTable < nCellLevel));
    return bTableRowEnd;
}

rtl_TextEncoding SwWW8ImplReader::GetCharSetFromLanguage()
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
    const SvxLanguageItem *pLang = static_cast<const SvxLanguageItem*>(GetFormatAttr(RES_CHRATR_LANGUAGE));
    LanguageType eLang = pLang ? pLang->GetLanguage() : LANGUAGE_SYSTEM;
    css::lang::Locale aLocale(LanguageTag::convertToLocale(eLang));
    return msfilter::util::getBestTextEncodingFromLocale(aLocale);
}

rtl_TextEncoding SwWW8ImplReader::GetCJKCharSetFromLanguage()
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
    const SvxLanguageItem *pLang = static_cast<const SvxLanguageItem*>(GetFormatAttr(RES_CHRATR_CJK_LANGUAGE));
    LanguageType eLang = pLang ? pLang->GetLanguage() : LANGUAGE_SYSTEM;
    css::lang::Locale aLocale(LanguageTag::convertToLocale(eLang));
    return msfilter::util::getBestTextEncodingFromLocale(aLocale);
}

rtl_TextEncoding SwWW8ImplReader::GetCurrentCharSet()
{
    /*
    #i2015
    If the hard charset is set use it, if not see if there is an open
    character run that has set the charset, if not then fallback to the
    current underlying paragraph style.
    */
    rtl_TextEncoding eSrcCharSet = m_eHardCharSet;
    if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
    {
        if (!m_aFontSrcCharSets.empty())
            eSrcCharSet = m_aFontSrcCharSets.top();
        if ((eSrcCharSet == RTL_TEXTENCODING_DONTKNOW) && m_nCharFormat >= 0 && static_cast<size_t>(m_nCharFormat) < m_vColl.size() )
            eSrcCharSet = m_vColl[m_nCharFormat].GetCharSet();
        if ((eSrcCharSet == RTL_TEXTENCODING_DONTKNOW) && StyleExists(m_nCurrentColl) && m_nCurrentColl < m_vColl.size())
            eSrcCharSet = m_vColl[m_nCurrentColl].GetCharSet();
        if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
            eSrcCharSet = GetCharSetFromLanguage();
    }
    return eSrcCharSet;
}

//Takashi Ono for CJK
rtl_TextEncoding SwWW8ImplReader::GetCurrentCJKCharSet()
{
    /*
    #i2015
    If the hard charset is set use it, if not see if there is an open
    character run that has set the charset, if not then fallback to the
    current underlying paragraph style.
    */
    rtl_TextEncoding eSrcCharSet = m_eHardCharSet;
    if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
    {
        if (!m_aFontSrcCJKCharSets.empty())
            eSrcCharSet = m_aFontSrcCJKCharSets.top();
        if ((eSrcCharSet == RTL_TEXTENCODING_DONTKNOW) && m_nCharFormat >= 0 && static_cast<size_t>(m_nCharFormat) < m_vColl.size() )
            eSrcCharSet = m_vColl[m_nCharFormat].GetCJKCharSet();
        if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW && StyleExists(m_nCurrentColl) && m_nCurrentColl < m_vColl.size())
            eSrcCharSet = m_vColl[m_nCurrentColl].GetCJKCharSet();
        if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
            eSrcCharSet = GetCJKCharSetFromLanguage();
    }
    return eSrcCharSet;
}

void SwWW8ImplReader::PostProcessAttrs()
{
    if (m_pPostProcessAttrsInfo != nullptr)
    {
        SfxItemIter aIter(m_pPostProcessAttrsInfo->mItemSet);

        const SfxPoolItem * pItem = aIter.GetCurItem();
        if (pItem != nullptr)
        {
            do
            {
                m_xCtrlStck->NewAttr(*m_pPostProcessAttrsInfo->mPaM.GetPoint(),
                                   *pItem);
                m_xCtrlStck->SetAttr(*m_pPostProcessAttrsInfo->mPaM.GetMark(),
                                   pItem->Which());
            }
            while (!aIter.IsAtEnd() && nullptr != (pItem = aIter.NextItem()));
        }

        m_pPostProcessAttrsInfo.reset();
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
static std::size_t Custom8BitToUnicode(rtl_TextToUnicodeConverter hConverter,
    sal_Char const *pIn, std::size_t nInLen, sal_Unicode *pOut, std::size_t nOutLen)
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

    std::size_t nDestChars=0;
    std::size_t nConverted=0;

    do
    {
        sal_uInt32 nInfo = 0;
        sal_Size nThisConverted=0;

        nDestChars += rtl_convertTextToUnicode(hConverter, nullptr,
            pIn+nConverted, nInLen-nConverted,
            pOut+nDestChars, nOutLen-nDestChars,
            nFlags, &nInfo, &nThisConverted);

        OSL_ENSURE(nInfo == 0, "A character conversion failed!");

        nConverted += nThisConverted;

        if (
            nInfo & RTL_TEXTTOUNICODE_INFO_UNDEFINED ||
            nInfo & RTL_TEXTTOUNICODE_INFO_MBUNDEFINED
           )
        {
            sal_Size nOtherConverted;
            rtl_TextToUnicodeConverter hCP1252Converter =
                rtl_createTextToUnicodeConverter(RTL_TEXTENCODING_MS_1252);
            nDestChars += rtl_convertTextToUnicode(hCP1252Converter, nullptr,
                pIn+nConverted, 1,
                pOut+nDestChars, nOutLen-nDestChars,
                nFlags2, &nInfo, &nOtherConverted);
            rtl_destroyTextToUnicodeConverter(hCP1252Converter);
            nConverted+=1;
        }
    } while (nConverted < nInLen);

    return nDestChars;
}

bool SwWW8ImplReader::LangUsesHindiNumbers(LanguageType nLang)
{
    bool bResult = false;

    switch (static_cast<sal_uInt16>(nLang))
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

namespace
{
    OUString makeOUString(rtl_uString *pStr, sal_Int32 nAllocLen)
    {
        //if read len was in or around that of allocated len, just reuse pStr
        if (nAllocLen < pStr->length + 256)
            return OUString(pStr, SAL_NO_ACQUIRE);
        //otherwise copy the shorter used section to release extra mem
        OUString sRet(pStr->buffer, pStr->length);
        rtl_uString_release(pStr);
        return sRet;
    }
}

/**
 * Return value: true for non special chars
 */
bool SwWW8ImplReader::ReadPlainChars(WW8_CP& rPos, sal_Int32 nEnd, sal_Int32 nCpOfs)
{
    sal_Int32 nRequestedStrLen = nEnd - rPos;

    OSL_ENSURE(nRequestedStrLen, "String is 0");
    if (nRequestedStrLen <= 0)
        return true;

    sal_Int32 nRequestedPos = m_xSBase->WW8Cp2Fc(nCpOfs+rPos, &m_bIsUnicode);
    bool bValidPos = checkSeek(*m_pStrm, nRequestedPos);
    OSL_ENSURE(bValidPos, "Document claimed to have more text than available");
    if (!bValidPos)
    {
        // Swallow missing range, e.g. #i95550#
        rPos+=nRequestedStrLen;
        return true;
    }

    std::size_t nAvailableStrLen = m_pStrm->remainingSize() / (m_bIsUnicode ? 2 : 1);
    OSL_ENSURE(nAvailableStrLen, "Document claimed to have more text than available");
    if (!nAvailableStrLen)
    {
        // Swallow missing range, e.g. #i95550#
        rPos+=nRequestedStrLen;
        return true;
    }

    sal_Int32 nValidStrLen = std::min<std::size_t>(nRequestedStrLen, nAvailableStrLen);

    // Reset Unicode flag and correct FilePos if needed.
    // Note: Seek is not expensive, as we're checking inline whether or not
    // the correct FilePos has already been reached.
    const sal_Int32 nStrLen = std::min(nValidStrLen, SAL_MAX_INT32-1);

    rtl_TextEncoding eSrcCharSet = m_bVer67 ? GetCurrentCharSet() :
        RTL_TEXTENCODING_MS_1252;
    if (m_bVer67 && eSrcCharSet == RTL_TEXTENCODING_MS_932)
    {
        /*
         fdo#82904

         Older documents exported as word 95 that use unicode aware fonts will
         have the charset of those fonts set to RTL_TEXTENCODING_MS_932 on
         export as the conversion from RTL_TEXTENCODING_UNICODE. This is a serious
         pain.

         We will try and use a fallback encoding if the conversion from
         RTL_TEXTENCODING_MS_932 fails, but you can get unlucky and get a document
         which isn't really in RTL_TEXTENCODING_MS_932 but parts of it form
         valid RTL_TEXTENCODING_MS_932 by chance :-(

         We're not the only ones that struggle with this: Here's the help from
         MSOffice 2003 on the topic:

         <<
          Earlier versions of Microsoft Word were sometimes used in conjunction with
          third-party language-processing add-in programs designed to support Chinese or
          Korean on English versions of Microsoft Windows. Use of these add-ins sometimes
          results in incorrect text display in more recent versions of Word.

          However, you can set options to convert these documents so that text is
          displayed correctly. On the Tools menu, click Options, and then click the
          General tab. In the English Word 6.0/95 documents list, select Contain Asian
          text (to have Word interpret the text as Asian code page data, regardless of
          its font) or Automatically detect Asian text (to have Word attempt to determine
          which parts of the text are meant to be Asian).
        >>

        What we can try here is to ignore a RTL_TEXTENCODING_MS_932 codepage if
        the language is not Japanese
        */

        const SfxPoolItem * pItem = GetFormatAttr(RES_CHRATR_CJK_LANGUAGE);
        if (pItem != nullptr && LANGUAGE_JAPANESE != static_cast<const SvxLanguageItem *>(pItem)->GetLanguage())
        {
            SAL_WARN("sw.ww8", "discarding word95 RTL_TEXTENCODING_MS_932 encoding");
            eSrcCharSet = GetCharSetFromLanguage();
        }
    }
    const rtl_TextEncoding eSrcCJKCharSet = m_bVer67 ? GetCurrentCJKCharSet() :
        RTL_TEXTENCODING_MS_1252;

    // allocate unicode string data
    rtl_uString *pStr = rtl_uString_alloc(nStrLen);
    sal_Unicode* pBuffer = pStr->buffer;
    sal_Unicode* pWork = pBuffer;

    std::unique_ptr<sal_Char[]> p8Bits;

    rtl_TextToUnicodeConverter hConverter = nullptr;
    if (!m_bIsUnicode || m_bVer67)
        hConverter = rtl_createTextToUnicodeConverter(eSrcCharSet);

    if (!m_bIsUnicode)
        p8Bits.reset( new sal_Char[nStrLen] );

    // read the stream data
    sal_uInt8   nBCode = 0;
    sal_uInt16 nUCode;

    LanguageType nCTLLang = LANGUAGE_SYSTEM;
    const SfxPoolItem * pItem = GetFormatAttr(RES_CHRATR_CTL_LANGUAGE);
    if (pItem != nullptr)
        nCTLLang = static_cast<const SvxLanguageItem *>(pItem)->GetLanguage();

    sal_Int32 nL2;
    for (nL2 = 0; nL2 < nStrLen; ++nL2)
    {
        if (m_bIsUnicode)
            m_pStrm->ReadUInt16( nUCode ); // unicode  --> read 2 bytes
        else
        {
            m_pStrm->ReadUChar( nBCode ); // old code --> read 1 byte
            nUCode = nBCode;
        }

        if (m_pStrm->GetError())
        {
            rPos = WW8_CP_MAX-10; // -> eof or other error
            std::free(pStr);
            return true;
        }

        if ((32 > nUCode) || (0xa0 == nUCode))
        {
            m_pStrm->SeekRel( m_bIsUnicode ? -2 : -1 );
            break; // Special character < 32, == 0xa0 found
        }

        if (m_bIsUnicode)
        {
            if (!m_bVer67)
                *pWork++ = nUCode;
            else
            {
                if (nUCode >= 0x3000) //0x8000 ?
                {
                    sal_Char aTest[2];
                    aTest[0] = static_cast< sal_Char >((nUCode & 0xFF00) >> 8);
                    aTest[1] = static_cast< sal_Char >(nUCode & 0x00FF);
                    OUString aTemp(aTest, 2, eSrcCJKCharSet);
                    OSL_ENSURE(aTemp.getLength() == 1, "so much for that theory");
                    *pWork++ = aTemp[0];
                }
                else
                {
                    sal_Char cTest = static_cast< sal_Char >(nUCode & 0x00FF);
                    pWork += Custom8BitToUnicode(hConverter, &cTest, 1, pWork, 1);
                }
            }
        }
        else
            p8Bits[nL2] = nBCode;
    }

    if (nL2)
    {
        const sal_Int32 nEndUsed = !m_bIsUnicode
            ? Custom8BitToUnicode(hConverter, p8Bits.get(), nL2, pBuffer, nStrLen)
            : pWork - pBuffer;

        if (m_bRegardHindiDigits && m_bBidi && LangUsesHindiNumbers(nCTLLang))
        {
            for (sal_Int32 nI = 0; nI < nEndUsed; ++nI, ++pBuffer)
                *pBuffer = TranslateToHindiNumbers(*pBuffer);
        }

        pStr->buffer[nEndUsed] = 0;
        pStr->length = nEndUsed;

        emulateMSWordAddTextToParagraph(makeOUString(pStr, nStrLen));
        pStr = nullptr;
        rPos += nL2;
        if (!m_aApos.back()) // a para end in apo doesn't count
            m_bWasParaEnd = false; // No CR
    }

    if (hConverter)
        rtl_destroyTextToUnicodeConverter(hConverter);
    if (pStr)
        rtl_uString_release(pStr);
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

// That's four categories, we however have three categories. Given that problem
// here we would ideally find out "what would word do" to see what font/language
// word would assign to characters based on the unicode range they fall into and
// hack the word one onto the range we use. However it's unclear what word's
// categorization is. So we don't do that here yet.

// Additional to the categorization, when word encounters weak text for ambiguous
// chars it uses idcthint to indicate which way to bias. We don't have a idcthint
// feature in writer.

// So what we currently do here then is to split our text into non-weak/weak
// sections and uses word's idcthint to determine what font it would use and
// force that on for the segment. Following what we *do* know about word's
// categorization, we know that the range 0x0020 and 0x007F is sprmCRgFtc0 in
// word, something we map to LATIN, so we consider all weaks chars in that range
// to auto-bias to LATIN.

// See https://bugs.libreoffice.org/show_bug.cgi?id=34319 for an example
void SwWW8ImplReader::emulateMSWordAddTextToParagraph(const OUString& rAddString)
{
    if (rAddString.isEmpty())
        return;

    uno::Reference<i18n::XBreakIterator> xBI(g_pBreakIt->GetBreakIter());
    assert(xBI.is());

    sal_Int16 nScript = lcl_getScriptType(xBI, rAddString, 0);
    sal_Int32 nLen = rAddString.getLength();

    OUString sParagraphText;
    const SwContentNode *pCntNd = m_pPaM->GetContentNode();
    const SwTextNode* pNd = pCntNd ? pCntNd->GetTextNode() : nullptr;
    if (pNd)
        sParagraphText = pNd->GetText();
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
        const SvxFontItem *pOverriddenItems[] = {nullptr, nullptr, nullptr};
        bool aForced[] = {false, false, false};

        int nLclIdctHint = 0xFF;
        if (nScript == i18n::ScriptType::WEAK)
        {
            const SfxInt16Item *pIdctHint = static_cast<const SfxInt16Item*>(GetFormatAttr(RES_CHRATR_IDCTHINT));
            nLclIdctHint = pIdctHint->GetValue();
        }
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
                    const SvxFontItem *pSourceFont = static_cast<const SvxFontItem*>(GetFormatAttr(nForceFromFontId));
                    sal_uInt16 nDestId = aIds[nWriterScript-1];
                    const SvxFontItem *pDestFont = static_cast<const SvxFontItem*>(GetFormatAttr(nDestId));
                    bWriterWillUseSameFontAsWordAutomatically = sameFontIgnoringIrrelevantFields(*pSourceFont, *pDestFont);
                }
            }

            // Writer won't use the same font as word, so force the issue
            if (!bWriterWillUseSameFontAsWordAutomatically)
            {
                const SvxFontItem *pSourceFont = static_cast<const SvxFontItem*>(GetFormatAttr(nForceFromFontId));

                for (size_t i = 0; i < SAL_N_ELEMENTS(aIds); ++i)
                {
                    const SvxFontItem *pDestFont = static_cast<const SvxFontItem*>(GetFormatAttr(aIds[i]));
                    aForced[i] = aIds[i] != nForceFromFontId && *pSourceFont != *pDestFont;
                    if (aForced[i])
                    {
                        pOverriddenItems[i] =
                            static_cast<const SvxFontItem*>(m_xCtrlStck->GetStackAttr(*m_pPaM->GetPoint(), aIds[i]));

                        SvxFontItem aForceFont(*pSourceFont);
                        aForceFont.SetWhich(aIds[i]);
                        m_xCtrlStck->NewAttr(*m_pPaM->GetPoint(), aForceFont);
                    }
                }
            }
        }

        simpleAddTextToParagraph(sChunk);

        for (size_t i = 0; i < SAL_N_ELEMENTS(aIds); ++i)
        {
            if (aForced[i])
            {
                m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), aIds[i]);
                if (pOverriddenItems[i])
                    m_xCtrlStck->NewAttr(*m_pPaM->GetPoint(), *(pOverriddenItems[i]));
            }
        }

        nPos = nEnd;
        if (nPos < nLen)
            nScript = lcl_getScriptType(xBI, rAddString, nPos);
    }
}

void SwWW8ImplReader::simpleAddTextToParagraph(const OUString& rAddString)
{
    if (rAddString.isEmpty())
        return;

#if OSL_DEBUG_LEVEL > 1
    SAL_INFO("sw.ww8", "<addTextToParagraph>" << rAddString << "</addTextToParagraph>");
#endif

    const SwContentNode *pCntNd = m_pPaM->GetContentNode();
    const SwTextNode* pNd = pCntNd ? pCntNd->GetTextNode() : nullptr;

    OSL_ENSURE(pNd, "What the hell, where's my text node");

    if (!pNd)
        return;

    const sal_Int32 nCharsLeft = SAL_MAX_INT32 - pNd->GetText().getLength();
    if (nCharsLeft > 0)
    {
        if (rAddString.getLength() <= nCharsLeft)
        {
            m_rDoc.getIDocumentContentOperations().InsertString(*m_pPaM, rAddString);
        }
        else
        {
            m_rDoc.getIDocumentContentOperations().InsertString(*m_pPaM, rAddString.copy(0, nCharsLeft));
            AppendTextNode(*m_pPaM->GetPoint());
            m_rDoc.getIDocumentContentOperations().InsertString(*m_pPaM, rAddString.copy(nCharsLeft));
        }
    }
    else
    {
        AppendTextNode(*m_pPaM->GetPoint());
        m_rDoc.getIDocumentContentOperations().InsertString(*m_pPaM, rAddString);
    }

    m_bReadTable = false;
}

/**
 * Return value: true for para end
 */
bool SwWW8ImplReader::ReadChars(WW8_CP& rPos, WW8_CP nNextAttr, long nTextEnd,
    long nCpOfs)
{
    long nEnd = ( nNextAttr < nTextEnd ) ? nNextAttr : nTextEnd;

    if (m_bSymbol || m_bIgnoreText)
    {
        WW8_CP nRequested = nEnd - rPos;
        if (m_bSymbol) // Insert special chars
        {
            sal_uInt64 nMaxPossible = m_pStrm->remainingSize();
            if (static_cast<sal_uInt64>(nRequested) > nMaxPossible)
            {
                SAL_WARN("sw.ww8", "document claims to have more characters, " << nRequested << " than remaining, " << nMaxPossible);
                nRequested = nMaxPossible;
            }

            for (WW8_CP nCh = 0; nCh < nRequested; ++nCh)
            {
                m_rDoc.getIDocumentContentOperations().InsertString( *m_pPaM, OUString(m_cSymbol) );
            }
            m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_FONT );
            m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_CJK_FONT );
            m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_CTL_FONT );
        }
        m_pStrm->SeekRel(nRequested);
        rPos = nEnd; // Ignore until attribute end
        return false;
    }

    while (true)
    {
        if (ReadPlainChars(rPos, nEnd, nCpOfs))
            return false; // Done

        bool bStartLine = ReadChar(rPos, nCpOfs);
        rPos++;
        if (m_bPgSecBreak || bStartLine || rPos == nEnd) // CR or Done
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
    if (!m_nInTable)
    {
        bool IsTemp=true;
        SwTextNode* pTemp = m_pPaM->GetNode().GetTextNode();
        if (pTemp && pTemp->GetText().isEmpty()
                && (m_bFirstPara || m_bFirstParaOfPage))
        {
            IsTemp = false;
            AppendTextNode(*m_pPaM->GetPoint());
            pTemp->SetAttr(*GetDfltAttr(RES_PARATR_NUMRULE));
        }

        m_bPgSecBreak = true;
        m_xCtrlStck->KillUnlockedAttrs(*m_pPaM->GetPoint());
        /*
        If it's a 0x0c without a paragraph end before it, act like a
        paragraph end, but nevertheless, numbering (and perhaps other
        similar constructs) do not exist on the para.
        */
        if (!m_bWasParaEnd && IsTemp)
        {
            bParaEndAdded = true;
            if (0 >= m_pPaM->GetPoint()->nContent.GetIndex())
            {
                if (SwTextNode* pTextNode = m_pPaM->GetNode().GetTextNode())
                {
                    pTextNode->SetAttr(
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
    std::size_t nRequestedPos = m_xSBase->WW8Cp2Fc(nCpOfs+nPosCp, &m_bIsUnicode);
    if (!checkSeek(*m_pStrm, nRequestedPos))
        return false;

    sal_uInt8 nBCode(0);
    sal_uInt16 nWCharVal(0);
    if( m_bIsUnicode )
        m_pStrm->ReadUInt16( nWCharVal ); // unicode  --> read 2 bytes
    else
    {
        m_pStrm -> ReadUChar( nBCode ); // old code --> read 1 byte
        nWCharVal = nBCode;
    }

    sal_Unicode cInsert = '\x0';
    bool bParaMark = false;

    if ( 0xc != nWCharVal )
        m_bFirstParaOfPage = false;

    switch (nWCharVal)
    {
        case 0:
            {
                // Page number
                SwPageNumberField aField(
                    static_cast<SwPageNumberFieldType*>(m_rDoc.getIDocumentFieldsAccess().GetSysFieldType(
                    SwFieldIds::PageNumber )), PG_RANDOM, SVX_NUM_ARABIC);
                m_rDoc.getIDocumentContentOperations().InsertPoolItem(*m_pPaM, SwFormatField(aField));
            }
            break;
        case 0xe:
            // if there is only one column word treats a column break like a pagebreak.
            if (m_aSectionManager.CurrentSectionColCount() < 2)
                bParaMark = HandlePageBreakChar();
            else if (!m_nInTable)
            {
                // Always insert a txtnode for a column break, e.g. ##
                SwContentNode *pCntNd=m_pPaM->GetContentNode();
                if (pCntNd!=nullptr && pCntNd->Len()>0) // if par is empty not break is needed
                    AppendTextNode(*m_pPaM->GetPoint());
                m_rDoc.getIDocumentContentOperations().InsertPoolItem(*m_pPaM, SvxFormatBreakItem(SvxBreak::ColumnBefore, RES_BREAK));
            }
            break;
        case 0x7:
            {
                bNewParaEnd = true;
                WW8PLCFxDesc* pPap = m_xPlcxMan->GetPap();
                //The last paragraph of each cell is terminated by a special
                //paragraph mark called a cell mark. Following the cell mark
                //that ends the last cell of a table row, the table row is
                //terminated by a special paragraph mark called a row mark
                //
                //So the 0x7 should be right at the end of the previous
                //range to be a real cell-end.
                if (pPap->nOrigStartPos == nPosCp+1 ||
                    pPap->nOrigStartPos == WW8_CP_MAX)
                {
                    TabCellEnd();       // Table cell/row end
                }
                else
                    bParaMark = true;
            }
            break;
        case 0xf:
            if( !m_bSpec )        // "Satellite"
                cInsert = u'\x00a4';
            break;
        case 0x14:
            if( !m_bSpec )        // "Para End" char
                cInsert = u'\x00b5';
                    //TODO: should this be U+00B6 PILCROW SIGN rather than
                    // U+00B5 MICRO SIGN?
            break;
        case 0x15:
            if( !m_bSpec )        // Juristenparagraph
            {
                cp_set::iterator aItr = m_aTOXEndCps.find(static_cast<WW8_CP>(nPosCp));
                if (aItr == m_aTOXEndCps.end())
                    cInsert = u'\x00a7';
                else
                    m_aTOXEndCps.erase(aItr);
            }
            break;
        case 0x9:
            cInsert = '\x9';    // Tab
            break;
        case 0xb:
            cInsert = '\xa';    // Hard NewLine
            break;
        case 0xc:
            bParaMark = HandlePageBreakChar();
            break;
        case 0x1e:              // Non-breaking hyphen
            m_rDoc.getIDocumentContentOperations().InsertString( *m_pPaM, OUString(CHAR_HARDHYPHEN) );
            break;
        case 0x1f:              // Non-required hyphens
            m_rDoc.getIDocumentContentOperations().InsertString( *m_pPaM, OUString(CHAR_SOFTHYPHEN) );
            break;
        case 0xa0:              // Non-breaking spaces
            m_rDoc.getIDocumentContentOperations().InsertString( *m_pPaM, OUString(CHAR_HARDBLANK)  );
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
                    long nCurPos = m_pStrm->Tell();
                    sal_uInt16 nWordCode(0);

                    if( m_bIsUnicode )
                        m_pStrm->ReadUInt16( nWordCode );
                    else
                    {
                        sal_uInt8 nByteCode(0);
                        m_pStrm->ReadUChar( nByteCode );
                        nWordCode = nByteCode;
                    }
                    if( nWordCode == 0x1 )
                        bReadObj = false;
                    m_pStrm->Seek( nCurPos );
                }
                if( !bReadObj )
                {
                    SwFrameFormat *pResult = nullptr;
                    if (m_bObj)
                        pResult = ImportOle();
                    else if (m_bSpec)
                        pResult = ImportGraf();

                    // If we have a bad 0x1 insert a space instead.
                    if (!pResult)
                    {
                        cInsert = ' ';
                        OSL_ENSURE(!m_bObj && !m_bEmbeddObj && !m_nObjLocFc,
                            "WW8: Please report this document, it may have a "
                            "missing graphic");
                    }
                    else
                    {
                        // reset the flags.
                        m_bObj = m_bEmbeddObj = false;
                        m_nObjLocFc = 0;
                    }
                }
            }
            break;
        case 0x8:
            if( !m_bObj )
                Read_GrafLayer( nPosCp );
            break;
        case 0xd:
            bNewParaEnd = bParaMark = true;
            if (m_nInTable > 1)
            {
                /*
                #i9666#/#i23161#
                Yes complex, if there is an entry in the undocumented PLCF
                which I believe to be a record of cell and row boundaries
                see if the magic bit which I believe to mean cell end is
                set. I also think btw that the third byte of the 4 byte
                value is the level of the cell
                */
                WW8PLCFspecial* pTest = m_xPlcxMan->GetMagicTables();
                if (pTest && pTest->SeekPosExact(nPosCp+1+nCpOfs) &&
                    pTest->Where() == nPosCp+1+nCpOfs)
                {
                    WW8_FC nPos;
                    void *pData;
                    sal_uInt32 nData = pTest->Get(nPos, pData) ? SVBT32ToUInt32(*static_cast<SVBT32*>(pData))
                                                               : 0;
                    if (nData & 0x2) // Might be how it works
                    {
                        TabCellEnd();
                        bParaMark = false;
                    }
                }
                // tdf#106799: We expect TTP marks to be also cell marks,
                // but sometimes sprmPFInnerTtp comes without sprmPFInnerTableCell
                else if (m_bWasTabCellEnd || m_bWasTabRowEnd)
                {
                    TabCellEnd();
                    bParaMark = false;
                }
            }

            m_bWasTabCellEnd = false;

            break;              // line end
        case 0x5:               // Annotation reference
        case 0x13:
            break;
        case 0x2:               // TODO: Auto-Footnote-Number, should be replaced by SwWW8ImplReader::End_Footnote later
            if (!m_aFootnoteStack.empty())
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
    if (!m_aApos.back()) // a para end in apo doesn't count
        m_bWasParaEnd = bNewParaEnd;
    return bParaMark;
}

void SwWW8ImplReader::ProcessCurrentCollChange(WW8PLCFManResult& rRes,
    bool* pStartAttr, bool bCallProcessSpecial)
{
    sal_uInt16 nOldColl = m_nCurrentColl;
    m_nCurrentColl = m_xPlcxMan->GetColl();

    // Invalid Style-Id
    if (m_nCurrentColl >= m_vColl.size() || !m_vColl[m_nCurrentColl].m_pFormat || !m_vColl[m_nCurrentColl].m_bColl)
    {
        m_nCurrentColl = 0;
        m_bParaAutoBefore = false;
        m_bParaAutoAfter = false;
    }
    else
    {
        m_bParaAutoBefore = m_vColl[m_nCurrentColl].m_bParaAutoBefore;
        m_bParaAutoAfter = m_vColl[m_nCurrentColl].m_bParaAutoAfter;
    }

    if (nOldColl >= m_vColl.size())
        nOldColl = 0; // guess! TODO make sure this is what we want

    bool bTabRowEnd = false;
    if( pStartAttr && bCallProcessSpecial && !m_bInHyperlink )
    {
        bool bReSync;
        // Frame/Table/Autonumbering List Level
        bTabRowEnd = ProcessSpecial(bReSync, rRes.nCurrentCp + m_xPlcxMan->GetCpOfs());
        if( bReSync )
            *pStartAttr = m_xPlcxMan->Get( &rRes ); // Get Attribut-Pos again
    }

    if (!bTabRowEnd && StyleExists(m_nCurrentColl))
    {
        SetTextFormatCollAndListLevel( *m_pPaM, m_vColl[ m_nCurrentColl ]);
        ChkToggleAttr(m_vColl[ nOldColl ].m_n81Flags, m_vColl[ m_nCurrentColl ].m_n81Flags);
        ChkToggleBiDiAttr(m_vColl[nOldColl].m_n81BiDiFlags,
            m_vColl[m_nCurrentColl].m_n81BiDiFlags);
    }
}

long SwWW8ImplReader::ReadTextAttr(WW8_CP& rTextPos, long nTextEnd, bool& rbStartLine, int nDepthGuard)
{
    long nSkipChars = 0;
    WW8PLCFManResult aRes;

    OSL_ENSURE(m_pPaM->GetNode().GetTextNode(), "Missing txtnode");
    bool bStartAttr = m_xPlcxMan->Get(&aRes); // Get Attribute position again
    aRes.nCurrentCp = rTextPos;                  // Current Cp position

    bool bNewSection = (aRes.nFlags & MAN_MASK_NEW_SEP) && !m_bIgnoreText;
    if ( bNewSection ) // New Section
    {
        OSL_ENSURE(m_pPaM->GetNode().GetTextNode(), "Missing txtnode");
        // Create PageDesc and fill it
        m_aSectionManager.CreateSep(rTextPos);
        // -> 0xc was a Sectionbreak, but not a Pagebreak;
        // Create PageDesc and fill it
        m_bPgSecBreak = false;
        OSL_ENSURE(m_pPaM->GetNode().GetTextNode(), "Missing txtnode");
    }

    // New paragraph over Plcx.Fkp.papx
    if ( (aRes.nFlags & MAN_MASK_NEW_PAP)|| rbStartLine )
    {
        ProcessCurrentCollChange( aRes, &bStartAttr,
            MAN_MASK_NEW_PAP == (aRes.nFlags & MAN_MASK_NEW_PAP) &&
            !m_bIgnoreText );
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
                    ImportSprm(aRes.pMemPos, aRes.nMemLen, aRes.nSprmId);
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
                    WW8_CP nMaxLegalSkip = nTextEnd - rTextPos;
                    // Skip Field/Footnote-/End-Note here
                    rTextPos += std::min<WW8_CP>(nSkipChars, nMaxLegalSkip);
                    nSkipPos = rTextPos-1;
                }
            }
            else
                EndExtSprm( aRes.nSprmId );
        }
    }

    sal_Int32 nRequestedPos = m_xSBase->WW8Cp2Fc(m_xPlcxMan->GetCpOfs() + rTextPos, &m_bIsUnicode);
    bool bValidPos = checkSeek(*m_pStrm, nRequestedPos);
    SAL_WARN_IF(!bValidPos, "sw.ww8", "Document claimed to have text at an invalid position, skip attributes for region");

    // Find next Attr position (and Skip attributes of field contents if needed)
    if (nSkipChars && !m_bIgnoreText)
        m_xCtrlStck->MarkAllAttrsOld();
    bool bOldIgnoreText = m_bIgnoreText;
    m_bIgnoreText = true;
    sal_uInt16 nOldColl = m_nCurrentColl;
    bool bDoPlcxManPlusPLus = true;
    long nNext;
    do
    {
        if( bDoPlcxManPlusPLus )
            m_xPlcxMan->advance();
        nNext = bValidPos ? m_xPlcxMan->Where() : nTextEnd;

        if (m_pPostProcessAttrsInfo &&
            m_pPostProcessAttrsInfo->mnCpStart == nNext)
        {
            m_pPostProcessAttrsInfo->mbCopy = true;
        }

        if( (0 <= nNext) && (nSkipPos >= nNext) )
        {
            if (nDepthGuard >= 1024)
            {
                SAL_WARN("sw.ww8", "ReadTextAttr hit recursion limit");
                nNext = nTextEnd;
            }
            else
                nNext = ReadTextAttr(rTextPos, nTextEnd, rbStartLine, nDepthGuard + 1);
            bDoPlcxManPlusPLus = false;
            m_bIgnoreText = true;
        }

        if (m_pPostProcessAttrsInfo &&
            nNext > m_pPostProcessAttrsInfo->mnCpEnd)
        {
            m_pPostProcessAttrsInfo->mbCopy = false;
        }
    }
    while( nSkipPos >= nNext );
    m_bIgnoreText    = bOldIgnoreText;
    if( nSkipChars )
    {
        m_xCtrlStck->KillUnlockedAttrs( *m_pPaM->GetPoint() );
        if( nOldColl != m_xPlcxMan->GetColl() )
            ProcessCurrentCollChange(aRes, nullptr, false);
    }

    return nNext;
}

//Revised 2012.8.16 for the complex attribute presentation of 0x0D in MS
bool SwWW8ImplReader::IsParaEndInCPs(sal_Int32 nStart, sal_Int32 nEnd,bool bSdOD) const
{
    //Revised for performance consideration
    if (nStart == -1 || nEnd == -1 || nEnd < nStart )
        return false;

    return std::any_of(m_aEndParaPos.rbegin(), m_aEndParaPos.rend(),
        [=](const WW8_CP& rPos) {
            //Revised 2012.8.16,to the 0x0D,the attribute will have two situations
            //*********within***********exact******
            //*********but also sample with only left and the position of 0x0d is the edge of the right side***********
            return (bSdOD && ((nStart < rPos && nEnd > rPos) || (nStart == nEnd && rPos == nStart))) ||
                (!bSdOD && (nStart < rPos && nEnd >= rPos));
        }
    );
}

//Clear the para end position recorded in reader intermittently for the least impact on loading performance
void SwWW8ImplReader::ClearParaEndPosition()
{
    if ( !m_aEndParaPos.empty() )
        m_aEndParaPos.clear();
}

void SwWW8ImplReader::ReadAttrs(WW8_CP& rTextPos, WW8_CP& rNext, long nTextEnd, bool& rbStartLine)
{
    // Dow we have attributes?
    if( rTextPos >= rNext )
    {
        do
        {
            m_aCurrAttrCP = rTextPos;
            rNext = ReadTextAttr(rTextPos, nTextEnd, rbStartLine);
            if (rTextPos == rNext && rTextPos >= nTextEnd)
                break;
        }
        while( rTextPos >= rNext );

    }
    else if ( rbStartLine )
    {
    /* No attributes, but still a new line.
     * If a line ends with a line break and paragraph attributes or paragraph templates
     * do NOT change the line end was not added to the Plcx.Fkp.papx i.e. (nFlags & MAN_MASK_NEW_PAP)
     * is false.
     * Due to this we need to set the template here as a kind of special treatment.
     */
    if (!m_bCpxStyle && m_nCurrentColl < m_vColl.size())
            SetTextFormatCollAndListLevel(*m_pPaM, m_vColl[m_nCurrentColl]);
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
    m_xPlcxMan->TransferOpenSprms(aStack);

    while (!aStack.empty())
    {
        sal_uInt16 nSprmId = aStack.top();
        if ((0 < nSprmId) && (( eFTN > nSprmId) || (0x0800 <= nSprmId)))
            EndSprm(nSprmId);
        aStack.pop();
    }

    EndSpecial();
}

bool SwWW8ImplReader::ReadText(WW8_CP nStartCp, WW8_CP nTextLen, ManTypes nType)
{
    bool bJoined=false;

    bool bStartLine = true;
    short nCrCount = 0;
    short nDistance = 0;

    m_bWasParaEnd = false;
    m_nCurrentColl    =  0;
    m_xCurrentItemSet.reset();
    m_nCharFormat    = -1;
    m_bSpec = false;
    m_bPgSecBreak = false;

    m_xPlcxMan.reset(new WW8PLCFMan(m_xSBase.get(), nType, nStartCp));
    long nCpOfs = m_xPlcxMan->GetCpOfs(); // Offset for Header/Footer, Footnote

    WW8_CP nNext = m_xPlcxMan->Where();
    m_pPreviousNode = nullptr;
    sal_uInt8 nDropLines = 0;
    SwCharFormat* pNewSwCharFormat = nullptr;
    const SwCharFormat* pFormat = nullptr;

    bool bValidPos = checkSeek(*m_pStrm, m_xSBase->WW8Cp2Fc(nStartCp + nCpOfs, &m_bIsUnicode));
    if (!bValidPos)
        return false;

    WW8_CP l = nStartCp;
    const WW8_CP nMaxPossible = WW8_CP_MAX-nStartCp;
    if (nTextLen > nMaxPossible)
    {
        SAL_WARN_IF(nTextLen > nMaxPossible, "sw.ww8", "TextLen too long");
        nTextLen = nMaxPossible;
    }
    WW8_CP nTextEnd = nStartCp+nTextLen;
    while (l < nTextEnd)
    {
        ReadAttrs( l, nNext, nTextEnd, bStartLine );// Takes SectionBreaks into account, too
        OSL_ENSURE(m_pPaM->GetNode().GetTextNode(), "Missing txtnode");

        if (m_pPostProcessAttrsInfo != nullptr)
            PostProcessAttrs();

        if (l >= nTextEnd)
            break;

        bStartLine = ReadChars(l, nNext, nTextEnd, nCpOfs);

        // If the previous paragraph was a dropcap then do not
        // create a new txtnode and join the two paragraphs together
        if (bStartLine && !m_pPreviousNode) // Line end
        {
            bool bSplit = true;
            if (m_bCareFirstParaEndInToc)
            {
                m_bCareFirstParaEndInToc = false;
                if (m_pPaM->End() && m_pPaM->End()->nNode.GetNode().GetTextNode() &&  m_pPaM->End()->nNode.GetNode().GetTextNode()->Len() == 0)
                    bSplit = false;
            }
            if (m_bCareLastParaEndInToc)
            {
                m_bCareLastParaEndInToc = false;
                if (m_pPaM->End() && m_pPaM->End()->nNode.GetNode().GetTextNode() &&  m_pPaM->End()->nNode.GetNode().GetTextNode()->Len() == 0)
                    bSplit = false;
            }
            if (bSplit)
            {
                // We will record the CP of a paragraph end ('0x0D'), if current loading contents is from main stream;
                if (m_bOnLoadingMain)
                    m_aEndParaPos.push_back(l-1);
                AppendTextNode(*m_pPaM->GetPoint());
            }
        }

        if (m_pPreviousNode && bStartLine)
        {
            SwTextNode* pEndNd = m_pPaM->GetNode().GetTextNode();
            const sal_Int32 nDropCapLen = m_pPreviousNode->GetText().getLength();

            // Need to reset the font size and text position for the dropcap
            {
                SwPaM aTmp(*pEndNd, 0, *pEndNd, nDropCapLen+1);
                m_xCtrlStck->Delete(aTmp);
            }

            // Get the default document dropcap which we can use as our template
            const SwFormatDrop* defaultDrop =
                static_cast<const SwFormatDrop*>( GetFormatAttr(RES_PARATR_DROP));
            SwFormatDrop aDrop(*defaultDrop);

            aDrop.GetLines() = nDropLines;
            aDrop.GetDistance() = nDistance;
            aDrop.GetChars() = writer_cast<sal_uInt8>(nDropCapLen);
            // Word has no concept of a "whole word dropcap"
            aDrop.GetWholeWord() = false;

            if (pFormat)
                aDrop.SetCharFormat(const_cast<SwCharFormat*>(pFormat));
            else if(pNewSwCharFormat)
                aDrop.SetCharFormat(pNewSwCharFormat);

            SwPosition aStart(*pEndNd);
            m_xCtrlStck->NewAttr(aStart, aDrop);
            m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_PARATR_DROP);
            m_pPreviousNode = nullptr;
        }
        else if (m_bDropCap)
        {
            // If we have found a dropcap store the textnode
            m_pPreviousNode = m_pPaM->GetNode().GetTextNode();

            SprmResult aDCS;
            if (m_bVer67)
                aDCS = m_xPlcxMan->GetPapPLCF()->HasSprm(46);
            else
                aDCS = m_xPlcxMan->GetPapPLCF()->HasSprm(0x442C);

            if (aDCS.pSprm && aDCS.nRemainingData >= 1)
                nDropLines = (*aDCS.pSprm) >> 3;
            else    // There is no Drop Cap Specifier hence no dropcap
                m_pPreviousNode = nullptr;

            SprmResult aDistance = m_xPlcxMan->GetPapPLCF()->HasSprm(0x842F);
            if (aDistance.pSprm && aDistance.nRemainingData >= 2)
                nDistance = SVBT16ToShort(aDistance.pSprm);
            else
                nDistance = 0;

            const SwFormatCharFormat *pSwFormatCharFormat = nullptr;

            if (m_xCurrentItemSet)
                pSwFormatCharFormat = &(ItemGet<SwFormatCharFormat>(*m_xCurrentItemSet, RES_TXTATR_CHARFMT));

            if (pSwFormatCharFormat)
                pFormat = pSwFormatCharFormat->GetCharFormat();

            if (m_xCurrentItemSet && !pFormat)
            {
                OUString sPrefix = "WW8Dropcap" + OUString::number(m_nDropCap++);
                pNewSwCharFormat = m_rDoc.MakeCharFormat(sPrefix, m_rDoc.GetDfltCharFormat());
                m_xCurrentItemSet->ClearItem(RES_CHRATR_ESCAPEMENT);
                pNewSwCharFormat->SetFormatAttr(*m_xCurrentItemSet);
            }

            m_xCurrentItemSet.reset();
            m_bDropCap=false;
        }

        if (bStartLine || m_bWasTabRowEnd)
        {
            // Call all 64 CRs; not for Header and the like
            if ((nCrCount++ & 0x40) == 0 && nType == MAN_MAINTEXT && l <= nTextLen)
            {
                if (nTextLen < WW8_CP_MAX/100)
                    m_nProgress = static_cast<sal_uInt16>(l * 100 / nTextLen);
                else
                    m_nProgress = static_cast<sal_uInt16>(l / nTextLen * 100);
                m_xProgress->Update(m_nProgress); // Update
            }
        }

        // If we have encountered a 0x0c which indicates either section of
        // pagebreak then look it up to see if it is a section break, and
        // if it is not then insert a page break. If it is a section break
        // it will be handled as such in the ReadAttrs of the next loop
        if (m_bPgSecBreak)
        {
            // We need only to see if a section is ending at this cp,
            // the plcf will already be sitting on the correct location
            // if it is there.
            WW8PLCFxDesc aTemp;
            aTemp.nStartPos = aTemp.nEndPos = WW8_CP_MAX;
            if (m_xPlcxMan->GetSepPLCF())
                m_xPlcxMan->GetSepPLCF()->GetSprms(&aTemp);
            if ((aTemp.nStartPos != l) && (aTemp.nEndPos != l))
            {
                // #i39251# - insert text node for page break, if no one inserted.
                // #i43118# - refine condition: the anchor
                // control stack has to have entries, otherwise it's not needed
                // to insert a text node.
                if (!bStartLine && !m_xAnchorStck->empty())
                {
                    AppendTextNode(*m_pPaM->GetPoint());
                }
                m_rDoc.getIDocumentContentOperations().InsertPoolItem(*m_pPaM,
                    SvxFormatBreakItem(SvxBreak::PageBefore, RES_BREAK));
                m_bFirstParaOfPage = true;
                m_bPgSecBreak = false;
            }
        }
    }

    m_pPreviousNode = nullptr;

    if (m_pPaM->GetPoint()->nContent.GetIndex())
        AppendTextNode(*m_pPaM->GetPoint());

    if (!m_bInHyperlink)
        bJoined = JoinNode(*m_pPaM);

    CloseAttrEnds();

    m_xPlcxMan.reset();
    return bJoined;
}

SwWW8ImplReader::SwWW8ImplReader(sal_uInt8 nVersionPara, SotStorage* pStorage,
    SvStream* pSt, SwDoc& rD, const OUString& rBaseURL, bool bNewDoc, bool bSkipImages, SwPosition const &rPos)
    : m_pDocShell(rD.GetDocShell())
    , m_pStg(pStorage)
    , m_pStrm(pSt)
    , m_pTableStream(nullptr)
    , m_pDataStream(nullptr)
    , m_rDoc(rD)
    , m_pPaM(nullptr)
    , m_aSectionManager(*this)
    , m_aExtraneousParas(rD)
    , m_aInsertedTables(rD)
    , m_aSectionNameGenerator(rD, "WW")
    , m_aGrfNameGenerator(bNewDoc, OUString('G'))
    , m_aParaStyleMapper(rD)
    , m_aCharStyleMapper(rD)
    , m_pFlyFormatOfJustInsertedGraphic(nullptr)
    , m_pFormatOfJustInsertedApo(nullptr)
    , m_pPreviousNumPaM(nullptr)
    , m_pPrevNumRule(nullptr)
    , m_aTextNodesHavingFirstLineOfstSet()
    , m_aTextNodesHavingLeftIndentSet()
    , m_pCurrentColl(nullptr)
    , m_pDfltTextFormatColl(nullptr)
    , m_pStandardFormatColl(nullptr)
    , m_pDrawModel(nullptr)
    , m_pDrawPg(nullptr)
    , m_pNumFieldType(nullptr)
    , m_sBaseURL(rBaseURL)
    , m_nIniFlags(0)
    , m_nIniFlags1(0)
    , m_nFieldFlags(0)
    , m_bRegardHindiDigits( false )
    , m_bDrawCpOValid( false )
    , m_nDrawCpO(0)
    , m_nPicLocFc(0)
    , m_nObjLocFc(0)
    , m_nIniFlyDx(0)
    , m_nIniFlyDy(0)
    , m_eTextCharSet(RTL_TEXTENCODING_ASCII_US)
    , m_eStructCharSet(RTL_TEXTENCODING_ASCII_US)
    , m_eHardCharSet(RTL_TEXTENCODING_DONTKNOW)
    , m_nProgress(0)
    , m_nCurrentColl(0)
    , m_nFieldNum(0)
    , m_nLFOPosition(USHRT_MAX)
    , m_nCharFormat(0)
    , m_nDrawXOfs(0)
    , m_nDrawYOfs(0)
    , m_nDrawXOfs2(0)
    , m_nDrawYOfs2(0)
    , m_cSymbol(0)
    , m_nWantedVersion(nVersionPara)
    , m_nSwNumLevel(0xff)
    , m_nWwNumType(0xff)
    , m_nListLevel(WW8ListManager::nMaxLevel)
    , m_bNewDoc(bNewDoc)
    , m_bSkipImages(bSkipImages)
    , m_bReadNoTable(false)
    , m_bPgSecBreak(false)
    , m_bSpec(false)
    , m_bObj(false)
    , m_bTxbxFlySection(false)
    , m_bHasBorder(false)
    , m_bSymbol(false)
    , m_bIgnoreText(false)
    , m_nInTable(0)
    , m_bWasTabRowEnd(false)
    , m_bWasTabCellEnd(false)
    , m_bAnl(false)
    , m_bHdFtFootnoteEdn(false)
    , m_bFootnoteEdn(false)
    , m_bIsHeader(false)
    , m_bIsFooter(false)
    , m_bIsUnicode(false)
    , m_bCpxStyle(false)
    , m_bStyNormal(false)
    , m_bWWBugNormal(false)
    , m_bNoAttrImport(false)
    , m_bInHyperlink(false)
    , m_bWasParaEnd(false)
    , m_bVer67(false)
    , m_bVer6(false)
    , m_bVer7(false)
    , m_bVer8(false)
    , m_bEmbeddObj(false)
    , m_bCurrentAND_fNumberAcross(false)
    , m_bNoLnNumYet(true)
    , m_bFirstPara(true)
    , m_bFirstParaOfPage(false)
    , m_bParaAutoBefore(false)
    , m_bParaAutoAfter(false)
    , m_bDropCap(false)
    , m_nDropCap(0)
    , m_bBidi(false)
    , m_bReadTable(false)
    , m_bLoadingTOXCache(false)
    , m_nEmbeddedTOXLevel(0)
    , m_bLoadingTOXHyperlink(false)
    , m_pPreviousNode(nullptr)
    , m_bCareFirstParaEndInToc(false)
    , m_bCareLastParaEndInToc(false)
    , m_aTOXEndCps()
    , m_aCurrAttrCP(-1)
    , m_bOnLoadingMain(false)
{
    m_pStrm->SetEndian( SvStreamEndian::LITTLE );
    m_aApos.push_back(false);

    mpCursor = m_rDoc.CreateUnoCursor(rPos);
}

SwWW8ImplReader::~SwWW8ImplReader()
{
}

void SwWW8ImplReader::DeleteStack(std::unique_ptr<SwFltControlStack> pStck)
{
    if( pStck )
    {
        pStck->SetAttr( *m_pPaM->GetPoint(), 0, false);
        pStck->SetAttr( *m_pPaM->GetPoint(), 0, false);
    }
    else
    {
        OSL_ENSURE( false, "WW stack already deleted" );
    }
}

void wwSectionManager::SetSegmentToPageDesc(const wwSection &rSection,
    bool bIgnoreCols)
{
    SwPageDesc &rPage = *rSection.mpPage;

    SetNumberingType(rSection, rPage);

    SwFrameFormat &rFormat = rPage.GetMaster();

    if(mrReader.m_xWDop->fUseBackGroundInAllmodes) // #i56806# Make sure mrReader is initialized
        mrReader.GrafikCtor();

    if (mrReader.m_xWDop->fUseBackGroundInAllmodes && mrReader.m_xMSDffManager)
    {
        tools::Rectangle aRect(0, 0, 100, 100); // A dummy, we don't care about the size
        SvxMSDffImportData aData(aRect);
        SdrObject* pObject = nullptr;
        if (mrReader.m_xMSDffManager->GetShape(0x401, pObject, aData) && !aData.empty())
        {
            // Only handle shape if it is a background shape
            if (aData.begin()->get()->nFlags & ShapeFlag::Background)
            {
                SfxItemSet aSet(rFormat.GetAttrSet());
                mrReader.MatchSdrItemsIntoFlySet(pObject, aSet, mso_lineSimple,
                                                 mso_lineSolid, mso_sptRectangle, aRect);
                rFormat.SetFormatAttr(aSet.Get(RES_BACKGROUND));
            }
        }
        SdrObject::Free(pObject);
    }
    wwULSpaceData aULData;
    GetPageULData(rSection, aULData);
    SetPageULSpaceItems(rFormat, aULData, rSection);

    rPage.SetVerticalAdjustment( rSection.mnVerticalAdjustment );

    SetPage(rPage, rFormat, rSection, bIgnoreCols);

    if (!(rSection.maSep.pgbApplyTo & 1))
        SwWW8ImplReader::SetPageBorder(rFormat, rSection);
    if (!(rSection.maSep.pgbApplyTo & 2))
        SwWW8ImplReader::SetPageBorder(rPage.GetFirstMaster(), rSection);

    mrReader.SetDocumentGrid(rFormat, rSection);
}

void wwSectionManager::SetUseOn(wwSection &rSection)
{
    bool bMirror = mrReader.m_xWDop->fMirrorMargins ||
        mrReader.m_xWDop->doptypography.f2on1;

    UseOnPage eUseBase = bMirror ? UseOnPage::Mirror : UseOnPage::All;
    UseOnPage eUse = eUseBase;
    if (!mrReader.m_xWDop->fFacingPages)
        eUse |= UseOnPage::HeaderShare | UseOnPage::FooterShare;
    if (!rSection.HasTitlePage())
        eUse |= UseOnPage::FirstShare;

    OSL_ENSURE(rSection.mpPage, "Makes no sense to call me with no pages to set");
    if (rSection.mpPage)
        rSection.mpPage->WriteUseOn(eUse);
}

/**
 * Set the page descriptor on this node, handle the different cases for a text
 * node or a table
 */
static void GiveNodePageDesc(SwNodeIndex const &rIdx, const SwFormatPageDesc &rPgDesc,
    SwDoc &rDoc)
{
    /*
    If it's a table here, apply the pagebreak to the table
    properties, otherwise we add it to the para at this
    position
    */
    if (rIdx.GetNode().IsTableNode())
    {
        SwTable& rTable =
            rIdx.GetNode().GetTableNode()->GetTable();
        SwFrameFormat* pApply = rTable.GetFrameFormat();
        OSL_ENSURE(pApply, "impossible");
        if (pApply)
            pApply->SetFormatAttr(rPgDesc);
    }
    else
    {
        SwPosition aPamStart(rIdx);
        aPamStart.nContent.Assign(
            rIdx.GetNode().GetContentNode(), 0);
        SwPaM aPage(aPamStart);

        rDoc.getIDocumentContentOperations().InsertPoolItem(aPage, rPgDesc);
    }
}

/**
 * Map a word section to a writer page descriptor
 */
SwFormatPageDesc wwSectionManager::SetSwFormatPageDesc(mySegIter const &rIter,
    mySegIter const &rStart, bool bIgnoreCols)
{
    if (mrReader.m_bNewDoc && rIter == rStart)
    {
        rIter->mpPage =
            mrReader.m_rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD);
    }
    else
    {
        rIter->mpPage = mrReader.m_rDoc.MakePageDesc(
            SwViewShell::GetShellRes()->GetPageDescName(mnDesc, ShellResource::NORMAL_PAGE),
            nullptr, false);
    }
    OSL_ENSURE(rIter->mpPage, "no page!");
    if (!rIter->mpPage)
        return SwFormatPageDesc();

    // Set page before hd/ft
    const wwSection *pPrevious = nullptr;
    if (rIter != rStart)
        pPrevious = &(*(rIter-1));
    SetHdFt(*rIter, std::distance(rStart, rIter), pPrevious);
    SetUseOn(*rIter);

    // Set hd/ft after set page
    SetSegmentToPageDesc(*rIter, bIgnoreCols);

    SwFormatPageDesc aRet(rIter->mpPage);

    rIter->mpPage->SetFollow(rIter->mpPage);

    if (rIter->PageRestartNo())
        aRet.SetNumOffset(rIter->PageStartAt());

    ++mnDesc;
    return aRet;
}

void wwSectionManager::InsertSegments()
{
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
            mrReader.m_rDoc.getIDocumentContentOperations().InsertPoolItem( start, SvxFormatBreakItem(SvxBreak::ColumnBefore, RES_BREAK));
            continue;
        }

        mySegIter aNext = aIter+1;
        mySegIter aPrev = (aIter == aStart) ? aIter : aIter-1;

        // If two following sections are different in following properties, Word will interpret a continuous
        // section break between them as if it was a section break next page.
        bool bThisAndPreviousAreCompatible = ((aIter->GetPageWidth() == aPrev->GetPageWidth()) &&
            (aIter->GetPageHeight() == aPrev->GetPageHeight()) && (aIter->IsLandScape() == aPrev->IsLandScape()));

        bool bInsertSection = (aIter != aStart) && aIter->IsContinuous() &&  bThisAndPreviousAreCompatible;
        bool bInsertPageDesc = !bInsertSection;
        // HACK Force new pagedesc if margins change, otherwise e.g. floating tables may be anchored improperly.
        if( aIter->maSep.dyaTop != aPrev->maSep.dyaTop || aIter->maSep.dyaBottom != aPrev->maSep.dyaBottom
            || aIter->maSep.dxaLeft != aPrev->maSep.dxaLeft || aIter->maSep.dxaRight != aPrev->maSep.dxaRight )
            bInsertPageDesc = true;
        bool bProtected = SectionIsProtected(*aIter); // do we really  need this ?? I guess I have a different logic in editshell which disables this...

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
            bool bThisAndNextAreCompatible = (aNext == aEnd) ||
                ((aIter->GetPageWidth() == aNext->GetPageWidth()) &&
                 (aIter->GetPageHeight() == aNext->GetPageHeight()) &&
                 (aIter->IsLandScape() == aNext->IsLandScape()));

            if ((aNext != aEnd && aNext->IsContinuous() && bThisAndNextAreCompatible) || bProtected)
            {
                bIgnoreCols = true;
                if ((aIter->NoCols() > 1) || bProtected)
                    bInsertSection = true;
            }

            SwFormatPageDesc aDesc(SetSwFormatPageDesc(aIter, aStart, bIgnoreCols));
            if (!aDesc.GetPageDesc())
                continue;

            // special case handling for odd/even section break
            // a) as before create a new page style for the section break
            // b) set Layout of generated page style to right/left ( according
            //    to section break odd/even )
            // c) create a new style to follow the break page style
            if ( aIter->maSep.bkc == 3 || aIter->maSep.bkc == 4 )
            {
                // SetSwFormatPageDesc calls some methods that could
                // modify aIter (e.g. wwSection ).
                // Since  we call SetSwFormatPageDesc below to generate the
                // 'Following' style of the Break style, it is safer
                // to take  a copy of the contents of aIter.
                wwSection aTmpSection = *aIter;
                // create a new following page style
                SwFormatPageDesc aFollow(SetSwFormatPageDesc(aIter, aStart, bIgnoreCols));
                // restore any contents of aIter trashed by SetSwFormatPageDesc
                *aIter = aTmpSection;

                // Handle the section break
                UseOnPage eUseOnPage = UseOnPage::Left;
                if ( aIter->maSep.bkc == 4 ) // Odd ( right ) Section break
                    eUseOnPage = UseOnPage::Right;

                // Keep the share flags.
                aDesc.GetPageDesc()->SetUseOn( eUseOnPage );
                aDesc.GetPageDesc()->SetFollow( aFollow.GetPageDesc() );
            }

            GiveNodePageDesc(aIter->maStart, aDesc, mrReader.m_rDoc);
        }

        SwTextNode* pTextNd = nullptr;
        if (bInsertSection)
        {
            // Start getting the bounds of this section
            SwPaM aSectPaM(*mrReader.m_pPaM, mrReader.m_pPaM);
            SwNodeIndex aAnchor(aSectPaM.GetPoint()->nNode);
            if (aNext != aEnd)
            {
                aAnchor = aNext->maStart;
                aSectPaM.GetPoint()->nNode = aAnchor;
                aSectPaM.GetPoint()->nContent.Assign(
                    aNext->maStart.GetNode().GetContentNode(), 0);
                aSectPaM.Move(fnMoveBackward);
            }

            const SwPosition* pPos  = aSectPaM.GetPoint();
            SwTextNode const*const pSttNd = pPos->nNode.GetNode().GetTextNode();
            const SwTableNode* pTableNd = pSttNd ? pSttNd->FindTableNode() : nullptr;
            if (pTableNd)
            {
                pTextNd =
                    mrReader.m_rDoc.GetNodes().MakeTextNode(aAnchor,
                    mrReader.m_rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TEXT ));

                aSectPaM.GetPoint()->nNode.Assign(*pTextNd);
                aSectPaM.GetPoint()->nContent.Assign(
                    aSectPaM.GetContentNode(), 0);
            }

            aSectPaM.SetMark();

            aSectPaM.GetPoint()->nNode = aIter->maStart;
            aSectPaM.GetPoint()->nContent.Assign(
                aSectPaM.GetContentNode(), 0);

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
                SwFormatPageDesc aDesc(SetSwFormatPageDesc(aIter, aStart, true));
                if (aDesc.GetPageDesc())
                {
                    sal_uLong nStart = aSectPaM.Start()->nNode.GetIndex();
                    sal_uLong nEnd   = aSectPaM.End()->nNode.GetIndex();
                    for(; nStart <= nEnd; ++nStart)
                    {
                        SwNode* pNode = mrReader.m_rDoc.GetNodes()[nStart];
                        if (!pNode)
                            continue;
                        if (sw::util::HasPageBreak(*pNode))
                        {
                            SwNodeIndex aIdx(*pNode);
                            GiveNodePageDesc(aIdx, aDesc, mrReader.m_rDoc);
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

            // End getting the bounds of this section, quite a job eh?
            SwSectionFormat *pRet = InsertSection(aSectPaM, *aIter);
            // The last section if continuous is always unbalanced
            if (pRet)
            {
                // Set the columns to be UnBalanced if that compatibility option is set
                if (mrReader.m_xWDop->fNoColumnBalance)
                    pRet->SetFormatAttr(SwFormatNoBalancedColumns(true));
                else
                {
                    // Otherwise set to unbalanced if the following section is
                    // not continuous, (which also means that the last section
                    // is unbalanced)
                    if (aNext == aEnd || !aNext->IsContinuous())
                        pRet->SetFormatAttr(SwFormatNoBalancedColumns(true));
                }
            }
        }

        if (pTextNd)
        {
            SwNodeIndex aIdx(*pTextNd);
            SwPaM aTest(aIdx);
            mrReader.m_rDoc.getIDocumentContentOperations().DelFullPara(aTest);
            pTextNd = nullptr;
        }
    }
}

void wwExtraneousParas::delete_all_from_doc()
{
    auto aEnd = m_aTextNodes.rend();
    for (auto aI = m_aTextNodes.rbegin(); aI != aEnd; ++aI)
    {
        SwTextNode *pTextNode = *aI;
        SwNodeIndex aIdx(*pTextNode);
        SwPaM aTest(aIdx);
        m_rDoc.getIDocumentContentOperations().DelFullPara(aTest);
    }
    m_aTextNodes.clear();
}

void SwWW8ImplReader::StoreMacroCmds()
{
    if (m_xWwFib->m_lcbCmds)
    {
        bool bValidPos = checkSeek(*m_pTableStream, m_xWwFib->m_fcCmds);
        if (!bValidPos)
            return;

        uno::Reference < embed::XStorage > xRoot(m_pDocShell->GetStorage());

        if (!xRoot.is())
            return;

        try
        {
            uno::Reference < io::XStream > xStream =
                    xRoot->openStreamElement( SL::aMSMacroCmds, embed::ElementModes::READWRITE );
            std::unique_ptr<SvStream> xOutStream(::utl::UcbStreamHelper::CreateStream(xStream));

            sal_uInt32 lcbCmds = std::min<sal_uInt32>(m_xWwFib->m_lcbCmds, m_pTableStream->remainingSize());
            std::unique_ptr<sal_uInt8[]> xBuffer(new sal_uInt8[lcbCmds]);
            m_xWwFib->m_lcbCmds = m_pTableStream->ReadBytes(xBuffer.get(), lcbCmds);
            xOutStream->WriteBytes(xBuffer.get(), m_xWwFib->m_lcbCmds);
        }
        catch (...)
        {
        }
    }
}

void SwWW8ImplReader::ReadDocVars()
{
    std::vector<OUString> aDocVarStrings;
    std::vector<ww::bytes> aDocVarStringIds;
    std::vector<OUString> aDocValueStrings;
    WW8ReadSTTBF(!m_bVer67, *m_pTableStream, m_xWwFib->m_fcStwUser,
        m_xWwFib->m_lcbStwUser, m_bVer67 ? 2 : 0, m_eStructCharSet,
        aDocVarStrings, &aDocVarStringIds, &aDocValueStrings);
    if (!m_bVer67) {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            m_pDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps(
            xDPS->getDocumentProperties());
        OSL_ENSURE(xDocProps.is(), "DocumentProperties is null");
        uno::Reference<beans::XPropertyContainer> xUserDefinedProps =
            xDocProps->getUserDefinedProperties();
        OSL_ENSURE(xUserDefinedProps.is(), "UserDefinedProperties is null");

        for(size_t i=0; i<aDocVarStrings.size(); i++)
        {
            const OUString &rName = aDocVarStrings[i];
            uno::Any aValue;
            aValue <<= rName;
            try {
                xUserDefinedProps->addProperty( rName,
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
    if( m_pStg )
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            m_pDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps(
            xDPS->getDocumentProperties());
        OSL_ENSURE(xDocProps.is(), "DocumentProperties is null");

        if (xDocProps.is())
        {
            if ( m_xWwFib->m_fDot )
            {
                OUString sTemplateURL;
                SfxMedium* pMedium = m_pDocShell->GetMedium();
                if ( pMedium )
                {
                    const OUString& aName = pMedium->GetName();
                    INetURLObject aURL( aName );
                    sTemplateURL = aURL.GetMainURL(INetURLObject::DecodeMechanism::ToIUri);
                    if ( !sTemplateURL.isEmpty() )
                        xDocProps->setTemplateURL( sTemplateURL );
                }
            }
            else if (m_xWwFib->m_lcbSttbfAssoc) // not a template, and has a SttbfAssoc
            {
                auto nCur = m_pTableStream->Tell();
                Sttb aSttb;
                // point at tgc record
                if (!checkSeek(*m_pTableStream, m_xWwFib->m_fcSttbfAssoc) || !aSttb.Read(*m_pTableStream))
                    SAL_WARN("sw.ww8", "** Read of SttbAssoc data failed!!!! ");
                m_pTableStream->Seek( nCur ); // return to previous position, is that necessary?
                OUString sPath = aSttb.getStringAtIndex( 0x1 );
                OUString aURL;
                // attempt to convert to url (won't work for obvious reasons on linux)
                if ( !sPath.isEmpty() )
                    osl::FileBase::getFileURLFromSystemPath( sPath, aURL );
                if (aURL.isEmpty())
                    xDocProps->setTemplateURL( aURL );
                else
                    xDocProps->setTemplateURL( sPath );

            }
            sfx2::LoadOlePropertySet(xDocProps, m_pStg);
        }
    }
}

static void lcl_createTemplateToProjectEntry( const uno::Reference< container::XNameContainer >& xPrjNameCache, const OUString& sTemplatePathOrURL, const OUString& sVBAProjName )
{
    if ( xPrjNameCache.is() )
    {
        INetURLObject aObj;
        aObj.SetURL( sTemplatePathOrURL );
        bool bIsURL = aObj.GetProtocol() != INetProtocol::NotValid;
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
    WW8Customizations( SvStream*, WW8Fib const & );
    void  Import( SwDocShell* pShell );
};

WW8Customizations::WW8Customizations( SvStream* pTableStream, WW8Fib const & rFib ) : mpTableStream(pTableStream), mWw8Fib( rFib )
{
}

void WW8Customizations::Import( SwDocShell* pShell )
{
    if ( mWw8Fib.m_lcbCmds == 0 || !IsEightPlus(mWw8Fib.GetFIBVersion()) )
        return;
    try
    {
        Tcg aTCG;
        long nCur = mpTableStream->Tell();
        if (!checkSeek(*mpTableStream, mWw8Fib.m_fcCmds)) // point at tgc record
        {
            SAL_WARN("sw.ww8", "** Seek to Customization data failed!!!! ");
            return;
        }
        bool bReadResult = aTCG.Read( *mpTableStream );
        mpTableStream->Seek( nCur ); // return to previous position, is that necessary?
        if ( !bReadResult )
        {
            SAL_WARN("sw.ww8", "** Read of Customization data failed!!!! ");
            return;
        }
        aTCG.ImportCustomToolBar( *pShell );
    }
    catch(...)
    {
        SAL_WARN("sw.ww8", "** Read of Customization data failed!!!! epically");
    }
}

void SwWW8ImplReader::ReadGlobalTemplateSettings( const OUString& sCreatedFrom, const uno::Reference< container::XNameContainer >& xPrjNameCache )
{
    if (utl::ConfigManager::IsFuzzing())
        return;

    SvtPathOptions aPathOpt;
    const OUString& aAddinPath = aPathOpt.GetAddinPath();
    uno::Sequence< OUString > sGlobalTemplates;

    // first get the autoload addins in the directory STARTUP
    uno::Reference<ucb::XSimpleFileAccess3> xSFA(ucb::SimpleFileAccess::create(::comphelper::getProcessComponentContext()));

    if( xSFA->isFolder( aAddinPath ) )
        sGlobalTemplates = xSFA->getFolderContents( aAddinPath, false );

    sal_Int32 nEntries = sGlobalTemplates.getLength();
    for ( sal_Int32 i=0; i<nEntries; ++i )
    {
        INetURLObject aObj;
        aObj.SetURL( sGlobalTemplates[ i ] );
        bool bIsURL = aObj.GetProtocol() != INetProtocol::NotValid;
        OUString aURL;
        if ( bIsURL )
                aURL = sGlobalTemplates[ i ];
        else
                osl::FileBase::getFileURLFromSystemPath( sGlobalTemplates[ i ], aURL );
        if ( !aURL.endsWithIgnoreAsciiCase( ".dot" ) || ( !sCreatedFrom.isEmpty() && sCreatedFrom == aURL ) )
            continue; // don't try and read the same document as ourselves

        tools::SvRef<SotStorage> rRoot = new SotStorage( aURL, StreamMode::STD_READWRITE );

        BasicProjImportHelper aBasicImporter( *m_pDocShell );
        // Import vba via oox filter
        aBasicImporter.import( m_pDocShell->GetMedium()->GetInputStream() );
        lcl_createTemplateToProjectEntry( xPrjNameCache, aURL, aBasicImporter.getProjectName() );
        // Read toolbars & menus
        tools::SvRef<SotStorageStream> refMainStream = rRoot->OpenSotStream( "WordDocument");
        refMainStream->SetEndian(SvStreamEndian::LITTLE);
        WW8Fib aWwFib( *refMainStream, 8 );
        tools::SvRef<SotStorageStream> xTableStream =
                rRoot->OpenSotStream(aWwFib.m_fWhichTableStm ? SL::a1Table : SL::a0Table, StreamMode::STD_READ);

        if (xTableStream.is() && ERRCODE_NONE == xTableStream->GetError())
        {
            xTableStream->SetEndian(SvStreamEndian::LITTLE);
            WW8Customizations aGblCustomisations( xTableStream.get(), aWwFib );
            aGblCustomisations.Import( m_pDocShell );
        }
    }
}

ErrCode SwWW8ImplReader::CoreLoad(WW8Glossary const *pGloss)
{
    m_rDoc.SetDocumentType( SwDoc::DOCTYPE_MSWORD );
    if (m_bNewDoc && m_pStg && !pGloss)
    {
        // Initialize RDF metadata, to be able to add statements during the import.
        try
        {
            uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(m_rDoc.GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW);
            uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
            uno::Reference<embed::XStorage> xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
            const uno::Reference<rdf::XURI> xBaseURI(sfx2::createBaseURI(xComponentContext, xStorage, m_sBaseURL));
            uno::Reference<task::XInteractionHandler> xHandler;
            xDocumentMetadataAccess->loadMetadataFromStorage(xStorage, xBaseURI, xHandler);
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("sw.ww8", "failed to initialize RDF metadata");
        }
        ReadDocInfo();
    }

    ::ww8::WW8FibData * pFibData = new ::ww8::WW8FibData();

    if (m_xWwFib->m_fReadOnlyRecommended)
        pFibData->setReadOnlyRecommended(true);
    else
        pFibData->setReadOnlyRecommended(false);

    if (m_xWwFib->m_fWriteReservation)
        pFibData->setWriteReservation(true);
    else
        pFibData->setWriteReservation(false);

    ::sw::tExternalDataPointer pExternalFibData(pFibData);

    m_rDoc.getIDocumentExternalData().setExternalData(::sw::tExternalDataType::FIB, pExternalFibData);

    ::sw::tExternalDataPointer pSttbfAsoc
          (new ::ww8::WW8Sttb<ww8::WW8Struct>(*m_pTableStream, m_xWwFib->m_fcSttbfAssoc, m_xWwFib->m_lcbSttbfAssoc));

    m_rDoc.getIDocumentExternalData().setExternalData(::sw::tExternalDataType::STTBF_ASSOC, pSttbfAsoc);

    if (m_xWwFib->m_fWriteReservation || m_xWwFib->m_fReadOnlyRecommended)
    {
        SwDocShell * pDocShell = m_rDoc.GetDocShell();
        if (pDocShell)
            pDocShell->SetReadOnlyUI();
    }

    m_pPaM = mpCursor.get();

    m_xCtrlStck.reset(new SwWW8FltControlStack(&m_rDoc, m_nFieldFlags, *this));

    m_xRedlineStack.reset(new sw::util::RedlineStack(m_rDoc));

    /*
        RefFieldStck: Keeps track of bookmarks which may be inserted as
        variables instead.
    */
    m_xReffedStck.reset(new SwWW8ReferencedFltEndStack(&m_rDoc, m_nFieldFlags));
    m_xReffingStck.reset(new SwWW8FltRefStack(&m_rDoc, m_nFieldFlags));

    m_xAnchorStck.reset(new SwWW8FltAnchorStack(&m_rDoc, m_nFieldFlags));

    size_t nPageDescOffset = m_rDoc.GetPageDescCnt();

    SwNodeIndex aSttNdIdx( m_rDoc.GetNodes() );
    SwRelNumRuleSpaces aRelNumRule(m_rDoc, m_bNewDoc);

    RedlineFlags eMode = RedlineFlags::ShowInsert;

    m_xSprmParser.reset(new wwSprmParser(*m_xWwFib));

    // Set handy helper variables
    m_bVer6  = (6 == m_xWwFib->m_nVersion);
    m_bVer7  = (7 == m_xWwFib->m_nVersion);
    m_bVer67 = m_bVer6 || m_bVer7;
    m_bVer8  = (8 == m_xWwFib->m_nVersion);

    m_eTextCharSet = WW8Fib::GetFIBCharset(m_xWwFib->m_chse, m_xWwFib->m_lid);
    m_eStructCharSet = WW8Fib::GetFIBCharset(m_xWwFib->m_chseTables, m_xWwFib->m_lid);

    m_bWWBugNormal = m_xWwFib->m_nProduct == 0xc03d;

    if (!m_bNewDoc)
        aSttNdIdx = m_pPaM->GetPoint()->nNode;

    m_xProgress.reset(new ImportProgress(m_pDocShell, 0, 100));

    // read Font Table
    m_xFonts.reset(new WW8Fonts(*m_pTableStream, *m_xWwFib));

    // Document Properties
    m_xWDop.reset(new WW8Dop(*m_pTableStream, m_xWwFib->m_nFib, m_xWwFib->m_fcDop,
        m_xWwFib->m_lcbDop));

    if (m_bNewDoc)
        ImportDop();

    /*
        Import revisioning data: author names
    */
    if( m_xWwFib->m_lcbSttbfRMark )
    {
        ReadRevMarkAuthorStrTabl(*m_pTableStream,
                                 m_xWwFib->m_fcSttbfRMark,
                                 m_xWwFib->m_lcbSttbfRMark, m_rDoc);
    }

    // Initialize our String/ID map for Linked Sections
    std::vector<OUString> aLinkStrings;
    std::vector<ww::bytes> aStringIds;

    WW8ReadSTTBF(!m_bVer67, *m_pTableStream, m_xWwFib->m_fcSttbFnm,
        m_xWwFib->m_lcbSttbFnm, m_bVer67 ? 2 : 0, m_eStructCharSet,
        aLinkStrings, &aStringIds);

    for (size_t i=0; i < aLinkStrings.size() && i < aStringIds.size(); ++i)
    {
        const ww::bytes& stringId = aStringIds[i];
        if (stringId.size() < sizeof(WW8_STRINGID))
        {
            SAL_WARN("sw.ww8", "SwWW8ImplReader::CoreLoad: WW8_STRINGID is too short");
            continue;
        }
        const WW8_STRINGID *stringIdStruct = reinterpret_cast<const WW8_STRINGID*>(stringId.data());
        m_aLinkStringMap[SVBT16ToShort(stringIdStruct->nStringId)] = aLinkStrings[i];
    }

    ReadDocVars(); // import document variables as meta information.

    m_xProgress->Update(m_nProgress);    // Update

    m_xLstManager.reset(new WW8ListManager(*m_pTableStream, *this));

    /*
        first (1) import all styles (see WW8PAR2.CXX)
            BEFORE the import of the lists !!
    */
    m_xProgress->Update(m_nProgress);    // Update
    m_xStyles.reset(new WW8RStyle(*m_xWwFib, this)); // Styles
    m_xStyles->Import();

    /*
        In the end: (also see WW8PAR3.CXX)

        Go through all Styles and attach respective List Format
        AFTER we imported the Styles and AFTER we imported the Lists!
    */
    m_xProgress->Update(m_nProgress); // Update
    m_xStyles->PostProcessStyles();

    if (!m_vColl.empty())
        SetOutlineStyles();

    m_xSBase.reset(new WW8ScannerBase(m_pStrm,m_pTableStream,m_pDataStream, m_xWwFib.get()));

    static const SvxNumType eNumTA[16] =
    {
        SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
        SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
        SVX_NUM_ARABIC, SVX_NUM_ARABIC
    };

    if (m_xSBase->AreThereFootnotes())
    {
        static const SwFootnoteNum eNumA[4] =
        {
            FTNNUM_DOC, FTNNUM_CHAPTER, FTNNUM_PAGE, FTNNUM_DOC
        };

        SwFootnoteInfo aInfo;
        aInfo = m_rDoc.GetFootnoteInfo(); // Copy-Ctor private

        aInfo.ePos = FTNPOS_PAGE;
        aInfo.eNum = eNumA[m_xWDop->rncFootnote];
        sal_uInt16 nfcFootnoteRef = m_xWDop->nfcFootnoteRef & 0xF;
        aInfo.aFormat.SetNumberingType( eNumTA[nfcFootnoteRef] );
        if( m_xWDop->nFootnote )
            aInfo.nFootnoteOffset = m_xWDop->nFootnote - 1;
        m_rDoc.SetFootnoteInfo( aInfo );
    }
    if (m_xSBase->AreThereEndnotes())
    {
        SwEndNoteInfo aInfo;
        aInfo = m_rDoc.GetEndNoteInfo(); // Same as for Footnote
        sal_uInt16 nfcEdnRef = m_xWDop->nfcEdnRef & 0xF;
        aInfo.aFormat.SetNumberingType( eNumTA[nfcEdnRef] );
        if( m_xWDop->nEdn )
            aInfo.nFootnoteOffset = m_xWDop->nEdn - 1;
        m_rDoc.SetEndNoteInfo( aInfo );
    }

    if (m_xWwFib->m_lcbPlcfhdd)
        m_xHdFt.reset(new WW8PLCF_HdFt(m_pTableStream, *m_xWwFib, *m_xWDop));

    if (!m_bNewDoc)
    {
        // inserting into an existing document:
        // As only complete paragraphs are inserted, the current one
        // needs to be split - once or even twice.
        const SwPosition* pPos = m_pPaM->GetPoint();

        // split current paragraph to get new paragraph for the insertion
        m_rDoc.getIDocumentContentOperations().SplitNode( *pPos, false );

        // another split, if insertion position was not at the end of the current paragraph.
        SwTextNode const*const pTextNd = pPos->nNode.GetNode().GetTextNode();
        if ( pTextNd->GetText().getLength() )
        {
            m_rDoc.getIDocumentContentOperations().SplitNode( *pPos, false );
            // move PaM back to the newly empty paragraph
            m_pPaM->Move( fnMoveBackward );
        }

        // suppress insertion of tables inside footnotes.
        const sal_uLong nNd = pPos->nNode.GetIndex();
        m_bReadNoTable = ( nNd < m_rDoc.GetNodes().GetEndOfInserts().GetIndex() &&
                       m_rDoc.GetNodes().GetEndOfInserts().StartOfSectionIndex() < nNd );
    }

    m_xProgress->Update(m_nProgress); // Update

    // loop for each glossary entry and add dummy section node
    if (pGloss)
    {
        WW8PLCF aPlc(*m_pTableStream, m_xWwFib->m_fcPlcfglsy, m_xWwFib->m_lcbPlcfglsy, 0);

        WW8_CP nStart, nEnd;
        void* pDummy;

        for (int i = 0; i < pGloss->GetNoStrings(); ++i, aPlc.advance())
        {
            SwNodeIndex aIdx( m_rDoc.GetNodes().GetEndOfContent());
            SwTextFormatColl* pColl =
                m_rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD,
                false);
            SwStartNode *pNode =
                m_rDoc.GetNodes().MakeTextSection(aIdx,
                SwNormalStartNode,pColl);
            m_pPaM->GetPoint()->nNode = pNode->GetIndex()+1;
            m_pPaM->GetPoint()->nContent.Assign(m_pPaM->GetContentNode(),0);
            aPlc.Get( nStart, nEnd, pDummy );
            ReadText(nStart,nEnd-nStart-1,MAN_MAINTEXT);
        }
    }
    else // ordinary case
    {
        if (m_bNewDoc && m_pStg && !pGloss) /*meaningless for a glossary */
        {
            m_pDocShell->SetIsTemplate( m_xWwFib->m_fDot ); // point at tgc record
            uno::Reference<document::XDocumentPropertiesSupplier> const
                xDocPropSupp(m_pDocShell->GetModel(), uno::UNO_QUERY_THROW);
            uno::Reference< document::XDocumentProperties > xDocProps( xDocPropSupp->getDocumentProperties(), uno::UNO_QUERY_THROW );

            OUString sCreatedFrom = xDocProps->getTemplateURL();
            uno::Reference< container::XNameContainer > xPrjNameCache;
            uno::Reference< lang::XMultiServiceFactory> xSF(m_pDocShell->GetModel(), uno::UNO_QUERY);
            if ( xSF.is() )
                xPrjNameCache.set( xSF->createInstance( "ooo.vba.VBAProjectNameProvider" ), uno::UNO_QUERY );

            // Read Global templates
            ReadGlobalTemplateSettings( sCreatedFrom, xPrjNameCache );

            // Create and insert Word vba Globals
            uno::Any aGlobs;
            uno::Sequence< uno::Any > aArgs(1);
            aArgs[ 0 ] <<= m_pDocShell->GetModel();
            try
            {
                aGlobs <<= ::comphelper::getProcessServiceFactory()->createInstanceWithArguments( "ooo.vba.word.Globals", aArgs );
            }
            catch (const uno::Exception&)
            {
                SAL_WARN("sw.ww8", "SwWW8ImplReader::CoreLoad: ooo.vba.word.Globals is not available");
            }

#if HAVE_FEATURE_SCRIPTING
            if (!utl::ConfigManager::IsFuzzing())
            {
                BasicManager *pBasicMan = m_pDocShell->GetBasicManager();
                if (pBasicMan)
                    pBasicMan->SetGlobalUNOConstant( "VBAGlobals", aGlobs );
            }
#endif
            BasicProjImportHelper aBasicImporter( *m_pDocShell );
            // Import vba via oox filter
            bool bRet = aBasicImporter.import( m_pDocShell->GetMedium()->GetInputStream() );

            lcl_createTemplateToProjectEntry( xPrjNameCache, sCreatedFrom, aBasicImporter.getProjectName() );
            WW8Customizations aCustomisations( m_pTableStream, *m_xWwFib );
            aCustomisations.Import( m_pDocShell );

            if( bRet )
                m_rDoc.SetContainsMSVBasic(true);

            StoreMacroCmds();
        }
        m_bOnLoadingMain = true;
        ReadText(0, m_xWwFib->m_ccpText, MAN_MAINTEXT);
        m_bOnLoadingMain = false;
    }

    m_xProgress->Update(m_nProgress); // Update

    if (m_pDrawPg && m_xMSDffManager && m_xMSDffManager->GetShapeOrders())
    {
        // Helper array to chain the inserted frames (instead of SdrTextObj)
        SvxMSDffShapeTxBxSort aTxBxSort;

        // Ensure correct z-order of read Escher objects
        sal_uInt16 nShapeCount = m_xMSDffManager->GetShapeOrders()->size();

        for (sal_uInt16 nShapeNum=0; nShapeNum < nShapeCount; nShapeNum++)
        {
            SvxMSDffShapeOrder *pOrder =
                (*m_xMSDffManager->GetShapeOrders())[nShapeNum].get();
            // Insert Pointer into new Sort array
            if (pOrder->nTxBxComp && pOrder->pFly)
                aTxBxSort.insert(pOrder);
        }
        // Chain Frames
        if( !aTxBxSort.empty() )
        {
            SwFormatChain aChain;
            for( SvxMSDffShapeTxBxSort::iterator it = aTxBxSort.begin(); it != aTxBxSort.end(); ++it )
            {
                SvxMSDffShapeOrder *pOrder = *it;

                // Initialize FlyFrame Formats
                SwFlyFrameFormat* pFlyFormat     = pOrder->pFly;
                SwFlyFrameFormat* pNextFlyFormat = nullptr;
                SwFlyFrameFormat* pPrevFlyFormat = nullptr;

                // Determine successor, if we can
                SvxMSDffShapeTxBxSort::iterator tmpIter1 = it;
                ++tmpIter1;
                if( tmpIter1 != aTxBxSort.end() )
                {
                    SvxMSDffShapeOrder *pNextOrder = *tmpIter1;
                    if ((0xFFFF0000 & pOrder->nTxBxComp)
                           == (0xFFFF0000 & pNextOrder->nTxBxComp))
                        pNextFlyFormat = pNextOrder->pFly;
                }
                // Determine predecessor, if we can
                if( it != aTxBxSort.begin() )
                {
                    SvxMSDffShapeTxBxSort::iterator tmpIter2 = it;
                    --tmpIter2;
                    SvxMSDffShapeOrder *pPrevOrder = *tmpIter2;
                    if ((0xFFFF0000 & pOrder->nTxBxComp)
                           == (0xFFFF0000 & pPrevOrder->nTxBxComp))
                        pPrevFlyFormat = pPrevOrder->pFly;
                }
                // If successor or predecessor present, insert the
                // chain at the FlyFrame Format
                if (pNextFlyFormat || pPrevFlyFormat)
                {
                    aChain.SetNext( pNextFlyFormat );
                    aChain.SetPrev( pPrevFlyFormat );
                    pFlyFormat->SetFormatAttr( aChain );
                }
            }
        }
    }

    if (m_bNewDoc)
    {
        if( m_xWDop->fRevMarking )
            eMode |= RedlineFlags::On;
        if( m_xWDop->fRMView )
            eMode |= RedlineFlags::ShowDelete;
    }

    m_aInsertedTables.DelAndMakeTableFrames();
    m_aSectionManager.InsertSegments();

    m_vColl.clear();

    m_xStyles.reset();

    m_xFormImpl.reset();
    GrafikDtor();
    m_xMSDffManager.reset();
    m_xHdFt.reset();
    m_xSBase.reset();
    m_xWDop.reset();
    m_xFonts.reset();
    m_pAtnNames.reset();
    m_xSprmParser.reset();
    m_xProgress.reset();

    m_pDataStream = nullptr;
    m_pTableStream = nullptr;

    DeleteCtrlStack();
    DeleteAnchorStack();
    DeleteRefStacks();
    m_pLastAnchorPos.reset();//ensure this is deleted before UpdatePageDescs
    // remove extra paragraphs after attribute ctrl
    // stacks etc. are destroyed, and before fields
    // are updated
    m_aExtraneousParas.delete_all_from_doc();
    // ofz#10994 remove any trailing fly paras before processing redlines
    m_xWFlyPara.reset();
    m_xSFlyPara.reset();
    m_xRedlineStack->closeall(*m_pPaM->GetPoint());
    while (!m_aFrameRedlines.empty())
        m_aFrameRedlines.pop();
    m_xRedlineStack.reset();

    // For i120928,achieve the graphics from the special bookmark with is for graphic bullet
    {
        std::vector<const SwGrfNode*> vecBulletGrf;
        std::vector<SwFrameFormat*> vecFrameFormat;

        IDocumentMarkAccess* const pMarkAccess = m_rDoc.getIDocumentMarkAccess();
        if ( pMarkAccess )
        {
            IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->findBookmark( "_PictureBullets" );
            if ( ppBkmk != pMarkAccess->getBookmarksEnd() &&
                       IDocumentMarkAccess::GetType( *(ppBkmk->get()) ) == IDocumentMarkAccess::MarkType::BOOKMARK )
            {
                SwTextNode* pTextNode = ppBkmk->get()->GetMarkStart().nNode.GetNode().GetTextNode();

                if ( pTextNode )
                {
                    const SwpHints* pHints = pTextNode->GetpSwpHints();
                    for( size_t nHintPos = 0; pHints && nHintPos < pHints->Count(); ++nHintPos)
                    {
                        const SwTextAttr *pHt = pHints->Get(nHintPos);
                        const sal_Int32 st = pHt->GetStart();
                        if( pHt
                            && pHt->Which() == RES_TXTATR_FLYCNT
                            && (st >= ppBkmk->get()->GetMarkStart().nContent.GetIndex()) )
                        {
                            SwFrameFormat* pFrameFormat = pHt->GetFlyCnt().GetFrameFormat();
                            vecFrameFormat.push_back(pFrameFormat);
                            const SwNodeIndex* pNdIdx = pFrameFormat->GetContent().GetContentIdx();
                            const SwNodes* pNodesArray = (pNdIdx != nullptr)
                                                         ? &(pNdIdx->GetNodes())
                                                         : nullptr;
                            const SwGrfNode *pGrf = (pNodesArray != nullptr)
                                                    ? dynamic_cast<const SwGrfNode*>((*pNodesArray)[pNdIdx->GetIndex() + 1])
                                                    : nullptr;
                            vecBulletGrf.push_back(pGrf);
                        }
                    }
                    // update graphic bullet information
                    size_t nCount = m_xLstManager->GetWW8LSTInfoNum();
                    for (size_t i = 0; i < nCount; ++i)
                    {
                        SwNumRule* pRule = m_xLstManager->GetNumRule(i);
                        for (sal_uInt16 j = 0; j < MAXLEVEL; ++j)
                        {
                            SwNumFormat aNumFormat(pRule->Get(j));
                            const sal_Int16 nType = aNumFormat.GetNumberingType();
                            const sal_uInt16 nGrfBulletCP = aNumFormat.GetGrfBulletCP();
                            if ( nType == SVX_NUM_BITMAP
                                 && vecBulletGrf.size() > nGrfBulletCP
                                 && vecBulletGrf[nGrfBulletCP] != nullptr )
                            {
                                Graphic aGraphic = vecBulletGrf[nGrfBulletCP]->GetGrf();
                                SvxBrushItem aBrush(aGraphic, GPOS_AREA, SID_ATTR_BRUSH);
                                const vcl::Font& aFont = numfunc::GetDefBulletFont();
                                int nHeight = aFont.GetFontHeight() * 12;
                                Size aPrefSize( aGraphic.GetPrefSize());
                                if (aPrefSize.Height() * aPrefSize.Width() != 0 )
                                {
                                    int nWidth = (nHeight * aPrefSize.Width()) / aPrefSize.Height();
                                    Size aSize(nWidth, nHeight);
                                    aNumFormat.SetGraphicBrush(&aBrush, &aSize);
                                }
                                else
                                {
                                    aNumFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                                    aNumFormat.SetBulletChar(0x2190);
                                }
                                pRule->Set( j, aNumFormat );
                            }
                        }
                    }
                    // Remove additional pictures
                    for (SwFrameFormat* p : vecFrameFormat)
                    {
                        m_rDoc.getIDocumentLayoutAccess().DelLayoutFormat(p);
                    }
                }
            }
        }
        m_xLstManager.reset();
    }

    m_pPosAfterTOC.reset();
    mpCursor.reset();
    m_pPaM = nullptr;

    UpdateFields();

    // delete the pam before the call for hide all redlines (Bug 73683)
    if (m_bNewDoc)
      m_rDoc.getIDocumentRedlineAccess().SetRedlineFlags(eMode);

    UpdatePageDescs(m_rDoc, nPageDescOffset);

    return ERRCODE_NONE;
}

ErrCode SwWW8ImplReader::SetSubStreams(tools::SvRef<SotStorageStream> &rTableStream,
    tools::SvRef<SotStorageStream> &rDataStream)
{
    ErrCode nErrRet = ERRCODE_NONE;
    // 6 stands for "6 OR 7", 7 stands for "ONLY 7"
    switch (m_xWwFib->m_nVersion)
    {
        case 6:
        case 7:
            m_pTableStream = m_pStrm;
            m_pDataStream = m_pStrm;
            break;
        case 8:
            if(!m_pStg)
            {
                OSL_ENSURE( m_pStg, "Version 8 always needs to have a Storage!!" );
                nErrRet = ERR_SWG_READ_ERROR;
                break;
            }

            rTableStream = m_pStg->OpenSotStream(
                m_xWwFib->m_fWhichTableStm ? SL::a1Table : SL::a0Table,
                StreamMode::STD_READ);

            m_pTableStream = rTableStream.get();
            m_pTableStream->SetEndian( SvStreamEndian::LITTLE );

            rDataStream = m_pStg->OpenSotStream(SL::aData, StreamMode::STD_READ);

            if (rDataStream.is() && ERRCODE_NONE == rDataStream->GetError())
            {
                m_pDataStream = rDataStream.get();
                m_pDataStream->SetEndian(SvStreamEndian::LITTLE);
            }
            else
                m_pDataStream = m_pStrm;
            break;
        default:
            // Program error!
            OSL_ENSURE( false, "We forgot to encode nVersion!" );
            nErrRet = ERR_SWG_READ_ERROR;
            break;
    }
    return nErrRet;
}

namespace
{
    std::unique_ptr<utl::TempFile> MakeTemp(SvFileStream &rSt)
    {
        std::unique_ptr<utl::TempFile> pT(new utl::TempFile);
        pT->EnableKillingFile();
        rSt.Open(pT->GetFileName(), StreamMode::READWRITE | StreamMode::SHARE_DENYWRITE);
        return pT;
    }

#define WW_BLOCKSIZE 0x200

    void DecryptRC4(msfilter::MSCodec97& rCtx, SvStream &rIn, SvStream &rOut)
    {
        const std::size_t nLen = rIn.TellEnd();
        rIn.Seek(0);

        sal_uInt8 in[WW_BLOCKSIZE];
        for (std::size_t nI = 0, nBlock = 0; nI < nLen; nI += WW_BLOCKSIZE, ++nBlock)
        {
            std::size_t nBS = std::min<size_t>(nLen - nI, WW_BLOCKSIZE);
            nBS = rIn.ReadBytes(in, nBS);
            rCtx.InitCipher(nBlock);
            rCtx.Decode(in, nBS, in, nBS);
            rOut.WriteBytes(in, nBS);
        }
    }

    void DecryptXOR(msfilter::MSCodec_XorWord95 &rCtx, SvStream &rIn, SvStream &rOut)
    {
        std::size_t nSt = rIn.Tell();
        std::size_t nLen = rIn.TellEnd();

        rCtx.InitCipher();
        rCtx.Skip(nSt);

        sal_uInt8 in[0x4096];
        for (std::size_t nI = nSt; nI < nLen; nI += 0x4096)
        {
            std::size_t nBS = std::min<size_t>(nLen - nI, 0x4096 );
            nBS = rIn.ReadBytes(in, nBS);
            rCtx.Decode(in, nBS);
            rOut.WriteBytes(in, nBS);
        }
    }

    // moan, copy and paste :-(
    OUString QueryPasswordForMedium(SfxMedium& rMedium)
    {
        OUString aPassw;

        const SfxItemSet* pSet = rMedium.GetItemSet();
        const SfxPoolItem *pPasswordItem;

        if(pSet && SfxItemState::SET == pSet->GetItemState(SID_PASSWORD, true, &pPasswordItem))
            aPassw = static_cast<const SfxStringItem *>(pPasswordItem)->GetValue();
        else
        {
            try
            {
                uno::Reference< task::XInteractionHandler > xHandler( rMedium.GetInteractionHandler() );
                if( xHandler.is() )
                {
                    ::comphelper::DocPasswordRequest* pRequest = new ::comphelper::DocPasswordRequest(
                        ::comphelper::DocPasswordRequestType::MS, task::PasswordRequestMode_PASSWORD_ENTER,
                        INetURLObject( rMedium.GetOrigURL() ).GetName( INetURLObject::DecodeMechanism::WithCharset ) );
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

    uno::Sequence< beans::NamedValue > InitXorWord95Codec( ::msfilter::MSCodec_XorWord95& rCodec, SfxMedium& rMedium, WW8Fib const * pWwFib )
    {
        uno::Sequence< beans::NamedValue > aEncryptionData;
        const SfxUnoAnyItem* pEncryptionData = SfxItemSet::GetItem<SfxUnoAnyItem>(rMedium.GetItemSet(), SID_ENCRYPTIONDATA, false);
        if ( pEncryptionData && ( pEncryptionData->GetValue() >>= aEncryptionData ) && !rCodec.InitCodec( aEncryptionData ) )
            aEncryptionData.realloc( 0 );

        if ( !aEncryptionData.getLength() )
        {
            OUString sUniPassword = QueryPasswordForMedium( rMedium );

            OString sPassword(OUStringToOString(sUniPassword,
                WW8Fib::GetFIBCharset(pWwFib->m_chseTables, pWwFib->m_lid)));

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

                rtlRandomPool aRandomPool = rtl_random_createPool();
                sal_uInt8 pDocId[ 16 ];
                rtl_random_getBytes( aRandomPool, pDocId, 16 );

                rtl_random_destroyPool( aRandomPool );

                sal_uInt16 pStd97Pass[16];
                memset( pStd97Pass, 0, sizeof( pStd97Pass ) );
                for( sal_Int32 nChar = 0; nChar < nLen; ++nChar )
                    pStd97Pass[nChar] = sUniPassword[nChar];

                aCodec97.InitKey( pStd97Pass, pDocId );

                // merge the EncryptionData, there should be no conflicts
                ::comphelper::SequenceAsHashMap aEncryptionHash( aEncryptionData );
                aEncryptionHash.update( ::comphelper::SequenceAsHashMap( aCodec97.GetEncryptionData() ) );
                aEncryptionHash >> aEncryptionData;
            }
        }

        return aEncryptionData;
    }

    uno::Sequence< beans::NamedValue > Init97Codec(msfilter::MSCodec97& rCodec, sal_uInt8 const pDocId[16], SfxMedium& rMedium)
    {
        uno::Sequence< beans::NamedValue > aEncryptionData;
        const SfxUnoAnyItem* pEncryptionData = SfxItemSet::GetItem<SfxUnoAnyItem>(rMedium.GetItemSet(), SID_ENCRYPTIONDATA, false);
        if ( pEncryptionData && ( pEncryptionData->GetValue() >>= aEncryptionData ) && !rCodec.InitCodec( aEncryptionData ) )
            aEncryptionData.realloc( 0 );

        if ( !aEncryptionData.getLength() )
        {
            OUString sUniPassword = QueryPasswordForMedium( rMedium );

            sal_Int32 nLen = sUniPassword.getLength();
            if ( nLen <= 15 )
            {
                sal_uInt16 pPassword[16];
                memset( pPassword, 0, sizeof( pPassword ) );
                for( sal_Int32 nChar = 0; nChar < nLen; ++nChar )
                    pPassword[nChar] = sUniPassword[nChar];

                rCodec.InitKey( pPassword, pDocId );
                aEncryptionData = rCodec.GetEncryptionData();
            }
        }

        return aEncryptionData;
    }
}

//TO-DO: merge this with lclReadFilepass8_Strong in sc which uses a different
//stream thing
static bool lclReadCryptoAPIHeader(msfilter::RC4EncryptionInfo &info, SvStream &rStream)
{
    //It is possible there are other variants in existence but these
    //are the defaults I get with Word 2013

    rStream.ReadUInt32(info.header.flags);
    if (oox::getFlag( info.header.flags, msfilter::ENCRYPTINFO_EXTERNAL))
        return false;

    sal_uInt32 nHeaderSize(0);
    rStream.ReadUInt32(nHeaderSize);
    sal_uInt32 actualHeaderSize = sizeof(info.header);

    if (nHeaderSize < actualHeaderSize)
        return false;

    rStream.ReadUInt32(info.header.flags);
    rStream.ReadUInt32(info.header.sizeExtra);
    rStream.ReadUInt32(info.header.algId);
    rStream.ReadUInt32(info.header.algIdHash);
    rStream.ReadUInt32(info.header.keyBits);
    rStream.ReadUInt32(info.header.providedType);
    rStream.ReadUInt32(info.header.reserved1);
    rStream.ReadUInt32(info.header.reserved2);

    rStream.SeekRel(nHeaderSize - actualHeaderSize);

    rStream.ReadUInt32(info.verifier.saltSize);
    if (info.verifier.saltSize != msfilter::SALT_LENGTH)
        return false;
    rStream.ReadBytes(&info.verifier.salt, sizeof(info.verifier.salt));
    rStream.ReadBytes(&info.verifier.encryptedVerifier, sizeof(info.verifier.encryptedVerifier));

    rStream.ReadUInt32(info.verifier.encryptedVerifierHashSize);
    if (info.verifier.encryptedVerifierHashSize != RTL_DIGEST_LENGTH_SHA1)
        return false;
    rStream.ReadBytes(&info.verifier.encryptedVerifierHash, info.verifier.encryptedVerifierHashSize);

    // check flags and algorithm IDs, required are AES128 and SHA-1
    if (!oox::getFlag(info.header.flags, msfilter::ENCRYPTINFO_CRYPTOAPI))
        return false;

    if (oox::getFlag(info.header.flags, msfilter::ENCRYPTINFO_AES))
        return false;

    if (info.header.algId != msfilter::ENCRYPT_ALGO_RC4)
        return false;

    // hash algorithm ID 0 defaults to SHA-1 too
    if (info.header.algIdHash != 0 && info.header.algIdHash != msfilter::ENCRYPT_HASH_SHA1)
        return false;

    return true;
}

ErrCode SwWW8ImplReader::LoadThroughDecryption(WW8Glossary *pGloss)
{
    ErrCode nErrRet = ERRCODE_NONE;
    if (pGloss)
        m_xWwFib = pGloss->GetFib();
    else
        m_xWwFib.reset(new WW8Fib(*m_pStrm, m_nWantedVersion));

    if (m_xWwFib->m_nFibError)
        nErrRet = ERR_SWG_READ_ERROR;

    tools::SvRef<SotStorageStream> xTableStream, xDataStream;

    if (!nErrRet)
        nErrRet = SetSubStreams(xTableStream, xDataStream);

    std::unique_ptr<utl::TempFile> pTempMain;
    std::unique_ptr<utl::TempFile> pTempTable;
    std::unique_ptr<utl::TempFile> pTempData;
    SvFileStream aDecryptMain;
    SvFileStream aDecryptTable;
    SvFileStream aDecryptData;

    bool bDecrypt = false;
    enum {RC4CryptoAPI, RC4, XOR, Other} eAlgo = Other;
    if (m_xWwFib->m_fEncrypted && !nErrRet)
    {
        if (!pGloss)
        {
            bDecrypt = true;
            if (8 != m_xWwFib->m_nVersion)
                eAlgo = XOR;
            else
            {
                if (m_xWwFib->m_nKey != 0)
                    eAlgo = XOR;
                else
                {
                    m_pTableStream->Seek(0);
                    sal_uInt32 nEncType(0);
                    m_pTableStream->ReadUInt32(nEncType);
                    if (nEncType == msfilter::VERSION_INFO_1997_FORMAT)
                        eAlgo = RC4;
                    else if (nEncType == msfilter::VERSION_INFO_2007_FORMAT || nEncType == msfilter::VERSION_INFO_2007_FORMAT_SP2)
                        eAlgo = RC4CryptoAPI;
                }
            }
        }
    }

    if (bDecrypt)
    {
        nErrRet = ERRCODE_SVX_WRONGPASS;
        SfxMedium* pMedium = m_pDocShell->GetMedium();

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
                    uno::Sequence< beans::NamedValue > aEncryptionData = InitXorWord95Codec(aCtx, *pMedium, m_xWwFib.get());

                    // if initialization has failed the EncryptionData should be empty
                    if (aEncryptionData.getLength() && aCtx.VerifyKey(m_xWwFib->m_nKey, m_xWwFib->m_nHash))
                    {
                        nErrRet = ERRCODE_NONE;
                        pTempMain = MakeTemp(aDecryptMain);

                        m_pStrm->Seek(0);
                        size_t nUnencryptedHdr =
                            (8 == m_xWwFib->m_nVersion) ? 0x44 : 0x34;
                        sal_uInt8 *pIn = new sal_uInt8[nUnencryptedHdr];
                        nUnencryptedHdr = m_pStrm->ReadBytes(pIn, nUnencryptedHdr);
                        aDecryptMain.WriteBytes(pIn, nUnencryptedHdr);
                        delete [] pIn;

                        DecryptXOR(aCtx, *m_pStrm, aDecryptMain);

                        if (!m_pTableStream || m_pTableStream == m_pStrm)
                            m_pTableStream = &aDecryptMain;
                        else
                        {
                            pTempTable = MakeTemp(aDecryptTable);
                            DecryptXOR(aCtx, *m_pTableStream, aDecryptTable);
                            m_pTableStream = &aDecryptTable;
                        }

                        if (!m_pDataStream || m_pDataStream == m_pStrm)
                            m_pDataStream = &aDecryptMain;
                        else
                        {
                            pTempData = MakeTemp(aDecryptData);
                            DecryptXOR(aCtx, *m_pDataStream, aDecryptData);
                            m_pDataStream = &aDecryptData;
                        }

                        pMedium->GetItemSet()->ClearItem( SID_PASSWORD );
                        pMedium->GetItemSet()->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData ) ) );
                    }
                }
                break;
                case RC4:
                case RC4CryptoAPI:
                {
                    std::unique_ptr<msfilter::MSCodec97> xCtx;
                    msfilter::RC4EncryptionInfo info;
                    bool bCouldReadHeaders;

                    if (eAlgo == RC4)
                    {
                        xCtx.reset(new msfilter::MSCodec_Std97);
                        assert(sizeof(info.verifier.encryptedVerifierHash) >= RTL_DIGEST_LENGTH_MD5);
                        bCouldReadHeaders =
                            checkRead(*m_pTableStream, info.verifier.salt, sizeof(info.verifier.salt)) &&
                            checkRead(*m_pTableStream, info.verifier.encryptedVerifier, sizeof(info.verifier.encryptedVerifier)) &&
                            checkRead(*m_pTableStream, info.verifier.encryptedVerifierHash, RTL_DIGEST_LENGTH_MD5);
                    }
                    else
                    {
                        xCtx.reset(new msfilter::MSCodec_CryptoAPI);
                        bCouldReadHeaders = lclReadCryptoAPIHeader(info, *m_pTableStream);
                    }

                    // if initialization has failed the EncryptionData should be empty
                    uno::Sequence< beans::NamedValue > aEncryptionData;
                    if (bCouldReadHeaders)
                        aEncryptionData = Init97Codec(*xCtx, info.verifier.salt, *pMedium);
                    else
                        nErrRet = ERRCODE_SVX_READ_FILTER_CRYPT;
                    if (aEncryptionData.getLength() && xCtx->VerifyKey(info.verifier.encryptedVerifier,
                                                                       info.verifier.encryptedVerifierHash))
                    {
                        nErrRet = ERRCODE_NONE;

                        pTempMain = MakeTemp(aDecryptMain);

                        m_pStrm->Seek(0);
                        std::size_t nUnencryptedHdr = 0x44;
                        sal_uInt8 *pIn = new sal_uInt8[nUnencryptedHdr];
                        nUnencryptedHdr = m_pStrm->ReadBytes(pIn, nUnencryptedHdr);

                        DecryptRC4(*xCtx, *m_pStrm, aDecryptMain);

                        aDecryptMain.Seek(0);
                        aDecryptMain.WriteBytes(pIn, nUnencryptedHdr);
                        delete [] pIn;

                        pTempTable = MakeTemp(aDecryptTable);
                        DecryptRC4(*xCtx, *m_pTableStream, aDecryptTable);
                        m_pTableStream = &aDecryptTable;

                        if (!m_pDataStream || m_pDataStream == m_pStrm)
                            m_pDataStream = &aDecryptMain;
                        else
                        {
                            pTempData = MakeTemp(aDecryptData);
                            DecryptRC4(*xCtx, *m_pDataStream, aDecryptData);
                            m_pDataStream = &aDecryptData;
                        }

                        pMedium->GetItemSet()->ClearItem( SID_PASSWORD );
                        pMedium->GetItemSet()->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData ) ) );
                    }
                }
                break;
            }
        }

        if (nErrRet == ERRCODE_NONE)
        {
            m_pStrm = &aDecryptMain;

            m_xWwFib.reset(new WW8Fib(*m_pStrm, m_nWantedVersion));
            if (m_xWwFib->m_nFibError)
                nErrRet = ERR_SWG_READ_ERROR;
        }
    }

    if (!nErrRet)
        nErrRet = CoreLoad(pGloss);

    pTempMain.reset();
    pTempTable.reset();
    pTempData.reset();

    m_xWwFib.reset();
    return nErrRet;
}

void SwWW8ImplReader::SetOutlineStyles()
{
    // If we are inserted into a document then don't clobber existing outline
    // levels.
    sal_uInt16 nOutlineStyleListLevelWithAssignment = 0;
    if (!m_bNewDoc)
    {
        ww8::ParaStyles aOutLined(sw::util::GetParaStyles(m_rDoc));
        sw::util::SortByAssignedOutlineStyleListLevel(aOutLined);
        ww8::ParaStyles::reverse_iterator aEnd = aOutLined.rend();
        for ( ww8::ParaStyles::reverse_iterator aIter = aOutLined.rbegin(); aIter < aEnd; ++aIter)
        {
            if ((*aIter)->IsAssignedToListLevelOfOutlineStyle())
                nOutlineStyleListLevelWithAssignment |= 1 << (*aIter)->GetAssignedOutlineStyleLevel();
            else
                break;
        }
    }

    // Check applied WW8 list styles at WW8 Built-In Heading Styles
    // - Choose the list style which occurs most often as the one which provides
    //   the list level properties for the Outline Style.
    // - Populate temporary list of WW8 Built-In Heading Styles for further
    // iteration
    std::vector<SwWW8StyInf*> aWW8BuiltInHeadingStyles;
    const SwNumRule* pChosenWW8ListStyle = nullptr;
    {
        std::map<const SwNumRule*, int> aWW8ListStyleCounts;
        for (SwWW8StyInf & rSI : m_vColl)
        {
            if (!rSI.IsWW8BuiltInHeadingStyle() || !rSI.HasWW8OutlineLevel())
            {
                continue;
            }

            aWW8BuiltInHeadingStyles.push_back(&rSI);

            const SwNumRule* pWW8ListStyle = rSI.GetOutlineNumrule();
            if (pWW8ListStyle != nullptr)
            {
                std::map<const SwNumRule*, int>::iterator aCountIter
                    = aWW8ListStyleCounts.find(pWW8ListStyle);
                if (aCountIter == aWW8ListStyleCounts.end())
                {
                    aWW8ListStyleCounts[pWW8ListStyle] = 1;
                }
                else
                {
                    ++(aCountIter->second);
                }
            }
        }

        int nCurrentMaxCount = 0;
        for (const auto& rEntry : aWW8ListStyleCounts)
        {
            if (rEntry.second > nCurrentMaxCount)
            {
                nCurrentMaxCount = rEntry.second;
                pChosenWW8ListStyle = rEntry.first;
            }
        }
    }

    // - set list level properties of Outline Style - ODF's list style applied
    // by default to headings
    // - assign corresponding Heading Paragraph Styles to the Outline Style
    // - If a heading Paragraph Styles is not applying the WW8 list style which
    // had been chosen as
    //   the one which provides the list level properties for the Outline Style,
    // its assignment to
    //   the Outline Style is removed. A potential applied WW8 list style is
    // assigned directly and
    //   its default outline level is applied.
    SwNumRule aOutlineRule(*m_rDoc.GetOutlineNumRule());
    bool bAppliedChangedOutlineStyle = false;
    for (const SwWW8StyInf* pStyleInf : aWW8BuiltInHeadingStyles)
    {
        if (!pStyleInf->m_bColl) //Character Style
            continue;

        const sal_uInt16 nOutlineStyleListLevelOfWW8BuiltInHeadingStyle
            = 1 << pStyleInf->mnWW8OutlineLevel;
        if (nOutlineStyleListLevelOfWW8BuiltInHeadingStyle
            & nOutlineStyleListLevelWithAssignment)
        {
            continue;
        }

        if (pChosenWW8ListStyle != nullptr && pStyleInf->mnWW8OutlineLevel
                                           == pStyleInf->m_nListLevel)
        {
            const SwNumFormat& rRule
                = pChosenWW8ListStyle->Get(pStyleInf->mnWW8OutlineLevel);
            aOutlineRule.Set(pStyleInf->mnWW8OutlineLevel, rRule);
            bAppliedChangedOutlineStyle = true;
        }

        // in case that there are more styles on this level ignore them
        nOutlineStyleListLevelWithAssignment
            |= nOutlineStyleListLevelOfWW8BuiltInHeadingStyle;

        SwTextFormatColl* pTextFormatColl = static_cast<SwTextFormatColl*>(pStyleInf->m_pFormat);
        if (pStyleInf->GetOutlineNumrule() != pChosenWW8ListStyle
            || (pStyleInf->m_nListLevel < WW8ListManager::nMaxLevel
                && pStyleInf->mnWW8OutlineLevel != pStyleInf->m_nListLevel))
        {
            // WW8 Built-In Heading Style does not apply the chosen one.
            // --> delete assignment to OutlineStyle, but keep its current
            // outline level
            pTextFormatColl->DeleteAssignmentToListLevelOfOutlineStyle();
            // Apply existing WW8 list style a normal list style at the
            // Paragraph Style
            if (pStyleInf->GetOutlineNumrule() != nullptr)
            {
                pTextFormatColl->SetFormatAttr(
                    SwNumRuleItem(pStyleInf->GetOutlineNumrule()->GetName()));
            }
            // apply default outline level of WW8 Built-in Heading Style
            const sal_uInt8 nOutlineLevel
                = SwWW8StyInf::WW8OutlineLevelToOutlinelevel(
                    pStyleInf->mnWW8OutlineLevel);
            pTextFormatColl->SetFormatAttr(
                SfxUInt16Item(RES_PARATR_OUTLINELEVEL, nOutlineLevel));
        }
        else
        {
            pTextFormatColl->AssignToListLevelOfOutlineStyle(
                pStyleInf->mnWW8OutlineLevel);
        }
    }

    if (bAppliedChangedOutlineStyle)
    {
        m_rDoc.SetOutlineNumRule(aOutlineRule);
    }
}

const OUString* SwWW8ImplReader::GetAnnotationAuthor(sal_uInt16 nIdx)
{
    if (!m_pAtnNames && m_xWwFib->m_lcbGrpStAtnOwners)
    {
        // Determine authors: can be found in the TableStream
        m_pAtnNames.reset(new std::vector<OUString>);
        SvStream& rStrm = *m_pTableStream;

        long nOldPos = rStrm.Tell();
        rStrm.Seek( m_xWwFib->m_fcGrpStAtnOwners );

        long nRead = 0, nCount = m_xWwFib->m_lcbGrpStAtnOwners;
        while (nRead < nCount && rStrm.good())
        {
            if( m_bVer67 )
            {
                m_pAtnNames->push_back(read_uInt8_PascalString(rStrm,
                    RTL_TEXTENCODING_MS_1252));
                nRead += m_pAtnNames->rbegin()->getLength() + 1; // Length + sal_uInt8 count
            }
            else
            {
                m_pAtnNames->push_back(read_uInt16_PascalString(rStrm));
                // Unicode: double the length + sal_uInt16 count
                nRead += (m_pAtnNames->rbegin()->getLength() + 1)*2;
            }
        }
        rStrm.Seek( nOldPos );
    }

    const OUString *pRet = nullptr;
    if (m_pAtnNames && nIdx < m_pAtnNames->size())
        pRet = &((*m_pAtnNames)[nIdx]);
    return pRet;
}

void SwWW8ImplReader::GetSmartTagInfo(SwFltRDFMark& rMark)
{
    if (!m_pSmartTagData && m_xWwFib->m_lcbFactoidData)
    {
        m_pSmartTagData.reset(new WW8SmartTagData);
        m_pSmartTagData->Read(*m_pTableStream, m_xWwFib->m_fcFactoidData, m_xWwFib->m_lcbFactoidData);
    }

    if (!m_pSmartTagData)
        return;

    // Check if the handle is a valid smart tag bookmark index.
    size_t nIndex = rMark.GetHandle();
    if (nIndex >= m_pSmartTagData->m_aPropBags.size())
        return;

    // Check if the smart tag bookmark refers to a valid factoid type.
    const MSOPropertyBag& rPropertyBag = m_pSmartTagData->m_aPropBags[rMark.GetHandle()];
    auto& rFactoidTypes = m_pSmartTagData->m_aPropBagStore.m_aFactoidTypes;
    auto itPropertyBag = std::find_if(rFactoidTypes.begin(), rFactoidTypes.end(),
        [&rPropertyBag](const MSOFactoidType& rType) { return rType.m_nId == rPropertyBag.m_nId; });
    if (itPropertyBag == rFactoidTypes.end())
        return;

    // Check if the factoid is an RDF one.
    const MSOFactoidType& rFactoidType = *itPropertyBag;
    if (rFactoidType.m_aUri != "http://www.w3.org/1999/02/22-rdf-syntax-ns#")
        return;

    // Finally put the relevant attributes to the mark.
    std::vector< std::pair<OUString, OUString> > aAttributes;
    for (const MSOProperty& rProperty : rPropertyBag.m_aProperties)
    {
        OUString aKey;
        OUString aValue;
        if (rProperty.m_nKey < m_pSmartTagData->m_aPropBagStore.m_aStringTable.size())
            aKey = m_pSmartTagData->m_aPropBagStore.m_aStringTable[rProperty.m_nKey];
        if (rProperty.m_nValue < m_pSmartTagData->m_aPropBagStore.m_aStringTable.size())
            aValue = m_pSmartTagData->m_aPropBagStore.m_aStringTable[rProperty.m_nValue];
        if (!aKey.isEmpty() && !aValue.isEmpty())
            aAttributes.emplace_back(aKey, aValue);
    }
    rMark.SetAttributes(aAttributes);
}

ErrCode SwWW8ImplReader::LoadDoc(WW8Glossary *pGloss)
{
    ErrCode nErrRet = ERRCODE_NONE;

    {
        static const sal_Char* aNames[ 13 ] = {
            "WinWord/WW", "WinWord/WW8", "WinWord/WWFT",
            "WinWord/WWFLX", "WinWord/WWFLY",
            "WinWord/WWF",
            "WinWord/WWFA0", "WinWord/WWFA1", "WinWord/WWFA2",
            "WinWord/WWFB0", "WinWord/WWFB1", "WinWord/WWFB2",
            "WinWord/RegardHindiDigits"
        };
        sal_uInt64 aVal[ 13 ];

        SwFilterOptions aOpt( 13, aNames, aVal );

        m_nIniFlags = aVal[ 0 ];
        m_nIniFlags1= aVal[ 1 ];
        // Moves Flys by x twips to the right or left
        m_nIniFlyDx = aVal[ 3 ];
        m_nIniFlyDy = aVal[ 4 ];

        m_nFieldFlags = aVal[ 5 ];
        m_nFieldTagAlways[0] = aVal[ 6 ];
        m_nFieldTagAlways[1] = aVal[ 7 ];
        m_nFieldTagAlways[2] = aVal[ 8 ];
        m_nFieldTagBad[0] = aVal[ 9 ];
        m_nFieldTagBad[1] = aVal[ 10 ];
        m_nFieldTagBad[2] = aVal[ 11 ];
        m_bRegardHindiDigits = aVal[ 12 ] > 0;
    }

    sal_uInt16 nMagic(0);
    m_pStrm->ReadUInt16( nMagic );

    // Remember: 6 means "6 OR 7", 7 means "JUST 7"
    switch (m_nWantedVersion)
    {
        case 6:
        case 7:
            if (
                 0xa59b != nMagic && 0xa59c != nMagic &&
                 0xa5dc != nMagic && 0xa5db != nMagic &&
                 (nMagic < 0xa697 || nMagic > 0xa699)
               )
            {
                // Test for own 97 fake!
                if (m_pStg && 0xa5ec == nMagic)
                {
                    sal_uLong nCurPos = m_pStrm->Tell();
                    if (m_pStrm->Seek(nCurPos + 22))
                    {
                        sal_uInt32 nfcMin;
                        m_pStrm->ReadUInt32( nfcMin );
                        if (0x300 != nfcMin)
                            nErrRet = ERR_WW6_NO_WW6_FILE_ERR;
                    }
                    m_pStrm->Seek( nCurPos );
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
            OSL_ENSURE( false, "We forgot to encode nVersion!" );
            break;
    }

    if (!nErrRet)
        nErrRet = LoadThroughDecryption(pGloss);

    m_rDoc.PropagateOutlineRule();

    return nErrRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT Reader* ImportDOC()
{
    return new WW8Reader;
}

namespace
{
    class FontCacheGuard
    {
    public:
        ~FontCacheGuard()
        {
            FlushFontCache();
        }
    };
}

bool TestImportDOC(SvStream &rStream, const OUString &rFltName)
{
    FontCacheGuard aFontCacheGuard;
    std::unique_ptr<Reader> xReader(ImportDOC());

    tools::SvRef<SotStorage> xStorage;
    xReader->m_pStream = &rStream;
    if (rFltName != "WW6")
    {
        try
        {
            xStorage = tools::SvRef<SotStorage>(new SotStorage(rStream));
            if (xStorage->GetError())
                return false;
        }
        catch (...)
        {
            return false;
        }
        xReader->m_pStorage = xStorage.get();
    }
    xReader->SetFltName(rFltName);

    SwGlobals::ensure();

    SfxObjectShellLock xDocSh(new SwDocShell(SfxObjectCreateMode::INTERNAL));
    xDocSh->DoInitNew();
    SwDoc *pD =  static_cast<SwDocShell*>((&xDocSh))->GetDoc();

    SwNodeIndex aIdx(pD->GetNodes().GetEndOfContent(), -1);
    SwPaM aPaM(aIdx);
    aPaM.GetPoint()->nContent.Assign(aIdx.GetNode().GetContentNode(), 0);
    pD->SetInReading(true);
    bool bRet = xReader->Read(*pD, OUString(), aPaM, OUString()) == ERRCODE_NONE;
    pD->SetInReading(false);

    return bRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportWW8(SvStream &rStream)
{
    return TestImportDOC(rStream, "CWW8");
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportWW6(SvStream &rStream)
{
    return TestImportDOC(rStream, "CWW6");
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportWW2(SvStream &rStream)
{
    return TestImportDOC(rStream, "WW6");
}

ErrCode WW8Reader::OpenMainStream( tools::SvRef<SotStorageStream>& rRef, sal_uInt16& rBuffSize )
{
    ErrCode nRet = ERR_SWG_READ_ERROR;
    OSL_ENSURE(m_pStorage, "Where is my Storage?");
    rRef = m_pStorage->OpenSotStream( "WordDocument", StreamMode::READ | StreamMode::SHARE_DENYALL);

    if( rRef.is() )
    {
        if( ERRCODE_NONE == rRef->GetError() )
        {
            sal_uInt16 nOld = rRef->GetBufferSize();
            rRef->SetBufferSize( rBuffSize );
            rBuffSize = nOld;
            nRet = ERRCODE_NONE;
        }
        else
            nRet = rRef->GetError();
    }
    return nRet;
}

ErrCode WW8Reader::Read(SwDoc &rDoc, const OUString& rBaseURL, SwPaM &rPaM, const OUString & /* FileName */)
{
    sal_uInt16 nOldBuffSize = 32768;
    bool bNew = !m_bInsertMode; // New Doc (no inserting)

    tools::SvRef<SotStorageStream> refStrm; // So that no one else can steal the Stream
    SvStream* pIn = m_pStream;

    ErrCode nRet = ERRCODE_NONE;
    sal_uInt8 nVersion = 8;

    const OUString sFltName = GetFltName();
    if ( sFltName=="WW6" )
    {
        if (m_pStream)
            nVersion = 6;
        else
        {
            OSL_ENSURE(false, "WinWord 95 Reader-Read without Stream");
            nRet = ERR_SWG_READ_ERROR;
        }
    }
    else
    {
        if ( sFltName=="CWW6" )
            nVersion = 6;
        else if ( sFltName=="CWW7" )
            nVersion = 7;

        if( m_pStorage.is() )
        {
            nRet = OpenMainStream( refStrm, nOldBuffSize );
            pIn = refStrm.get();
        }
        else
        {
            OSL_ENSURE(false, "WinWord 95/97 Reader-Read without Storage");
            nRet = ERR_SWG_READ_ERROR;
        }
    }

    if( !nRet )
    {
        std::unique_ptr<SwWW8ImplReader> pRdr(new SwWW8ImplReader(nVersion, m_pStorage.get(), pIn, rDoc,
            rBaseURL, bNew, m_bSkipImages, *rPaM.GetPoint()));
        if (bNew)
        {
            // Remove Frame and offsets from Frame Template
            Reader::ResetFrameFormats( rDoc );

            rPaM.GetBound().nContent.Assign(nullptr, 0);
            rPaM.GetBound(false).nContent.Assign(nullptr, 0);

        }
        try
        {
            nRet = pRdr->LoadDoc();
        }
        catch( const std::exception& )
        {
            nRet = ERR_WW8_NO_WW8_FILE_ERR;
        }

        if( refStrm.is() )
        {
            refStrm->SetBufferSize( nOldBuffSize );
            refStrm.clear();
        }
        else
        {
            pIn->ResetError();
        }

    }
    return nRet;
}

SwReaderType WW8Reader::GetReaderType()
{
    return SwReaderType::Storage | SwReaderType::Stream;
}

bool WW8Reader::HasGlossaries() const
{
    return true;
}

bool WW8Reader::ReadGlossaries(SwTextBlocks& rBlocks, bool bSaveRelFiles) const
{
    bool bRet=false;

    WW8Reader *pThis = const_cast<WW8Reader *>(this);

    sal_uInt16 nOldBuffSize = 32768;
    tools::SvRef<SotStorageStream> refStrm;
    if (!pThis->OpenMainStream(refStrm, nOldBuffSize))
    {
        WW8Glossary aGloss( refStrm, 8, m_pStorage.get() );
        bRet = aGloss.Load( rBlocks, bSaveRelFiles );
    }
    return bRet;
}

bool SwMSDffManager::GetOLEStorageName(sal_uInt32 nOLEId, OUString& rStorageName,
    tools::SvRef<SotStorage>& rSrcStorage, uno::Reference < embed::XStorage >& rDestStorage) const
{
    bool bRet = false;

    sal_Int32 nPictureId = 0;
    if (rReader.m_pStg)
    {
        // Via the TextBox-PLCF we get the right char Start-End positions
        // We should then find the EmbeddedField and the corresponding Sprms
        // in that Area.
        // We only need the Sprm for the Picture Id.
        long nOldPos = rReader.m_pStrm->Tell();
        {
            // #i32596# - consider return value of method
            // <rReader.GetTxbxTextSttEndCp(..)>. If it returns false, method
            // wasn't successful. Thus, continue in this case.
            // Note: Ask MM for initialization of <nStartCp> and <nEndCp>.
            // Note: Ask MM about assertions in method <rReader.GetTxbxTextSttEndCp(..)>.
            WW8_CP nStartCp, nEndCp;
            if ( rReader.m_bDrawCpOValid && rReader.GetTxbxTextSttEndCp(nStartCp, nEndCp,
                            static_cast<sal_uInt16>((nOLEId >> 16) & 0xFFFF),
                            static_cast<sal_uInt16>(nOLEId & 0xFFFF)) )
            {
                WW8PLCFxSaveAll aSave;
                rReader.m_xPlcxMan->SaveAllPLCFx( aSave );

                nStartCp += rReader.m_nDrawCpO;
                nEndCp   += rReader.m_nDrawCpO;
                WW8PLCFx_Cp_FKP* pChp = rReader.m_xPlcxMan->GetChpPLCF();
                wwSprmParser aSprmParser(*rReader.m_xWwFib);
                while (nStartCp <= nEndCp && !nPictureId)
                {
                    if (!pChp->SeekPos( nStartCp))
                        break;
                    WW8PLCFxDesc aDesc;
                    pChp->GetSprms( &aDesc );

                    if (aDesc.nSprmsLen && aDesc.pMemPos) // Attributes present
                    {
                        long nLen = aDesc.nSprmsLen;
                        const sal_uInt8* pSprm = aDesc.pMemPos;

                        while (nLen >= 2 && !nPictureId)
                        {
                            sal_uInt16 nId = aSprmParser.GetSprmId(pSprm);
                            sal_uInt16 nSL = aSprmParser.GetSprmSize(nId, pSprm, nLen);

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

                rReader.m_xPlcxMan->RestoreAllPLCFx( aSave );
            }
        }
        rReader.m_pStrm->Seek( nOldPos );
    }

    if( bRet )
    {
        rStorageName = "_";
        rStorageName += OUString::number(nPictureId);
        rSrcStorage = rReader.m_pStg->OpenSotStorage(SL::aObjectPool);
        if (!rReader.m_pDocShell)
            bRet=false;
        else
            rDestStorage = rReader.m_pDocShell->GetStorage();
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
bool SwMSDffManager::ShapeHasText(sal_uLong, sal_uLong) const
{
    return true;
}

bool SwWW8ImplReader::InEqualOrHigherApo(int nLvl) const
{
    if (nLvl)
        --nLvl;
    // #i60827# - check size of <maApos> to assure that <maApos.begin() + nLvl> can be performed.
    if ( sal::static_int_cast< sal_Int32>(nLvl) >= sal::static_int_cast< sal_Int32>(m_aApos.size()) )
    {
        return false;
    }
    auto aIter = std::find(m_aApos.begin() + nLvl, m_aApos.end(), true);
    return aIter != m_aApos.end();
}

bool SwWW8ImplReader::InEqualApo(int nLvl) const
{
    // If we are in a table, see if an apo was inserted at the level below the table.
    if (nLvl)
        --nLvl;
    if (nLvl < 0 || static_cast<size_t>(nLvl) >= m_aApos.size())
        return false;
    return m_aApos[nLvl];
}

namespace sw
{
    namespace hack
    {
        Position::Position(const SwPosition &rPos)
            : maPtNode(rPos.nNode), mnPtContent(rPos.nContent.GetIndex())
        {
        }

        Position::Position(const Position &rPos)
            : maPtNode(rPos.maPtNode), mnPtContent(rPos.mnPtContent)
        {
        }

        Position::operator SwPosition() const
        {
            SwPosition aRet(maPtNode);
            aRet.nContent.Assign(maPtNode.GetNode().GetContentNode(), mnPtContent);
            return aRet;
        }
    }
}

SwMacroInfo::SwMacroInfo()
    : SdrObjUserData( SdrInventor::ScOrSwDraw, SW_UD_IMAPDATA )
    , mnShapeId(-1)
{
}

SwMacroInfo::~SwMacroInfo()
{
}

std::unique_ptr<SdrObjUserData> SwMacroInfo::Clone( SdrObject* /*pObj*/ ) const
{
   return std::unique_ptr<SdrObjUserData>(new SwMacroInfo( *this ));
}

std::unique_ptr<SfxItemSet> SwWW8ImplReader::SetCurrentItemSet(std::unique_ptr<SfxItemSet> pItemSet)
{
    std::unique_ptr<SfxItemSet> xRet(std::move(m_xCurrentItemSet));
    m_xCurrentItemSet = std::move(pItemSet);
    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
