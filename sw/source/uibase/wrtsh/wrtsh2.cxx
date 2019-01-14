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

#include <hintids.hxx>
#include <svl/macitem.hxx>
#include <sfx2/frame.hxx>
#include <svl/urihelper.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/linkmgr.hxx>
#include <fmtinfmt.hxx>
#include <frmatr.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <fldbas.hxx>
#include <expfld.hxx>
#include <ddefld.hxx>
#include <docufld.hxx>
#include <reffld.hxx>
#include <swundo.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <viewopt.hxx>
#include <frmfmt.hxx>
#include <fmtfld.hxx>
#include <swtable.hxx>
#include <mdiexp.hxx>
#include <view.hxx>
#include <swevent.hxx>
#include <poolfmt.hxx>
#include <section.hxx>
#include <navicont.hxx>
#include <navipi.hxx>
#include <txtinet.hxx>
#include <cmdid.h>
#include <swabstdlg.hxx>
#include <SwRewriter.hxx>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <memory>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <sfx2/event.hxx>
#include <sal/log.hxx>

void SwWrtShell::Insert(SwField const &rField)
{
    ResetCursorStack();
    if(!CanInsert())
        return;
    StartAllAction();

    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, rField.GetDescription());

    StartUndo(SwUndoId::INSERT, &aRewriter);

    bool bDeleted = false;
    std::unique_ptr<SwPaM> pAnnotationTextRange;
    if ( HasSelection() )
    {
        if ( rField.GetTyp()->Which() == SwFieldIds::Postit )
        {
            // for annotation fields:
            // - keep the current selection in order to create a corresponding annotation mark
            // - collapse cursor to its end
            if ( IsTableMode() )
            {
                GetTableCrs()->Normalize( false );
                const SwPosition rStartPos( *(GetTableCrs()->GetMark()->nNode.GetNode().GetContentNode()), 0 );
                KillPams();
                if ( !IsEndOfPara() )
                {
                    EndPara();
                }
                const SwPosition rEndPos( *GetCurrentShellCursor().GetPoint() );
                pAnnotationTextRange.reset(new SwPaM( rStartPos, rEndPos ));
            }
            else
            {
                NormalizePam( false );
                const SwPaM& rCurrPaM = GetCurrentShellCursor();
                pAnnotationTextRange.reset(new SwPaM( *rCurrPaM.GetPoint(), *rCurrPaM.GetMark() ));
                ClearMark();
            }
        }
        else
        {
            bDeleted = DelRight();
        }
    }

    SwEditShell::Insert2(rField, bDeleted);

    if ( pAnnotationTextRange )
    {
        if ( GetDoc() != nullptr )
        {
            IDocumentMarkAccess* pMarksAccess = GetDoc()->getIDocumentMarkAccess();
            pMarksAccess->makeAnnotationMark( *pAnnotationTextRange, OUString() );
        }
        pAnnotationTextRange.reset();
    }

    EndUndo();
    EndAllAction();
}

// Start the field update

void SwWrtShell::UpdateInputFields( SwInputFieldList* pLst )
{
    // Go through the list of fields and updating
    std::unique_ptr<SwInputFieldList> pTmp;
    if (!pLst)
    {
        pTmp.reset(new SwInputFieldList( this ));
        pLst = pTmp.get();
    }

    const size_t nCnt = pLst->Count();
    if(nCnt)
    {
        pLst->PushCursor();

        bool bCancel = false;

        size_t nIndex = 0;
        FieldDialogPressedButton ePressedButton = FieldDialogPressedButton::NONE;

        SwField* pField = GetCurField();
        if (pField)
        {
            for (size_t i = 0; i < nCnt; i++)
            {
                if (pField == pLst->GetField(i))
                {
                    nIndex = i;
                    break;
                }
            }
        }

        while (!bCancel)
        {
            bool bPrev = nIndex > 0;
            bool bNext = nIndex < nCnt - 1;
            pLst->GotoFieldPos(nIndex);
            pField = pLst->GetField(nIndex);
            if (pField->GetTyp()->Which() == SwFieldIds::Dropdown)
            {
                bCancel = StartDropDownFieldDlg(pField, bPrev, bNext, GetView().GetFrameWeld(), &ePressedButton);
            }
            else
                bCancel = StartInputFieldDlg(pField, bPrev, bNext, GetView().GetFrameWeld(), &ePressedButton);

            if (!bCancel)
            {
                // Otherwise update error at multi-selection:
                pLst->GetField(nIndex)->GetTyp()->UpdateFields();

                if (ePressedButton == FieldDialogPressedButton::Previous && nIndex > 0)
                    nIndex--;
                else if (ePressedButton == FieldDialogPressedButton::Next && nIndex < nCnt - 1)
                    nIndex++;
                else
                    bCancel = true;
            }
        }

        pLst->PopCursor();
    }
}

