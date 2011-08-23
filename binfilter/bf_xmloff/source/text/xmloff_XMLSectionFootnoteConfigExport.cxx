/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "XMLSectionFootnoteConfigExport.hxx"

#include "xmlexp.hxx"


#include <com/sun/star/style/NumberingType.hpp>


#include "txtprmap.hxx"


#include "xmlnmspe.hxx"

#include "xmluconv.hxx"



#include <tools/debug.hxx>

#include "rtl/ustring.hxx"


#include <vector>
namespace binfilter {


using namespace ::binfilter::xmloff::token;

using ::std::vector;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::style::NumberingType::ARABIC;


void XMLSectionFootnoteConfigExport::exportXML(
    SvXMLExport& rExport,
    sal_Bool bEndnote,
    const vector<XMLPropertyState> *pProperties,
    sal_uInt32 nIdx,
    const UniReference<XMLPropertySetMapper> & rMapper)
{
    // store and initialize the values
    sal_Bool bNumOwn = sal_False;
    sal_Bool bNumRestart = sal_False;
    sal_Int16 nNumRestartAt = 0;
    sal_Int16 nNumberingType = ARABIC;
    OUString sNumPrefix;
    OUString sNumSuffix;
    sal_Bool bEnd = sal_False;

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
                    DBG_ASSERT( i == nIdx, 
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
                    DBG_ASSERT( i == nIdx, 
                                "received wrong property state index" );
                    rState.maValue >>= bEnd;
                    break;
            }
        }
    }

    // we only make an element if we have an own footnote/endnote numbering
    if (bEnd)
    {
        // start numbering
        OUStringBuffer sBuf;
        if (bNumRestart)
        {
            // restart number is stored as 0.., but interpreted as 1..
            SvXMLUnitConverter::convertNumber(sBuf, 
                                              (sal_Int32)(nNumRestartAt+1));
            rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_START_VALUE, 
                                 sBuf.makeStringAndClear());
        }

        if (bNumOwn)
        {
            // prefix and suffix
            if (sNumPrefix.getLength() > 0)
            {
                    rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_PREFIX,
                                         sNumPrefix);
            }
            if (sNumSuffix.getLength() > 0)
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
            rExport.GetMM100UnitConverter().convertNumLetterSync( 
                sBuf, nNumberingType );
            if (sBuf.getLength()) 
            {
                rExport.AddAttribute(XML_NAMESPACE_STYLE, 
                                     XML_NUM_LETTER_SYNC,
                                     sBuf.makeStringAndClear());
            }
        }

        // and finally, the element
        SvXMLElementExport rElem(rExport, XML_NAMESPACE_TEXT, 
                                 (bEndnote ? XML_ENDNOTES_CONFIGURATION :
                                             XML_FOOTNOTES_CONFIGURATION ), 
                                 sal_True, sal_True);
    }
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
