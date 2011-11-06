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



#ifndef SC_PARAWIN_HXX
#define SC_PARAWIN_HXX

#include "funcutl.hxx"
#include "global.hxx"       // ScAddress
#include <svtools/stdctrl.hxx>
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
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

        ::std::vector<sal_uInt16>   aVisibleArgMapping;
        const ScFuncDesc*   pFuncDesc;
        ScAnyRefDlg*    pMyParent;
        sal_uInt16          nArgs;      // unsuppressed arguments
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
        sal_Bool            bRefMode;

        sal_uInt16          nEdFocus;
        sal_uInt16          nActiveLine;

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

        void            InitArgInput( sal_uInt16 nPos, FixedText& rFtArg, ImageButton& rBtnFx,
                                        ArgEdit& rEdArg, formula::RefButton& rRefBtn);

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
                        ScParaWin(ScAnyRefDlg* pParent,Point aPos);
                        ~ScParaWin();

        void            SetFunctionDesc(const ScFuncDesc* pFDesc);
        void            SetArgumentOffset(sal_uInt16 nOffset);
        void            SetEditDesc(const String& aText);
        void            UpdateParas();
        void            ClearAll();

        sal_Bool            IsRefMode() {return bRefMode;}
        void            SetRefMode(sal_Bool bFlag) {bRefMode=bFlag;}

        sal_uInt16          GetActiveLine();
        void            SetActiveLine(sal_uInt16 no);
        formula::RefEdit*       GetActiveEdit();
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





#endif // SC_PARAWIN_HXX

