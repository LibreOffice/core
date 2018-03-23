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
#include <filter/msfilter/rtfutil.hxx>

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
