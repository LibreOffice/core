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

#include <memory>
#include <com/sun/star/embed/Aspects.hpp>

#include <hintids.hxx>

#include <officecfg/Office/Common.hxx>
#include <o3tl/any.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <vcl/svapp.hxx>
#include <sot/storage.hxx>
#include <svl/itemiter.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpage.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <svx/svdouno.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdview.hxx>
#include <svx/unopage.hxx>
#include <fmtcnct.hxx>
#include <fmtanchr.hxx>
#include <fmtsrnd.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtfollowtextflow.hxx>
#include <frmfmt.hxx>
#include <fmtcntnt.hxx>
#include <ndindex.hxx>
#include <doc.hxx>
#include <drawdoc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <pam.hxx>
#include <swrect.hxx>
#include <ndgrf.hxx>
#include <grfatr.hxx>
#include <ndole.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include "ww8par.hxx"
#include <breakit.hxx>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "attributeoutputbase.hxx"
#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include "wrtww8.hxx"
#include "escher.hxx"
#include <ndtxt.hxx>
#include "WW8FFData.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <IDocumentStylePoolAccess.hxx>
#include <oox/ole/olehelper.hxx>
#include <fmturl.hxx>
#include <frameformats.hxx>
#include <sfx2/sfxsids.hrc>
#include <unotools/saveopt.hxx>
#include <o3tl/enumrange.hxx>
#include <o3tl/enumarray.hxx>
#include <sfx2/docfile.hxx>
#include <tools/UnitConversion.hxx>

#include <algorithm>

using ::editeng::SvxBorderLine;
using namespace com::sun::star;
using namespace sw::util;
using namespace sw::types;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::drawing::XShape;

bool SwBasicEscherEx::IsRelUrl() const
{
    bool bRelUrl = false;
    SfxMedium * pMedium = rWrt.GetWriter().GetMedia();
    if ( pMedium )
        bRelUrl = pMedium->IsRemote()
            ? officecfg::Office::Common::Save::URL::Internet::get()
            : officecfg::Office::Common::Save::URL::FileSystem::get();
    return bRelUrl;
}

OUString SwBasicEscherEx::GetBasePath() const
{
    OUString sDocUrl;
    SfxMedium * pMedium = rWrt.GetWriter().GetMedia();
    if (pMedium)
    {
        const SfxItemSet* pPItemSet = pMedium->GetItemSet();
        if( pPItemSet )
        {
            const SfxStringItem* pPItem = dynamic_cast< const SfxStringItem* >( pPItemSet->GetItem( SID_FILE_NAME ) );
            if ( pPItem )
                sDocUrl = pPItem->GetValue();
        }
    }

    return sDocUrl.copy(0, sDocUrl.lastIndexOf('/') + 1);
}

OUString SwBasicEscherEx::BuildFileName(sal_uInt16& rnLevel, bool& rbRel, const OUString& rUrl)
{
    OUString aDosName( INetURLObject( rUrl ).getFSysPath( FSysStyle::Dos ) );
    rnLevel = 0;
    rbRel = IsRelUrl();

    if (rbRel)
    {
        // try to convert to relative file name
        OUString aTmpName( aDosName );
        aDosName = INetURLObject::GetRelURL( GetBasePath(), rUrl,
        INetURLObject::EncodeMechanism::WasEncoded, INetURLObject::DecodeMechanism::WithCharset );

        if (aDosName.startsWith(INET_FILE_SCHEME))
        {
            // not converted to rel -> back to old, return absolute flag
            aDosName = aTmpName;
            rbRel = false;
        }
        else if (aDosName.startsWith("./"))
        {
            aDosName = aDosName.copy(2);
        }
        else
        {
            while (aDosName.startsWith("../"))
            {
                ++rnLevel;
                aDosName = aDosName.copy(3);
            }
        }
    }
    return aDosName;
}

