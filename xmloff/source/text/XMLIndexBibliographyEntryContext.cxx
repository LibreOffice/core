/*************************************************************************
 *
 *  $RCSfile: XMLIndexBibliographyEntryContext.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2000-11-14 14:42:50 $
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


#ifndef _XMLOFF_XMLINDEXBIBLIOGRAPHYENTRYCONTEXT_HXX_
#include "XMLIndexBibliographyEntryContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXTEMPLATECONTEXT_HXX_
#include "XMLIndexTemplateContext.hxx"
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

#ifndef _COM_SUN_STAR_TEXT_BIBLIOGRAPHYDATAFIELD_HPP_
#include <com/sun/star/text/BibliographyDataField.hpp>
#endif


using namespace ::com::sun::star::text;

using ::rtl::OUString;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::PropertyValues;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;


const sal_Char sAPI_TokenType[] = "TokenType";
const sal_Char sAPI_CharacterStyleName[] = "CharacterStyleName";

TYPEINIT1( XMLIndexBibliographyEntryContext, XMLIndexSimpleEntryContext);

XMLIndexBibliographyEntryContext::XMLIndexBibliographyEntryContext(
    SvXMLImport& rImport,
    XMLIndexTemplateContext& rTemplate,
    sal_uInt16 nPrfx,
    const OUString& rLocalName ) :
        XMLIndexSimpleEntryContext(rImport,
                                   rTemplate.sTokenBibliographyDataField,
                                   rTemplate,
                                   nPrfx, rLocalName),
        nBibliographyInfo(BibliographyDataField::IDENTIFIER),
        bBibliographyInfoOK(sal_False)
{
}

XMLIndexBibliographyEntryContext::~XMLIndexBibliographyEntryContext()
{
}

SvXMLEnumMapEntry __READONLY_DATA aBibliographyDataFieldMap[] =
{
    { sXML_address,             BibliographyDataField::ADDRESS },
    { sXML_annote,              BibliographyDataField::ANNOTE },
    { sXML_author,              BibliographyDataField::AUTHOR },
    { sXML_bibiliographic_type, BibliographyDataField::BIBILIOGRAPHIC_TYPE },
    { sXML_booktitle,           BibliographyDataField::BOOKTITLE },
    { sXML_chapter,             BibliographyDataField::CHAPTER },
    { sXML_custom1,             BibliographyDataField::CUSTOM1 },
    { sXML_custom2,             BibliographyDataField::CUSTOM2 },
    { sXML_custom3,             BibliographyDataField::CUSTOM3 },
    { sXML_custom4,             BibliographyDataField::CUSTOM4 },
    { sXML_custom5,             BibliographyDataField::CUSTOM5 },
    { sXML_edition,             BibliographyDataField::EDITION },
    { sXML_editor,              BibliographyDataField::EDITOR },
    { sXML_howpublished,        BibliographyDataField::HOWPUBLISHED },
    { sXML_identifier,          BibliographyDataField::IDENTIFIER },
    { sXML_institution,         BibliographyDataField::INSTITUTION },
    { sXML_isbn,                BibliographyDataField::ISBN },
    { sXML_journal,             BibliographyDataField::JOURNAL },
    { sXML_month,               BibliographyDataField::MONTH },
    { sXML_note,                BibliographyDataField::NOTE },
    { sXML_number,              BibliographyDataField::NUMBER },
    { sXML_organizations,       BibliographyDataField::ORGANIZATIONS },
    { sXML_pages,               BibliographyDataField::PAGES },
    { sXML_publisher,           BibliographyDataField::PUBLISHER },
    { sXML_report_type,         BibliographyDataField::REPORT_TYPE },
    { sXML_school,              BibliographyDataField::SCHOOL },
    { sXML_series,              BibliographyDataField::SERIES },
    { sXML_title,               BibliographyDataField::TITLE },
    { sXML_url,                 BibliographyDataField::URL },
    { sXML_volume,              BibliographyDataField::VOLUME },
    { sXML_year,                BibliographyDataField::YEAR },
    { NULL, NULL }
};

void XMLIndexBibliographyEntryContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // handle both, style name and bibliography info
    sal_Int32 nLength = xAttrList->getLength();
    for(sal_Int32 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if (sLocalName.equalsAsciiL(sXML_style_name,
                                    sizeof(sXML_style_name)-1))
            {
                sCharStyleName = xAttrList->getValueByIndex(nAttr);
                bCharStyleNameOK = sal_True;
            }
            else if (sLocalName.equalsAsciiL(sXML_bibliography_data_field,
                                    sizeof(sXML_bibliography_data_field)-1))
            {
                sal_uInt16 nTmp;
                if (SvXMLUnitConverter::convertEnum(
                    nTmp, xAttrList->getValueByIndex(nAttr),
                    aBibliographyDataFieldMap))
                {
                    nBibliographyInfo = nTmp;
                    bBibliographyInfoOK = sal_True;
                }
            }
        }
    }

    // if we have a style name, set it!
    if (bCharStyleNameOK)
    {
        nValues++;
    }

    // always bibliography; else element is not valid
    nValues++;
}

void XMLIndexBibliographyEntryContext::EndElement()
{
    // only valid, if we have bibliography info
    if (bBibliographyInfoOK)
    {
        XMLIndexSimpleEntryContext::EndElement();
    }
}

void XMLIndexBibliographyEntryContext::FillPropertyValues(
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue> & rValues)
{
    // entry name and (optionally) style name in parent class
    XMLIndexSimpleEntryContext::FillPropertyValues(rValues);

    // bibliography data field
    sal_Int32 nIndex = bCharStyleNameOK ? 2 : 1;
    rValues[nIndex].Name = rTemplateContext.sBibliographyDataField;
    Any aAny;
    aAny <<= nBibliographyInfo;
    rValues[nIndex].Value = aAny;
}
