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

#ifndef SC_PARAWIN_HXX
#define SC_PARAWIN_HXX

#include "funcutl.hxx"
#include "global.hxx"       // ScAddress
#include <svtools/stdctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>

#ifndef _SVSTDARR_STRINGS

#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>

#endif
#include <vcl/tabctrl.hxx>

#include <vector>

class ScFuncDesc;

//============================================================================
#define NOT_FOUND 0xffff
//============================================================================

class ScParaWin : public TabPage
{
private:

        Link            aScrollLink;
        Link            aFxLink;
        Link            aArgModifiedLink;

        ::std::vector<USHORT>   aVisibleArgMapping;
        const ScFuncDesc*   pFuncDesc;
        ScAnyRefDlg*    pMyParent;
        USHORT          nArgs;      // unsuppressed arguments
        Font            aFntBold;
        Font            aFntLight;

        FixedInfo       aFtEditDesc;
        FixedText       aFtArgName;
        FixedInfo       aFtArgDesc;

        ImageButton     aBtnFx1;
        FixedText       aFtArg1;
        ArgEdit         aEdArg1;
        formula::RefButton      aRefBtn1;
        ImageButton     aBtnFx2;
        FixedText       aFtArg2;
        ArgEdit         aEdArg2;
        formula::RefButton      aRefBtn2;
        ImageButton     aBtnFx3;
        FixedText       aFtArg3;
        ArgEdit         aEdArg3;
        formula::RefButton      aRefBtn3;
        ImageButton     aBtnFx4;
        FixedText       aFtArg4;
        ArgEdit         aEdArg4;
        formula::RefButton      aRefBtn4;
        ScrollBar       aSlider;
        BOOL            bRefMode;

        USHORT          nEdFocus;
        USHORT          nActiveLine;

        ArgInput        aArgInput[4];
        String          aDefaultString;
        SvStrings       aParaArray;
        DECL_LINK( ScrollHdl, ScrollBar* );
        DECL_LINK( ModifyHdl, ArgInput* );
        DECL_LINK( GetEdFocusHdl, ArgInput* );
        DECL_LINK( GetFxFocusHdl, ArgInput* );
        DECL_LINK( GetFxHdl, ArgInput* );

protected:

        virtual void    SliderMoved();
        virtual void    ArgumentModified();
        virtual void    FxClick();

        void            InitArgInput( USHORT nPos, FixedText& rFtArg, ImageButton& rBtnFx,
                                        ArgEdit& rEdArg, formula::RefButton& rRefBtn);

        void            DelParaArray();
        void            SetArgumentDesc(const String& aText);
        void            SetArgumentText(const String& aText);


        void            SetArgName      (USHORT no,const String &aArg);
        void            SetArgNameFont  (USHORT no,const Font&);
        void            SetArgVal       (USHORT no,const String &aArg);

        void            HideParaLine(USHORT no);
        void            ShowParaLine(USHORT no);
        void            UpdateArgDesc( USHORT nArg );
        void            UpdateArgInput( USHORT nOffset, USHORT i );

public:
                        ScParaWin(ScAnyRefDlg* pParent,Point aPos);
                        ~ScParaWin();

        void            SetFunctionDesc(const ScFuncDesc* pFDesc);
        void            SetArgumentOffset(USHORT nOffset);
        void            SetEditDesc(const String& aText);
        void            UpdateParas();
        void            ClearAll();

        BOOL            IsRefMode() {return bRefMode;}
        void            SetRefMode(BOOL bFlag) {bRefMode=bFlag;}

        USHORT          GetActiveLine();
        void            SetActiveLine(USHORT no);
        formula::RefEdit*       GetActiveEdit();
        String          GetActiveArgName();

        String          GetArgument(USHORT no);
        void            SetArgument(USHORT no, const String& aString);
        void            SetArgumentFonts(const Font&aBoldFont,const Font&aLightFont);

        void            SetEdFocus(USHORT nEditLine); //Sichtbare Editzeilen
        USHORT          GetSliderPos();
        void            SetSliderPos(USHORT nSliderPos);

        void            SetScrollHdl( const Link& rLink ) { aScrollLink = rLink; }
        const Link&     GetScrollHdl() const { return aScrollLink; }

        void            SetArgModifiedHdl( const Link& rLink ) { aArgModifiedLink = rLink; }
        const Link&     GetArgModifiedHdl() const { return aArgModifiedLink; }

        void            SetFxHdl( const Link& rLink ) { aFxLink = rLink; }
        const Link&     GetFxHdl() const { return aFxLink; }
};





#endif // SC_PARAWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
