/*************************************************************************
 *
 *  $RCSfile: xmlitem.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mib $ $Date: 2001-07-04 14:14:57 $
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

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif

#ifndef _XMLIMPIT_HXX
#include "xmlimpit.hxx"
#endif

#ifndef _XMLITEM_HXX
#include "xmlitem.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

SvXMLItemSetContext::SvXMLItemSetContext( SvXMLImport& rImp, USHORT nPrfx,
                                          const OUString& rLName,
                                          const uno::Reference< xml::sax::XAttributeList >& xAttrList,
                                          SfxItemSet& rISet,
                                          const SvXMLImportItemMapper& rIMap,
                                          const SvXMLUnitConverter& rUnitConverter ):
    SvXMLImportContext( rImp, nPrfx, rLName ),
    rItemSet( rISet ),
    rIMapper( rIMap ),
    rUnitConv( rUnitConverter )

{
    rIMapper.importXML( rItemSet, xAttrList, rUnitConv,
                           GetImport().GetNamespaceMap() );
}

SvXMLItemSetContext::~SvXMLItemSetContext()
{
}

SvXMLImportContext *SvXMLItemSetContext::CreateChildContext( USHORT nPrefix,
                                            const OUString& rLocalName,
                                            const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLItemMapEntriesRef xMapEntries = rIMapper.getMapEntries();
    SvXMLItemMapEntry* pEntry = xMapEntries->getByName( nPrefix, rLocalName );

    if( pEntry && 0 != (pEntry->nMemberId & MID_FLAG_ELEMENT_ITEM_IMPORT) )
    {
        return CreateChildContext( nPrefix, rLocalName, xAttrList,
                                   rItemSet, *pEntry, rUnitConv );
    }
    else
    {
        return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }
}

/** This method is called from this instance implementation of
    CreateChildContext if the element matches an entry in the
    SvXMLImportItemMapper with the mid flag MID_FLAG_ELEMENT
*/
SvXMLImportContext *SvXMLItemSetContext::CreateChildContext( USHORT nPrefix,
                                   const rtl::OUString& rLocalName,
                                   const uno::Reference< xml::sax::XAttributeList >& xAttrList,
                                    SfxItemSet&  rItemSet,
                                   const SvXMLItemMapEntry& rEntry,
                                   const SvXMLUnitConverter& rUnitConv )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}


