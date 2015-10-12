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

#ifndef INCLUDED_SVTOOLS_IMAPPOLY_HXX
#define INCLUDED_SVTOOLS_IMAPPOLY_HXX

#include <svtools/svtdllapi.h>
#include <svtools/imapobj.hxx>
#include <tools/poly.hxx>

class Fraction;

class SVT_DLLPUBLIC IMapPolygonObject : public IMapObject
{
    tools::Polygon aPoly;
    Rectangle           aEllipse;
    bool                bEllipse;

    SVT_DLLPRIVATE void ImpConstruct( const tools::Polygon& rPoly, bool bPixel );

protected:

    // Binaer-Im-/Export
    virtual void        WriteIMapObject( SvStream& rOStm ) const override;
    virtual void        ReadIMapObject(  SvStream& rIStm ) override;

public:
                        IMapPolygonObject() : bEllipse(false) {}
                        IMapPolygonObject( const tools::Polygon& rPoly,
                                           const OUString& rURL,
                                           const OUString& rAltText,
                                           const OUString& rDesc,
                                           const OUString& rTarget,
                                           const OUString& rName,
                                           bool bActive = true,
                                           bool bPixelCoords = true );
    virtual             ~IMapPolygonObject() {}

    virtual sal_uInt16      GetType() const override;
    virtual bool        IsHit( const Point& rPoint ) const override;

    tools::Polygon      GetPolygon( bool bPixelCoords = true ) const;

    bool                HasExtraEllipse() const { return bEllipse; }
    const Rectangle&    GetExtraEllipse() const { return aEllipse; }
    void                SetExtraEllipse( const Rectangle& rEllipse );

    void                Scale( const Fraction& rFractX, const Fraction& rFracY );

    using IMapObject::IsEqual;
    bool                IsEqual( const IMapPolygonObject& rEqObj );

    // Im-/Export
    void                WriteCERN( SvStream& rOStm, const OUString& rBaseURL  ) const;
    void                WriteNCSA( SvStream& rOStm, const OUString& rBaseURL  ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
