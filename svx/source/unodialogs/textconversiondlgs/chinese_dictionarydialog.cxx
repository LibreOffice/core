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


#include "chinese_dictionarydialog.hxx"
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/linguistic2/ConversionPropertyType.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryList.hpp>
#include <com/sun/star/linguistic2/XConversionPropertyType.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <vcl/headbar.hxx>
#include <vcl/svlbitm.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/settings.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>
#include <unotools/intlwrapper.hxx>
#include <vcl/svapp.hxx>
#include <osl/diagnose.h>

namespace textconversiondlgs
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

DictionaryList::DictionaryList(std::unique_ptr<weld::TreeView> xControl)
    : m_xControl(std::move(xControl))
    , m_xIter(m_xControl->make_iterator())
    , m_pED_Term(nullptr)
    , m_pED_Mapping(nullptr)
    , m_pLB_Property(nullptr)
    , m_aToBeDeleted()
{
    m_xControl->make_sorted();
}

OUString DictionaryList::getPropertyTypeName( sal_Int16 nConversionPropertyType ) const
{
    if (!m_pLB_Property || !m_pLB_Property->get_count())
        return OUString();

    sal_uInt16 nPos = static_cast<sal_uInt16>( nConversionPropertyType )-1;
    if (nPos < m_pLB_Property->get_count())
        return m_pLB_Property->get_text(nPos);
    return m_pLB_Property->get_text(0);
}

void DictionaryList::save()
{
    if (!m_xDictionary.is())
        return;

    Reference< linguistic2::XConversionPropertyType > xPropertyType( m_xDictionary, uno::UNO_QUERY );

    sal_Int32 nN;
    DictionaryEntry* pE;

    for( nN = m_aToBeDeleted.size(); nN--; )
    {
        pE = m_aToBeDeleted[nN];
        m_xDictionary->removeEntry( pE->m_aTerm, pE->m_aMapping );
    }
    int nRowCount = m_xControl->n_children();
    for( nN = nRowCount; nN--; )
    {
        pE = getEntryOnPos( nN );
        if(pE->m_bNewEntry)
        {
            try
            {
                m_xDictionary->addEntry( pE->m_aTerm, pE->m_aMapping );
                xPropertyType->setPropertyType( pE->m_aTerm, pE->m_aMapping, pE->m_nConversionPropertyType );
            }
            catch( uno::Exception& )
            {

            }
        }
    }
    Reference< util::XFlushable > xFlush( m_xDictionary, uno::UNO_QUERY );
    if( xFlush.is() )
        xFlush->flush();
}

void DictionaryList::deleteAll()
{
    sal_Int32 nN;
    int nRowCount = m_xControl->n_children();
    for( nN = nRowCount; nN--; )
        deleteEntryOnPos( nN  );
    for( nN = m_aToBeDeleted.size(); nN--; )
    {
        DictionaryEntry* pE = m_aToBeDeleted[nN];
        delete pE;
    }
    m_aToBeDeleted.clear();
}

void DictionaryList::refillFromDictionary( sal_Int32 nTextConversionOptions )
{
    deleteAll();

    if(!m_xDictionary.is())
        return;

    Sequence< OUString > aLeftList(  m_xDictionary->getConversionEntries( linguistic2::ConversionDirection_FROM_LEFT ) );
    sal_Int32 nCount = aLeftList.getLength();

    Reference< linguistic2::XConversionPropertyType > xPropertyType( m_xDictionary, uno::UNO_QUERY );

    OUString aLeft, aRight;
    sal_Int16 nConversionPropertyType;

    for(sal_Int32 nN=0; nN<nCount; nN++)
    {
        aLeft  = aLeftList[nN];
        Sequence< OUString > aRightList( m_xDictionary->getConversions(
            aLeft, 0, aLeft.getLength()
            , linguistic2::ConversionDirection_FROM_LEFT, nTextConversionOptions ) );

        if(aRightList.getLength()!=1)
        {
            OSL_FAIL("The Chinese Translation Dictionary should have exactly one Mapping for each term.");
            continue;
        }

        aRight = aRightList[0];
        nConversionPropertyType = linguistic2::ConversionPropertyType::OTHER;
        if(xPropertyType.is())
            nConversionPropertyType = xPropertyType->getPropertyType(aLeft, aRight);

        DictionaryEntry* pEntry = new DictionaryEntry( aLeft, aRight, nConversionPropertyType );

        m_xControl->append(m_xIter.get());
        m_xControl->set_text(*m_xIter, pEntry->m_aTerm, 0);
        m_xControl->set_text(*m_xIter, pEntry->m_aMapping, 1);
        m_xControl->set_text(*m_xIter, getPropertyTypeName(pEntry->m_nConversionPropertyType), 2);
        m_xControl->set_id(*m_xIter, OUString::number(reinterpret_cast<sal_Int64>(pEntry)));
    }
}

