/*************************************************************************
 *
 *  $RCSfile: XMLIndexTOCSourceContext.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dvo $ $Date: 2000-11-21 11:53:19 $
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
const sal_Char sAPI_CreateFromLevelParagraphStyles[] = "CreateFromLevelParagraphStyles";

TYPEINIT1( XMLIndexTOCSourceContext, XMLIndexSourceBaseContext );

XMLIndexTOCSourceContext::XMLIndexTOCSourceContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    Reference<XPropertySet> & rPropSet) :
        XMLIndexSourceBaseContext(rImport, nPrfx, rLocalName,
                                  rPropSet, sal_True),
        // use all chapters by default
        nOutlineLevel(rImport.GetTextImport()->GetChapterNumbering()->
                                                                  getCount()),
        bUseOutline(sal_True),
        bUseMarks(sal_True),
        sCreateFromMarks(RTL_CONSTASCII_USTRINGPARAM(sAPI_CreateFromMarks)),
        sLevel(RTL_CONSTASCII_USTRINGPARAM(sAPI_Level)),
       sCreateFromOutline(RTL_CONSTASCII_USTRINGPARAM(sAPI_CreateFromOutline)),
        sCreateFromLevelParagraphStyles(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_CreateFromLevelParagraphStyles)),
        bUseParagraphStyles(sal_False)
{
}

XMLIndexTOCSourceContext::~XMLIndexTOCSourceContext()
{
}

void XMLIndexTOCSourceContext::ProcessAttribute(
    enum IndexSourceParamEnum eParam,
    const OUString& rValue)
{
    switch (eParam)
    {
        case XML_TOK_INDEXSOURCE_OUTLINE_LEVEL:
            if (rValue.equalsAsciiL(sXML_none, sizeof(sXML_none)-1))
            {
                bUseOutline = sal_False;
            }
            else
            {
                sal_Int32 nTmp;
                if (SvXMLUnitConverter::convertNumber(
                    nTmp, rValue, 1, GetImport().GetTextImport()->
                    GetChapterNumbering()->getCount()))
                {
                    bUseOutline = sal_True;
                    nOutlineLevel = nTmp;
                }
            }
            break;

        case XML_TOK_INDEXSOURCE_USE_INDEX_MARKS:
        {
            sal_Bool bTmp;
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bUseMarks = bTmp;
            }
            break;
        }

        case XML_TOK_INDEXSOURCE_USE_INDEX_SOURCE_STYLES:
        {
            sal_Bool bTmp;
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bUseParagraphStyles = bTmp;
            }
            break;
        }

        default:
            // default: ask superclass
            XMLIndexSourceBaseContext::ProcessAttribute(eParam, rValue);
            break;
    }
}

void XMLIndexTOCSourceContext::EndElement()
{
    Any aAny;

    aAny.setValue(&bUseMarks, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sCreateFromMarks, aAny);

    aAny.setValue(&bUseOutline, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sCreateFromOutline, aAny);

    aAny.setValue(&bUseParagraphStyles, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sCreateFromLevelParagraphStyles, aAny);

    aAny <<= (sal_Int16)nOutlineLevel;
    rIndexPropertySet->setPropertyValue(sLevel, aAny);

    // process common attributes
    XMLIndexSourceBaseContext::EndElement();
}


SvXMLImportContext* XMLIndexTOCSourceContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    if ( (XML_NAMESPACE_TEXT == nPrefix) &&
         (rLocalName.equalsAsciiL(sXML_table_of_content_entry_template,
                            sizeof(sXML_table_of_content_entry_template)-1)))
    {
        return new XMLIndexTemplateContext(GetImport(), rIndexPropertySet,
                                           nPrefix, rLocalName,
                                           aLevelNameTOCMap,
                                           sXML_outline_level,
                                           aLevelStylePropNameTOCMap,
                                           aAllowedTokenTypesTOC);
    }
    else
    {
        return XMLIndexSourceBaseContext::CreateChildContext(nPrefix,
                                                             rLocalName,
                                                             xAttrList);
    }
}
