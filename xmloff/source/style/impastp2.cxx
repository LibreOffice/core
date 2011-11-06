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
#include <rtl/ustrbuf.hxx>
#include "impastpl.hxx"

using namespace std;
using namespace rtl;

//#############################################################################
//
// Class SvXMLAutoStylePoolProperties_Impl
//

///////////////////////////////////////////////////////////////////////////////
//
// ctor class SvXMLAutoStylePoolProperties_Impl
//

SvXMLAutoStylePoolPropertiesP_Impl::SvXMLAutoStylePoolPropertiesP_Impl( XMLFamilyData_Impl* pFamilyData, const vector< XMLPropertyState >& rProperties )
: maProperties( rProperties ),
  mnPos       ( pFamilyData->mnCount )
{
    // create a name that hasn't been used before. The created name has not
    // to be added to the array, because it will never tried again
    OUStringBuffer sBuffer( 7 );
    do
    {
        pFamilyData->mnName++;
        sBuffer.append( pFamilyData->maStrPrefix );
        sBuffer.append( OUString::valueOf( (sal_Int32)pFamilyData->mnName ) );
        msName = sBuffer.makeStringAndClear();
    }
    while( pFamilyData->mpNameList->Seek_Entry( &msName, 0 ) );
}