DictionaryEntry* DictionaryList::getFirstSelectedEntry() const
{
    DictionaryEntry* pRet=nullptr;
    int nN = m_xControl->get_selected_index();
    if (nN != -1)
        pRet = getEntryOnPos( nN );
    return pRet;
}

DictionaryEntry* DictionaryList::getEntryOnPos(sal_Int32 nPos) const
{
    OUString sLBEntry = m_xControl->get_id(nPos);
    return reinterpret_cast<DictionaryEntry*>(sLBEntry.toInt64());
}

DictionaryEntry* DictionaryList::getTermEntry( const OUString& rTerm ) const
{
    int nRowCount = m_xControl->n_children();
    for( sal_Int32 nN = nRowCount; nN--; )
    {
        DictionaryEntry* pE = getEntryOnPos( nN );
        if( pE && rTerm == pE->m_aTerm )
            return pE;
    }
    return nullptr;
}

bool DictionaryList::hasTerm( const OUString& rTerm ) const
{
    return getTermEntry(rTerm) !=nullptr ;
}

void DictionaryList::addEntry(const OUString& rTerm, const OUString& rMapping,
                              sal_Int16 nConversionPropertyType, int nPos)
{
    if( hasTerm( rTerm ) )
        return;

    DictionaryEntry* pEntry = new DictionaryEntry( rTerm, rMapping, nConversionPropertyType, true );
    m_xControl->insert(nPos, m_xIter.get());
    m_xControl->set_text(*m_xIter, pEntry->m_aTerm, 0);
    m_xControl->set_text(*m_xIter, pEntry->m_aMapping, 1);
    m_xControl->set_text(*m_xIter, getPropertyTypeName(pEntry->m_nConversionPropertyType), 2);
    m_xControl->set_id(*m_xIter, OUString::number(reinterpret_cast<sal_Int64>(pEntry)));
    m_xControl->select(*m_xIter);
}

void DictionaryList::deleteEntryOnPos( sal_Int32 nPos  )
{
    DictionaryEntry* pEntry = getEntryOnPos( nPos );
    m_xControl->remove(nPos);
    if (pEntry)
    {
        if( pEntry->m_bNewEntry )
            delete pEntry;
        else
            m_aToBeDeleted.push_back( pEntry );
    }
}

int DictionaryList::deleteEntries( const OUString& rTerm )
{
    int nPos = -1;
    int nRowCount = m_xControl->n_children();
    for (sal_Int32 nN = nRowCount; nN--;)
    {
        DictionaryEntry* pCurEntry = getEntryOnPos( nN );
        if( rTerm == pCurEntry->m_aTerm )
        {
            nPos = nN;
            m_xControl->remove(nN);
            if( pCurEntry->m_bNewEntry )
                delete pCurEntry;
            else
                m_aToBeDeleted.push_back( pCurEntry );
        }
    }
    return nPos;
}

DictionaryEntry::DictionaryEntry( const OUString& rTerm, const OUString& rMapping
                    , sal_Int16 nConversionPropertyType
                    , bool bNewEntry )
        : m_aTerm( rTerm )
        , m_aMapping( rMapping )
        , m_nConversionPropertyType( nConversionPropertyType )
        , m_bNewEntry( bNewEntry )
{
    if( m_nConversionPropertyType == 0 )
        m_nConversionPropertyType = 1;
}

DictionaryEntry::~DictionaryEntry()
{
}

IMPL_LINK_NOARG(ChineseDictionaryDialog, SizeAllocHdl, const Size&, void)
{
    DictionaryList* pControl = m_xCT_DictionaryToTraditional->get_visible() ?
                                m_xCT_DictionaryToTraditional.get() :
                                m_xCT_DictionaryToSimplified.get();
    std::vector<int> aWidths;
    int x1, x2, y, width, height;
    if (!m_xED_Mapping->get_extents_relative_to(pControl->get_widget(), x1, y, width, height))
        return;
    aWidths.push_back(x1);
    if (!m_xLB_Property->get_extents_relative_to(pControl->get_widget(), x2, y, width, height))
        return;
    aWidths.push_back(x2 - x1);
    m_xCT_DictionaryToTraditional->get_widget().set_column_fixed_widths(aWidths);
    m_xCT_DictionaryToSimplified->get_widget().set_column_fixed_widths(aWidths);
}

