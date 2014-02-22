/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>

#include "reffact.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "rangenam.hxx"
#include "simpref.hxx"
#include "scmod.hxx"



#define ABS_SREF          SCA_VALID \
                        | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF          ABS_SREF \
                        | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

ScSimpleRefDlg::ScSimpleRefDlg(SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
    ScViewData* ptrViewData)
    : ScAnyRefDlg(pB, pCW, pParent, "SimpleRefDialog", "modules/scalc/ui/simplerefdialog.ui")
,

        pDoc            ( ptrViewData->GetDocument() ),
        bAutoReOpen     ( true ),
        bCloseOnButtonUp( false ),
        bSingleCell     ( false ),
        bMultiSelection ( false )
{
    get(m_pFtAssign, "area");
    get(m_pEdAssign, "assign");
    m_pEdAssign->SetReferences(this, m_pFtAssign);
    get(m_pRbAssign, "assignref");
    m_pRbAssign->SetReferences(this, m_pEdAssign);

    get(m_pBtnOk, "ok");
    get(m_pBtnCancel, "cancel");

    
    Init();
    SetDispatcherLock( true ); 
}


ScSimpleRefDlg::~ScSimpleRefDlg()
{
    SetDispatcherLock( false ); 
}


void ScSimpleRefDlg::FillInfo(SfxChildWinInfo& rWinInfo) const
{
    ScAnyRefDlg::FillInfo(rWinInfo);
    rWinInfo.bVisible=bAutoReOpen;
}


void ScSimpleRefDlg::SetRefString(const OUString &rStr)
{
    m_pEdAssign->SetText(rStr);
}


void ScSimpleRefDlg::Init()
{
    m_pBtnOk->SetClickHdl      ( LINK( this, ScSimpleRefDlg, OkBtnHdl ) );
    m_pBtnCancel->SetClickHdl  ( LINK( this, ScSimpleRefDlg, CancelBtnHdl ) );
    bCloseFlag=false;
}




void ScSimpleRefDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( m_pEdAssign->IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(m_pEdAssign);

        theCurArea = rRef;
        OUString aRefStr;
        if ( bSingleCell )
        {
            ScAddress aAdr = rRef.aStart;
            aRefStr = aAdr.Format(SCA_ABS_3D, pDocP, pDocP->GetAddressConvention());
        }
        else
            aRefStr = theCurArea.Format(ABS_DREF3D, pDocP, pDocP->GetAddressConvention());

        if ( bMultiSelection )
        {
            OUString aVal = m_pEdAssign->GetText();
            Selection aSel = m_pEdAssign->GetSelection();
            aSel.Justify();
            aVal = aVal.replaceAt( aSel.Min(), aSel.Len(), aRefStr );
            Selection aNewSel( aSel.Min(), aSel.Min()+aRefStr.getLength() );
            m_pEdAssign->SetRefString( aVal );
            m_pEdAssign->SetSelection( aNewSel );
        }
        else
            m_pEdAssign->SetRefString( aRefStr );

        aChangeHdl.Call( &aRefStr );
    }
}



bool ScSimpleRefDlg::Close()
{
    CancelBtnHdl(m_pBtnCancel);
    return true;
}


void ScSimpleRefDlg::SetActive()
{
    m_pEdAssign->GrabFocus();

    
    
    

    RefInputDone();
}

bool ScSimpleRefDlg::IsRefInputMode() const
{
    return true;
}

OUString ScSimpleRefDlg::GetRefString() const
{
    return m_pEdAssign->GetText();
}

void ScSimpleRefDlg::SetCloseHdl( const Link& rLink )
{
    aCloseHdl=rLink;
}

void ScSimpleRefDlg::SetUnoLinks( const Link& rDone, const Link& rAbort,
                                    const Link& rChange )
{
    aDoneHdl    = rDone;
    aAbortedHdl = rAbort;
    aChangeHdl  = rChange;
}

void ScSimpleRefDlg::SetFlags( bool bSetCloseOnButtonUp, bool bSetSingleCell, bool bSetMultiSelection )
{
    bCloseOnButtonUp = bSetCloseOnButtonUp;
    bSingleCell = bSetSingleCell;
    bMultiSelection = bSetMultiSelection;
}

void ScSimpleRefDlg::StartRefInput()
{
    if ( bMultiSelection )
    {
        
        m_pEdAssign->SetSelection( Selection( 0, m_pEdAssign->GetText().getLength() ) );
    }

    m_pRbAssign->DoRef();
    bCloseFlag = true;
}

void ScSimpleRefDlg::RefInputDone( bool bForced)
{
    ScAnyRefDlg::RefInputDone(bForced);
    if ( (bForced || bCloseOnButtonUp) && bCloseFlag )
        OkBtnHdl(m_pBtnOk);
}



IMPL_LINK_NOARG(ScSimpleRefDlg, OkBtnHdl)
{
    bAutoReOpen=false;
    OUString aResult=m_pEdAssign->GetText();
    aCloseHdl.Call(&aResult);
    Link aUnoLink = aDoneHdl;       
    DoClose( ScSimpleRefDlgWrapper::GetChildWindowId() );
    aUnoLink.Call( &aResult );
    return 0;
}


IMPL_LINK_NOARG(ScSimpleRefDlg, CancelBtnHdl)
{
    bAutoReOpen=false;
    OUString aResult=m_pEdAssign->GetText();
    aCloseHdl.Call(NULL);
    Link aUnoLink = aAbortedHdl;    
    DoClose( ScSimpleRefDlgWrapper::GetChildWindowId() );
    aUnoLink.Call( &aResult );
    return 0;
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