void SwBasicEscherEx::WriteHyperlinkWithinFly( SvMemoryStream& rStrm, const SwFormatURL* pINetFormatArg)
{
    if ( !pINetFormatArg ) return;

    const sal_uInt8 aGuidStdLink[ 16 ] ={
        0xD0, 0xC9, 0xEA, 0x79, 0xF9, 0xBA, 0xCE, 0x11, 0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9, 0x0B };
    const sal_uInt8 aGuidUrlMoniker[ 16 ] = {
        0xE0, 0xC9, 0xEA, 0x79, 0xF9, 0xBA, 0xCE, 0x11, 0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9, 0x0B };

    const sal_uInt8 aGuidFileMoniker[ 16 ] = {
        0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 };
    const sal_uInt8 aGuidFileTail[] = {
            0xFF, 0xFF, 0xAD, 0xDE, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
    //const sal_uInt18 WW8_ID_HLINK               = 0x01B8;
    constexpr sal_uInt32 WW8_HLINK_BODY             = 0x00000001;   /// Contains file link or URL.
    constexpr sal_uInt32 WW8_HLINK_ABS              = 0x00000002;   /// Absolute path.
    //const sal_uInt32 WW8_HLINK_DESCR            = 0x00000014;   /// Description.
    constexpr sal_uInt32 WW8_HLINK_MARK             = 0x00000008;   /// Text mark.
    constexpr sal_uInt32 WW8_HLINK_FRAME            = 0x00000080;   /// Target frame.
    //const sal_uInt32 WW8_HLINK_UNC              = 0x00000100;   /// UNC path.
    SvMemoryStream tmpStrm;
    OUString tmpTextMark;

    OUString rUrl = pINetFormatArg->GetURL();
    OUString rTarFrame = pINetFormatArg->GetTargetFrameName();
    sal_uInt32 nFlags = 0;

    INetURLObject aUrlObj( rUrl );
    const INetProtocol eProtocol = aUrlObj.GetProtocol();

    //Target Frame
    if (!rTarFrame.isEmpty())
    {
        SwWW8Writer::WriteLong(tmpStrm, rTarFrame.getLength()+1);
        SwWW8Writer::WriteString16(tmpStrm, rTarFrame, false);

        tmpStrm.WriteUInt16( 0 );

        nFlags |= WW8_HLINK_FRAME;
    }

    // file link or URL
    if (eProtocol == INetProtocol::File || (eProtocol == INetProtocol::NotValid && rUrl[0] != '#'))
    {
        sal_uInt16 nLevel;
        bool bRel;
        OUString aFileName( BuildFileName( nLevel, bRel, rUrl ));

        if( !bRel )
            nFlags |= WW8_HLINK_ABS;

        nFlags |= WW8_HLINK_BODY;

        tmpStrm.WriteBytes(aGuidFileMoniker, sizeof(aGuidFileMoniker));
        tmpStrm.WriteUInt16( nLevel );
        SwWW8Writer::WriteLong(tmpStrm, aFileName.getLength()+1);
        SwWW8Writer::WriteString8( tmpStrm, aFileName, true, RTL_TEXTENCODING_MS_1252 );
        tmpStrm.WriteBytes(aGuidFileTail, sizeof(aGuidFileTail));

        //For UNICODE
        SwWW8Writer::WriteLong(tmpStrm, 2*aFileName.getLength()+6);
        SwWW8Writer::WriteLong(tmpStrm, 2*aFileName.getLength());
        tmpStrm.WriteUInt16( 0x0003 );
        SwWW8Writer::WriteString16(tmpStrm, aFileName, false);
    }
    else if( eProtocol != INetProtocol::NotValid )
    {
        tmpStrm.WriteBytes(aGuidUrlMoniker, sizeof(aGuidUrlMoniker));
        SwWW8Writer::WriteLong(tmpStrm, 2*(rUrl.getLength()+1));

        SwWW8Writer::WriteString16(tmpStrm, rUrl, true);
        nFlags |= WW8_HLINK_BODY | WW8_HLINK_ABS;
    }
    else if (rUrl[0] == '#' )
    {
        OUString aTextMark(rUrl.copy( 1 ));
        aTextMark = aTextMark.replaceFirst(".", "!");
        tmpTextMark = aTextMark;
    }

    if (tmpTextMark.isEmpty() && aUrlObj.HasMark())
    {
        tmpTextMark = aUrlObj.GetMark();
    }

    if (!tmpTextMark.isEmpty())
    {
        SwWW8Writer::WriteLong(tmpStrm, tmpTextMark.getLength()+1);
        SwWW8Writer::WriteString16(tmpStrm, tmpTextMark, true);

        nFlags |= WW8_HLINK_MARK;
    }

    rStrm.WriteBytes(aGuidStdLink, 16);
    rStrm .WriteUInt32( 2 )
          .WriteUInt32( nFlags );
    tmpStrm.Seek( STREAM_SEEK_TO_BEGIN );
    sal_uInt32 const nLen = tmpStrm.remainingSize();
    if(nLen >0)
    {
        std::unique_ptr<sal_uInt8[]> pBuffer( new sal_uInt8[ nLen ] );
        tmpStrm.ReadBytes(pBuffer.get(), nLen);
        rStrm.WriteBytes(pBuffer.get(), nLen);
    }
}
void SwBasicEscherEx::PreWriteHyperlinkWithinFly(const SwFrameFormat& rFormat,EscherPropertyContainer& rPropOpt)
{
    const SfxPoolItem* pItem;
    const SwAttrSet& rAttrSet = rFormat.GetAttrSet();
    if (SfxItemState::SET != rAttrSet.GetItemState(RES_URL, true, &pItem))
        return;

    const SwFormatURL *pINetFormat = dynamic_cast<const SwFormatURL*>(pItem);
    if (!pINetFormat || pINetFormat->GetURL().isEmpty())
        return;

    SvMemoryStream aStrm;
    WriteHyperlinkWithinFly( aStrm, pINetFormat );
    rPropOpt.AddOpt(ESCHER_Prop_pihlShape, true, 0, aStrm);
    sal_uInt32 nValue;
    OUString aNamestr = pINetFormat->GetName();
    if (!aNamestr.isEmpty())
    {
        rPropOpt.AddOpt(ESCHER_Prop_wzName, aNamestr );
    }
    if(rPropOpt.GetOpt( ESCHER_Prop_fPrint, nValue))
    {
        nValue|=0x03080008;
        rPropOpt.AddOpt(ESCHER_Prop_fPrint, nValue );
    }
    else
        rPropOpt.AddOpt(ESCHER_Prop_fPrint, 0x03080008 );
}

namespace
{
    /// Get the Z ordering number for a DrawObj in a WW8Export.
    /// @param rWrt The containing WW8Export.
    /// @param pObj pointer to the drawing object.
    /// @returns The ordering number.
    sal_uLong lcl_getSdrOrderNumber(const WW8Export& rWrt, DrawObj const *pObj)
    {
        return rWrt.GetSdrOrdNum(pObj->maContent.GetFrameFormat());
    };

    /// A function object to act as a predicate comparing the ordering numbers
    /// of two drawing objects in a WW8Export.
    class CompareDrawObjs
    {
    private:
        const WW8Export& wrt;

    public:
        explicit CompareDrawObjs(const WW8Export& rWrt) : wrt(rWrt) {};
        bool operator()(DrawObj const *a, DrawObj const *b) const
        {
            sal_uLong aSort = lcl_getSdrOrderNumber(wrt, a);
            sal_uLong bSort = lcl_getSdrOrderNumber(wrt, b);
            return aSort < bSort;
        }
    };

    /// Make a z-order sorted copy of a collection of DrawObj objects.
    /// @param rWrt    The containing WW8Export.
    /// @param rSrcArr The source array.
    /// @param rDstArr The destination array.
    void lcl_makeZOrderArray(const WW8Export& rWrt,
                                    std::vector<DrawObj> &rSrcArr,
                                    std::vector<DrawObj*> &rDstArr)
    {
        rDstArr.clear();
        rDstArr.reserve(rSrcArr.size());
        for(DrawObj & i : rSrcArr)
        {
            rDstArr.push_back( &i );
        }
        std::sort(rDstArr.begin(), rDstArr.end(), CompareDrawObjs(rWrt));
    }

}

// get a part fix for this type of element
bool WW8Export::MiserableFormFieldExportHack(const SwFrameFormat& rFrameFormat)
{
    const SdrObject *pObject = rFrameFormat.FindRealSdrObject();
    if (!pObject || pObject->GetObjInventor() != SdrInventor::FmForm)
        return false;

    const SdrUnoObj *pFormObj = dynamic_cast< const SdrUnoObj* >(pObject);
    if (!pFormObj)
        return false;

    uno::Reference< awt::XControlModel > xControlModel =
        pFormObj->GetUnoControlModel();
    uno::Reference< lang::XServiceInfo > xInfo(xControlModel,
        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xControlModel, uno::UNO_QUERY);
    if (!xInfo.is())
        return false;

    if (xInfo->supportsService("com.sun.star.form.component.ComboBox"))
    {
        DoComboBox(xPropSet);
        return true;
    }

    return false;
}

void WW8Export::DoComboBox(uno::Reference<beans::XPropertySet> const & xPropSet)
{
    OUString sSelected;
    uno::Sequence<OUString> aListItems;
    xPropSet->getPropertyValue("StringItemList") >>= aListItems;
    if (aListItems.hasElements())
    {
        uno::Any aTmp = xPropSet->getPropertyValue("DefaultText");
        auto pStr = o3tl::tryAccess<OUString>(aTmp);
        if (pStr)
            sSelected = *pStr;
    }

    OUString sName;
    {
        uno::Any aTmp = xPropSet->getPropertyValue("Name");
        auto pStr = o3tl::tryAccess<OUString>(aTmp);
        if (pStr)
            sName = *pStr;
    }

    OUString sHelp;
    {
        // property "Help" does not exist and due to the no-existence an exception is thrown.
        try
        {
            uno::Any aTmp = xPropSet->getPropertyValue("HelpText");
            auto pStr = o3tl::tryAccess<OUString>(aTmp);
            if (pStr)
                sHelp = *pStr;
        }
        catch( const uno::Exception& )
        {}
    }

    OUString sToolTip;
    {
        uno::Any aTmp = xPropSet->getPropertyValue("Name");
        auto pStr = o3tl::tryAccess<OUString>(aTmp);
        if (pStr)
            sToolTip = *pStr;
    }

    DoComboBox(sName, sHelp, sToolTip, sSelected, aListItems);
}

void WW8Export::DoComboBox(const OUString &rName,
                             const OUString &rHelp,
                             const OUString &rToolTip,
                             const OUString &rSelected,
                             const uno::Sequence<OUString> &rListItems)
{
    OutputField(nullptr, ww::eFORMDROPDOWN, FieldString(ww::eFORMDROPDOWN),
             FieldFlags::Start | FieldFlags::CmdStart);
    // write the reference to the "picture" structure
    sal_uInt64 nDataStt = pDataStrm->Tell();
    m_pChpPlc->AppendFkpEntry( Strm().Tell() );

    WriteChar( 0x01 );

    static sal_uInt8 aArr1[] =
    {
        0x03, 0x6a, 0,0,0,0,    // sprmCPicLocation
        0x06, 0x08, 0x01,       // sprmCFData
        0x55, 0x08, 0x01,       // sprmCFSpec
        0x02, 0x08, 0x01        // sprmCFFieldVanish
    };
    sal_uInt8* pDataAdr = aArr1 + 2;
    Set_UInt32( pDataAdr, nDataStt );

    m_pChpPlc->AppendFkpEntry(Strm().Tell(), sizeof(aArr1), aArr1);

    OutputField(nullptr, ww::eFORMDROPDOWN, FieldString(ww::eFORMDROPDOWN),
             FieldFlags::Close);

    ::sw::WW8FFData aFFData;

    aFFData.setType(2);
    aFFData.setName(rName);
    aFFData.setHelp(rHelp);
    aFFData.setStatus(rToolTip);

    sal_uInt32 nListItems = rListItems.getLength();

    for (sal_uInt32 i = 0; i < nListItems; i++)
    {
        if (i < 0x20 && rSelected == rListItems[i])
            aFFData.setResult(::sal::static_int_cast<sal_uInt8>(i));
        aFFData.addListboxEntry(rListItems[i]);
    }

    aFFData.Write(pDataStrm);
}

void WW8Export::DoFormText(const SwInputField * pField)
{
    OutputField(nullptr, ww::eFORMTEXT, FieldString(ww::eFORMTEXT),
        FieldFlags::Start | FieldFlags::CmdStart);
    // write the reference to the "picture" structure
    sal_uInt64 nDataStt = pDataStrm->Tell();
    m_pChpPlc->AppendFkpEntry( Strm().Tell() );

    WriteChar( 0x01 );
    static sal_uInt8 aArr1[] = {
        0x02, 0x08, 0x81,        // sprmCFFieldVanish
        0x03, 0x6a, 0,0,0,0,    // sprmCPicLocation

        0x06, 0x08, 0x01,       // sprmCFData
        0x55, 0x08, 0x01       // sprmCFSpec
    };
    sal_uInt8* pDataAdr = aArr1 + 5;
    Set_UInt32( pDataAdr, nDataStt );

    m_pChpPlc->AppendFkpEntry(Strm().Tell(),
                sizeof( aArr1 ), aArr1 );

    ::sw::WW8FFData aFFData;

    aFFData.setType(0);
    aFFData.setName(pField->GetPar2());
    aFFData.setHelp(pField->GetHelp());
    aFFData.setStatus(pField->GetToolTip());
    aFFData.Write(pDataStrm);

    OutputField(nullptr, ww::eFORMTEXT, OUString(), FieldFlags::CmdEnd);

    const OUString fieldStr( pField->ExpandField(true, nullptr) );
    SwWW8Writer::WriteString16(Strm(), fieldStr, false);

    static sal_uInt8 aArr2[] = {
        0x55, 0x08, 0x01,  // sprmCFSpec
        0x75, 0x08, 0x01       // ???
    };

    pDataAdr = aArr2 + 2;
    Set_UInt32( pDataAdr, nDataStt );
    m_pChpPlc->AppendFkpEntry(Strm().Tell(),
                sizeof( aArr2 ), aArr2 );

    OutputField(nullptr, ww::eFORMTEXT, OUString(), FieldFlags::Close);
}

PlcDrawObj::~PlcDrawObj()
{
}

//It's irritating to have to change the RTL frames position into LTR ones
//so that word will have to place them in the right place. Doubly so that
//the SO drawings and writer frames have different ideas themselves as to
//how to be positioned when in RTL mode!
bool RTLGraphicsHack(SwTwips &rLeft, SwTwips nWidth,
sal_Int16 eHoriOri, sal_Int16 eHoriRel, SwTwips nPageLeft,
    SwTwips nPageRight, SwTwips nPageSize)
{
    bool bRet = false;
    if (eHoriOri == text::HoriOrientation::NONE)
    {
        if (eHoriRel == text::RelOrientation::PAGE_FRAME)
        {
            rLeft = nPageSize - rLeft;
            bRet = true;
        }
        else if (
                  (eHoriRel == text::RelOrientation::PAGE_PRINT_AREA) ||
                  (eHoriRel == text::RelOrientation::FRAME) ||
                  (eHoriRel == text::RelOrientation::PRINT_AREA)
                )
        {
            rLeft = nPageSize - nPageLeft - nPageRight - rLeft;
            bRet = true;
        }
    }
    if (bRet)
        rLeft -= nWidth;
    return bRet;
}

static bool RTLDrawingsHack(SwTwips &rLeft,
    sal_Int16 eHoriOri, sal_Int16 eHoriRel, SwTwips nPageLeft,
    SwTwips nPageRight, SwTwips nPageSize)
{
    bool bRet = false;
    if (eHoriOri == text::HoriOrientation::NONE)
    {
        if (eHoriRel == text::RelOrientation::PAGE_FRAME)
        {
            rLeft = nPageSize + rLeft;
            bRet = true;
        }
        else if (
                  (eHoriRel == text::RelOrientation::PAGE_PRINT_AREA) ||
                  (eHoriRel == text::RelOrientation::FRAME) ||
                  (eHoriRel == text::RelOrientation::PRINT_AREA)
                )
        {
            rLeft = nPageSize - nPageLeft - nPageRight + rLeft;
            bRet = true;
        }
    }
    return bRet;
}

void WW8Export::MiserableRTLFrameFormatHack(SwTwips &rLeft, SwTwips &rRight,
    const ww8::Frame &rFrameFormat)
{
    //Require nasty bidi swap
    if (SvxFrameDirection::Horizontal_RL_TB != m_rDoc.GetTextDirection(rFrameFormat.GetPosition()))
        return;

    SwTwips nWidth = rRight - rLeft;
    SwTwips nPageLeft, nPageRight;
    SwTwips nPageSize = CurrentPageWidth(nPageLeft, nPageRight);

    const SwFormatHoriOrient& rHOr = rFrameFormat.GetFrameFormat().GetHoriOrient();

    bool bRet = false;
    ww8::Frame::WriterSource eSource = rFrameFormat.GetWriterType();
    if (eSource == ww8::Frame::eDrawing || eSource == ww8::Frame::eFormControl)
    {
        if (RTLDrawingsHack(rLeft, rHOr.GetHoriOrient(),
            rHOr.GetRelationOrient(), nPageLeft, nPageRight, nPageSize))
        {
            bRet = true;
        }
    }
    else
    {
        if (RTLGraphicsHack(rLeft, nWidth, rHOr.GetHoriOrient(),
            rHOr.GetRelationOrient(), nPageLeft, nPageRight, nPageSize))
        {
            bRet = true;
        }
    }
    if (bRet)
        rRight = rLeft + nWidth;
}

void PlcDrawObj::WritePlc( WW8Export& rWrt ) const
{
    if (8 > rWrt.pFib->m_nVersion)    // Cannot export drawobject in vers 7-
        return;

    sal_uInt32 nFcStart = rWrt.pTableStrm->Tell();

    if (maDrawObjs.empty())
        return;

    // write CPs
    WW8Fib& rFib = *rWrt.pFib;
    WW8_CP nCpOffs = GetCpOffset(rFib);

    for (const auto& rDrawObj : maDrawObjs)
        SwWW8Writer::WriteLong(*rWrt.pTableStrm, rDrawObj.mnCp - nCpOffs);

    SwWW8Writer::WriteLong(*rWrt.pTableStrm, rFib.m_ccpText + rFib.m_ccpFootnote +
        rFib.m_ccpHdr + rFib.m_ccpEdn + rFib.m_ccpTxbx + rFib.m_ccpHdrTxbx + 1);

    for (const auto& rDrawObj : maDrawObjs)
    {
        // write the fspa-struct
        const ww8::Frame &rFrameFormat = rDrawObj.maContent;
        const SwFrameFormat &rFormat = rFrameFormat.GetFrameFormat();
        const SdrObject* pObj = rFormat.FindRealSdrObject();

        tools::Rectangle aRect;
        SwFormatVertOrient rVOr = rFormat.GetVertOrient();
        SwFormatHoriOrient rHOr = rFormat.GetHoriOrient();
        // #i30669# - convert the positioning attributes.
        // Most positions are converted, if layout information exists.
        const bool bPosConverted =
            WinwordAnchoring::ConvertPosition( rHOr, rVOr, rFormat );

        Point aObjPos;
        bool bHasHeightWidthSwapped(false);
        if (RES_FLYFRMFMT == rFormat.Which())
        {
            SwRect aLayRect(rFormat.FindLayoutRect(false, &aObjPos));
            // the Object is not visible - so get the values from
            // the format. The Position may not be correct.
            if( aLayRect.IsEmpty() )
                aRect.SetSize( rFormat.GetFrameSize().GetSize() );
            else
            {
                // #i56090# Do not only consider the first client
                // Note that we actually would have to find the maximum size of the
                // frame format clients. However, this already should work in most cases.
                const SwRect aSizeRect(rFormat.FindLayoutRect());
                if ( aSizeRect.Width() > aLayRect.Width() )
                    aLayRect.Width( aSizeRect.Width() );

                aRect = aLayRect.SVRect();
            }
        }
        else
        {
            OSL_ENSURE(pObj, "Where is the SDR-Object?");
            if (pObj)
            {
                aRect = pObj->GetLogicRect();

                // rotating to vertical means swapping height and width as seen in SvxMSDffManager::ImportShape
                const Degree100 nAngle = NormAngle36000( pObj->GetRotateAngle() );
                const bool bAllowSwap = pObj->GetObjIdentifier() != OBJ_LINE && pObj->GetObjIdentifier() != OBJ_GRUP;
                if ( bAllowSwap && (( nAngle > 4500_deg100 && nAngle <= 13500_deg100 ) || ( nAngle > 22500_deg100 && nAngle <= 31500_deg100 )) )
                {
                    const tools::Long nWidth  = aRect.getWidth();
                    const tools::Long nHeight = aRect.getHeight();
                    aRect.setWidth( nHeight );
                    aRect.setHeight( nWidth );
                    bHasHeightWidthSwapped = true;
                }
            }
        }

        // #i30669# - use converted position, if conversion is performed.
        // Unify position determination of Writer fly frames
        // and drawing objects.
        if ( bPosConverted )
        {
            aRect.SetPos( Point( rHOr.GetPos(), rVOr.GetPos() ) );
        }
        else
        {
            aRect -= rDrawObj.maParentPos;
            aObjPos = aRect.TopLeft();
            if (text::VertOrientation::NONE == rVOr.GetVertOrient())
            {
                // #i22673#
                sal_Int16 eOri = rVOr.GetRelationOrient();
                if (eOri == text::RelOrientation::CHAR || eOri == text::RelOrientation::TEXT_LINE)
                    aObjPos.setY( -rVOr.GetPos() );
                else
                    aObjPos.setY( rVOr.GetPos() );
            }
            if (text::HoriOrientation::NONE == rHOr.GetHoriOrient())
                aObjPos.setX( rHOr.GetPos() );
            aRect.SetPos( aObjPos );
        }

        sal_Int32 nThick = rDrawObj.mnThick;

        //If we are being exported as an inline hack, set
        //corner to 0 and forget about border thickness for positioning
        if (rFrameFormat.IsInline())
        {
            aRect.SetPos(Point(0,0));
            nThick = 0;
        }

        // spid
        SwWW8Writer::WriteLong(*rWrt.pTableStrm, rDrawObj.mnShapeId);

        SwTwips nLeft = aRect.Left() + nThick;
        SwTwips nRight = aRect.Right() - nThick;
        SwTwips nTop = aRect.Top() + nThick;
        SwTwips nBottom = aRect.Bottom() - nThick;

        // tdf#93675, 0 below line/paragraph and/or top line/paragraph with
        // wrap top+bottom or other wraps is affecting the line directly
        // above the anchor line, which seems odd, but a tiny adjustment
        // here to bring the top down convinces msoffice to wrap like us
        if (nTop == 0 && !rFrameFormat.IsInline() &&
            rVOr.GetVertOrient() == text::VertOrientation::NONE &&
            rVOr.GetRelationOrient() == text::RelOrientation::FRAME)
        {
            nTop = 8;
        }

        //Nasty swap for bidi if necessary
        rWrt.MiserableRTLFrameFormatHack(nLeft, nRight, rFrameFormat);

        // tdf#70838. Word relates the position to the unrotated rectangle,
        // Writer to the rotated one. Because the rotation is around center,
        // the difference counts half.
        if(pObj && pObj->GetRotateAngle())
        {
            SwTwips nXOff;
            SwTwips nYOff;
            SwTwips nSnapWidth = pObj->GetSnapRect().getWidth();
            SwTwips nSnapHeight = pObj->GetSnapRect().getHeight();
            SwTwips nLogicWidth = pObj->GetLogicRect().getWidth();
            SwTwips nLogicHeight = pObj->GetLogicRect().getHeight();
            // +1 for to compensate integer arithmetic rounding errors
            if(bHasHeightWidthSwapped)
            {
                nXOff = (nSnapWidth - nLogicHeight + 1) / 2;
                nYOff = (nSnapHeight - nLogicWidth + 1) / 2;
            }
            else
            {
                nXOff = (nSnapWidth - nLogicWidth + 1) / 2;
                nYOff = (nSnapHeight - nLogicHeight + 1) / 2;
            }
            nLeft += nXOff;
            nRight += nXOff;
            nTop += nYOff;
            nBottom += nYOff;
        }

        //xaLeft/yaTop/xaRight/yaBottom - rel. to anchor
        //(most of) the border is outside the graphic is word, so
        //change dimensions to fit
        SwWW8Writer::WriteLong(*rWrt.pTableStrm, nLeft);
        SwWW8Writer::WriteLong(*rWrt.pTableStrm, nTop);
        SwWW8Writer::WriteLong(*rWrt.pTableStrm, nRight);
        SwWW8Writer::WriteLong(*rWrt.pTableStrm, nBottom);

        //fHdr/bx/by/wr/wrk/fRcaSimple/fBelowText/fAnchorLock
        sal_uInt16 nFlags=0;
        //If nFlags isn't 0x14 its overridden by the escher properties
        if (RndStdIds::FLY_AT_PAGE == rFormat.GetAnchor().GetAnchorId())
            nFlags = 0x0000;
        else
            nFlags = 0x0014;        // x-rel to text,  y-rel to text

        const SwFormatSurround& rSurr = rFormat.GetSurround();
        sal_uInt16 nContour = rSurr.IsContour() ? 0x0080 : 0x0040;
        css::text::WrapTextMode eSurround = rSurr.GetSurround();

        /*
         #i3958#
         The inline elements being export as anchored to character inside
         the shape field hack are required to be wrap through so as to flow
         over the following dummy 0x01 graphic
        */
        if (rFrameFormat.IsInline())
            eSurround = css::text::WrapTextMode_THROUGH;

        switch (eSurround)
        {
            case css::text::WrapTextMode_NONE:
                nFlags |= 0x0020;
                break;
            case css::text::WrapTextMode_THROUGH:
                nFlags |= 0x0060;
                break;
            case css::text::WrapTextMode_PARALLEL:
                nFlags |= 0x0000 | nContour;
                break;
            case css::text::WrapTextMode_DYNAMIC:
                nFlags |= 0x0600 | nContour;
                break;
            case css::text::WrapTextMode_LEFT:
                nFlags |= 0x0200 | nContour;
                break;
            case css::text::WrapTextMode_RIGHT:
                nFlags |= 0x0400 | nContour;
                break;
            default:
                OSL_ENSURE(false, "Unsupported surround type for export");
                break;
        }
        if (pObj && (pObj->GetLayer() == rWrt.m_rDoc.getIDocumentDrawModelAccess().GetHellId() ||
                pObj->GetLayer() == rWrt.m_rDoc.getIDocumentDrawModelAccess().GetInvisibleHellId()))
        {
            nFlags |= 0x4000;
        }

        /*
         #i3958# Required to make this inline stuff work in WordXP, not
         needed for 2003 interestingly
         */
        if (rFrameFormat.IsInline())
            nFlags |= 0x8000;

        SwWW8Writer::WriteShort(*rWrt.pTableStrm, nFlags);

        // cTxbx
        SwWW8Writer::WriteLong(*rWrt.pTableStrm, 0);
    }

    RegisterWithFib(rFib, nFcStart, rWrt.pTableStrm->Tell() - nFcStart);
}

void MainTextPlcDrawObj::RegisterWithFib(WW8Fib &rFib, sal_uInt32 nStart,
    sal_uInt32 nLen) const
{
    rFib.m_fcPlcfspaMom = nStart;
    rFib.m_lcbPlcfspaMom = nLen;
}

WW8_CP MainTextPlcDrawObj::GetCpOffset(const WW8Fib &) const
{
    return 0;
}

void HdFtPlcDrawObj::RegisterWithFib(WW8Fib &rFib, sal_uInt32 nStart,
    sal_uInt32 nLen) const
{
    rFib.m_fcPlcfspaHdr = nStart;
    rFib.m_lcbPlcfspaHdr = nLen;
}

WW8_CP HdFtPlcDrawObj::GetCpOffset(const WW8Fib &rFib) const
{
    return rFib.m_ccpText + rFib.m_ccpFootnote;
}

bool PlcDrawObj::Append( WW8Export const & rWrt, WW8_CP nCp, const ww8::Frame& rFormat,
    const Point& rNdTopLeft )
{
    bool bRet = false;
    const SwFrameFormat &rFrameFormat = rFormat.GetFrameFormat();
    if (TXT_HDFT == rWrt.m_nTextTyp || TXT_MAINTEXT == rWrt.m_nTextTyp)
    {
        if (RES_FLYFRMFMT == rFrameFormat.Which())
        {
            // check for textflyframe and if it is the first in a Chain
            if (rFrameFormat.GetContent().GetContentIdx())
                bRet = true;
        }
        else
            bRet = true;
    }

    if (bRet)
    {
        DrawObj aObj(rFormat, nCp, rNdTopLeft, rWrt.TrueFrameDirection(rFrameFormat),
            rWrt.GetHdFtIndex());
        maDrawObjs.push_back(aObj);
    }
    return bRet;
}

void DrawObj::SetShapeDetails(sal_uInt32 nId, sal_Int32 nThick)
{
    mnShapeId = nId;
    mnThick = nThick;
}

bool WW8_WrPlcTextBoxes::WriteText( WW8Export& rWrt )
{
    rWrt.m_bInWriteEscher = true;
    WW8_CP& rccp=TXT_TXTBOX == nTyp ? rWrt.pFib->m_ccpTxbx : rWrt.pFib->m_ccpHdrTxbx;

    bool bRet = WriteGenericText( rWrt, nTyp, rccp );

    WW8_CP nCP = rWrt.Fc2Cp( rWrt.Strm().Tell() );
    WW8Fib& rFib = *rWrt.pFib;
    WW8_CP nMyOffset = rFib.m_ccpText + rFib.m_ccpFootnote + rFib.m_ccpHdr + rFib.m_ccpAtn
                            + rFib.m_ccpEdn;
    if( TXT_TXTBOX == nTyp )
        rWrt.m_pFieldTextBxs->Finish( nCP, nMyOffset );
    else
        rWrt.m_pFieldHFTextBxs->Finish( nCP, nMyOffset + rFib.m_ccpTxbx );
    rWrt.m_bInWriteEscher = false;
    return bRet;
}

void WW8_WrPlcTextBoxes::Append( const SdrObject& rObj, sal_uInt32 nShapeId )
{
    aContent.push_back( &rObj );
    aShapeIds.push_back( nShapeId );
    //save NULL, if we have an actual SdrObject
    aSpareFormats.push_back(nullptr);
}

void WW8_WrPlcTextBoxes::Append( const SwFrameFormat* pFormat, sal_uInt32 nShapeId )
{
    //no sdr object, we insert a NULL in the aContent and save the real fmt in aSpareFormats.
    aContent.push_back( nullptr );
    aShapeIds.push_back( nShapeId );
    aSpareFormats.push_back(pFormat);
}

const std::vector<sal_uInt32>* WW8_WrPlcTextBoxes::GetShapeIdArr() const
{
    return &aShapeIds;
}

sal_uInt32 WW8Export::GetSdrOrdNum( const SwFrameFormat& rFormat ) const
{
    sal_uInt32 nOrdNum;
    const SdrObject* pObj = rFormat.FindRealSdrObject();
    if( pObj )
        nOrdNum = pObj->GetOrdNum();
    else
    {
        // no Layout for this format, then recalc the ordnum
        SwFrameFormat* pFormat = const_cast<SwFrameFormat*>(&rFormat);
        nOrdNum = std::distance(m_rDoc.GetSpzFrameFormats()->begin(),
                                m_rDoc.GetSpzFrameFormats()->find( pFormat ) );

        const SwDrawModel* pModel = m_rDoc.getIDocumentDrawModelAccess().GetDrawModel();
        if( pModel )
            nOrdNum += pModel->GetPage( 0 )->GetObjCount();
    }
    return nOrdNum;
}

void WW8Export::AppendFlyInFlys(const ww8::Frame& rFrameFormat,
    const Point& rNdTopLeft)
{
    OSL_ENSURE(!m_pEscher, "the EscherStream was already written!");
    if (m_pEscher)
        return ;
    PlcDrawObj *pDrwO;
    if (TXT_HDFT == m_nTextTyp)
        pDrwO = m_pHFSdrObjs.get();
    else
        pDrwO = m_pSdrObjs.get();

    if (rFrameFormat.IsInline())
    {
        OutputField(nullptr, ww::eSHAPE, FieldString(ww::eSHAPE),
            FieldFlags::Start | FieldFlags::CmdStart | FieldFlags::CmdEnd);
    }

    WW8_CP nCP = Fc2Cp(Strm().Tell());
    bool bSuccess = pDrwO->Append(*this, nCP, rFrameFormat, rNdTopLeft);
    OSL_ENSURE(bSuccess, "Couldn't export a graphical element!");

    if (bSuccess)
    {
        static const sal_uInt8 aSpec8[] =
        {
            0x03, 0x6a, 0, 0, 0, 0, // sprmCObjLocation
            0x55, 0x08, 1           // sprmCFSpec
        };
                                                // fSpec-Attribute true

        // A special character is required in the text for DrawObjects,
        // therefore a fSpec-Attribute
        m_pChpPlc->AppendFkpEntry( Strm().Tell() );
        WriteChar( 0x8 );
        m_pChpPlc->AppendFkpEntry( Strm().Tell(), sizeof( aSpec8 ), aSpec8 );

        //Need dummy picture frame
        if (rFrameFormat.IsInline())
            OutGrf(rFrameFormat);
    }

    if (rFrameFormat.IsInline())
        OutputField(nullptr, ww::eSHAPE, OUString(), FieldFlags::Close);
}

MSWord_SdrAttrIter::MSWord_SdrAttrIter( MSWordExportBase& rWr,
    const EditTextObject& rEditObj, sal_uInt8 nTyp )
    : MSWordAttrIter( rWr ), pEditObj(&rEditObj), pEditPool(nullptr), mnTyp(nTyp)
{
    NextPara( 0 );
}

void MSWord_SdrAttrIter::NextPara( sal_Int32 nPar )
{
    nPara = nPar;
    // Ignore change of attribute at position 0, because we expect that
    // the attributes are outputted at start of a paragraph anyway.
    aChrTextAtrArr.clear();
    aChrSetArr.clear();
    nCurrentSwPos = nTmpSwPos = 0;

    SfxItemSet aSet( pEditObj->GetParaAttribs( nPara ));
    pEditPool = aSet.GetPool();
    eNdChrSet = ItemGet<SvxFontItem>(aSet,EE_CHAR_FONTINFO).GetCharSet();

    assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());
    nScript = g_pBreakIt->GetBreakIter()->getScriptType( pEditObj->GetText(nPara), 0);

    pEditObj->GetCharAttribs( nPara, aTextAtrArr );
    nCurrentSwPos = SearchNext( 1 );
}

