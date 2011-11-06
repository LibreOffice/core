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
#include "precompiled_svx.hxx"


#include "svdoutlinercache.hxx"
#include <svx/svdoutl.hxx>
#include <svx/svdmodel.hxx>

extern SdrOutliner* SdrMakeOutliner( sal_uInt16 nOutlinerMode, SdrModel* pModel );

SdrOutlinerCache::SdrOutlinerCache( SdrModel* pModel )
:   mpModel( pModel ),
    mpModeOutline( NULL ),
    mpModeText( NULL )
{
}

SdrOutliner* SdrOutlinerCache::createOutliner( sal_uInt16 nOutlinerMode )
{
    SdrOutliner* pOutliner = NULL;

    if( (OUTLINERMODE_OUTLINEOBJECT == nOutlinerMode) && mpModeOutline )
    {
        pOutliner = mpModeOutline;
        mpModeOutline = NULL;
    }
    else if( (OUTLINERMODE_TEXTOBJECT == nOutlinerMode) && mpModeText )
    {
        pOutliner = mpModeText;
        mpModeText = NULL;
    }
    else
    {
        pOutliner = SdrMakeOutliner( nOutlinerMode, mpModel );
        Outliner& aDrawOutliner = mpModel->GetDrawOutliner();
        pOutliner->SetCalcFieldValueHdl( aDrawOutliner.GetCalcFieldValueHdl() );
    }

    return pOutliner;
}

SdrOutlinerCache::~SdrOutlinerCache()
{
    if( mpModeOutline )
    {
        delete mpModeOutline;
        mpModeOutline = NULL;
    }

    if( mpModeText )
    {
        delete mpModeText;
        mpModeText = NULL;
    }
}

void SdrOutlinerCache::disposeOutliner( SdrOutliner* pOutliner )
{
    if( pOutliner )
    {
        sal_uInt16 nOutlMode = pOutliner->GetOutlinerMode();

        if( (OUTLINERMODE_OUTLINEOBJECT == nOutlMode) && (NULL == mpModeOutline) )
        {
            mpModeOutline = pOutliner;
            pOutliner->Clear();
            pOutliner->SetVertical( false );

            // #101088# Deregister on outliner, might be reused from outliner cache
            pOutliner->SetNotifyHdl( Link() );
        }
        else if( (OUTLINERMODE_TEXTOBJECT == nOutlMode) && (NULL == mpModeText) )
        {
            mpModeText = pOutliner;
            pOutliner->Clear();
            pOutliner->SetVertical( false );

            // #101088# Deregister on outliner, might be reused from outliner cache
            pOutliner->SetNotifyHdl( Link() );
        }
        else
        {
            delete pOutliner;
        }
    }
}


