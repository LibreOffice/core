/*************************************************************************
 *
 *  $RCSfile: txtexppr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-18 11:18:30 $
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

#ifndef _XMLOFF_TEXTPRMAP_HXX_
#include "txtprmap.hxx"
#endif

#ifndef _XMLOFF_TXTEXPPR_HXX
#include "txtexppr.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;

void XMLTextExportPropertySetMapper::handleElementItem(
        const Reference< xml::sax::XDocumentHandler > & rHandler,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt16 nFlags ) const
{
    XMLTextExportPropertySetMapper *pThis =
           ((XMLTextExportPropertySetMapper *)this);

    switch( getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex ) )
    {
    case CTF_DROPCAPFORMAT:
        pThis->maDropCapExport.exportXML( rProperty.maValue, bDropWholeWord,
                                          sDropCharStyle, rNamespaceMap );
        pThis->bDropWholeWord = sal_False;
        pThis->sDropCharStyle = OUString();
        break;

    case CTF_TABSTOP:
        pThis->maTabStopExport.Export( rProperty.maValue, rNamespaceMap );
        break;

    case CTF_TEXTCOLUMNS:
        pThis->maTextColumnsExport.exportXML( rProperty.maValue );
        break;

    default:
        DBG_ASSERT( !this, "unknown element property" );
        break;
    }
}

void XMLTextExportPropertySetMapper::handleSpecialItem(
        SvXMLAttributeList& rAttrList,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    XMLTextExportPropertySetMapper *pThis =
           ((XMLTextExportPropertySetMapper *)this);

    switch( getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex ) )
    {
    case CTF_DROPCAPWHOLEWORD:
        DBG_ASSERT( !bDropWholeWord, "drop whole word is set already!" );
        pThis->bDropWholeWord = *(sal_Bool *)rProperty.maValue.getValue();
        break;
    case CTF_DROPCAPCHARSTYLE:
        DBG_ASSERT( !sDropCharStyle.getLength(),
                    "drop char style is set already!" );
        rProperty.maValue >>= pThis->sDropCharStyle;
        break;
    case CTF_NUMBERINGSTYLENAME:
    case CTF_PAGEDESCNAME:
    case CTF_OLDTEXTBACKGROUND:
        // There's nothing to do here!
        break;
    default:
        DBG_ASSERT( !this, "unknown special property" );
        break;
    }
}

XMLTextExportPropertySetMapper::XMLTextExportPropertySetMapper(
        const UniReference< XMLPropertySetMapper >& rMapper,
        SvXMLExport& rExport ) :
    SvXMLExportPropertyMapper( rMapper ),
    bDropWholeWord( sal_False ),
    maTabStopExport( rExport.GetDocHandler(), rExport.GetMM100UnitConverter() ),
    maDropCapExport( rExport.GetDocHandler(), rExport.GetMM100UnitConverter() ),
    maTextColumnsExport( rExport )
{
}

XMLTextExportPropertySetMapper::~XMLTextExportPropertySetMapper()
{
}




