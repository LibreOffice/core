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

#include <vcl/weld.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>

#include <vector>

namespace textconversiondlgs
{

struct DictionaryEntry final
{
    DictionaryEntry( const OUString& rTerm, const OUString& rMapping
                    , sal_Int16 nConversionPropertyType //linguistic2::ConversionPropertyType
                    , bool bNewEntry = false );

    ~DictionaryEntry();

    OUString const m_aTerm;
    OUString const m_aMapping;
    sal_Int16     m_nConversionPropertyType; //linguistic2::ConversionPropertyType

    bool const     m_bNewEntry;
};

class DictionaryList
{
public:
    DictionaryList(std::unique_ptr<weld::TreeView> xTreeView);

    void init(const css::uno::Reference< css::linguistic2::XConversionDictionary>& xDictionary,
        weld::Entry *pED_Term, weld::Entry *pED_Mapping, weld::ComboBox *pLB_Property);

    void deleteAll();
    void refillFromDictionary( sal_Int32 nTextConversionOptions /*i18n::TextConversionOption*/ );
    void save();

    DictionaryEntry* getTermEntry( const OUString& rTerm ) const;
    bool hasTerm( const OUString& rTerm ) const;

    void addEntry( const OUString& rTerm, const OUString& rMapping
            , sal_Int16 nConversionPropertyType /*linguistic2::ConversionPropertyType*/, int nPos = -1);
    int deleteEntries( const OUString& rTerm ); //return lowest position of deleted entries or -1 if no entry was deleted
    void deleteEntryOnPos( sal_Int32 nPos  );
    DictionaryEntry* getEntryOnPos( sal_Int32 nPos ) const;
    DictionaryEntry* getFirstSelectedEntry() const;

    void set_size_request(int nWidth, int nHeight) { m_xControl->set_size_request(nWidth, nHeight); }
    void hide() { m_xControl->hide(); }
    void show() { m_xControl->show(); }
    void connect_changed(const Link<weld::TreeView&, void>& rLink) { m_xControl->connect_changed(rLink); }
    void connect_column_clicked(const Link<int, void>& rLink) { m_xControl->connect_column_clicked(rLink); }
    bool get_sort_order() const { return m_xControl->get_sort_order(); }
    void set_sort_order(bool bAscending) { return m_xControl->set_sort_order(bAscending); }
    void set_sort_column(int nColumn) { return m_xControl->set_sort_column(nColumn); }
    int get_sort_column() const { return m_xControl->get_sort_column(); }
    int get_selected_index() const { return m_xControl->get_selected_index(); }
    int get_height_rows(int nRows) const { return m_xControl->get_height_rows(nRows); }
    bool get_visible() const { return m_xControl->get_visible(); }
    void set_sort_indicator(TriState eState, int nColumn) { m_xControl->set_sort_indicator(eState, nColumn); }
    weld::TreeView& get_widget() const { return *m_xControl; }

private:
    OUString getPropertyTypeName( sal_Int16 nConversionPropertyType /*linguistic2::ConversionPropertyType*/ ) const;

    DECL_LINK(ResizeHdl, const Size&, void);

public:
    css::uno::Reference<css::linguistic2::XConversionDictionary>  m_xDictionary;

private:
    std::unique_ptr<weld::TreeView> m_xControl;
    std::unique_ptr<weld::TreeIter> m_xIter;
    weld::Entry* m_pED_Term;
    weld::Entry* m_pED_Mapping;
    weld::ComboBox* m_pLB_Property;

    std::vector< DictionaryEntry* > m_aToBeDeleted;
};

class ChineseDictionaryDialog : public weld::GenericDialogController
{
public:
    explicit ChineseDictionaryDialog(weld::Window* pParent);
    virtual ~ChineseDictionaryDialog() override;

    //this method should be called once before calling execute
    void setDirectionAndTextConversionOptions( bool bDirectionToSimplified, sal_Int32 nTextConversionOptions /*i18n::TextConversionOption*/ );

    virtual short run() override;

private:
    DECL_LINK( DirectionHdl, weld::Button&, void );
    DECL_LINK( EditFieldsHdl, weld::Entry&, void );
    DECL_LINK( EditFieldsListBoxHdl, weld::ComboBox&, void );
    DECL_LINK( MappingSelectHdl, weld::TreeView&, void );
    DECL_LINK( AddHdl, weld::Button&, void );
    DECL_LINK( ModifyHdl, weld::Button&, void );
    DECL_LINK( DeleteHdl, weld::Button&, void );
    static void HeaderBarClick(DictionaryList& rList, int nColumn);
    DECL_LINK(ToSimplifiedHeaderBarClick, int, void);
    DECL_LINK(ToTraditionalHeaderBarClick, int, void);
    DECL_LINK(SizeAllocHdl, const Size&, void);

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

    css::uno::Reference<css::uno::XComponentContext> m_xContext;

    std::unique_ptr<weld::RadioButton> m_xRB_To_Simplified;
    std::unique_ptr<weld::RadioButton> m_xRB_To_Traditional;

    std::unique_ptr<weld::CheckButton> m_xCB_Reverse;

    std::unique_ptr<weld::Label> m_xFT_Term;
    std::unique_ptr<weld::Entry> m_xED_Term;

    std::unique_ptr<weld::Label> m_xFT_Mapping;
    std::unique_ptr<weld::Entry> m_xED_Mapping;

    std::unique_ptr<weld::Label> m_xFT_Property;
    std::unique_ptr<weld::ComboBox> m_xLB_Property;

    std::unique_ptr<DictionaryList> m_xCT_DictionaryToSimplified;
    std::unique_ptr<DictionaryList> m_xCT_DictionaryToTraditional;

    std::unique_ptr<weld::Button>  m_xPB_Add;
    std::unique_ptr<weld::Button>  m_xPB_Modify;
    std::unique_ptr<weld::Button>  m_xPB_Delete;
};


} //end namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
