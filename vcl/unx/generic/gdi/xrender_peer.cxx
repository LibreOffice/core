/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <stdio.h>

#include <rtl/ustring.hxx>
#include <osl/module.h>

#include <unx/salunx.h>
#include <unx/saldata.hxx>
#include <unx/saldisp.hxx>

#include <xrender_peer.hxx>

using namespace rtl;

// ---------------------------------------------------------------------------

XRenderPeer::XRenderPeer()
    : mpDisplay( GetGenericData()->GetSalDisplay()->GetDisplay() )
    , mpStandardFormatA8( NULL )
{
    InitRenderLib();
}

// ---------------------------------------------------------------------------

XRenderPeer& XRenderPeer::GetInstance()
{
    static XRenderPeer aPeer;
    return aPeer;
}

// ---------------------------------------------------------------------------

void XRenderPeer::InitRenderLib()
{
    int nDummy;
    // needed to initialize libXrender internals
    XRenderQueryExtension( mpDisplay, &nDummy, &nDummy );

    // the 8bit alpha mask format must be there
    XRenderPictFormat aPictFormat={0,0,8,{0,0,0,0,0,0,0,0xFF},0};
    mpStandardFormatA8 = FindPictureFormat( PictFormatAlphaMask|PictFormatDepth, aPictFormat );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
