/*************************************************************************
 *
 *  $RCSfile: XMLChangedRegionImportContext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mib $ $Date: 2001-03-09 07:23:23 $
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

#ifndef _XMLOFF_XMLCHANGEDREGIONIMPORTCONTEXT_HXX
#include "XMLChangedRegionImportContext.hxx"
#endif

#ifndef _XMLOFF_XMLCHANGEELEMENTIMPORTCONTEXT_HXX
#include "XMLChangeElementImportContext.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTCURSOR_HPP_
#include <com/sun/star/text/XTextCursor.hpp>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
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



using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::text::XTextCursor;
using ::com::sun::star::util::DateTime;
using ::com::sun::star::xml::sax::XAttributeList;



TYPEINIT1(XMLChangedRegionImportContext, SvXMLImportContext);

XMLChangedRegionImportContext::XMLChangedRegionImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName) :
        SvXMLImportContext(rImport, nPrefix, rLocalName)
{
}

XMLChangedRegionImportContext::~XMLChangedRegionImportContext()
{
}

void XMLChangedRegionImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // process attributes: id
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        if ( (XML_NAMESPACE_TEXT == nPrefix) &&
             (sLocalName.equalsAsciiL(sXML_id,
                                      sizeof(sXML_id)-1)) )
        {
            sID = xAttrList->getValueByIndex(nAttr);
        }
    }
}

SvXMLImportContext* XMLChangedRegionImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLImportContext* pContext = NULL;

    if (XML_NAMESPACE_TEXT == nPrefix)
    {
        if (rLocalName.equalsAsciiL(sXML_insertion, sizeof(sXML_insertion)-1)||
            rLocalName.equalsAsciiL(sXML_deletion, sizeof(sXML_deletion)-1) ||
            rLocalName.equalsAsciiL(sXML_format_change,
                                    sizeof(sXML_format_change)-1) )
        {
            // create XMLChangeElementImportContext for all kinds of changes
            pContext = new XMLChangeElementImportContext(
               GetImport(), nPrefix, rLocalName,
               rLocalName.equalsAsciiL(sXML_deletion, sizeof(sXML_deletion)-1),
               *this);
        }
        // else: it may be a text element, see below
    }

    if (NULL == pContext)
    {
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);

        // was it a text element? If not, use default!
        if (NULL == pContext)
        {
            pContext = SvXMLImportContext::CreateChildContext(
                nPrefix, rLocalName, xAttrList);
        }
    }

    return pContext;
}

void XMLChangedRegionImportContext::EndElement()
{
    // restore old XCursor (if necessary)
    if (xOldCursor.is())
    {
        // delete last paragraph
        // (one extra paragraph was inserted in the beginning)
        UniReference<XMLTextImportHelper> rHelper =
            GetImport().GetTextImport();
        rHelper->DeleteParagraph();

        GetImport().GetTextImport()->SetCursor(xOldCursor);
        xOldCursor = NULL;
    }
}

void XMLChangedRegionImportContext::SetChangeInfo(
    const OUString& rType,
    const OUString& rAuthor,
    const OUString& rComment,
    const OUString& rDate)
{
    DateTime aDateTime;
    if (SvXMLUnitConverter::convertDateTime(aDateTime, rDate))
    {
        GetImport().GetTextImport()->RedlineAdd(
            rType, sID, rAuthor, rComment, aDateTime);
    }
}

void XMLChangedRegionImportContext::UseRedlineText()
{
    // if we haven't already installed the redline cursor, do it now
    if (! xOldCursor.is())
    {
        // get TextImportHelper and old Cursor
        UniReference<XMLTextImportHelper> rHelper(GetImport().GetTextImport());
        Reference<XTextCursor> xCursor( rHelper->GetCursor() );

        // create Redline and new Cursor
        Reference<XTextCursor> xNewCursor =
            rHelper->RedlineCreateText(xCursor, sID);

        if (xNewCursor.is())
        {
            // save old cursor and install new one
            xOldCursor = xCursor;
            rHelper->SetCursor( xNewCursor );
        }
        // else: leave as is
    }
}
