/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "htmlreqifreader.hxx"

#include <comphelper/scopeguard.hxx>
#include <filter/msfilter/rtfutil.hxx>
#include <rtl/strbuf.hxx>
#include <sot/storage.hxx>
#include <svtools/parrtf.hxx>
#include <svtools/rtfkeywd.hxx>
#include <svtools/rtftoken.h>
#include <tools/stream.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include <vcl/cvtgrf.hxx>
#include <ndole.hxx>

namespace
{
/// RTF parser that just extracts a single OLE2 object from a file.
class ReqIfRtfReader : public SvRTFParser
{
public:
    ReqIfRtfReader(SvStream& rStream);
    void NextToken(int nToken) override;
    bool WriteObjectData(SvStream& rOLE);

private:
    bool m_bInObjData = false;
    OStringBuffer m_aHex;
};

ReqIfRtfReader::ReqIfRtfReader(SvStream& rStream)
    : SvRTFParser(rStream)
{
}

void ReqIfRtfReader::NextToken(int nToken)
{
    switch (nToken)
    {
        case '}':
            m_bInObjData = false;
            break;
        case RTF_TEXTTOKEN:
            if (m_bInObjData)
                m_aHex.append(OUStringToOString(aToken, RTL_TEXTENCODING_ASCII_US));
            break;
        case RTF_OBJDATA:
            m_bInObjData = true;
            break;
    }
}

bool ReqIfRtfReader::WriteObjectData(SvStream& rOLE)
{
    return msfilter::rtfutil::ExtractOLE2FromObjdata(m_aHex.makeStringAndClear(), rOLE);
}

/// Looks up what OLE1 calls the ClassName, see [MS-OLEDS] 2.3.8 CompObjStream.
OString ExtractOLEClassName(const tools::SvRef<SotStorage>& xStorage)
{
    OString aRet;

    SotStorageStream* pCompObj = xStorage->OpenSotStream("\1CompObj");
    if (!pCompObj)
        return aRet;

    pCompObj->Seek(0);
    pCompObj->SeekRel(28); // Header
    if (!pCompObj->good())
        return aRet;

    sal_uInt32 nData;
    pCompObj->ReadUInt32(nData); // AnsiUserType
    pCompObj->SeekRel(nData);
    if (!pCompObj->good())
        return aRet;

    pCompObj->ReadUInt32(nData); // AnsiClipboardFormat
    pCompObj->SeekRel(nData);
    if (!pCompObj->good())
        return aRet;

    pCompObj->ReadUInt32(nData); // Reserved1
    return read_uInt8s_ToOString(*pCompObj, nData);
}

/// Parses the presentation stream of an OLE2 storage.
bool ParseOLE2Presentation(SvStream& rOle2, sal_uInt32& nWidth, sal_uInt32& nHeight,
                           SvStream& rPresentationData)
{
    // See [MS-OLEDS] 2.3.4, OLEPresentationStream
    rOle2.Seek(0);
    tools::SvRef<SotStorage> pStorage = new SotStorage(rOle2);
    tools::SvRef<SotStorageStream> xOle2Presentation
        = pStorage->OpenSotStream("\002OlePres000", StreamMode::STD_READ);

    // Read AnsiClipboardFormat.
    sal_uInt32 nMarkerOrLength = 0;
    xOle2Presentation->ReadUInt32(nMarkerOrLength);
    if (nMarkerOrLength != 0xffffffff)
        // FormatOrAnsiString is not present
        return false;
    sal_uInt32 nFormatOrAnsiLength = 0;
    xOle2Presentation->ReadUInt32(nFormatOrAnsiLength);
    if (nFormatOrAnsiLength != 0x00000003) // CF_METAFILEPICT
        return false;

    // Read TargetDeviceSize.
    sal_uInt32 nTargetDeviceSize = 0;
    xOle2Presentation->ReadUInt32(nTargetDeviceSize);
    if (nTargetDeviceSize != 0x00000004)
        // TargetDevice is present
        return false;

    sal_uInt32 nAspect = 0;
    xOle2Presentation->ReadUInt32(nAspect);
    sal_uInt32 nLindex = 0;
    xOle2Presentation->ReadUInt32(nLindex);
    sal_uInt32 nAdvf = 0;
    xOle2Presentation->ReadUInt32(nAdvf);
    sal_uInt32 nReserved1 = 0;
    xOle2Presentation->ReadUInt32(nReserved1);
    xOle2Presentation->ReadUInt32(nWidth);
    xOle2Presentation->ReadUInt32(nHeight);
    sal_uInt32 nSize = 0;
    xOle2Presentation->ReadUInt32(nSize);

    // Read Data.
    if (nSize > xOle2Presentation->remainingSize())
        return false;
    std::vector<char> aBuffer(nSize);
    xOle2Presentation->ReadBytes(aBuffer.data(), aBuffer.size());
    rPresentationData.WriteBytes(aBuffer.data(), aBuffer.size());

    return true;
}

/// Inserts an OLE1 header before an OLE2 storage.
OString InsertOLE1Header(SvStream& rOle2, SvStream& rOle1, sal_uInt32& nWidth, sal_uInt32& nHeight)
{
    rOle2.Seek(0);
    tools::SvRef<SotStorage> xStorage(new SotStorage(rOle2));
    if (xStorage->GetError() != ERRCODE_NONE)
        return OString();

    OString aClassName = ExtractOLEClassName(xStorage);

    // Write ObjectHeader, see [MS-OLEDS] 2.2.4.
    rOle1.Seek(0);
    // OLEVersion.
    rOle1.WriteUInt32(0x00000501);

    // FormatID is EmbeddedObject.
    rOle1.WriteUInt32(0x00000002);

    // ClassName
    rOle1.WriteUInt32(aClassName.getLength());
    if (!aClassName.isEmpty())
    {
        rOle1.WriteOString(aClassName);
        // Null terminated pascal string.
        rOle1.WriteChar(0);
    }

    // TopicName.
    rOle1.WriteUInt32(0);

    // ItemName.
    rOle1.WriteUInt32(0);

    // NativeDataSize
    rOle1.WriteUInt32(rOle2.TellEnd());

    // Write the actual native data.
    rOle2.Seek(0);
    rOle1.WriteStream(rOle2);

    // Write Presentation.
    SvMemoryStream aPresentationData;
    if (ParseOLE2Presentation(rOle2, nWidth, nHeight, aPresentationData))
    {
        // OLEVersion.
        rOle1.WriteUInt32(0x00000501);
        // FormatID: constant means the ClassName field is present.
        rOle1.WriteUInt32(0x00000005);
        // ClassName: null terminated pascal string.
        OString aPresentationClassName("METAFILEPICT");
        rOle1.WriteUInt32(aPresentationClassName.getLength() + 1);
        rOle1.WriteOString(aPresentationClassName);
        rOle1.WriteChar(0);
        // Width.
        rOle1.WriteUInt32(nWidth);
        // Height.
        rOle1.WriteUInt32(nHeight * -1);
        // PresentationDataSize
        sal_uInt32 nPresentationData = aPresentationData.Tell();
        rOle1.WriteUInt32(8 + nPresentationData);
        // Reserved1-4.
        rOle1.WriteUInt16(0x0008);
        rOle1.WriteUInt16(0x31b1);
        rOle1.WriteUInt16(0x1dd9);
        rOle1.WriteUInt16(0x0000);
        aPresentationData.Seek(0);
        rOle1.WriteStream(aPresentationData, nPresentationData);
    }

    return aClassName;
}

/// Writes rGraphic with size from rOLENode to rRtf as an RTF hexdump.
void WrapOleGraphicInRtf(SvStream& rRtf, const SwOLENode& rOLENode, const Graphic& rGraphic)
{
    // Start result.
    rRtf.WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_RESULT);

