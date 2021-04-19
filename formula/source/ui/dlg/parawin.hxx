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

#pragma once

#include <formula/funcutl.hxx>
#include <vcl/weld.hxx>
#include "ControlHelper.hxx"
#include <vector>

namespace formula
{

#define NOT_FOUND 0xffff

class IFunctionDescription;
class IControlReferenceHandler;

class ParaWin
{
private:
        Link<ParaWin&,void>  aFxLink;
        Link<ParaWin&,void>  aArgModifiedLink;

        ::std::vector<sal_uInt16>   aVisibleArgMapping;
        const IFunctionDescription* pFuncDesc;
        IControlReferenceHandler*   pMyParent;
        sal_uInt16       nArgs;     // unsuppressed arguments, may be >= VAR_ARGS to indicate repeating parameters
        sal_uInt16       nMaxArgs;  // max arguments, limited to supported number of arguments
        vcl::Font        aFntBold;
        vcl::Font        aFntLight;

        OUString        m_sOptional;
        OUString        m_sRequired;

        sal_uInt16      nEdFocus;
        sal_uInt16      nActiveLine;

        ArgInput        aArgInput[4];
        OUString        aDefaultString;
        ::std::vector<OUString> aParaArray;

        std::unique_ptr<weld::Builder> m_xBuilder;
        std::unique_ptr<weld::Container> m_xContainer;

        std::unique_ptr<weld::ScrolledWindow> m_xSlider;
        std::unique_ptr<weld::Widget> m_xParamGrid;
        std::unique_ptr<weld::Widget> m_xGrid;

        std::unique_ptr<weld::Label> m_xFtEditDesc;
        std::unique_ptr<weld::Label> m_xFtArgName;
        std::unique_ptr<weld::Label> m_xFtArgDesc;

        std::unique_ptr<weld::Button> m_xBtnFx1;
        std::unique_ptr<weld::Button> m_xBtnFx2;
        std::unique_ptr<weld::Button> m_xBtnFx3;
        std::unique_ptr<weld::Button> m_xBtnFx4;

        std::unique_ptr<weld::Label> m_xFtArg1;
        std::unique_ptr<weld::Label> m_xFtArg2;
        std::unique_ptr<weld::Label> m_xFtArg3;
        std::unique_ptr<weld::Label> m_xFtArg4;

        std::unique_ptr<ArgEdit> m_xEdArg1;
        std::unique_ptr<ArgEdit> m_xEdArg2;
        std::unique_ptr<ArgEdit> m_xEdArg3;
        std::unique_ptr<ArgEdit> m_xEdArg4;

        std::unique_ptr<RefButton> m_xRefBtn1;
        std::unique_ptr<RefButton> m_xRefBtn2;
        std::unique_ptr<RefButton> m_xRefBtn3;
        std::unique_ptr<RefButton> m_xRefBtn4;

        DECL_LINK( ScrollHdl, weld::ScrolledWindow&, void);
        DECL_LINK( ModifyHdl, ArgInput&, void );
        DECL_LINK( GetEdFocusHdl, ArgInput&, void );
        DECL_LINK( GetFxFocusHdl, ArgInput&, void );
        DECL_LINK( GetFxHdl, ArgInput&, void );

        void            ArgumentModified();

        void            InitArgInput(sal_uInt16 nPos, weld::Label& rFtArg, weld::Button& rBtnFx,
                                     ArgEdit& rEdArg, RefButton& rRefBtn);

        void            SetArgumentDesc(const OUString& aText);
        void            SetArgumentText(const OUString& aText);


        void            SetArgName      (sal_uInt16 no,const OUString &aArg);
        void            SetArgNameFont  (sal_uInt16 no,const vcl::Font&);

        void            UpdateArgDesc( sal_uInt16 nArg );
        void            UpdateArgInput( sal_uInt16 nOffset, sal_uInt16 i );

public:
        ParaWin(weld::Container* pParent, IControlReferenceHandler* _pDlg);
        ~ParaWin();

        void            SetFunctionDesc(const IFunctionDescription* pFDesc);
        void            SetArgumentOffset(sal_uInt16 nOffset);
        void            SetEditDesc(const OUString& aText);
        void            UpdateParas();
        void            ClearAll();

        sal_uInt16      GetActiveLine() const { return nActiveLine;}
        void            SetActiveLine(sal_uInt16 no);
        RefEdit*    GetActiveEdit();
        OUString        GetActiveArgName() const;

        OUString        GetArgument(sal_uInt16 no);
        void            SetArgument(sal_uInt16 no, std::u16string_view aString);
        void            SetArgumentFonts(const vcl::Font& aBoldFont,const vcl::Font& aLightFont);

        void            SetEdFocus(); // visible edit lines
        sal_uInt16      GetSliderPos() const;
        void            SetSliderPos(sal_uInt16 nSliderPos);

        void            SetArgModifiedHdl( const Link<ParaWin&,void>& rLink ) { aArgModifiedLink = rLink; }
        void            SetFxHdl( const Link<ParaWin&,void>& rLink ) { aFxLink = rLink; }

        void            SliderMoved();

        void            Show() { m_xContainer->show(); }
};


} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
