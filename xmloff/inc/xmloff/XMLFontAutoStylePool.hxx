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



#ifndef _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX
#define _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <rtl/ustring.hxx>
#include <xmloff/uniref.hxx>

class XMLFontAutoStylePool_Impl;
class XMLFontAutoStylePoolNames_Impl;
class SvXMLExport;

class XMLOFF_DLLPUBLIC XMLFontAutoStylePool : public UniRefBase
{
    SvXMLExport& rExport;

    XMLFontAutoStylePool_Impl *pPool;
    XMLFontAutoStylePoolNames_Impl *pNames;
    sal_uInt32 nName;

protected:

    SvXMLExport& GetExport() { return rExport; }

public:

    XMLFontAutoStylePool( SvXMLExport& rExport );
    ~XMLFontAutoStylePool();

    ::rtl::OUString Add(
            const ::rtl::OUString& rFamilyName,
            const ::rtl::OUString& rStyleName,
            sal_Int16 nFamily,
            sal_Int16 nPitch,
            rtl_TextEncoding eEnc );

    ::rtl::OUString Find(
            const ::rtl::OUString& rFamilyName,
            const ::rtl::OUString& rStyleName,
            sal_Int16 nFamily,
            sal_Int16 nPitch,
            rtl_TextEncoding eEnc )const;
    ::rtl::OUString Find( const ::rtl::OUString& rInternalName ) const;

    void exportXML();
};

#endif  //  _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX
