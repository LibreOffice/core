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

#include "parawin.hxx"
#include "parawin.hrc"
#include "helpids.hrc"
#include "formula/formdata.hxx"
#include "formula/IFunctionDescription.hxx"
#include "ModuleHelper.hxx"
#include "ForResId.hrc"

#define VAR_ARGS 30
#define PAIRED_VAR_ARGS (VAR_ARGS + VAR_ARGS)
namespace formula
{
//============================================================================

ParaWin::ParaWin(Window* pParent,IControlReferenceHandler* _pDlg,Point aPos):
    TabPage         (pParent,ModuleRes(RID_FORMULATAB_PARAMETER)),
    pFuncDesc       ( NULL ),
    pMyParent       (_pDlg),
    aFtEditDesc     ( this, ModuleRes( FT_EDITDESC ) ),
    aFtArgName      ( this, ModuleRes( FT_PARNAME ) ),
    aFtArgDesc      ( this, ModuleRes( FT_PARDESC ) ),

    aBtnFx1         ( this, ModuleRes( BTN_FX1 ) ),
    aFtArg1         ( this, ModuleRes( FT_ARG1 ) ),
    aEdArg1         ( this, ModuleRes( ED_ARG1 ) ),
    aRefBtn1        ( this, ModuleRes( RB_ARG1 ) ),

    aBtnFx2         ( this, ModuleRes( BTN_FX2 ) ),
    aFtArg2         ( this, ModuleRes( FT_ARG2 ) ),
    aEdArg2         ( this, ModuleRes( ED_ARG2 ) ),
    aRefBtn2        ( this, ModuleRes( RB_ARG2 ) ),

    aBtnFx3         ( this, ModuleRes( BTN_FX3 ) ),
    aFtArg3         ( this, ModuleRes( FT_ARG3 ) ),
    aEdArg3         ( this, ModuleRes( ED_ARG3 ) ),
    aRefBtn3        ( this, ModuleRes( RB_ARG3 ) ),

    aBtnFx4         ( this, ModuleRes( BTN_FX4 ) ),
    aFtArg4         ( this, ModuleRes( FT_ARG4 ) ),
    aEdArg4         ( this, ModuleRes( ED_ARG4 ) ),
    aRefBtn4        ( this, ModuleRes( RB_ARG4 ) ),

