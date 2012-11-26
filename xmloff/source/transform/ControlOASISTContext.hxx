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



#ifndef _XMLOFF_CONTROLASISTCONTEXT_HXX
#define _XMLOFF_CONTROLASISTCONTEXT_HXX

#include "TransformerContext.hxx"

class XMLControlOASISTransformerContext : public XMLTransformerContext
{
    ::rtl::OUString m_aElemQName;
    sal_Bool m_bCreateControl;

public:
    XMLControlOASISTransformerContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_Bool bCreateControl );

    virtual ~XMLControlOASISTransformerContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();
};

#endif  //  _XMLOFF_CONTROLASISTCONTEXT_HXX