// Listener class: will close InputField dialog if input field(s)
// is(are) deleted (for instance, by an extension) after the dialog shows up.
// Otherwise, the for loop in SwWrtShell::UpdateInputFields will crash when doing:
//         'pTmp->GetField( i )->GetTyp()->UpdateFields();'
// on a deleted field.
class FieldDeletionModify : public SwModify
{
    public:
        FieldDeletionModify(AbstractFieldInputDlg* pInputFieldDlg, SwField* pField)
            : mpInputFieldDlg(pInputFieldDlg)
            , mpFormatField(nullptr)
        {
            SwInputField *const pInputField(dynamic_cast<SwInputField*>(pField));
            SwSetExpField *const pSetExpField(dynamic_cast<SwSetExpField*>(pField));

            if (pInputField && pInputField->GetFormatField())
            {
                mpFormatField = pInputField->GetFormatField();
            }
            else if (pSetExpField && pSetExpField->GetFormatField())
            {
                mpFormatField = pSetExpField->GetFormatField();
            }

            // Register for possible field deletion while dialog is open
            if (mpFormatField)
                mpFormatField->Add(this);
        }

        virtual ~FieldDeletionModify() override
        {
            // Dialog closed, remove modification listener
            EndListeningAll();
        }

        void Modify( const SfxPoolItem* pOld, const SfxPoolItem *) override
        {
            // Input field has been deleted: better to close the dialog
            if (pOld)
            {
                switch (pOld->Which())
                {
                case RES_REMOVE_UNO_OBJECT:
                case RES_OBJECTDYING:
                    mpFormatField = nullptr;
                    mpInputFieldDlg->EndDialog(RET_CANCEL);
                    break;
                }
            }
        }
    private:
        VclPtr<AbstractFieldInputDlg> mpInputFieldDlg;
        SwFormatField* mpFormatField;
};

// Start input dialog for a specific field
bool SwWrtShell::StartInputFieldDlg(SwField* pField, bool bPrevButton, bool bNextButton,
                                    weld::Window* pParentWin, SwWrtShell::FieldDialogPressedButton* pPressedButton)
{

    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractFieldInputDlg> pDlg(pFact->CreateFieldInputDlg(pParentWin, *this, pField, bPrevButton, bNextButton));

    bool bRet;

    {
        FieldDeletionModify aModify(pDlg.get(), pField);
        bRet = RET_CANCEL == pDlg->Execute();
    }

    if (pPressedButton)
    {
        if (pDlg->PrevButtonPressed())
            *pPressedButton = FieldDialogPressedButton::Previous;
        else if (pDlg->NextButtonPressed())
            *pPressedButton = FieldDialogPressedButton::Next;
    }

    pDlg.disposeAndClear();
    GetWin()->Update();
    return bRet;
}

