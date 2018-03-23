/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "htmlreqifreader.hxx"

#include <rtl/character.hxx>
#include <rtl/strbuf.hxx>
#include <svtools/parrtf.hxx>
#include <svtools/rtftoken.h>
#include <tools/stream.hxx>

namespace
{
int AsHex(char ch)
{
    int ret = 0;
    if (rtl::isAsciiDigit(static_cast<unsigned char>(ch)))
        ret = ch - '0';
    else
    {
        if (ch >= 'a' && ch <= 'f')
            ret = ch - 'a';
        else if (ch >= 'A' && ch <= 'F')
            ret = ch - 'A';
        else
            return -1;
        ret += 10;
    }
    return ret;
}

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
    int b = 0, count = 2;

    SvMemoryStream aBuf;
    for (int i = 0; i < m_aHex.getLength(); ++i)
    {
        char ch = m_aHex[i];
        if (ch != 0x0d && ch != 0x0a)
        {
            b = b << 4;
            sal_Int8 parsed = AsHex(ch);
            if (parsed == -1)
                return false;
            b += parsed;
            count--;
            if (!count)
            {
                aBuf.WriteChar(b);
                count = 2;
                b = 0;
            }
        }
    }

    // Skip ObjectHeader, see [MS-OLEDS] 2.2.4.
    if (aBuf.Tell())
    {
        aBuf.Seek(0);
        sal_uInt32 nData;
        aBuf.ReadUInt32(nData); // OLEVersion
        aBuf.ReadUInt32(nData); // FormatID
        aBuf.ReadUInt32(nData); // ClassName
        aBuf.SeekRel(nData);
        aBuf.ReadUInt32(nData); // TopicName
        aBuf.SeekRel(nData);
        aBuf.ReadUInt32(nData); // ItemName
        aBuf.SeekRel(nData);
        aBuf.ReadUInt32(nData); // NativeDataSize

        if (nData)
        {
            rOLE.WriteStream(aBuf);
            rOLE.Seek(0);
        }
    }

    return true;
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
