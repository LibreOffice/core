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
#ifndef INCLUDED_SVTOOLS_IMAPRECT_HXX
#define INCLUDED_SVTOOLS_IMAPRECT_HXX

#include <svtools/svtdllapi.h>
#include <svtools/imapobj.hxx>
#include <tools/gen.hxx>

namespace boost { template<typename T> class rational; }

class SVT_DLLPUBLIC IMapRectangleObject : public IMapObject
{
    Rectangle           aRect;

    SVT_DLLPRIVATE void             ImpConstruct( const Rectangle& rRect, bool bPixel );

protected:

    // Binaer-Im-/Export
    virtual void        WriteIMapObject( SvStream& rOStm ) const SAL_OVERRIDE;
    virtual void        ReadIMapObject(  SvStream& rIStm ) SAL_OVERRIDE;

public:

                        IMapRectangleObject() {};
                        IMapRectangleObject( const Rectangle& rRect,
                                             const OUString& rURL,
                                             const OUString& rAltText,
                                             const OUString& rDesc,
                                             const OUString& rTarget,
                                             const OUString& rName,
                                             bool bActive = true,
                                             bool bPixelCoords = true );
    virtual             ~IMapRectangleObject() {};

    virtual sal_uInt16  GetType() const SAL_OVERRIDE;
    virtual bool        IsHit( const Point& rPoint ) const SAL_OVERRIDE;

    Rectangle           GetRectangle( bool bPixelCoords = true ) const;

    // liefert das BoundRect des Rechteck-Objektes in 1/100mm
    virtual Rectangle   GetBoundRect() const SAL_OVERRIDE { return aRect; }

    void                Scale( const boost::rational<long>& rFractX, const boost::rational<long>& rFracY );

    using IMapObject::IsEqual;
    bool                IsEqual( const IMapRectangleObject& rEqObj );

    // Im-/Export
    void                WriteCERN( SvStream& rOStm, const OUString& rBaseURL  ) const;
    void                WriteNCSA( SvStream& rOStm, const OUString& rBaseURL  ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
