/*************************************************************************
 *
 *  $RCSfile: XMLTextListItemContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-09-25 06:57:28 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
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
#ifndef _XMLOFF_TXTPARAI_HXX
#include "txtparai.hxx"
#endif
#ifndef _XMLTEXTLISTBLOCKCONTEXT_HXX
#include "XMLTextListBlockContext.hxx"
#endif
#ifndef _XMLOFF_TXTIMP_HXX
#include "txtimp.hxx"
#endif


#ifndef _XMLTEXTLISTITEMCONTEXT_HXX
#include "XMLTextListItemContext.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

TYPEINIT1( XMLTextListItemContext, SvXMLImportContext );

XMLTextListItemContext::XMLTextListItemContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rTxtImp, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Bool bIsHeader ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rTxtImport( rTxtImp ),
    nStartValue( -1 )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        if( !bIsHeader && XML_NAMESPACE_TEXT == nPrefix &&
            aLocalName.compareToAscii( sXML_start_value ) == 0 )
        {
            sal_Int32 nTmp = rValue.toInt32();
            if( nTmp >= 0 && nTmp <= SHRT_MAX )
                nStartValue = (sal_Int16)nTmp;
        }
    }

    DBG_ASSERT( !rTxtImport.GetListItem(),
        "SwXMLListItemContext::SwXMLListItemContext: list item is existing" );

    // If this is a <text:list-item> element, then remember it as a sign
    // that a bullet has to be generated.
    if( !bIsHeader )
        rTxtImport.SetListItem( this );
}

XMLTextListItemContext::~XMLTextListItemContext()
{
}

void XMLTextListItemContext::EndElement()
{
    // finish current list item
    rTxtImport.SetListItem( 0 );
}

SvXMLImportContext *XMLTextListItemContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = rTxtImport.GetTextElemTokenMap();
    sal_Bool bOrdered = sal_False;
    sal_Bool bHeading = sal_False;
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_TEXT_H:
        bHeading = sal_True;
    case XML_TOK_TEXT_P:
        pContext = new XMLParaContext( GetImport(),
                                       nPrefix, rLocalName,
                                       xAttrList, bHeading );
        break;
    case XML_TOK_TEXT_ORDERED_LIST:
        bOrdered = sal_True;
    case XML_TOK_TEXT_UNORDERED_LIST:
        pContext = new XMLTextListBlockContext( GetImport(), rTxtImport,
                                            nPrefix, rLocalName,
                                            xAttrList, bOrdered );
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}


