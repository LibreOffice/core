/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FORMULA_SOURCE_UI_DLG_PARAWIN_HXX
#define INCLUDED_FORMULA_SOURCE_UI_DLG_PARAWIN_HXX

#include <svtools/svmedit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/scrbar.hxx>

#include <vector>
#include "formula/funcutl.hxx"
#include "ControlHelper.hxx"

namespace formula
{

#define NOT_FOUND 0xffff

class IFunctionDescription;
class IControlReferenceHandler;

class ParaWin : public TabPage
{
private:
        Link<ParaWin&,void>  aFxLink;
        Link<ParaWin&,void>  aArgModifiedLink;

        ::std::vector<sal_uInt16>   aVisibleArgMapping;
        const IFunctionDescription* pFuncDesc;
        IControlReferenceHandler*   pMyParent;
        sal_uInt16          nArgs;      // unsuppressed arguments
        vcl::Font        aFntBold;
        vcl::Font        aFntLight;

        VclPtr<FixedText>       m_pFtEditDesc;
        VclPtr<FixedText>       m_pFtArgName;
        VclPtr<FixedText>       m_pFtArgDesc;

        VclPtr<PushButton>      m_pBtnFx1;
        VclPtr<FixedText>       m_pFtArg1;
        VclPtr<ArgEdit>         m_pEdArg1;
        VclPtr<RefButton>       m_pRefBtn1;

        VclPtr<PushButton>      m_pBtnFx2;
        VclPtr<FixedText>       m_pFtArg2;
        VclPtr<ArgEdit>         m_pEdArg2;
        VclPtr<RefButton>       m_pRefBtn2;

        VclPtr<PushButton>      m_pBtnFx3;
        VclPtr<FixedText>       m_pFtArg3;
        VclPtr<ArgEdit>         m_pEdArg3;
        VclPtr<RefButton>       m_pRefBtn3;

        VclPtr<PushButton>      m_pBtnFx4;
        VclPtr<FixedText>       m_pFtArg4;
        VclPtr<ArgEdit>         m_pEdArg4;
        VclPtr<RefButton>       m_pRefBtn4;

        VclPtr<ScrollBar>       m_pSlider;
        OUString        m_sOptional;
        OUString        m_sRequired;

        sal_uInt16      nEdFocus;
        sal_uInt16      nActiveLine;

        ArgInput        aArgInput[4];
        OUString        aDefaultString;
        ::std::vector<OUString>
                        aParaArray;

        DECL_LINK( ScrollHdl, ScrollBar*, void);
        DECL_LINK( ModifyHdl, ArgInput&, void );
        DECL_LINK( GetEdFocusHdl, ArgInput&, void );
        DECL_LINK( GetFxFocusHdl, ArgInput&, void );
        DECL_LINK( GetFxHdl, ArgInput&, void );

protected:

        void            SliderMoved();
        void            ArgumentModified();

        void            InitArgInput( sal_uInt16 nPos, FixedText& rFtArg, PushButton& rBtnFx,
                                        ArgEdit& rEdArg, RefButton& rRefBtn);

        void            SetArgumentDesc(const OUString& aText);
        void            SetArgumentText(const OUString& aText);


        void            SetArgName      (sal_uInt16 no,const OUString &aArg);
        void            SetArgNameFont  (sal_uInt16 no,const vcl::Font&);

        void            UpdateArgDesc( sal_uInt16 nArg );
        void            UpdateArgInput( sal_uInt16 nOffset, sal_uInt16 i );

public:
                        ParaWin(vcl::Window* pParent,IControlReferenceHandler* _pDlg);
                        virtual ~ParaWin() override;
        virtual void    dispose() override;

        void            SetFunctionDesc(const IFunctionDescription* pFDesc);
        void            SetArgumentOffset(sal_uInt16 nOffset);
        void            SetEditDesc(const OUString& aText);
        void            UpdateParas();
        void            ClearAll();

        sal_uInt16      GetActiveLine() { return nActiveLine;}
        void            SetActiveLine(sal_uInt16 no);
        RefEdit*        GetActiveEdit();
        OUString        GetActiveArgName();

        OUString        GetArgument(sal_uInt16 no);
        void            SetArgument(sal_uInt16 no, const OUString& aString);
        void            SetArgumentFonts(const vcl::Font& aBoldFont,const vcl::Font& aLightFont);

        void            SetEdFocus(); // visible edit lines
        sal_uInt16      GetSliderPos();
        void            SetSliderPos(sal_uInt16 nSliderPos);

        void            SetArgModifiedHdl( const Link<ParaWin&,void>& rLink ) { aArgModifiedLink = rLink; }
        void            SetFxHdl( const Link<ParaWin&,void>& rLink ) { aFxLink = rLink; }
};


} // formula

#endif // INCLUDED_FORMULA_SOURCE_UI_DLG_PARAWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
