/*************************************************************************
 *
 *  $RCSfile: XMLSectionFootnoteConfigExport.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dvo $ $Date: 2001-06-29 21:07:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLSECTIONFOOTNOTECONFIGEXPORT_HXX
#include "XMLSectionFootnoteConfigExport.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif

#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include "maptype.hxx"
#endif

#ifndef _XMLOFF_TXTPRMAP_HXX
#include "txtprmap.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLNUME_HXX
#include "xmlnume.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRING
#include <rtl/ustring>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <vector>


using namespace ::xmloff::token;

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
