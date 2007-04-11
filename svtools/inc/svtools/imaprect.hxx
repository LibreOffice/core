/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imaprect.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:23:05 $
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
#ifndef _GOODIES_IMAPRECT_HXX
#define _GOODIES_IMAPRECT_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _GOODIES_IMAPOBJ_HXX
#include <svtools/imapobj.hxx>
#endif

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

class Fraction;

/******************************************************************************
|*
|*
|*
\******************************************************************************/

class SVT_DLLPUBLIC IMapRectangleObject : public IMapObject
{
    Rectangle           aRect;

    SVT_DLLPRIVATE void             ImpConstruct( const Rectangle& rRect, BOOL bPixel );

protected:

    // Binaer-Im-/Export
    virtual void        WriteIMapObject( SvStream& rOStm ) const;
    virtual void        ReadIMapObject(  SvStream& rIStm );

public:

                        IMapRectangleObject() {};
                        IMapRectangleObject( const Rectangle& rRect,
                                             const String& rURL,
                                             const String& rDescription,
                                             BOOL bActive = TRUE,
                                             BOOL bPixelCoords = TRUE );
                        IMapRectangleObject( const Rectangle& rRect,
                                             const String& rURL,
                                             const String& rDescription,
                                             const String& rTarget,
                                             BOOL bActive = TRUE,
                                             BOOL bPixelCoords = TRUE );
                        IMapRectangleObject( const Rectangle& rRect,
                                             const String& rURL,
                                             const String& rDescription,
                                             const String& rTarget,
                                             const String& rName,
                                             BOOL bActive = TRUE,
                                             BOOL bPixelCoords = TRUE );
    virtual             ~IMapRectangleObject() {};

    virtual UINT16      GetType() const;
    virtual BOOL        IsHit( const Point& rPoint ) const;

    Rectangle           GetRectangle( BOOL bPixelCoords = TRUE ) const;

    // liefert das BoundRect des Rechteck-Objektes in 1/100mm
    virtual Rectangle   GetBoundRect() const { return aRect; }

    void                Scale( const Fraction& rFractX, const Fraction& rFracY );

    using IMapObject::IsEqual;
    BOOL                IsEqual( const IMapRectangleObject& rEqObj );

    // Im-/Export
    void                WriteCERN( SvStream& rOStm, const String& rBaseURL  ) const;
    void                WriteNCSA( SvStream& rOStm, const String& rBaseURL  ) const;
};

#endif
