/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtfsize.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 08:54:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _FMTFSIZE_HXX
#define _FMTFSIZE_HXX

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _SWTYPES_HXX //autogen
#include <swtypes.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif

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
    BYTE      nWidthPercent;    //Fuer Tabellen kann die Breite in Prozent
    BYTE      nHeightPercent;   //angegeben sein.
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
    virtual BOOL             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

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

    BYTE    GetHeightPercent() const{ return nHeightPercent; }
    BYTE    GetWidthPercent() const { return nWidthPercent;  }
    void    SetHeightPercent( BYTE n ) { nHeightPercent = n; }
    void    SetWidthPercent ( BYTE n ) { nWidthPercent  = n; }
};

inline const SwFmtFrmSize &SwAttrSet::GetFrmSize(BOOL bInP) const
    { return (const SwFmtFrmSize&)Get( RES_FRM_SIZE,bInP); }

inline const SwFmtFrmSize &SwFmt::GetFrmSize(BOOL bInP) const
    { return aSet.GetFrmSize(bInP); }

#endif

