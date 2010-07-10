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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_formula.hxx"



//----------------------------------------------------------------------------

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

    aFtArg1         ( this, ModuleRes( FT_ARG1 ) ),
    aFtArg2         ( this, ModuleRes( FT_ARG2 ) ),
    aFtArg3         ( this, ModuleRes( FT_ARG3 ) ),
    aFtArg4         ( this, ModuleRes( FT_ARG4 ) ),

    aBtnFx1         ( this, ModuleRes( BTN_FX1 ) ),
    aBtnFx2         ( this, ModuleRes( BTN_FX2 ) ),
    aBtnFx3         ( this, ModuleRes( BTN_FX3 ) ),
    aBtnFx4         ( this, ModuleRes( BTN_FX4 ) ),

    aEdArg1         ( this, ModuleRes( ED_ARG1 ) ),
    aEdArg2         ( this, ModuleRes( ED_ARG2 ) ),
    aEdArg3         ( this, ModuleRes( ED_ARG3 ) ),
    aEdArg4         ( this, ModuleRes( ED_ARG4 ) ),

    aRefBtn1        ( this, ModuleRes( RB_ARG1 ) ),
    aRefBtn2        ( this, ModuleRes( RB_ARG2 ) ),
    aRefBtn3        ( this, ModuleRes( RB_ARG3 ) ),
    aRefBtn4        ( this, ModuleRes( RB_ARG4 ) ),

    aSlider         ( this, ModuleRes( WND_SLIDER ) ),
    m_sOptional     ( ModuleRes( STR_OPTIONAL ) ),
    m_sRequired     ( ModuleRes( STR_REQUIRED ) ),
    bRefMode        (FALSE)
{
    Image aFxHC( ModuleRes( IMG_FX_H ) );
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

    aBtnFx1.SetModeImage( aFxHC, BMP_COLOR_HIGHCONTRAST );
    aBtnFx2.SetModeImage( aFxHC, BMP_COLOR_HIGHCONTRAST );
    aBtnFx3.SetModeImage( aFxHC, BMP_COLOR_HIGHCONTRAST );
    aBtnFx4.SetModeImage( aFxHC, BMP_COLOR_HIGHCONTRAST );

    InitArgInput( 0, aFtArg1, aBtnFx1, aEdArg1, aRefBtn1);
    InitArgInput( 1, aFtArg2, aBtnFx2, aEdArg2, aRefBtn2);
    InitArgInput( 2, aFtArg3, aBtnFx3, aEdArg3, aRefBtn3);
    InitArgInput( 3, aFtArg4, aBtnFx4, aEdArg4, aRefBtn4);
    ClearAll();
}

void ParaWin::UpdateArgDesc( USHORT nArg )
{
    if (nArg==NOT_FOUND) return;

    if ( nArgs > 4 )
        nArg = sal::static_int_cast<USHORT>( nArg + GetSliderPos() );
        //@ nArg += (USHORT)aSlider.GetThumbPos();

    if ( (nArgs > 0) && (nArg<nArgs) )
    {
        String  aArgDesc;
        String  aArgName;

        SetArgumentDesc( String() );
        SetArgumentText( String() );

        if ( nArgs < VAR_ARGS )
        {
            USHORT nRealArg = (aVisibleArgMapping.size() < nArg) ? aVisibleArgMapping[nArg] : nArg;
            aArgDesc  = pFuncDesc->getParameterDescription(nRealArg);
            aArgName  = pFuncDesc->getParameterName(nRealArg);
            aArgName += ' ';
            aArgName += (pFuncDesc->isParameterOptional(nRealArg)) ? m_sOptional : m_sRequired ;
        }
        else
        {
            USHORT nFix = nArgs - VAR_ARGS;
            USHORT nPos = ( nArg < nFix ? nArg : nFix );
            USHORT nRealArg = (nPos < aVisibleArgMapping.size() ?
                    aVisibleArgMapping[nPos] : aVisibleArgMapping.back());
            aArgDesc  = pFuncDesc->getParameterDescription(nRealArg);
            aArgName  = pFuncDesc->getParameterName(nRealArg);
            if ( nArg >= nFix )
                aArgName += String::CreateFromInt32(nArg-nFix+1);
            aArgName += ' ';

            aArgName += (nArg > nFix || pFuncDesc->isParameterOptional(nRealArg)) ? m_sOptional : m_sRequired ;
        }

        SetArgumentDesc(aArgDesc);
        SetArgumentText(aArgName);
    }
}

