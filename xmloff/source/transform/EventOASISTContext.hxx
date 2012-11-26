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



#ifndef _XMLOFF_EVENTOASISTCONTEXT_HXX
#define _XMLOFF_EVENTOASISTCONTEXT_HXX

#include "RenameElemTContext.hxx"

class XMLTransformerOASISEventMap_Impl;

class XMLEventOASISTransformerContext : public XMLRenameElemTransformerContext
{
public:
    XMLEventOASISTransformerContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName );
    virtual ~XMLEventOASISTransformerContext();

    static XMLTransformerOASISEventMap_Impl *CreateFormEventMap();
    static XMLTransformerOASISEventMap_Impl *CreateEventMap();
    static void FlushEventMap( XMLTransformerOASISEventMap_Impl *p );
    static ::rtl::OUString GetEventName( sal_uInt16 nPrefix,
                             const ::rtl::OUString& rName,
                             XMLTransformerOASISEventMap_Impl& rMap,
                             XMLTransformerOASISEventMap_Impl* pMap2    );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

#endif  //  _XMLOFF_EVENTOASISTCONTEXT_HXX

