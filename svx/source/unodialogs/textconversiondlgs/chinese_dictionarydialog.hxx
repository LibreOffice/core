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
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
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
    DictionaryEntry( const rtl::OUString& rTerm, const rtl::OUString& rMapping
                    , sal_Int16 nConversionPropertyType //linguistic2::ConversionPropertyType
                    , sal_Bool bNewEntry = sal_False );

    virtual ~DictionaryEntry();

    bool operator==( const DictionaryEntry& rE ) const;

    rtl::OUString m_aTerm;
    rtl::OUString m_aMapping;
    sal_Int16     m_nConversionPropertyType; //linguistic2::ConversionPropertyType

    sal_Bool      m_bNewEntry;
};

class DictionaryList : public SvHeaderTabListBox
{
public:
    DictionaryList( Window* pParent, const ResId& );
    DictionaryList( Window* pParent );
    virtual ~DictionaryList();

    HeaderBar* createHeaderBar( const String& rColumn1, const String& rColumn2, const String& rColumn3
                  , long nWidth1, long nWidth2, long nWidth3 );

    void initDictionaryControl( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XConversionDictionary>& xDictionary
                            , ListBox* pPropertyTypeNameListBox );
    void activate( HeaderBar* pHeaderBar );
    void deleteAll();
    void refillFromDictionary( sal_Int32 nTextConversionOptions /*i18n::TextConversionOption*/ );
    void save();

    DictionaryEntry* getTermEntry( const rtl::OUString& rTerm ) const;
    bool hasTerm( const rtl::OUString& rTerm ) const;

    void addEntry( const rtl::OUString& rTerm, const rtl::OUString& rMapping
            , sal_Int16 nConversionPropertyType /*linguistic2::ConversionPropertyType*/, sal_uIntPtr nPos = LIST_APPEND );
    sal_uIntPtr deleteEntries( const rtl::OUString& rTerm ); //return lowest position of deleted entries or LIST_APPEND if no entry was deleted
    void deleteEntryOnPos( sal_Int32 nPos  );
    DictionaryEntry* getEntryOnPos( sal_Int32 nPos ) const;
    DictionaryEntry* getFirstSelectedEntry() const;

    void sortByColumn( sal_uInt16 nSortColumnIndex, bool bSortAtoZ );
    sal_uInt16 getSortColumn() const;

    virtual void Resize();

private:
    String getPropertyTypeName( sal_Int16 nConversionPropertyType /*linguistic2::ConversionPropertyType*/ ) const;
    String makeTabString( const DictionaryEntry& rEntry ) const;

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
    ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiComponentFactory>         m_xFactory;
};

//.............................................................................
} //end namespace
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
