/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include "precompiled_rptxml.hxx"
#include "xmlAutoStyle.hxx"
#include "xmlHelper.hxx"
#include "xmlExport.hxx"
#include <xmloff/families.hxx>
#include <tools/debug.hxx>

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
