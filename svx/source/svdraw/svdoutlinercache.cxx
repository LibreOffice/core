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

// tdf#93994 define from which number of outliners incarnated it should be
// tried to relese these by the owner
#define MAX_NUMBERS_OF_CACHED_OUTLINERS     (20)

SdrOutlinerCache::SdrOutlinerCache( SdrModel* pModel )
:   mpModel( pModel ),
    mpModeOutline( nullptr ),
    mpModeText( nullptr )
{
}

SdrOutliner* SdrOutlinerCache::createOutliner( sal_uInt16 nOutlinerMode )
{
    SdrOutliner* pOutliner = nullptr;

    // tdf#93994 for text object outliners if we have more than MAX_NUMBERS_OF_CACHED_OUTLINERS
    // created, try to release one of these first before trying to create another one
    if((OUTLINERMODE_TEXTOBJECT == nOutlinerMode) && !mpModeText && maActiveOutliners.size() > MAX_NUMBERS_OF_CACHED_OUTLINERS)
    {
        // try to free one of the used outliners
        SdrOutliner* pCandidate = maActiveOutliners.back();

        if(pCandidate)
        {
            pCandidate->tryToReleaseSdrOutliner();
        }
    }

    if( (OUTLINERMODE_OUTLINEOBJECT == nOutlinerMode) && mpModeOutline )
    {
        pOutliner = mpModeOutline;
        mpModeOutline = nullptr;
    }
    else if( (OUTLINERMODE_TEXTOBJECT == nOutlinerMode) && mpModeText )
    {
        pOutliner = mpModeText;
        mpModeText = nullptr;
    }
    else
    {
        pOutliner = SdrMakeOutliner(nOutlinerMode, *mpModel);
        Outliner& aDrawOutliner = mpModel->GetDrawOutliner();
        pOutliner->SetCalcFieldValueHdl( aDrawOutliner.GetCalcFieldValueHdl() );
        maActiveOutliners.push_back(pOutliner);

        // tdf#93994 warn if more than a decent amount (MAX_NUMBERS_OF_CACHED_OUTLINERS) of Outliners exist at the same time.
        // This should never be necessary and is a hint that something is going utterly wrong.
        SAL_WARN_IF(maActiveOutliners.size() > MAX_NUMBERS_OF_CACHED_OUTLINERS, "svx.form", "SdrOutlinerCache: Very many Outliners created in parallell, this should not be necessary" );
    }

    return pOutliner;
}

SdrOutlinerCache::~SdrOutlinerCache()
{
    if( mpModeOutline )
    {
        delete mpModeOutline;
        mpModeOutline = nullptr;
    }

    if( mpModeText )
    {
        delete mpModeText;
        mpModeText = nullptr;
    }
}

void SdrOutlinerCache::disposeOutliner( SdrOutliner* pOutliner )
{
    if( pOutliner )
    {
        sal_uInt16 nOutlMode = pOutliner->GetOutlinerMode();

        if( (OUTLINERMODE_OUTLINEOBJECT == nOutlMode) && (nullptr == mpModeOutline) )
        {
            mpModeOutline = pOutliner;
            pOutliner->Clear();
            pOutliner->SetVertical( false );

            // Deregister on outliner, might be reused from outliner cache
            pOutliner->SetNotifyHdl( Link<EENotify&,void>() );
        }
        else if( (OUTLINERMODE_TEXTOBJECT == nOutlMode) && (nullptr == mpModeText) )
        {
            mpModeText = pOutliner;
            pOutliner->Clear();
            pOutliner->SetVertical( false );

            // Deregister on outliner, might be reused from outliner cache
            pOutliner->SetNotifyHdl( Link<EENotify&,void>() );
        }
        else
        {
            maActiveOutliners.erase(std::remove(maActiveOutliners.begin(), maActiveOutliners.end(), pOutliner), maActiveOutliners.end());
            delete pOutliner;
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
