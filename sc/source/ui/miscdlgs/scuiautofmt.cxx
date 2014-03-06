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


// AutoFormat-Dialog:

ScAutoFormatDlg::ScAutoFormatDlg(Window* pParent,
    ScAutoFormat* pAutoFormat,
    const ScAutoFormatData* pSelFormatData,
    ScViewData *pViewData)
    : ModalDialog(pParent, "AutoFormatTableDialog",
        "modules/scalc/ui/autoformattable.ui")
    , aStrTitle(ScResId(STR_ADD_AUTOFORMAT_TITLE))
    , aStrLabel(ScResId(STR_ADD_AUTOFORMAT_LABEL))
    , aStrClose(ScResId(STR_BTN_AUTOFORMAT_CLOSE))
    , aStrDelTitle(ScResId(STR_DEL_AUTOFORMAT_TITLE))
    , aStrDelMsg(ScResId(STR_DEL_AUTOFORMAT_MSG))
    , aStrRename(ScResId(STR_RENAME_AUTOFORMAT_TITLE))
    , pFormat(pAutoFormat)
    , pSelFmtData(pSelFormatData)
    , nIndex(0)
    , bCoreDataChanged(false)
    , bFmtInserted(false)
{
    get(m_pLbFormat, "formatlb");
    get(m_pWndPreview, "preview");
    m_pWndPreview->DetectRTL(pViewData);
    get(m_pBtnOk, "ok");
    get(m_pBtnCancel, "cancel");
    get(m_pBtnAdd, "add");
    get(m_pBtnRemove, "remove");
    get(m_pBtnRename, "rename");
    get(m_pBtnNumFormat, "numformatcb");
    get(m_pBtnBorder, "bordercb");
    get(m_pBtnFont, "fontcb");
    get(m_pBtnPattern, "patterncb");
    get(m_pBtnAlignment, "alignmentcb");
    get(m_pBtnAdjust, "autofitcb");

    Init();
    ScAutoFormat::iterator it = pFormat->begin();
    m_pWndPreview->NotifyChange(it->second);
}

void ScAutoFormatDlg::Init()
{
    m_pLbFormat->SetSelectHdl( LINK( this, ScAutoFormatDlg, SelFmtHdl ) );
    m_pBtnNumFormat->SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_pBtnBorder->SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_pBtnFont->SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_pBtnPattern->SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_pBtnAlignment->SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_pBtnAdjust->SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_pBtnAdd->SetClickHdl ( LINK( this, ScAutoFormatDlg, AddHdl ) );
    m_pBtnRemove->SetClickHdl ( LINK( this, ScAutoFormatDlg, RemoveHdl ) );
    m_pBtnOk->SetClickHdl ( LINK( this, ScAutoFormatDlg, CloseHdl ) );
    m_pBtnCancel->SetClickHdl ( LINK( this, ScAutoFormatDlg, CloseHdl ) );
    m_pBtnRename->SetClickHdl ( LINK( this, ScAutoFormatDlg, RenameHdl ) );
    m_pLbFormat->SetDoubleClickHdl( LINK( this, ScAutoFormatDlg, DblClkHdl ) );

    ScAutoFormat::const_iterator it = pFormat->begin(), itEnd = pFormat->end();
    for (; it != itEnd; ++it)
        m_pLbFormat->InsertEntry(it->second->GetName());

    if (pFormat->size() == 1)
        m_pBtnRemove->Disable();

    m_pLbFormat->SelectEntryPos( 0 );
    m_pBtnRename->Disable();
    m_pBtnRemove->Disable();

    nIndex = 0;
    UpdateChecks();

    if ( !pSelFmtData )
    {
        m_pBtnAdd->Disable();
        m_pBtnRemove->Disable();
        bFmtInserted = true;
    }
}

void ScAutoFormatDlg::UpdateChecks()
{
    const ScAutoFormatData* pData = pFormat->findByIndex(nIndex);

    m_pBtnNumFormat->Check( pData->GetIncludeValueFormat() );
    m_pBtnBorder->Check( pData->GetIncludeFrame() );
    m_pBtnFont->Check( pData->GetIncludeFont() );
    m_pBtnPattern->Check( pData->GetIncludeBackground() );
    m_pBtnAlignment->Check( pData->GetIncludeJustify() );
    m_pBtnAdjust->Check( pData->GetIncludeWidthHeight() );
}


// Handler:


IMPL_LINK( ScAutoFormatDlg, CloseHdl, PushButton *, pBtn )
{
    if (pBtn == m_pBtnOk || pBtn == m_pBtnCancel)
    {
        if ( bCoreDataChanged )
            ScGlobal::GetOrCreateAutoFormat()->Save();

        EndDialog( (pBtn == m_pBtnOk) ? RET_OK : RET_CANCEL );
    }
    return 0;
}



