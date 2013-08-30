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

#include <svx/svdoutl.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdotext.hxx>
#include <editeng/editstat.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/eeitem.hxx>
#include <svl/itempool.hxx>

DBG_NAME(SdrOutliner)

SdrOutliner::SdrOutliner( SfxItemPool* pItemPool, sal_uInt16 nMode )
:   Outliner( pItemPool, nMode ),
    //mpPaintInfoRec( NULL )
    mpVisualizedPage(0)
{
    DBG_CTOR(SdrOutliner,NULL);
}


SdrOutliner::~SdrOutliner()
{
    DBG_DTOR(SdrOutliner,NULL);
}


void SdrOutliner::SetTextObj( const SdrTextObj* pObj )
{
    if( pObj && pObj != mpTextObj.get() )
    {
        SetUpdateMode(sal_False);
        sal_uInt16 nOutlinerMode2 = OUTLINERMODE_OUTLINEOBJECT;
        if ( !pObj->IsOutlText() )
            nOutlinerMode2 = OUTLINERMODE_TEXTOBJECT;
        Init( nOutlinerMode2 );

        SetGlobalCharStretching(100,100);

        sal_uIntPtr nStat = GetControlWord();
        nStat &= ~( EE_CNTRL_STRETCHING | EE_CNTRL_AUTOPAGESIZE );
        SetControlWord(nStat);

        Size aNullSize;
        Size aMaxSize( 100000,100000 );
        SetMinAutoPaperSize( aNullSize );
        SetMaxAutoPaperSize( aMaxSize );
        SetPaperSize( aMaxSize );
        ClearPolygon();
    }

    mpTextObj.reset( const_cast< SdrTextObj* >(pObj) );
}

void SdrOutliner::SetTextObjNoInit( const SdrTextObj* pObj )
{
    mpTextObj.reset( const_cast< SdrTextObj* >(pObj) );
}

OUString SdrOutliner::CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos,
                                     Color*& rpTxtColor, Color*& rpFldColor)
{
    bool bOk = false;
    OUString aRet;

    if(mpTextObj.is())
        bOk = static_cast< SdrTextObj* >( mpTextObj.get())->CalcFieldValue(rField, nPara, nPos, sal_False, rpTxtColor, rpFldColor, aRet);

    if (!bOk)
        aRet = Outliner::CalcFieldValue(rField, nPara, nPos, rpTxtColor, rpFldColor);

    return aRet;
}

const SdrTextObj* SdrOutliner::GetTextObj() const
{
    if( mpTextObj.is() )
        return static_cast< SdrTextObj* >( mpTextObj.get() );
    else
        return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
