/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "svdoutlinercache.hxx"
#include <svx/svdoutl.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdetc.hxx>

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

            // Deregister on outliner, might be reused from outliner cache
            pOutliner->SetNotifyHdl( Link() );
        }
        else if( (OUTLINERMODE_TEXTOBJECT == nOutlMode) && (NULL == mpModeText) )
        {
            mpModeText = pOutliner;
            pOutliner->Clear();
            pOutliner->SetVertical( false );

            // Deregister on outliner, might be reused from outliner cache
            pOutliner->SetNotifyHdl( Link() );
        }
        else
        {
            delete pOutliner;
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