IMPL_LINK_NOARG_INLINE_START(ScAutoFormatDlg, DblClkHdl)
{
    if ( bCoreDataChanged )
        ScGlobal::GetOrCreateAutoFormat()->Save();

    EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(ScAutoFormatDlg, DblClkHdl)



IMPL_LINK( ScAutoFormatDlg, CheckHdl, Button *, pBtn )
{
    ScAutoFormatData* pData = pFormat->findByIndex(nIndex);
    bool bCheck = ((CheckBox*)pBtn)->IsChecked();

    if ( pBtn == m_pBtnNumFormat )
        pData->SetIncludeValueFormat( bCheck );
    else if ( pBtn == m_pBtnBorder )
        pData->SetIncludeFrame( bCheck );
    else if ( pBtn == m_pBtnFont )
        pData->SetIncludeFont( bCheck );
    else if ( pBtn == m_pBtnPattern )
        pData->SetIncludeBackground( bCheck );
    else if ( pBtn == m_pBtnAlignment )
        pData->SetIncludeJustify( bCheck );
    else if ( pBtn == m_pBtnAdjust )
        pData->SetIncludeWidthHeight( bCheck );

    if ( !bCoreDataChanged )
    {
        m_pBtnCancel->SetText( aStrClose );
        bCoreDataChanged = true;
    }

    m_pWndPreview->NotifyChange( pData );

    return 0;
}



IMPL_LINK_NOARG(ScAutoFormatDlg, AddHdl)
{
    if ( !bFmtInserted && pSelFmtData )
    {
        OUString aStrStandard( SfxResId(STR_STANDARD) );
        OUString aFormatName;
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
                aFormatName = pDlg->GetInputString();

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
                        m_pLbFormat->InsertEntry(aFormatName, nPos);
                        m_pLbFormat->SelectEntry( aFormatName );
                        m_pBtnAdd->Disable();

                        if ( !bCoreDataChanged )
                        {
                            m_pBtnCancel->SetText( aStrClose );
                            bCoreDataChanged = true;
                        }

                        SelFmtHdl( 0 );
                        bOk = true;
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
                bOk = true;

            delete pDlg;
        }
    }

    return 0;
}



IMPL_LINK_NOARG(ScAutoFormatDlg, RemoveHdl)
{
    if ( (nIndex > 0) && (m_pLbFormat->GetEntryCount() > 0) )
    {
        OUString aMsg( aStrDelMsg.getToken( 0, '#' ) );

        aMsg += m_pLbFormat->GetSelectEntry();
        aMsg += aStrDelMsg.getToken( 1, '#' );

        if ( RET_YES ==
             QueryBox( this, WinBits( WB_YES_NO | WB_DEF_YES ), aMsg ).Execute() )
        {
            m_pLbFormat->RemoveEntry( nIndex );
            m_pLbFormat->SelectEntryPos( nIndex-1 );

            if ( nIndex-1 == 0 )
                m_pBtnRemove->Disable();

            if ( !bCoreDataChanged )
            {
                m_pBtnCancel->SetText( aStrClose );
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

        OUString aFormatName = m_pLbFormat->GetSelectEntry();
        OUString aEntry;

        ScStringInputDlg* pDlg = new ScStringInputDlg( this,
                                         aStrRename,
                                         aStrLabel,
                                         aFormatName,
                                         HID_SC_REN_AFMT_DLG, HID_SC_REN_AFMT_NAME );
        if( pDlg->Execute() == RET_OK )
        {
            bool bFmtRenamed = false;
            aFormatName = pDlg->GetInputString();

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

                    m_pLbFormat->RemoveEntry(nIndex );
                    const ScAutoFormatData* p = pFormat->findByIndex(nIndex);
                    ScAutoFormatData* pNewData
                        = new ScAutoFormatData(*p);

                    it = pFormat->begin();
                    std::advance(it, nIndex);
                    pFormat->erase(it);

                    pNewData->SetName( aFormatName );

                    pFormat->insert(pNewData);

                    m_pLbFormat->SetUpdateMode(false);
                    m_pLbFormat->Clear();
                    for (it = pFormat->begin(); it != itEnd; ++it)
                    {
                        aEntry = it->second->GetName();
                        m_pLbFormat->InsertEntry( aEntry );
                    }

                    m_pLbFormat->SetUpdateMode(true);
                    m_pLbFormat->SelectEntry( aFormatName);

                    if ( !bCoreDataChanged )
                    {
                        m_pBtnCancel->SetText( aStrClose );
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



IMPL_LINK_NOARG(ScAutoFormatDlg, SelFmtHdl)
{
    nIndex = m_pLbFormat->GetSelectEntryPos();
    UpdateChecks();

    if ( nIndex == 0 )
    {
        m_pBtnRename->Disable();
        m_pBtnRemove->Disable();
    }
    else
    {
        m_pBtnRename->Enable();
        m_pBtnRemove->Enable();
    }

    ScAutoFormatData* p = pFormat->findByIndex(nIndex);
    m_pWndPreview->NotifyChange(p);

    return 0;
}



OUString ScAutoFormatDlg::GetCurrFormatName()
{
    const ScAutoFormatData* p = pFormat->findByIndex(nIndex);
    return p ? p->GetName() : OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