void DictionaryList::init(const Reference< linguistic2::XConversionDictionary>& xDictionary,
    weld::Entry *pED_Term, weld::Entry *pED_Mapping, weld::ComboBox *pLB_Property)
{
    if (m_xDictionary.is())
        return;

    m_xDictionary = xDictionary;

    m_pED_Term = pED_Term;
    m_pED_Mapping = pED_Mapping;
    m_pLB_Property = pLB_Property;

    m_xControl->set_sort_column(0);
    m_xControl->set_sort_indicator(TRISTATE_TRUE, 0);

    std::vector<int> aWidths;
    aWidths.push_back(m_pED_Term->get_preferred_size().Width());
    aWidths.push_back(m_pED_Mapping->get_preferred_size().Width());
    m_xControl->set_column_fixed_widths(aWidths);
}

void ChineseDictionaryDialog::initDictionaryControl(DictionaryList *pList,
    const Reference< linguistic2::XConversionDictionary>& xDictionary)
{
    //set widgets to track the width of for columns
    pList->init(xDictionary,
        m_xED_Term.get(), m_xED_Mapping.get(), m_xLB_Property.get());
}

ChineseDictionaryDialog::ChineseDictionaryDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "svx/ui/chinesedictionary.ui", "ChineseDictionaryDialog")
    , m_nTextConversionOptions(i18n::TextConversionOption::NONE)
    , m_xRB_To_Simplified(m_xBuilder->weld_radio_button("tradtosimple"))
    , m_xRB_To_Traditional(m_xBuilder->weld_radio_button("simpletotrad"))
    , m_xCB_Reverse(m_xBuilder->weld_check_button("reverse"))
    , m_xFT_Term(m_xBuilder->weld_label("termft"))
    , m_xED_Term(m_xBuilder->weld_entry("term"))
    , m_xFT_Mapping(m_xBuilder->weld_label("mappingft"))
    , m_xED_Mapping(m_xBuilder->weld_entry("mapping"))
    , m_xFT_Property(m_xBuilder->weld_label("propertyft"))
    , m_xLB_Property(m_xBuilder->weld_combo_box("property"))
    , m_xCT_DictionaryToSimplified(new DictionaryList(m_xBuilder->weld_tree_view("tradtosimpleview")))
    , m_xCT_DictionaryToTraditional(new DictionaryList(m_xBuilder->weld_tree_view("simpletotradview")))
    , m_xPB_Add(m_xBuilder->weld_button("add"))
    , m_xPB_Modify(m_xBuilder->weld_button("modify"))
    , m_xPB_Delete(m_xBuilder->weld_button("delete"))
{
    m_xCT_DictionaryToSimplified->set_size_request(-1, m_xCT_DictionaryToSimplified->get_height_rows(8));
    m_xCT_DictionaryToTraditional->set_size_request(-1, m_xCT_DictionaryToTraditional->get_height_rows(8));

    SvtLinguConfig  aLngCfg;
    bool bValue;
    Any aAny( aLngCfg.GetProperty( OUString( UPN_IS_REVERSE_MAPPING ) ) );
    if( aAny >>= bValue )
        m_xCB_Reverse->set_active( bValue );

    m_xLB_Property->set_active(0);

    Reference< linguistic2::XConversionDictionary > xDictionary_To_Simplified;
    Reference< linguistic2::XConversionDictionary > xDictionary_To_Traditional;
    //get dictionaries
    {
        if(!m_xContext.is())
            m_xContext.set( ::cppu::defaultBootstrap_InitialComponentContext() );
        if(m_xContext.is())
        {
            Reference< linguistic2::XConversionDictionaryList > xDictionaryList = linguistic2::ConversionDictionaryList::create(m_xContext);
            Reference< container::XNameContainer > xContainer( xDictionaryList->getDictionaryContainer() );
            if(xContainer.is())
            {
                try
                {
                    OUString aNameTo_Simplified("ChineseT2S");
                    OUString aNameTo_Traditional("ChineseS2T");
                    lang::Locale aLocale;
                    aLocale.Language = "zh";

                    if( xContainer->hasByName( aNameTo_Simplified ) )
                        xDictionary_To_Simplified.set(
                                xContainer->getByName( aNameTo_Simplified ), UNO_QUERY );
                    else
                    {
                        aLocale.Country = "TW";
                        xDictionary_To_Simplified.set(
                                xDictionaryList->addNewDictionary( aNameTo_Simplified
                                    , aLocale, linguistic2::ConversionDictionaryType::SCHINESE_TCHINESE
                                ), UNO_QUERY );
                    }
                    if (xDictionary_To_Simplified.is())
                        xDictionary_To_Simplified->setActive( true );


                    if( xContainer->hasByName( aNameTo_Traditional ) )
                        xDictionary_To_Traditional.set(
                                xContainer->getByName( aNameTo_Traditional ), UNO_QUERY );
                    else
                    {
                        aLocale.Country = "CN";
                        xDictionary_To_Traditional.set(
                                xDictionaryList->addNewDictionary( aNameTo_Traditional
                                    ,aLocale, linguistic2::ConversionDictionaryType::SCHINESE_TCHINESE),
                                UNO_QUERY );
                    }
                    if (xDictionary_To_Traditional.is())
                        xDictionary_To_Traditional->setActive( true );

                }
                catch(const uno::Exception& )
                {
                }
            }
        }
    }

    //init dictionary controls
    initDictionaryControl(m_xCT_DictionaryToSimplified.get(), xDictionary_To_Simplified);
    initDictionaryControl(m_xCT_DictionaryToTraditional.get(), xDictionary_To_Traditional);

    //set hdl
    m_xCT_DictionaryToSimplified->connect_column_clicked(LINK(this, ChineseDictionaryDialog, ToSimplifiedHeaderBarClick));
    m_xCT_DictionaryToTraditional->connect_column_clicked(LINK(this, ChineseDictionaryDialog, ToTraditionalHeaderBarClick));

    updateAfterDirectionChange();

    m_xED_Term->connect_changed( LINK( this, ChineseDictionaryDialog, EditFieldsHdl ) );
    m_xED_Mapping->connect_changed( LINK( this, ChineseDictionaryDialog, EditFieldsHdl ) );
    m_xLB_Property->connect_changed( LINK( this, ChineseDictionaryDialog, EditFieldsListBoxHdl ) );

    m_xRB_To_Simplified->connect_clicked( LINK( this, ChineseDictionaryDialog, DirectionHdl ) );
    m_xRB_To_Traditional->connect_clicked( LINK( this, ChineseDictionaryDialog, DirectionHdl ) );

    m_xCT_DictionaryToSimplified->connect_changed( LINK( this, ChineseDictionaryDialog, MappingSelectHdl ));
    m_xCT_DictionaryToTraditional->connect_changed( LINK( this, ChineseDictionaryDialog, MappingSelectHdl ));

    m_xPB_Add->connect_clicked( LINK( this, ChineseDictionaryDialog, AddHdl ) );
    m_xPB_Modify->connect_clicked( LINK( this, ChineseDictionaryDialog, ModifyHdl ) );
    m_xPB_Delete->connect_clicked( LINK( this, ChineseDictionaryDialog, DeleteHdl ) );

    m_xED_Mapping->connect_size_allocate(LINK(this, ChineseDictionaryDialog, SizeAllocHdl));
    m_xLB_Property->connect_size_allocate(LINK(this, ChineseDictionaryDialog, SizeAllocHdl));
}

