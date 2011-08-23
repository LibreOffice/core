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
#ifndef _FMTFSIZE_HXX
#define _FMTFSIZE_HXX

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SWTYPES_HXX //autogen
#include <swtypes.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
class IntlWrapper; 
namespace binfilter {



//Die Framesize ---------------------------------

enum SwFrmSize
{
    ATT_VAR_SIZE,		//Frm ist in der Var-Richtung variabel
    ATT_FIX_SIZE,		//Frm ist in der Var-Richtung unbeweglich
    ATT_MIN_SIZE		//Der Wert in der Var-Richtung beschreibt eine
                        //Minimalgroesse, die nicht unter- wohl aber
                        //ueberschritten werden kann.
};

class SwFmtFrmSize: public SfxPoolItem
{
    Size	  aSize;
    SwFrmSize eFrmSize;
    BYTE 	  nWidthPercent;	//Fuer Tabellen kann die Breite in Prozent
    BYTE	  nHeightPercent;	//angegeben sein.
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
    virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
    virtual SvStream&		Store(SvStream &, USHORT nIVer ) const;
    virtual	BOOL        	 QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	BOOL			 PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    virtual USHORT			 GetVersion( USHORT nFFVer ) const;

    SwFrmSize GetSizeType() const { return eFrmSize; }
    void SetSizeType( SwFrmSize eSize ) { eFrmSize = eSize; }

    const Size& GetSize() const { return aSize; }
          void  SetSize( const Size &rNew ) { aSize = rNew; }

    SwTwips GetHeight() const { return aSize.Height(); }
    SwTwips GetWidth()  const { return aSize.Width();  }
    void	SetHeight( const SwTwips nNew ) { aSize.Height() = nNew; }
    void	SetWidth ( const SwTwips nNew ) { aSize.Width()  = nNew; }

    BYTE    GetHeightPercent() const{ return nHeightPercent; }
    BYTE	GetWidthPercent() const { return nWidthPercent;  }
    void	SetHeightPercent( BYTE n ) { nHeightPercent = n; }
    void	SetWidthPercent ( BYTE n ) { nWidthPercent  = n; }

    // Umrechnung der Groesse zwischen SW31/SW40
    Size	GetSizeConvertedToSw31( const SvxLRSpaceItem *pLRSpace,
                                    const SvxULSpaceItem *pULSpace ) const;
    Size	GetSizeConvertedFromSw31( const SvxLRSpaceItem *pLRSpace,
                                      const SvxULSpaceItem *pULSpace ) const;
};

#if !(defined(MACOSX) &&  ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline const SwFmtFrmSize &SwAttrSet::GetFrmSize(BOOL bInP) const
    { return (const SwFmtFrmSize&)Get( RES_FRM_SIZE,bInP); }

inline const SwFmtFrmSize &SwFmt::GetFrmSize(BOOL bInP) const
    { return aSet.GetFrmSize(bInP); }
#endif

} //namespace binfilter
#endif

