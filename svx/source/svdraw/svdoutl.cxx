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

#include <optional>
#include <svx/svdoutl.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpage.hxx>
#include <editeng/editstat.hxx>
#include <svl/itempool.hxx>
#include <editeng/editview.hxx>


SdrOutliner::SdrOutliner( SfxItemPool* pItemPool, OutlinerMode nMode )
:   Outliner( pItemPool, nMode ),
    mpVisualizedPage(nullptr)
{
}


SdrOutliner::~SdrOutliner()
{
}


void SdrOutliner::SetTextObj( const SdrTextObj* pObj )
{
    if( pObj && pObj != mxWeakTextObj.get().get() )
    {
        SetUpdateLayout(false);
        OutlinerMode nOutlinerMode2 = OutlinerMode::OutlineObject;
        if ( !pObj->IsOutlText() )
            nOutlinerMode2 = OutlinerMode::TextObject;
        Init( nOutlinerMode2 );

        resetScalingParameters();

        EEControlBits nStat = GetControlWord();
        nStat &= ~EEControlBits( EEControlBits::STRETCHING | EEControlBits::AUTOPAGESIZE );
        SetControlWord(nStat);

        Size aMaxSize( 100000,100000 );
        SetMinAutoPaperSize( Size() );
        SetMaxAutoPaperSize( aMaxSize );
        SetPaperSize( aMaxSize );
        SetTextColumns(pObj->GetTextColumnsNumber(), pObj->GetTextColumnsSpacing());
        ClearPolygon();
    }

    mxWeakTextObj = const_cast< SdrTextObj* >(pObj);
}

void SdrOutliner::SetTextObjNoInit( const SdrTextObj* pObj )
{
    mxWeakTextObj = const_cast< SdrTextObj* >(pObj);
}

OUString SdrOutliner::CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos,
                                     std::optional<Color>& rpTxtColor, std::optional<Color>& rpFldColor,
                                     std::optional<FontLineStyle>& rpFldLineStyle)
{
    bool bOk = false;
    OUString aRet;

    if(auto pTextObj = mxWeakTextObj.get())
        bOk = pTextObj->CalcFieldValue(rField, nPara, nPos, false, rpTxtColor, rpFldColor, rpFldLineStyle, aRet);

    if (!bOk)
        aRet = Outliner::CalcFieldValue(rField, nPara, nPos, rpTxtColor, rpFldColor, rpFldLineStyle);

    return aRet;
}

const SdrTextObj* SdrOutliner::GetTextObj() const
{
    return mxWeakTextObj.get().get();
}

bool SdrOutliner::hasEditViewCallbacks() const
{
    for (size_t a(0); a < GetViewCount(); a++)
    {
        OutlinerView* pOutlinerView = GetView(a);

        if (pOutlinerView && pOutlinerView->GetEditView().getEditViewCallbacks())
        {
            return true;
        }
    }

    return false;
}

std::optional<bool> SdrOutliner::GetCompatFlag(SdrCompatibilityFlag eFlag) const
{
    if( mpVisualizedPage )
    {
        return {mpVisualizedPage->getSdrModelFromSdrPage().GetCompatibilityFlag(eFlag)};
    }
    return {};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
