/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: showview.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:48:57 $
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

#ifndef SD_SHOW_VIEW_HXX
#define SD_SHOW_VIEW_HXX

#ifndef _SVX_FMVIEW_HXX
#include <svx/fmview.hxx>
#endif

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

    virtual void    CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, USHORT nPaintMode, ::sdr::contact::ViewObjectContactRedirector* pRedirector = 0L);
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


