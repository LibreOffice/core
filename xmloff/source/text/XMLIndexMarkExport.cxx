/*************************************************************************
 *
 *  $RCSfile: XMLIndexMarkExport.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 18:20:38 $
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

#ifndef _XMLOFF_XMLINDEXMARKEXPORT_HXX_
#include "XMLIndexMarkExport.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif


using namespace ::xmloff::token;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;



XMLIndexMarkExport::XMLIndexMarkExport(
    SvXMLExport& rExp,
    XMLTextParagraphExport& rParaExp) :
        rExport(rExp),
        rParaExport(rParaExp),
        sLevel(RTL_CONSTASCII_USTRINGPARAM("Level")),
        sUserIndexName(RTL_CONSTASCII_USTRINGPARAM("UserIndexName")),
        sPrimaryKey(RTL_CONSTASCII_USTRINGPARAM("PrimaryKey")),
        sSecondaryKey(RTL_CONSTASCII_USTRINGPARAM("SecondaryKey")),
        sDocumentIndexMark(RTL_CONSTASCII_USTRINGPARAM("DocumentIndexMark")),
        sIsStart(RTL_CONSTASCII_USTRINGPARAM("IsStart")),
        sIsCollapsed(RTL_CONSTASCII_USTRINGPARAM("IsCollapsed")),
        sAlternativeText(RTL_CONSTASCII_USTRINGPARAM("AlternativeText")),
        sTextReading(RTL_CONSTASCII_USTRINGPARAM("TextReading")),
        sPrimaryKeyReading(RTL_CONSTASCII_USTRINGPARAM("PrimaryKeyReading")),
        sSecondaryKeyReading(RTL_CONSTASCII_USTRINGPARAM
                             ("SecondaryKeyReading")),
        sMainEntry(RTL_CONSTASCII_USTRINGPARAM("IsMainEntry"))
{
}

const enum XMLTokenEnum lcl_pTocMarkNames[] =
    { XML_TOC_MARK, XML_TOC_MARK_START, XML_TOC_MARK_END };
const enum XMLTokenEnum lcl_pUserIndexMarkName[] =
    { XML_USER_INDEX_MARK,
          XML_USER_INDEX_MARK_START, XML_USER_INDEX_MARK_END };
const enum XMLTokenEnum lcl_pAlphaIndexMarkName[] =
    { XML_ALPHABETICAL_INDEX_MARK,
          XML_ALPHABETICAL_INDEX_MARK_START,
          XML_ALPHABETICAL_INDEX_MARK_END };


XMLIndexMarkExport::~XMLIndexMarkExport()
{
}

void XMLIndexMarkExport::ExportIndexMark(
    const Reference<XPropertySet> & rPropSet,
    sal_Bool bAutoStyles)
{
    /// index marks have no styles!
    if (!bAutoStyles)
    {
        const enum XMLTokenEnum * pElements = NULL;
        sal_Int8 nElementNo = -1;

        // get index mark
        Any aAny;
        aAny = rPropSet->getPropertyValue(sDocumentIndexMark);
        Reference<XPropertySet> xIndexMarkPropSet;
        aAny >>= xIndexMarkPropSet;

        // common: handling of start, end, collapsed entries and
        // alternative text

        // collapsed/alternative text entry?
        aAny = rPropSet->getPropertyValue(sIsCollapsed);
        if (*(sal_Bool *)aAny.getValue())
        {
            // collapsed entry: needs alternative text
            nElementNo = 0;

            aAny = xIndexMarkPropSet->getPropertyValue(sAlternativeText);
            OUString sTmp;
            aAny >>= sTmp;
            DBG_ASSERT(sTmp.getLength() > 0,
                       "collapsed index mark without alternative text");
            rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_STRING_VALUE, sTmp);
        }
        else
        {
            // start and end entries: has ID
            aAny = rPropSet->getPropertyValue(sIsStart);
            nElementNo = *(sal_Bool *)aAny.getValue() ? 1 : 2;

            // generate ID
            OUStringBuffer sBuf;
            GetID(sBuf, xIndexMarkPropSet);
            rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_ID,
                                 sBuf.makeStringAndClear());
        }

        // distinguish between TOC, user, alphab. index marks by
        // asking for specific properties
        // Export attributes for -mark-start and -mark elements,
        // but not for -mark-end
        Reference<XPropertySetInfo> xPropertySetInfo =
            xIndexMarkPropSet->getPropertySetInfo();
        if (xPropertySetInfo->hasPropertyByName(sUserIndexName))
        {
            // user index mark
            pElements = lcl_pUserIndexMarkName;
            if (nElementNo != 2)
            {
                ExportUserIndexMarkAttributes(xIndexMarkPropSet);
            }
        }
        else if (xPropertySetInfo->hasPropertyByName(sPrimaryKey))
        {
            // alphabetical index mark
            pElements = lcl_pAlphaIndexMarkName;
            if (nElementNo != 2)
            {
                ExportAlphabeticalIndexMarkAttributes(xIndexMarkPropSet);
            }
        }
        else
        {
            // table of content:
            pElements = lcl_pTocMarkNames;
            if (nElementNo != 2)
            {
                ExportTOCMarkAttributes(xIndexMarkPropSet);
            }
        }

        // export element
        DBG_ASSERT(pElements != NULL, "illegal element array");
        DBG_ASSERT(nElementNo >= 0, "illegal name array index");
        DBG_ASSERT(nElementNo <= 2, "illegal name array index");

        if ((pElements != NULL) && (nElementNo != -1))
        {
            SvXMLElementExport aElem(rExport,
                                     XML_NAMESPACE_TEXT,
                                     pElements[nElementNo],
                                     sal_False, sal_False);
        }
    }
}

void XMLIndexMarkExport::ExportTOCMarkAttributes(
    const Reference<XPropertySet> & rPropSet)
{
    // outline level
    sal_Int16 nLevel;
    Any aAny = rPropSet->getPropertyValue(sLevel);
    aAny >>= nLevel;
    OUStringBuffer sBuf;
    SvXMLUnitConverter::convertNumber(sBuf, (sal_Int32)nLevel);
    rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_OUTLINE_LEVEL,
                             sBuf.makeStringAndClear());
}

void lcl_ExportPropertyString( SvXMLExport& rExport,
                               const Reference<XPropertySet> & rPropSet,
                               const OUString sProperty,
                               XMLTokenEnum eToken,
                               Any& rAny )
{
    rAny = rPropSet->getPropertyValue( sProperty );

    OUString sValue;
    if( rAny >>= sValue )
    {
        if( sValue.getLength() > 0 )
        {
            rExport.AddAttribute( XML_NAMESPACE_TEXT, eToken, sValue );
        }
    }
}

void lcl_ExportPropertyBool( SvXMLExport& rExport,
                             const Reference<XPropertySet> & rPropSet,
                             const OUString sProperty,
                             XMLTokenEnum eToken,
                             Any& rAny )
{
    rAny = rPropSet->getPropertyValue( sProperty );

    sal_Bool bValue;
    if( rAny >>= bValue )
    {
        if( bValue )
        {
            rExport.AddAttribute( XML_NAMESPACE_TEXT, eToken, XML_TRUE );
        }
    }
}

void XMLIndexMarkExport::ExportUserIndexMarkAttributes(
    const Reference<XPropertySet> & rPropSet)
{
    // name of user index
    // (unless it's the default index; then it has no name)
    Any aAny;
    lcl_ExportPropertyString( rExport, rPropSet, sUserIndexName, XML_INDEX_NAME, aAny );

    // additionally export outline level; just reuse ExportTOCMarkAttributes
    ExportTOCMarkAttributes( rPropSet );
}

void XMLIndexMarkExport::ExportAlphabeticalIndexMarkAttributes(
    const Reference<XPropertySet> & rPropSet)
{
    // primary and secondary keys (if available)
    Any aAny;
    lcl_ExportPropertyString( rExport, rPropSet, sTextReading, XML_STRING_VALUE_PHONETIC, aAny );
    lcl_ExportPropertyString( rExport, rPropSet, sPrimaryKey, XML_KEY1, aAny );
    lcl_ExportPropertyString( rExport, rPropSet, sPrimaryKeyReading, XML_KEY1_PHONETIC, aAny );
    lcl_ExportPropertyString( rExport, rPropSet, sSecondaryKey, XML_KEY2, aAny );
    lcl_ExportPropertyString( rExport, rPropSet, sSecondaryKeyReading, XML_KEY2_PHONETIC, aAny );
    lcl_ExportPropertyBool( rExport, rPropSet, sMainEntry, XML_MAIN_ENTRY, aAny );
}

void XMLIndexMarkExport::GetID(
    OUStringBuffer& sBuf,
    const Reference<XPropertySet> & rPropSet)
{
    static const sal_Char sPrefix[] = "IMark";

    // HACK: use address of object to form identifier
    sal_Int64 nId = (sal_Int64)rPropSet.get();
    sBuf.appendAscii(sPrefix, sizeof(sPrefix)-1);
    sBuf.append(nId);
}
