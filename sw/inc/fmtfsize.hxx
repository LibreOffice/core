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


#ifndef _FMTFSIZE_HXX
#define _FMTFSIZE_HXX

#include <tools/gen.hxx>
#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <swtypes.hxx>
#include <format.hxx>

class IntlWrapper;

//Die Framesize ---------------------------------

enum SwFrmSize
{
    ATT_VAR_SIZE,       //Frm ist in der Var-Richtung variabel
    ATT_FIX_SIZE,       //Frm ist in der Var-Richtung unbeweglich
    ATT_MIN_SIZE        //Der Wert in der Var-Richtung beschreibt eine
                        //Minimalgroesse, die nicht unter- wohl aber
                        //ueberschritten werden kann.
};

class SW_DLLPUBLIC SwFmtFrmSize: public SfxPoolItem
{
    Size      aSize;
    SwFrmSize eFrmHeightType;
    SwFrmSize eFrmWidthType;
    sal_uInt8     nWidthPercent;    //Fuer Tabellen kann die Breite in Prozent
    sal_uInt8     nHeightPercent;   //angegeben sein.
                                //Fuer Rahmen koennen Hoehe und/oder Breite
                                //in Prozent angegeben sein. Wenn nur eine
                                //der Angaben in Prozent angeben ist, kann
                                //durch den ausgezeichneten Wert 0xFF in der
                                //anderen Prozentangabe bestimmt werden, das
                                //sich diese Richtung proportional zur anderen
                                //verhaelt. Basis fuer die Umrechnung sind fuer
                                //diesen Fall die Angaben in der Size.
                                //Die Prozentwerte beziehen sich immer auf die
                                //Umgebung in der das Objekt steht (PrtArea)
                                //Auf die Bildschirmbreite abzueglich Raender
                                //in der BrowseView wenn die Umgebung die Seite
                                //ist.
public:
    SwFmtFrmSize( SwFrmSize eSize = ATT_VAR_SIZE,
                  SwTwips nWidth = 0, SwTwips nHeight = 0 );
    SwFmtFrmSize& operator=( const SwFmtFrmSize& rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    SwFrmSize GetHeightSizeType() const { return eFrmHeightType; }
    void SetHeightSizeType( SwFrmSize eSize ) { eFrmHeightType = eSize; }

    SwFrmSize GetWidthSizeType() const { return eFrmWidthType; }
    void SetWidthSizeType( SwFrmSize eSize ) { eFrmWidthType = eSize; }

    const Size& GetSize() const { return aSize; }
          void  SetSize( const Size &rNew ) { aSize = rNew; }

    SwTwips GetHeight() const { return aSize.Height(); }
    SwTwips GetWidth()  const { return aSize.Width();  }
    void    SetHeight( const SwTwips nNew ) { aSize.Height() = nNew; }
    void    SetWidth ( const SwTwips nNew ) { aSize.Width()  = nNew; }

    sal_uInt8    GetHeightPercent() const{ return nHeightPercent; }
    sal_uInt8   GetWidthPercent() const { return nWidthPercent;  }
    void    SetHeightPercent( sal_uInt8 n ) { nHeightPercent = n; }
    void    SetWidthPercent ( sal_uInt8 n ) { nWidthPercent  = n; }
};

inline const SwFmtFrmSize &SwAttrSet::GetFrmSize(sal_Bool bInP) const
    { return (const SwFmtFrmSize&)Get( RES_FRM_SIZE,bInP); }

inline const SwFmtFrmSize &SwFmt::GetFrmSize(sal_Bool bInP) const
    { return aSet.GetFrmSize(bInP); }

#endif

