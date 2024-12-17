/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <pdf/ResourceDict.hxx>
#include <pdf/COSWriter.hxx>

namespace vcl::pdf
{
namespace
{
void appendResourceMap(OStringBuffer& rBuf, const char* pPrefix,
                       std::map<OString, sal_Int32> const& rList)
{
    if (rList.empty())
        return;
    rBuf.append('/');
    rBuf.append(pPrefix);
    rBuf.append("<<");
    int ni = 0;
    for (auto const& item : rList)
    {
        if (!item.first.isEmpty() && item.second > 0)
        {
            rBuf.append('/');
            rBuf.append(item.first);
            rBuf.append(' ');
            rBuf.append(item.second);
            rBuf.append(" 0 R");
            if (((++ni) & 7) == 0)
                rBuf.append('\n');
        }
    }
    rBuf.append(">>\n");
}
}

void ResourceDict::append(OStringBuffer& rBuf, sal_Int32 nFontDictObject,
                          PDFWriter::PDFVersion eVersion)
{
    COSWriter aWriter(rBuf);
    aWriter.startDict();
    if (nFontDictObject)
        aWriter.writeKeyAndReference("/Font", nFontDictObject);
    appendResourceMap(rBuf, "XObject", m_aXObjects);
    appendResourceMap(rBuf, "ExtGState", m_aExtGStates);
    appendResourceMap(rBuf, "Shading", m_aShadings);
    appendResourceMap(rBuf, "Pattern", m_aPatterns);

    if (eVersion < PDFWriter::PDFVersion::PDF_2_0)
    {
        rBuf.append("/ProcSet");
        rBuf.append("[");
        rBuf.append("/PDF/Text");
        if (!m_aXObjects.empty())
            rBuf.append("/ImageC/ImageI/ImageB");
        rBuf.append("]\n");
    }
    aWriter.endDict();
}

} // end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
