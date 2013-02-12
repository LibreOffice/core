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


#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <svl/zforlist.hxx>
#include <vcl/msgbox.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/sfxresid.hxx>
#include "sc.hrc"
#include "scmod.hxx"
#include "attrib.hxx"
#include "zforauto.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "autoform.hxx"
#include "strindlg.hxx"
#include "miscdlgs.hrc"
#include "scuiautofmt.hxx"
#include "scresid.hxx"
#include "document.hxx"

//========================================================================
// AutoFormat-Dialog:

ScAutoFormatDlg::ScAutoFormatDlg( Window*                   pParent,
                                  ScAutoFormat*             pAutoFormat,
                                  const ScAutoFormatData*   pSelFormatData,
                                  ScDocument*               pDoc ) :

    ModalDialog     ( pParent, ScResId( RID_SCDLG_AUTOFORMAT ) ),
    //
    aFlFormat       ( this, ScResId( FL_FORMAT ) ),
    aLbFormat       ( this, ScResId( LB_FORMAT ) ),
    pWndPreview     ( new ScAutoFmtPreview( this, ScResId( WND_PREVIEW ), pDoc ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) ),
    aBtnAdd         ( this, ScResId( BTN_ADD ) ),
    aBtnRemove      ( this, ScResId( BTN_REMOVE ) ),
    aBtnMore        ( this, ScResId( BTN_MORE ) ),
    aFlFormatting   ( this, ScResId( FL_FORMATTING ) ),
    aBtnNumFormat   ( this, ScResId( BTN_NUMFORMAT ) ),
    aBtnBorder      ( this, ScResId( BTN_BORDER ) ),
    aBtnFont        ( this, ScResId( BTN_FONT ) ),
    aBtnPattern     ( this, ScResId( BTN_PATTERN ) ),
    aBtnAlignment   ( this, ScResId( BTN_ALIGNMENT ) ),
    aBtnAdjust      ( this, ScResId( BTN_ADJUST ) ),
    aBtnRename      ( this, ScResId( BTN_RENAME ) ),
    aStrTitle       ( ScResId( STR_ADD_TITLE ) ),
    aStrLabel       ( ScResId( STR_ADD_LABEL ) ),
    aStrClose       ( ScResId( STR_BTN_CLOSE ) ),
    aStrDelTitle    ( ScResId( STR_DEL_TITLE ) ),
    aStrDelMsg      ( ScResId( STR_DEL_MSG ) ) ,
    aStrRename      ( ScResId( STR_RENAME_TITLE ) ),
    //
    pFormat         ( pAutoFormat ),
    pSelFmtData     ( pSelFormatData ),
    nIndex          ( 0 ),
    bCoreDataChanged( false ),
    bFmtInserted    ( false )
{
    Init();
    ScAutoFormat::iterator it = pFormat->begin();
    pWndPreview->NotifyChange(it->second);
    FreeResource();
}

//------------------------------------------------------------------------

ScAutoFormatDlg::~ScAutoFormatDlg()
{
    delete pWndPreview;
}

//------------------------------------------------------------------------

void ScAutoFormatDlg::Init()
{
    aLbFormat    .SetSelectHdl( LINK( this, ScAutoFormatDlg, SelFmtHdl ) );
    aBtnNumFormat.SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    aBtnBorder   .SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    aBtnFont     .SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    aBtnPattern  .SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    aBtnAlignment.SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    aBtnAdjust   .SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    aBtnAdd      .SetClickHdl ( LINK( this, ScAutoFormatDlg, AddHdl ) );
    aBtnRemove   .SetClickHdl ( LINK( this, ScAutoFormatDlg, RemoveHdl ) );
    aBtnOk       .SetClickHdl ( LINK( this, ScAutoFormatDlg, CloseHdl ) );
    aBtnCancel   .SetClickHdl ( LINK( this, ScAutoFormatDlg, CloseHdl ) );
    aBtnRename   .SetClickHdl ( LINK( this, ScAutoFormatDlg, RenameHdl ) );
    aLbFormat    .SetDoubleClickHdl( LINK( this, ScAutoFormatDlg, DblClkHdl ) );

    aBtnMore.AddWindow( &aBtnRename );
    aBtnMore.AddWindow( &aBtnNumFormat );
    aBtnMore.AddWindow( &aBtnBorder );
    aBtnMore.AddWindow( &aBtnFont );
    aBtnMore.AddWindow( &aBtnPattern );
    aBtnMore.AddWindow( &aBtnAlignment );
    aBtnMore.AddWindow( &aBtnAdjust );
    aBtnMore.AddWindow( &aFlFormatting );

    ScAutoFormat::const_iterator it = pFormat->begin(), itEnd = pFormat->end();
    for (; it != itEnd; ++it)
        aLbFormat.InsertEntry(it->second->GetName());

    if (pFormat->size() == 1)
        aBtnRemove.Disable();

    aLbFormat.SelectEntryPos( 0 );
    aBtnRename.Disable();
    aBtnRemove.Disable();

    nIndex = 0;
    UpdateChecks();

    if ( !pSelFmtData )
    {
        aBtnAdd.Disable();
        aBtnRemove.Disable();
        bFmtInserted = true;
    }
}

