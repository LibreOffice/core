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
#include <svx/svdoutl.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdotext.hxx>
#include <editeng/editstat.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/eeitem.hxx>
#include <svl/itempool.hxx>

DBG_NAME(SdrOutliner)
/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
SdrOutliner::SdrOutliner( SfxItemPool* pItemPool, sal_uInt16 nMode )
:   Outliner( pItemPool, nMode ),
    //mpPaintInfoRec( NULL )
    mpVisualizedPage(0)
{
    DBG_CTOR(SdrOutliner,NULL);
}


/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/
SdrOutliner::~SdrOutliner()
{
    DBG_DTOR(SdrOutliner,NULL);
}


/*************************************************************************
|*
|*
|*
\************************************************************************/
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

/*************************************************************************
|*
|*
|*
\************************************************************************/
void SdrOutliner::SetTextObjNoInit( const SdrTextObj* pObj )
{
    mpTextObj.reset( const_cast< SdrTextObj* >(pObj) );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/
XubString SdrOutliner::CalcFieldValue(const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos,
                                     Color*& rpTxtColor, Color*& rpFldColor)
{
    FASTBOOL bOk = sal_False;
    XubString aRet;

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
