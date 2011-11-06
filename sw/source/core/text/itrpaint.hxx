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


#ifndef _ITRPAINT_HXX
#define _ITRPAINT_HXX
#include "itrtxt.hxx"

class SwSaveClip;          // SwTxtPainter
class SwMultiPortion;

/*************************************************************************
 *                      class SwTxtPainter
 *************************************************************************/

class SwTxtPainter : public SwTxtCursor
{
    sal_Bool bPaintDrop;

    SwLinePortion *CalcPaintOfst( const SwRect &rPaint );
    void CheckSpecialUnderline( const SwLinePortion* pPor,
                                long nAdjustBaseLine = 0 );
protected:
    void CtorInitTxtPainter( SwTxtFrm *pFrm, SwTxtPaintInfo *pInf );
    inline SwTxtPainter(SwTxtNode* pTxtNode) : SwTxtCursor(pTxtNode) { }

public:
    inline SwTxtPainter( SwTxtFrm *pTxtFrm, SwTxtPaintInfo *pTxtPaintInf ) : SwTxtCursor(pTxtFrm!=NULL?pTxtFrm->GetTxtNode():NULL)
           { CtorInitTxtPainter( pTxtFrm, pTxtPaintInf ); }
    void DrawTextLine( const SwRect &rPaint, SwSaveClip &rClip,
                       const sal_Bool bUnderSz );
    void PaintDropPortion();
    // if PaintMultiPortion is called recursively, we have to pass the
    // surrounding SwBidiPortion
    void PaintMultiPortion( const SwRect &rPaint, SwMultiPortion& rMulti,
                            const SwMultiPortion* pEnvPor = 0 );
    inline void SetPaintDrop( const sal_Bool bNew ) { bPaintDrop = bNew; }
    inline sal_Bool IsPaintDrop() const { return bPaintDrop; }
    inline SwTxtPaintInfo &GetInfo()
        { return (SwTxtPaintInfo&)SwTxtIter::GetInfo(); }
    inline const SwTxtPaintInfo &GetInfo() const
        { return (const SwTxtPaintInfo&)SwTxtIter::GetInfo(); }
};



#endif
