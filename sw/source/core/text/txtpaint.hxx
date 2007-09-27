/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtpaint.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:21:42 $
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
#ifndef _TXTPAINT_HXX
#define _TXTPAINT_HXX
#ifndef _SV_OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif

class SwRect;               // SwSaveClip

#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif

/*************************************************************************
 *                      class SwSaveClip
 *************************************************************************/

class SwSaveClip
{
    Region   aClip;
    const sal_Bool     bOn;
          sal_Bool     bChg;
protected:
    OutputDevice* pOut;
    void _ChgClip( const SwRect &rRect, const SwTxtFrm* pFrm,
                   sal_Bool bEnlargeRect );
public:
    inline SwSaveClip( OutputDevice* pOut );
    inline ~SwSaveClip();
    inline void ChgClip( const SwRect &rRect, const SwTxtFrm* pFrm = 0,
                         sal_Bool bEnlargeRect = sal_False)
             { if( pOut ) _ChgClip( rRect, pFrm, bEnlargeRect ); }
           void Reset();
    inline sal_Bool IsOn()  const { return bOn; }
    inline sal_Bool IsChg() const { return bChg; }
    inline sal_Bool IsOut() const { return 0 != pOut; }
    inline OutputDevice *GetOut() { return pOut; }
};

inline SwSaveClip::SwSaveClip( OutputDevice* pOutDev ) :
    bOn( pOutDev && pOutDev->IsClipRegion() ),
    bChg( sal_False ),
    pOut(pOutDev)
{}

inline SwSaveClip::~SwSaveClip()
{
    Reset();
}

#ifndef PRODUCT

/*************************************************************************
 *                          class SwDbgOut
 *************************************************************************/

class SwDbgOut
{
protected:
        OutputDevice* pOut;
public:
        inline SwDbgOut( OutputDevice* pOutDev, const sal_Bool bOn = sal_True );
};

/*************************************************************************
 *                          class DbgColor
 *************************************************************************/

class DbgColor
{
    Font *pFnt;
    Color aColor;
public:
        inline DbgColor( Font *pFont, const sal_Bool bOn = sal_True,
                         const ColorData eColor = COL_BLUE );
        inline ~DbgColor();
};

/*************************************************************************
 *                          class DbgBrush
 *************************************************************************/

class DbgBackColor : public SwDbgOut
{
        Color   aOldFillColor;
public:
        DbgBackColor( OutputDevice* pOut, const sal_Bool bOn = sal_True,
                  ColorData nColor = COL_YELLOW );
       ~DbgBackColor();
};

/*************************************************************************
 *                          class DbgRect
 *************************************************************************/

class DbgRect : public SwDbgOut
{
public:
        DbgRect( OutputDevice* pOut, const Rectangle &rRect,
                 const sal_Bool bOn = sal_True,
                 ColorData eColor = COL_LIGHTBLUE );
};

/*************************************************************************
 *                      Inline-Implementierung
 *************************************************************************/

inline SwDbgOut::SwDbgOut( OutputDevice* pOutDev, const sal_Bool bOn )
               :pOut( bOn ? pOutDev : 0 )
{ }


inline DbgColor::DbgColor( Font *pFont, const sal_Bool bOn,
                 const ColorData eColor )
    :pFnt( bOn ? pFont : 0 )
{
    if( pFnt )
    {
        aColor = pFnt->GetColor();
        pFnt->SetColor( Color( eColor ) );
    }
}

inline DbgColor::~DbgColor()
{
    if( pFnt )
        pFnt->SetColor( aColor );
}

inline DbgBackColor::DbgBackColor( OutputDevice* pOutDev, const sal_Bool bOn,
                           ColorData eColor )
    :SwDbgOut( pOutDev, bOn )
{
    if( pOut )
    {
        aOldFillColor = pOut->GetFillColor();
        pOut->SetFillColor( Color(eColor) );
    }
}

inline DbgBackColor::~DbgBackColor()
{
    if( pOut )
    {
        pOut->SetFillColor( aOldFillColor );
    }
}

inline DbgRect::DbgRect( OutputDevice* pOutDev, const Rectangle &rRect,
                         const sal_Bool bOn,
                         ColorData eColor )
    : SwDbgOut( pOutDev, bOn )
{
    if( pOut )
    {
        const Color aColor( eColor );
        Color aLineColor = pOut->GetLineColor();
        pOut->SetLineColor( aColor );
        Color aFillColor = pOut->GetFillColor();
        pOut->SetFillColor( Color(COL_TRANSPARENT) );
        pOut->DrawRect( rRect );
        pOut->SetLineColor( aLineColor );
        pOut->SetFillColor( aFillColor );
    }
}

#endif



#endif