    // Start pict.
    rRtf.WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_PICT);

    rRtf.WriteCharPtr(OOO_STRING_SVTOOLS_RTF_WMETAFILE "8");
    Size aSize(rOLENode.GetTwipSize());
    rRtf.WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PICW);
    rRtf.WriteCharPtr(OString::number(aSize.getWidth()).getStr());
    rRtf.WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PICH);
    rRtf.WriteCharPtr(OString::number(aSize.getHeight()).getStr());
    rRtf.WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PICWGOAL);
    rRtf.WriteCharPtr(OString::number(aSize.getWidth()).getStr());
    rRtf.WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PICHGOAL);
    rRtf.WriteCharPtr(OString::number(aSize.getHeight()).getStr());
    SvMemoryStream aGraphicStream;
    if (GraphicConverter::Export(aGraphicStream, rGraphic, ConvertDataFormat::WMF) == ERRCODE_NONE)
    {
        auto pGraphicAry = static_cast<const sal_uInt8*>(aGraphicStream.GetData());
        sal_uInt64 nSize = aGraphicStream.TellEnd();
        msfilter::rtfutil::StripMetafileHeader(pGraphicAry, nSize);
        rRtf.WriteCharPtr(SAL_NEWLINE_STRING);
        msfilter::rtfutil::WriteHex(pGraphicAry, nSize, &rRtf);
    }

    // End pict.
    rRtf.WriteCharPtr("}");

    // End result.
    rRtf.WriteCharPtr("}");
}
}

