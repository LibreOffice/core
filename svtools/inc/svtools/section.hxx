/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: section.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:33:28 $
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

#ifndef _SV_SECTION_HXX
#define _SV_SECTION_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef _SV_DOCKWIN_HXX
#include <vcl/dockwin.hxx>
#endif

#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif

#ifndef _SV_EXPANDER_HXX
#include <svtools/expander.hxx>
#endif

#ifndef _SV_PRIVSPLT_HXX
#include <svtools/privsplt.hxx>
#endif

class SvSectionControl;

class SvSection : public DockingWindow
{
private:
        SvSectionControl*       pSectionControl;
        USHORT                  nSectionID;
        Window*                 pChildWin;
        XubString               aName;
        long                    nHeight;
        long                    nMinHeight;
        long                    nMinWidth;
        long                    nOldHeight;

        DECL_LINK( ToggleHdl,   SvExpander* );
        DECL_LINK( SplitHdl,    SvPrivatSplit* );

        void                    ImplExpandSection();
        void                    ImplShrinkSection();

protected:
        SvExpander              aExpander;
        Window                  aChildWinContainer;
        SvPrivatSplit           aPrivatSplit;

        virtual void            Resize();
        virtual void            Paint( const Rectangle& rRect );
        virtual long            Notify( NotifyEvent& rNEvt );

public:
        SvSection(const XubString& rName,SvSectionControl* pParent,
                    WinBits nStyle = 0);
        SvSection(const XubString& rName,SvSectionControl* pParent,
                    Window* pChildWin, WinBits nStyle = 0);

        virtual void            SetScrollPos(long nPos);
        void                    SetChildPos(long nPos);

        void                    SetSectionControl(SvSectionControl* pParent);
        const SvSectionControl* GetSectionControl()const ;
        void                    SetSectionText( const XubString& );
        const XubString&            GetSectionText() const {return aName;}

        void                    SetChildWin(Window* pChildWin);
        Window*                 GetChildWin() const {return pChildWin;}

        void                    SetSectionID(USHORT nId) {nSectionID=nId;}
        USHORT                  GetSectionID() const {return nSectionID;}

        void                    SetMaxHeight(long nHeight);
        long                    GetMaxHeight();

        void                    SetMinHeight(long nHeight);
        long                    GetMinHeight();

        void                    SetMinWidth(long nWidth);
        long                    GetMinWidth();

        void                    ExpandSection();
        void                    ShrinkSection();
        BOOL                    IsExpanded();
};

#endif