ChineseDictionaryDialog::~ChineseDictionaryDialog()
{
}

void ChineseDictionaryDialog::setDirectionAndTextConversionOptions( bool bDirectionToSimplified, sal_Int32 nTextConversionOptions /*i18n::TextConversionOption*/ )
{
    if( bDirectionToSimplified == m_xRB_To_Simplified->get_active()
        && nTextConversionOptions == m_nTextConversionOptions )
        return;

    m_nTextConversionOptions = nTextConversionOptions;

    if (bDirectionToSimplified)
        m_xRB_To_Simplified->set_active(true);
    else
        m_xRB_To_Traditional->set_active(true);
    updateAfterDirectionChange();
}

IMPL_LINK_NOARG(ChineseDictionaryDialog, DirectionHdl, weld::Button&, void)
{
    updateAfterDirectionChange();
}

void ChineseDictionaryDialog::updateAfterDirectionChange()
{
    Reference< linguistic2::XConversionDictionary > xDictionary;

    if (m_xRB_To_Simplified->get_active())
    {
        m_xCT_DictionaryToTraditional->hide();
        m_xCT_DictionaryToSimplified->show();
        xDictionary = m_xCT_DictionaryToSimplified->m_xDictionary;
    }
    else
    {
        m_xCT_DictionaryToSimplified->hide();
        m_xCT_DictionaryToTraditional->show();
        xDictionary = m_xCT_DictionaryToTraditional->m_xDictionary;
    }

    updateButtons();
}