rtl_TextEncoding MSWord_SdrAttrIter::GetNextCharSet() const
{
    if( !aChrSetArr.empty() )
        return aChrSetArr.back();
    return eNdChrSet;
}

// the first parameter in SearchNext() returns if it's a TextAtr
sal_Int32 MSWord_SdrAttrIter::SearchNext( sal_Int32 nStartPos )
{
    sal_Int32 nMinPos = SAL_MAX_INT32;
    for(const auto& rTextAtr : aTextAtrArr)
    {
        sal_Int32 nPos = rTextAtr.nStart; // first character attribute
        if( nPos >= nStartPos && nPos <= nMinPos )
        {
            nMinPos = nPos;
            SetCharSet(rTextAtr, true);
        }

        nPos = rTextAtr.nEnd;              // last character attribute + 1
        if( nPos >= nStartPos && nPos < nMinPos )
        {
            nMinPos = nPos;
            SetCharSet(rTextAtr, false);
        }
    }
    return nMinPos;
}

void MSWord_SdrAttrIter::SetCharSet(const EECharAttrib& rAttr, bool bStart)
{
    const SfxPoolItem& rItem = *rAttr.pAttr;
    if( rItem.Which() != EE_CHAR_FONTINFO )
    {
        return;
    }

    if( bStart )
    {
        rtl_TextEncoding eChrSet = static_cast<const SvxFontItem&>(rItem).GetCharSet();
        aChrSetArr.push_back( eChrSet );
        aChrTextAtrArr.push_back( &rAttr );
    }
    else
    {
        std::vector<const EECharAttrib*>::iterator it =
           std::find( aChrTextAtrArr.begin(), aChrTextAtrArr.end(), &rAttr );
        if ( it != aChrTextAtrArr.end() )
        {
            aChrSetArr.erase( aChrSetArr.begin() + (it - aChrTextAtrArr.begin()) );
            aChrTextAtrArr.erase( it );
        }
    }
}

void MSWord_SdrAttrIter::OutEEField(const SfxPoolItem& rHt)
{
    const SvxFieldItem &rField = static_cast<const SvxFieldItem &>(rHt);
    const SvxFieldData *pField = rField.GetField();
    if (auto pURL = dynamic_cast< const SvxURLField *>( pField ))
    {
        sal_uInt8 nOldTextTyp = m_rExport.m_nTextTyp;
        m_rExport.m_nTextTyp = mnTyp;
        m_rExport.AttrOutput().StartURL( pURL->GetURL(), pURL->GetTargetFrame() );

        const OUString &rStr = pURL->GetRepresentation();
        m_rExport.AttrOutput().RawText(rStr, GetNodeCharSet());

        m_rExport.AttrOutput().EndURL(false);
        m_rExport.m_nTextTyp = nOldTextTyp;
    }
}

void MSWord_SdrAttrIter::OutAttr( sal_Int32 nSwPos )
{
    //Collect the which ids belong to the run that we will export after
    //outputting the underlying paragraph attributes. We will exclude
    //writing these from the underlying paragraph attributes to avoid
    //duplicate attributes in docx export. Doesn't matter in doc
    //export as later props just override earlier ones.
    std::set<sal_uInt16> aUsedRunWhichs;
    for(const auto& rTextAtr : aTextAtrArr)
    {
        if (nSwPos >= rTextAtr.nStart && nSwPos < rTextAtr.nEnd)
        {
            sal_uInt16 nWhich = rTextAtr.pAttr->Which();
            aUsedRunWhichs.insert(nWhich);
        }

        if( nSwPos < rTextAtr.nStart )
            break;
    }

    OutParaAttr(true, &aUsedRunWhichs);

    if (aTextAtrArr.empty())
        return;

    const sw::BroadcastingModify* pOldMod = m_rExport.m_pOutFormatNode;
    m_rExport.m_pOutFormatNode = nullptr;

    const SfxItemPool* pSrcPool = pEditPool;
    const SfxItemPool& rDstPool = m_rExport.m_rDoc.GetAttrPool();

    nTmpSwPos = nSwPos;
    // Did we already produce a <w:sz> element?
    m_rExport.m_bFontSizeWritten = false;
    for(const auto& rTextAtr : aTextAtrArr)
    {
        if (nSwPos >= rTextAtr.nStart && nSwPos < rTextAtr.nEnd)
        {
            sal_uInt16 nWhich = rTextAtr.pAttr->Which();
            if (nWhich == EE_FEATURE_FIELD)
            {
                OutEEField(*(rTextAtr.pAttr));
                continue;
            }
            if (nWhich == EE_FEATURE_TAB)
            {
                m_rExport.WriteChar(0x9);
                continue;
            }

            const sal_uInt16 nSlotId = pSrcPool->GetSlotId(nWhich);
            if (nSlotId && nWhich != nSlotId)
            {
                nWhich = rDstPool.GetWhich(nSlotId);
                if (nWhich && nWhich != nSlotId &&
                    nWhich < RES_UNKNOWNATR_BEGIN &&
                    m_rExport.CollapseScriptsforWordOk(nScript,nWhich))
                {
                    // use always the SW-Which Id !
                    std::unique_ptr<SfxPoolItem> pI(rTextAtr.pAttr->Clone());
                    pI->SetWhich( nWhich );
                    // Will this item produce a <w:sz> element?
                    bool bFontSizeItem = nWhich == RES_CHRATR_FONTSIZE || nWhich == RES_CHRATR_CJK_FONTSIZE;
                    if (!m_rExport.m_bFontSizeWritten || !bFontSizeItem)
                        m_rExport.AttrOutput().OutputItem( *pI );
                    if (bFontSizeItem)
                        m_rExport.m_bFontSizeWritten = true;
                }
            }
        }

        if( nSwPos < rTextAtr.nStart )
            break;
    }
    m_rExport.m_bFontSizeWritten = false;

    nTmpSwPos = 0;      // HasTextItem only allowed in the above area
    m_rExport.m_pOutFormatNode = pOldMod;
}

bool MSWord_SdrAttrIter::IsTextAttr(sal_Int32 nSwPos)
{
    return std::any_of(aTextAtrArr.begin(), aTextAtrArr.end(),
        [nSwPos](const EECharAttrib& rTextAtr) {
            return (nSwPos >= rTextAtr.nStart && nSwPos < rTextAtr.nEnd) &&
                (rTextAtr.pAttr->Which() == EE_FEATURE_FIELD ||
                 rTextAtr.pAttr->Which() == EE_FEATURE_TAB);
        }
    );
}

