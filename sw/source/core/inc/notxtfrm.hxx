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


#ifndef SW_NOTXTFRM_HXX
#define SW_NOTXTFRM_HXX

#include <cntfrm.hxx>

class SwNoTxtNode;
class OutputDevice;
class SwBorderAttrs;
class Bitmap;
struct SwCrsrMoveState;

class SwNoTxtFrm: public SwCntntFrm
{
    friend void _FrmFinit();

    short    nWeight;                   // "Wichtigkeit" der Grafik

    const Size& GetSize() const;

    void InitCtor();

    void Format ( const SwBorderAttrs *pAttrs = 0 );
    void PaintCntnt  ( OutputDevice*, const SwRect&, const SwRect& ) const;
    /// OD 25.09.2002 #99739# - delete unused 3rd parameter
    void PaintPicture( OutputDevice*, const SwRect& ) const;
protected:
    virtual void MakeAll();
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );
public:
    SwNoTxtFrm( SwNoTxtNode * const, SwFrm* );
    ~SwNoTxtFrm();

    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;
    virtual sal_Bool GetCharRect( SwRect &, const SwPosition&,
                              SwCrsrMoveState* = 0) const;
    sal_Bool GetCrsrOfst(SwPosition* pPos, Point& aPoint,
                     SwCrsrMoveState* = 0) const;

    const Size &GetGrfSize() const  { return GetSize(); }
    void GetGrfArea( SwRect &rRect, SwRect * = 0, sal_Bool bMirror = sal_True ) const;

    sal_Bool IsTransparent() const;

    void StopAnimation( OutputDevice* = 0 ) const;
    sal_Bool HasAnimation()  const;

    // Routinen fuer den Grafik-Cache
    sal_uInt16 GetWeight() { return nWeight; }
};

#endif