IMPL_LINK_NOARG(ChineseDictionaryDialog, EditFieldsListBoxHdl, weld::ComboBox&, void)
{
    updateButtons();
}

IMPL_LINK_NOARG(ChineseDictionaryDialog, EditFieldsHdl, weld::Entry&, void)
{
    updateButtons();
}

IMPL_LINK_NOARG(ChineseDictionaryDialog, MappingSelectHdl, weld::TreeView&, void)
{
    DictionaryEntry* pE = getActiveDictionary().getFirstSelectedEntry();
    if (pE)
    {
        m_xED_Term->set_text( pE->m_aTerm );
        m_xED_Mapping->set_text( pE->m_aMapping );
        sal_Int16 nPos = pE->m_nConversionPropertyType-1;
        if (nPos<0 || nPos>=m_xLB_Property->get_count())
            nPos=0;
        if (m_xLB_Property->get_count())
            m_xLB_Property->set_active(nPos);
    }

    updateButtons();
}

bool ChineseDictionaryDialog::isEditFieldsHaveContent() const
{
    return !m_xED_Term->get_text().isEmpty() && !m_xED_Mapping->get_text().isEmpty();
}

bool ChineseDictionaryDialog::isEditFieldsContentEqualsSelectedListContent() const
{
    DictionaryEntry* pE = getActiveDictionary().getFirstSelectedEntry();
    if( pE )
    {
        if (pE->m_aTerm != m_xED_Term->get_text())
            return false;
        if (pE->m_aMapping != m_xED_Mapping->get_text())
            return false;
        if (pE->m_nConversionPropertyType != m_xLB_Property->get_active() + 1)
            return false;
        return true;
    }
    return false;
}

const DictionaryList& ChineseDictionaryDialog::getActiveDictionary() const
{
    if( m_xRB_To_Traditional->get_active() )
        return *m_xCT_DictionaryToTraditional;
    return *m_xCT_DictionaryToSimplified;
}

DictionaryList& ChineseDictionaryDialog::getActiveDictionary()
{
    if( m_xRB_To_Traditional->get_active() )
        return *m_xCT_DictionaryToTraditional;
    return *m_xCT_DictionaryToSimplified;
}

const DictionaryList& ChineseDictionaryDialog::getReverseDictionary() const
{
    if( m_xRB_To_Traditional->get_active() )
        return *m_xCT_DictionaryToSimplified;
    return *m_xCT_DictionaryToTraditional;
}

DictionaryList& ChineseDictionaryDialog::getReverseDictionary()
{
    if( m_xRB_To_Traditional->get_active() )
        return *m_xCT_DictionaryToSimplified;
    return *m_xCT_DictionaryToTraditional;
}

void ChineseDictionaryDialog::updateButtons()
{
    bool bAdd = isEditFieldsHaveContent() && !getActiveDictionary().hasTerm(m_xED_Term->get_text());
    m_xPB_Add->set_sensitive( bAdd );

    m_xPB_Delete->set_sensitive(!bAdd && getActiveDictionary().get_selected_index() != -1);

    bool bModify = false;
    {
        DictionaryEntry* pFirstSelectedEntry = getActiveDictionary().getFirstSelectedEntry();
        bModify = !bAdd && pFirstSelectedEntry && pFirstSelectedEntry->m_aTerm == m_xED_Term->get_text();
        if( bModify && isEditFieldsContentEqualsSelectedListContent() )
            bModify = false;
    }
    m_xPB_Modify->set_sensitive( bModify );
}

IMPL_LINK_NOARG(ChineseDictionaryDialog, AddHdl, weld::Button&, void)
{
    if( !isEditFieldsHaveContent() )
        return;

    sal_Int16 nConversionPropertyType = m_xLB_Property->get_active() + 1;

    getActiveDictionary().addEntry( m_xED_Term->get_text(), m_xED_Mapping->get_text(), nConversionPropertyType );

    if( m_xCB_Reverse->get_active() )
    {
        getReverseDictionary().deleteEntries( m_xED_Mapping->get_text() );
        getReverseDictionary().addEntry( m_xED_Mapping->get_text(), m_xED_Term->get_text(), nConversionPropertyType );
    }

    updateButtons();
}

