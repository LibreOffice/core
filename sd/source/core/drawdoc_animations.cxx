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

#include "drawdoc.hxx"
#include "cusshow.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;

/** replaces a slide from all custom shows with a new one or removes a slide from
    all custom shows if pNewPage is 0.
*/
void SdDrawDocument::ReplacePageInCustomShows( const SdPage* pOldPage, const SdPage* pNewPage )
{
    if ( mpCustomShowList )
    {
        for (sal_uLong i = 0; i < mpCustomShowList->Count(); i++)
        {
            SdCustomShow* pCustomShow = (SdCustomShow*) mpCustomShowList->GetObject(i);
            if( pNewPage == 0 )
                pCustomShow->RemovePage(pOldPage);
            else
                pCustomShow->ReplacePage(pOldPage,pNewPage);
        }
    }
}

extern Reference< XPresentation2 > CreatePresentation( const SdDrawDocument& rDocument );

const Reference< XPresentation2 >& SdDrawDocument::getPresentation() const
{
    if( !mxPresentation.is() )
    {
        const_cast< SdDrawDocument* >( this )->mxPresentation = CreatePresentation(*this);
    }
    return mxPresentation;
}
