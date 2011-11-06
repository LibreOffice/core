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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_PAGEMASTERPROPMAPPER_HXX
#include "PageMasterPropMapper.hxx"
#endif


#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include <xmloff/PageMasterStyleMap.hxx>
#endif
#include "PageMasterPropHdlFactory.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;


//______________________________________________________________________________

XMLPageMasterPropSetMapper::XMLPageMasterPropSetMapper():
    XMLPropertySetMapper( aXMLPageMasterStyleMap, new XMLPageMasterPropHdlFactory())
{
}

XMLPageMasterPropSetMapper::XMLPageMasterPropSetMapper(
        const XMLPropertyMapEntry* pEntries,
        const UniReference< XMLPropertyHandlerFactory >& rFactory ) :
    XMLPropertySetMapper( pEntries, rFactory )
{
}

XMLPageMasterPropSetMapper::~XMLPageMasterPropSetMapper()
{
}

