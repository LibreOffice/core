/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imapcirc.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:22:20 $
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
#ifndef _GOODIES_IMAPCIRC_HXX
#define _GOODIES_IMAPCIRC_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#ifndef _GOODIES_IMAPOBJ_HXX
#include <svtools/imapobj.hxx>
#endif

class Fraction;

/******************************************************************************
|*
|*
|*
\******************************************************************************/

class SVT_DLLPUBLIC IMapCircleObject : public IMapObject
{
    Point               aCenter;
    ULONG               nRadius;

    void                ImpConstruct( const Point& rCenter, ULONG nRad, BOOL bPixel );

protected:

    // Binaer-Im-/Export
    virtual void        WriteIMapObject( SvStream& rOStm ) const;
    virtual void        ReadIMapObject(  SvStream& rIStm );

public:

                        IMapCircleObject() {};
                        IMapCircleObject( const Point& rCenter, ULONG nRad,
                                          const String& rURL,
                                          const String& rDescription,
                                          BOOL bActive = TRUE,
                                          BOOL bPixelCoords = TRUE );
                        IMapCircleObject( const Point& rCenter, ULONG nRad,
                                          const String& rURL,
                                          const String& rDescription,
                                          const String& rTarget,
                                          BOOL bActive = TRUE,
                                          BOOL bPixelCoords = TRUE );
                        IMapCircleObject( const Point& rCenter, ULONG nRad,
                                          const String& rURL,
                                          const String& rDescription,
                                          const String& rTarget,
                                          const String& rName,
                                          BOOL bActive = TRUE,
                                          BOOL bPixelCoords = TRUE );
    virtual             ~IMapCircleObject() {};

    virtual UINT16      GetType() const;
    virtual BOOL        IsHit( const Point& rPoint ) const;

    Point               GetCenter( BOOL bPixelCoords = TRUE ) const;
    ULONG               GetRadius( BOOL bPixelCoords = TRUE ) const;

    // liefert das BoundRect des Kreis-Objektes in 1/100mm
    virtual Rectangle   GetBoundRect() const;

    void                Scale( const Fraction& rFractX, const Fraction& rFracY );

    using IMapObject::IsEqual;
    BOOL                IsEqual( const IMapCircleObject& rEqObj );

    // Im-/Export
    void                WriteCERN( SvStream& rOStm, const String& rBaseURL  ) const;
    void                WriteNCSA( SvStream& rOStm, const String& rBaseURL  ) const;
};

#endif
