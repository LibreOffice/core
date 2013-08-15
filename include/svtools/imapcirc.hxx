/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _GOODIES_IMAPCIRC_HXX
#define _GOODIES_IMAPCIRC_HXX

#include "svtools/svtdllapi.h"
#include <tools/gen.hxx>
#include <svtools/imapobj.hxx>

class Fraction;

class SVT_DLLPUBLIC IMapCircleObject : public IMapObject
{
    Point               aCenter;
    sal_Int32               nRadius;

    void                ImpConstruct( const Point& rCenter, sal_uLong nRad, sal_Bool bPixel );

protected:

    // Binaer-Im-/Export
    virtual void        WriteIMapObject( SvStream& rOStm ) const;
    virtual void        ReadIMapObject(  SvStream& rIStm );

public:

                        IMapCircleObject() {};
                        IMapCircleObject( const Point& rCenter, sal_uLong nRad,
                                          const OUString& rURL,
                                          const OUString& rAltText,
                                          const OUString& rDesc,
                                          const OUString& rTarget,
                                          const OUString& rName,
                                          sal_Bool bActive = sal_True,
                                          sal_Bool bPixelCoords = sal_True );
    virtual             ~IMapCircleObject() {};

    virtual sal_uInt16      GetType() const;
    virtual sal_Bool        IsHit( const Point& rPoint ) const;

    Point               GetCenter( sal_Bool bPixelCoords = sal_True ) const;
    sal_uLong               GetRadius( sal_Bool bPixelCoords = sal_True ) const;

    // liefert das BoundRect des Kreis-Objektes in 1/100mm
    virtual Rectangle   GetBoundRect() const;

    void                Scale( const Fraction& rFractX, const Fraction& rFracY );

    using IMapObject::IsEqual;
    sal_Bool                IsEqual( const IMapCircleObject& rEqObj );

    // Im-/Export
    void                WriteCERN( SvStream& rOStm, const OUString& rBaseURL  ) const;
    void                WriteNCSA( SvStream& rOStm, const OUString& rBaseURL  ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
