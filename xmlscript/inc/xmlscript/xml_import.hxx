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


#if ! defined _XMLSCRIPT_XML_IMPORT_HXX_
#define _XMLSCRIPT_XML_IMPORT_HXX_

#include "com/sun/star/xml/input/XRoot.hpp"
#include "com/sun/star/xml/sax/XExtendedDocumentHandler.hpp"


namespace xmlscript
{

/*##############################################################################

    IMPORTING

##############################################################################*/

/** Creates a document handler to be used for SAX1 parser that can handle
    namespaces.  Namespace URI are mapped to integer ids for performance.
    Implementing the XImporter interface, you will get a startRootElement()
    for the root element of your XML document and subsequent
    startChildElement() callbacks for each sub element.
    Namespaces of tags are identified by their integer value.

    @param xRoot
           initial object being called for root context
    @param bSingleThreadedUse
           flag whether context management is synchronized.
    @return
            document handler for parser
*/
::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL createDocumentHandler(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::input::XRoot > const & xRoot,
    bool bSingleThreadedUse = true )
    SAL_THROW( () );

}

#endif
