/*************************************************************************
 *
 *  $RCSfile: XMLSectionFootnoteConfigExport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dvo $ $Date: 2001-02-19 11:00:25 $
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

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
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

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
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


using ::std::vector;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;


void XMLSectionFootnoteConfigExport::exportXML(
    const SvXMLExport& _rExport,
    sal_Bool bEndnote,
    const vector<XMLPropertyState> *pProperties,
    sal_uInt32 nIdx,
    const UniReference<XMLPropertySetMapper> & rMapper)
{
    // HACK: DO NOT COMMIT
    // HACK: DO NOT COMMIT
    // HACK: DO NOT COMMIT
    // HACK: DO NOT COMMIT
    // HACK: DO NOT COMMIT
    // HACK: DO NOT COMMIT
    // HACK: DO NOT COMMIT
    SvXMLExport& rExport = (SvXMLExport&)_rExport;


#ifndef PRODUCT
    // check whether the properties are in the property state vector as
    // expected. If any of these assertions fail, someone messed up the
    // propertymap in txtprmap.cxx

    static sal_Int16 aFootnoteIds[] =
    {
        CTF_SECTION_FOOTNOTE_NUM_OWN,
        CTF_SECTION_FOOTNOTE_NUM_RESTART,
        CTF_SECTION_FOOTNOTE_NUM_RESTART_AT,
        CTF_SECTION_FOOTNOTE_NUM_TYPE,
        CTF_SECTION_FOOTNOTE_NUM_PREFIX,
        CTF_SECTION_FOOTNOTE_NUM_SUFFIX,
        CTF_SECTION_FOOTNOTE_END
    };

    static sal_Int16 aEndnoteIds[] =
    {
        CTF_SECTION_ENDNOTE_NUM_OWN,
        CTF_SECTION_ENDNOTE_NUM_RESTART,
        CTF_SECTION_ENDNOTE_NUM_RESTART_AT,
        CTF_SECTION_ENDNOTE_NUM_TYPE,
        CTF_SECTION_ENDNOTE_NUM_PREFIX,
        CTF_SECTION_ENDNOTE_NUM_SUFFIX,
        CTF_SECTION_ENDNOTE_END
    };

    sal_Int16* pIds = bEndnote ? aEndnoteIds : aFootnoteIds;

    // check map entries
    DBG_ASSERT( pIds[0] ==
                rMapper->GetEntryContextId((*pProperties)[nIdx-6].mnIndex),
                "expect consecutive map entries" );
    DBG_ASSERT( pIds[1] ==
                rMapper->GetEntryContextId((*pProperties)[nIdx-5].mnIndex),
                "expect consecutive map entries" );
    DBG_ASSERT( pIds[2] ==
                rMapper->GetEntryContextId((*pProperties)[nIdx-4].mnIndex),
                "expect consecutive map entries" );
    DBG_ASSERT( pIds[3] ==
                rMapper->GetEntryContextId((*pProperties)[nIdx-3].mnIndex),
                "expect consecutive map entries" );
    DBG_ASSERT( pIds[4] ==
                rMapper->GetEntryContextId((*pProperties)[nIdx-2].mnIndex),
                "expect consecutive map entries" );
    DBG_ASSERT( pIds[5] ==
                rMapper->GetEntryContextId((*pProperties)[nIdx-1].mnIndex),
                "expect consecutive map entries" );
    DBG_ASSERT( pIds[6] ==
                rMapper->GetEntryContextId((*pProperties)[nIdx].mnIndex),
                "expect consecutive map entries" );
#endif

    // get the values from the properties
    sal_Bool bNumOwn;
    sal_Bool bNumRestart;
    sal_Int16 nNumRestartAt;
    sal_Int16 nNumberingType;
    OUString sNumPrefix;
    OUString sNumSuffix;
    sal_Bool bEnd;

    (*pProperties)[nIdx-6].maValue >>= bNumOwn;
    (*pProperties)[nIdx-5].maValue >>= bNumRestart;
    (*pProperties)[nIdx-4].maValue >>= nNumRestartAt;
    (*pProperties)[nIdx-3].maValue >>= nNumberingType;
    (*pProperties)[nIdx-2].maValue >>= sNumPrefix;
    (*pProperties)[nIdx-1].maValue >>= sNumSuffix;
    (*pProperties)[nIdx  ].maValue >>= bEnd;

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
            rExport.AddAttribute(XML_NAMESPACE_TEXT, sXML_start_value,
                                 sBuf.makeStringAndClear());
        }

        if (bNumOwn)
        {
            // prefix and suffix
            if (sNumPrefix.getLength() > 0)
            {
                    rExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_num_prefix,
                                         sNumPrefix);
            }
            if (sNumSuffix.getLength() > 0)
            {
                rExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_num_suffix,
                                     sNumSuffix);
            }

            // number type: num format
            rExport.AddAttributeASCII(XML_NAMESPACE_STYLE, sXML_num_format,
                                      SvxXMLNumRuleExport::GetNumFormatValue(
                                          nNumberingType));

            // and letter sync, if applicable
            const sal_Char* pLetterSync =
                SvxXMLNumRuleExport::GetNumLetterSync(nNumberingType);
            if (NULL != pLetterSync)
            {
                rExport.AddAttributeASCII(XML_NAMESPACE_STYLE,
                                          sXML_num_letter_sync, pLetterSync);
            }
        }

        // and finally, the element
        SvXMLElementExport rElem(rExport, XML_NAMESPACE_TEXT,
                                 (bEndnote ? sXML_endnotes_configuration :
                                             sXML_footnotes_configuration ),
                                 sal_True, sal_True);
    }
}
