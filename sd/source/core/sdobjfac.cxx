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
#include "precompiled_sd.hxx"


#include <basic/sbx.hxx>

#include <app.hxx>
#include "sdpage.hxx"
#include "sdobjfac.hxx"
#include "glob.hxx"
#include "anminfo.hxx"
#include "imapinfo.hxx"
#include "drawdoc.hxx"


SdObjectFactory aSdObjectFactory;

/*************************************************************************
|*
|* void SdObjectFactory::MakeUserData()
|*
\************************************************************************/

IMPL_LINK( SdObjectFactory, MakeUserData, SdrObjFactory *, pObjFactory )
{
    if ( pObjFactory->nInventor == SdUDInventor )
    {
        switch( pObjFactory->nIdentifier )
        {
            case( SD_ANIMATIONINFO_ID ):
                pObjFactory->pNewData = new SdAnimationInfo( *pObjFactory->pObj );
            break;

            case( SD_IMAPINFO_ID ):
                pObjFactory->pNewData = new SdIMapInfo;
            break;

            default:
            break;
        }
    }

    if ( pObjFactory->pNewData )
        return 0;

    if( aOldMakeUserDataLink.IsSet() )
        aOldMakeUserDataLink.Call( this );

    return 0;
}


