/*************************************************************************
 *
 *  $RCSfile: XMLIndexTOCSourceContext.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2000-11-02 15:51:18 $
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


#ifndef _XMLOFF_XMLINDEXTOCSOURCECONTEXT_HXX_
#include "XMLIndexTOCSourceContext.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif

#ifndef _XMLOFF_XMLINDEXTEMPLATECONTEXT_HXX_
#include "XMLIndexTemplateContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXTITLETEMPLATECONTEXT_HXX_
#include "XMLIndexTitleTemplateContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXTOCSTYLESCONTEXT_HXX_
#include "XMLIndexTOCStylesContext.hxx"
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_TEXTIMP_HXX_
#include "txtimp.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif




using ::rtl::OUString;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;

const sal_Char sAPI_CreateFromChapter[] = "CreateFromChapter";
const sal_Char sAPI_CreateFromOutline[] = "CreateFromOutline";
const sal_Char sAPI_CreateFromMarks[] = "CreateFromMarks";
const sal_Char sAPI_Level[] = "Level";

TYPEINIT1( XMLIndexTOCSourceContext, SvXMLImportContext );

XMLIndexTOCSourceContext::XMLIndexTOCSourceContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    Reference<XPropertySet> & rPropSet) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        rTOCPropertySet(rPropSet),
        // use all chapters by default
        nOutlineLevel(rImport.GetTextImport()->GetChapterNumbering()->
                                                                  getCount()),
        bUseOutline(sal_True),
        bUseMarks(sal_True),
        bChapterIndex(sal_False),
        bRelativeTabs(sal_True),
        sCreateFromMarks(RTL_CONSTASCII_USTRINGPARAM(sAPI_CreateFromMarks)),
        sLevel(RTL_CONSTASCII_USTRINGPARAM(sAPI_Level)),
       sCreateFromChapter(RTL_CONSTASCII_USTRINGPARAM(sAPI_CreateFromChapter)),
       sCreateFromOutline(RTL_CONSTASCII_USTRINGPARAM(sAPI_CreateFromOutline))
{
}

XMLIndexTOCSourceContext::~XMLIndexTOCSourceContext()
{
}

enum IndexTOCSourceTokenEnum
{
    XML_TOK_TOCSOURCE_OUTLINE_LEVEL,
    XML_TOK_TOCSOURCE_USE_INDEX_MARKS,
    XML_TOK_TOCSOURCE_INDEX_SCOPE,
    XML_TOK_TOCSOURCE_RELATIVE_TABS
};

static __FAR_DATA SvXMLTokenMapEntry aIndexTOCSourceTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_outline_level, XML_TOK_TOCSOURCE_OUTLINE_LEVEL},
    { XML_NAMESPACE_TEXT, sXML_use_index_marks,
          XML_TOK_TOCSOURCE_USE_INDEX_MARKS },
    { XML_NAMESPACE_TEXT, sXML_index_scope, XML_TOK_TOCSOURCE_INDEX_SCOPE },
    { XML_NAMESPACE_TEXT, sXML_relative_tab_stop_position,
          XML_TOK_TOCSOURCE_RELATIVE_TABS } ,
    XML_TOKEN_MAP_END
};

void XMLIndexTOCSourceContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLTokenMap aTokenMap(aIndexTOCSourceTokenMap);

    // process attributes
    sal_Int32 nLength = xAttrList->getLength();
    for(sal_Int32 i=0; i<nLength; i++)
    {

        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );

        OUString sAttr = xAttrList->getValueByIndex(i);

        switch (aTokenMap.Get(nPrefix, sLocalName))
        {
            case XML_TOK_TOCSOURCE_OUTLINE_LEVEL:
                if (sAttr.equalsAsciiL(sXML_none, sizeof(sXML_none)-1))
                {
                    bUseOutline = sal_False;
                }
                else
                {
                    sal_Int32 nTmp;
                    if (SvXMLUnitConverter::convertNumber(
                                nTmp, sAttr, 1, GetImport().GetTextImport()->
                                            GetChapterNumbering()->getCount()))
                    {
                        bUseOutline = sal_True;
                        nOutlineLevel = nTmp;
                    }
                }
                break;

            case XML_TOK_TOCSOURCE_USE_INDEX_MARKS:
            {
                sal_Bool bTmp;
                if (SvXMLUnitConverter::convertBool(bTmp, sAttr))
                {
                    bUseMarks = bTmp;
                }
                break;
            }

            case XML_TOK_TOCSOURCE_INDEX_SCOPE:
                if (sAttr.equalsAsciiL(sXML_chapter, sizeof(sXML_chapter)-1))
                {
                    bChapterIndex = sal_True;
                }
                break;

            case XML_TOK_TOCSOURCE_RELATIVE_TABS:
            {
                sal_Bool bTmp;
                if (SvXMLUnitConverter::convertBool(bTmp, sAttr))
                {
                    bRelativeTabs = bTmp;
                }
                break;
            }

            default:
                // unknown attribute -> ignore
                break;
        }
    }
}

void XMLIndexTOCSourceContext::EndElement()
{
    Any aAny;

    aAny.setValue(&bRelativeTabs, ::getBooleanCppuType());
//  rTOCPropertySet->setPropertyValue(s..., aAny);

    aAny.setValue(&bChapterIndex, ::getBooleanCppuType());
    rTOCPropertySet->setPropertyValue(sCreateFromChapter, aAny);

    aAny.setValue(&bUseMarks, ::getBooleanCppuType());
    rTOCPropertySet->setPropertyValue(sCreateFromMarks, aAny);

    aAny.setValue(&bUseOutline, ::getBooleanCppuType());
    rTOCPropertySet->setPropertyValue(sCreateFromOutline, aAny);

    aAny <<= (sal_Int16)nOutlineLevel;
    rTOCPropertySet->setPropertyValue(sLevel, aAny);
}

SvXMLImportContext* XMLIndexTOCSourceContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if (XML_NAMESPACE_TEXT == nPrefix)
    {
        if (rLocalName.equalsAsciiL(sXML_index_entry_template,
                                    sizeof(sXML_index_entry_template)-1))
        {
            pContext = new XMLIndexTemplateContext(GetImport(),
                                                   rTOCPropertySet,
                                                   nPrefix, rLocalName);
        }
        else if (rLocalName.equalsAsciiL(sXML_index_title_template,
                                         sizeof(sXML_index_title_template)-1))
        {
            pContext = new XMLIndexTitleTemplateContext(GetImport(),
                                                        rTOCPropertySet,
                                                        nPrefix, rLocalName);
        }
        else if (rLocalName.equalsAsciiL(sXML_index_source_styles,
                                         sizeof(sXML_index_source_styles)-1))
        {
            pContext = new XMLIndexTOCStylesContext(GetImport(),
                                                    rTOCPropertySet,
                                                    nPrefix, rLocalName);
        }
        // else: unknown element in text namespace -> ignore
    }
    // else: unknown namespace -> ignore

    // use default namespace
    if (pContext == NULL)
    {
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);
    }

    return pContext;
}
