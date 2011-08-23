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
#include <bf_svtools/expander.hxx>
#endif

#ifndef _SV_PRIVSPLT_HXX
#include <bf_svtools/privsplt.hxx>
#endif

namespace binfilter
{

class SvSectionControl;

class SvSection : public DockingWindow
{
private:
        SvSectionControl*		pSectionControl;
        USHORT					nSectionID;
        Window*					pChildWin;
        XubString				aName;
        long					nHeight;
        long					nMinHeight;
        long					nMinWidth;
        long					nOldHeight;

        DECL_LINK( ToggleHdl,	SvExpander* );
        DECL_LINK( SplitHdl,	SvPrivatSplit* );

        void					ImplExpandSection();
        void					ImplShrinkSection();

protected:
        SvExpander				aExpander;
        Window					aChildWinContainer;
        SvPrivatSplit			aPrivatSplit;

        virtual void			Resize();
        virtual void			Paint( const Rectangle& rRect );
        virtual long			Notify( NotifyEvent& rNEvt );

public:
        SvSection(const XubString& rName,SvSectionControl* pParent,
                    WinBits nStyle = 0);
        SvSection(const XubString& rName,SvSectionControl* pParent, 
                    Window* pChildWin, WinBits nStyle = 0);

        virtual void			SetScrollPos(long nPos);
        void					SetChildPos(long nPos);

        void					SetSectionControl(SvSectionControl* pParent);
        const SvSectionControl*	GetSectionControl()const ;
        void					SetSectionText( const XubString& );
        const XubString&			GetSectionText() const {return aName;}

        void					SetChildWin(Window* pChildWin);
        Window*					GetChildWin() const {return pChildWin;}

        void					SetSectionID(USHORT nId) {nSectionID=nId;}
        USHORT					GetSectionID() const {return nSectionID;}

        void					SetMaxHeight(long nHeight);
        long					GetMaxHeight();

        void					SetMinHeight(long nHeight);
        long					GetMinHeight();

        void					SetMinWidth(long nWidth);
        long					GetMinWidth();

        void					ExpandSection();
        void					ShrinkSection();
        BOOL					IsExpanded();
};							

}

#endif	
