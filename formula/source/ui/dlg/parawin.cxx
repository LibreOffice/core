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
#include <vcl/vclenum.hxx>
#include <sal/log.hxx>

#include "parawin.hxx"
#include <formula/IFunctionDescription.hxx>
#include <formula/funcvarargs.h>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <core_resource.hxx>
#include <rtl/math.hxx>

namespace formula
{

// Formula token argument count is sal_uInt8, max 255, edit offset 254.
constexpr sal_uInt16 kMaxArgCount = 255;
constexpr sal_uInt16 kMaxArgOffset = kMaxArgCount - 1;

ParaWin::ParaWin(weld::Container* pParent, IControlReferenceHandler& rDlg)
    : m_pFuncDesc(nullptr)
    , m_rMyParent(rDlg)
    , m_sOptional(ForResId(STR_OPTIONAL))
    , m_sRequired(ForResId(STR_REQUIRED))
    , m_xBuilder(Application::CreateBuilder(pParent, u"formula/ui/parameter.ui"_ustr))
    , m_xContainer(m_xBuilder->weld_container(u"ParameterPage"_ustr))
    , m_xSlider(m_xBuilder->weld_scrolled_window(u"scrollbar"_ustr, true))
    , m_xParamGrid(m_xBuilder->weld_widget(u"paramgrid"_ustr))
    , m_xGrid(m_xBuilder->weld_widget(u"grid"_ustr))
    , m_xFtEditDesc(m_xBuilder->weld_label(u"editdesc"_ustr))
    , m_xFtArgName(m_xBuilder->weld_label(u"parname"_ustr))
    , m_xFtArgDesc(m_xBuilder->weld_label(u"pardesc"_ustr))
    , m_xBtnFx1(m_xBuilder->weld_button(u"FX1"_ustr))
    , m_xBtnFx2(m_xBuilder->weld_button(u"FX2"_ustr))
    , m_xBtnFx3(m_xBuilder->weld_button(u"FX3"_ustr))
    , m_xBtnFx4(m_xBuilder->weld_button(u"FX4"_ustr))
    , m_xFtArg1(m_xBuilder->weld_label(u"FT_ARG1"_ustr))
    , m_xFtArg2(m_xBuilder->weld_label(u"FT_ARG2"_ustr))
    , m_xFtArg3(m_xBuilder->weld_label(u"FT_ARG3"_ustr))
    , m_xFtArg4(m_xBuilder->weld_label(u"FT_ARG4"_ustr))
    , m_xEdArg1(new ArgEdit(m_xBuilder->weld_entry(u"ED_ARG1"_ustr)))
    , m_xEdArg2(new ArgEdit(m_xBuilder->weld_entry(u"ED_ARG2"_ustr)))
    , m_xEdArg3(new ArgEdit(m_xBuilder->weld_entry(u"ED_ARG3"_ustr)))
    , m_xEdArg4(new ArgEdit(m_xBuilder->weld_entry(u"ED_ARG4"_ustr)))
    , m_xRefBtn1(new RefButton(m_xBuilder->weld_button(u"RB_ARG1"_ustr)))
    , m_xRefBtn2(new RefButton(m_xBuilder->weld_button(u"RB_ARG2"_ustr)))
    , m_xRefBtn3(new RefButton(m_xBuilder->weld_button(u"RB_ARG3"_ustr)))
    , m_xRefBtn4(new RefButton(m_xBuilder->weld_button(u"RB_ARG4"_ustr)))
{
    // Space for three lines of text in function description.
    m_xFtEditDesc->set_label(u"X\nX\nX\n"_ustr);
    auto nEditHeight = m_xFtEditDesc->get_preferred_size().Height();
    m_xFtEditDesc->set_size_request(-1, nEditHeight);
    m_xFtEditDesc->set_label(u""_ustr);
    // Space for two lines of text in parameter description.
    m_xFtArgDesc->set_label(u"X\nX\n"_ustr);
    auto nArgHeight = m_xFtArgDesc->get_preferred_size().Height();
    m_xFtArgDesc->set_size_request(-1, nArgHeight);
    m_xFtArgDesc->set_label(u""_ustr);

    m_xBtnFx1->set_from_icon_name(BMP_FX);
    m_xBtnFx2->set_from_icon_name(BMP_FX);
    m_xBtnFx3->set_from_icon_name(BMP_FX);
    m_xBtnFx4->set_from_icon_name(BMP_FX);

    //lock down initial preferences
    m_xParamGrid->set_size_request(-1, m_xParamGrid->get_preferred_size().Height());
    Size aSize(m_xContainer->get_preferred_size());
    m_xContainer->set_size_request(aSize.Width(), aSize.Height());

    m_aDefaultString = m_xFtEditDesc->get_label();
    m_nEdFocus = NOT_FOUND;
    m_nActiveLine = 0;

    m_xSlider->connect_vadjustment_value_changed(LINK(this, ParaWin, ScrollHdl));

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

    if (m_nMaxArgs > 4)
        nArg = sal::static_int_cast<sal_uInt16>( nArg + GetSliderPos() );

    if ((m_nMaxArgs <= 0) || (nArg >= m_nMaxArgs))
        return;

    OUString  aArgDesc;
    OUString  aArgName;

    SetArgumentDesc( OUString() );
    SetArgumentText( OUString() );

    if (m_nArgs < VAR_ARGS)
    {
        sal_uInt16 nRealArg
            = (nArg < m_aVisibleArgMapping.size()) ? m_aVisibleArgMapping[nArg] : nArg;
        aArgDesc = m_pFuncDesc->getParameterDescription(nRealArg);
        aArgName = m_pFuncDesc->getParameterName(nRealArg) + " "
                   + ((m_pFuncDesc->isParameterOptional(nRealArg)) ? m_sOptional : m_sRequired);
    }
    else if (m_nArgs < PAIRED_VAR_ARGS)
    {
        sal_uInt16 nFix = m_nArgs - VAR_ARGS;
        sal_uInt16 nPos = std::min( nArg, nFix );
        sal_uInt16 nRealArg = (nPos < m_aVisibleArgMapping.size() ? m_aVisibleArgMapping[nPos]
                                                                  : m_aVisibleArgMapping.back());
        aArgDesc = m_pFuncDesc->getParameterDescription(nRealArg);
        aArgName = m_pFuncDesc->getParameterName(nRealArg);
        sal_uInt16 nVarArgsStart = m_pFuncDesc->getVarArgsStart();
        if ( nArg >= nVarArgsStart )
        {
            OUString aFuncName = m_pFuncDesc->getFunctionName();
            sal_Int16 nShifted = (aFuncName.equalsIgnoreAsciiCase(u"LAMBDA") ||
                aFuncName.equalsIgnoreAsciiCase(u"MAP")) ? nPos : 0;
            aArgName += OUString::number( nArg-nVarArgsStart + 1 + nShifted );
        }
        aArgName += " "
                    + ((nArg > nFix || m_pFuncDesc->isParameterOptional(nRealArg)) ? m_sOptional
                                                                                   : m_sRequired);
    }
    else
    {
        sal_uInt16 nFix = m_nArgs - PAIRED_VAR_ARGS;
        sal_uInt16 nPos;
        if ( nArg < nFix )
            nPos = nArg;
        else
            nPos = nFix + ( (nArg-nFix) % 2);
        sal_uInt16 nRealArg = (nPos < m_aVisibleArgMapping.size() ? m_aVisibleArgMapping[nPos]
                                                                  : m_aVisibleArgMapping.back());
        aArgDesc = m_pFuncDesc->getParameterDescription(nRealArg);
        aArgName = m_pFuncDesc->getParameterName(nRealArg);
        sal_uInt16 nVarArgsStart = m_pFuncDesc->getVarArgsStart();
        if ( nArg >= nVarArgsStart )
        {
            sal_Int16 nShifted
                = m_pFuncDesc->getFunctionName().equalsIgnoreAsciiCase(u"LET") ? nPos / 2 : 0;
            aArgName += OUString::number( (nArg-nVarArgsStart)/2 + 1 + nShifted );
        }
        aArgName
            += " "
               + ((nArg > (nFix + 1) || m_pFuncDesc->isParameterOptional(nRealArg)) ? m_sOptional
                                                                                    : m_sRequired);
    }

    SetArgumentDesc(aArgDesc);
    SetArgumentText(aArgName);
}

void ParaWin::UpdateArgInput( sal_uInt16 nOffset, sal_uInt16 i )
{
    sal_uInt16 nArg = nOffset + i;
    if (nArg > kMaxArgOffset)
        return;

    if (m_nArgs < VAR_ARGS)
    {
        if (nArg < m_nMaxArgs)
        {
            sal_uInt16 nRealArg = m_aVisibleArgMapping[nArg];
            SetArgNameFont(i,
                           (m_pFuncDesc->isParameterOptional(nRealArg)) ? m_aFntLight : m_aFntBold);
            SetArgName(i, m_pFuncDesc->getParameterName(nRealArg));
        }
    }
    else if (m_nArgs < PAIRED_VAR_ARGS)
    {
        sal_uInt16 nFix = m_nArgs - VAR_ARGS;
        sal_uInt16 nPos = std::min( nArg, nFix );
        sal_uInt16 nRealArg = (nPos < m_aVisibleArgMapping.size() ? m_aVisibleArgMapping[nPos]
                                                                  : m_aVisibleArgMapping.back());
        SetArgNameFont(i, (nArg > nFix || m_pFuncDesc->isParameterOptional(nRealArg)) ? m_aFntLight
                                                                                      : m_aFntBold);
        sal_uInt16 nVarArgsStart = m_pFuncDesc->getVarArgsStart();
        if ( nArg >= nVarArgsStart )
        {
            OUString aFuncName = m_pFuncDesc->getFunctionName();
            sal_Int16 nShifted = (aFuncName.equalsIgnoreAsciiCase(u"LAMBDA") ||
                aFuncName.equalsIgnoreAsciiCase(u"MAP")) ? nPos : 0;
            OUString aArgName = m_pFuncDesc->getParameterName(nRealArg)
                                + OUString::number(nArg - nVarArgsStart + 1 + nShifted);
            SetArgName( i, aArgName );
        }
        else
            SetArgName(i, m_pFuncDesc->getParameterName(nRealArg));
    }
    else
    {
        sal_uInt16 nFix = m_nArgs - PAIRED_VAR_ARGS;
        sal_uInt16 nPos;
        if ( nArg < nFix )
            nPos = nArg;
        else
            nPos = nFix + ( (nArg-nFix) % 2);
        sal_uInt16 nRealArg = (nPos < m_aVisibleArgMapping.size() ? m_aVisibleArgMapping[nPos]
                                                                  : m_aVisibleArgMapping.back());
        SetArgNameFont(i, (nArg > (nFix + 1) || m_pFuncDesc->isParameterOptional(nRealArg))
                              ? m_aFntLight
                              : m_aFntBold);
        sal_uInt16 nVarArgsStart = m_pFuncDesc->getVarArgsStart();
        if ( nArg >= nVarArgsStart )
        {
            sal_Int16 nShifted
                = m_pFuncDesc->getFunctionName().equalsIgnoreAsciiCase(u"LET") ? nPos / 2 : 0;
            OUString aArgName = m_pFuncDesc->getParameterName(nRealArg)
                                + OUString::number((nArg - nVarArgsStart) / 2 + 1 + nShifted);
            SetArgName( i, aArgName );
        }
        else
            SetArgName(i, m_pFuncDesc->getParameterName(nRealArg));
    }
    if (nArg < m_nMaxArgs)
        m_aArgInput[i].SetArgVal(m_aParaArray[nArg]);
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
    if (no >= m_nMaxArgs)
        return;

    tools::Long nOffset = GetSliderPos();
    m_nActiveLine = no;
    tools::Long nNewEdPos = static_cast<tools::Long>(m_nActiveLine) - nOffset;
    if(nNewEdPos<0 || nNewEdPos>3)
    {
        nOffset+=nNewEdPos;
        SetSliderPos(static_cast<sal_uInt16>(nOffset));
        nOffset=GetSliderPos();
    }
    m_nEdFocus = no - static_cast<sal_uInt16>(nOffset);
    UpdateArgDesc(m_nEdFocus);
}

RefEdit* ParaWin::GetActiveEdit()
{
    if (m_nMaxArgs > 0 && m_nEdFocus != NOT_FOUND)
    {
        return m_aArgInput[m_nEdFocus].GetArgEdPtr();
    }
    else
    {
        return nullptr;
    }
}


OUString ParaWin::GetArgument(sal_uInt16 no)
{
    OUString aStr;
    if (no < m_aParaArray.size())
    {
        aStr = m_aParaArray[no];
        if (no == m_nActiveLine && aStr.isEmpty())
            aStr += " ";
    }
    return aStr;
}

OUString  ParaWin::GetActiveArgName() const
{
    OUString aStr;
    if (m_nMaxArgs > 0 && m_nEdFocus != NOT_FOUND)
    {
        aStr = m_aArgInput[m_nEdFocus].GetArgName();
    }
    return aStr;
}


void ParaWin::SetArgument(sal_uInt16 no, std::u16string_view aString)
{
    if (no < m_aParaArray.size())
        m_aParaArray[no] = comphelper::string::stripStart(aString, ' ');
}

void ParaWin::SetArgumentFonts(const vcl::Font&aBoldFont,const vcl::Font&aLightFont)
{
    m_aFntBold = aBoldFont;
    m_aFntLight = aLightFont;
}

void ParaWin::SetFunctionDesc(const IFunctionDescription* pFDesc)
{
    m_pFuncDesc = pFDesc;

    SetArgumentDesc( OUString() );
    SetArgumentText( OUString() );
    SetEditDesc( OUString() );
    m_nMaxArgs = m_nArgs = 0;
    if (m_pFuncDesc != nullptr)
    {
        if (!m_pFuncDesc->getDescription().isEmpty())
        {
            SetEditDesc(m_pFuncDesc->getDescription());
        }
        else
        {
            SetEditDesc(m_aDefaultString);
        }
        m_nArgs = m_pFuncDesc->getSuppressedArgumentCount();
        m_nMaxArgs = std::min(m_nArgs, kMaxArgCount);
        if (sal_uInt16 nVarArgsLimit = m_pFuncDesc->getVarArgsLimit())
            m_nMaxArgs = std::min(nVarArgsLimit, m_nMaxArgs);
        m_pFuncDesc->fillVisibleArgumentMapping(m_aVisibleArgMapping);
        m_xSlider->set_vpolicy(VclPolicyType::NEVER);
        m_xSlider->set_size_request(-1, -1);
        OUString sHelpId = m_pFuncDesc->getHelpId();
        m_xContainer->set_help_id(sHelpId);
        m_xEdArg1->GetWidget()->set_help_id(sHelpId);
        m_xEdArg2->GetWidget()->set_help_id(sHelpId);
        m_xEdArg3->GetWidget()->set_help_id(sHelpId);
        m_xEdArg4->GetWidget()->set_help_id(sHelpId);

        SetActiveLine(0);
    }
    else
    {
        m_nActiveLine = 0;
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
    m_aArgInput[no].SetArgName(aText);
    m_aArgInput[no].UpdateAccessibleNames();
}

void ParaWin::SetArgNameFont(sal_uInt16 no,const vcl::Font& aFont)
{
    m_aArgInput[no].SetArgNameFont(aFont);
}

void ParaWin::SetEdFocus()
{
    UpdateArgDesc(0);
    if (!m_aParaArray.empty())
        m_aArgInput[0].GetArgEdPtr()->GrabFocus();
}

void ParaWin::InitArgInput(sal_uInt16 nPos, weld::Label& rFtArg, weld::Button& rBtnFx,
                           ArgEdit& rEdArg, RefButton& rRefBtn)
{
    rRefBtn.SetReferences(&m_rMyParent, &rEdArg);
    rEdArg.SetReferences(&m_rMyParent, &rFtArg);

    m_aArgInput[nPos].InitArgInput(rFtArg, rBtnFx, rEdArg, rRefBtn);

    m_aArgInput[nPos].Hide();

    m_aArgInput[nPos].SetFxClickHdl(LINK(this, ParaWin, GetFxHdl));
    m_aArgInput[nPos].SetFxFocusHdl(LINK(this, ParaWin, GetFxFocusHdl));
    m_aArgInput[nPos].SetEdFocusHdl(LINK(this, ParaWin, GetEdFocusHdl));
    m_aArgInput[nPos].SetEdModifyHdl(LINK(this, ParaWin, ModifyHdl));
    m_aArgInput[nPos].UpdateAccessibleNames();
}

void ParaWin::ClearAll()
{
    SetFunctionDesc(nullptr);
    SetArgumentOffset(0);
}

void ParaWin::SetArgumentOffset(sal_uInt16 nOffset)
{
    m_aParaArray.clear();
    m_xSlider->vadjustment_set_value(0);

    m_aParaArray.resize(m_nMaxArgs);

    if (m_nMaxArgs > 0)
    {
        for (int i = 0; i < 4 && i < m_nMaxArgs; i++)
        {
            m_aArgInput[i].SetArgVal(OUString());
            m_aArgInput[i].GetArgEdPtr()->Init(
                (i == 0) ? nullptr : m_aArgInput[i - 1].GetArgEdPtr(),
                (i == 3 || i == m_nMaxArgs - 1) ? nullptr : m_aArgInput[i + 1].GetArgEdPtr(),
                *m_xSlider, *this, m_nMaxArgs);
        }
    }

    UpdateParas();

    if (m_nMaxArgs < 5)
    {
        m_xSlider->set_vpolicy(VclPolicyType::NEVER);
        m_xSlider->set_size_request(-1, -1);
    }
    else
    {
        m_xSlider->vadjustment_configure(nOffset, m_nMaxArgs, 1, 4, 4);
        m_xSlider->set_vpolicy(VclPolicyType::ALWAYS);
        Size aPrefSize(m_xGrid->get_preferred_size());
        m_xSlider->set_size_request(aPrefSize.Width(), aPrefSize.Height());
    }
}

void ParaWin::UpdateParas()
{
    sal_uInt16 i;
    sal_uInt16 nOffset = GetSliderPos();

    if (m_nMaxArgs > 0)
    {
        for (i = 0; (i < m_nMaxArgs) && (i < 4); i++)
        {
            UpdateArgInput( nOffset, i );
            m_aArgInput[i].Show();
        }
    }

    for (i = m_nMaxArgs; i < 4; i++)
        m_aArgInput[i].Hide();
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
    if (m_nEdFocus != NOT_FOUND)
    {
        UpdateArgDesc(m_nEdFocus);
        m_aArgInput[m_nEdFocus].SelectAll();
        m_nActiveLine = m_nEdFocus + nOffset;
        ArgumentModified();
        m_aArgInput[m_nEdFocus].SelectAll(); // ensure all is still selected
        m_aArgInput[m_nEdFocus].UpdateAccessibleNames();
    }
}

void ParaWin::ArgumentModified() { m_aArgModifiedLink.Call(*this); }

IMPL_LINK( ParaWin, GetFxHdl, ArgInput&, rPtr, void )
{
    sal_uInt16 nOffset = GetSliderPos();
    m_nEdFocus = NOT_FOUND;
    for (size_t nPos = 0; nPos < std::size(m_aArgInput); ++nPos)
    {
        if (&rPtr == &m_aArgInput[nPos])
        {
            m_nEdFocus = nPos;
            break;
        }
    }

    if (m_nEdFocus != NOT_FOUND)
    {
        m_aArgInput[m_nEdFocus].SelectAll();
        m_nActiveLine = m_nEdFocus + nOffset;
        m_aFxLink.Call(*this);
    }
}

IMPL_LINK( ParaWin, GetFxFocusHdl, ArgInput&, rPtr, void )
{
    sal_uInt16 nOffset = GetSliderPos();
    m_nEdFocus = NOT_FOUND;
    for (size_t nPos = 0; nPos < std::size(m_aArgInput); ++nPos)
    {
        if (&rPtr == &m_aArgInput[nPos])
        {
            m_nEdFocus = nPos;
            break;
        }
    }

    if (m_nEdFocus != NOT_FOUND)
    {
        m_aArgInput[m_nEdFocus].SelectAll();
        UpdateArgDesc(m_nEdFocus);
        m_nActiveLine = m_nEdFocus + nOffset;
    }
}

IMPL_LINK( ParaWin, GetEdFocusHdl, ArgInput&, rPtr, void )
{
    sal_uInt16 nOffset = GetSliderPos();
    m_nEdFocus = NOT_FOUND;
    for (size_t nPos = 0; nPos < std::size(m_aArgInput); ++nPos)
    {
        if (&rPtr == &m_aArgInput[nPos])
        {
            m_nEdFocus = nPos;
            break;
        }
    }

    if (m_nEdFocus != NOT_FOUND)
    {
        m_aArgInput[m_nEdFocus].SelectAll();
        UpdateArgDesc(m_nEdFocus);
        m_nActiveLine = m_nEdFocus + nOffset;
        ArgumentModified();
        m_aArgInput[m_nEdFocus].SelectAll(); // ensure all is still selected
        m_aArgInput[m_nEdFocus].UpdateAccessibleNames();
    }
}

IMPL_LINK_NOARG(ParaWin, ScrollHdl, weld::ScrolledWindow&, void)
{
    SliderMoved();
}

IMPL_LINK( ParaWin, ModifyHdl, ArgInput&, rPtr, void )
{
    sal_uInt16 nOffset = GetSliderPos();
    m_nEdFocus = NOT_FOUND;
    for (size_t nPos = 0; nPos < std::size(m_aArgInput); ++nPos)
    {
        if (&rPtr == &m_aArgInput[nPos])
        {
            m_nEdFocus = nPos;
            break;
        }
    }
    if (m_nEdFocus != NOT_FOUND)
    {
        size_t nPara = m_nEdFocus + nOffset;
        if (nPara < m_aParaArray.size())
            m_aParaArray[nPara] = m_aArgInput[m_nEdFocus].GetArgVal();
        else
        {
            SAL_WARN("formula.ui", "ParaWin::ModifyHdl - shot in foot: nPara "
                                       << nPara << " >= aParaArray.size() " << m_aParaArray.size()
                                       << " with nEdFocus " << m_nEdFocus
                                       << " and aArgInput[nEdFocus].GetArgVal() '"
                                       << m_aArgInput[m_nEdFocus].GetArgVal() << "'");
        }
        UpdateArgDesc(m_nEdFocus);
        m_nActiveLine = static_cast<sal_uInt16>(nPara);
    }

    ArgumentModified();
}


} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
