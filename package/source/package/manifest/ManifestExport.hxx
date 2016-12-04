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



#ifndef _MANIFEST_EXPORT_HXX
#define _MANIFEST_EXPORT_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue;}
    namespace xml { namespace sax { class XDocumentHandler; } }
} } }
class ManifestExport
{
public:
    ManifestExport(com::sun::star::uno::Reference < com::sun::star::xml::sax::XDocumentHandler > xHandler, const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > > &rManList );
};

#endif

