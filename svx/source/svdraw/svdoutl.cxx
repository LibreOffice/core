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
#include <svx/svdoutl.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdotext.hxx>
#include <editeng/editstat.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/eeitem.hxx>
#include <svl/itempool.hxx>

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
SdrOutliner::SdrOutliner( SfxItemPool* pItemPool, sal_uInt16 nMode )
:   Outliner( pItemPool, nMode ),
    mpVisualizedPage(0)
{
}


/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/
SdrOutliner::~SdrOutliner()
{
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
    bool bOk = sal_False;
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

// eof
