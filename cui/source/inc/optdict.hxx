/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    FixedLine            aNewDictBox;
    FixedText           aNameText;
    Edit                aNameEdit;
    FixedText           aLanguageText;
    SvxLanguageBox      aLanguageLB;
    CheckBox            aExceptBtn;
    OKButton            aOKBtn;
    CancelButton        aCancelBtn;
    HelpButton          aHelpBtn;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >     xSpell;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    xNewDic;

#ifdef _SVX_OPTDICT_CXX
    DECL_LINK(OKHdl_Impl, void *);
    DECL_LINK(ModifyHdl_Impl, void *);
#endif

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

#ifdef _SVX_OPTDICT_CXX
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

    void            RemoveDictEntry(SvLBoxEntry* pEntry);
    sal_uInt16          GetLBInsertPos(const String &rDicWord);

#endif

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