// HasItem is used for the consolidation  of the double attribute Underline and
// WordLineMode as a TextItem. OutAttr() calls the output function, which can
// query for other items at the start position of attribute via HasItem().
// Only attributes with an end can be queried.
// The search is done with bDeep.
const SfxPoolItem* MSWord_SdrAttrIter::HasTextItem(sal_uInt16 nWhich) const
{
    nWhich = sw::hack::TransformWhichBetweenPools(*pEditPool,
        m_rExport.m_rDoc.GetAttrPool(), nWhich);
    if (nWhich)
    {
        for (const auto& rTextAtr : aTextAtrArr)
        {
            if (nWhich == rTextAtr.pAttr->Which() && nTmpSwPos >= rTextAtr.nStart && nTmpSwPos < rTextAtr.nEnd)
                return rTextAtr.pAttr;    // Found
            if (nTmpSwPos < rTextAtr.nStart)
                return nullptr;
        }
    }
    return nullptr;
}

const SfxPoolItem& MSWord_SdrAttrIter::GetItem( sal_uInt16 nWhich ) const
{
    using sw::hack::GetSetWhichFromSwDocWhich;
    const SfxPoolItem* pRet = HasTextItem(nWhich);
    if (!pRet)
    {
        SfxItemSet aSet(pEditObj->GetParaAttribs(nPara));
        nWhich = GetSetWhichFromSwDocWhich(aSet, m_rExport.m_rDoc, nWhich);
        OSL_ENSURE(nWhich, "Impossible, catastrophic failure imminent");
        pRet = &aSet.Get(nWhich);
    }
    return *pRet;
}

//Drawing shapes properties inherit from a different pool that the document
//styles. On export to .doc[x] they will default to style "Normal". Here explicitly
//set any items which are not already set, but differ from "Normal".
void MSWord_SdrAttrIter::SetItemsThatDifferFromStandard(bool bCharAttr, SfxItemSet& rSet)
{
    SwTextFormatColl* pC = m_rExport.m_rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool
        (RES_POOLCOLL_STANDARD, false);

    SfxWhichIter aWhichIter(rSet);
    for (sal_uInt16 nEEWhich = aWhichIter.FirstWhich(); nEEWhich; nEEWhich = aWhichIter.NextWhich())
    {
        if (SfxItemState::SET != rSet.GetItemState(nEEWhich, false))
        {
            sal_uInt16 nSwWhich = sw::hack::TransformWhichBetweenPools(m_rExport.m_rDoc.GetAttrPool(),
                *pEditPool, nEEWhich);
            if (!nSwWhich)
                continue;
            bool bWanted = ( bCharAttr ? ( nSwWhich >= RES_CHRATR_BEGIN && nSwWhich < RES_TXTATR_END )
                            : ( nSwWhich >= RES_PARATR_BEGIN && nSwWhich < RES_FRMATR_END ) );
            if (!bWanted)
                continue;

            const SfxPoolItem& rDrawItem = rSet.Get(nEEWhich);
            const SfxPoolItem& rStandardItem = pC->GetFormatAttr(nSwWhich);
            if (rDrawItem != rStandardItem)
                rSet.Put(rDrawItem);
        }
    }
}

