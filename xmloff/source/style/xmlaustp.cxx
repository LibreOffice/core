/*************************************************************************
 *
 *  $RCSfile: xmlaustp.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: mib $ $Date: 2000-11-20 10:15:09 $
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
#ifndef _XMLOFF_XMLASTPL_IMPL_HXX
#include "impastpl.hxx"
#endif
#ifndef _XMLOFF_XMLASTPLP_HXX
#include "xmlaustp.hxx"
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include "PageMasterStyleMap.hxx"
#endif
#ifndef _XMLOFF_PAGEMASTEREXPORTPROPMAPPER_HXX
#include "PageMasterExportPropMapper.hxx"
#endif
#ifndef _XMLBACKGROUNDIMAGEEXPORT_HXX
#include "XMLBackgroundImageExport.hxx"
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

using namespace ::std;
using namespace ::rtl;
using namespace ::com::sun::star;


void SvXMLAutoStylePoolP::exportStyleAttributes(
        SvXMLAttributeList& rAttrList,
        sal_Int32 nFamily,
        const vector< XMLPropertyState >& rProperties,
        const SvXMLExportPropertyMapper& rPropExp,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    if( nFamily == XML_STYLE_FAMILY_PAGE_MASTER )
    {
        for( vector< XMLPropertyState >::const_iterator pProp = rProperties.begin(); pProp != rProperties.end(); pProp++ )
        {
            UniReference< XMLPropertySetMapper > aPropMapper = rPropExp.getPropertySetMapper();
            sal_Int32 nIndex = pProp->mnIndex;
            sal_Int16 nContextID = aPropMapper->GetEntryContextId( nIndex );
            switch( nContextID )
            {
                case CTF_PM_PAGEUSAGE:
                {
                    OUString sAttrName( rNamespaceMap.GetQNameByKey(
                        aPropMapper->GetEntryNameSpace( nIndex ), aPropMapper->GetEntryXMLName( nIndex ) ) );
                    OUString sCDATA( RTL_CONSTASCII_USTRINGPARAM( sXML_CDATA ) );
                    OUString sValue;
                    const XMLPropertyHandler* pPropHdl = aPropMapper->GetPropertyHandler( nIndex );
                    if( pPropHdl && pPropHdl->exportXML( sValue, pProp->maValue, rUnitConverter ) &&
                        (sValue.compareToAscii( sXML_all ) != 0) )
                        rAttrList.AddAttribute( sAttrName, sCDATA, sValue );
                }
                break;
            }
        }
    }
}

void SvXMLAutoStylePoolP::exportStyleContent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
        sal_Int32 nFamily,
        const vector< XMLPropertyState >& rProperties,
        const SvXMLExportPropertyMapper& rPropExp,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    if( nFamily == XML_STYLE_FAMILY_PAGE_MASTER )
    {
        OUString sType( RTL_CONSTASCII_USTRINGPARAM( sXML_CDATA ) );
        OUString sWS( RTL_CONSTASCII_USTRINGPARAM( sXML_WS ) );

        sal_Int32       nHeaderStartIndex(-1);
        sal_Int32       nHeaderEndIndex(-1);
        sal_Int32       nFooterStartIndex(-1);
        sal_Int32       nFooterEndIndex(-1);
        sal_Bool        bHeaderStartIndex(sal_False);
        sal_Bool        bHeaderEndIndex(sal_False);
        sal_Bool        bFooterStartIndex(sal_False);
        sal_Bool        bFooterEndIndex(sal_False);

        UniReference< XMLPropertySetMapper > aPropMapper = rPropExp.getPropertySetMapper();

        sal_Int32 nIndex(0);
        while(nIndex < aPropMapper->GetEntryCount())
        {
            switch( aPropMapper->GetEntryContextId( nIndex ) & CTF_PM_FLAGMASK )
            {
                case CTF_PM_HEADERFLAG:
                {
                    if (!bHeaderStartIndex)
                    {
                        nHeaderStartIndex = nIndex;
                        bHeaderStartIndex = sal_True;
                    }
                    if (bFooterStartIndex && !bFooterEndIndex)
                    {
                        nFooterEndIndex = nIndex;
                        bFooterEndIndex = sal_True;
                    }
                }
                break;
                case CTF_PM_FOOTERFLAG:
                {
                    if (!bFooterStartIndex)
                    {
                        nFooterStartIndex = nIndex;
                        bFooterStartIndex = sal_True;
                    }
                    if (bHeaderStartIndex && !bHeaderEndIndex)
                    {
                        nHeaderEndIndex = nIndex;
                        bHeaderEndIndex = sal_True;
                    }
                }
                break;
            }
            nIndex++;
        }
        if (!bHeaderEndIndex)
            nHeaderEndIndex = nIndex;
        if (!bFooterEndIndex)
            nFooterEndIndex = nIndex;

        uno::Reference< xml::sax::XAttributeList > xEmptyList;

        OUString sNameHeader( rNamespaceMap.GetQNameByKey( XML_NAMESPACE_STYLE, OUString::createFromAscii( sXML_header_style ) ) );

        rHandler->ignorableWhitespace( sWS );
        rHandler->startElement( sNameHeader, xEmptyList );
        rPropExp.exportXML(rHandler, rProperties, rUnitConverter, rNamespaceMap, nHeaderStartIndex, nHeaderEndIndex, XML_EXPORT_FLAG_IGN_WS);
        rHandler->ignorableWhitespace( sWS );
        rHandler->endElement( sNameHeader );

        OUString sNameFooter( rNamespaceMap.GetQNameByKey( XML_NAMESPACE_STYLE, OUString::createFromAscii( sXML_footer_style ) ) );

        rHandler->ignorableWhitespace( sWS );
        rHandler->startElement( sNameFooter, xEmptyList );
        rPropExp.exportXML(rHandler, rProperties, rUnitConverter, rNamespaceMap, nFooterStartIndex, nFooterEndIndex, XML_EXPORT_FLAG_IGN_WS);
        rHandler->ignorableWhitespace( sWS );
        rHandler->endElement( sNameFooter );
    }
}

SvXMLAutoStylePoolP::SvXMLAutoStylePoolP()
{
    pImpl = new SvXMLAutoStylePoolP_Impl;
}

SvXMLAutoStylePoolP::~SvXMLAutoStylePoolP()
{
    delete pImpl;
}

// TODO: romove this
void SvXMLAutoStylePoolP::AddFamily(
        sal_Int32 nFamily,
        const OUString& rStrName,
        SvXMLExportPropertyMapper* pMapper,
        OUString aStrPrefix,
        sal_Bool bAsFamily )
{
    UniReference <SvXMLExportPropertyMapper> xTmp = pMapper;
    AddFamily( nFamily, rStrName, xTmp, aStrPrefix, bAsFamily );
}

void SvXMLAutoStylePoolP::AddFamily(
        sal_Int32 nFamily,
        const OUString& rStrName,
        const UniReference < SvXMLExportPropertyMapper > & rMapper,
        const OUString& rStrPrefix,
        sal_Bool bAsFamily )
{
    pImpl->AddFamily( nFamily, rStrName, rMapper, rStrPrefix, bAsFamily );
}

void SvXMLAutoStylePoolP::RegisterName( sal_Int32 nFamily,
                                         const OUString& rName )
{
    pImpl->RegisterName( nFamily, rName );
}

OUString SvXMLAutoStylePoolP::Add( sal_Int32 nFamily,
                                  const vector< XMLPropertyState >& rProperties )
{
    OUString sEmpty;
    return pImpl->Add( nFamily, sEmpty, rProperties );
}

OUString SvXMLAutoStylePoolP::Add( sal_Int32 nFamily,
                                  const OUString& rParent,
                                  const vector< XMLPropertyState >& rProperties )
{
    return pImpl->Add( nFamily, rParent, rProperties );
}

OUString SvXMLAutoStylePoolP::AddAndCache( sal_Int32 nFamily,
                                  const vector< XMLPropertyState >& rProperties )
{
    OUString sEmpty;
    return pImpl->Add( nFamily, sEmpty, rProperties, sal_True );
}

OUString SvXMLAutoStylePoolP::AddAndCache( sal_Int32 nFamily,
                                  const OUString& rParent,
                                  const vector< XMLPropertyState >& rProperties )
{
    return pImpl->Add( nFamily, rParent, rProperties, sal_True );
}

OUString SvXMLAutoStylePoolP::AddAndCache( sal_Int32 nFamily,
                                  const OUString& rParent )
{
    return pImpl->AddToCache( nFamily, rParent );
}

OUString SvXMLAutoStylePoolP::Find( sal_Int32 nFamily,
                                   const vector< XMLPropertyState >& rProperties ) const
{
    OUString sEmpty;
    return pImpl->Find( nFamily, sEmpty, rProperties );
}

OUString SvXMLAutoStylePoolP::Find( sal_Int32 nFamily,
                                   const OUString& rParent,
                                   const vector< XMLPropertyState >& rProperties ) const
{
    return pImpl->Find( nFamily, rParent, rProperties );
}

OUString SvXMLAutoStylePoolP::FindAndRemoveCached( sal_Int32 nFamily ) const
{
    return pImpl->FindAndRemoveCached( nFamily );
}


void SvXMLAutoStylePoolP::exportXML( sal_Int32 nFamily,
    const uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
    const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap) const
{
    pImpl->exportXML( nFamily, rHandler, rUnitConverter,
                      rNamespaceMap, this);
}

void SvXMLAutoStylePoolP::ClearEntries()
{
    pImpl->ClearEntries();
}
