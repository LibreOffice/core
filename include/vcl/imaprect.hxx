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
#pragma once

#include <config_options.h>
#include <vcl/dllapi.h>
#include <vcl/imapobj.hxx>
#include <tools/gen.hxx>

class Fraction;

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) IMapRectangleObject final : public IMapObject
{
    tools::Rectangle           aRect;

    SAL_DLLPRIVATE void             ImpConstruct( const tools::Rectangle& rRect, bool bPixel );

    // binary import/export
    virtual void        WriteIMapObject( SvStream& rOStm ) const override;
    virtual void        ReadIMapObject(  SvStream& rIStm ) override;

public:

                        IMapRectangleObject() {};
                        IMapRectangleObject( const tools::Rectangle& rRect,
                                             const OUString& rURL,
                                             const OUString& rAltText,
                                             const OUString& rDesc,
                                             const OUString& rTarget,
                                             const OUString& rName,
                                             bool bActive = true,
                                             bool bPixelCoords = true );

    virtual IMapObjectType GetType() const override;
    virtual bool        IsHit( const Point& rPoint ) const override;

    tools::Rectangle           GetRectangle( bool bPixelCoords = true ) const;

    void                Scale( const Fraction& rFractX, const Fraction& rFracY );

    using IMapObject::IsEqual;
    bool                IsEqual( const IMapRectangleObject& rEqObj ) const;

    // import/export
    void                WriteCERN( SvStream& rOStm  ) const;
    void                WriteNCSA( SvStream& rOStm  ) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
