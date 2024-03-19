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

#include <string.h>

#include <wrtsh.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <charfmt.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editdata.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editview.hxx>
#include <editeng/langitem.hxx>

#include <svl/languageoptions.hxx>
#include <svtools/langtab.hxx>
#include <svl/slstitm.hxx>
#include <svl/stritem.hxx>
#include <svx/svxids.hrc>
#include <osl/diagnose.h>

#include <ndtxt.hxx>
#include <pam.hxx>
#include <view.hxx>
#include <viewopt.hxx>

#include <langhelper.hxx>

using namespace ::com::sun::star;

namespace SwLangHelper
{

    void GetLanguageStatus( OutlinerView* pOLV, SfxItemSet& rSet )
    {
        ESelection aSelection = pOLV->GetSelection();
        EditView& rEditView=pOLV->GetEditView();
        EditEngine& rEditEngine = rEditView.getEditEngine();

        // the value of used script types
        const SvtScriptType nScriptType =pOLV->GetSelectedScriptType();
        OUString aScriptTypesInUse( OUString::number( static_cast<int>(nScriptType) ) );//rEditEngine.GetScriptType(aSelection)

        // get keyboard language
        OUString aKeyboardLang;
        LanguageType nLang = rEditView.GetInputLanguage();
        if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
            aKeyboardLang = SvtLanguageTable::GetLanguageString( nLang );

        // get the language that is in use
        OUString aCurrentLang("*");
        SfxItemSet aSet(pOLV->GetAttribs());
        nLang = SwLangHelper::GetCurrentLanguage( aSet,nScriptType );
        if (nLang != LANGUAGE_DONTKNOW)
            aCurrentLang = SvtLanguageTable::GetLanguageString( nLang );

        // build sequence for status value
        uno::Sequence<OUString> aSeq{ aCurrentLang,
                                        aScriptTypesInUse,
                                        aKeyboardLang,
                                        SwLangHelper::GetTextForLanguageGuessing(&rEditEngine, aSelection)
        };

        // set sequence as status value
        SfxStringListItem aItem( SID_LANGUAGE_STATUS );
        aItem.SetStringList( aSeq );
        rSet.Put( aItem );
    }

    bool SetLanguageStatus( OutlinerView* pOLV, SfxRequest &rReq, SwView const &rView, SwWrtShell &rSh )
    {
        bool bRestoreSelection = false;
        ESelection   aSelection  = pOLV->GetSelection();
        EditView   & rEditView   = pOLV->GetEditView();
        SfxItemSet aEditAttr(rEditView.GetEmptyItemSet());

        // get the language
        OUString aNewLangText;

        const SfxStringItem* pItem = rReq.GetArg<SfxStringItem>(SID_LANGUAGE_STATUS);
        if (pItem)
            aNewLangText = pItem->GetValue();

        //!! Remember the view frame right now...
        //!! (call to GetView().GetViewFrame() will break if the
        //!! SwTextShell got destroyed meanwhile.)
        SfxViewFrame& rViewFrame = rView.GetViewFrame();

        if (aNewLangText == "*" )
        {
            // open the dialog "Tools/Options/Languages and Locales - General"
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateVclDialog( rView.GetFrameWeld(), SID_LANGUAGE_OPTIONS ));
            pDlg->Execute();
        }
        else
        {
            // setting the new language...
            if (!aNewLangText.isEmpty())
            {
                static constexpr OUString aSelectionLangPrefix(u"Current_"_ustr);
                static constexpr OUString aParagraphLangPrefix(u"Paragraph_"_ustr);
                static constexpr OUString aDocumentLangPrefix(u"Default_"_ustr);

                sal_Int32 nPos = 0;
                bool bForSelection = true;
                bool bForParagraph = false;
                if (-1 != (nPos = aNewLangText.indexOf( aSelectionLangPrefix )))
                {
                    // ... for the current selection
                    aNewLangText = aNewLangText.replaceAt(nPos, aSelectionLangPrefix.getLength(), u"");
                    bForSelection = true;
                }
                else if (-1 != (nPos = aNewLangText.indexOf( aParagraphLangPrefix )))
                {
                    // ... for the current paragraph language
                    aNewLangText = aNewLangText.replaceAt(nPos, aParagraphLangPrefix.getLength(), u"");
                    bForSelection = true;
                    bForParagraph = true;
                }
                else if (-1 != (nPos = aNewLangText.indexOf( aDocumentLangPrefix )))
                {
                    // ... as default document language
                    aNewLangText = aNewLangText.replaceAt(nPos, aDocumentLangPrefix.getLength(), u"");
                    bForSelection = false;
                }

                if (bForParagraph)
                {
                    bRestoreSelection = true;
                    SwLangHelper::SelectPara( rEditView, aSelection );
                    aSelection = pOLV->GetSelection();
                }
                if (!bForSelection) // document language to be changed...
                {
                    rSh.StartAction();
                    rSh.LockView( true );
                    rSh.Push();

                    // prepare to apply new language to all text in document
                    rSh.SelAll();
                    rSh.ExtendedSelectAll();
                }

                if (aNewLangText == "LANGUAGE_NONE")
                    SwLangHelper::SetLanguage_None( rSh, pOLV, aSelection, bForSelection, aEditAttr );
                else if (aNewLangText == "RESET_LANGUAGES")
                    SwLangHelper::ResetLanguages( rSh, pOLV );
                else
                    SwLangHelper::SetLanguage( rSh, pOLV, aSelection, aNewLangText, bForSelection, aEditAttr );

                if (!bForSelection)
                {
                    // need to release view and restore selection...
                    rSh.Pop(SwCursorShell::PopMode::DeleteCurrent);
                    rSh.LockView( false );
                    rSh.EndAction();
                }
            }
        }