void ParaWin::UpdateArgInput( USHORT nOffset, USHORT i )
{
    USHORT nArg = nOffset + i;
    if ( nArgs < VAR_ARGS)
    {
        if(nArg<nArgs)
        {
            USHORT nRealArg = aVisibleArgMapping[nArg];
            SetArgNameFont  (i,(pFuncDesc->isParameterOptional(nRealArg))
                                            ? aFntLight : aFntBold );
            SetArgName      (i,pFuncDesc->getParameterName(nRealArg));
        }
    }
    else
    {
        USHORT nFix = nArgs - VAR_ARGS;
        USHORT nPos = ( nArg < nFix ? nArg : nFix );
        USHORT nRealArg = (nPos < aVisibleArgMapping.size() ?
                aVisibleArgMapping[nPos] : aVisibleArgMapping.back());
        SetArgNameFont( i,
                (nArg > nFix || pFuncDesc->isParameterOptional(nRealArg)) ?
                aFntLight : aFntBold );
        if ( nArg >= nFix )
        {
            String aArgName( pFuncDesc->getParameterName(nRealArg) );
            aArgName += String::CreateFromInt32(nArg-nFix+1);
            SetArgName( i, aArgName );
        }
        else
            SetArgName( i, pFuncDesc->getParameterName(nRealArg) );
    }
    if(nArg<nArgs) SetArgVal(i,aParaArray[nArg]);
    //@ aArgInput[i].SetArgVal( *(pArgArr[nOffset+i]) );

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

USHORT ParaWin::GetActiveLine()
{
    return nActiveLine;
}

void ParaWin::SetActiveLine(USHORT no)
{
    if(no<nArgs)
    {
        long nOffset = GetSliderPos();
        nActiveLine=no;
        long nNewEdPos=(long)nActiveLine-nOffset;
        if(nNewEdPos<0 || nNewEdPos>3)
        {
            nOffset+=nNewEdPos;
            SetSliderPos((USHORT) nOffset);
            nOffset=GetSliderPos();
        }
        nEdFocus=no-(USHORT)nOffset;
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


String ParaWin::GetArgument(USHORT no)
{
    String aStr;
    if(no<aParaArray.size())
    {
        aStr=aParaArray[no];
        if(no==nActiveLine && aStr.Len()==0)
            aStr+=' ';
    }
    return aStr;
}

String  ParaWin::GetActiveArgName()
{
    String aStr;
    if(nArgs>0 && nEdFocus!=NOT_FOUND)
    {
        aStr=aArgInput[nEdFocus].GetArgName();
    }
    return aStr;
}


void ParaWin::SetArgument(USHORT no, const String& aString)
{
    if(no<aParaArray.size())
    {
        aParaArray[no] = aString;
        aParaArray[no].EraseLeadingChars();
    }
}

void ParaWin::DelParaArray()
{
    ::std::vector<String>().swap(aParaArray);
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
        if ( pFuncDesc->getDescription().getLength() )
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
        rtl::OString sHelpId = pFuncDesc->getHelpId();
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

void ParaWin::SetArgumentText(const String& aText)
{
    aFtArgName.SetText(aText);
}

void ParaWin::SetArgumentDesc(const String& aText)
{
    aFtArgDesc.SetText(aText);
}

void ParaWin::SetEditDesc(const String& aText)
{
    aFtEditDesc.SetText(aText);
}

void ParaWin::SetArgName(USHORT no,const String& aText)
{
    aArgInput[no].SetArgName(aText);
}

void ParaWin::SetArgNameFont(USHORT no,const Font& aFont)
{
    aArgInput[no].SetArgNameFont(aFont);
}

void ParaWin::SetArgVal(USHORT no,const String& aText)
{
    aArgInput[no].SetArgVal(aText);
}

void ParaWin::HideParaLine(USHORT no)
{
    aArgInput[no].Hide();
}

void ParaWin::ShowParaLine(USHORT no)
{
    aArgInput[no].Show();
}

void ParaWin::SetEdFocus(USHORT no)
{
    UpdateArgDesc(no);
    if(no<4 && no<aParaArray.size())
        aArgInput[no].GetArgEdPtr()->GrabFocus();
}


void ParaWin::InitArgInput( USHORT nPos, FixedText& rFtArg, ImageButton& rBtnFx,
                        ArgEdit& rEdArg, RefButton& rRefBtn)
{

    rRefBtn.SetReferences(pMyParent,&rEdArg);
    rEdArg.SetRefDialog(pMyParent);

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

void ParaWin::SetArgumentOffset(USHORT nOffset)
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
        //aSlider.SetEndScrollHdl( LINK( this, ScFormulaDlg, ScrollHdl ) );
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
    USHORT i;
    USHORT nOffset = GetSliderPos();

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


USHORT ParaWin::GetSliderPos()
{
    return (USHORT) aSlider.GetThumbPos();
}

void ParaWin::SetSliderPos(USHORT nSliderPos)
{
    USHORT nOffset = GetSliderPos();

    if(aSlider.IsVisible() && nOffset!=nSliderPos)
    {
        aSlider.SetThumbPos(nSliderPos);
        for ( USHORT i=0; i<4; i++ )
        {
            UpdateArgInput( nSliderPos, i );
        }
    }
}

void ParaWin::SliderMoved()
{
    USHORT nOffset = GetSliderPos();

    for ( USHORT i=0; i<4; i++ )
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
    USHORT nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for ( USHORT nPos=0; nPos<5;nPos++)
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
    USHORT nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for ( USHORT nPos=0; nPos<5;nPos++)
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
    USHORT nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for ( USHORT nPos=0; nPos<5;nPos++)
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


IMPL_LINK( ParaWin, ScrollHdl, ScrollBar*, EMPTYARG )
{
    SliderMoved();

    return 0;
}

IMPL_LINK( ParaWin, ModifyHdl, ArgInput*, pPtr )
{
    USHORT nOffset = GetSliderPos();
    nEdFocus=NOT_FOUND;
    for ( USHORT nPos=0; nPos<5;nPos++)
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
