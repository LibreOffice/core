/*************************************************************************
 *
 *  $RCSfile: XMLSectionFootnoteConfigImport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2001-03-19 09:41:43 $
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

#ifndef _XMLOFF_XMLSECTIONFOOTNOTECONFIGIMPORT_HXX
#include "XMLSectionFootnoteConfigImport.hxx"
#endif

#ifndef _RTL_USTRING
#include <rtl/ustring>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include "maptype.hxx"
#endif

#ifndef _XMLOFF_XMLNUMI_HXX
#include "xmlnumi.hxx"
#endif

#include <vector>

using namespace ::com::sun::star::style;
using ::rtl::OUString;
using ::std::vector;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;


TYPEINIT1(XMLSectionFootnoteConfigImport, SvXMLImportContext);


XMLSectionFootnoteConfigImport::XMLSectionFootnoteConfigImport(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    vector<XMLPropertyState> & rProps,
    sal_Int32 nIndex) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        rProperties(rProps),
        nPropIndex(nIndex)
{
}

XMLSectionFootnoteConfigImport::~XMLSectionFootnoteConfigImport()
{
}

void XMLSectionFootnoteConfigImport::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    sal_Bool bEnd = sal_True;   // we're inside the element, so this is true
    sal_Bool bNumOwn = sal_False;
    sal_Bool bNumRestart = sal_False;
    sal_Int16 nNumRestartAt = 0;
    OUString sNumPrefix;
    OUString sNumSuffix;
    OUString sNumFormat;
    OUString sNumLetterSync;

    // iterate over xattribute list and fill values
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        OUString sAttrValue = xAttrList->getValueByIndex(nAttr);

        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if (sLocalName.equalsAsciiL(sXML_start_value,
                                        sizeof(sXML_start_value)-1))
            {
                sal_Int32 nTmp;
                if (SvXMLUnitConverter::convertNumber(nTmp, sAttrValue))
                {
                    nNumRestartAt = nTmp - 1;
                    bNumRestart = sal_True;
                }
            }
        }
        else if (XML_NAMESPACE_STYLE == nPrefix)
        {
            if (sLocalName.equalsAsciiL(sXML_num_prefix,
                                        sizeof(sXML_num_prefix)-1))
            {
                sNumPrefix = sAttrValue;
                bNumOwn = sal_True;
            }
            else if (sLocalName.equalsAsciiL(sXML_num_suffix,
                                             sizeof(sXML_num_suffix)-1))
            {
                sNumSuffix = sAttrValue;
                bNumOwn = sal_True;
            }
            else if (sLocalName.equalsAsciiL(sXML_num_format,
                                             sizeof(sXML_num_format)-1))
            {
                sNumFormat = sAttrValue;
                bNumOwn = sal_True;
            }
            else if (sLocalName.equalsAsciiL(sXML_num_letter_sync,
                                             sizeof(sXML_num_letter_sync)-1))
            {
                sNumLetterSync = sAttrValue;
                bNumOwn = sal_True;
            }
        }
    }

    // OK, now we have all values and can fill the XMLPropertyState vector
    Any aAny;

    aAny.setValue( &bNumOwn, ::getBooleanCppuType() );
    XMLPropertyState aNumOwn( nPropIndex-6, aAny );
    rProperties.push_back(aNumOwn);

    aAny.setValue( &bNumRestart, ::getBooleanCppuType() );
    XMLPropertyState aNumRestart( nPropIndex-5, aAny );
    rProperties.push_back(aNumRestart);

    aAny <<= nNumRestartAt;
    XMLPropertyState aNumRestartAtState( nPropIndex-4, aAny );
    rProperties.push_back(aNumRestartAtState);

    sal_Int16 nNumType = NumberingType::ARABIC;
    GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                                    sNumFormat,
                                                    sNumLetterSync );
    aAny <<= nNumType;
    XMLPropertyState aNumFormatState( nPropIndex-3, aAny);
    rProperties.push_back(aNumFormatState);

    aAny <<= sNumPrefix;
    XMLPropertyState aPrefixState( nPropIndex-2, aAny);
    rProperties.push_back(aPrefixState);

    aAny <<= sNumSuffix;
    XMLPropertyState aSuffixState( nPropIndex-1, aAny);
    rProperties.push_back(aSuffixState);

    aAny.setValue( &bEnd, ::getBooleanCppuType() );
    XMLPropertyState aEndState( nPropIndex, aAny );
    rProperties.push_back(aEndState);
}