void MSWord_SdrAttrIter::OutParaAttr(bool bCharAttr, const std::set<sal_uInt16>* pWhichsToIgnore)
{
    SfxItemSet aSet( pEditObj->GetParaAttribs( nPara ));

    SetItemsThatDifferFromStandard(bCharAttr, aSet);

    if (!aSet.Count())
        return;

    const SfxItemSet* pOldSet = m_rExport.GetCurItemSet();
    m_rExport.SetCurItemSet( &aSet );

    SfxItemIter aIter( aSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();

    const SfxItemPool* pSrcPool = pEditPool,
                     * pDstPool = &m_rExport.m_rDoc.GetAttrPool();

    do
    {
        sal_uInt16 nWhich = pItem->Which();
        if (pWhichsToIgnore && pWhichsToIgnore->find(nWhich) != pWhichsToIgnore->end())
            continue;

        sal_uInt16 nSlotId = pSrcPool->GetSlotId(nWhich);

        if ( nSlotId && nWhich != nSlotId &&
             0 != ( nWhich = pDstPool->GetWhich( nSlotId ) ) &&
             nWhich != nSlotId &&
             ( bCharAttr ? ( nWhich >= RES_CHRATR_BEGIN && nWhich < RES_TXTATR_END )
                         : ( nWhich >= RES_PARATR_BEGIN && nWhich < RES_FRMATR_END ) ) )
        {
            // use always the SW-Which Id !
            std::unique_ptr<SfxPoolItem> pI(pItem->Clone());
            pI->SetWhich( nWhich );
            if (m_rExport.CollapseScriptsforWordOk(nScript,nWhich))
                m_rExport.AttrOutput().OutputItem(*pI);
        }
    } while ((pItem = aIter.NextItem()));
    m_rExport.SetCurItemSet( pOldSet );
}

void WW8Export::WriteSdrTextObj(const SdrTextObj& rTextObj, sal_uInt8 nTyp)
{
    std::optional<OutlinerParaObject> pParaObj;

    /*
    #i13885#
    When the object is actively being edited, that text is not set into
    the objects normal text object, but lives in a separate object.
    */
    if (rTextObj.IsTextEditActive())
    {
        pParaObj = rTextObj.CreateEditOutlinerParaObject();
    }
    else if (rTextObj.GetOutlinerParaObject())
    {
        pParaObj = *rTextObj.GetOutlinerParaObject();
    }

    if( pParaObj )
    {
        WriteOutliner(*pParaObj, nTyp);
    }
}

void WW8Export::WriteOutliner(const OutlinerParaObject& rParaObj, sal_uInt8 nTyp)
{
    bool bAnyWrite = false;
    const EditTextObject& rEditObj = rParaObj.GetTextObject();
    MSWord_SdrAttrIter aAttrIter( *this, rEditObj, nTyp );

    sal_Int32 nPara = rEditObj.GetParagraphCount();
    sal_uInt8 bNul = 0;
    for( sal_Int32 n = 0; n < nPara; ++n )
    {
        if( n )
            aAttrIter.NextPara( n );

        OSL_ENSURE( pO->empty(), " pO is not empty at start of line" );

        OUString aStr( rEditObj.GetText( n ));
        sal_Int32 nCurrentPos = 0;
        const sal_Int32 nEnd = aStr.getLength();

        const SfxItemSet& aSet(rEditObj.GetParaAttribs(n));
        bool bIsRTLPara = false;
        const SfxPoolItem *pItem;
        if(SfxItemState::SET == aSet.GetItemState(EE_PARA_WRITINGDIR, true, &pItem))
        {
            SvxFrameDirection nDir = static_cast<const SvxFrameDirectionItem*>(pItem)->GetValue();
            bIsRTLPara = SvxFrameDirection::Horizontal_RL_TB == nDir;
        }

        do {
            const sal_Int32 nNextAttr = std::min(aAttrIter.WhereNext(), nEnd);

            bool bTextAtr = aAttrIter.IsTextAttr( nCurrentPos );
            if( !bTextAtr )
                OutSwString(aStr, nCurrentPos, nNextAttr - nCurrentPos);

            // At the end of the line the attributes are extended over the CR.
            // exception: foot note at line end
            if( nNextAttr == nEnd && !bTextAtr )
                WriteCR();              // CR after it

                                                // output of character attributes
            aAttrIter.OutAttr( nCurrentPos );   // nCurrentPos - 1 ??

            if (bIsRTLPara)
            {
                // This is necessary to make word order correct in MS Word.
                // In theory we should do this for complex-script runs only,
                // but Outliner does not split runs like Writer core did.
                // Fortunately, both MS Word and Writer seems to tolerate
                // that we turn it on for non complex-script runs.
                AttrOutput().OutputItem(SfxInt16Item(RES_CHRATR_BIDIRTL, 1));
            }

            m_pChpPlc->AppendFkpEntry( Strm().Tell(),
                                            pO->size(), pO->data() );
            pO->clear();

            // exception: foot note at line end
            if( nNextAttr == nEnd && bTextAtr )
                WriteCR();              // CR after it
            nCurrentPos = nNextAttr;
            aAttrIter.NextPos();
        }
        while( nCurrentPos < nEnd );

        OSL_ENSURE( pO->empty(), " pO is not empty at start of line" );

        pO->push_back( bNul );        // Style # as short
        pO->push_back( bNul );

        aAttrIter.OutParaAttr(false);

        sal_uInt64 nPos = Strm().Tell();
        m_pPapPlc->AppendFkpEntry( Strm().Tell(),
                                        pO->size(), pO->data() );
        pO->clear();
        m_pChpPlc->AppendFkpEntry( nPos );
    }

    bAnyWrite = 0 != nPara;
    if( !bAnyWrite )
        WriteStringAsPara( OUString() );
}

void WinwordAnchoring::WriteData( EscherEx& rEx ) const
{
    //Toplevel groups get their winword extra data attached, and sub elements
    //use the defaults
    if (rEx.GetGroupLevel() > 1)
        return;

    SvStream& rSt = rEx.GetStream();
    //The last argument denotes the number of sub properties in this atom
    if (mbInline)
    {
        rEx.AddAtom(18, DFF_msofbtUDefProp, 3, 3); //Prop id is 0xF122
        rSt.WriteUInt16( 0x0390 ).WriteUInt32( 3 );
        rSt.WriteUInt16( 0x0392 ).WriteUInt32( 3 );
        //This sub property is required to be in the dummy inline frame as
        //well
        rSt.WriteUInt16( 0x053F ).WriteUInt32( nInlineHack );
    }
    else
    {
        rEx.AddAtom(24, DFF_msofbtUDefProp, 3, 4 ); //Prop id is 0xF122
        rSt.WriteUInt16( 0x038F ).WriteUInt32( mnXAlign );
        rSt.WriteUInt16( 0x0390 ).WriteUInt32( mnXRelTo );
        rSt.WriteUInt16( 0x0391 ).WriteUInt32( mnYAlign );
        rSt.WriteUInt16( 0x0392 ).WriteUInt32( mnYRelTo );
    }
}

void WW8Export::CreateEscher()
{
    SfxItemState eBackSet = m_rDoc.GetPageDesc(0).GetMaster().
        GetItemState(RES_BACKGROUND);
    if (m_pHFSdrObjs->size() || m_pSdrObjs->size() || SfxItemState::SET == eBackSet)
    {
        OSL_ENSURE( !m_pEscher, "Who did not deleted the pointer?" );
        SvMemoryStream* pEscherStrm = new SvMemoryStream;
        pEscherStrm->SetEndian(SvStreamEndian::LITTLE);
        m_pEscher = new SwEscherEx(pEscherStrm, *this);
    }
}

void WW8Export::WriteEscher()
{
    if (m_pEscher)
    {
        sal_uInt64 nStart = pTableStrm->Tell();

        m_pEscher->WritePictures();
        m_pEscher->FinishEscher();

        pFib->m_fcDggInfo = nStart;
        pFib->m_lcbDggInfo = pTableStrm->Tell() - nStart;
        delete m_pEscher;
        m_pEscher = nullptr;
    }
}

void SwEscherEx::WritePictures()
{
    if( SvStream* pPicStrm = static_cast< SwEscherExGlobal& >( *mxGlobal ).GetPictureStream() )
    {
        // set the blip - entries to the correct stream pos
        sal_Int32 nEndPos = rWrt.Strm().Tell();
        mxGlobal->SetNewBlipStreamOffset( nEndPos );

        pPicStrm->Seek( 0 );
        rWrt.Strm().WriteStream( *pPicStrm );
    }
    Flush();
}

// Output- Routines for Escher Export

SwEscherExGlobal::SwEscherExGlobal()
{
}

SwEscherExGlobal::~SwEscherExGlobal()
{
}

SvStream* SwEscherExGlobal::ImplQueryPictureStream()
{
    // this function will be called exactly once
    mxPicStrm = std::make_shared<SvMemoryStream>();
    mxPicStrm->SetEndian(SvStreamEndian::LITTLE);
    return mxPicStrm.get();
}

SwBasicEscherEx::SwBasicEscherEx(SvStream* pStrm, WW8Export& rWW8Wrt)
    : EscherEx( std::make_shared<SwEscherExGlobal>(), pStrm), rWrt(rWW8Wrt), pEscherStrm(pStrm)
{
    Init();
}

SwBasicEscherEx::~SwBasicEscherEx()
{
}

void SwBasicEscherEx::WriteFrameExtraData(const SwFrameFormat&)
{
    AddAtom(4, ESCHER_ClientAnchor);
    GetStream().WriteUInt32( 0x80000000 );
}

void SwBasicEscherEx::WriteEmptyFlyFrame(const SwFrameFormat& rFormat, sal_uInt32 nShapeId)
{
    OpenContainer(ESCHER_SpContainer);
    AddShape(ESCHER_ShpInst_PictureFrame, ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty, nShapeId);
    // store anchor attribute
    WriteFrameExtraData(rFormat);

    AddAtom(6, DFF_msofbtUDefProp, 3, 1); //Prop id is 0xF122
    GetStream().WriteUInt16( 0x053F ).WriteUInt32( nInlineHack );

    CloseContainer();   // ESCHER_SpContainer
}

static ShapeFlag AddMirrorFlags(ShapeFlag nFlags, const SwMirrorGrf &rMirror)
{
    switch (rMirror.GetValue())
    {
        default:
        case MirrorGraph::Dont:
            break;
        case MirrorGraph::Vertical:
            nFlags |= ShapeFlag::FlipH;
            break;
        case MirrorGraph::Horizontal:
            nFlags |= ShapeFlag::FlipV;
            break;
        case MirrorGraph::Both:
            nFlags |= ShapeFlag::FlipH | ShapeFlag::FlipV;
            break;

    }
    return nFlags;
}
//For i120928,this function is added to export graphic of bullet
void SwBasicEscherEx::WriteGrfBullet(const Graphic& rGrf)
{
    OpenContainer( ESCHER_SpContainer );
    AddShape(ESCHER_ShpInst_PictureFrame, ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty, 0x401);
    EscherPropertyContainer aPropOpt;
    GraphicObject aGraphicObject( rGrf );
    OString aUniqueId = aGraphicObject.GetUniqueID();
    if ( !aUniqueId.isEmpty() )
    {
        sal_uInt32 nBlibId = mxGlobal->GetBlibID( *(mxGlobal->QueryPictureStream()), aGraphicObject );
        if (nBlibId)
            aPropOpt.AddOpt(ESCHER_Prop_pib, nBlibId, true);
    }
    aPropOpt.AddOpt( ESCHER_Prop_pibFlags, ESCHER_BlipFlagDefault );
    aPropOpt.AddOpt( ESCHER_Prop_dyTextTop, DrawModelToEmu(0));
    aPropOpt.AddOpt( ESCHER_Prop_dyTextBottom, DrawModelToEmu(0));
    aPropOpt.AddOpt( ESCHER_Prop_dxTextLeft, DrawModelToEmu(0));
    aPropOpt.AddOpt( ESCHER_Prop_dxTextRight, DrawModelToEmu(0));
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
    aPropOpt.AddOpt( ESCHER_Prop_dyTextTop, 0 );
    aPropOpt.AddOpt( ESCHER_Prop_dyTextBottom, 0 );
    aPropOpt.AddOpt( ESCHER_Prop_dxTextLeft, 0 );
    aPropOpt.AddOpt( ESCHER_Prop_dxTextRight, 0 );
    const Color aTmpColor( COL_WHITE );
    std::shared_ptr<SvxBrushItem> aBrush(std::make_shared<SvxBrushItem>(aTmpColor, RES_BACKGROUND));
    const SvxBrushItem* pRet = rWrt.GetCurrentPageBgBrush();
    if (pRet && (pRet->GetGraphic() ||( pRet->GetColor() != COL_TRANSPARENT)))
        aBrush.reset(pRet->Clone());
    WriteBrushAttr(*aBrush, aPropOpt);

    aPropOpt.AddOpt( ESCHER_Prop_pictureActive, 0 );
    aPropOpt.Commit( GetStream() );
    AddAtom(4, ESCHER_ClientAnchor);
    GetStream().WriteUInt32( 0x80000000 );
    CloseContainer();
}

sal_Int32 SwBasicEscherEx::WriteGrfFlyFrame(const SwFrameFormat& rFormat, sal_uInt32 nShapeId)
{
    sal_Int32 nBorderThick=0;
    SwNoTextNode *pNd = GetNoTextNodeFromSwFrameFormat(rFormat);
    SwGrfNode *pGrfNd = pNd ? pNd->GetGrfNode() : nullptr;
    OSL_ENSURE(pGrfNd, "No SwGrfNode ?, suspicious");
    if (!pGrfNd)
        return nBorderThick;

    OpenContainer( ESCHER_SpContainer );

    const SwMirrorGrf &rMirror = pGrfNd->GetSwAttrSet().GetMirrorGrf();
    AddShape(ESCHER_ShpInst_PictureFrame,
             AddMirrorFlags(ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty, rMirror),
             nShapeId);

    EscherPropertyContainer aPropOpt;

    sal_uInt32 nFlags = ESCHER_BlipFlagDefault;

    if (pGrfNd->IsLinkedFile())
    {
        OUString sURL;
        pGrfNd->GetFileFilterNms( &sURL, nullptr );

        ww::bytes aBuf;
        SwWW8Writer::InsAsString16( aBuf, sURL );
        SwWW8Writer::InsUInt16( aBuf, 0 );

        aPropOpt.AddOpt(ESCHER_Prop_pibName, true, aBuf.size(), aBuf);
        nFlags = ESCHER_BlipFlagLinkToFile | ESCHER_BlipFlagURL |
                    ESCHER_BlipFlagDoNotSave;
    }
    else
    {
        const Graphic&  aGraphic(pGrfNd->GetGrf());
        GraphicObject   aGraphicObject( aGraphic );
        OString aUniqueId = aGraphicObject.GetUniqueID();

        if (!aUniqueId.isEmpty())
        {
            sal_uInt32 nBlibId = mxGlobal->GetBlibID( *QueryPictureStream(), aGraphicObject);
            if (nBlibId)
                aPropOpt.AddOpt(ESCHER_Prop_pib, nBlibId, true);
        }
    }

    aPropOpt.AddOpt( ESCHER_Prop_pibFlags, nFlags );
    nBorderThick = WriteFlyFrameAttr(rFormat,mso_sptPictureFrame,aPropOpt);
    WriteGrfAttr(*pGrfNd, rFormat, aPropOpt);

    aPropOpt.Commit( GetStream() );

    // store anchor attribute
    WriteFrameExtraData( rFormat );

    CloseContainer();   // ESCHER_SpContainer
    return nBorderThick;
}

void SwBasicEscherEx::WriteGrfAttr(const SwNoTextNode& rNd, const SwFrameFormat& rFormat,
    EscherPropertyContainer& rPropOpt)
{
    const SfxPoolItem* pItem;
    GraphicDrawMode nMode = GraphicDrawMode::Standard;
    sal_Int32 nContrast = 0;
    sal_Int16 nBrightness = 0;

    if (SfxItemState::SET == rNd.GetSwAttrSet().GetItemState(RES_GRFATR_CONTRAST,
        true, &pItem))
    {
        nContrast = static_cast<const SfxInt16Item*>(pItem)->GetValue();
    }

    if (SfxItemState::SET == rNd.GetSwAttrSet().GetItemState(RES_GRFATR_LUMINANCE,
        true, &pItem))
    {
        nBrightness = static_cast<const SfxInt16Item*>(pItem)->GetValue();
    }

    if (SfxItemState::SET == rNd.GetSwAttrSet().GetItemState(RES_GRFATR_DRAWMODE,
        true, &pItem))
    {
        nMode = static_cast<GraphicDrawMode>(static_cast<const SfxEnumItemInterface*>(pItem)->GetEnumValue());
        if (nMode == GraphicDrawMode::Watermark)
        {
            /*
            There is no real watermark mode in word, we must use standard
            mode and modify our ones by 70% extra brightness and 70% less
            contrast. This means that unmodified default OOo watermark
            will turn back into watermark, and modified OOo watermark will
            change into a close visual representation in standardmode
            */
            nBrightness += 70;
            if (nBrightness > 100)
                nBrightness = 100;
            nContrast -= 70;
            if (nContrast < -100)
                nContrast = -100;
            nMode = GraphicDrawMode::Standard;
        }
    }

    sal_uInt32 nPictureMode;
    if (nMode == GraphicDrawMode::Greys)
        nPictureMode = 0x40004;
    else if (nMode == GraphicDrawMode::Mono)
        nPictureMode = 0x60006;
    else
        nPictureMode = 0;
    rPropOpt.AddOpt( ESCHER_Prop_pictureActive, nPictureMode );

    if (nContrast != 0)
    {
        nContrast+=100;
        if (nContrast == 100)
            nContrast = 0x10000;
        else if (nContrast < 100)
        {
            nContrast *= 0x10000;
            nContrast /= 100;
        }
        else if (nContrast < 200)
            nContrast = (100 * 0x10000) / (200-nContrast);
        else
            nContrast = 0x7fffffff;
        rPropOpt.AddOpt( ESCHER_Prop_pictureContrast, nContrast);
    }

    if (nBrightness != 0)
        rPropOpt.AddOpt( ESCHER_Prop_pictureBrightness, nBrightness * 327 );

    sal_Int32 nCropL = 0;
    sal_Int32 nCropR = 0;
    sal_Int32 nCropT = 0;
    sal_Int32 nCropB = 0;
    if (SfxItemState::SET == rNd.GetSwAttrSet().GetItemState(RES_GRFATR_CROPGRF,
        true, &pItem))
    {
        const SwCropGrf& rCrop = *static_cast<const SwCropGrf*>(pItem);
        nCropL += rCrop.GetLeft();
        nCropR += rCrop.GetRight();
        nCropT += rCrop.GetTop();
        nCropB += rCrop.GetBottom();
    }

    // simulate border padding as a negative crop.
    if (SfxItemState::SET == rFormat.GetItemState(RES_BOX, false, &pItem))
    {
        const SvxBoxItem& rBox = *static_cast<const SvxBoxItem*>(pItem);
        nCropL -= rBox.GetDistance( SvxBoxItemLine::LEFT );
        nCropR -= rBox.GetDistance( SvxBoxItemLine::RIGHT );
        nCropT -= rBox.GetDistance( SvxBoxItemLine::TOP );
        nCropB -= rBox.GetDistance( SvxBoxItemLine::BOTTOM );
    }

    const Size aSz( rNd.GetTwipSize() );
    if( 0 != nCropL )
        rPropOpt.AddOpt( ESCHER_Prop_cropFromLeft, ToFract16( nCropL, aSz.Width()) );
    if( 0 != nCropR )
        rPropOpt.AddOpt( ESCHER_Prop_cropFromRight, ToFract16( nCropR, aSz.Width()));
    if( 0 != nCropT )
        rPropOpt.AddOpt( ESCHER_Prop_cropFromTop, ToFract16( nCropT, aSz.Height()));
    if( 0 != nCropB )
        rPropOpt.AddOpt( ESCHER_Prop_cropFromBottom, ToFract16( nCropB, aSz.Height()));
}

void SwBasicEscherEx::SetPicId(const SdrObject &, sal_uInt32,
    EscherPropertyContainer &)
{
}

void SwEscherEx::SetPicId(const SdrObject &rSdrObj, sal_uInt32 nShapeId,
    EscherPropertyContainer &rPropOpt)
{
    pTextBxs->Append(rSdrObj, nShapeId);
    sal_uInt32 nPicId = pTextBxs->Count();
    nPicId *= 0x10000;
    rPropOpt.AddOpt( ESCHER_Prop_pictureId, nPicId );
}

sal_Int32 SwBasicEscherEx::WriteOLEFlyFrame(const SwFrameFormat& rFormat, sal_uInt32 nShapeId)
{
    sal_Int32 nBorderThick = 0;
    if (const SdrObject* pSdrObj = rFormat.FindRealSdrObject())
    {
        SwNodeIndex aIdx(*rFormat.GetContent().GetContentIdx(), 1);
        SwOLENode& rOLENd = *aIdx.GetNode().GetOLENode();
        sal_Int64 nAspect = rOLENd.GetAspect();

        uno::Reference < embed::XEmbeddedObject > xObj(rOLENd.GetOLEObj().GetOleRef());

        // the rectangle is used to transport the size of the object
        // the left, top corner is set to ( 0, 0 ) by default constructor,
        // if the width and height are set correctly bRectIsSet should be set to true
        awt::Rectangle aRect;
        bool bRectIsSet = false;

        // TODO/LATER: should the icon size be stored in case of iconified object?
        if ( xObj.is() && nAspect != embed::Aspects::MSOLE_ICON )
        {
            try
            {
                awt::Size aSize = xObj->getVisualAreaSize( nAspect );
                aRect.Width = aSize.Width;
                aRect.Height = aSize.Height;
                bRectIsSet = true;
            }
            catch( const uno::Exception& )
            {}
        }

        /*
        #i5970#
        Export floating ole2 .doc ver 8+ wmf ole2 previews as emf previews
        instead ==> allows unicode text to be preserved
        */
#ifdef OLE_PREVIEW_AS_EMF
        const Graphic* pGraphic = rOLENd.GetGraphic();
#endif
        OpenContainer(ESCHER_SpContainer);

        EscherPropertyContainer aPropOpt;
        const SwMirrorGrf &rMirror = rOLENd.GetSwAttrSet().GetMirrorGrf();
        WriteOLEPicture(aPropOpt,
                        AddMirrorFlags(ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty | ShapeFlag::OLEShape, rMirror),
                        pGraphic ? *pGraphic : Graphic(), *pSdrObj, nShapeId, bRectIsSet ? &aRect : nullptr );

        nBorderThick = WriteFlyFrameAttr(rFormat, mso_sptPictureFrame, aPropOpt);
        WriteGrfAttr(rOLENd, rFormat, aPropOpt);
        aPropOpt.Commit(GetStream());

        // store anchor attribute
        WriteFrameExtraData( rFormat );

        CloseContainer();   // ESCHER_SpContainer
    }
    return nBorderThick;
}

void SwBasicEscherEx::WriteBrushAttr(const SvxBrushItem &rBrush,
    EscherPropertyContainer& rPropOpt)
{
    bool bSetOpacity = false;
    sal_uInt32 nOpaque = 0;
    if (const GraphicObject *pGraphicObject = rBrush.GetGraphicObject())
    {
        OString aUniqueId = pGraphicObject->GetUniqueID();
        if (!aUniqueId.isEmpty())
        {
            sal_uInt32 nBlibId = mxGlobal->GetBlibID(*QueryPictureStream(), *pGraphicObject);
            if (nBlibId)
                rPropOpt.AddOpt(ESCHER_Prop_fillBlip,nBlibId,true);
        }

        nOpaque = 255 - pGraphicObject->GetAttr().GetAlpha();
        if (0 != nOpaque)
            bSetOpacity = true;

        rPropOpt.AddOpt( ESCHER_Prop_fillType, ESCHER_FillPicture );
        rPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
        rPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0 );
    }
    else
    {
        sal_uInt32 nFillColor = GetColor(rBrush.GetColor());
        rPropOpt.AddOpt( ESCHER_Prop_fillColor, nFillColor );
        rPropOpt.AddOpt( ESCHER_Prop_fillBackColor, nFillColor ^ 0xffffff );
        rPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100010 );

        nOpaque = 255 - rBrush.GetColor().GetAlpha();
        if (0 != nOpaque)
            bSetOpacity = true;
    }

    if (bSetOpacity)
    {
        nOpaque = (nOpaque * 100) / 0xFE;
        nOpaque = ((100 - nOpaque) << 16) / 100;
        rPropOpt.AddOpt(ESCHER_Prop_fillOpacity, nOpaque);
    }
}

