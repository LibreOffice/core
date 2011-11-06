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


#ifndef _BLINK_HXX
#define _BLINK_HXX

class SwLinePortion;
class SwRootFrm;
class SwTxtFrm;

#include <vcl/timer.hxx>
#include <tools/gen.hxx>

#include <svl/svarray.hxx>

class SwBlinkPortion
{
    Point               aPos;
    const SwLinePortion *pPor;
    const SwRootFrm     *pFrm;
    sal_uInt16              nDir;
public:
    SwBlinkPortion( const SwLinePortion* pPortion, sal_uInt16 nDirection )
            { pPor = pPortion; nDir = nDirection; }
    SwBlinkPortion( const SwBlinkPortion* pBlink, const SwLinePortion* pPort )
    {   pPor = pPort; pFrm = pBlink->pFrm; aPos = pBlink->aPos; nDir = pBlink->nDir; }
    void SetPos( const Point& aNew ){ aPos = aNew; }
    const Point& GetPos() const{ return aPos; }
    void SetRootFrm( const SwRootFrm* pNew ){ pFrm = pNew; }
    const SwRootFrm* GetRootFrm() const{ return pFrm; }
    const SwLinePortion *GetPortion() const{ return pPor; }
    sal_uInt16 GetDirection() const { return nDir; }
    sal_Bool operator<( const SwBlinkPortion& rBlinkPortion ) const
    { return (long)pPor < (long)rBlinkPortion.pPor; }
    sal_Bool operator==( const SwBlinkPortion& rBlinkPortion ) const
    { return (long)pPor == (long)rBlinkPortion.pPor; }
};

typedef SwBlinkPortion* SwBlinkPortionPtr;
SV_DECL_PTRARR_SORT_DEL(SwBlinkList, SwBlinkPortionPtr, 0, 10)

class SwBlink
{
    SwBlinkList     aList;
    AutoTimer       aTimer;
    sal_Bool            bVisible;
public:
    SwBlink();
    ~SwBlink();

    DECL_LINK( Blinker, Timer * );

    void Insert( const Point& rPoint, const SwLinePortion* pPor,
                 const SwTxtFrm *pTxtFrm, sal_uInt16 nDir );
    void Replace( const SwLinePortion* pOld, const SwLinePortion* pNew );
    void Delete( const SwLinePortion* pPor );
    void FrmDelete( const SwRootFrm* pRoot );
    inline sal_Bool IsVisible() const { return bVisible ; }
};

// Blink-Manager, globale Variable, in Blink.Cxx angelegt
extern SwBlink *pBlink;


#endif

