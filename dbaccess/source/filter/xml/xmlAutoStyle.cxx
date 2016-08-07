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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaxml.hxx"
#ifndef DBA_XMLAUTOSTYLE_HXX
#include "xmlAutoStyle.hxx"
#endif
#ifndef DBA_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif
#ifndef DBA_XMLEXPORT_HXX
#include "xmlExport.hxx"
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
namespace dbaxml
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
    if ( nFamily == XML_STYLE_FAMILY_TABLE_COLUMN )
    {
        UniReference< XMLPropertySetMapper > aPropMapper = rODBExport.GetColumnStylesPropertySetMapper();
        ::std::vector< XMLPropertyState >::const_iterator i = rProperties.begin();
        ::std::vector< XMLPropertyState >::const_iterator aEnd = rProperties.end();
        for ( ; i != aEnd ; ++i )
        {
            sal_Int16 nContextID = aPropMapper->GetEntryContextId(i->mnIndex);
            switch (nContextID)
            {
                case CTF_DB_NUMBERFORMAT :
                {
                    sal_Int32 nNumberFormat = 0;
                    if ( i->maValue >>= nNumberFormat )
                    {
                        rtl::OUString sAttrValue = rODBExport.getDataStyleName(nNumberFormat);
                        if ( sAttrValue.getLength() )
                        {
                            GetExport().AddAttribute(
                                aPropMapper->GetEntryNameSpace(i->mnIndex),
                                aPropMapper->GetEntryXMLName(i->mnIndex),
                                sAttrValue );
                        }
                    }
                    break;
                }
            }
        }
    }
}
DBG_NAME(OXMLAutoStylePoolP)
// -----------------------------------------------------------------------------
OXMLAutoStylePoolP::OXMLAutoStylePoolP(ODBExport& rTempODBExport):
    SvXMLAutoStylePoolP(rTempODBExport),
    rODBExport(rTempODBExport)
{
    DBG_CTOR(OXMLAutoStylePoolP,NULL);

}
// -----------------------------------------------------------------------------
OXMLAutoStylePoolP::~OXMLAutoStylePoolP()
{

    DBG_DTOR(OXMLAutoStylePoolP,NULL);
}
// -----------------------------------------------------------------------------

//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
