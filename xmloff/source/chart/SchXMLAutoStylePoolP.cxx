/*************************************************************************
 *
 *  $RCSfile: SchXMLAutoStylePoolP.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bm $ $Date: 2001-02-14 17:11:24 $
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
#include "SchXMLAutoStylePoolP.hxx"
#include "PropertyMap.hxx"

#ifndef SCH_XMLEXPORT_HXX_
#include "SchXMLExport.hxx"
#endif
#ifndef _XMLOFF_CHARTPROPERTYSETMAPPER_HXX_
#include "XMLChartPropertySetMapper.hxx"
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

SchXMLAutoStylePoolP::SchXMLAutoStylePoolP( SchXMLExport& rSchXMLExport ) :
        mrSchXMLExport( rSchXMLExport )
{}

SchXMLAutoStylePoolP::~SchXMLAutoStylePoolP()
{}

void SchXMLAutoStylePoolP::exportStyleAttributes(
    SvXMLAttributeList& rAttrList,
    sal_Int32 nFamily,
    const ::std::vector< XMLPropertyState >& rProperties,
    const SvXMLExportPropertyMapper& rPropExp,
    const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap ) const
{
    const rtl::OUString sCDATA( RTL_CONSTASCII_USTRINGPARAM( sXML_CDATA ));
    SvXMLAutoStylePoolP::exportStyleAttributes( rAttrList, nFamily, rProperties,
                                                rPropExp, rUnitConverter, rNamespaceMap );

    if( nFamily == XML_STYLE_FAMILY_SCH_CHART_ID )
    {
        ::std::vector< XMLPropertyState >::const_iterator iter = rProperties.begin();
        for( iter; (iter != rProperties.end()); iter++ )
        {
            UniReference< XMLPropertySetMapper > aPropMapper =
                mrSchXMLExport.GetPropertySetMapper();
            sal_Int16 nContextID = aPropMapper->GetEntryContextId( iter->mnIndex );
            if( nContextID == XML_SCH_CONTEXT_SPECIAL_NUMBER_FORMAT )
            {
                sal_Int32 nNumberFormat;
                if( iter->maValue >>= nNumberFormat )
                {
                    rtl::OUString sAttrValue = mrSchXMLExport.getDataStyleName( nNumberFormat );
                    if( sAttrValue.getLength() )
                    {
                        rtl::OUString sAttrName( rNamespaceMap.GetQNameByKey(
                            aPropMapper->GetEntryNameSpace( iter->mnIndex ),
                            aPropMapper->GetEntryXMLName( iter->mnIndex ) ));
                        rAttrList.AddAttribute( sAttrName, sCDATA, sAttrValue );
                    }
                }
            }
        }
    }
}
