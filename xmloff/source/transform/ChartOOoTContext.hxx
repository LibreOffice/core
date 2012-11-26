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



#ifndef _XMLOFF_CHARTOOOTCONTEXT_HXX
#define _XMLOFF_CHARTOOOTCONTEXT_HXX

#include "TransformerContext.hxx"

class XMLChartOOoTransformerContext : public XMLTransformerContext
{
public:
    XMLChartOOoTransformerContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName    );
    virtual ~XMLChartOOoTransformerContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

#endif  //  _XMLOFF_CHARTOOOTCONTEXT_HXX

