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



#ifndef _XMLBACKGROUNDIMAGEEXPORT_HXX
#define _XMLBACKGROUNDIMAGEEXPORT_HXX

#ifndef _SAL_TYPES_H
#include <sal/types.h>
#endif

namespace rtl { class OUString; }
namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }

class SvXMLExport;

class XMLBackgroundImageExport
{
    SvXMLExport&        rExport;

protected:

    SvXMLExport& GetExport() { return rExport; }
public:

    XMLBackgroundImageExport( SvXMLExport& rExport );

    ~XMLBackgroundImageExport();

    void exportXML( const ::com::sun::star::uno::Any& rURL,
                    const ::com::sun::star::uno::Any *pPos,
                    const ::com::sun::star::uno::Any *pFilter,
                    const ::com::sun::star::uno::Any *pTransparency,
                    sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName );
};


#endif

