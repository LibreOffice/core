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



#ifndef _XMLOFF_PERSMIXEDCONTENTTCONTEXT_HXX
#define _XMLOFF_PERSMIXEDCONTENTTCONTEXT_HXX

#include <rtl/ref.hxx>

#include <vector>
#include "DeepTContext.hxx"

class XMLPersMixedContentTContext : public XMLPersElemContentTContext
{
public:
    // mixed content persistence only
    XMLPersMixedContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName );

    // mixed content persistence + attribute processing
    XMLPersMixedContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nActionMap );

    // mixed content persistence + renaming
    XMLPersMixedContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken );

    // mixed content persistence + renaming + attribute processing
    XMLPersMixedContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken,
                              sal_uInt16 nActionMap );

    virtual void Characters( const ::rtl::OUString& rChars );

    virtual ~XMLPersMixedContentTContext();
};

#endif  //  _XMLOFF_PERSMIXEDCONTENTTCONTEXT_HXX

