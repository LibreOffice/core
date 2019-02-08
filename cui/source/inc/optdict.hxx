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
#ifndef INCLUDED_CUI_SOURCE_INC_OPTDICT_HXX
#define INCLUDED_CUI_SOURCE_INC_OPTDICT_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/timer.hxx>
#include <vcl/edit.hxx>
#include <vcl/decoview.hxx>
#include <com/sun/star/util/Language.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>


#include <svtools/simptabl.hxx>
#include <svx/langbox.hxx>

namespace com{namespace sun{namespace star{
namespace linguistic2{
    class XDictionary;
    class XSpellChecker1;
}}}}

// forward ---------------------------------------------------------------


// class SvxNewDictionaryDialog ------------------------------------------

class SvxNewDictionaryDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xNameEdit;
    std::unique_ptr<LanguageBox> m_xLanguageLB;
    std::unique_ptr<weld::CheckButton> m_xExceptBtn;
    std::unique_ptr<weld::Button> m_xOKBtn;
    css::uno::Reference<css::linguistic2::XDictionary> m_xNewDic;

    DECL_LINK(OKHdl_Impl, weld::Button&, void);
    DECL_LINK(ModifyHdl_Impl, weld::Entry&, void);

public:
    SvxNewDictionaryDialog(weld::Window* pParent);

    const css::uno::Reference<css::linguistic2::XDictionary>& GetNewDictionary() { return m_xNewDic; }
};

// class SvxEditDictionaryDialog -----------------------------------------

class SvxEditDictionaryDialog : public weld::GenericDialogController
{
private:
    OUString                sModify;
    OUString                sNew;
    OUString                sReplaceFT_Text;

    css::uno::Sequence<
        css::uno::Reference<
            css::linguistic2::XDictionary >  > aDics;  //! snapshot copy to work on

    bool            bFirstSelect;
    bool            bDoNothing;
    bool            bDicIsReadonly;

    weld::TreeView* m_pWordsLB;
    std::unique_ptr<weld::ComboBox> m_xAllDictsLB;
    std::unique_ptr<weld::Label> m_xLangFT;
    std::unique_ptr<LanguageBox> m_xLangLB;
    std::unique_ptr<weld::Entry> m_xWordED;
    std::unique_ptr<weld::Label> m_xReplaceFT;
    std::unique_ptr<weld::Entry> m_xReplaceED;
    std::unique_ptr<weld::TreeView> m_xSingleColumnLB;
    std::unique_ptr<weld::TreeView> m_xDoubleColumnLB;
    std::unique_ptr<weld::Button> m_xNewReplacePB;
    std::unique_ptr<weld::Button> m_xDeletePB;

    DECL_LINK(SelectBookHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(SelectLangHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(NewDelButtonHdl, weld::Button&, void);
    DECL_LINK(NewDelActionHdl, weld::Entry&, bool);
    DECL_LINK(ModifyHdl, weld::Entry&, void);
    DECL_LINK(EntrySizeAllocHdl, const Size&, void);
    DECL_STATIC_LINK(SvxEditDictionaryDialog, InsertTextHdl, OUString&, bool);
    bool NewDelHdl(const weld::Widget*);

    void            ShowWords_Impl( sal_uInt16 nId );
    void            SetLanguage_Impl( LanguageType nLanguage );
    bool            IsDicReadonly_Impl() const { return bDicIsReadonly; }
    void            SetDicReadonly_Impl( css::uno::Reference<
                            css::linguistic2::XDictionary > const &xDic );

    void            RemoveDictEntry(int nEntry);
    int             GetLBInsertPos(const OUString &rDicWord);

public:
    SvxEditDictionaryDialog(weld::Window* pParent, const OUString& rName);
    virtual ~SvxEditDictionaryDialog() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
