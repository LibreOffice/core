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
#include "precompiled_forms.hxx"

#include "resourcehelper.hxx"
#include "frm_resource.hxx"

#include <rtl/ustring.hxx>
#include <tools/string.hxx>

using rtl::OUString;

#define OUSTRING(x) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(x))

namespace xforms
{

OUString getResource( sal_uInt16 nResourceId )
{
    return getResource( nResourceId, OUString(), OUString(), OUString() );
}

OUString getResource( sal_uInt16 nResourceId,
                      const OUString& rInfo1 )
{
    return getResource( nResourceId, rInfo1, OUString(), OUString() );
}

OUString getResource( sal_uInt16 nResourceId,
                      const OUString& rInfo1,
                      const OUString& rInfo2 )
{
    return getResource( nResourceId, rInfo1, rInfo2, OUString() );
}

OUString getResource( sal_uInt16 nResourceId,
                      const OUString& rInfo1,
                      const OUString& rInfo2,
                      const OUString& rInfo3 )
{
    OUString sResource = frm::ResourceManager::loadString( nResourceId );
    OSL_ENSURE( sResource.getLength() > 0, "resource not found?" );

    // use old style String class for search and replace, so we don't have to
    // code this again.
    String sString( sResource );
    sString.SearchAndReplaceAll( String(OUSTRING("$1")), String(rInfo1) );
    sString.SearchAndReplaceAll( String(OUSTRING("$2")), String(rInfo2) );
    sString.SearchAndReplaceAll( String(OUSTRING("$3")), String(rInfo3) );
    return OUString( sString );
}

}   // namespace xforms
