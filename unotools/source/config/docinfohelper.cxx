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
#include "precompiled_unotools.hxx"

#include <rtl/ustrbuf.hxx>
#include <tools/inetdef.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/docinfohelper.hxx>

using namespace ::com::sun::star;

namespace utl
{

::rtl::OUString DocInfoHelper::GetGeneratorString()
{
    rtl::OUStringBuffer aResult;

    // First product: branded name + version
    // version is <product_versions>_<product_extension>$<platform>
    utl::ConfigManager* pMgr = utl::ConfigManager::GetConfigManager();
    if ( pMgr )
    {
        // plain product name
        rtl::OUString aValue;
        uno::Any aAny = pMgr->GetDirectConfigProperty(
                                            utl::ConfigManager::PRODUCTNAME);
        if ( (aAny >>= aValue) && aValue.getLength() )
        {
            aResult.append( aValue.replace( ' ', '_' ) );
            aResult.append( (sal_Unicode)'/' );

            aAny = pMgr->GetDirectConfigProperty(
                                        utl::ConfigManager::PRODUCTVERSION);
            if ( (aAny >>= aValue) && aValue.getLength() )
            {
                aResult.append( aValue.replace( ' ', '_' ) );

                aAny = pMgr->GetDirectConfigProperty(
                                        utl::ConfigManager::PRODUCTEXTENSION);
                if ( (aAny >>= aValue) && aValue.getLength() )
                {
                    aResult.append( (sal_Unicode)'_' );
                    aResult.append( aValue.replace( ' ', '_' ) );
                }
            }

            aResult.append( (sal_Unicode)'$' );
            aResult.append( ::rtl::OUString::createFromAscii(
                                    TOOLS_INETDEF_OS ).replace( ' ', '_' ) );

            aResult.append( (sal_Unicode)' ' );
        }
    }

    // second product: OpenOffice.org_project/<build_information>
    // build_information has '(' and '[' encoded as '$', ')' and ']' ignored
    // and ':' replaced by '-'
    {
        aResult.appendAscii( "OpenOffice.org_project/" );
        ::rtl::OUString aDefault;
        ::rtl::OUString aBuildId( Bootstrap::getBuildIdData( aDefault ) );
        for( sal_Int32 i=0; i < aBuildId.getLength(); i++ )
        {
            sal_Unicode c = aBuildId[i];
            switch( c )
            {
            case '(':
            case '[':
                aResult.append( (sal_Unicode)'$' );
                break;
            case ')':
            case ']':
                break;
            case ':':
                aResult.append( (sal_Unicode)'-' );
                break;
            default:
                aResult.append( c );
                break;
            }
        }
    }

    return aResult.makeStringAndClear();
}

} // end of namespace utl

