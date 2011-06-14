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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#include <cppuhelper/weak.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <tools/color.hxx>

#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/gradient.hxx>
#include <vcl/cvtgrf.hxx>

#include <svtools/wallitem.hxx>
#include <svl/cntwall.hxx>

// -----------------------------------------------------------------------

//static
void SfxBrushItemLink::Set( SfxBrushItemLink* pLink )
{
    SfxBrushItemLink** ppLink =  (SfxBrushItemLink**)GetAppData(SHL_BRUSHITEM);
    if( !*ppLink )
        *ppLink = pLink;
    else
        delete pLink;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
