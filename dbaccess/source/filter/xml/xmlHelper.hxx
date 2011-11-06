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


#ifndef DBA_XMLHELPER_HXX
#define DBA_XMLHELPER_HXX

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include <xmloff/xmlprmap.hxx>
#endif
#ifndef _XMLOFF_CONTEXTID_HXX_
#include <xmloff/contextid.hxx>
#endif
#ifndef _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_
#include <xmloff/controlpropertyhdl.hxx>
#endif

#include <memory>

#define CTF_DB_ROWHEIGHT                            (XML_DB_CTF_START + 1)
#define CTF_DB_ISVISIBLE                            (XML_DB_CTF_START + 2)
#define CTF_DB_MASTERPAGENAME                       (XML_DB_CTF_START + 3)
#define CTF_DB_NUMBERFORMAT                         (XML_DB_CTF_START + 4)
#define CTF_DB_COLUMN_TEXT_ALIGN                    (XML_DB_CTF_START + 5)

#define XML_DB_TYPE_EQUAL                           (XML_DB_TYPES_START + 1)

namespace dbaxml
{
    class OPropertyHandlerFactory : public ::xmloff::OControlPropertyHandlerFactory
    {
    protected:
        mutable ::std::auto_ptr<XMLConstantsPropertyHandler>    m_pDisplayHandler;
        mutable ::std::auto_ptr<XMLPropertyHandler>             m_pTextAlignHandler;
    public:
        OPropertyHandlerFactory();
        virtual ~OPropertyHandlerFactory();

        virtual const XMLPropertyHandler* GetPropertyHandler(sal_Int32 _nType) const;
    };

    class OXMLHelper
    {
    public:
        static UniReference < XMLPropertySetMapper > GetTableStylesPropertySetMapper();
        static UniReference < XMLPropertySetMapper > GetColumnStylesPropertySetMapper();
        static UniReference < XMLPropertySetMapper > GetCellStylesPropertySetMapper();
        static UniReference < XMLPropertySetMapper > GetRowStylesPropertySetMapper();
    };
// -----------------------------------------------------------------------------
} // dbaxml
// -----------------------------------------------------------------------------
#endif // DBA_XMLHELPER_HXX

