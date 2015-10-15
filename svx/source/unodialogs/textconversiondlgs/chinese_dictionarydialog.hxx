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

#ifndef INCLUDED_SVX_SOURCE_UNODIALOGS_TEXTCONVERSIONDLGS_CHINESE_DICTIONARYDIALOG_HXX
#define INCLUDED_SVX_SOURCE_UNODIALOGS_TEXTCONVERSIONDLGS_CHINESE_DICTIONARYDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/simptabl.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>

#include <vector>


namespace textconversiondlgs
{



/**
*/

struct DictionaryEntry
{
    DictionaryEntry( const OUString& rTerm, const OUString& rMapping
                    , sal_Int16 nConversionPropertyType //linguistic2::ConversionPropertyType
                    , bool bNewEntry = false );

    virtual ~DictionaryEntry();

    bool operator==( const DictionaryEntry& rE ) const;

    OUString m_aTerm;
    OUString m_aMapping;
    sal_Int16     m_nConversionPropertyType; //linguistic2::ConversionPropertyType

    bool     m_bNewEntry;
};

class DictionaryList : public SvSimpleTable
{
public:
    DictionaryList(SvSimpleTableContainer& rParent, WinBits nBits);
    virtual ~DictionaryList();
    virtual void dispose() override;

    void init(const css::uno::Reference< css::linguistic2::XConversionDictionary>& xDictionary,
        vcl::Window *pED_Term, vcl::Window *pED_Mapping, ListBox *pLB_Property,
        vcl::Window *pFT_Term, vcl::Window *pFT_Mapping, vcl::Window *pFT_Property);

    void deleteAll();
    void refillFromDictionary( sal_Int32 nTextConversionOptions /*i18n::TextConversionOption*/ );
    void save();

    DictionaryEntry* getTermEntry( const OUString& rTerm ) const;
    bool hasTerm( const OUString& rTerm ) const;

    void addEntry( const OUString& rTerm, const OUString& rMapping
            , sal_Int16 nConversionPropertyType /*linguistic2::ConversionPropertyType*/, sal_uIntPtr nPos = TREELIST_APPEND );
    sal_uIntPtr deleteEntries( const OUString& rTerm ); //return lowest position of deleted entries or LIST_APPEND if no entry was deleted
    void deleteEntryOnPos( sal_Int32 nPos  );
    DictionaryEntry* getEntryOnPos( sal_Int32 nPos ) const;
    DictionaryEntry* getFirstSelectedEntry() const;

    void sortByColumn( sal_uInt16 nSortColumnIndex, bool bSortAtoZ );
    sal_uInt16 getSortColumn() const { return m_nSortColumnIndex;}

private:
    OUString getPropertyTypeName( sal_Int16 nConversionPropertyType /*linguistic2::ConversionPropertyType*/ ) const;
    OUString makeTabString( const DictionaryEntry& rEntry ) const;

    DECL_LINK_TYPED( CompareHdl, const SvSortData&, sal_Int32 );
    sal_Int32 ColumnCompare( SvTreeListEntry* pLeft, SvTreeListEntry* pRight );
    SvLBoxItem* getItemAtColumn( SvTreeListEntry* pEntry, sal_uInt16 nColumn ) const;

    void setColSizes();

    virtual void Resize() override;

public:
    css::uno::Reference<css::linguistic2::XConversionDictionary>  m_xDictionary;

private:
    VclPtr<vcl::Window>     m_pED_Term;
    VclPtr<vcl::Window>     m_pED_Mapping;
    VclPtr<ListBox>         m_pLB_Property;

    std::vector< DictionaryEntry* > m_aToBeDeleted;

    sal_uInt16      m_nSortColumnIndex;
};

class ChineseDictionaryDialog : public ModalDialog
{
public:
    explicit ChineseDictionaryDialog( vcl::Window* pParent );
    virtual ~ChineseDictionaryDialog();
    virtual void dispose() override;

    //this method should be called once before calling execute
    void setDirectionAndTextConversionOptions( bool bDirectionToSimplified, sal_Int32 nTextConversionOptions /*i18n::TextConversionOption*/ );

    virtual short   Execute() override;

private:
    DECL_LINK_TYPED( DirectionHdl, Button*, void );
    DECL_LINK_TYPED( EditFieldsHdl, Edit&, void );
    DECL_LINK_TYPED( EditFieldsListBoxHdl, ListBox&, void );
    DECL_LINK_TYPED( MappingSelectHdl, SvTreeListBox*, void );
    DECL_LINK_TYPED( AddHdl, Button*, void );
    DECL_LINK_TYPED( ModifyHdl, Button*, void );
    DECL_LINK_TYPED( DeleteHdl, Button*, void );
    DECL_LINK_TYPED( HeaderBarClick, HeaderBar*, void );

    void initDictionaryControl(DictionaryList *pList,
        const css::uno::Reference< css::linguistic2::XConversionDictionary>& xDictionary);

    void updateAfterDirectionChange();
    void updateButtons();

    bool isEditFieldsHaveContent() const;
    bool isEditFieldsContentEqualsSelectedListContent() const;

    DictionaryList& getActiveDictionary();
    DictionaryList& getReverseDictionary();

    const DictionaryList& getActiveDictionary() const;
    const DictionaryList& getReverseDictionary() const;

private:
    sal_Int32    m_nTextConversionOptions; //i18n::TextConversionOption

    VclPtr<RadioButton> m_pRB_To_Simplified;
    VclPtr<RadioButton> m_pRB_To_Traditional;

    VclPtr<CheckBox>    m_pCB_Reverse;

    VclPtr<FixedText>   m_pFT_Term;
    VclPtr<Edit>        m_pED_Term;

    VclPtr<FixedText>   m_pFT_Mapping;
    VclPtr<Edit>        m_pED_Mapping;

    VclPtr<FixedText>   m_pFT_Property;
    VclPtr<ListBox>     m_pLB_Property;

    VclPtr<SvSimpleTableContainer> mpToSimplifiedContainer;
    VclPtr<DictionaryList>         m_pCT_DictionaryToSimplified;
    VclPtr<SvSimpleTableContainer> mpToTraditionalContainer;
    VclPtr<DictionaryList>         m_pCT_DictionaryToTraditional;

    VclPtr<PushButton>  m_pPB_Add;
    VclPtr<PushButton>  m_pPB_Modify;
    VclPtr<PushButton>  m_pPB_Delete;

    css::uno::Reference<css::uno::XComponentContext> m_xContext;
};


} //end namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