bool SwWrtShell::StartDropDownFieldDlg(SwField* pField, bool bPrevButton, bool bNextButton,
                                       weld::Window* pParentWin, SwWrtShell::FieldDialogPressedButton* pPressedButton)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractDropDownFieldDialog> pDlg(pFact->CreateDropDownFieldDialog(pParentWin, *this, pField, bPrevButton, bNextButton));
    const short nRet = pDlg->Execute();

    if (pPressedButton)
    {
        if (pDlg->PrevButtonPressed())
            *pPressedButton = FieldDialogPressedButton::Previous;
        else if (pDlg->NextButtonPressed())
            *pPressedButton = FieldDialogPressedButton::Next;
    }

    pDlg.disposeAndClear();
    bool bRet = RET_CANCEL == nRet;
    GetWin()->Update();
    if(RET_YES == nRet)
    {
        GetView().GetViewFrame()->GetDispatcher()->Execute(FN_EDIT_FIELD, SfxCallMode::SYNCHRON);
    }
    return bRet;
}

// Insert directory - remove selection

void SwWrtShell::InsertTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet)
{
    if(!CanInsert())
        return;

    if(HasSelection())
        DelRight();

    SwEditShell::InsertTableOf(rTOX, pSet);
}

// Update directory - remove selection

void SwWrtShell::UpdateTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet)
{
    if(CanInsert())
    {
        SwEditShell::UpdateTableOf(rTOX, pSet);

        if (pSet == nullptr)
        {
            SwDoc *const pDoc_ = GetDoc();
            if (pDoc_)
            {
                pDoc_->GetIDocumentUndoRedo().DelAllUndoObj();
            }
        }
    }
}

// handler for click on the field given as parameter.
// the cursor is positioned on the field.

void SwWrtShell::ClickToField( const SwField& rField )
{
    // cross reference field must not be selected because it moves the cursor
    if (SwFieldIds::GetRef != rField.GetTyp()->Which())
    {
        StartAllAction();
        Right( CRSR_SKIP_CHARS, true, 1, false ); // Select the field.
        NormalizePam();
        EndAllAction();
    }

    m_bIsInClickToEdit = true;
    switch( rField.GetTyp()->Which() )
    {
    case SwFieldIds::JumpEdit:
        {
            sal_uInt16 nSlotId = 0;
            switch( rField.GetFormat() )
            {
            case JE_FMT_TABLE:
                nSlotId = FN_INSERT_TABLE;
                break;

            case JE_FMT_FRAME:
                nSlotId = FN_INSERT_FRAME;
                break;

            case JE_FMT_GRAPHIC:    nSlotId = SID_INSERT_GRAPHIC;       break;
            case JE_FMT_OLE:        nSlotId = SID_INSERT_OBJECT;        break;

            }

            if( nSlotId )
            {
                StartUndo( SwUndoId::START );
                //#97295# immediately select the right shell
                GetView().StopShellTimer();
                GetView().GetViewFrame()->GetDispatcher()->Execute( nSlotId,
                            SfxCallMode::SYNCHRON|SfxCallMode::RECORD );
                EndUndo( SwUndoId::END );
            }
        }
        break;

    case SwFieldIds::Macro:
        {
            const SwMacroField *pField = static_cast<const SwMacroField*>(&rField);
            const OUString sText( rField.GetPar2() );
            OUString sRet( sText );
            ExecMacro( pField->GetSvxMacro(), &sRet );

            // return value changed?
            if( sRet != sText )
            {
                StartAllAction();
                const_cast<SwField&>(rField).SetPar2( sRet );
                rField.GetTyp()->UpdateFields();
                EndAllAction();
            }
        }
        break;

    case SwFieldIds::GetRef:
        StartAllAction();
        SwCursorShell::GotoRefMark( static_cast<const SwGetRefField&>(rField).GetSetRefName(),
                                    static_cast<const SwGetRefField&>(rField).GetSubType(),
                                    static_cast<const SwGetRefField&>(rField).GetSeqNo() );
        EndAllAction();
        break;

    case SwFieldIds::Input:
        {
            const SwInputField* pInputField = dynamic_cast<const SwInputField*>(&rField);
            if ( pInputField == nullptr )
            {
                StartInputFieldDlg(const_cast<SwField*>(&rField), false, false, GetView().GetFrameWeld());
            }
        }
        break;

    case SwFieldIds::SetExp:
        if( static_cast<const SwSetExpField&>(rField).GetInputFlag() )
            StartInputFieldDlg(const_cast<SwField*>(&rField), false, false, GetView().GetFrameWeld());
        break;
    case SwFieldIds::Dropdown :
        StartDropDownFieldDlg(const_cast<SwField*>(&rField), false, false, GetView().GetFrameWeld());
    break;
    default:
        SAL_WARN_IF(rField.IsClickable(), "sw", "unhandled clickable field!");
    }

    m_bIsInClickToEdit = false;
}

