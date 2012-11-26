/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

void ClientView::InvalidateOneWin(::Window& rWin, const basegfx::B2DRange& rRange)
{
    if(!rRange.isEmpty())
    {
        const Region aRegion(Rectangle(
            basegfx::fround(rRange.getMinX()), basegfx::fround(rRange.getMinY()),
            basegfx::fround(rRange.getMaxX()), basegfx::fround(rRange.getMaxY())));

        CompleteRedraw(&rWin, aRegion);
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void ClientView::CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector )
{
    DrawView::CompleteRedraw(pOutDev, rReg, pRedirector);
}


} // end of namespace sd
