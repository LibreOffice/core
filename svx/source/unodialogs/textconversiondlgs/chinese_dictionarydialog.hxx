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
// header for class FixedLine
#include <vcl/fixed.hxx>
// header for class RadioButton
#include <vcl/button.hxx>
// header for class Edit
#include <vcl/edit.hxx>
// header for class ListBox
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

    void init(const css::uno::Reference< css::linguistic2::XConversionDictionary>& xDictionary,
        Window *pED_Term, Window *pED_Mapping, ListBox *pLB_Property,
        Window *pFT_Term, Window *pFT_Mapping, Window *pFT_Property);

    void deleteAll();
    void refillFromDictionary( sal_Int32 nTextConversionOptions /*i18n::TextConversionOption*/ );
    void save();

    DictionaryEntry* getTermEntry( const OUString& rTerm ) const;
    bool hasTerm( const OUString& rTerm ) const;

    void addEntry( const OUString& rTerm, const OUString& rMapping
            , sal_Int16 nConversionPropertyType /*linguistic2::ConversionPropertyType*/, sal_uIntPtr nPos = LIST_APPEND );
    sal_uIntPtr deleteEntries( const OUString& rTerm ); //return lowest position of deleted entries or LIST_APPEND if no entry was deleted
    void deleteEntryOnPos( sal_Int32 nPos  );
    DictionaryEntry* getEntryOnPos( sal_Int32 nPos ) const;
    DictionaryEntry* getFirstSelectedEntry() const;

    void sortByColumn( sal_uInt16 nSortColumnIndex, bool bSortAtoZ );
    sal_uInt16 getSortColumn() const;

private:
    OUString getPropertyTypeName( sal_Int16 nConversionPropertyType /*linguistic2::ConversionPropertyType*/ ) const;
    OUString makeTabString( const DictionaryEntry& rEntry ) const;

    DECL_LINK( CompareHdl, SvSortData* );
    sal_Int32 ColumnCompare( SvTreeListEntry* pLeft, SvTreeListEntry* pRight );
    SvLBoxItem* getItemAtColumn( SvTreeListEntry* pEntry, sal_uInt16 nColumn ) const;

    void setColSizes();

    virtual void Resize();

public:
    css::uno::Reference<css::linguistic2::XConversionDictionary>  m_xDictionary;

private:
    Window*     m_pED_Term;
    Window*     m_pED_Mapping;
    ListBox*    m_pLB_Property;

    std::vector< DictionaryEntry* > m_aToBeDeleted;

    sal_uInt16      m_nSortColumnIndex;
};

class ChineseDictionaryDialog : public ModalDialog
{
public:
    ChineseDictionaryDialog( Window* pParent );
    virtual ~ChineseDictionaryDialog();

    //this method should be called once before calling execute
    void setDirectionAndTextConversionOptions( bool bDirectionToSimplified, sal_Int32 nTextConversionOptions /*i18n::TextConversionOption*/ );

    virtual short   Execute();

private:
    DECL_LINK( DirectionHdl, void* );
    DECL_LINK(EditFieldsHdl, void *);
    DECL_LINK( MappingSelectHdl, void* );
    DECL_LINK( AddHdl, void* );
    DECL_LINK( ModifyHdl, void* );
    DECL_LINK( DeleteHdl, void* );
    DECL_LINK( HeaderBarClick, HeaderBar* );

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

    RadioButton* m_pRB_To_Simplified;
    RadioButton* m_pRB_To_Traditional;

    CheckBox*    m_pCB_Reverse;

    FixedText*   m_pFT_Term;
    Edit*        m_pED_Term;

    FixedText*   m_pFT_Mapping;
    Edit*        m_pED_Mapping;

    FixedText*   m_pFT_Property;
    ListBox*     m_pLB_Property;

    SvSimpleTableContainer* mpToSimplifiedContainer;
    DictionaryList* m_pCT_DictionaryToSimplified;
    SvSimpleTableContainer* mpToTraditionalContainer;
    DictionaryList* m_pCT_DictionaryToTraditional;

    PushButton*  m_pPB_Add;
    PushButton*  m_pPB_Modify;
    PushButton*  m_pPB_Delete;

    css::uno::Reference<css::uno::XComponentContext> m_xContext;
};


} //end namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
