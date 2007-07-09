/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlAutoStyle.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:17 $
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
#include "precompiled_reportdesign.hxx"

#ifndef RPT_XMLAUTOSTYLE_HXX
#include "xmlAutoStyle.hxx"
#endif
#ifndef RPT_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif
#ifndef RPT_XMLEXPORT_HXX
#include "xmlExport.hxx"
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

namespace rptxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

void OXMLAutoStylePoolP::exportStyleAttributes(
            SvXMLAttributeList& rAttrList,
            sal_Int32 nFamily,
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp
            , const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const
{
    SvXMLAutoStylePoolP::exportStyleAttributes( rAttrList, nFamily, rProperties, rPropExp, rUnitConverter, rNamespaceMap );
    if ( nFamily == XML_STYLE_FAMILY_TABLE_CELL )
    {
        UniReference< XMLPropertySetMapper > aPropMapper = rORptExport.GetCellStylePropertyMapper();
        ::std::vector< XMLPropertyState >::const_iterator i = rProperties.begin();
        ::std::vector< XMLPropertyState >::const_iterator aEnd = rProperties.end();
        for (; i != aEnd ; ++i)
        {
            sal_Int16 nContextID = aPropMapper->GetEntryContextId(i->mnIndex);
            switch (nContextID)
            {
                case CTF_RPT_NUMBERFORMAT :
                {
                    rtl::OUString sAttrValue;
                    if ( i->maValue >>= sAttrValue )
                    {
                        if ( sAttrValue.getLength() )
                        {
                            rORptExport.AddAttribute(
                                aPropMapper->GetEntryNameSpace(i->mnIndex),
                                aPropMapper->GetEntryXMLName(i->mnIndex),
                                sAttrValue );
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}
DBG_NAME( rpt_OXMLAutoStylePoolP )
// -----------------------------------------------------------------------------
OXMLAutoStylePoolP::OXMLAutoStylePoolP(ORptExport& rTempORptExport):
    SvXMLAutoStylePoolP(rTempORptExport),
    rORptExport(rTempORptExport)
{
    DBG_CTOR( rpt_OXMLAutoStylePoolP,NULL);

}
// -----------------------------------------------------------------------------
OXMLAutoStylePoolP::~OXMLAutoStylePoolP()
{

    DBG_DTOR( rpt_OXMLAutoStylePoolP,NULL);
}
// -----------------------------------------------------------------------------

//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
