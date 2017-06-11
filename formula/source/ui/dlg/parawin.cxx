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
#include <svl/zforlist.hxx>
#include <svl/stritem.hxx>
#include <vcl/settings.hxx>

#include "parawin.hxx"
#include "formula/formdata.hxx"
#include "formula/IFunctionDescription.hxx"
#include <formula/funcvarargs.h>
#include "ModuleHelper.hxx"
#include "strings.hrc"
#include "bitmaps.hlst"

namespace formula
{


ParaWin::ParaWin(vcl::Window* pParent,IControlReferenceHandler* _pDlg):
    TabPage         (pParent, "ParameterPage", "formula/ui/parameter.ui"),
    pFuncDesc       ( nullptr ),
    pMyParent       (_pDlg),
    m_sOptional     ( ModuleRes( STR_OPTIONAL ) ),
    m_sRequired     ( ModuleRes( STR_REQUIRED ) )
{
    get(m_pFtEditDesc, "editdesc");
    get(m_pFtArgName, "parname");
    get(m_pFtArgDesc, "pardesc");

    // Space for three lines of text in function description.
    m_pFtEditDesc->SetText("X\nX\nX\n");
    long nEditHeight = m_pFtEditDesc->GetOptimalSize().Height();
    m_pFtEditDesc->set_height_request(nEditHeight);
    m_pFtEditDesc->SetText("");
    // Space for two lines of text in parameter description.
    m_pFtArgDesc->SetText("X\nX\n");
    long nArgHeight = m_pFtArgDesc->GetOptimalSize().Height();
    m_pFtArgDesc->set_height_request(nArgHeight);
    m_pFtArgDesc->SetText("");

    get(m_pBtnFx1, "FX1");
    m_pBtnFx1->SetModeImage(Image(BitmapEx(BMP_FX)));
    get(m_pBtnFx2, "FX2");
    m_pBtnFx2->SetModeImage(Image(BitmapEx(BMP_FX)));
    get(m_pBtnFx3, "FX3");
    m_pBtnFx3->SetModeImage(Image(BitmapEx(BMP_FX)));
    get(m_pBtnFx4, "FX4");
    m_pBtnFx4->SetModeImage(Image(BitmapEx(BMP_FX)));

    get(m_pFtArg1, "FT_ARG1");
    get(m_pFtArg2, "FT_ARG2");
    get(m_pFtArg3, "FT_ARG3");
    get(m_pFtArg4, "FT_ARG4");

    get(m_pEdArg1, "ED_ARG1");
    get(m_pEdArg2, "ED_ARG2");
    get(m_pEdArg3, "ED_ARG3");
    get(m_pEdArg4, "ED_ARG4");

    get(m_pRefBtn1, "RB_ARG1");
    get(m_pRefBtn2, "RB_ARG2");
    get(m_pRefBtn3, "RB_ARG3");
    get(m_pRefBtn4, "RB_ARG4");

    get(m_pSlider, "scrollbar");

    //lock down initial preferences
    vcl::Window *pGrid = get<vcl::Window>("paramgrid");
    pGrid->set_height_request(pGrid->get_preferred_size().Height());
    Size aSize(get_preferred_size());
    set_width_request(aSize.Width());
    set_height_request(aSize.Height());

    aDefaultString=m_pFtEditDesc->GetText();
    nEdFocus=NOT_FOUND;
    nActiveLine=0;

    m_pSlider->SetEndScrollHdl( LINK( this, ParaWin, ScrollHdl ) );
    m_pSlider->SetScrollHdl( LINK( this, ParaWin, ScrollHdl ) );

    InitArgInput( 0, *m_pFtArg1, *m_pBtnFx1, *m_pEdArg1, *m_pRefBtn1);
    InitArgInput( 1, *m_pFtArg2, *m_pBtnFx2, *m_pEdArg2, *m_pRefBtn2);
    InitArgInput( 2, *m_pFtArg3, *m_pBtnFx3, *m_pEdArg3, *m_pRefBtn3);
    InitArgInput( 3, *m_pFtArg4, *m_pBtnFx4, *m_pEdArg4, *m_pRefBtn4);
    ClearAll();
}

void ParaWin::UpdateArgDesc( sal_uInt16 nArg )
{
    if (nArg==NOT_FOUND) return;

    if ( nArgs > 4 )
        nArg = sal::static_int_cast<sal_uInt16>( nArg + GetSliderPos() );

    if ( (nArgs > 0) && (nArg<nArgs) )
    {
        OUString  aArgDesc;
        OUString  aArgName;

        SetArgumentDesc( OUString() );
        SetArgumentText( OUString() );

        if ( nArgs < VAR_ARGS )
        {
            sal_uInt16 nRealArg = (nArg < aVisibleArgMapping.size()) ? aVisibleArgMapping[nArg] : nArg;
            aArgDesc  = pFuncDesc->getParameterDescription(nRealArg);
            aArgName  = pFuncDesc->getParameterName(nRealArg);
            aArgName += " ";
            aArgName += (pFuncDesc->isParameterOptional(nRealArg)) ? m_sOptional : m_sRequired ;
        }
        else if ( nArgs < PAIRED_VAR_ARGS )
        {
            sal_uInt16 nFix = nArgs - VAR_ARGS;
            sal_uInt16 nPos = ( nArg < nFix ? nArg : nFix );
            sal_uInt16 nRealArg = (nPos < aVisibleArgMapping.size() ?
                    aVisibleArgMapping[nPos] : aVisibleArgMapping.back());
            aArgDesc  = pFuncDesc->getParameterDescription(nRealArg);
            aArgName  = pFuncDesc->getParameterName(nRealArg);
            sal_uInt16 nVarArgsStart = pFuncDesc->getVarArgsStart();
            if ( nArg >= nVarArgsStart )
                aArgName += OUString::number( nArg-nVarArgsStart+1 );
            aArgName += " ";

            aArgName += (nArg > nFix || pFuncDesc->isParameterOptional(nRealArg)) ? m_sOptional : m_sRequired ;
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
            aArgName += " ";

            aArgName += (nArg > (nFix+1) || pFuncDesc->isParameterOptional(nRealArg)) ? m_sOptional : m_sRequired ;
        }

        SetArgumentDesc(aArgDesc);
        SetArgumentText(aArgName);
    }
}

void ParaWin::UpdateArgInput( sal_uInt16 nOffset, sal_uInt16 i )
{
    sal_uInt16 nArg = nOffset + i;
    if ( nArgs < VAR_ARGS)
    {
        if(nArg<nArgs)
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
        sal_uInt16 nPos = ( nArg < nFix ? nArg : nFix );
        sal_uInt16 nRealArg = (nPos < aVisibleArgMapping.size() ?
                aVisibleArgMapping[nPos] : aVisibleArgMapping.back());
        SetArgNameFont( i,
                (nArg > nFix || pFuncDesc->isParameterOptional(nRealArg)) ?
                aFntLight : aFntBold );
        sal_uInt16 nVarArgsStart = pFuncDesc->getVarArgsStart();
        if ( nArg >= nVarArgsStart )
        {
            OUString aArgName( pFuncDesc->getParameterName(nRealArg) );
            aArgName += OUString::number(nArg-nVarArgsStart+1);
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
            OUString aArgName( pFuncDesc->getParameterName(nRealArg) );
            aArgName += OUString::number( (nArg-nVarArgsStart)/2 + 1 );
            SetArgName( i, aArgName );
        }
        else
            SetArgName( i, pFuncDesc->getParameterName(nRealArg) );
    }
    if (nArg<nArgs)
        aArgInput[i].SetArgVal(aParaArray[nArg]);
}

ParaWin::~ParaWin()
{
    disposeOnce();
}

void ParaWin::dispose()
{
    // #i66422# if the focus changes during destruction of the controls,
    // don't call the focus handlers
    Link<Control&,void> aEmptyLink;
    m_pBtnFx1->SetGetFocusHdl( aEmptyLink );
    m_pBtnFx2->SetGetFocusHdl( aEmptyLink );
    m_pBtnFx3->SetGetFocusHdl( aEmptyLink );
    m_pBtnFx4->SetGetFocusHdl( aEmptyLink );
    m_pFtEditDesc.clear();
    m_pFtArgName.clear();
    m_pFtArgDesc.clear();
    m_pBtnFx1.clear();
    m_pFtArg1.clear();
    m_pEdArg1.clear();
    m_pRefBtn1.clear();
    m_pBtnFx2.clear();
    m_pFtArg2.clear();
    m_pEdArg2.clear();
    m_pRefBtn2.clear();
    m_pBtnFx3.clear();
    m_pFtArg3.clear();
    m_pEdArg3.clear();
    m_pRefBtn3.clear();
    m_pBtnFx4.clear();
    m_pFtArg4.clear();
    m_pEdArg4.clear();
    m_pRefBtn4.clear();
    m_pSlider.clear();
    TabPage::dispose();
}


void ParaWin::SetActiveLine(sal_uInt16 no)
{
    if(no<nArgs)
    {
        long nOffset = GetSliderPos();
        nActiveLine=no;
        long nNewEdPos=(long)nActiveLine-nOffset;
        if(nNewEdPos<0 || nNewEdPos>3)
        {
            nOffset+=nNewEdPos;
            SetSliderPos((sal_uInt16) nOffset);
            nOffset=GetSliderPos();
        }
        nEdFocus=no-(sal_uInt16)nOffset;
        UpdateArgDesc( nEdFocus );
    }
}

RefEdit* ParaWin::GetActiveEdit()
{
    if(nArgs>0 && nEdFocus!=NOT_FOUND)
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

OUString  ParaWin::GetActiveArgName()
{
    OUString aStr;
    if(nArgs>0 && nEdFocus!=NOT_FOUND)
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
    nArgs = 0;
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
        pFuncDesc->fillVisibleArgumentMapping(aVisibleArgMapping);
        m_pSlider->Hide();
        OString sHelpId = pFuncDesc->getHelpId();
        SetHelpId( sHelpId );
        m_pEdArg1->SetHelpId( sHelpId );
        m_pEdArg2->SetHelpId( sHelpId );
        m_pEdArg3->SetHelpId( sHelpId );
        m_pEdArg4->SetHelpId( sHelpId );

        SetActiveLine(0);
    }
    else
    {
        nActiveLine=0;
    }

}

void ParaWin::SetArgumentText(const OUString& aText)
{
    m_pFtArgName->SetText(aText);
}

void ParaWin::SetArgumentDesc(const OUString& aText)
{
    m_pFtArgDesc->SetText(aText);
}

void ParaWin::SetEditDesc(const OUString& aText)
{
    m_pFtEditDesc->SetText(aText);
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
    if(0<aParaArray.size())
        aArgInput[0].GetArgEdPtr()->GrabFocus();
}


void ParaWin::InitArgInput( sal_uInt16 nPos, FixedText& rFtArg, PushButton& rBtnFx,
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
    m_pSlider->SetThumbPos(0);

    aParaArray.resize(nArgs);

    if ( nArgs > 0 )
    {
        for ( int i=0; i<4 && i<nArgs; i++ )
        {
            OUString aString;
            aArgInput[i].SetArgVal(aString);
            aArgInput[i].GetArgEdPtr()->Init(
                (i==0)               ? nullptr : aArgInput[i-1].GetArgEdPtr(),
                (i==3 || i==nArgs-1) ? nullptr : aArgInput[i+1].GetArgEdPtr(),
                                       *m_pSlider, nArgs );
        }
    }

    if ( nArgs < 5 )
    {
        m_pSlider->Hide();
    }
    else
    {
        m_pSlider->SetPageSize( 4 );
        m_pSlider->SetVisibleSize( 4 );
        m_pSlider->SetLineSize( 1 );
        m_pSlider->SetRange( Range( 0, nArgs ) );
        m_pSlider->SetThumbPos( nOffset );
        m_pSlider->Show();
    }

    UpdateParas();
}

void ParaWin::UpdateParas()
{
    sal_uInt16 i;
    sal_uInt16 nOffset = GetSliderPos();

    if ( nArgs > 0 )
    {
        for ( i=0; (i<nArgs) && (i<4); i++ )
        {
            UpdateArgInput( nOffset, i );
            aArgInput[i].Show();
        }
    }

    for ( i=nArgs; i<4; i++ )
        aArgInput[i].Hide();
}


sal_uInt16 ParaWin::GetSliderPos()
{
    return (sal_uInt16) m_pSlider->GetThumbPos();
}

void ParaWin::SetSliderPos(sal_uInt16 nSliderPos)
{
    sal_uInt16 nOffset = GetSliderPos();

    if(m_pSlider->IsVisible() && nOffset!=nSliderPos)
    {
        m_pSlider->SetThumbPos(nSliderPos);
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
        aArgInput[nEdFocus].SetArgSelection(Selection(0,SELECTION_MAX ));
        nActiveLine=nEdFocus+nOffset;
        ArgumentModified();
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
    for (sal_uInt16 nPos=0; nPos < SAL_N_ELEMENTS(aArgInput); ++nPos)
    {
        if(&rPtr == &aArgInput[nPos])
        {
            nEdFocus=nPos;
            break;
        }
    }

    if(nEdFocus!=NOT_FOUND)
    {
        aArgInput[nEdFocus].SetArgSelection(Selection(0,SELECTION_MAX ));
        nActiveLine=nEdFocus+nOffset;
        aFxLink.Call(*this);
    }
}

IMPL_LINK( ParaWin, GetFxFocusHdl, ArgInput&, rPtr, void )
{
    sal_uInt16 nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for (sal_uInt16 nPos=0; nPos < SAL_N_ELEMENTS(aArgInput); ++nPos)
    {
        if(&rPtr == &aArgInput[nPos])
        {
            nEdFocus=nPos;
            break;
        }
    }

    if(nEdFocus!=NOT_FOUND)
    {
        aArgInput[nEdFocus].SetArgSelection(Selection(0,SELECTION_MAX ));
        UpdateArgDesc( nEdFocus );
        nActiveLine=nEdFocus+nOffset;
    }
}


IMPL_LINK( ParaWin, GetEdFocusHdl, ArgInput&, rPtr, void )
{
    sal_uInt16 nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for (sal_uInt16 nPos=0; nPos < SAL_N_ELEMENTS(aArgInput); ++nPos)
    {
        if(&rPtr == &aArgInput[nPos])
        {
            nEdFocus=nPos;
            break;
        }
    }

    if(nEdFocus!=NOT_FOUND)
    {
        aArgInput[nEdFocus].SetArgSelection(Selection(0,SELECTION_MAX ));
        UpdateArgDesc( nEdFocus );
        nActiveLine=nEdFocus+nOffset;
        ArgumentModified();
        aArgInput[nEdFocus].UpdateAccessibleNames();
    }
}


IMPL_LINK_NOARG(ParaWin, ScrollHdl, ScrollBar*, void)
{
    SliderMoved();
}

IMPL_LINK( ParaWin, ModifyHdl, ArgInput&, rPtr, void )
{
    sal_uInt16 nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for (sal_uInt16 nPos=0; nPos < SAL_N_ELEMENTS(aArgInput); ++nPos)
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
