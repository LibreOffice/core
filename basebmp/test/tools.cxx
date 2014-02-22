/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */



#include <basegfx/vector/b2isize.hxx>
#include <basegfx/point/b2ipoint.hxx>

#include <basebmp/color.hxx>
#include <basebmp/bitmapdevice.hxx>

using namespace ::basebmp;

int countPixel( const BitmapDeviceSharedPtr& rDevice,
                Color                        checkColor )
{
    int count(0);
    const basegfx::B2ISize& rSize( rDevice->getSize() );
    for( sal_Int32 y=0; y<rSize.getY(); ++y )
        for( sal_Int32 x=0; x<rSize.getX(); ++x )
            if( rDevice->getPixel( basegfx::B2IPoint(x,y) ) == checkColor )
                ++count;

    return count;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