void SwWrtShell::ClickToINetAttr( const SwFormatINetFormat& rItem, LoadUrlFlags nFilter )
{
    if( rItem.GetValue().isEmpty() )
        return ;

    m_bIsInClickToEdit = true;

    // At first run the possibly set ObjectSelect Macro
    const SvxMacro* pMac = rItem.GetMacro( SvMacroItemId::OnClick );
    if( pMac )
    {
        SwCallMouseEvent aCallEvent;
        aCallEvent.Set( &rItem );
        GetDoc()->CallEvent( SvMacroItemId::OnClick, aCallEvent );
    }

    // So that the implementation of templates is displayed immediately
    ::LoadURL( *this, rItem.GetValue(), nFilter, rItem.GetTargetFrame() );
    const SwTextINetFormat* pTextAttr = rItem.GetTextINetFormat();
    if( pTextAttr )
    {
        const_cast<SwTextINetFormat*>(pTextAttr)->SetVisited( true );
        const_cast<SwTextINetFormat*>(pTextAttr)->SetVisitedValid( true );
    }

    m_bIsInClickToEdit = false;
}

bool SwWrtShell::ClickToINetGrf( const Point& rDocPt, LoadUrlFlags nFilter )
{
    bool bRet = false;
    OUString sURL;
    OUString sTargetFrameName;
    const SwFrameFormat* pFnd = IsURLGrfAtPos( rDocPt, &sURL, &sTargetFrameName );
    if( pFnd && !sURL.isEmpty() )
    {
        bRet = true;
        // At first run the possibly set ObjectSelect Macro
        SwCallMouseEvent aCallEvent;
        aCallEvent.Set(EVENT_OBJECT_URLITEM, pFnd);
        GetDoc()->CallEvent(SvMacroItemId::OnClick, aCallEvent);

        ::LoadURL(*this, sURL, nFilter, sTargetFrameName);
    }
    return bRet;
}

