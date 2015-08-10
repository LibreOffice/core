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


#include <basegfx/matrix/b2dhommatrix.hxx>
#include "checkerboardwipe.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon CheckerBoardWipe::operator () ( double t )
{
    const double d = (1.0 / m_unitsPerEdge);
    ::basegfx::B2DHomMatrix aTransform;
    aTransform.scale( ::basegfx::pruneScaleValue( d * 2.0 * t ),
                      ::basegfx::pruneScaleValue( d ) );

    ::basegfx::B2DPolyPolygon res;
    for ( sal_Int32 i = m_unitsPerEdge; i--; )
    {
        ::basegfx::B2DHomMatrix transform( aTransform );
        if ((i % 2) == 1) // odd line
            transform.translate( -d, 0.0 );
        for ( sal_Int32 j = (m_unitsPerEdge / 2) + 1; j--; )
        {
            ::basegfx::B2DPolyPolygon poly( m_unitRect );
            poly.transform( transform );
            res.append( poly );
            transform.translate( d * 2.0, 0.0 );
        }
        aTransform.translate( 0.0, d ); // next line
    }
    return res;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
