/*************************************************************************
 *
 *  $RCSfile: XMLSectionSourceDDEImportContext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dvo $ $Date: 2000-11-20 19:56:50 $
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

#ifndef _XMLOFF_XMLSECTIONSOURCEDDEIMPORTCONTEXT_HXX_
#include "XMLSectionSourceDDEImportContext.hxx"
#endif

#ifndef _XMLOFF_XMLSECTIONIMPORTCONTEXT_HXX_
#include "XMLSectionImportContext.hxx"
#endif

#ifndef _COM_SUN_STAR_TEXT_SECTIONFILELINK_HPP_
#include <com/sun/star/text/SectionFileLink.hpp>
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

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif


using ::rtl::OUString;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

const sal_Char sAPI_DDECommandFile[] = "DDECommandFile";
const sal_Char sAPI_DDECommandType[] = "DDECommandType";
const sal_Char sAPI_DDECommandElement[] = "DDECommandElement";
const sal_Char sAPI_IsAutomaticUpdate[] = "IsAutomaticUpdate";


TYPEINIT1(XMLSectionSourceDDEImportContext, SvXMLImportContext);

XMLSectionSourceDDEImportContext::XMLSectionSourceDDEImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    Reference<XPropertySet> & rSectPropSet) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        rSectionPropertySet(rSectPropSet),
        sDdeCommandFile(RTL_CONSTASCII_USTRINGPARAM(sAPI_DDECommandFile)),
        sDdeCommandType(RTL_CONSTASCII_USTRINGPARAM(sAPI_DDECommandType)),
       sDdeCommandElement(RTL_CONSTASCII_USTRINGPARAM(sAPI_DDECommandElement)),
        sIsAutomaticUpdate(RTL_CONSTASCII_USTRINGPARAM(sAPI_IsAutomaticUpdate))
{
}

XMLSectionSourceDDEImportContext::~XMLSectionSourceDDEImportContext()
{
}

enum XMLSectionSourceDDEToken
{
    XML_TOK_SECTION_DDE_APPLICATION,
    XML_TOK_SECTION_DDE_TOPIC,
    XML_TOK_SECTION_DDE_ITEM,
    XML_TOK_SECTION_IS_AUTOMATIC_UPDATE
};

static __FAR_DATA SvXMLTokenMapEntry aSectionSourceDDETokenMap[] =
{
    { XML_NAMESPACE_OFFICE, sXML_dde_application,
          XML_TOK_SECTION_DDE_APPLICATION },
    { XML_NAMESPACE_OFFICE, sXML_dde_topic, XML_TOK_SECTION_DDE_TOPIC },
    { XML_NAMESPACE_OFFICE, sXML_dde_item, XML_TOK_SECTION_DDE_ITEM },
    { XML_NAMESPACE_OFFICE, sXML_automatic_update,
          XML_TOK_SECTION_IS_AUTOMATIC_UPDATE },
    XML_TOKEN_MAP_END
};


void XMLSectionSourceDDEImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLTokenMap aTokenMap(aSectionSourceDDETokenMap);
    OUString sApplication;
    OUString sTopic;
    OUString sItem;
    sal_Bool bAutomaticUpdate = sal_False;

    sal_Int32 nLength = xAttrList->getLength();
    for(sal_Int32 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );

        switch (aTokenMap.Get(nPrefix, sLocalName))
        {
            case XML_TOK_SECTION_DDE_APPLICATION:
                sApplication = xAttrList->getValueByIndex(nAttr);
                break;
            case XML_TOK_SECTION_DDE_TOPIC:
                sTopic = xAttrList->getValueByIndex(nAttr);
                break;
            case XML_TOK_SECTION_DDE_ITEM:
                sItem = xAttrList->getValueByIndex(nAttr);
                break;
            case XML_TOK_SECTION_IS_AUTOMATIC_UPDATE:
            {
                sal_Bool bTmp;
                if (SvXMLUnitConverter::convertBool(
                    bTmp, xAttrList->getValueByIndex(nAttr)))
                {
                    bAutomaticUpdate = bTmp;
                }
                break;
            }
            default:
                ; // ignore
                break;
        }
    }

    // DDE not supported on all platforms; query property first
    if (rSectionPropertySet->getPropertySetInfo()->
        hasPropertyByName(sDdeCommandFile))
    {
        Any aAny;

        aAny <<= sApplication;
        rSectionPropertySet->setPropertyValue(sDdeCommandFile, aAny);

        aAny <<= sTopic;
        rSectionPropertySet->setPropertyValue(sDdeCommandType, aAny);

        aAny <<= sItem;
        rSectionPropertySet->setPropertyValue(sDdeCommandElement, aAny);

        aAny.setValue(&bAutomaticUpdate, ::getBooleanCppuType());
        rSectionPropertySet->setPropertyValue(sIsAutomaticUpdate, aAny);
    }
}

void XMLSectionSourceDDEImportContext::EndElement()
{
    // nothing to be done!
}

SvXMLImportContext* XMLSectionSourceDDEImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    // ignore -> default context
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}