namespace SwReqIfReader
{
bool ExtractOleFromRtf(SvStream& rRtf, SvStream& rOle, bool& bOwnFormat)
{
    // Add missing header/footer.
    SvMemoryStream aRtf;
    aRtf.WriteOString("{\\rtf1");
    aRtf.WriteStream(rRtf);
    aRtf.WriteOString("}");
    aRtf.Seek(0);

    // Read the RTF markup.
    tools::SvRef<ReqIfRtfReader> xReader(new ReqIfRtfReader(aRtf));
    SvParserState eState = xReader->CallParser();
    if (eState == SvParserState::Error)
        return false;

    // Write the OLE2 data.
    if (!xReader->WriteObjectData(rOle))
        return false;

    tools::SvRef<SotStorage> pStorage = new SotStorage(rOle);
    OUString aFilterName = SvxMSDffManager::GetFilterNameFromClassID(pStorage->GetClassName());
    bOwnFormat = !aFilterName.isEmpty();
    if (!bOwnFormat)
    {
        // Real OLE2 data, we're done.
        rOle.Seek(0);
        return true;
    }

    // ODF-in-OLE2 case, extract actual data.
    SvMemoryStream aMemory;
    SvxMSDffManager::ExtractOwnStream(*pStorage, aMemory);
    rOle.Seek(0);
    aMemory.Seek(0);
    rOle.WriteStream(aMemory);
    // Stream length is current position + 1.
    rOle.SetStreamSize(aMemory.GetSize() + 1);
    rOle.Seek(0);
    return true;
}

bool WrapOleInRtf(SvStream& rOle2, SvStream& rRtf, SwOLENode& rOLENode)
{
    sal_uInt64 nPos = rOle2.Tell();
    comphelper::ScopeGuard g([&rOle2, nPos] { rOle2.Seek(nPos); });

    // Write OLE1 header, then the RTF wrapper.
    SvMemoryStream aOLE1;
    sal_uInt32 nWidth = 0;
    sal_uInt32 nHeight = 0;
    OString aClassName = InsertOLE1Header(rOle2, aOLE1, nWidth, nHeight);

    // Start object.
    rRtf.WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_OBJECT);
    rRtf.WriteCharPtr(OOO_STRING_SVTOOLS_RTF_OBJEMB);

    // Start objclass.
    rRtf.WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_OBJCLASS " ");
    rRtf.WriteOString(aClassName);
    // End objclass.
    rRtf.WriteCharPtr("}");

    // Object size.
    rRtf.WriteCharPtr(OOO_STRING_SVTOOLS_RTF_OBJW);
    rRtf.WriteCharPtr(OString::number(nWidth).getStr());
    rRtf.WriteCharPtr(OOO_STRING_SVTOOLS_RTF_OBJH);
    rRtf.WriteCharPtr(OString::number(nHeight).getStr());

    // Start objdata.
    rRtf.WriteCharPtr(
        "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_OBJDATA SAL_NEWLINE_STRING);
    msfilter::rtfutil::WriteHex(static_cast<const sal_uInt8*>(aOLE1.GetData()), aOLE1.GetSize(),
                                &rRtf);
    // End objdata.
    rRtf.WriteCharPtr("}");

    if (const Graphic* pGraphic = rOLENode.GetGraphic())
        WrapOleGraphicInRtf(rRtf, rOLENode, *pGraphic);

    // End object.
    rRtf.WriteCharPtr("}");

    return true;
}

bool WrapGraphicInRtf(const Graphic& rGraphic, const Size& rLogicSize, SvStream& rRtf)
{
    rRtf.WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_PICT);

    GfxLink aLink = rGraphic.GetGfxLink();
    const sal_uInt8* pGraphicAry = aLink.GetData();
    sal_uInt64 nSize = aLink.GetDataSize();
    OString aBlipType;
    bool bIsWMF = false;
    switch (aLink.GetType())
    {
        case GfxLinkType::NativeBmp:
            aBlipType = OOO_STRING_SVTOOLS_RTF_WBITMAP;
            break;
        case GfxLinkType::NativeJpg:
            aBlipType = OOO_STRING_SVTOOLS_RTF_JPEGBLIP;
            break;
        case GfxLinkType::NativePng:
            aBlipType = OOO_STRING_SVTOOLS_RTF_PNGBLIP;
            break;
        case GfxLinkType::NativeWmf:
            if (aLink.IsEMF())
                aBlipType = OOO_STRING_SVTOOLS_RTF_EMFBLIP;
            else
            {
                aBlipType = OOO_STRING_SVTOOLS_RTF_WMETAFILE;
                bIsWMF = true;
            }
            break;
        default:
            break;
    }

    if (aBlipType.isEmpty())
        return false;

    rRtf.WriteOString(aBlipType);

    Size aMapped(rGraphic.GetPrefSize());
    rRtf.WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PICW);
    rRtf.WriteOString(OString::number(aMapped.Width()));
    rRtf.WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PICH);
    rRtf.WriteOString(OString::number(aMapped.Height()));

    rRtf.WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PICWGOAL);
    rRtf.WriteOString(OString::number(rLogicSize.Width()));
    rRtf.WriteCharPtr(OOO_STRING_SVTOOLS_RTF_PICHGOAL);
    rRtf.WriteOString(OString::number(rLogicSize.Height()));

    if (bIsWMF)
    {
        rRtf.WriteOString(OString::number(8));
        msfilter::rtfutil::StripMetafileHeader(pGraphicAry, nSize);
    }
    rRtf.WriteOString(SAL_NEWLINE_STRING);

    msfilter::rtfutil::WriteHex(pGraphicAry, nSize, &rRtf);
    rRtf.WriteOString(SAL_NEWLINE_STRING);

    // End pict.
    rRtf.WriteCharPtr("}");
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
