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

#ifndef FORMULA_PARAWIN_HXX
#define FORMULA_PARAWIN_HXX

#include <svtools/stdctrl.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/scrbar.hxx>

#include <vector>
#include "formula/funcutl.hxx"
#include "ControlHelper.hxx"
#include "ModuleHelper.hxx"

namespace formula
{
//============================================================================
#define NOT_FOUND 0xffff
//============================================================================
class IFunctionDescription;
class IControlReferenceHandler;

class ParaWin : public TabPage
{
private:
        OModuleClient   m_aModuleClient;
        Link            aScrollLink;
        Link            aFxLink;
        Link            aArgModifiedLink;

        ::std::vector<sal_uInt16>   aVisibleArgMapping;
        const IFunctionDescription* pFuncDesc;
        IControlReferenceHandler*   pMyParent;
        sal_uInt16          nArgs;      // unsuppressed arguments
        Font            aFntBold;
        Font            aFntLight;

        FixedInfo       aFtEditDesc;
        FixedText       aFtArgName;
        FixedInfo       aFtArgDesc;

        FixedText       aFtArg1;
        FixedText       aFtArg2;
        FixedText       aFtArg3;
        FixedText       aFtArg4;

        ImageButton     aBtnFx1;
        ImageButton     aBtnFx2;
        ImageButton     aBtnFx3;
        ImageButton     aBtnFx4;

        ArgEdit         aEdArg1;
        ArgEdit         aEdArg2;
        ArgEdit         aEdArg3;
        ArgEdit         aEdArg4;

        RefButton   aRefBtn1;
        RefButton   aRefBtn2;
        RefButton   aRefBtn3;
        RefButton   aRefBtn4;

        ScrollBar       aSlider;
        String          m_sOptional;
        String          m_sRequired;
        sal_Bool            bRefMode;

        sal_uInt16          nEdFocus;
        sal_uInt16          nActiveLine;

        ArgInput        aArgInput[4];
        String          aDefaultString;
        ::std::vector<String>
                        aParaArray;

        DECL_LINK( ScrollHdl, ScrollBar* );
        DECL_LINK( ModifyHdl, ArgInput* );
        DECL_LINK( GetEdFocusHdl, ArgInput* );
        DECL_LINK( GetFxFocusHdl, ArgInput* );
        DECL_LINK( GetFxHdl, ArgInput* );

protected:

        virtual void    SliderMoved();
        virtual void    ArgumentModified();
        virtual void    FxClick();

        void            InitArgInput( sal_uInt16 nPos, FixedText& rFtArg, ImageButton& rBtnFx,
                                        ArgEdit& rEdArg, RefButton& rRefBtn);

        void            DelParaArray();
        void            SetArgumentDesc(const String& aText);
        void            SetArgumentText(const String& aText);


        void            SetArgName      (sal_uInt16 no,const String &aArg);
        void            SetArgNameFont  (sal_uInt16 no,const Font&);
        void            SetArgVal       (sal_uInt16 no,const String &aArg);

        void            HideParaLine(sal_uInt16 no);
        void            ShowParaLine(sal_uInt16 no);
        void            UpdateArgDesc( sal_uInt16 nArg );
        void            UpdateArgInput( sal_uInt16 nOffset, sal_uInt16 i );

public:
                        ParaWin(Window* pParent,IControlReferenceHandler* _pDlg,Point aPos);
                        ~ParaWin();

        void            SetFunctionDesc(const IFunctionDescription* pFDesc);
        void            SetArgumentOffset(sal_uInt16 nOffset);
        void            SetEditDesc(const String& aText);
        void            UpdateParas();
        void            ClearAll();

        sal_Bool            IsRefMode() {return bRefMode;}
        void            SetRefMode(sal_Bool bFlag) {bRefMode=bFlag;}

        sal_uInt16          GetActiveLine();
        void            SetActiveLine(sal_uInt16 no);
        RefEdit*        GetActiveEdit();
        String          GetActiveArgName();

        String          GetArgument(sal_uInt16 no);
        void            SetArgument(sal_uInt16 no, const String& aString);
        void            SetArgumentFonts(const Font&aBoldFont,const Font&aLightFont);

        void            SetEdFocus(sal_uInt16 nEditLine); //Sichtbare Editzeilen
        sal_uInt16          GetSliderPos();
        void            SetSliderPos(sal_uInt16 nSliderPos);

        void            SetScrollHdl( const Link& rLink ) { aScrollLink = rLink; }
        const Link&     GetScrollHdl() const { return aScrollLink; }

        void            SetArgModifiedHdl( const Link& rLink ) { aArgModifiedLink = rLink; }
        const Link&     GetArgModifiedHdl() const { return aArgModifiedLink; }

        void            SetFxHdl( const Link& rLink ) { aFxLink = rLink; }
        const Link&     GetFxHdl() const { return aFxLink; }
};



} // formula

#endif // FORMULA_PARAWIN_HXX

