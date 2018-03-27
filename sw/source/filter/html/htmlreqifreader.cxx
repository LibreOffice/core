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
#include <rtl/character.hxx>
#include <rtl/strbuf.hxx>
#include <sot/storage.hxx>
#include <svtools/parrtf.hxx>
#include <svtools/rtfkeywd.hxx>
#include <svtools/rtftoken.h>
#include <tools/stream.hxx>

namespace
{
/// RTF parser that just extracts a single OLE2 object from a file.
class ReqIfRtfReader : public SvRTFParser
{
public:
    ReqIfRtfReader(SvStream& rStream);
    void NextToken(int nToken) override;
    bool WriteObjectData(SvStream& rStream);

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

/// Inserts an OLE1 header before an OLE2 storage.
OString InsertOLE1Header(SvStream& rOle2, SvStream& rOle1)
{
    rOle2.Seek(0);
    tools::SvRef<SotStorage> xStorage(new SotStorage(rOle2));
    if (xStorage->GetError() != ERRCODE_NONE)
        return OString();

    OString aClassName = ExtractOLEClassName(xStorage);

    // Write ObjectHeader, see [MS-OLEDS] 2.2.4.
    rOle1.Seek(0);
    // OLEVersion.
    rOle1.WriteUInt32(0);

    // FormatID is EmbeddedObject.
    rOle1.WriteUInt32(0x00000002);

    // ClassName
    rOle1.WriteUInt32(aClassName.getLength());
    rOle1.WriteOString(aClassName);
    // Null terminated pascal string.
    rOle1.WriteChar(0);

    // TopicName.
    rOle1.WriteUInt32(0);

    // ItemName.
    rOle1.WriteUInt32(0);

    // NativeDataSize
    rOle2.Seek(STREAM_SEEK_TO_END);
    rOle1.WriteUInt32(rOle2.Tell());

    // Write the actual native data.
    rOle2.Seek(0);
    rOle1.WriteStream(rOle2);

    return aClassName;
}
}

namespace SwReqIfReader
{
bool ExtractOleFromRtf(SvStream& rRtf, SvStream& rOle)
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
    return xReader->WriteObjectData(rOle);
}

bool WrapOleInRtf(SvStream& rOle2, SvStream& rRtf)
{
    sal_uInt64 nPos = rOle2.Tell();
    comphelper::ScopeGuard g([&rOle2, nPos] { rOle2.Seek(nPos); });

    // Write OLE1 header, then the RTF wrapper.
    SvMemoryStream aOLE1;
    OString aClassName = InsertOLE1Header(rOle2, aOLE1);

    // Start object.
    rRtf.WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_OBJECT);
    rRtf.WriteCharPtr(OOO_STRING_SVTOOLS_RTF_OBJEMB);

    // Start objclass.
    rRtf.WriteCharPtr("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_OBJCLASS " ");
    rRtf.WriteOString(aClassName);
    // End objclass.
    rRtf.WriteCharPtr("}");

    // Start objdata.
    rRtf.WriteCharPtr(
        "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_OBJDATA SAL_NEWLINE_STRING);
    msfilter::rtfutil::WriteHex(static_cast<const sal_uInt8*>(aOLE1.GetData()), aOLE1.GetSize(),
                                &rRtf);
    // End objdata.
    rRtf.WriteCharPtr("}");
    // End object.
    rRtf.WriteCharPtr("}");

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