sal_Int32 SwBasicEscherEx::WriteFlyFrameAttr(const SwFrameFormat& rFormat,
    MSO_SPT eShapeType, EscherPropertyContainer& rPropOpt)
{
    sal_Int32 nLineWidth=0;
    const SfxPoolItem* pItem;
    bool bFirstLine = true;
    if (SfxItemState::SET == rFormat.GetItemState(RES_BOX, true, &pItem))
    {
        static const o3tl::enumarray<SvxBoxItemLine, sal_uInt16> aExhperProp =
        {
            ESCHER_Prop_dyTextTop,  ESCHER_Prop_dyTextBottom,
            ESCHER_Prop_dxTextLeft, ESCHER_Prop_dxTextRight
        };
        const SvxBorderLine* pLine;

        for( SvxBoxItemLine n : o3tl::enumrange<SvxBoxItemLine>() )
        {
            pLine = static_cast<const SvxBoxItem*>(pItem)->GetLine( n );
            if( nullptr != pLine )
            {
                if( bFirstLine )
                {
                    sal_uInt32 nLineColor = GetColor(pLine->GetColor());
                    rPropOpt.AddOpt( ESCHER_Prop_lineColor, nLineColor );
                    rPropOpt.AddOpt( ESCHER_Prop_lineBackColor,
                        nLineColor ^ 0xffffff );

                    MSO_LineStyle eStyle;
                    if( pLine->isDouble() )
                    {
                        // double line
                        nLineWidth = pLine->GetWidth();
                        if( pLine->GetInWidth() == pLine->GetOutWidth() )
                            eStyle = mso_lineDouble;
                        else if( pLine->GetInWidth() < pLine->GetOutWidth() )
                            eStyle = mso_lineThickThin;
                        else
                            eStyle = mso_lineThinThick;
                    }
                    else
                    {
                        // simple line
                        eStyle = mso_lineSimple;
                        nLineWidth = pLine->GetWidth();
                    }

                    rPropOpt.AddOpt( ESCHER_Prop_lineStyle, eStyle );
                    rPropOpt.AddOpt( ESCHER_Prop_lineWidth,
                        DrawModelToEmu( nLineWidth ));

                    MSO_LineDashing eDashing = mso_lineSolid;
                    switch (pLine->GetBorderLineStyle())
                    {
                        case SvxBorderLineStyle::DASHED:
                            eDashing = mso_lineDashGEL;
                            break;
                        case SvxBorderLineStyle::DOTTED:
                            eDashing = mso_lineDotGEL;
                            break;
                        case SvxBorderLineStyle::SOLID:
                        default:
                            break;
                    }
                    rPropOpt.AddOpt( ESCHER_Prop_lineDashing, eDashing );
                    rPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x8000E );

                    //Use import logic to determine how much of border will go
                    //outside graphic
                    nLineWidth = SwMSDffManager::GetEscherLineMatch(
                        eStyle,eShapeType,nLineWidth);
                    bFirstLine = false;
                }
                rPropOpt.AddOpt( aExhperProp[ n ], DrawModelToEmu(
                    static_cast<const SvxBoxItem*>(pItem)->GetDistance( n ) ));
            }
            else
                rPropOpt.AddOpt( aExhperProp[ n ], DrawModelToEmu(static_cast<const SvxBoxItem*>(pItem)->GetDistance( n )) );
        }
    }
    else
    {
        rPropOpt.AddOpt( ESCHER_Prop_dyTextTop, 0 );
        rPropOpt.AddOpt( ESCHER_Prop_dyTextBottom, 0 );
        rPropOpt.AddOpt( ESCHER_Prop_dxTextLeft, 0 );
        rPropOpt.AddOpt( ESCHER_Prop_dxTextRight, 0 );
    }

    if( bFirstLine )                // no valid line found
    {
        rPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
    }
    const SwAttrSet& rAttrSet = rFormat.GetAttrSet();
    if (SfxItemState::SET == rAttrSet.GetItemState(RES_BOX, false, &pItem))
    {
        const SvxBoxItem* pBox = static_cast<const SvxBoxItem*>(pItem);
        if( pBox )
        {
            const SfxPoolItem* pShadItem;
            if (SfxItemState::SET
                == rAttrSet.GetItemState(RES_SHADOW, true, &pShadItem))
            {
                const SvxShadowItem* pSI = static_cast<const SvxShadowItem*>(pShadItem);

                constexpr sal_uInt32 nShadowType = 131074;   // shadow type of ms word. need to set the default value.

                Color  nColor = pSI->GetColor();
                sal_Int32 nOffX
                    = o3tl::convert(pSI->GetWidth(), o3tl::Length::twip, o3tl::Length::emu);
                sal_Int32 nOffY
                    = o3tl::convert(pSI->GetWidth(), o3tl::Length::twip, o3tl::Length::emu);

                SvxShadowLocation eLocation = pSI->GetLocation();
                if( (eLocation!=SvxShadowLocation::NONE) && (pSI->GetWidth()!=0) )
                {
                    switch( eLocation )
                    {
                    case SvxShadowLocation::TopLeft:
                        {
                            nOffX = -nOffX;
                            nOffY = -nOffY;
                        }
                        break;
                    case SvxShadowLocation::TopRight:
                        {
                            nOffY = -nOffY;
                        }
                        break;
                    case SvxShadowLocation::BottomLeft:
                        {
                            nOffX = -nOffX;
                        }
                        break;
                    case SvxShadowLocation::BottomRight:
                        break;
                    default:
                        break;
                    }

                    rPropOpt.AddOpt( DFF_Prop_shadowColor,      wwUtility::RGBToBGR(nColor));
                    rPropOpt.AddOpt( DFF_Prop_shadowOffsetX,    nOffX );
                    rPropOpt.AddOpt( DFF_Prop_shadowOffsetY,    nOffY );
                    rPropOpt.AddOpt( DFF_Prop_fshadowObscured,  nShadowType );
                }
            }
        }
    }

    // SwWW8ImplReader::Read_GrafLayer() imports these as opaque
    // unconditionally, so if both are true, don't export the property.
    const bool bIsInHeader = sw::IsFlyFrameFormatInHeader(rFormat);
    const bool bIsThrough = rFormat.GetSurround().GetValue() == css::text::WrapTextMode_THROUGH;

    // Anything (like a transparent image) that allows text to wrap through should not force a non-transparent background,
    // and neither should the commonly seen backgrounds anchored in headers.
    if (bIsInHeader || bIsThrough)
    {
        std::unique_ptr<SvxBrushItem> aBrush(rFormat.makeBackgroundBrushItem());

        if(aBrush)
        {
            WriteBrushAttr(*aBrush, rPropOpt);
        }
    }
    else
    {
        // for unknown reasons, force exporting a non-transparent background on fly frames.
        std::shared_ptr<SvxBrushItem> aBrush(rWrt.TrueFrameBgBrush(rFormat));

        if(aBrush)
        {
            WriteBrushAttr(*aBrush, rPropOpt);
        }
    }

    const SdrObject* pObj = rFormat.FindRealSdrObject();

    if( pObj && (pObj->GetLayer() == GetHellLayerId() ||
        pObj->GetLayer() == GetInvisibleHellId() ) && !(bIsInHeader && bIsThrough))
    {
        rPropOpt.AddOpt( ESCHER_Prop_fPrint, 0x200020 );
    }

    PreWriteHyperlinkWithinFly(rFormat,rPropOpt);

    return nLineWidth;
}

sal_Int32 SwEscherEx::WriteFlyFrameAttr(const SwFrameFormat& rFormat, MSO_SPT eShapeType,
    EscherPropertyContainer& rPropOpt)
{
    sal_Int32 nLineWidth = SwBasicEscherEx::WriteFlyFrameAttr(rFormat, eShapeType,
        rPropOpt);

    /*
     These are not in SwBasicEscherEx::WriteFlyFrameAttr because inline objs
     can't do it in word and it hacks it in by stretching the graphic that
     way, perhaps we should actually draw in this space into the graphic we
     are exporting!
     */
    const SfxPoolItem* pItem;
    if (SfxItemState::SET == rFormat.GetItemState(RES_LR_SPACE, true, &pItem))
    {
        rPropOpt.AddOpt( ESCHER_Prop_dxWrapDistLeft,
                DrawModelToEmu( static_cast<const SvxLRSpaceItem*>(pItem)->GetLeft() ) );
        rPropOpt.AddOpt( ESCHER_Prop_dxWrapDistRight,
                DrawModelToEmu( static_cast<const SvxLRSpaceItem*>(pItem)->GetRight() ) );
    }
    else
    {
        rPropOpt.AddOpt( ESCHER_Prop_dxWrapDistLeft, 0 );
        rPropOpt.AddOpt( ESCHER_Prop_dxWrapDistRight, 0 );
    }

    if (SfxItemState::SET == rFormat.GetItemState(RES_UL_SPACE, true, &pItem))
    {
        rPropOpt.AddOpt( ESCHER_Prop_dyWrapDistTop,
                DrawModelToEmu( static_cast<const SvxULSpaceItem*>(pItem)->GetUpper() ) );
        rPropOpt.AddOpt( ESCHER_Prop_dyWrapDistBottom,
                DrawModelToEmu( static_cast<const SvxULSpaceItem*>(pItem)->GetLower() ) );
    }

    if (rFormat.GetSurround().IsContour())
    {
        if (const SwNoTextNode *pNd = GetNoTextNodeFromSwFrameFormat(rFormat))
        {
            const tools::PolyPolygon *pPolyPoly = pNd->HasContour();
            if (pPolyPoly && pPolyPoly->Count())
            {
                tools::Polygon aPoly = CorrectWordWrapPolygonForExport(*pPolyPoly, pNd, /*bCorrectCrop=*/false);
                SvMemoryStream aPolyDump;
                aPolyDump.SetEndian(SvStreamEndian::LITTLE);

                sal_uInt16 nLen = aPoly.GetSize();
                aPolyDump.WriteUInt16( nLen );
                aPolyDump.WriteUInt16( nLen );
                aPolyDump.WriteUInt16( 8 );
                for (sal_uInt16 nI = 0; nI < nLen; ++nI)
                {
                    aPolyDump.WriteUInt32( aPoly[nI].X() );
                    aPolyDump.WriteUInt32( aPoly[nI].Y() );
                }

                rPropOpt.AddOpt(DFF_Prop_pWrapPolygonVertices, false, 0, aPolyDump);
            }
        }
    }

    PreWriteHyperlinkWithinFly(rFormat,rPropOpt);

    return nLineWidth;
}

void SwBasicEscherEx::Init()
{
    MapUnit eMap = MapUnit::MapTwip;
    if (SwDrawModel *pModel = rWrt.m_rDoc.getIDocumentDrawModelAccess().GetDrawModel())
    {
        // PPT works only with units of 576DPI
        // WW however is using twips, i.e 1440DPI.
        eMap = pModel->GetScaleUnit();
    }

    // MS-DFF-Properties mostly are in EMU (English Metric Units)
    Fraction aFact = conversionFract(o3tl::Length::mm100, o3tl::Length::emu);
    aFact /= GetMapFactor(MapUnit::Map100thMM, eMap).X();
    mnEmuMul = aFact.GetNumerator();
    mnEmuDiv = aFact.GetDenominator();

    SetHellLayerId(rWrt.m_rDoc.getIDocumentDrawModelAccess().GetHellId());
}

sal_Int32 SwBasicEscherEx::ToFract16(sal_Int32 nVal, sal_uInt32 nMax)
{
    if (nMax)
    {
        if (nVal >= 0)
        {
            sal_Int32 nMSVal = (nVal / 65536) * nMax;
            nMSVal += (nVal * 65536) / nMax;
            return nMSVal;
        } else {
            // negative fraction does not have "-0", fractional part is always
            // positive:  -0.4 represented as -1 + 0.6
            sal_Int32 const nDiv = (nVal / sal_Int32(nMax)) - 1;
            sal_uInt32 nMSVal = (sal_uInt32(nDiv) << 16) & 0xffff0000;
            nMSVal += (nVal * 65536) / sal_Int32(nMax) + (-nDiv * 65536);
            return nMSVal;
        }
    }
    return 0;
}

SdrLayerID SwBasicEscherEx::GetInvisibleHellId() const
{
    return rWrt.m_rDoc.getIDocumentDrawModelAccess().GetInvisibleHellId();
}

void SwBasicEscherEx::WritePictures()
{
    if( SvStream* pPicStrm = static_cast< SwEscherExGlobal& >( *mxGlobal ).GetPictureStream() )
    {
        // set the blip - entries to the correct stream pos
        sal_Int32 nEndPos = pPicStrm->Tell();
        mxGlobal->WriteBlibStoreEntry(*pEscherStrm, 1, nEndPos);

        pPicStrm->Seek(0);
        pEscherStrm->WriteStream( *pPicStrm );
    }
}

SwEscherEx::SwEscherEx(SvStream* pStrm, WW8Export& rWW8Wrt)
    : SwBasicEscherEx(pStrm, rWW8Wrt),
    pTextBxs(nullptr)
{
    aHostData.SetClientData(&aWinwordAnchoring);
    OpenContainer( ESCHER_DggContainer );

    sal_uInt16 nColorCount = 4;
    pStrm ->WriteUInt16( nColorCount << 4 )     // instance
           .WriteUInt16( ESCHER_SplitMenuColors )   // record type
           .WriteUInt32( nColorCount * 4 )      // size
           .WriteUInt32( 0x08000004 )
           .WriteUInt32( 0x08000001 )
           .WriteUInt32( 0x08000002 )
           .WriteUInt32( 0x100000f7 );

    CloseContainer();   // ESCHER_DggContainer

    sal_uInt8 i = 2;     // for header/footer and the other
    PlcDrawObj *pSdrObjs = rWrt.m_pHFSdrObjs.get();
    pTextBxs = rWrt.m_pHFTextBxs.get();

    // if no header/footer -> skip over
    if (!pSdrObjs->size())
    {
        --i;
        pSdrObjs = rWrt.m_pSdrObjs.get();
        pTextBxs = rWrt.m_pTextBxs.get();
    }

    for( ; i--; pSdrObjs = rWrt.m_pSdrObjs.get(), pTextBxs = rWrt.m_pTextBxs.get() )
    {
        // "dummy char" (or any Count ?) - why? Only Microsoft knows it.
        GetStream().WriteChar( i );

        OpenContainer( ESCHER_DgContainer );

        EnterGroup();

        sal_uLong nSecondShapeId = pSdrObjs == rWrt.m_pSdrObjs.get() ? GenerateShapeId() : 0;

        // write now all Writer-/DrawObjects
        DrawObjPointerVector aSorted;
        MakeZOrderArrAndFollowIds(pSdrObjs->GetObjArr(), aSorted);

        sal_uInt32 nShapeId=0;
        for (auto& pObj : aSorted)
        {
            sal_Int32 nBorderThick=0;
            OSL_ENSURE(pObj, "impossible");
            if (!pObj)
                continue;
            const ww8::Frame &rFrame = pObj->maContent;
            const SwFrameFormat& rFormat = rFrame.GetFrameFormat();

            switch (rFrame.GetWriterType())
            {
                case ww8::Frame::eTextBox:
                case ww8::Frame::eOle:
                case ww8::Frame::eGraphic:
                    nBorderThick = WriteFlyFrame(*pObj, nShapeId, aSorted);
                    break;
                case ww8::Frame::eFormControl:
                    nShapeId = GenerateShapeId();
                    WriteOCXControl(rFormat, nShapeId);
                    break;
                case ww8::Frame::eDrawing:
                {
                    aWinwordAnchoring.SetAnchoring(rFormat);
                    const SdrObject* pSdrObj = rFormat.FindRealSdrObject();
                    if (pSdrObj)
                    {
                        nShapeId = AddSdrObject(*pSdrObj);
                    }
#if OSL_DEBUG_LEVEL > 0
                    else
                        OSL_ENSURE( false, "Where is the SDR-Object?" );
#endif
                }
                break;
                default:
                    break;
            }

            if( !nShapeId )
            {
                nShapeId = AddDummyShape();
            }

            pObj->SetShapeDetails(nShapeId, nBorderThick);
        }

        EndSdrObjectPage();         // ???? Bugfix for 74724

        if( nSecondShapeId )
        {
            OpenContainer( ESCHER_SpContainer );

            AddShape( ESCHER_ShpInst_Rectangle,
                      ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty | ShapeFlag::Background,
                      nSecondShapeId );

            EscherPropertyContainer aPropOpt;
            const SwFrameFormat &rFormat = rWrt.m_rDoc.GetPageDesc(0).GetMaster();
            const SfxPoolItem* pItem = nullptr;
            SfxItemState eState = rFormat.GetItemState(RES_BACKGROUND, true,
                &pItem);
            if (SfxItemState::SET == eState && pItem)
            {
                const SvxBrushItem* pBrush = static_cast<const SvxBrushItem*>(pItem);
                WriteBrushAttr(*pBrush, aPropOpt);

                SvxGraphicPosition ePos = pBrush->GetGraphicPos();
                if( ePos != GPOS_NONE && ePos != GPOS_AREA )
                {
                    /* #i56806# 0x033F parameter specifies a 32-bit field of shape boolean properties.
                    0x10001 means fBackground and fUsefBackground flag are true thus background
                    picture will be shown as "tiled" fill.*/
                    aPropOpt.AddOpt( ESCHER_Prop_fBackground, 0x10001 );
                }
            }
            aPropOpt.AddOpt( ESCHER_Prop_lineColor, 0x8000001 );
            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x00080008 );
            aPropOpt.AddOpt( ESCHER_Prop_shadowColor, 0x8000002 );
            aPropOpt.AddOpt( ESCHER_Prop_lineWidth, 0 );

            aPropOpt.Commit( *pStrm );

            AddAtom( 4, ESCHER_ClientData );
            GetStream().WriteInt32( 1 );

            CloseContainer();   // ESCHER_SpContainer
        }
        CloseContainer();   // ESCHER_DgContainer
    }
}

