/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ClientView.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:32:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_CLIENT_VIEW_HXX
#define SD_CLIENT_VIEW_HXX

#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif

namespace sd {

class DrawViewShell;

/************************************************************************
|*
|* Die SdClientView wird fuer DrawDocShell::Draw() verwendet
|*
\************************************************************************/

class ClientView
    : public DrawView
{
public:
    ClientView (
        DrawDocShell* pDocSh,
        OutputDevice* pOutDev,
        DrawViewShell* pShell);
    virtual ~ClientView (void);

    virtual void CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, USHORT nPaintMode=0,
        ::sdr::contact::ViewObjectContactRedirector* pRedirector = 0L);

    // Wenn die View kein Invalidate() an den Fenstern durchfuehren soll, muss
    // man diese beiden folgenden Methoden ueberladen und entsprechend anders
    // reagieren.
    virtual void InvalidateOneWin(::Window& rWin);
    virtual void InvalidateOneWin(::Window& rWin, const Rectangle& rRect);
};

} // end of namespace sd

#endif

