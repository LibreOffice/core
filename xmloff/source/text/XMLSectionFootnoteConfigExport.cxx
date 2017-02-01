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

#include "XMLSectionFootnoteConfigExport.hxx"
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlprmap.hxx>
#include <com/sun/star/style/NumberingType.hpp>
#include <sax/tools/converter.hxx>
#include <xmloff/maptype.hxx>

#include <xmloff/txtprmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnume.hxx>
#include <tools/debug.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <vector>


using namespace ::xmloff::token;

using ::std::vector;
using css::style::NumberingType::ARABIC;


void XMLSectionFootnoteConfigExport::exportXML(
    SvXMLExport& rExport,
    bool bEndnote,
    const vector<XMLPropertyState> *pProperties,
    sal_uInt32 nIdx,
    const rtl::Reference<XMLPropertySetMapper> & rMapper)
{
    // store and initialize the values
    bool bNumOwn = false;
    bool bNumRestart = false;
    sal_Int16 nNumRestartAt = 0;
    sal_Int16 nNumberingType = ARABIC;
    OUString sNumPrefix;
    OUString sNumSuffix;
    bool bEnd = false;

    // find entries in property states vector
    sal_uInt32 nCount = pProperties->size();
    for(sal_uInt32 i = 0; i < nCount; i++)
    {
        const XMLPropertyState& rState = (*pProperties)[i];

        sal_Int16 nContextId = rMapper->GetEntryContextId(rState.mnIndex);
        if (!bEndnote)
        {
            switch (nContextId)
            {
                case CTF_SECTION_FOOTNOTE_NUM_OWN:
                    rState.maValue >>= bNumOwn;
                    break;
                case CTF_SECTION_FOOTNOTE_NUM_RESTART:
                    rState.maValue >>= bNumRestart;
                    break;
                case CTF_SECTION_FOOTNOTE_NUM_RESTART_AT:
                    rState.maValue >>= nNumRestartAt;
                    break;
                case CTF_SECTION_FOOTNOTE_NUM_TYPE:
                    rState.maValue >>= nNumberingType;
                    break;
                case CTF_SECTION_FOOTNOTE_NUM_PREFIX:
                    rState.maValue >>= sNumPrefix;
                    break;
                case CTF_SECTION_FOOTNOTE_NUM_SUFFIX:
                    rState.maValue >>= sNumSuffix;
                    break;
                case CTF_SECTION_FOOTNOTE_END:
                    SAL_WARN_IF( i != nIdx, "xmloff",
                                "received wrong property state index" );
                    rState.maValue >>= bEnd;
                    break;
            }
        }
        else
        {
            switch (nContextId)
            {
                case CTF_SECTION_ENDNOTE_NUM_OWN:
                    rState.maValue >>= bNumOwn;
                    break;
                case CTF_SECTION_ENDNOTE_NUM_RESTART:
                    rState.maValue >>= bNumRestart;
                    break;
                case CTF_SECTION_ENDNOTE_NUM_RESTART_AT:
                    rState.maValue >>= nNumRestartAt;
                    break;
                case CTF_SECTION_ENDNOTE_NUM_TYPE:
                    rState.maValue >>= nNumberingType;
                    break;
                case CTF_SECTION_ENDNOTE_NUM_PREFIX:
                    rState.maValue >>= sNumPrefix;
                    break;
                case CTF_SECTION_ENDNOTE_NUM_SUFFIX:
                    rState.maValue >>= sNumSuffix;
                    break;
                case CTF_SECTION_ENDNOTE_END:
                    SAL_WARN_IF( i != nIdx, "xmloff",
                                "received wrong property state index" );
                    rState.maValue >>= bEnd;
                    break;
            }
        }
    }

    // we only make an element if we have an own footnote/endnote numbering
    if (bEnd)
    {
        rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_NOTE_CLASS,
                                 GetXMLToken( bEndnote ? XML_ENDNOTE
                                                         : XML_FOOTNOTE ) );
        // start numbering
        OUStringBuffer sBuf;
        if (bNumRestart)
        {
            // restart number is stored as 0.., but interpreted as 1..
            rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_START_VALUE,
                                 OUString::number(nNumRestartAt+1));
        }

        if (bNumOwn)
        {
            // prefix and suffix
            if (!sNumPrefix.isEmpty())
            {
                    rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_PREFIX,
                                         sNumPrefix);
            }
            if (!sNumSuffix.isEmpty())
            {
                rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_SUFFIX,
                                     sNumSuffix);
            }

            // number type: num format
            rExport.GetMM100UnitConverter().convertNumFormat( sBuf,
                                                              nNumberingType );
            rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_FORMAT,
                                 sBuf.makeStringAndClear());

            // and letter sync, if applicable
            SvXMLUnitConverter::convertNumLetterSync(
                sBuf, nNumberingType );
            if (!sBuf.isEmpty())
            {
                rExport.AddAttribute(XML_NAMESPACE_STYLE,
                                     XML_NUM_LETTER_SYNC,
                                     sBuf.makeStringAndClear());
            }
        }

        // and finally, the element
        SvXMLElementExport rElem(rExport, XML_NAMESPACE_TEXT,
                                 XML_NOTES_CONFIGURATION,
                                 true, true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
