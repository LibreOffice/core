/*************************************************************************
 *
 *  $RCSfile: PageMasterExportPropMapper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-26 08:35:06 $
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

#ifndef _XMLOFF_PAGEMASTEREXPORTPROPMAPPER_HXX
#include "PageMasterExportPropMapper.hxx"
#endif

#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include "PageMasterStyleMap.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


//______________________________________________________________________________

XMLPageMasterExportPropMapper::XMLPageMasterExportPropMapper(
        const UniReference< XMLPropertySetMapper >& rMapper,
        SvXMLExport& rExport ) :
    SvXMLExportPropertyMapper( rMapper ),
    aBackgroundImageExport( rExport ),
    aTextColumnsExport( rExport )
{
}

XMLPageMasterExportPropMapper::~XMLPageMasterExportPropMapper()
{
}

void XMLPageMasterExportPropMapper::handleElementItem(
        const uno::Reference< xml::sax::XDocumentHandler >& rHandler,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt16 nFlags,
        const ::std::vector< XMLPropertyState >* pProperties,
        sal_uInt32 nIdx ) const
{
    XMLPageMasterExportPropMapper* pThis = (XMLPageMasterExportPropMapper*) this;

    switch( getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex ) )
    {
        case CTF_PM_GRAPHICURL:
        {
            DBG_ASSERT( pProperties && (nIdx >= 2), "property vector missing" );
            const Any*  pPos    = NULL;
            const Any*  pFilter = NULL;
            if( pProperties && (nIdx >= 2) )
            {
                const XMLPropertyState& rPos = (*pProperties)[nIdx - 2];
                DBG_ASSERT( getPropertySetMapper()->GetEntryContextId( rPos.mnIndex ) == CTF_PM_GRAPHICPOSITION,
                            "invalid property map: pos expected" );
                if( getPropertySetMapper()->GetEntryContextId( rPos.mnIndex ) == CTF_PM_GRAPHICPOSITION )
                    pPos = &rPos.maValue;

                const XMLPropertyState& rFilter = (*pProperties)[nIdx - 1];
                DBG_ASSERT( getPropertySetMapper()->GetEntryContextId( rFilter.mnIndex ) == CTF_PM_GRAPHICFILTER,
                            "invalid property map: filter expected" );
                if( getPropertySetMapper()->GetEntryContextId( rFilter.mnIndex ) == CTF_PM_GRAPHICFILTER )
                    pFilter = &rFilter.maValue;
            }
            sal_uInt32 nPropIndex = rProperty.mnIndex;
            pThis->aBackgroundImageExport.exportXML( rProperty.maValue, pPos, pFilter,
                getPropertySetMapper()->GetEntryNameSpace( nPropIndex ),
                getPropertySetMapper()->GetEntryXMLName( nPropIndex ) );
        }
        case CTF_PM_TEXTCOLUMNS:
            pThis->aTextColumnsExport.exportXML( rProperty.maValue );
            break;
    }
}

void XMLPageMasterExportPropMapper::handleSpecialItem(
        SvXMLAttributeList& rAttrList,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        const ::std::vector< XMLPropertyState >* pProperties,
        sal_uInt32 nIdx) const
{
}

