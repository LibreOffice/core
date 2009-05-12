/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: showview.hxx,v $
 * $Revision: 1.8 $
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

#ifndef SD_SHOW_VIEW_HXX
#define SD_SHOW_VIEW_HXX

#include <svx/fmview.hxx>

class SdDrawDocument;

namespace sd {

class ViewShell;

/*************************************************************************
|*
|*    Beschreibung      ShowView ist die View fuer die Diashow
|*
\************************************************************************/

class ShowView
    : public FmFormView
{
public:
                    // wenn waehrend des Zeichnens Plugins 'connected'
                    // werden sollen, muss pWWin ein Zeiger auf das Fenster
                    // sein, das das Plugin benutzen soll
    ShowView (
        SdDrawDocument* pDoc,
        OutputDevice* pOut,
        ViewShell* pViewShell,
        ::Window* pWin = NULL);
    virtual ~ShowView (void);

    SdDrawDocument& GetDoc() const          { return *pDrDoc; }

            void    SetAllowInvalidate(BOOL bFlag);
            BOOL    IsInvalidateAllowed() const;

            void    SetAllowMasterPageCaching(BOOL bAllow)
                    { bAllowMasterPageCaching = bAllow; }
            BOOL    IsMasterPageCachingAllowed() const
                    { return(bAllowMasterPageCaching); }

    virtual void    CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0L);
    virtual void    InvalidateOneWin(::Window& rWin);
    virtual void    InvalidateOneWin(::Window& rWin, const Rectangle& rRect);

    virtual void    DoConnect(SdrOle2Obj* pOleObj);

private:
    SdDrawDocument* pDrDoc;
    ViewShell* mpViewSh;
    Window*         pWindowForPlugIns;
    USHORT          nAllowInvalidateSmph;
    BOOL            bAllowMasterPageCaching;

};

} // end of namespace sd

#endif