    aSlider         ( this, ModuleRes( WND_SLIDER ) ),
    m_sOptional     ( ModuleRes( STR_OPTIONAL ) ),
    m_sRequired     ( ModuleRes( STR_REQUIRED ) ),
    bRefMode        (sal_False)
{
    FreeResource();
    aDefaultString=aFtEditDesc.GetText();

    SetPosPixel(aPos);
    nEdFocus=NOT_FOUND;
    nActiveLine=0;
    Size aSize = aSlider.GetSizePixel();
    aSize.Width() = GetSettings().GetStyleSettings().GetScrollBarSize();
    aSlider.SetSizePixel( aSize );
    aSlider.SetEndScrollHdl( LINK( this, ParaWin, ScrollHdl ) );
    aSlider.SetScrollHdl( LINK( this, ParaWin, ScrollHdl ) );

    InitArgInput( 0, aFtArg1, aBtnFx1, aEdArg1, aRefBtn1);
    InitArgInput( 1, aFtArg2, aBtnFx2, aEdArg2, aRefBtn2);
    InitArgInput( 2, aFtArg3, aBtnFx3, aEdArg3, aRefBtn3);
    InitArgInput( 3, aFtArg4, aBtnFx4, aEdArg4, aRefBtn4);
    ClearAll();
}

void ParaWin::UpdateArgDesc( sal_uInt16 nArg )
{
    if (nArg==NOT_FOUND) return;

    if ( nArgs > 4 )
        nArg = sal::static_int_cast<sal_uInt16>( nArg + GetSliderPos() );

    if ( (nArgs > 0) && (nArg<nArgs) )
    {
        String  aArgDesc;
        String  aArgName;

        SetArgumentDesc( String() );
        SetArgumentText( String() );

        if ( nArgs < VAR_ARGS )
        {
            sal_uInt16 nRealArg = (aVisibleArgMapping.size() < nArg) ? aVisibleArgMapping[nArg] : nArg;
            aArgDesc  = pFuncDesc->getParameterDescription(nRealArg);
            aArgName  = pFuncDesc->getParameterName(nRealArg);
            aArgName += ' ';
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
            if ( nArg >= nFix )
                aArgName += OUString::number( nArg-nFix+1 );
            aArgName += ' ';

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
            if ( nArg >= nFix )
                aArgName += OUString::number( (nArg-nFix)/2 + 1 );
            aArgName += ' ';

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
        if ( nArg >= nFix )
        {
            String aArgName( pFuncDesc->getParameterName(nRealArg) );
            aArgName += OUString::number(nArg-nFix+1);
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
        if ( nArg >= nFix )
        {
            String aArgName( pFuncDesc->getParameterName(nRealArg) );
            aArgName += OUString::number( (nArg-nFix)/2 + 1 );
            SetArgName( i, aArgName );
        }
        else
            SetArgName( i, pFuncDesc->getParameterName(nRealArg) );
    }
    if(nArg<nArgs) SetArgVal(i,aParaArray[nArg]);

}

ParaWin::~ParaWin()
{
    // #i66422# if the focus changes during destruction of the controls,
    // don't call the focus handlers
    Link aEmptyLink;
    aBtnFx1.SetGetFocusHdl( aEmptyLink );
    aBtnFx2.SetGetFocusHdl( aEmptyLink );
    aBtnFx3.SetGetFocusHdl( aEmptyLink );
    aBtnFx4.SetGetFocusHdl( aEmptyLink );
}

sal_uInt16 ParaWin::GetActiveLine()
{
    return nActiveLine;
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
        return NULL;
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

void ParaWin::DelParaArray()
{
    ::std::vector<OUString>().swap(aParaArray);
}

void ParaWin::SetArgumentFonts(const Font&aBoldFont,const Font&aLightFont)
{
    aFntBold=aBoldFont;
    aFntLight=aLightFont;
}

void ParaWin::SetFunctionDesc(const IFunctionDescription* pFDesc)
{
    pFuncDesc=pFDesc;

    SetArgumentDesc( String() );
    SetArgumentText( String() );
    SetEditDesc( String() );
    nArgs = 0;
    if ( pFuncDesc!=NULL)
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
        aSlider.Hide();
        OString sHelpId = pFuncDesc->getHelpId();
        SetHelpId( sHelpId );
        aEdArg1.SetHelpId( sHelpId );
        aEdArg2.SetHelpId( sHelpId );
        aEdArg3.SetHelpId( sHelpId );
        aEdArg4.SetHelpId( sHelpId );

        //  Unique-IDs muessen gleich bleiben fuer Automatisierung
        SetUniqueId( HID_FORMULA_FAP_PAGE );
        aEdArg1.SetUniqueId( HID_FORMULA_FAP_EDIT1 );
        aEdArg2.SetUniqueId( HID_FORMULA_FAP_EDIT2 );
        aEdArg3.SetUniqueId( HID_FORMULA_FAP_EDIT3 );
        aEdArg4.SetUniqueId( HID_FORMULA_FAP_EDIT4 );
        SetActiveLine(0);
    }
    else
    {
        nActiveLine=0;
    }

}

void ParaWin::SetArgumentText(const OUString& aText)
{
    aFtArgName.SetText(aText);
}

void ParaWin::SetArgumentDesc(const OUString& aText)
{
    aFtArgDesc.SetText(aText);
}

void ParaWin::SetEditDesc(const OUString& aText)
{
    aFtEditDesc.SetText(aText);
}

void ParaWin::SetArgName(sal_uInt16 no,const OUString& aText)
{
    aArgInput[no].SetArgName(aText);
}

void ParaWin::SetArgNameFont(sal_uInt16 no,const Font& aFont)
{
    aArgInput[no].SetArgNameFont(aFont);
}

void ParaWin::SetArgVal(sal_uInt16 no,const OUString& aText)
{
    aArgInput[no].SetArgVal(aText);
}

void ParaWin::HideParaLine(sal_uInt16 no)
{
    aArgInput[no].Hide();
}

void ParaWin::ShowParaLine(sal_uInt16 no)
{
    aArgInput[no].Show();
}

void ParaWin::SetEdFocus(sal_uInt16 no)
{
    UpdateArgDesc(no);
    if(no<4 && no<aParaArray.size())
        aArgInput[no].GetArgEdPtr()->GrabFocus();
}


void ParaWin::InitArgInput( sal_uInt16 nPos, FixedText& rFtArg, ImageButton& rBtnFx,
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
}

void ParaWin::ClearAll()
{
    SetFunctionDesc(NULL);
    SetArgumentOffset(0);
}

void ParaWin::SetArgumentOffset(sal_uInt16 nOffset)
{
    DelParaArray();
    aSlider.SetThumbPos(0);

    aParaArray.resize(nArgs);

    if ( nArgs > 0 )
    {
        for ( int i=0; i<4 && i<nArgs; i++ )
        {
            String aString;
            aArgInput[i].SetArgVal(aString);
            aArgInput[i].GetArgEdPtr()->Init(
                (i==0)               ? (ArgEdit *)NULL : aArgInput[i-1].GetArgEdPtr(),
                (i==3 || i==nArgs-1) ? (ArgEdit *)NULL : aArgInput[i+1].GetArgEdPtr(),
                                       aSlider, nArgs );
        }
    }

    if ( nArgs < 5 )
    {
        aSlider.Hide();
    }
    else
    {
        aSlider.SetPageSize( 4 );
        aSlider.SetVisibleSize( 4 );
        aSlider.SetLineSize( 1 );
        aSlider.SetRange( Range( 0, nArgs ) );
        aSlider.SetThumbPos( nOffset );
        aSlider.Show();
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
            ShowParaLine(i);
        }
    }

    for ( i=nArgs; i<4; i++ ) HideParaLine(i);
}


sal_uInt16 ParaWin::GetSliderPos()
{
    return (sal_uInt16) aSlider.GetThumbPos();
}

void ParaWin::SetSliderPos(sal_uInt16 nSliderPos)
{
    sal_uInt16 nOffset = GetSliderPos();

    if(aSlider.IsVisible() && nOffset!=nSliderPos)
    {
        aSlider.SetThumbPos(nSliderPos);
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
    }
    aScrollLink.Call(this);
}

void ParaWin::ArgumentModified()
{
    aArgModifiedLink.Call(this);
}

void ParaWin::FxClick()
{
    aFxLink.Call(this);
}


IMPL_LINK( ParaWin, GetFxHdl, ArgInput*, pPtr )
{
    sal_uInt16 nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for ( sal_uInt16 nPos=0; nPos<5;nPos++)
    {
        if(pPtr == &aArgInput[nPos])
        {
            nEdFocus=nPos;
            break;
        }
    }

    if(nEdFocus!=NOT_FOUND)
    {
        aArgInput[nEdFocus].SetArgSelection(Selection(0,SELECTION_MAX ));
        nActiveLine=nEdFocus+nOffset;
        FxClick();
    }
    return 0;
}

IMPL_LINK( ParaWin, GetFxFocusHdl, ArgInput*, pPtr )
{
    sal_uInt16 nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for ( sal_uInt16 nPos=0; nPos<5;nPos++)
    {
        if(pPtr == &aArgInput[nPos])
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
    return 0;
}



IMPL_LINK( ParaWin, GetEdFocusHdl, ArgInput*, pPtr )
{
    sal_uInt16 nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for ( sal_uInt16 nPos=0; nPos<5;nPos++)
    {
        if(pPtr == &aArgInput[nPos])
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
    }

    return 0;
}


IMPL_LINK_NOARG(ParaWin, ScrollHdl)
{
    SliderMoved();

    return 0;
}

IMPL_LINK( ParaWin, ModifyHdl, ArgInput*, pPtr )
{
    sal_uInt16 nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for ( sal_uInt16 nPos=0; nPos<5;nPos++)
    {
        if(pPtr == &aArgInput[nPos])
        {
            nEdFocus=nPos;
            break;
        }
    }
    if(nEdFocus!=NOT_FOUND)
    {
        aParaArray[nEdFocus+nOffset] = aArgInput[nEdFocus].GetArgVal();
        UpdateArgDesc( nEdFocus);
        nActiveLine=nEdFocus+nOffset;
    }

    ArgumentModified();
    return 0;
}



} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
