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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <services/mediatypedetectionhelper.hxx>
#include <services.h>
#include <svl/inettype.hxx>
#include <tools/string.hxx>
#include <rtl/logfile.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework
{

using namespace ::com::sun::star        ;
using namespace ::rtl                   ;

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
MediaTypeDetectionHelper::MediaTypeDetectionHelper( const uno::Reference< lang::XMultiServiceFactory >& xFactory )
    : m_xFactory( xFactory )
{
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
MediaTypeDetectionHelper::~MediaTypeDetectionHelper()
{
}

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   MediaTypeDetectionHelper
                                        , ::cppu::OWeakObject
                                        , SERVICENAME_MEDIATYPEDETECTIONHELPER
                                        , IMPLEMENTATIONNAME_MEDIATYPEDETECTIONHELPER
                                    )

DEFINE_INIT_SERVICE                 (   MediaTypeDetectionHelper,
                                        {
                                        }
                                    )

//*****************************************************************************************************************
//   XStringMapping
//*****************************************************************************************************************

//virtual
sal_Bool SAL_CALL MediaTypeDetectionHelper::mapStrings(
        uno::Sequence< OUString >& rSeq )
        throw(uno::RuntimeException)
{
    sal_Bool bModified = sal_False;
    for( sal_Int32 i = rSeq.getLength(); i--; )
    {

        OUString& rUrl = rSeq[i];
        INetContentType eType = INetContentTypes::GetContentTypeFromURL( rUrl );

        UniString aType( INetContentTypes::GetContentType( eType ) );
        if( aType.Len() )
        {
            rUrl = aType;
            bModified = sal_True;
        }
    }
    return bModified;
}

}   // namespace framework

