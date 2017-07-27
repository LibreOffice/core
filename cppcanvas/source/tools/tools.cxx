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


#include <tools.hxx>


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace tools
    {
        uno::Sequence< double > intSRGBAToDoubleSequence( Color::IntSRGBA aColor  )
        {
            uno::Sequence< double > aRes( 4 );

            aRes[0] = getRed(aColor) / 255.0;
            aRes[1] = getGreen(aColor) / 255.0;
            aRes[2] = getBlue(aColor) / 255.0;
            aRes[3] = getAlpha(aColor) / 255.0;

            return aRes;
        }

        Color::IntSRGBA doubleSequenceToIntSRGBA( const uno::Sequence< double >& rColor  )
        {
            return makeColor( static_cast<sal_uInt8>( 255*rColor[0] + .5 ),
                              static_cast<sal_uInt8>( 255*rColor[1] + .5 ),
                              static_cast<sal_uInt8>( 255*rColor[2] + .5 ),
                              static_cast<sal_uInt8>( 255*rColor[3] + .5 ) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
