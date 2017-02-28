/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTABOUTCONFIG_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTABOUTCONFIG_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <i18nutil/searchopt.hxx>
#include <sfx2/tabdlg.hxx>
#include <svtools/simptabl.hxx>
#include <vcl/edit.hxx>

#include <vector>

namespace svx { class OptHeaderTabListBox; }
class CuiAboutConfigTabPage;
class CuiAboutConfigValueDialog;
struct Prop_Impl;

class CuiCustomMultilineEdit : public Edit
{
public:
    bool bNumericOnly;
    CuiCustomMultilineEdit( vcl::Window* pParent, WinBits nStyle )
        : Edit( pParent, nStyle )
        , bNumericOnly(false)
    {}

    virtual void KeyInput( const KeyEvent& rKeyEvent ) override;
    virtual Size GetOptimalSize() const override;
};

class CuiAboutConfigTabPage : public ModelessDialog
{
private:
    VclPtr<SvSimpleTableContainer> m_pPrefCtrl;
    VclPtr<PushButton> m_pResetBtn;
    VclPtr<PushButton> m_pEditBtn;
    VclPtr<PushButton> m_pSearchBtn;
    VclPtr<Edit> m_pSearchEdit;

    SvTreeListEntries m_modifiedPrefBoxEntries;
    std::vector< std::shared_ptr< Prop_Impl > > m_vectorOfModified;
    VclPtr< SvSimpleTable > m_pPrefBox;

     //for search
    i18nutil::SearchOptions2 m_options;
    SvTreeListEntries m_prefBoxEntries;

    void AddToModifiedVector( const std::shared_ptr< Prop_Impl >& rProp );
    static std::vector< OUString > commaStringToSequence( const OUString& rCommaSepString );
    void InsertEntry( SvTreeListEntry *pEntry);

    DECL_LINK( StandardHdl_Impl, Button*, void );
    DECL_LINK( DoubleClickHdl_Impl, SvTreeListBox*, bool );
    DECL_LINK( ResetBtnHdl_Impl, Button*, void );
    DECL_LINK( SearchHdl_Impl, Button*, void );
    DECL_LINK( ExpandingHdl_Impl, SvTreeListBox*, bool );

public:
   explicit CuiAboutConfigTabPage(vcl::Window* pParent);
   virtual ~CuiAboutConfigTabPage() override;
   virtual void dispose() override;
   void     InsertEntry(const OUString &rPropertyPath, const OUString& rProp, const OUString& rStatus, const OUString& rType, const OUString& rValue,
                        SvTreeListEntry *pParentEntry, bool bInsertToPrefBox);
   void     Reset();
   void     FillItems(const css::uno::Reference<css::container::XNameAccess>& xNameAccess,
                      SvTreeListEntry *pParentEntry = nullptr, int lineage = 0, bool bLoadAll = false);
   static css::uno::Reference< css::container::XNameAccess > getConfigAccess( const OUString& sNodePath, bool bUpdate );
   void FillItemSet();
};

class CuiAboutConfigValueDialog : public ModalDialog
{
private:
    VclPtr<CuiCustomMultilineEdit> m_pEDValue;

public:
    CuiAboutConfigValueDialog( vcl::Window* pWindow, const OUString& rValue , int limit = 0);
    virtual ~CuiAboutConfigValueDialog() override;
    virtual void dispose() override;

    OUString getValue()
    {
        return m_pEDValue->GetText();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
