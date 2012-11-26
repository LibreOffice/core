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



#ifndef _XMLOFF_DEEPTCONTEXT_HXX
#define _XMLOFF_DEEPTCONTEXT_HXX

#include <rtl/ref.hxx>

#include <vector>
#include "PersAttrListTContext.hxx"
#include "TContextVector.hxx"

class XMLPersElemContentTContext : public XMLPersAttrListTContext
{
    XMLTransformerContextVector m_aChildContexts;

public:
    // element content persistence only
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName );

    // element content persistence + attribute processing
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nActionMap );

    // element content persistence + renaming
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken );

    // element content persistence + renaming + attribute processing
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken,
                              sal_uInt16 nActionMap );

    virtual ~XMLPersElemContentTContext();

    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::rtl::OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    void AddContent( XMLTransformerContext *pContent );

    sal_Bool HasElementContent() const
    {
        return static_cast<sal_Bool>( !m_aChildContexts.empty() );
    }

    virtual void ExportContent();
};

#endif  //  _XMLOFF_DEEPTCONTEXT_HXX

