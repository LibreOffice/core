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

#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>

#include "parawin.hxx"
#include <formula/IFunctionDescription.hxx>
#include <formula/funcvarargs.h>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <core_resource.hxx>

namespace formula
{

// Formula token argument count is sal_uInt8, max 255, edit offset 254.
constexpr sal_uInt16 kMaxArgCount = 255;
constexpr sal_uInt16 kMaxArgOffset = kMaxArgCount - 1;

ParaWin::ParaWin(weld::Container* pParent,IControlReferenceHandler* _pDlg)
    : pFuncDesc(nullptr)
    , pMyParent(_pDlg)
    , m_sOptional(ForResId(STR_OPTIONAL))
    , m_sRequired(ForResId(STR_REQUIRED))
    , m_xBuilder(Application::CreateBuilder(pParent, "formula/ui/parameter.ui"))
    , m_xContainer(m_xBuilder->weld_container("ParameterPage"))
    , m_xSlider(m_xBuilder->weld_scrolled_window("scrollbar", true))
    , m_xParamGrid(m_xBuilder->weld_widget("paramgrid"))
    , m_xGrid(m_xBuilder->weld_widget("grid"))
    , m_xFtEditDesc(m_xBuilder->weld_label("editdesc"))
    , m_xFtArgName(m_xBuilder->weld_label("parname"))
    , m_xFtArgDesc(m_xBuilder->weld_label("pardesc"))
    , m_xBtnFx1(m_xBuilder->weld_button("FX1"))
    , m_xBtnFx2(m_xBuilder->weld_button("FX2"))
    , m_xBtnFx3(m_xBuilder->weld_button("FX3"))
    , m_xBtnFx4(m_xBuilder->weld_button("FX4"))
    , m_xFtArg1(m_xBuilder->weld_label("FT_ARG1"))
    , m_xFtArg2(m_xBuilder->weld_label("FT_ARG2"))
    , m_xFtArg3(m_xBuilder->weld_label("FT_ARG3"))
    , m_xFtArg4(m_xBuilder->weld_label("FT_ARG4"))
    , m_xEdArg1(new ArgEdit(m_xBuilder->weld_entry("ED_ARG1")))
    , m_xEdArg2(new ArgEdit(m_xBuilder->weld_entry("ED_ARG2")))
    , m_xEdArg3(new ArgEdit(m_xBuilder->weld_entry("ED_ARG3")))
    , m_xEdArg4(new ArgEdit(m_xBuilder->weld_entry("ED_ARG4")))
    , m_xRefBtn1(new RefButton(m_xBuilder->weld_button("RB_ARG1")))
    , m_xRefBtn2(new RefButton(m_xBuilder->weld_button("RB_ARG2")))
    , m_xRefBtn3(new RefButton(m_xBuilder->weld_button("RB_ARG3")))
    , m_xRefBtn4(new RefButton(m_xBuilder->weld_button("RB_ARG4")))
{
    // Space for three lines of text in function description.
    m_xFtEditDesc->set_label("X\nX\nX\n");
    auto nEditHeight = m_xFtEditDesc->get_preferred_size().Height();
    m_xFtEditDesc->set_size_request(-1, nEditHeight);
    m_xFtEditDesc->set_label("");
    // Space for two lines of text in parameter description.
    m_xFtArgDesc->set_label("X\nX\n");
    auto nArgHeight = m_xFtArgDesc->get_preferred_size().Height();
    m_xFtArgDesc->set_size_request(-1, nArgHeight);
    m_xFtArgDesc->set_label("");

    m_xBtnFx1->set_from_icon_name(BMP_FX);
    m_xBtnFx2->set_from_icon_name(BMP_FX);
    m_xBtnFx3->set_from_icon_name(BMP_FX);
    m_xBtnFx4->set_from_icon_name(BMP_FX);

    //lock down initial preferences
    m_xParamGrid->set_size_request(-1, m_xParamGrid->get_preferred_size().Height());
    Size aSize(m_xContainer->get_preferred_size());
    m_xContainer->set_size_request(aSize.Width(), aSize.Height());

    aDefaultString = m_xFtEditDesc->get_label();
    nEdFocus = NOT_FOUND;
    nActiveLine = 0;

    m_xSlider->connect_vadjustment_changed(LINK(this, ParaWin, ScrollHdl));

    InitArgInput( 0, *m_xFtArg1, *m_xBtnFx1, *m_xEdArg1, *m_xRefBtn1);
    InitArgInput( 1, *m_xFtArg2, *m_xBtnFx2, *m_xEdArg2, *m_xRefBtn2);
    InitArgInput( 2, *m_xFtArg3, *m_xBtnFx3, *m_xEdArg3, *m_xRefBtn3);
    InitArgInput( 3, *m_xFtArg4, *m_xBtnFx4, *m_xEdArg4, *m_xRefBtn4);
    ClearAll();
}

void ParaWin::UpdateArgDesc( sal_uInt16 nArg )
{
    if (nArg == NOT_FOUND)
        return;

    if (nMaxArgs > 4)
        nArg = sal::static_int_cast<sal_uInt16>( nArg + GetSliderPos() );

    if ((nMaxArgs <= 0) || (nArg >= nMaxArgs))
        return;

    OUString  aArgDesc;
    OUString  aArgName;

    SetArgumentDesc( OUString() );
    SetArgumentText( OUString() );

    if ( nArgs < VAR_ARGS )
    {
        sal_uInt16 nRealArg = (nArg < aVisibleArgMapping.size()) ? aVisibleArgMapping[nArg] : nArg;
        aArgDesc  = pFuncDesc->getParameterDescription(nRealArg);
        aArgName  = pFuncDesc->getParameterName(nRealArg) + " " +
            ((pFuncDesc->isParameterOptional(nRealArg)) ? m_sOptional : m_sRequired);
    }
    else if ( nArgs < PAIRED_VAR_ARGS )
    {
        sal_uInt16 nFix = nArgs - VAR_ARGS;
        sal_uInt16 nPos = std::min( nArg, nFix );
        sal_uInt16 nRealArg = (nPos < aVisibleArgMapping.size() ?
                aVisibleArgMapping[nPos] : aVisibleArgMapping.back());
        aArgDesc  = pFuncDesc->getParameterDescription(nRealArg);
        aArgName  = pFuncDesc->getParameterName(nRealArg);
        sal_uInt16 nVarArgsStart = pFuncDesc->getVarArgsStart();
        if ( nArg >= nVarArgsStart )
            aArgName += OUString::number( nArg-nVarArgsStart+1 );
        aArgName += " " + ((nArg > nFix || pFuncDesc->isParameterOptional(nRealArg)) ? m_sOptional : m_sRequired) ;
    }
    else
    {
        sal_uInt16 nFix = nArgs - PAIRED_VAR_ARGS;
        sal_uInt16 nPos;
        if ( nArg < nFix )
            nPos = nArg;
        else
            nPos = nFix + ( (nArg-nFix) % 2);
        sal_uInt16 nRealArg = (nPos < aVisibleArgMapping.size() ?
                aVisibleArgMapping[nPos] : aVisibleArgMapping.back());
        aArgDesc  = pFuncDesc->getParameterDescription(nRealArg);
        aArgName  = pFuncDesc->getParameterName(nRealArg);
        sal_uInt16 nVarArgsStart = pFuncDesc->getVarArgsStart();
        if ( nArg >= nVarArgsStart )
            aArgName += OUString::number( (nArg-nVarArgsStart)/2 + 1 );
        aArgName += " " + ((nArg > (nFix+1) || pFuncDesc->isParameterOptional(nRealArg)) ? m_sOptional : m_sRequired) ;
    }

    SetArgumentDesc(aArgDesc);
    SetArgumentText(aArgName);
}

void ParaWin::UpdateArgInput( sal_uInt16 nOffset, sal_uInt16 i )
{
    sal_uInt16 nArg = nOffset + i;
    if (nArg > kMaxArgOffset)
        return;

    if ( nArgs < VAR_ARGS)
    {
        if (nArg < nMaxArgs)
        {
            sal_uInt16 nRealArg = aVisibleArgMapping[nArg];
            SetArgNameFont  (i,(pFuncDesc->isParameterOptional(nRealArg))
                                            ? aFntLight : aFntBold );
            SetArgName      (i,pFuncDesc->getParameterName(nRealArg));
        }
    }
    else if ( nArgs < PAIRED_VAR_ARGS)
    {
        sal_uInt16 nFix = nArgs - VAR_ARGS;
        sal_uInt16 nPos = std::min( nArg, nFix );
        sal_uInt16 nRealArg = (nPos < aVisibleArgMapping.size() ?
                aVisibleArgMapping[nPos] : aVisibleArgMapping.back());
        SetArgNameFont( i,
                (nArg > nFix || pFuncDesc->isParameterOptional(nRealArg)) ?
                aFntLight : aFntBold );
        sal_uInt16 nVarArgsStart = pFuncDesc->getVarArgsStart();
        if ( nArg >= nVarArgsStart )
        {
            OUString aArgName = pFuncDesc->getParameterName(nRealArg) +
                OUString::number(nArg-nVarArgsStart+1);
            SetArgName( i, aArgName );
        }
        else
            SetArgName( i, pFuncDesc->getParameterName(nRealArg) );
    }
    else
    {
        sal_uInt16 nFix = nArgs - PAIRED_VAR_ARGS;
        sal_uInt16 nPos;
        if ( nArg < nFix )
            nPos = nArg;
        else
            nPos = nFix + ( (nArg-nFix) % 2);
        sal_uInt16 nRealArg = (nPos < aVisibleArgMapping.size() ?
                aVisibleArgMapping[nPos] : aVisibleArgMapping.back());
        SetArgNameFont( i,
                (nArg > (nFix+1) || pFuncDesc->isParameterOptional(nRealArg)) ?
                aFntLight : aFntBold );
        sal_uInt16 nVarArgsStart = pFuncDesc->getVarArgsStart();
        if ( nArg >= nVarArgsStart )
        {
            OUString aArgName = pFuncDesc->getParameterName(nRealArg) +
                OUString::number( (nArg-nVarArgsStart)/2 + 1 );
            SetArgName( i, aArgName );
        }
        else
            SetArgName( i, pFuncDesc->getParameterName(nRealArg) );
    }
    if (nArg < nMaxArgs)
        aArgInput[i].SetArgVal(aParaArray[nArg]);
}

ParaWin::~ParaWin()
{
    // #i66422# if the focus changes during destruction of the controls,
    // don't call the focus handlers
    Link<weld::Widget&,void> aEmptyLink;
    m_xBtnFx1->connect_focus_in(aEmptyLink);
    m_xBtnFx2->connect_focus_in(aEmptyLink);
    m_xBtnFx3->connect_focus_in(aEmptyLink);
    m_xBtnFx4->connect_focus_in(aEmptyLink);
}

void ParaWin::SetActiveLine(sal_uInt16 no)
{
    if (no >= nMaxArgs)
        return;

    long nOffset = GetSliderPos();
    nActiveLine=no;
    long nNewEdPos=static_cast<long>(nActiveLine)-nOffset;
    if(nNewEdPos<0 || nNewEdPos>3)
    {
        nOffset+=nNewEdPos;
        SetSliderPos(static_cast<sal_uInt16>(nOffset));
        nOffset=GetSliderPos();
    }
    nEdFocus=no-static_cast<sal_uInt16>(nOffset);
    UpdateArgDesc( nEdFocus );
}

RefEdit* ParaWin::GetActiveEdit()
{
    if (nMaxArgs > 0 && nEdFocus != NOT_FOUND)
    {
        return aArgInput[nEdFocus].GetArgEdPtr();
    }
    else
    {
        return nullptr;
    }
}


OUString ParaWin::GetArgument(sal_uInt16 no)
{
    OUString aStr;
    if(no<aParaArray.size())
    {
        aStr=aParaArray[no];
        if(no==nActiveLine && aStr.isEmpty())
            aStr += " ";
    }
    return aStr;
}

OUString  ParaWin::GetActiveArgName() const
{
    OUString aStr;
    if (nMaxArgs > 0 && nEdFocus != NOT_FOUND)
    {
        aStr=aArgInput[nEdFocus].GetArgName();
    }
    return aStr;
}


void ParaWin::SetArgument(sal_uInt16 no, const OUString& aString)
{
    if (no < aParaArray.size())
        aParaArray[no] = comphelper::string::stripStart(aString, ' ');
}

void ParaWin::SetArgumentFonts(const vcl::Font&aBoldFont,const vcl::Font&aLightFont)
{
    aFntBold=aBoldFont;
    aFntLight=aLightFont;
}

void ParaWin::SetFunctionDesc(const IFunctionDescription* pFDesc)
{
    pFuncDesc=pFDesc;

    SetArgumentDesc( OUString() );
    SetArgumentText( OUString() );
    SetEditDesc( OUString() );
    nMaxArgs = nArgs = 0;
    if ( pFuncDesc!=nullptr)
    {
        if ( !pFuncDesc->getDescription().isEmpty() )
        {
            SetEditDesc(pFuncDesc->getDescription());
        }
        else
        {
            SetEditDesc(aDefaultString);
        }
        nArgs = pFuncDesc->getSuppressedArgumentCount();
        nMaxArgs = std::min( nArgs, kMaxArgCount);
        if (sal_uInt16 nVarArgsLimit = pFuncDesc->getVarArgsLimit())
            nMaxArgs = std::min( nVarArgsLimit, nMaxArgs);
        pFuncDesc->fillVisibleArgumentMapping(aVisibleArgMapping);
        m_xSlider->set_vpolicy(VclPolicyType::NEVER);
        m_xSlider->set_size_request(-1, -1);
        OString sHelpId = pFuncDesc->getHelpId();
        m_xContainer->set_help_id(sHelpId);
        m_xEdArg1->GetWidget()->set_help_id(sHelpId);
        m_xEdArg2->GetWidget()->set_help_id(sHelpId);
        m_xEdArg3->GetWidget()->set_help_id(sHelpId);
        m_xEdArg4->GetWidget()->set_help_id(sHelpId);

        SetActiveLine(0);
    }
    else
    {
        nActiveLine=0;
    }

}

void ParaWin::SetArgumentText(const OUString& aText)
{
    m_xFtArgName->set_label(aText);
}

void ParaWin::SetArgumentDesc(const OUString& aText)
{
    m_xFtArgDesc->set_label(aText);
}

void ParaWin::SetEditDesc(const OUString& aText)
{
    m_xFtEditDesc->set_label(aText);
}

void ParaWin::SetArgName(sal_uInt16 no,const OUString& aText)
{
    aArgInput[no].SetArgName(aText);
    aArgInput[no].UpdateAccessibleNames();
}

void ParaWin::SetArgNameFont(sal_uInt16 no,const vcl::Font& aFont)
{
    aArgInput[no].SetArgNameFont(aFont);
}

void ParaWin::SetEdFocus()
{
    UpdateArgDesc(0);
    if(!aParaArray.empty())
        aArgInput[0].GetArgEdPtr()->GrabFocus();
}

void ParaWin::InitArgInput(sal_uInt16 nPos, weld::Label& rFtArg, weld::Button& rBtnFx,
                           ArgEdit& rEdArg, RefButton& rRefBtn)
{

    rRefBtn.SetReferences(pMyParent, &rEdArg);
    rEdArg.SetReferences(pMyParent, &rFtArg);

    aArgInput[nPos].InitArgInput (&rFtArg,&rBtnFx,&rEdArg,&rRefBtn);

    aArgInput[nPos].Hide();

    aArgInput[nPos].SetFxClickHdl   ( LINK( this, ParaWin, GetFxHdl ) );
    aArgInput[nPos].SetFxFocusHdl   ( LINK( this, ParaWin, GetFxFocusHdl ) );
    aArgInput[nPos].SetEdFocusHdl   ( LINK( this, ParaWin, GetEdFocusHdl ) );
    aArgInput[nPos].SetEdModifyHdl  ( LINK( this, ParaWin, ModifyHdl ) );
    aArgInput[nPos].UpdateAccessibleNames();
}

void ParaWin::ClearAll()
{
    SetFunctionDesc(nullptr);
    SetArgumentOffset(0);
}

void ParaWin::SetArgumentOffset(sal_uInt16 nOffset)
{
    aParaArray.clear();
    m_xSlider->vadjustment_set_value(0);

    aParaArray.resize(nMaxArgs);

    if (nMaxArgs > 0)
    {
        for ( int i=0; i<4 && i<nMaxArgs; i++ )
        {
            aArgInput[i].SetArgVal(OUString());
            aArgInput[i].GetArgEdPtr()->Init(
                (i==0)                  ? nullptr : aArgInput[i-1].GetArgEdPtr(),
                (i==3 || i==nMaxArgs-1) ? nullptr : aArgInput[i+1].GetArgEdPtr(),
                                          *m_xSlider, *this, nMaxArgs );
        }
    }

    UpdateParas();

    if (nMaxArgs < 5)
    {
        m_xSlider->set_vpolicy(VclPolicyType::NEVER);
        m_xSlider->set_size_request(-1, -1);
    }
    else
    {
        m_xSlider->vadjustment_configure(nOffset, 0, nMaxArgs, 1, 4, 4);
        m_xSlider->set_vpolicy(VclPolicyType::ALWAYS);
        Size aPrefSize(m_xGrid->get_preferred_size());
        m_xSlider->set_size_request(aPrefSize.Width(), aPrefSize.Height());
    }
}

void ParaWin::UpdateParas()
{
    sal_uInt16 i;
    sal_uInt16 nOffset = GetSliderPos();

    if ( nMaxArgs > 0 )
    {
        for ( i=0; (i<nMaxArgs) && (i<4); i++ )
        {
            UpdateArgInput( nOffset, i );
            aArgInput[i].Show();
        }
    }

    for ( i=nMaxArgs; i<4; i++ )
        aArgInput[i].Hide();
}


sal_uInt16 ParaWin::GetSliderPos() const
{
    return static_cast<sal_uInt16>(m_xSlider->vadjustment_get_value());
}

void ParaWin::SetSliderPos(sal_uInt16 nSliderPos)
{
    sal_uInt16 nOffset = GetSliderPos();

    if(m_xSlider->get_visible() && nOffset!=nSliderPos)
    {
        m_xSlider->vadjustment_set_value(nSliderPos);
        for ( sal_uInt16 i=0; i<4; i++ )
        {
            UpdateArgInput( nSliderPos, i );
        }
    }
}

void ParaWin::SliderMoved()
{
    sal_uInt16 nOffset = GetSliderPos();

    for ( sal_uInt16 i=0; i<4; i++ )
    {
        UpdateArgInput( nOffset, i );
    }
    if(nEdFocus!=NOT_FOUND)
    {
        UpdateArgDesc( nEdFocus );
        aArgInput[nEdFocus].SelectAll();
        nActiveLine=nEdFocus+nOffset;
        ArgumentModified();
        aArgInput[nEdFocus].SelectAll(); // ensure all is still selected
        aArgInput[nEdFocus].UpdateAccessibleNames();
    }
}

void ParaWin::ArgumentModified()
{
    aArgModifiedLink.Call(*this);
}

IMPL_LINK( ParaWin, GetFxHdl, ArgInput&, rPtr, void )
{
    sal_uInt16 nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for (size_t nPos=0; nPos < SAL_N_ELEMENTS(aArgInput); ++nPos)
    {
        if(&rPtr == &aArgInput[nPos])
        {
            nEdFocus=nPos;
            break;
        }
    }

    if(nEdFocus!=NOT_FOUND)
    {
        aArgInput[nEdFocus].SelectAll();
        nActiveLine=nEdFocus+nOffset;
        aFxLink.Call(*this);
    }
}

IMPL_LINK( ParaWin, GetFxFocusHdl, ArgInput&, rPtr, void )
{
    sal_uInt16 nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for (size_t nPos=0; nPos < SAL_N_ELEMENTS(aArgInput); ++nPos)
    {
        if(&rPtr == &aArgInput[nPos])
        {
            nEdFocus=nPos;
            break;
        }
    }

    if(nEdFocus!=NOT_FOUND)
    {
        aArgInput[nEdFocus].SelectAll();
        UpdateArgDesc( nEdFocus );
        nActiveLine=nEdFocus+nOffset;
    }
}

IMPL_LINK( ParaWin, GetEdFocusHdl, ArgInput&, rPtr, void )
{
    sal_uInt16 nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for (size_t nPos=0; nPos < SAL_N_ELEMENTS(aArgInput); ++nPos)
    {
        if(&rPtr == &aArgInput[nPos])
        {
            nEdFocus=nPos;
            break;
        }
    }

    if(nEdFocus!=NOT_FOUND)
    {
        aArgInput[nEdFocus].SelectAll();
        UpdateArgDesc( nEdFocus );
        nActiveLine=nEdFocus+nOffset;
        ArgumentModified();
        aArgInput[nEdFocus].SelectAll(); // ensure all is still selected
        aArgInput[nEdFocus].UpdateAccessibleNames();
    }
}

IMPL_LINK_NOARG(ParaWin, ScrollHdl, weld::ScrolledWindow&, void)
{
    SliderMoved();
}

IMPL_LINK( ParaWin, ModifyHdl, ArgInput&, rPtr, void )
{
    sal_uInt16 nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for (size_t nPos=0; nPos < SAL_N_ELEMENTS(aArgInput); ++nPos)
    {
        if(&rPtr == &aArgInput[nPos])
        {
            nEdFocus=nPos;
            break;
        }
    }
    if(nEdFocus!=NOT_FOUND)
    {
        size_t nPara = nEdFocus + nOffset;
        if (nPara < aParaArray.size())
            aParaArray[nPara] = aArgInput[nEdFocus].GetArgVal();
        else
        {
            SAL_WARN("formula.ui","ParaWin::ModifyHdl - shot in foot: nPara " <<
                    nPara << " >= aParaArray.size() " << aParaArray.size() <<
                    " with nEdFocus " << nEdFocus <<
                    " and aArgInput[nEdFocus].GetArgVal() '" << aArgInput[nEdFocus].GetArgVal() << "'");
        }
        UpdateArgDesc( nEdFocus);
        nActiveLine = static_cast<sal_uInt16>(nPara);
    }

    ArgumentModified();
}


} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
