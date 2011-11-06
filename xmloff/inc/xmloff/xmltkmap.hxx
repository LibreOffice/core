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



#ifndef _XMLOFF_XMLTKMAP_HXX
#define _XMLOFF_XMLTKMAP_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <sal/types.h>
#include <xmloff/xmltoken.hxx>

namespace rtl { class OUString; }

class SvXMLTokenMap_Impl;
class SvXMLTokenMapEntry_Impl;

#define XML_TOK_UNKNOWN 0xffffU
#define XML_TOKEN_MAP_END { 0xffffU, ::xmloff::token::XML_TOKEN_INVALID, 0U }

struct SvXMLTokenMapEntry
{
    sal_uInt16  nPrefixKey;
    enum ::xmloff::token::XMLTokenEnum eLocalName;
    sal_uInt16  nToken;
};

class XMLOFF_DLLPUBLIC SvXMLTokenMap
{
    SvXMLTokenMap_Impl      *pImpl;

    SAL_DLLPRIVATE SvXMLTokenMapEntry_Impl *_Find(
        sal_uInt16 nKind, const ::rtl::OUString& rName ) const;

public:

    SvXMLTokenMap( const SvXMLTokenMapEntry* pMap );
    ~SvXMLTokenMap();

    sal_uInt16 Get( sal_uInt16 nPrefix, const ::rtl::OUString& rLName ) const;
};

#endif  //  _XMLOFF_XMLTKMAP_HXX