//------------------------------------------------------------------------

void ScAutoFormatDlg::UpdateChecks()
{
    const ScAutoFormatData* pData = pFormat->findByIndex(nIndex);

    aBtnNumFormat.Check( pData->GetIncludeValueFormat() );
    aBtnBorder   .Check( pData->GetIncludeFrame() );
    aBtnFont     .Check( pData->GetIncludeFont() );
    aBtnPattern  .Check( pData->GetIncludeBackground() );
    aBtnAlignment.Check( pData->GetIncludeJustify() );
    aBtnAdjust   .Check( pData->GetIncludeWidthHeight() );
}

//------------------------------------------------------------------------
// Handler:
//---------

IMPL_LINK( ScAutoFormatDlg, CloseHdl, PushButton *, pBtn )
{
    if ( pBtn == &aBtnOk || pBtn == &aBtnCancel )
    {
        if ( bCoreDataChanged )
            ScGlobal::GetOrCreateAutoFormat()->Save();

        EndDialog( (pBtn == &aBtnOk) ? RET_OK : RET_CANCEL );
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG_INLINE_START(ScAutoFormatDlg, DblClkHdl)
{
    if ( bCoreDataChanged )
        ScGlobal::GetOrCreateAutoFormat()->Save();

    EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(ScAutoFormatDlg, DblClkHdl)

//------------------------------------------------------------------------

IMPL_LINK( ScAutoFormatDlg, CheckHdl, Button *, pBtn )
{
    ScAutoFormatData* pData = pFormat->findByIndex(nIndex);
    bool bCheck = ((CheckBox*)pBtn)->IsChecked();

    if ( pBtn == &aBtnNumFormat )
        pData->SetIncludeValueFormat( bCheck );
    else if ( pBtn == &aBtnBorder )
        pData->SetIncludeFrame( bCheck );
    else if ( pBtn == &aBtnFont )
        pData->SetIncludeFont( bCheck );
    else if ( pBtn == &aBtnPattern )
        pData->SetIncludeBackground( bCheck );
    else if ( pBtn == &aBtnAlignment )
        pData->SetIncludeJustify( bCheck );
    else if ( pBtn == &aBtnAdjust )
        pData->SetIncludeWidthHeight( bCheck );

    if ( !bCoreDataChanged )
    {
        aBtnCancel.SetText( aStrClose );
        bCoreDataChanged = true;
    }

    pWndPreview->NotifyChange( pData );

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(ScAutoFormatDlg, AddHdl)
{
    if ( !bFmtInserted && pSelFmtData )
    {
        String              aStrStandard( SfxResId(STR_STANDARD) );
        rtl::OUString aFormatName;
        ScStringInputDlg*   pDlg;
        bool bOk = false;

        while ( !bOk )
        {
            pDlg = new ScStringInputDlg( this,
                                         aStrTitle,
                                         aStrLabel,
                                         aFormatName,
                                         HID_SC_ADD_AUTOFMT, HID_SC_AUTOFMT_NAME );

            if ( pDlg->Execute() == RET_OK )
            {
                pDlg->GetInputString( aFormatName );

                if ( !aFormatName.isEmpty() && !aFormatName.equals(aStrStandard) )
                {
                    ScAutoFormatData* pNewData
                        = new ScAutoFormatData( *pSelFmtData );

                    pNewData->SetName( aFormatName );
                    bFmtInserted = pFormat->insert(pNewData);

                    if ( bFmtInserted )
                    {
                        ScAutoFormat::const_iterator it = pFormat->find(pNewData);
                        ScAutoFormat::const_iterator itBeg = pFormat->begin();
                        size_t nPos = std::distance(itBeg, it);
                        aLbFormat.InsertEntry(aFormatName, nPos);
                        aLbFormat.SelectEntry( aFormatName );
                        aBtnAdd.Disable();

                        if ( !bCoreDataChanged )
                        {
                            aBtnCancel.SetText( aStrClose );
                            bCoreDataChanged = true;
                        }

                        SelFmtHdl( 0 );
                        bOk = sal_True;
                    }
                    else
                        delete pNewData;

                }

                if ( !bFmtInserted )
                {
                    sal_uInt16 nRet = ErrorBox( this,
                                            WinBits( WB_OK_CANCEL | WB_DEF_OK),
                                            ScGlobal::GetRscString(STR_INVALID_AFNAME)
                                          ).Execute();

                    bOk = ( nRet == RET_CANCEL );
                }
            }
            else
                bOk = sal_True;

            delete pDlg;
        }
    }

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(ScAutoFormatDlg, RemoveHdl)
{
    if ( (nIndex > 0) && (aLbFormat.GetEntryCount() > 0) )
    {
        String aMsg( aStrDelMsg.GetToken( 0, '#' ) );

        aMsg += aLbFormat.GetSelectEntry();
        aMsg += aStrDelMsg.GetToken( 1, '#' );

        if ( RET_YES ==
             QueryBox( this, WinBits( WB_YES_NO | WB_DEF_YES ), aMsg ).Execute() )
        {
            aLbFormat.RemoveEntry( nIndex );
            aLbFormat.SelectEntryPos( nIndex-1 );

            if ( nIndex-1 == 0 )
                aBtnRemove.Disable();

            if ( !bCoreDataChanged )
            {
                aBtnCancel.SetText( aStrClose );
                bCoreDataChanged = true;
            }

            ScAutoFormat::iterator it = pFormat->begin();
            std::advance(it, nIndex);
            pFormat->erase(it);
            nIndex--;

            SelFmtHdl( 0 );
        }
    }

    SelFmtHdl( 0 );

    return 0;
}

IMPL_LINK_NOARG(ScAutoFormatDlg, RenameHdl)
{
    sal_Bool bOk = false;
    while( !bOk )
    {

        rtl::OUString aFormatName = aLbFormat.GetSelectEntry();
        String aEntry;

        ScStringInputDlg* pDlg = new ScStringInputDlg( this,
                                         aStrRename,
                                         aStrLabel,
                                         aFormatName,
                                         HID_SC_REN_AFMT_DLG, HID_SC_REN_AFMT_NAME );
        if( pDlg->Execute() == RET_OK )
        {
            bool bFmtRenamed = false;
            pDlg->GetInputString( aFormatName );

            if (!aFormatName.isEmpty())
            {
                ScAutoFormat::iterator it = pFormat->begin(), itEnd = pFormat->end();
                for (; it != itEnd; ++it)
                {
                    aEntry = it->second->GetName();
                    if (aFormatName.equals(aEntry))
                        break;
                }
                if (it == itEnd)
                {
                    // Format mit dem Namen noch nicht vorhanden, also
                    // umbenennen

                    aLbFormat.RemoveEntry(nIndex );
                    const ScAutoFormatData* p = pFormat->findByIndex(nIndex);
                    ScAutoFormatData* pNewData
                        = new ScAutoFormatData(*p);

                    it = pFormat->begin();
                    std::advance(it, nIndex);
                    pFormat->erase(it);

                    pNewData->SetName( aFormatName );

                    pFormat->insert(pNewData);

                    aLbFormat.SetUpdateMode(false);
                    aLbFormat.Clear();
                    for (it = pFormat->begin(); it != itEnd; ++it)
                    {
                        aEntry = it->second->GetName();
                        aLbFormat.InsertEntry( aEntry );
                    }

                    aLbFormat.SetUpdateMode(true);
                    aLbFormat.SelectEntry( aFormatName);

                    if ( !bCoreDataChanged )
                    {
                        aBtnCancel.SetText( aStrClose );
                        bCoreDataChanged = true;
                    }


                    SelFmtHdl( 0 );
                    bOk = true;
                    bFmtRenamed = true;
                }
            }
            if( !bFmtRenamed )
            {
                bOk = RET_CANCEL == ErrorBox( this,
                                    WinBits( WB_OK_CANCEL | WB_DEF_OK),
                                    ScGlobal::GetRscString(STR_INVALID_AFNAME)
                                    ).Execute();
            }
        }
        else
            bOk = true;
        delete pDlg;
    }

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(ScAutoFormatDlg, SelFmtHdl)
{
    nIndex = aLbFormat.GetSelectEntryPos();
    UpdateChecks();

    if ( nIndex == 0 )
    {
        aBtnRename.Disable();
        aBtnRemove.Disable();
    }
    else
    {
        aBtnRename.Enable();
        aBtnRemove.Enable();
    }

    ScAutoFormatData* p = pFormat->findByIndex(nIndex);
    pWndPreview->NotifyChange(p);

    return 0;
}

//------------------------------------------------------------------------

rtl::OUString ScAutoFormatDlg::GetCurrFormatName()
{
    const ScAutoFormatData* p = pFormat->findByIndex(nIndex);
    return p ? p->GetName() : rtl::OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
