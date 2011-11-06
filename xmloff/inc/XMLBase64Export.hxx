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


#ifndef _XMLOFF_XMLBASE64EXPORT_HXX
#define _XMLOFF_XMLBASE64EXPORT_HXX

#include <com/sun/star/uno/Reference.h>

namespace com { namespace sun { namespace star { namespace io {
    class XInputStream; } } } }
class SvXMLExport;

class XMLBase64Export
{
    SvXMLExport&        rExport;

protected:

    SvXMLExport& GetExport() { return rExport; }

public:

    XMLBase64Export( SvXMLExport& rExport );

    sal_Bool exportXML( const ::com::sun::star::uno::Reference <
            ::com::sun::star::io::XInputStream > & rIn );
    sal_Bool exportElement( const ::com::sun::star::uno::Reference <
            ::com::sun::star::io::XInputStream > & rIn,
            sal_uInt16 nNamespace,
            enum ::xmloff::token::XMLTokenEnum eName );
    sal_Bool exportOfficeBinaryDataElement(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::io::XInputStream > & rIn );
};


#endif
