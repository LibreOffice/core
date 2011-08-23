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
#ifndef _GOODIES_IMAPCIRC_HXX
#define _GOODIES_IMAPCIRC_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "bf_svtools/svtdllapi.h"
#endif

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#ifndef _GOODIES_IMAPOBJ_HXX
#include <bf_svtools/imapobj.hxx>
#endif

class Fraction;

namespace binfilter
{

class  IMapCircleObject : public IMapObject
{
    Point				aCenter;
    ULONG				nRadius;

    void				ImpConstruct( const Point& rCenter, ULONG nRad, BOOL bPixel );

protected:

    // Binaer-Im-/Export
    virtual void		WriteIMapObject( SvStream& rOStm ) const;
    virtual void		ReadIMapObject(  SvStream& rIStm );

public:

                        IMapCircleObject() {};
                        IMapCircleObject( const Point& rCenter, ULONG nRad,
                                          const String& rURL,
                                          const String& rAltText,
                                          const String& rDesc,
                                          const String& rTarget,
                                          const String& rName,
                                          BOOL bActive = TRUE,
                                          BOOL bPixelCoords = TRUE );
    virtual				~IMapCircleObject() {};

    virtual UINT16		GetType() const;
    virtual BOOL		IsHit( const Point& rPoint ) const;

    Point				GetCenter( BOOL bPixelCoords = TRUE ) const;
    ULONG				GetRadius( BOOL bPixelCoords = TRUE ) const;

    // liefert das BoundRect des Kreis-Objektes in 1/100mm
    virtual Rectangle	GetBoundRect() const;

    void				Scale( const Fraction& rFractX, const Fraction& rFracY );

    using IMapObject::IsEqual;
    BOOL				IsEqual( const IMapCircleObject& rEqObj );
};

}

#endif
