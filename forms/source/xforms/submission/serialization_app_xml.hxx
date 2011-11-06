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



#ifndef __SERIALIZATION_APP_XML_HXX
#define __SERIALIZATION_APP_XML_HXX

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "serialization.hxx"

#include <comphelper/componentcontext.hxx>

class CSerializationAppXML : public CSerialization
{
private:
    ::comphelper::ComponentContext                  m_aContext;
    CSS::uno::Reference< CSS::io::XOutputStream >   m_xBuffer;

    void serialize_node(const CSS::uno::Reference< CSS::xml::dom::XNode >& aNode);
    void serialize_nodeset();

public:
    CSerializationAppXML();

    virtual void serialize();
    //    virtual void setSource(const CSS::uno::Reference< CSS::xml::xpath::XXPathObject >& object);
    //    virtual void setProperties(const CSS::uno::Sequence< CSS::beans::NamedValue >& props);
    virtual CSS::uno::Reference< CSS::io::XInputStream > getInputStream();
};

#endif