void LoadURL( SwViewShell& rVSh, const OUString& rURL, LoadUrlFlags nFilter,
              const OUString& rTargetFrameName )
{
    OSL_ENSURE( !rURL.isEmpty(), "what should be loaded here?" );
    if( rURL.isEmpty() )
        return ;

    // The shell could be 0 also!!!!!
    if ( dynamic_cast<const SwCursorShell*>( &rVSh) ==  nullptr )
        return;

    OUString sFileURL = rURL;
    INetURLObject aURL( sFileURL );
    if( aURL.GetProtocol() == INetProtocol::NotValid && !sFileURL.startsWith("#") )
    {
        // May be the relative link -> try to convert to absolute path
        OUString sParentPath =
            rVSh.GetDoc()->GetDocShell()->GetMedium()->GetURLObject().GetPath();

        bool bCorrectURL = true;
        aURL = INetURLObject();
        bCorrectURL &= aURL.setFSysPath( sParentPath, FSysStyle::Detect );
        bCorrectURL &= aURL.insertName( sFileURL );

        if( bCorrectURL )
            sFileURL = aURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous );
    }

    // We are doing tiledRendering, let the client handles the URL loading,
    // unless we are jumping to a TOC mark.
    if (comphelper::LibreOfficeKit::isActive() && !rURL.startsWith("#"))
    {
        rVSh.GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_HYPERLINK_CLICKED, sFileURL.toUtf8().getStr());
        return;
    }

    //A CursorShell is always a WrtShell
    SwWrtShell &rSh = static_cast<SwWrtShell&>(rVSh);

    SwDocShell* pDShell = rSh.GetView().GetDocShell();
    OSL_ENSURE( pDShell, "No DocShell?!");
    OUString sTargetFrame(rTargetFrameName);
    if (sTargetFrame.isEmpty() && pDShell)
    {
        using namespace ::com::sun::star;
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps
            = xDPS->getDocumentProperties();
        sTargetFrame = xDocProps->getDefaultTarget();
    }

    OUString sReferer;
    if( pDShell && pDShell->GetMedium() )
        sReferer = pDShell->GetMedium()->GetName();
    SfxViewFrame* pViewFrame = rSh.GetView().GetViewFrame();
    SfxFrameItem aView( SID_DOCFRAME, pViewFrame );
    SfxStringItem aName( SID_FILE_NAME, sFileURL );
    SfxStringItem aTargetFrameName( SID_TARGETNAME, sTargetFrame );
    SfxStringItem aReferer( SID_REFERER, sReferer );

    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, false );
    //#39076# Silent can be removed accordingly to SFX.
    SfxBoolItem aBrowse( SID_BROWSE, true );

    if ((nFilter & LoadUrlFlags::NewView) && !comphelper::LibreOfficeKit::isActive())
        aTargetFrameName.SetValue( "_blank" );

    const SfxPoolItem* aArr[] = {
                &aName,
                &aNewView, /*&aSilent,*/
                &aReferer,
                &aView, &aTargetFrameName,
                &aBrowse,
                nullptr
    };

    pViewFrame->GetDispatcher()->GetBindings()->Execute( SID_OPENDOC, aArr,
            SfxCallMode::ASYNCHRON|SfxCallMode::RECORD );
}

void SwWrtShell::NavigatorPaste( const NaviContentBookmark& rBkmk,
                                    const sal_uInt16 nAction )
{
    if( EXCHG_IN_ACTION_COPY == nAction )
    {
        // Insert
        OUString sURL = rBkmk.GetURL();
        // Is this is a jump within the current Doc?
        const SwDocShell* pDocShell = GetView().GetDocShell();
        if(pDocShell->HasName())
        {
            const OUString rName = pDocShell->GetMedium()->GetURLObject().GetURLNoMark();

            if (sURL.startsWith(rName))
            {
                if (sURL.getLength()>rName.getLength())
                {
                    sURL = sURL.copy(rName.getLength());
                }
                else
                {
                    sURL.clear();
                }
            }
        }
        SwFormatINetFormat aFormat( sURL, OUString() );
        InsertURL( aFormat, rBkmk.GetDescription() );
    }
    else
    {
        SwSectionData aSection( FILE_LINK_SECTION, GetUniqueSectionName() );
        OUString aLinkFile = rBkmk.GetURL().getToken(0, '#')
            + OUStringLiteral1(sfx2::cTokenSeparator)
            + OUStringLiteral1(sfx2::cTokenSeparator)
            + rBkmk.GetURL().getToken(1, '#');
        aSection.SetLinkFileName( aLinkFile );
        aSection.SetProtectFlag( true );
        const SwSection* pIns = InsertSection( aSection );
        if( EXCHG_IN_ACTION_MOVE == nAction && pIns )
        {
            aSection = SwSectionData(*pIns);
            aSection.SetLinkFileName( OUString() );
            aSection.SetType( CONTENT_SECTION );
            aSection.SetProtectFlag( false );

            // the update of content from linked section at time delete
            // the undostack. Then the change of the section don't create
            // any undoobject. -  BUG 69145
            bool bDoesUndo = DoesUndo();
            SwUndoId nLastUndoId(SwUndoId::EMPTY);
            if (GetLastUndoInfo(nullptr, & nLastUndoId))
            {
                if (SwUndoId::INSSECTION != nLastUndoId)
                {
                    DoUndo(false);
                }
            }
            UpdateSection( GetSectionFormatPos( *pIns->GetFormat() ), aSection );
            DoUndo( bDoesUndo );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
