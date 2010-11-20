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
#include "precompiled_sd.hxx"
#include <sfx2/app.hxx>


#include "ClientView.hxx"
#include "drawview.hxx"
#include "sdpage.hxx"

class DrawDocShell;

namespace sd {

class DrawViewShell;


/*************************************************************************
|*
|* Ctor: Die ClientView wird fuer DrawDocShell::Draw() verwendet
|*
\************************************************************************/

ClientView::ClientView(
    DrawDocShell* pDocSh,
    OutputDevice* pOutDev,
    DrawViewShell* pShell)
    : DrawView (pDocSh, pOutDev, pShell)
{
}


/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

ClientView::~ClientView()
{
}

/*************************************************************************
|*
|* Wenn die View kein Invalidate() an den Fenstern durchfuehren soll, muss
|* man diese Methode ueberladen und entsprechend anders reagieren.
|*
\************************************************************************/

void ClientView::InvalidateOneWin(::Window& rWin)
{
    Region aRegion;
    CompleteRedraw(&rWin, aRegion);
}

/*************************************************************************
|*
|* Wenn die View kein Invalidate() an den Fenstern durchfuehren soll, muss
|* man diese Methode ueberladen und entsprechend anders reagieren.
|*
\************************************************************************/

void ClientView::InvalidateOneWin(::Window& rWin, const Rectangle& rRect)
{
    CompleteRedraw(&rWin, rRect);
}

void ClientView::CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector )
{
    DrawView::CompleteRedraw(pOutDev, rReg, pRedirector);
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
