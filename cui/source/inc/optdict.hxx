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
#ifndef _SVX_OPTDICT_HXX
#define _SVX_OPTDICT_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/combobox.hxx>
#include <vcl/timer.hxx>
#include <vcl/edit.hxx>
#include <vcl/decoview.hxx>
#include <com/sun/star/util/Language.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>


#include <svx/simptabl.hxx>
#include <svx/langbox.hxx>

namespace com{namespace sun{namespace star{
namespace linguistic2{
    class XDictionary;
    class XSpellChecker1;
}}}}

// forward ---------------------------------------------------------------


// class SvxNewDictionaryDialog ------------------------------------------

class SvxNewDictionaryDialog : public ModalDialog
{
private:
    Edit*                pNameEdit;
    SvxLanguageBox*      pLanguageLB;
    CheckBox*            pExceptBtn;
    OKButton*            pOKBtn;
    CancelButton*        pCancelBtn;
    HelpButton*          pHelpBtn;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >     xSpell;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    xNewDic;

    DECL_LINK(OKHdl_Impl, void *);
    DECL_LINK(ModifyHdl_Impl, void *);

public:
    SvxNewDictionaryDialog( Window* pParent,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XSpellChecker1 >  &xSpl );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >
                GetNewDictionary() { return xNewDic; }
};

// class SvxDictEdit ----------------------------------------------------

class SvxDictEdit : public Edit
{
    Link    aActionLink;
    sal_Bool    bSpaces;

    public:
                    SvxDictEdit(Window* pParent, const ResId& rResId) :
                        Edit(pParent, rResId), bSpaces(sal_False){}

    void            SetActionHdl( const Link& rLink )
                                { aActionLink = rLink;}

    void            SetSpaces(sal_Bool bSet)
                                {bSpaces = bSet;}

    virtual void    KeyInput( const KeyEvent& rKEvent );
};

// class SvxEditDictionaryDialog -----------------------------------------

class SvxEditDictionaryDialog : public ModalDialog
{
private:

    FixedText               aBookFT;
    ListBox                 aAllDictsLB;
    FixedText               aLangFT;
    SvxLanguageBox          aLangLB;

    FixedText               aWordFT;
    SvxDictEdit             aWordED;
    FixedText               aReplaceFT;
    SvxDictEdit             aReplaceED;
    SvTabListBox            aWordsLB;
    PushButton              aNewReplacePB;
    PushButton              aDeletePB;
    FixedLine                aEditDictsBox;

    HelpButton              aHelpBtn;
    CancelButton            aCloseBtn;
    String                  sModify;
    String                  sNew;
    DecorationView          aDecoView;

    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XDictionary >  > aDics;  //! snapshot copy to work on
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >     xSpell;

    short               nOld;
    long                nWidth;
    sal_Bool            bFirstSelect;
    sal_Bool            bDoNothing;
    sal_Bool                bDicIsReadonly;

    DECL_LINK(SelectBookHdl_Impl, void *);
    DECL_LINK(SelectLangHdl_Impl, void *);
    DECL_LINK(SelectHdl, SvTabListBox*);
    DECL_LINK(NewDelHdl, PushButton*);
    DECL_LINK(ModifyHdl, Edit*);


    void            ShowWords_Impl( sal_uInt16 nId );
    void            SetLanguage_Impl( ::com::sun::star::util::Language nLanguage );
    sal_Bool            IsDicReadonly_Impl() const { return bDicIsReadonly; }
    void            SetDicReadonly_Impl( ::com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XDictionary >  &xDic );

    void            RemoveDictEntry(SvTreeListEntry* pEntry);
    sal_uInt16          GetLBInsertPos(const String &rDicWord);

protected:

    virtual void    Paint( const Rectangle& rRect );

public:
    SvxEditDictionaryDialog( Window* pParent,
            const String& rName,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XSpellChecker1> &xSpl );
    ~SvxEditDictionaryDialog();

    sal_uInt16 GetSelectedDict() {return aAllDictsLB.GetSelectEntryPos();}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
