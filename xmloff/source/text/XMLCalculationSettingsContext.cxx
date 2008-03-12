/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLCalculationSettingsContext.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:00:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_XMLCALCULATIONSETTINGSCONTEXT_HXX
#include <XMLCalculationSettingsContext.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif


using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

XMLCalculationSettingsContext::XMLCalculationSettingsContext( SvXMLImport& rImport,
                                    sal_uInt16 p_nPrefix,
                                    const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
: SvXMLImportContext ( rImport, p_nPrefix, rLocalName )
, nYear( 1930 )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if ( IsXMLToken( aLocalName, XML_NULL_YEAR ) )
            {
                sal_Int32 nTemp;
                GetImport().GetMM100UnitConverter().convertNumber(nTemp, sValue);
                nYear= static_cast <sal_Int16> (nTemp);
            }
        }
    }
}

XMLCalculationSettingsContext::~XMLCalculationSettingsContext()
{
}
void XMLCalculationSettingsContext::EndElement()
{
    if (nYear != 1930 )
    {
        Reference < XTextDocument > xTextDoc ( GetImport().GetModel(), UNO_QUERY);
        if (xTextDoc.is())
        {
            Reference < XPropertySet > xPropSet ( xTextDoc, UNO_QUERY );
            OUString sTwoDigitYear ( RTL_CONSTASCII_USTRINGPARAM ( "TwoDigitYear" ) );
            Any aAny;
            aAny <<= nYear;
            xPropSet->setPropertyValue ( sTwoDigitYear, aAny );
        }
    }
}
