/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
