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



#ifndef _XMLOFF_XMLELEMENTPROPERTYCONTEXT_HXX
#define _XMLOFF_XMLELEMENTPROPERTYCONTEXT_HXX

#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlictxt.hxx>

class SvXMLImport;

namespace rtl
{
    class OUString;
}

class XMLElementPropertyContext : public SvXMLImportContext
{
    sal_Bool        bInsert;

protected:

    ::std::vector< XMLPropertyState > &rProperties;
    XMLPropertyState aProp;

    sal_Bool IsInsert() const { return bInsert; }
    void SetInsert( sal_Bool bIns ) { bInsert = bIns; }

public:
    XMLElementPropertyContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                               const ::rtl::OUString& rLName,
                               const XMLPropertyState& rProp,
                                ::std::vector< XMLPropertyState > &rProps );

    virtual ~XMLElementPropertyContext();

    virtual void EndElement();
};


#endif  //  _XMLOFF_ELEMENTPROPERTYCONTEXT_HXX