SwEscherEx::~SwEscherEx()
{
}

void SwEscherEx::FinishEscher()
{
    pEscherStrm->Seek(0);
    rWrt.pTableStrm->WriteStream( *pEscherStrm );
    delete pEscherStrm;
    pEscherStrm = nullptr;
}


namespace
{
    template<typename OrientType>
    void lcl_SetRelationOrient(OrientType& rOrient, const sw::WW8AnchorConv eConv, const std::function<void()>& fDefault)
    {
        switch(eConv)
        {
            case sw::WW8AnchorConv::RELTOTABLECELL:
                // #i33818#
                rOrient.SetRelationOrient(text::RelOrientation::PAGE_PRINT_AREA);
                break;
            case sw::WW8AnchorConv::CONV2PG:
                rOrient.SetRelationOrient(text::RelOrientation::PAGE_FRAME);
                break;
            case sw::WW8AnchorConv::CONV2COL_OR_PARA:
                rOrient.SetRelationOrient(text::RelOrientation::FRAME);
                break;
            case sw::WW8AnchorConv::CONV2CHAR:
                rOrient.SetRelationOrient(text::RelOrientation::CHAR);
                break;
            case sw::WW8AnchorConv::CONV2LINE:
                rOrient.SetRelationOrient(text::RelOrientation::TEXT_LINE);
                break;
            default:
                fDefault();
        }
    }
}
/** method to perform conversion of positioning attributes with the help
    of corresponding layout information

    #i30669#
    Because most of the Writer object positions doesn't correspond to the
    object positions in WW8, this method converts the positioning
    attributes. For this conversion the corresponding layout information
    is needed. If no layout information exists - e.g. no layout exists - no
    conversion is performed.
    No conversion is performed for as-character anchored objects. Whose
    object positions are already treated special in method <WriteData(..)>.

    @param _iorHoriOri
    input/output parameter - containing the current horizontal position
    attributes, which are converted by this method.

    @param _iorVertOri
    input/output parameter - containing the current vertical position
    attributes, which are converted by this method.

    @param _rFrameFormat
    input parameter - frame format of the anchored object

    @return boolean, indicating, if a conversion has been performed.
*/
bool WinwordAnchoring::ConvertPosition( SwFormatHoriOrient& _iorHoriOri,
                                         SwFormatVertOrient& _iorVertOri,
                                         const SwFrameFormat& _rFrameFormat )
{
    const RndStdIds eAnchor = _rFrameFormat.GetAnchor().GetAnchorId();

    if ( (RndStdIds::FLY_AS_CHAR == eAnchor) || (RndStdIds::FLY_AT_FLY == eAnchor) )
    {
        // no conversion for as-character or at frame anchored objects
        return false;
    }

    // determine value of attribute 'Follow text flow', because positions aligned
    // at page areas have to be converted, if it's set.
    const bool bFollowTextFlow = _rFrameFormat.GetFollowTextFlow().GetValue();

    // check, if horizontal and vertical position have to be converted due to
    // the fact, that the object is anchored at a paragraph, which has a "column
    // break before" attribute
    bool bConvDueToAnchoredAtColBreakPara( false );
    if ( ( (eAnchor == RndStdIds::FLY_AT_PARA) || (eAnchor == RndStdIds::FLY_AT_CHAR) ) &&
         _rFrameFormat.GetAnchor().GetContentAnchor() &&
         _rFrameFormat.GetAnchor().GetContentAnchor()->nNode.GetNode().IsTextNode() )
    {
        SwTextNode& rAnchorTextNode =
            dynamic_cast<SwTextNode&>(_rFrameFormat.GetAnchor().GetContentAnchor()->nNode.GetNode());
        const SvxFormatBreakItem& rBreak = ItemGet<SvxFormatBreakItem>(rAnchorTextNode, RES_BREAK);
        if (rBreak.GetBreak() == SvxBreak::ColumnBefore)
        {
            bConvDueToAnchoredAtColBreakPara = true;
        }
    }

    sw::WW8AnchorConv eHoriConv(sw::WW8AnchorConv::NO_CONV);
    sw::WW8AnchorConv eVertConv(sw::WW8AnchorConv::NO_CONV);
    // convert horizontal position, if needed
    {

        // determine, if conversion has to be performed due to the position orientation
        bool bConvDueToOrientation( false );
        {
            const sal_Int16 eHOri = _iorHoriOri.GetHoriOrient();
            bConvDueToOrientation = eHOri == text::HoriOrientation::LEFT || eHOri == text::HoriOrientation::RIGHT ||
                                    eHOri == text::HoriOrientation::INSIDE || eHOri == text::HoriOrientation::OUTSIDE ||
                                    ( eHOri != text::HoriOrientation::CENTER && _iorHoriOri.IsPosToggle() );
        }

        // determine conversion type due to the position relation
        if ( bConvDueToAnchoredAtColBreakPara )
        {
            eHoriConv = sw::WW8AnchorConv::CONV2PG;
        }
        else if ( _iorHoriOri.IsPosToggle()
                && _iorHoriOri.GetHoriOrient() == text::HoriOrientation::RIGHT )
        {
            eHoriConv = sw::WW8AnchorConv::NO_CONV;
            _iorHoriOri.SetHoriOrient( text::HoriOrientation::OUTSIDE );
        }
        else
        {
            switch ( _iorHoriOri.GetRelationOrient() )
            {
                case text::RelOrientation::PAGE_FRAME:
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    if ( bConvDueToOrientation || bFollowTextFlow )
                        eHoriConv = sw::WW8AnchorConv::CONV2PG;
                }
                break;
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_RIGHT:
                {
                    // relation not supported by WW8. Thus, conversion always needed.
                    eHoriConv = sw::WW8AnchorConv::CONV2PG;
                }
                break;
                case text::RelOrientation::FRAME:
                {
                    if ( bConvDueToOrientation )
                        eHoriConv = sw::WW8AnchorConv::CONV2COL_OR_PARA;
                }
                break;
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::FRAME_LEFT:
                case text::RelOrientation::FRAME_RIGHT:
                {
                    // relation not supported by WW8. Thus, conversion always needed.
                    eHoriConv = sw::WW8AnchorConv::CONV2COL_OR_PARA;
                }
                break;
                case text::RelOrientation::CHAR:
                {
                    if ( bConvDueToOrientation )
                        eHoriConv = sw::WW8AnchorConv::CONV2CHAR;
                }
                break;
                default:
                    OSL_FAIL( "<WinwordAnchoring::ConvertPosition(..)> - unknown horizontal relation" );
            }
        }
    }

    // convert vertical position, if needed
    {

        // determine, if conversion has to be performed due to the position orientation
        bool bConvDueToOrientation( false );
        {
            const sal_Int16 eVOri = _iorVertOri.GetVertOrient();
            bConvDueToOrientation = ( eVOri == text::VertOrientation::TOP ||
                                      eVOri == text::VertOrientation::BOTTOM ||
                                      eVOri == text::VertOrientation::CHAR_TOP ||
                                      eVOri == text::VertOrientation::CHAR_BOTTOM ||
                                      eVOri == text::VertOrientation::CHAR_CENTER ||
                                      eVOri == text::VertOrientation::LINE_TOP ||
                                      eVOri == text::VertOrientation::LINE_BOTTOM ||
                                      eVOri == text::VertOrientation::LINE_CENTER );
        }

        // determine conversion type due to the position relation
        if ( bConvDueToAnchoredAtColBreakPara )
        {
            eVertConv = sw::WW8AnchorConv::CONV2PG;
        }
        else
        {
            switch ( _iorVertOri.GetRelationOrient() )
            {
                case text::RelOrientation::PAGE_FRAME:
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    if ( bConvDueToOrientation || bFollowTextFlow )
                        eVertConv = sw::WW8AnchorConv::CONV2PG;
                }
                break;
                case text::RelOrientation::FRAME:
                {
                    if ( bConvDueToOrientation ||
                         _iorVertOri.GetVertOrient() == text::VertOrientation::CENTER )
                    {
                        eVertConv = sw::WW8AnchorConv::CONV2COL_OR_PARA;
                    }
                }
                break;
                case text::RelOrientation::PRINT_AREA:
                {
                    // relation not supported by WW8. Thus, conversion always needed.
                    eVertConv = sw::WW8AnchorConv::CONV2COL_OR_PARA;
                }
                break;
                case text::RelOrientation::CHAR:
                {
                    // relation not supported by WW8. Thus, conversion always needed.
                    eVertConv = sw::WW8AnchorConv::CONV2COL_OR_PARA;
                }
                break;
                case text::RelOrientation::TEXT_LINE:
                {
                    if ( bConvDueToOrientation ||
                         _iorVertOri.GetVertOrient() == text::VertOrientation::NONE )
                    {
                        eVertConv = sw::WW8AnchorConv::CONV2LINE;
                    }
                }
                break;
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_RIGHT:
                case text::RelOrientation::FRAME_LEFT:
                case text::RelOrientation::FRAME_RIGHT:
                default:
                    OSL_FAIL( "<WinwordAnchoring::ConvertPosition(..)> - unknown vertical relation" );
            }
        }

    }
    if (eVertConv != sw::WW8AnchorConv::NO_CONV || eHoriConv != sw::WW8AnchorConv::NO_CONV)
    {
        sw::WW8AnchorConvResult aResult(eHoriConv, eVertConv);
        _rFrameFormat.CallSwClientNotify(sw::WW8AnchorConvHint(aResult));
        if(!aResult.m_bConverted)
            return false;
        if (eHoriConv != sw::WW8AnchorConv::NO_CONV)
        {
            lcl_SetRelationOrient(_iorHoriOri, eHoriConv, [&_iorHoriOri]() {_iorHoriOri.SetHoriOrient(text::HoriOrientation::NONE);} );
            _iorHoriOri.SetPos(aResult.m_aPos.X());
        }
        if (eVertConv != sw::WW8AnchorConv::NO_CONV)
        {
            lcl_SetRelationOrient(_iorVertOri, eVertConv, [&_iorVertOri]() {_iorVertOri.SetVertOrient(text::VertOrientation::NONE);} );
            _iorVertOri.SetPos(aResult.m_aPos.Y());
        }
        return true;
    }
    return false;
}

void WinwordAnchoring::SetAnchoring(const SwFrameFormat& rFormat)
{
    const RndStdIds eAnchor = rFormat.GetAnchor().GetAnchorId();
    mbInline = (eAnchor == RndStdIds::FLY_AS_CHAR);

    SwFormatHoriOrient rHoriOri = rFormat.GetHoriOrient();
    SwFormatVertOrient rVertOri = rFormat.GetVertOrient();

    // #i30669# - convert the positioning attributes.
    // Most positions are converted, if layout information exists.
    const bool bPosConverted = ConvertPosition( rHoriOri, rVertOri, rFormat );

    const sal_Int16 eHOri = rHoriOri.GetHoriOrient();
    const sal_Int16 eVOri = rVertOri.GetVertOrient(); // #i22673#

    const sal_Int16 eHRel = rHoriOri.GetRelationOrient();
    const sal_Int16 eVRel = rVertOri.GetRelationOrient();

    // horizontal Adjustment
    switch (eHOri)
    {
        default:
        case text::HoriOrientation::NONE:
            mnXAlign = 0;
            break;
        case text::HoriOrientation::LEFT:
            mnXAlign = 1;
            break;
        case text::HoriOrientation::CENTER:
            mnXAlign = 2;
            break;
        case text::HoriOrientation::RIGHT:
            mnXAlign = 3;
            break;
        case text::HoriOrientation::INSIDE:
            mnXAlign = 4;
            break;
        case text::HoriOrientation::OUTSIDE:
            mnXAlign = 5;
            break;
    }

    // vertical Adjustment
    // #i22673#
    // When adjustment is vertically relative to line or to char
    // bottom becomes top and vice versa
    const bool bVertSwap = !bPosConverted &&
                           ( (eVRel == text::RelOrientation::CHAR) ||
                             (eVRel == text::RelOrientation::TEXT_LINE) );
    switch (eVOri)
    {
        default:
        case text::VertOrientation::NONE:
            mnYAlign = 0;
            break;
        case text::VertOrientation::TOP:
        case text::VertOrientation::LINE_TOP:
        case text::VertOrientation::CHAR_TOP:
            mnYAlign = bVertSwap ? 3 : 1;
            break;
        case text::VertOrientation::CENTER:
        case text::VertOrientation::LINE_CENTER:
            mnYAlign = 2;
            break;
        case text::VertOrientation::BOTTOM:
        case text::VertOrientation::LINE_BOTTOM:
        case text::VertOrientation::CHAR_BOTTOM:
            mnYAlign = bVertSwap ? 1 : 3;
            break;
    }

    // Adjustment is horizontally relative to...
    switch (eHRel)
    {
        case text::RelOrientation::PAGE_PRINT_AREA:
            mnXRelTo = 0;
            break;
        case text::RelOrientation::PAGE_FRAME:
        case text::RelOrientation::PAGE_LEFT:  //:-(
        case text::RelOrientation::PAGE_RIGHT: //:-(
            mnXRelTo = 1;
            break;
        case text::RelOrientation::FRAME:
        case text::RelOrientation::FRAME_LEFT: //:-(
        case text::RelOrientation::FRAME_RIGHT: //:-(
            if (eAnchor == RndStdIds::FLY_AT_PAGE)
                mnXRelTo = 1;
            else
                mnXRelTo = 2;
            break;
        case text::RelOrientation::PRINT_AREA:
            if (eAnchor == RndStdIds::FLY_AT_PAGE)
                mnXRelTo = 0;
            else
                mnXRelTo = 2;
            break;
        case text::RelOrientation::CHAR:
            mnXRelTo = 3;
            break;
        case text::RelOrientation::TEXT_LINE:
            break;
    }

        // Adjustment is vertically relative to...
    switch (eVRel)
    {
        case text::RelOrientation::PAGE_PRINT_AREA:
            mnYRelTo = 0;
            break;
        case text::RelOrientation::PAGE_FRAME:
            mnYRelTo = 1;
            break;
        case text::RelOrientation::PRINT_AREA:
            if (eAnchor == RndStdIds::FLY_AT_PAGE)
                mnYRelTo = 0;
            else
                mnYRelTo = 2;
            break;
        case text::RelOrientation::FRAME:
            if (eAnchor == RndStdIds::FLY_AT_PAGE)
                mnYRelTo = 1;
            else
                mnYRelTo = 2;
            break;
        case text::RelOrientation::CHAR:
        case text::RelOrientation::TEXT_LINE: // #i22673# - vertical alignment at top of line
        case text::RelOrientation::PAGE_LEFT:   //nonsense
        case text::RelOrientation::PAGE_RIGHT:  //nonsense
        case text::RelOrientation::FRAME_LEFT:  //nonsense
        case text::RelOrientation::FRAME_RIGHT: //nonsense
            mnYRelTo = 3;
            break;
    }
}

