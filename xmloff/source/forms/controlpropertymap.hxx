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



#ifndef _XMLOFF_FORMS_CONTROLPROPERTYMAP_HXX_
#define _XMLOFF_FORMS_CONTROLPROPERTYMAP_HXX_

#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlexppr.hxx>

struct XMLPropertyMapEntry;
//.........................................................................
namespace xmloff
{
//.........................................................................

    const XMLPropertyMapEntry* getControlStylePropertyMap( );

    void initializePropertyMaps();

    //=====================================================================
    //= OFormComponentStyleExportMapper
    //=====================================================================
    class OFormComponentStyleExportMapper : public SvXMLExportPropertyMapper
    {
    public:
        OFormComponentStyleExportMapper( const UniReference< XMLPropertySetMapper >& _rMapper );

        void handleSpecialItem(
            SvXMLAttributeList&                         _rAttrList,
            const XMLPropertyState&                     _rProperty,
            const SvXMLUnitConverter&                   _rUnitConverter,
            const SvXMLNamespaceMap&                    _rNamespaceMap,
            const ::std::vector< XMLPropertyState >*    _pProperties,
            sal_uInt32                                  _nIdx
        ) const;
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_CONTROLPROPERTYMAP_HXX_


