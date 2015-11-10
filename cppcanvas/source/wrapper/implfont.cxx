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


#include <implfont.hxx>
#include <canvas/canvastools.hxx>


using namespace ::com::sun::star;

/* Definition of Font class */

namespace cppcanvas
{
    namespace internal
    {

        ImplFont::ImplFont( const uno::Reference< rendering::XCanvas >& rCanvas,
                            const OUString& rFontName,
                            const double& rCellSize ) :
            mxCanvas( rCanvas ),
            mxFont( nullptr )
        {
            OSL_ENSURE( mxCanvas.is(), "ImplFont::ImplFont(): Invalid Canvas" );

            rendering::FontRequest aFontRequest;
            aFontRequest.FontDescription.FamilyName = rFontName;
            aFontRequest.CellSize = rCellSize;

            geometry::Matrix2D aFontMatrix;
            ::canvas::tools::setIdentityMatrix2D( aFontMatrix );

            mxFont = mxCanvas->createFont( aFontRequest,
                                           uno::Sequence< beans::PropertyValue >(),
                                           aFontMatrix );
        }


        ImplFont::~ImplFont()
        {
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
