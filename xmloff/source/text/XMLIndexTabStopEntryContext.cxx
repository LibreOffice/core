/*************************************************************************
 *
 *  $RCSfile: XMLIndexTabStopEntryContext.cxx,v $
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


#ifndef _XMLOFF_XMLINDEXTABSTOPENTRYCONTEXT_HXX_
#include "XMLIndexTabStopEntryContext.hxx"
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

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::xml::sax::XAttributeList;

TYPEINIT1( XMLIndexTabStopEntryContext, SvXMLImportContext );

XMLIndexTabStopEntryContext::XMLIndexTabStopEntryContext(
    SvXMLImport& rImport,
    XMLIndexTemplateContext& rTemplate,
    sal_uInt16 nPrfx,
    const OUString& rLocalName ) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        rTemplateContext(rTemplate),
        sLeaderChar(),
        nTabPosition(0),
        bTabPositionOK(sal_False),
        bTabRightAligned(sal_False),
        bLeaderCharOK(sal_False)
{
}

XMLIndexTabStopEntryContext::~XMLIndexTabStopEntryContext()
{
}

void XMLIndexTabStopEntryContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // process three attributes: type, position, leader char
    sal_Int32 nLength = xAttrList->getLength();
    for(sal_Int32 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        OUString sAttr = xAttrList->getValueByIndex(nAttr);
        if (XML_NAMESPACE_STYLE == nPrefix)
        {
            if (sLocalName.equalsAsciiL(sXML_type, sizeof(sXML_type)-1))
            {
                // if it's neither left nor right, value is
                // ignored. Since left is default, we only need to
                // check for right
                bTabRightAligned = sAttr.equalsAsciiL(sXML_right,
                                                      sizeof(sXML_right)-1);
            }
            else if (sLocalName.equalsAsciiL(sXML_position,
                                             sizeof(sXML_position)-1))
            {
                sal_Int32 nTmp;
                if (GetImport().GetMM100UnitConverter().
                                        convertMeasure(nTmp, sAttr))
                {
                    nTabPosition = nTmp;
                    bTabPositionOK = sal_True;
                }
            }
            else if (sLocalName.equalsAsciiL(sXML_leader_char,
                                             sizeof(sXML_leader_char)-1))
            {
                sLeaderChar = sAttr;
                // valid only, if we have a char!
                bLeaderCharOK = (sAttr.getLength() > 0);
            }
            // else: unknown style: attribute -> ignore
        }
        // else: no style attribute -> ignore
    }
}

void XMLIndexTabStopEntryContext::EndElement()
{
    // how many entries?
    sal_Int32 nValues = 2 + (bTabPositionOK ? 1 : 0) + (bLeaderCharOK ? 1 : 0);

    Sequence<PropertyValue> aValues(nValues);
    Any aAny;
    sal_Int32 nCount = 0;

    // type: tab stop
    aValues[nCount].Name = rTemplateContext.sTokenType;
    aAny <<= rTemplateContext.sTokenTabStop;
    aValues[nCount].Value = aAny;
    nCount++;

    // right aligned?
    aValues[nCount].Name = rTemplateContext.sTabStopRightAligned;
    aAny.setValue(&bTabRightAligned, ::getBooleanCppuType());
    aValues[nCount].Value = aAny;
    nCount++;

    // position
    if (bTabPositionOK)
    {
        aValues[nCount].Name = rTemplateContext.sTabStopPosition;
        aAny <<= nTabPosition;
        aValues[nCount].Value = aAny;
        nCount++;
    }

    // leader char
    if (bLeaderCharOK)
    {
        aValues[nCount].Name = rTemplateContext.sTabStopFillCharacter;
        aAny <<= sLeaderChar;
        aValues[nCount].Value = aAny;
        nCount++;
    }

    // add this template entry
    rTemplateContext.addTemplateEntry(aValues);
}