IMPL_LINK_NOARG(ChineseDictionaryDialog, ModifyHdl, weld::Button&, void)
{
    OUString aTerm( m_xED_Term->get_text() );
    OUString aMapping( m_xED_Mapping->get_text() );
    sal_Int16 nConversionPropertyType = m_xLB_Property->get_active() + 1;

    DictionaryList& rActive  = getActiveDictionary();
    DictionaryList& rReverse = getReverseDictionary();

    DictionaryEntry* pE = rActive.getFirstSelectedEntry();
    if( pE && pE->m_aTerm != aTerm )
        return;

    if( pE )
    {
        if( pE->m_aMapping != aMapping || pE->m_nConversionPropertyType != nConversionPropertyType )
        {
            if( m_xCB_Reverse->get_active() )
            {
                rReverse.deleteEntries( pE->m_aMapping );
                sal_uIntPtr nPos = rReverse.deleteEntries( aMapping );
                rReverse.addEntry( aMapping, aTerm, nConversionPropertyType, nPos );
            }

            sal_uIntPtr nPos = rActive.deleteEntries( aTerm );
            rActive.addEntry( aTerm, aMapping, nConversionPropertyType, nPos );
        }
    }

    updateButtons();
}

IMPL_LINK_NOARG(ChineseDictionaryDialog, DeleteHdl, weld::Button&, void)
{
    DictionaryList& rActive  = getActiveDictionary();
    DictionaryList& rReverse = getReverseDictionary();

    int nEntry = rActive.get_selected_index();
    if (nEntry != -1)
    {
        DictionaryEntry* pEntry = rActive.getEntryOnPos(nEntry);
        if (pEntry)
        {
            OUString aMapping = pEntry->m_aMapping;
            rActive.deleteEntryOnPos(nEntry);
            if (m_xCB_Reverse->get_active())
                rReverse.deleteEntries(aMapping);
        }
    }

    updateButtons();
}

short ChineseDictionaryDialog::run()
{
    sal_Int32 nTextConversionOptions = m_nTextConversionOptions;
    if(m_nTextConversionOptions & i18n::TextConversionOption::USE_CHARACTER_VARIANTS )
        nTextConversionOptions = nTextConversionOptions^i18n::TextConversionOption::USE_CHARACTER_VARIANTS ;

    m_xCT_DictionaryToSimplified->refillFromDictionary( nTextConversionOptions );
    m_xCT_DictionaryToTraditional->refillFromDictionary( m_nTextConversionOptions );

    short nRet = GenericDialogController::run();

    if( nRet == RET_OK )
    {
        //save settings to configuration
        SvtLinguConfig  aLngCfg;
        aLngCfg.SetProperty( OUString( UPN_IS_REVERSE_MAPPING ), uno::Any(m_xCB_Reverse->get_active()) );

        m_xCT_DictionaryToSimplified->save();
        m_xCT_DictionaryToTraditional->save();
    }

    m_xCT_DictionaryToSimplified->deleteAll();
    m_xCT_DictionaryToTraditional->deleteAll();

    return nRet;
}

void ChineseDictionaryDialog::HeaderBarClick(DictionaryList& rList, int nColumn)
{
    bool bSortAtoZ = rList.get_sort_order();

    //set new arrow positions in headerbar
    if (nColumn == rList.get_sort_column())
    {
        bSortAtoZ = !bSortAtoZ;
        rList.set_sort_order(bSortAtoZ);
    }
    else
    {
        rList.set_sort_indicator(TRISTATE_INDET, rList.get_sort_column());
        rList.set_sort_column(nColumn);
    }

    //sort lists
    rList.set_sort_indicator(bSortAtoZ ? TRISTATE_TRUE : TRISTATE_FALSE, nColumn);
}

IMPL_LINK(ChineseDictionaryDialog, ToSimplifiedHeaderBarClick, int, nColumn, void)
{
    HeaderBarClick(*m_xCT_DictionaryToSimplified, nColumn);
}

IMPL_LINK(ChineseDictionaryDialog, ToTraditionalHeaderBarClick, int, nColumn, void)
{
    HeaderBarClick(*m_xCT_DictionaryToTraditional, nColumn);
}

} //end namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
