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
#ifndef _SVX_OPTLINGU_HXX
#define _SVX_OPTLINGU_HXX

#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/field.hxx>
#include <svtools/fixedhyper.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/checklbx.hxx>
#include <svx/langbox.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace com{namespace sun{namespace star{
namespace beans{
    class XPropertySet;
}
namespace linguistic2{
    class XDictionary;
    class XDictionaryList;
}}}}

class SvTreeListEntry;
class SvxLinguData_Impl;

// define ----------------------------------------------------------------

#define GROUP_MODULES   ((sal_uInt16)0x0008)

// forward ---------------------------------------------------------------

class SvxEditModulesDlg : public ModalDialog
{
    FixedLine           aModulesFL;
    FixedText           aLanguageFT;
    SvxLanguageBox      aLanguageLB;

    SvxCheckListBox     aModulesCLB;
    PushButton          aPrioUpPB;
    PushButton          aPrioDownPB;
    PushButton          aBackPB;
    svt::FixedHyperlink aMoreDictsLink;

    FixedLine           aButtonsFL;
    HelpButton          aHelpPB;
    OKButton            aClosePB;

    String              sSpell;
    String              sHyph;
    String              sThes;
    String              sGrammar;

    SvxLinguData_Impl*  pDefaultLinguData;
    SvxLinguData_Impl&  rLinguData;

    SvLBoxButtonData*   pCheckButtonData;

    SvTreeListEntry*    CreateEntry(String& rTxt, sal_uInt16 nCol);

#ifdef _SVX_OPTLINGU_CXX
    DECL_LINK( SelectHdl_Impl, SvxCheckListBox * );
    DECL_LINK( UpDownHdl_Impl, PushButton * );
    DECL_LINK( ClickHdl_Impl, PushButton * );
    DECL_LINK(BackHdl_Impl, void *);
    DECL_LINK( SelectHdlLB_Impl, ListBox * );
    DECL_LINK( LangSelectHdl_Impl, ListBox* );
    DECL_LINK( BoxCheckButtonHdl_Impl, SvTreeListBox * );
    DECL_LINK(OpenURLHdl_Impl, void *);
#endif

public:
    SvxEditModulesDlg(Window* pParent, SvxLinguData_Impl& rData);
    virtual ~SvxEditModulesDlg();
};

// class SvxLinguTabPage -------------------------------------------------

class SvxLinguTabPage : public SfxTabPage
{
private:
    FixedLine           aLinguisticFL;
    FixedText           aLinguModulesFT;
    SvxCheckListBox     aLinguModulesCLB;
    PushButton          aLinguModulesEditPB;
    FixedText           aLinguDicsFT;
    SvxCheckListBox     aLinguDicsCLB;
    PushButton          aLinguDicsNewPB;
    PushButton          aLinguDicsEditPB;
    PushButton          aLinguDicsDelPB;
    FixedText           aLinguOptionsFT;
    SvxCheckListBox     aLinguOptionsCLB;
    PushButton          aLinguOptionsEditPB;
    svt::FixedHyperlink aMoreDictsLink;

    String              sCapitalWords;
    String              sWordsWithDigits;
    String              sCapitalization;
    String              sSpellSpecial;
    String              sSpellAuto;
    String              sGrammarAuto;
    String              sNumMinWordlen;
    String              sNumPreBreak;
    String              sNumPostBreak;
    String              sHyphAuto;
    String              sHyphSpecial;

    com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertySet >   xProp;

    com::sun::star::uno::Reference<
        com::sun::star::linguistic2::XDictionaryList >      xDicList;
    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::linguistic2::XDictionary > >    aDics;

    SvLBoxButtonData*   pCheckButtonData;

    SvxLinguData_Impl*  pLinguData;

    SvxLinguTabPage( Window* pParent, const SfxItemSet& rCoreSet );
    SvTreeListEntry*    CreateEntry(String& rTxt, sal_uInt16 nCol);

    void    AddDicBoxEntry( const com::sun::star::uno::Reference< com::sun::star::linguistic2::XDictionary > &rxDic, sal_uInt16 nIdx );
    sal_uLong   GetDicUserData( const com::sun::star::uno::Reference< com::sun::star::linguistic2::XDictionary > &rxDic, sal_uInt16 nIdx );

#ifdef _SVX_OPTLINGU_CXX
    DECL_LINK( SelectHdl_Impl, SvxCheckListBox * );
    DECL_LINK( ClickHdl_Impl, PushButton * );
    DECL_LINK( BoxDoubleClickHdl_Impl, SvTreeListBox * );
    DECL_LINK( BoxCheckButtonHdl_Impl, SvTreeListBox * );
    DECL_LINK(PostDblClickHdl_Impl, void *);
    DECL_LINK(OpenURLHdl_Impl, void *);

    void                UpdateModulesBox_Impl();
    void                UpdateDicBox_Impl();
#endif

public:
    virtual             ~SvxLinguTabPage();
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*  GetRanges();

    virtual sal_Bool    FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                HideGroups( sal_uInt16 nGrp );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