void SwEscherEx::WriteFrameExtraData( const SwFrameFormat& rFormat )
{
    aWinwordAnchoring.SetAnchoring(rFormat);
    aWinwordAnchoring.WriteData(*this);

    AddAtom(4, ESCHER_ClientAnchor);
    GetStream().WriteInt32( 0 );

    AddAtom(4, ESCHER_ClientData);
    GetStream().WriteInt32( 1 );
}

sal_Int32 SwEscherEx::WriteFlyFrame(const DrawObj &rObj, sal_uInt32 &rShapeId,
    DrawObjPointerVector &rPVec)
{
    const SwFrameFormat &rFormat = rObj.maContent.GetFrameFormat();

    // check for textflyframe and if it is the first in a Chain
    sal_Int32 nBorderThick = 0;
    const SwNodeIndex* pNdIdx = rFormat.GetContent().GetContentIdx();
    if( pNdIdx )
    {
        SwNodeIndex aIdx( *pNdIdx, 1 );
        switch( aIdx.GetNode().GetNodeType() )
        {
        case SwNodeType::Grf:
            rShapeId = GenerateShapeId();
            nBorderThick = WriteGrfFlyFrame( rFormat, rShapeId );
            break;
        case SwNodeType::Ole:
            rShapeId = GenerateShapeId();
            nBorderThick = WriteOLEFlyFrame( rFormat, rShapeId );
            break;
        default:
            if (const SdrObject* pObj = rFormat.FindRealSdrObject())
            {
                // check for the first in a Chain
                sal_uInt32 nTextId;
                sal_uInt16 nOff = 0;
                const SwFrameFormat* pFormat = &rFormat, *pPrev;
                while( nullptr != ( pPrev = pFormat->GetChain().GetPrev() ))
                {
                    ++nOff;
                    pFormat = pPrev;
                }

                rShapeId = GetFlyShapeId(rFormat, rObj.mnHdFtIndex, rPVec);
                if( !nOff )
                {
                    nTextId = pTextBxs->GetPos( pObj );
                    if( USHRT_MAX == nTextId )
                    {
                        pTextBxs->Append( *pObj, rShapeId );
                        nTextId = pTextBxs->Count();
                    }
                    else
                        ++nTextId;
                }
                else
                {
                    const SdrObject* pPrevObj = pFormat->FindRealSdrObject();
                    nTextId = pTextBxs->GetPos( pPrevObj );
                    if( USHRT_MAX == nTextId )
                    {
                        sal_uInt32 nPrevShapeId =
                            GetFlyShapeId(*pFormat, rObj.mnHdFtIndex, rPVec);
                        pTextBxs->Append( *pPrevObj, nPrevShapeId );
                        nTextId = pTextBxs->Count();
                    }
                    else
                        ++nTextId;
                }
                nTextId *= 0x10000;
                nTextId += nOff;

                nBorderThick = WriteTextFlyFrame(rObj, rShapeId, nTextId, rPVec);
            }

            //In browse mode the sdr object doesn't always exist. For example, the
            //object is in the hidden header/footer. We save the fmt directly
            //in such cases; we copy most of the logic from the block above
            const bool bBrowseMode = rFormat.getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE);
            if( bBrowseMode && rFormat.GetDoc())
            {
                if( !rFormat.GetChain().GetPrev() )//obj in header/footer?
                {
                    rShapeId = GetFlyShapeId(rFormat, rObj.mnHdFtIndex, rPVec);
                    pTextBxs->Append( &rFormat, rShapeId );
                    sal_uInt32 nTextId = pTextBxs->Count();

                    nTextId *= 0x10000;
                    nBorderThick = WriteTextFlyFrame(rObj, rShapeId, nTextId, rPVec);
                }
            }

        }
    }
    return nBorderThick;
}

static sal_uInt16 FindPos(const SwFrameFormat &rFormat, unsigned int nHdFtIndex,
    DrawObjPointerVector &rPVec)
{
    auto aIter = std::find_if(rPVec.begin(), rPVec.end(),
        [&rFormat, nHdFtIndex](const DrawObj* pObj) {
            OSL_ENSURE(pObj, "Impossible");
            return pObj &&
                nHdFtIndex == pObj->mnHdFtIndex &&
                &rFormat == (&pObj->maContent.GetFrameFormat());
        });
    if (aIter != rPVec.end())
        return static_cast< sal_uInt16 >(aIter - rPVec.begin());
    return USHRT_MAX;
}

sal_Int32 SwEscherEx::WriteTextFlyFrame(const DrawObj &rObj, sal_uInt32 nShapeId,
    sal_uInt32 nTextBox, DrawObjPointerVector &rPVec)
{
    const SwFrameFormat &rFormat = rObj.maContent.GetFrameFormat();
    SvxFrameDirection nDirection = rObj.mnDirection;

    sal_Int32 nBorderThick=0;
    OpenContainer( ESCHER_SpContainer );

    AddShape( ESCHER_ShpInst_TextBox, ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty, nShapeId );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt(ESCHER_Prop_lTxid, nTextBox);
    if (const SwFrameFormat *pNext = rFormat.GetChain().GetNext())
    {
        sal_uInt16 nPos = FindPos(*pNext, rObj.mnHdFtIndex, rPVec);
        if (USHRT_MAX != nPos && aFollowShpIds[nPos])
            aPropOpt.AddOpt(ESCHER_Prop_hspNext, aFollowShpIds[nPos]);
    }
    nBorderThick = WriteFlyFrameAttr( rFormat, mso_sptTextBox, aPropOpt );

    MSO_TextFlow nFlow;

    switch (nDirection)
    {
        default:
            OSL_ENSURE(false, "unknown direction type");
            [[fallthrough]];
        case SvxFrameDirection::Horizontal_LR_TB:
            nFlow=mso_txflHorzN;
        break;
        case SvxFrameDirection::Horizontal_RL_TB:
            nFlow=mso_txflHorzN;
        break;
        case SvxFrameDirection::Vertical_LR_TB: //not really possible in word
        case SvxFrameDirection::Vertical_RL_TB:
            nFlow=mso_txflTtoBA;
        break;
        case SvxFrameDirection::Vertical_LR_BT:
            nFlow = mso_txflBtoT;
        break;
    }
    aPropOpt.AddOpt( ESCHER_Prop_txflTextFlow, nFlow );

    aPropOpt.Commit( GetStream() );

    // store anchor attribute
    WriteFrameExtraData( rFormat );

    AddAtom( 4, ESCHER_ClientTextbox ); GetStream().WriteUInt32( nTextBox );

    CloseContainer();   // ESCHER_SpContainer
    return nBorderThick;
}

void SwBasicEscherEx::WriteOLEPicture(EscherPropertyContainer &rPropOpt,
    ShapeFlag nShapeFlags, const Graphic &rGraphic, const SdrObject &rObj,
    sal_uInt32 nShapeId, const awt::Rectangle* pVisArea )
{
    //nShapeFlags == 0xA00 + flips and ole active
    AddShape(ESCHER_ShpInst_PictureFrame, nShapeFlags, nShapeId);

    GraphicObject aGraphicObject(rGraphic);
    OString aId = aGraphicObject.GetUniqueID();
    if (!aId.isEmpty())
    {
        // SJ: the third parameter (pVisArea) should be set...
        sal_uInt32 nBlibId = mxGlobal->GetBlibID( *QueryPictureStream(), aGraphicObject, pVisArea);
        if (nBlibId)
            rPropOpt.AddOpt(ESCHER_Prop_pib, nBlibId, true);
    }

    SetPicId(rObj, nShapeId, rPropOpt);
    rPropOpt.AddOpt( ESCHER_Prop_pictureActive, 0x10000 );
}

void SwEscherEx::WriteOCXControl( const SwFrameFormat& rFormat, sal_uInt32 nShapeId )
{
    const SdrObject* pSdrObj = rFormat.FindRealSdrObject();
    if (!pSdrObj)
        return;

    OpenContainer( ESCHER_SpContainer );

    SwDrawModel *pModel = rWrt.m_rDoc.getIDocumentDrawModelAccess().GetDrawModel();
    OutputDevice *pDevice = Application::GetDefaultDevice();
    OSL_ENSURE(pModel && pDevice, "no model or device");

    // #i71538# use complete SdrViews
    // SdrExchangeView aExchange(pModel, pDevice);
    SdrView aExchange(*pModel, pDevice);
    const Graphic aGraphic(SdrExchangeView::GetObjGraphic(*pSdrObj));
    EscherPropertyContainer aPropOpt;
    WriteOLEPicture(aPropOpt,
        ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty | ShapeFlag::OLEShape, aGraphic,
        *pSdrObj, nShapeId, nullptr );

    WriteFlyFrameAttr( rFormat, mso_sptPictureFrame , aPropOpt );
    aPropOpt.Commit( GetStream() );

    // store anchor attribute
    WriteFrameExtraData( rFormat );

    CloseContainer();   // ESCHER_SpContainer

}

void SwEscherEx::MakeZOrderArrAndFollowIds(
    std::vector<DrawObj>& rSrcArr, DrawObjPointerVector&rDstArr)
{
    ::lcl_makeZOrderArray(rWrt, rSrcArr, rDstArr);

    //Now set up the follow IDs
    aFollowShpIds.clear();

    for (DrawObj* p : rDstArr)
    {
        const SwFrameFormat &rFormat = p->maContent.GetFrameFormat();
        bool bNeedsShapeId = false;

        if (RES_FLYFRMFMT == rFormat.Which())
        {
            const SwFormatChain &rChain = rFormat.GetChain();
            if (rChain.GetPrev() || rChain.GetNext())
                bNeedsShapeId = true;
        }

        sal_uLong nShapeId = bNeedsShapeId ? GenerateShapeId() : 0;

        aFollowShpIds.push_back(nShapeId);
    }
}

sal_uInt32 SwEscherEx::GetFlyShapeId(const SwFrameFormat& rFormat,
    unsigned int nHdFtIndex, DrawObjPointerVector &rpVec)
{
    sal_uInt16 nPos = FindPos(rFormat, nHdFtIndex, rpVec);
    sal_uInt32 nShapeId;
    if (USHRT_MAX != nPos)
    {
        nShapeId = aFollowShpIds[nPos];
        if (0 == nShapeId)
        {
            nShapeId = GenerateShapeId();
            aFollowShpIds[ nPos ] = nShapeId;
        }
    }
    else
        nShapeId = GenerateShapeId();
    return nShapeId;
}

sal_uInt32 SwEscherEx::QueryTextID(
    const uno::Reference< drawing::XShape>& xXShapeRef, sal_uInt32 nShapeId )
{
    sal_uInt32 nId = 0;
    if (SdrObject* pObj = SdrObject::getSdrObjectFromXShape(xXShapeRef))
    {
        pTextBxs->Append( *pObj, nShapeId );
        nId = pTextBxs->Count();
        nId *= 0x10000;
    }
    return nId;
}

SwMSConvertControls::SwMSConvertControls( SfxObjectShell const *pDSh, SwPaM *pP ) : oox
::ole::MSConvertOCXControls(  pDSh ? pDSh->GetModel() : nullptr ), pPaM( pP ), mnObjectId(0)
{
}


// in transitioning away old filter for ole/ocx controls, ReadOCXStream has been made pure virtual in
// filter/source/msocximex.cxx, so... we need an implementation here
bool  SwMSConvertControls::ReadOCXStream( tools::SvRef<SotStorage> const & rSrc1,
        css::uno::Reference< css::drawing::XShape > *pShapeRef,
        bool bFloatingCtrl )
{
    uno::Reference< form::XFormComponent > xFComp;
    bool bRes = oox::ole::MSConvertOCXControls::ReadOCXStorage( rSrc1, xFComp );
    if ( bRes && xFComp.is() )
    {
        css::awt::Size aSz;  // not used in import
        bRes = InsertControl( xFComp, aSz,pShapeRef,bFloatingCtrl);
    }
    return bRes;
}

void SwMSConvertControls::ExportControl(WW8Export &rWW8Wrt, const SdrUnoObj& rFormObj)
{
    const uno::Reference< awt::XControlModel >& xControlModel =
        rFormObj.GetUnoControlModel();

    //Why oh lord do we use so many different units ?
    //I think I painted myself into a little bit of a
    //corner by trying to use the uno interface for
    //controls export
    tools::Rectangle aRect = rFormObj.GetLogicRect();
    aRect.SetPos(Point(0,0));
    awt::Size aSize;
    aSize.Width = convertTwipToMm100(aRect.Right());
    aSize.Height = convertTwipToMm100(aRect.Bottom());

    //Open the ObjectPool
    tools::SvRef<SotStorage> xObjPool = rWW8Wrt.GetWriter().GetStorage().OpenSotStorage(SL::aObjectPool);

    //Create a destination storage for the microsoft control
    sal_uInt32 nObjId = ++mnObjectId;
    OUString sStorageName = "_" + OUString::number( static_cast<sal_Int64>( nObjId ));
    tools::SvRef<SotStorage> xOleStg = xObjPool->OpenSotStorage(sStorageName);

    if (!xOleStg.is())
        return;

    OUString sUName;
    if (!WriteOCXStream( mxModel, xOleStg,xControlModel,aSize,sUName))
        return;

    sal_uInt8 aSpecOLE[] =
    {
        0x03, 0x6a, 0xFF, 0xFF, 0xFF, 0xFF, // sprmCPicLocation
        0x0a, 0x08, 1,                  // sprmCFOLE2
        0x55, 0x08, 1,                  // sprmCFSpec
        0x56, 0x08, 1                   // sprmCFObj
    };
    //Set the obj id into the sprmCPicLocation
    sal_uInt8 *pData = aSpecOLE+2;
    Set_UInt32(pData,nObjId );

    OUString sField = FieldString(ww::eCONTROL) + "Forms." + sUName + ".1 \\s ";

    rWW8Wrt.OutputField(nullptr, ww::eCONTROL, sField,
        FieldFlags::Start|FieldFlags::CmdStart|FieldFlags::CmdEnd);

    rWW8Wrt.m_pChpPlc->AppendFkpEntry(rWW8Wrt.Strm().Tell(),sizeof(aSpecOLE),
        aSpecOLE);
    rWW8Wrt.WriteChar( 0x1 );
    rWW8Wrt.OutputField(nullptr, ww::eCONTROL, OUString(), FieldFlags::End | FieldFlags::Close);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
