/*************************************************************************
 *
 *  $RCSfile: sectctr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_SECTCTR_HXX
#define _SV_SECTCTR_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif

class ImplSplitWindow;
class ScrollBar;
class ScrollBarBox;
class SvSection;

#define SECTION_APPEND      ((USHORT)0xFFFF)
#define SECTION_NOTFOUND    ((USHORT)0xFFFF)

#define WB_SECTION_STYLE    WB_VSCROLL | WB_HSCROLL | WB_TABSTOP

class SvSectionControl : public Control
{
private:

    Window              aSplitWinContainer;
    ImplSplitWindow*    pSplitWin;
    ScrollBar*          pVScrollBar;
    ScrollBar*          pHScrollBar;
    ScrollBarBox*       pScrollBarBox;
    DockingWindow*      pDummy;
    long                nRealHeight;
    long                nMaxHeight;
    long                nMinWidth;
    Wallpaper           aWallpaper;

    DECL_LINK( ScrollHdl, ScrollBar* );
    DECL_LINK( EndScrollHdl, ScrollBar* );


protected:

    virtual void        Resize();
    virtual void        Paint( const Rectangle& rRect );
    virtual void        StateChanged( StateChangedType nStateChange );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    virtual void        Command( const CommandEvent& rCEvt );
    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual long        Notify( NotifyEvent& rNEvt );
    virtual long        KeyEventNotify( const KeyEvent& rKEvt );
    virtual void        SetPosSizePixel( long nX, long nY,long nWidth, long nHeight,USHORT nFlags);



    long                CalcMaxHeight();
    long                CalcRealHeight();
    long                CalcSectionWidth();

    void                SetScrollBars(BOOL bVert,BOOL bHorz);
    void                ShowScrollBarBox();
    void                UpdateScrollBars();

    BOOL                VScrollResize(Size &aSize);
    BOOL                HScrollResize(Size &aSize);
    void                SetChildPos(long nPos, BOOL bScrolling = TRUE);

public:
                        SvSectionControl( Window* pParent,WinBits nStyle = WB_SECTION_STYLE);
                        SvSectionControl( Window* pParent, const ResId& rResId );
                        ~SvSectionControl();


    void                InsertSection(  USHORT nSectionId,SvSection* pSection,long nSize,USHORT nPos);
    void                InsertSection(  USHORT nSectionId,SvSection* pSection,USHORT nPos);
    void                RemoveSection( USHORT nSectionId );
    void                Clear();

    USHORT              GetSectionCount() const;
    USHORT              GetSectionId( USHORT nPos ) const;
    USHORT              GetSectionPos( USHORT nSectionId ) const;
    USHORT              GetSectionId( const Point& rPos ) const;

    void                SetSectionSize( USHORT nId, long nNewSize );
    long                GetSectionSize( USHORT nId ) const;

    /*
    void                SetCurSectionId( USHORT nSectionId );
    USHORT              GetCurSectionId() const;

    void                SetFirstSectionId( USHORT nSectionId );
    USHORT              GetFirstSectionId() const { return GetSectionId( mnFirstSectionPos ); }

    void                MakeVisible( USHORT nSectionId );
    */

    void                SetSectionWidth( USHORT nSectionId, long nWidth);
    long                GetSectionWidth( USHORT nSectionId ) const;

    void                SetSection( USHORT nSectionId, SvSection* pPage );
    SvSection*          GetSection( USHORT nSectionId ) const;

    void                SetSectionText( USHORT nSectionId, const XubString& rText );
    XubString           GetSectionText( USHORT nSectionId ) const;

    void                SetHelpText( USHORT nSectionId, const XubString& rText );
    const XubString&    GetHelpText( USHORT nSectionId ) const;

    void                SetHelpId( USHORT nSectionId, ULONG nHelpId );
    ULONG               GetHelpId( USHORT nSectionId ) const;

    void                SetHelpText( const XubString& rText )
                            { Control::SetHelpText( rText ); }
    const XubString&    GetHelpText() const
                            { return Control::GetHelpText(); }

    void                SetHelpId( ULONG nId )
                            { Control::SetHelpId( nId ); }
    ULONG               GetHelpId() const
                            { return Control::GetHelpId(); }

    void                SetBackground( const Wallpaper& rBackground ){aWallpaper=rBackground; }
    const Wallpaper&    GetBackground() const { return aWallpaper; }

};


#endif
