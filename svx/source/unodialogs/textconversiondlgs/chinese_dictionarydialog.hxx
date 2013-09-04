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

#ifndef _TEXTCONVERSIONDLGS_CHINESE_DICTIONARYDLG_HXX
#define _TEXTCONVERSIONDLGS_CHINESE_DICTIONARYDLG_HXX

#include <vcl/dialog.hxx>
// header for class FixedLine
#include <vcl/fixed.hxx>
// header for class RadioButton
#include <vcl/button.hxx>
// header for class Edit
#include <vcl/edit.hxx>
// header for class ListBox
#include <vcl/lstbox.hxx>
// header for class SvHeaderTabListBox
#include <svtools/svtabbx.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>

#include <vector>

//.............................................................................
namespace textconversiondlgs
{
//.............................................................................

//-----------------------------------------------------------------------------
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

class DictionaryList : public SvHeaderTabListBox
{
public:
    DictionaryList( Window* pParent, const ResId& );
    DictionaryList( Window* pParent );
    virtual ~DictionaryList();

    HeaderBar* createHeaderBar( const OUString& rColumn1, const OUString& rColumn2, const OUString& rColumn3
                  , long nWidth1, long nWidth2, long nWidth3 );

    void initDictionaryControl( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XConversionDictionary>& xDictionary
                            , ListBox* pPropertyTypeNameListBox );
    void activate( HeaderBar* pHeaderBar );
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

    virtual void Resize();

private:
    OUString getPropertyTypeName( sal_Int16 nConversionPropertyType /*linguistic2::ConversionPropertyType*/ ) const;
    OUString makeTabString( const DictionaryEntry& rEntry ) const;

    DECL_LINK( CompareHdl, SvSortData* );
    StringCompare ColumnCompare( SvTreeListEntry* pLeft, SvTreeListEntry* pRight );
    SvLBoxItem* getItemAtColumn( SvTreeListEntry* pEntry, sal_uInt16 nColumn ) const;

public:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XConversionDictionary>   m_xDictionary;

private:
    HeaderBar*  m_pHeaderBar;
    ListBox*    m_pPropertyTypeNameListBox;

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
    DECL_LINK( HeaderBarClick, void* );

    void updateAfterDirectionChange();
    void updateButtons();

    bool isEditFieldsHaveContent() const;
    bool isEditFieldsContentEqualsSelectedListContent() const;

    DictionaryList& getActiveDictionary();
    DictionaryList& getReverseDictionary();

    const DictionaryList& getActiveDictionary() const;
    const DictionaryList& getReverseDictionary() const;

private:
    sal_Int32   m_nTextConversionOptions; //i18n::TextConversionOption

    RadioButton m_aRB_To_Simplified;
    RadioButton m_aRB_To_Traditional;

    CheckBox    m_aCB_Reverse;

    FixedText   m_aFT_Term;
    Edit        m_aED_Term;

    FixedText   m_aFT_Mapping;
    Edit        m_aED_Mapping;

    FixedText   m_aFT_Property;
    ListBox     m_aLB_Property;

    HeaderBar*      m_pHeaderBar;
    DictionaryList  m_aCT_DictionaryToSimplified;
    DictionaryList  m_aCT_DictionaryToTraditional;

    PushButton  m_aPB_Add;
    PushButton  m_aPB_Modify;
    PushButton  m_aPB_Delete;

    FixedLine   m_aFL_Bottomline;

    OKButton        m_aBP_OK;
    CancelButton    m_aBP_Cancel;
    HelpButton      m_aBP_Help;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >              m_xContext;
};

//.............................................................................
} //end namespace
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