        // invalidate slot to get the new language displayed
        rViewFrame.GetBindings().Invalidate( rReq.GetSlot() );

        rReq.Done();
        return bRestoreSelection;
    }

    void SetLanguage( SwWrtShell &rWrtSh, std::u16string_view rLangText, bool bIsForSelection, SfxItemSet &rCoreSet )
    {
        SetLanguage( rWrtSh, nullptr , ESelection(), rLangText, bIsForSelection, rCoreSet );
    }

    void SetLanguage( SwWrtShell &rWrtSh, OutlinerView const * pOLV, const ESelection& rSelection, std::u16string_view rLangText, bool bIsForSelection, SfxItemSet &rCoreSet )
    {
        const LanguageType nLang = SvtLanguageTable::GetLanguageType( rLangText );
        if (nLang == LANGUAGE_DONTKNOW)
            return;

        EditEngine* pEditEngine = pOLV ? &pOLV->GetEditView().getEditEngine() : nullptr;
        OSL_ENSURE( !pOLV || pEditEngine, "OutlinerView without EditEngine???" );

        //get ScriptType
        TypedWhichId<SvxLanguageItem> nLangWhichId(0);
        bool bIsSingleScriptType = true;
        switch (SvtLanguageOptions::GetScriptTypeOfLanguage( nLang ))
        {
            case SvtScriptType::LATIN :    nLangWhichId = pEditEngine ? EE_CHAR_LANGUAGE : RES_CHRATR_LANGUAGE; break;
            case SvtScriptType::ASIAN :    nLangWhichId = pEditEngine ? EE_CHAR_LANGUAGE_CJK : RES_CHRATR_CJK_LANGUAGE; break;
            case SvtScriptType::COMPLEX :  nLangWhichId = pEditEngine ? EE_CHAR_LANGUAGE_CTL : RES_CHRATR_CTL_LANGUAGE; break;
            default:
                bIsSingleScriptType = false;
                OSL_FAIL("unexpected case" );
        }
        if (!bIsSingleScriptType)
            return;

        // change language for selection or paragraph
        // (for paragraph is handled by previously having set the selection to the
        // whole paragraph)
        if (bIsForSelection)
        {
            // apply language to current selection
            if (pEditEngine)
            {
                rCoreSet.Put( SvxLanguageItem( nLang, nLangWhichId ));
                pEditEngine->QuickSetAttribs(rCoreSet, rSelection);
            }
            else
            {
                rWrtSh.GetCurAttr( rCoreSet );
                rCoreSet.Put( SvxLanguageItem( nLang, nLangWhichId ));
                rWrtSh.SetAttrSet( rCoreSet );
            }
        }
        else // change language for all text
        {
            // set document default language
            switch (nLangWhichId)
            {
                 case EE_CHAR_LANGUAGE :      nLangWhichId = RES_CHRATR_LANGUAGE; break;
                 case EE_CHAR_LANGUAGE_CJK :  nLangWhichId = RES_CHRATR_CJK_LANGUAGE; break;
                 case EE_CHAR_LANGUAGE_CTL :  nLangWhichId = RES_CHRATR_CTL_LANGUAGE; break;
            }
            //Set the default document language
            rWrtSh.SetDefault( SvxLanguageItem( nLang, nLangWhichId ) );
            rWrtSh.GetDoc()->GetDocShell()->Broadcast(SfxHint(SfxHintId::LanguageChanged));

            // #i102191: hard set respective language attribute in text document
            // (for all text in the document - which should be selected by now...)
            rWrtSh.SetAttrItem( SvxLanguageItem( nLang, nLangWhichId ) );
        }
    }

    void SetLanguage_None( SwWrtShell &rWrtSh, bool bIsForSelection, SfxItemSet &rCoreSet )
    {
        SetLanguage_None( rWrtSh,nullptr,ESelection(),bIsForSelection,rCoreSet );
    }

    void SetLanguage_None( SwWrtShell &rWrtSh, OutlinerView const * pOLV, const ESelection& rSelection, bool bIsForSelection, SfxItemSet &rCoreSet )
    {
        // EditEngine IDs
        const sal_uInt16 aLangWhichId_EE[3] =
        {
            EE_CHAR_LANGUAGE,
            EE_CHAR_LANGUAGE_CJK,
            EE_CHAR_LANGUAGE_CTL
        };

        // Writer IDs
        const sal_uInt16 aLangWhichId_Writer[3] =
        {
            RES_CHRATR_LANGUAGE,
            RES_CHRATR_CJK_LANGUAGE,
            RES_CHRATR_CTL_LANGUAGE
        };

        if (bIsForSelection)
        {
            // change language for selection or paragraph
            // (for paragraph is handled by previously having set the selection to the
            // whole paragraph)

            EditEngine* pEditEngine = pOLV ? &pOLV->GetEditView().getEditEngine() : nullptr;
            OSL_ENSURE( !pOLV || pEditEngine, "OutlinerView without EditEngine???" );
            if (pEditEngine)
            {
                for (sal_uInt16 i : aLangWhichId_EE)
                    rCoreSet.Put( SvxLanguageItem( LANGUAGE_NONE, i ));
                pEditEngine->QuickSetAttribs(rCoreSet, rSelection);
            }
            else
            {
                rWrtSh.GetCurAttr( rCoreSet );
                for (sal_uInt16 i : aLangWhichId_Writer)
                    rCoreSet.Put( SvxLanguageItem( LANGUAGE_NONE, i ));
                rWrtSh.SetAttrSet( rCoreSet );
            }
        }
        else // change language for all text
        {
            o3tl::sorted_vector<sal_uInt16> aAttribs;
            for (sal_uInt16 i : aLangWhichId_Writer)
            {
                rWrtSh.SetDefault( SvxLanguageItem( LANGUAGE_NONE, i ) );
                aAttribs.insert( i );
            }
            rWrtSh.GetDoc()->GetDocShell()->Broadcast(SfxHint(SfxHintId::LanguageChanged));

            // set all language attributes to default
            // (for all text in the document - which should be selected by now...)
            rWrtSh.ResetAttr( aAttribs );
        }
    }

    void ResetLanguages( SwWrtShell &rWrtSh, OutlinerView const * pOLV )
    {
        // reset language for current selection.
        // The selection should already have been expanded to the whole paragraph or
        // to all text in the document if those are the ranges where to reset
        // the language attributes

        if (pOLV)
        {
            EditView &rEditView = pOLV->GetEditView();
            rEditView.RemoveAttribs( true, EE_CHAR_LANGUAGE );
            rEditView.RemoveAttribs( true, EE_CHAR_LANGUAGE_CJK );
            rEditView.RemoveAttribs( true, EE_CHAR_LANGUAGE_CTL );

            // ugly hack, as it seems that EditView/EditEngine does not update their spellchecking marks
            // when setting a new language attribute
            EditEngine& rEditEngine = rEditView.getEditEngine();
            EEControlBits nCntrl = rEditEngine.GetControlWord();
            // turn off
            rEditEngine.SetControlWord(nCntrl & ~EEControlBits::ONLINESPELLING);
            //turn back on
            rEditEngine.SetControlWord(nCntrl);
            rEditEngine.CompleteOnlineSpelling();

            rEditView.Invalidate();
        }
        else
        {
            rWrtSh.ResetAttr(
                { RES_CHRATR_LANGUAGE, RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CTL_LANGUAGE });
        }
    }

    /// @returns : the language for the selected text that is set for the
    ///     specified attribute (script type).
    ///     If there are more than one languages used LANGUAGE_DONTKNOW will be returned.
    /// @param nLangWhichId : one of
    ///     RES_CHRATR_LANGUAGE, RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CTL_LANGUAGE,
    LanguageType GetLanguage( SwWrtShell &rSh, TypedWhichId<SvxLanguageItem> nLangWhichId )
    {
        SfxItemSet aSet( rSh.GetAttrPool(), nLangWhichId, nLangWhichId );
        rSh.GetCurAttr( aSet );

        return GetLanguage(aSet,nLangWhichId);
    }

    LanguageType GetLanguage( SfxItemSet const & aSet, TypedWhichId<SvxLanguageItem> nLangWhichId )
    {

        LanguageType nLang = LANGUAGE_SYSTEM;

        const SvxLanguageItem *pItem = nullptr;
        SfxItemState nState = aSet.GetItemState( nLangWhichId, true, &pItem );
        if (nState > SfxItemState::DEFAULT && pItem)
        {
            // the item is set and can be used
            nLang = pItem->GetLanguage();
        }
        else if (nState == SfxItemState::DEFAULT)
        {
            // since the attribute is not set: retrieve the default value
            nLang = aSet.GetPool()->GetUserOrPoolDefaultItem( nLangWhichId ).GetLanguage();
        }
        else if (nState == SfxItemState::INVALID)
        {
            // there is more than one language...
            nLang = LANGUAGE_DONTKNOW;
        }
        OSL_ENSURE( nLang != LANGUAGE_SYSTEM, "failed to get the language?" );

        return nLang;
    }

    /// @returns: the language in use for the selected text.
    ///     'In use' means the language(s) matching the script type(s) of the
    ///     selected text. Or in other words, the language a spell checker would use.
    ///     If there is more than one language LANGUAGE_DONTKNOW will be returned.
    LanguageType GetCurrentLanguage( SwWrtShell &rSh )
    {
        //set language attribute to use according to the script type
        TypedWhichId<SvxLanguageItem> nLangWhichId(0);
        bool bIsSingleScriptType = true;
        switch (rSh.GetScriptType())
        {
             case SvtScriptType::LATIN :    nLangWhichId = RES_CHRATR_LANGUAGE; break;
             case SvtScriptType::ASIAN :    nLangWhichId = RES_CHRATR_CJK_LANGUAGE; break;
             case SvtScriptType::COMPLEX :  nLangWhichId = RES_CHRATR_CTL_LANGUAGE; break;
             default: bIsSingleScriptType = false; break;
        }

        // get language according to the script type(s) in use
        LanguageType nCurrentLang = LANGUAGE_SYSTEM;
        if (bIsSingleScriptType)
            nCurrentLang = GetLanguage( rSh, nLangWhichId );
        else
        {
            // check if all script types are set to LANGUAGE_NONE and return
            // that if this is the case. Otherwise, having multiple script types
            // in use always means there are several languages in use...
            const TypedWhichId<SvxLanguageItem> aScriptTypes[3] =
            {
                RES_CHRATR_LANGUAGE,
                RES_CHRATR_CJK_LANGUAGE,
                RES_CHRATR_CTL_LANGUAGE
            };
            nCurrentLang = LANGUAGE_NONE;
            for (const TypedWhichId<SvxLanguageItem>& aScriptType : aScriptTypes)
            {
                LanguageType nTmpLang = GetLanguage( rSh, aScriptType );
                if (nTmpLang != LANGUAGE_NONE)
                {
                    nCurrentLang = LANGUAGE_DONTKNOW;
                    break;
                }
            }
        }
        OSL_ENSURE( nCurrentLang != LANGUAGE_SYSTEM, "failed to get the language?" );

        return nCurrentLang;
    }

    /// @returns: the language in use for the selected text.
    ///     'In use' means the language(s) matching the script type(s) of the
    ///     selected text. Or in other words, the language a spell checker would use.
    ///     If there is more than one language LANGUAGE_DONTKNOW will be returned.
    LanguageType GetCurrentLanguage( SfxItemSet const & aSet, SvtScriptType nScriptType )
    {
        //set language attribute to use according to the script type
        TypedWhichId<SvxLanguageItem> nLangWhichId(0);
        bool bIsSingleScriptType = true;
        switch (nScriptType)
        {
             case SvtScriptType::LATIN :    nLangWhichId = EE_CHAR_LANGUAGE; break;
             case SvtScriptType::ASIAN :    nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
             case SvtScriptType::COMPLEX :  nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
             default: bIsSingleScriptType = false;
        }

        // get language according to the script type(s) in use
        LanguageType nCurrentLang = LANGUAGE_SYSTEM;
        if (bIsSingleScriptType)
            nCurrentLang = GetLanguage( aSet, nLangWhichId );
        else
        {
            // check if all script types are set to LANGUAGE_NONE and return
            // that if this is the case. Otherwise, having multiple script types
            // in use always means there are several languages in use...
            const TypedWhichId<SvxLanguageItem> aScriptTypes[3] =
            {
                EE_CHAR_LANGUAGE,
                EE_CHAR_LANGUAGE_CJK,
                EE_CHAR_LANGUAGE_CTL
            };
            nCurrentLang = LANGUAGE_NONE;
            for (const TypedWhichId<SvxLanguageItem>& aScriptType : aScriptTypes)
            {
                LanguageType nTmpLang = GetLanguage( aSet, aScriptType );
                if (nTmpLang != LANGUAGE_NONE)
                {
                    nCurrentLang = LANGUAGE_DONTKNOW;
                    break;
                }
            }
        }
        OSL_ENSURE( nCurrentLang != LANGUAGE_SYSTEM, "failed to get the language?" );

        return nCurrentLang;
    }

    OUString GetTextForLanguageGuessing( SwWrtShell const &rSh )
    {
        // string for guessing language
        OUString aText;
        SwPaM *pCursor = rSh.GetCursor();
        SwTextNode *pNode = pCursor->GetPointNode().GetTextNode();
        if (pNode)
        {
            aText = pNode->GetText();
            if (!aText.isEmpty())
            {
                sal_Int32 nEnd = pCursor->GetPoint()->GetContentIndex();
                // at most 100 chars to the left...
                const sal_Int32 nStt = nEnd > 100 ? nEnd - 100 : 0;
                // ... and 100 to the right of the cursor position
                nEnd = aText.getLength() - nEnd > 100 ? nEnd + 100 : aText.getLength();
                aText = aText.copy( nStt, nEnd - nStt );
            }
        }
        return aText;
    }

    OUString GetTextForLanguageGuessing(EditEngine const * rEditEngine, const ESelection& rDocSelection)
    {
        // string for guessing language

        // get the full text of the paragraph that the end of selection is in
        OUString aText = rEditEngine->GetText(rDocSelection.nEndPos);
        if (!aText.isEmpty())
        {
            sal_Int32 nStt = 0;
            sal_Int32 nEnd = rDocSelection.nEndPos;
            // at most 100 chars to the left...
            nStt = nEnd > 100 ? nEnd - 100 : 0;
            // ... and 100 to the right of the cursor position
            nEnd = aText.getLength() - nEnd > 100 ? nEnd + 100 : aText.getLength();
            aText = aText.copy( nStt, nEnd - nStt );
        }

        return aText;
    }

    void SelectPara( EditView &rEditView, const ESelection &rCurSel )
    {
        ESelection aParaSel( rCurSel.nStartPara, 0, rCurSel.nStartPara, EE_TEXTPOS_ALL );
        rEditView.SetSelection( aParaSel );
    }

    void SelectCurrentPara( SwWrtShell &rWrtSh )
    {
        // select current para
        if (!rWrtSh.IsSttPara())
            rWrtSh.MovePara( GoCurrPara, fnParaStart );
        if (!rWrtSh.HasMark())
            rWrtSh.SetMark();
        rWrtSh.SwapPam();
        if (!rWrtSh.IsEndPara())
            rWrtSh.MovePara( GoCurrPara, fnParaEnd );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
