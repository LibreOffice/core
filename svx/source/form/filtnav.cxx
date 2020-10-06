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

#include <memory>
#include <filtnav.hxx>
#include <fmexch.hxx>
#include <helpids.h>
#include <fmprop.hxx>
#include <svx/strings.hrc>

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/sqlnode.hxx>
#include <cppuhelper/implbase.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <fmshimp.hxx>
#include <o3tl/safeint.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/request.hxx>
#include <svx/dialmgr.hxx>
#include <svx/fmshell.hxx>
#include <svx/fmtools.hxx>
#include <svx/svxids.hrc>
#include <vcl/settings.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>

#include <bitmaps.hlst>

#include <functional>

using namespace ::svxform;
using namespace ::connectivity;
using namespace ::dbtools;

namespace svxform
{
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::container::XIndexAccess;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::form::runtime::XFormController;
    using ::com::sun::star::form::runtime::XFilterController;
    using ::com::sun::star::form::runtime::XFilterControllerListener;
    using ::com::sun::star::form::runtime::FilterEvent;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::form::XForm;
    using ::com::sun::star::container::XChild;
    using ::com::sun::star::awt::XControl;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::util::XNumberFormatsSupplier;
    using ::com::sun::star::util::XNumberFormatter;
    using ::com::sun::star::util::NumberFormatter;
    using ::com::sun::star::sdbc::XRowSet;
    using ::com::sun::star::lang::Locale;
    using ::com::sun::star::sdb::SQLContext;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Sequence;


OFilterItemExchange::OFilterItemExchange()
    : m_pFormItem(nullptr)
{
}

void OFilterItemExchange::AddSupportedFormats()
{
    AddFormat(getFormatId());
}

SotClipboardFormatId OFilterItemExchange::getFormatId()
{
    static SotClipboardFormatId s_nFormat =
         SotExchange::RegisterFormatName("application/x-openoffice;windows_formatname=\"form.FilterControlExchange\"");
    DBG_ASSERT(static_cast<SotClipboardFormatId>(-1) != s_nFormat, "OFilterExchangeHelper::getFormatId: bad exchange id!");
    return s_nFormat;
}

OLocalExchange* OFilterExchangeHelper::createExchange() const
{
    return new OFilterItemExchange;
}

OUString FmFilterData::GetImage() const
{
    return OUString();
}

FmParentData::~FmParentData()
{
}

OUString FmFormItem::GetImage() const
{
    return RID_SVXBMP_FORM;
}

FmFilterItem* FmFilterItems::Find( const ::sal_Int32 _nFilterComponentIndex ) const
{
    for ( auto & pData : m_aChildren )
    {
        FmFilterItem& rCondition = dynamic_cast<FmFilterItem&>(*pData);
        if ( _nFilterComponentIndex == rCondition.GetComponentIndex() )
            return &rCondition;
    }
    return nullptr;
}

OUString FmFilterItems::GetImage() const
{
    return RID_SVXBMP_FILTER;
}

FmFilterItem::FmFilterItem( FmFilterItems* pParent,
                            const OUString& aFieldName,
                            const OUString& aText,
                            const sal_Int32 _nComponentIndex )
          :FmFilterData(pParent, aText)
          ,m_aFieldName(aFieldName)
          ,m_nComponentIndex( _nComponentIndex )
{
}

OUString FmFilterItem::GetImage() const
{
    return RID_SVXBMP_FIELD;
}

// Hints for communication between model and view

namespace {

class FmFilterHint : public SfxHint
{
    FmFilterData*   m_pData;

public:
    explicit FmFilterHint(FmFilterData* pData):m_pData(pData){}
    FmFilterData* GetData() const { return m_pData; }
};

class FmFilterInsertedHint : public FmFilterHint
{
    size_t m_nPos;   // Position relative to the parent of the data

public:
    FmFilterInsertedHint(FmFilterData* pData, size_t nRelPos)
        :FmFilterHint(pData)
        ,m_nPos(nRelPos){}

    size_t GetPos() const { return m_nPos; }
};

class FmFilterRemovedHint : public FmFilterHint
{
public:
    explicit FmFilterRemovedHint(FmFilterData* pData)
        :FmFilterHint(pData){}
};


class FmFilterTextChangedHint : public FmFilterHint
{
public:
    explicit FmFilterTextChangedHint(FmFilterData* pData)
        :FmFilterHint(pData){}
};

class FilterClearingHint : public SfxHint
{
public:
    FilterClearingHint(){}
};

class FmFilterCurrentChangedHint : public SfxHint
{
public:
    FmFilterCurrentChangedHint(){}
};

}

// class FmFilterAdapter, listener at the FilterControls
class FmFilterAdapter : public ::cppu::WeakImplHelper< XFilterControllerListener >
{
    FmFilterModel*              m_pModel;
    Reference< XIndexAccess >   m_xControllers;

public:
    FmFilterAdapter(FmFilterModel* pModel, const Reference< XIndexAccess >& xControllers);

// XEventListener
    virtual void SAL_CALL disposing(const EventObject& Source) override;

// XFilterControllerListener
    virtual void SAL_CALL predicateExpressionChanged( const FilterEvent& Event ) override;
    virtual void SAL_CALL disjunctiveTermRemoved( const FilterEvent& Event ) override;
    virtual void SAL_CALL disjunctiveTermAdded( const FilterEvent& Event ) override;

// helpers
    /// @throws RuntimeException
    void dispose();

    void AddOrRemoveListener( const Reference< XIndexAccess >& _rxControllers, const bool _bAdd );

    static void setText(sal_Int32 nPos,
        const FmFilterItem* pFilterItem,
        const OUString& rText);
};


FmFilterAdapter::FmFilterAdapter(FmFilterModel* pModel, const Reference< XIndexAccess >& xControllers)
    :m_pModel( pModel )
    ,m_xControllers( xControllers )
{
    AddOrRemoveListener( m_xControllers, true );
}


void FmFilterAdapter::dispose()
{
    AddOrRemoveListener( m_xControllers, false );
}


void FmFilterAdapter::AddOrRemoveListener( const Reference< XIndexAccess >& _rxControllers, const bool _bAdd )
{
    for (sal_Int32 i = 0, nLen = _rxControllers->getCount(); i < nLen; ++i)
    {
        Reference< XIndexAccess > xElement( _rxControllers->getByIndex(i), UNO_QUERY );

        // step down
        AddOrRemoveListener( xElement, _bAdd );

        // handle this particular controller
        Reference< XFilterController > xController( xElement, UNO_QUERY );
        OSL_ENSURE( xController.is(), "FmFilterAdapter::InsertElements: no XFilterController, cannot sync data!" );
        if ( xController.is() )
        {
            if ( _bAdd )
                xController->addFilterControllerListener( this );
            else
                xController->removeFilterControllerListener( this );
        }
    }
}


void FmFilterAdapter::setText(sal_Int32 nRowPos,
                              const FmFilterItem* pFilterItem,
                              const OUString& rText)
{
    FmFormItem* pFormItem = dynamic_cast<FmFormItem*>( pFilterItem->GetParent()->GetParent()  );
    assert(pFormItem);
    try
    {
        Reference< XFilterController > xController( pFormItem->GetController(), UNO_QUERY_THROW );
        xController->setPredicateExpression( pFilterItem->GetComponentIndex(), nRowPos, rText );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}


// XEventListener

void SAL_CALL FmFilterAdapter::disposing(const EventObject& /*e*/)
{
}


namespace
{
    OUString lcl_getLabelName_nothrow( const Reference< XControl >& _rxControl )
    {
        OUString sLabelName;
        try
        {
            Reference< XPropertySet > xModel( _rxControl->getModel(), UNO_QUERY_THROW );
            sLabelName = getLabelName( xModel );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
        return sLabelName;
    }

    Reference< XPropertySet > lcl_getBoundField_nothrow( const Reference< XControl >& _rxControl )
    {
        Reference< XPropertySet > xField;
        try
        {
            Reference< XPropertySet > xModelProps( _rxControl->getModel(), UNO_QUERY_THROW );
            xField.set( xModelProps->getPropertyValue( FM_PROP_BOUNDFIELD ), UNO_QUERY_THROW );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
        return xField;
    }
}

// XFilterControllerListener
void FmFilterAdapter::predicateExpressionChanged( const FilterEvent& Event )
{
    SolarMutexGuard aGuard;

    if ( !m_pModel )
        return;

    // the controller which sent the event
    Reference< XFormController > xController( Event.Source, UNO_QUERY_THROW );
    Reference< XFilterController > xFilterController( Event.Source, UNO_QUERY_THROW );
    Reference< XForm > xForm( xController->getModel(), UNO_QUERY_THROW );

    FmFormItem* pFormItem = m_pModel->Find( m_pModel->m_aChildren, xForm );
    OSL_ENSURE( pFormItem, "FmFilterAdapter::predicateExpressionChanged: don't know this form!" );
    if ( !pFormItem )
        return;

    const sal_Int32 nActiveTerm( xFilterController->getActiveTerm() );

    FmFilterData* pData = pFormItem->GetChildren()[nActiveTerm].get();
    FmFilterItems& rFilter = dynamic_cast<FmFilterItems&>(*pData);
    FmFilterItem* pFilterItem = rFilter.Find( Event.FilterComponent );
    if ( pFilterItem )
    {
        if ( !Event.PredicateExpression.isEmpty())
        {
            pFilterItem->SetText( Event.PredicateExpression );
            // notify the UI
            FmFilterTextChangedHint aChangeHint(pFilterItem);
            m_pModel->Broadcast( aChangeHint );
        }
        else
        {
            // no text anymore so remove the condition
            m_pModel->Remove(pFilterItem);
        }
    }
    else
    {
        // searching the component by field name
        OUString aFieldName( lcl_getLabelName_nothrow( xFilterController->getFilterComponent( Event.FilterComponent ) ) );

        std::unique_ptr<FmFilterItem> pNewFilterItem(new FmFilterItem(&rFilter, aFieldName, Event.PredicateExpression, Event.FilterComponent));
        m_pModel->Insert(rFilter.GetChildren().end(), std::move(pNewFilterItem));
    }

    // ensure there's one empty term in the filter, just in case the active term was previously empty
    m_pModel->EnsureEmptyFilterRows( *pFormItem );
}


void SAL_CALL FmFilterAdapter::disjunctiveTermRemoved( const FilterEvent& Event )
{
    SolarMutexGuard aGuard;

    Reference< XFormController > xController( Event.Source, UNO_QUERY_THROW );
    Reference< XFilterController > xFilterController( Event.Source, UNO_QUERY_THROW );
    Reference< XForm > xForm( xController->getModel(), UNO_QUERY_THROW );

    FmFormItem* pFormItem = m_pModel->Find( m_pModel->m_aChildren, xForm );
    OSL_ENSURE( pFormItem, "FmFilterAdapter::disjunctiveTermRemoved: don't know this form!" );
    if ( !pFormItem )
        return;

    auto& rTermItems = pFormItem->GetChildren();
    const bool bValidIndex = ( Event.DisjunctiveTerm >= 0 ) && ( o3tl::make_unsigned(Event.DisjunctiveTerm) < rTermItems.size() );
    OSL_ENSURE( bValidIndex, "FmFilterAdapter::disjunctiveTermRemoved: invalid term index!" );
    if ( !bValidIndex )
        return;

    // if the first term was removed, then the to-be first term needs its text updated
    if ( Event.DisjunctiveTerm == 0 )
    {
        rTermItems[1]->SetText( SvxResId(RID_STR_FILTER_FILTER_FOR));
        FmFilterTextChangedHint aChangeHint( rTermItems[1].get() );
        m_pModel->Broadcast( aChangeHint );
    }

    // finally remove the entry from the model
    m_pModel->Remove( rTermItems.begin() + Event.DisjunctiveTerm );

    // ensure there's one empty term in the filter, just in case the currently removed one was the last empty one
    m_pModel->EnsureEmptyFilterRows( *pFormItem );
}


void SAL_CALL FmFilterAdapter::disjunctiveTermAdded( const FilterEvent& Event )
{
    SolarMutexGuard aGuard;

    Reference< XFormController > xController( Event.Source, UNO_QUERY_THROW );
    Reference< XFilterController > xFilterController( Event.Source, UNO_QUERY_THROW );
    Reference< XForm > xForm( xController->getModel(), UNO_QUERY_THROW );

    FmFormItem* pFormItem = m_pModel->Find( m_pModel->m_aChildren, xForm );
    OSL_ENSURE( pFormItem, "FmFilterAdapter::disjunctiveTermAdded: don't know this form!" );
    if ( !pFormItem )
        return;

    const sal_Int32 nInsertPos = Event.DisjunctiveTerm;
    bool bValidIndex = ( nInsertPos >= 0 ) && ( o3tl::make_unsigned(nInsertPos) <= pFormItem->GetChildren().size() );
    if ( !bValidIndex )
    {
        OSL_FAIL( "FmFilterAdapter::disjunctiveTermAdded: invalid index!" );
        return;
    }

    auto insertPos = pFormItem->GetChildren().begin() + nInsertPos;

    // "Filter for" for first position, "Or" for the other positions
    std::unique_ptr<FmFilterItems> pFilterItems(new FmFilterItems(pFormItem, (nInsertPos?SvxResId(RID_STR_FILTER_FILTER_OR):SvxResId(RID_STR_FILTER_FILTER_FOR))));
    m_pModel->Insert( insertPos, std::move(pFilterItems) );
}


FmFilterModel::FmFilterModel()
              :FmParentData(nullptr, OUString())
              ,OSQLParserClient(comphelper::getProcessComponentContext())
              ,m_pCurrentItems(nullptr)
{
}


FmFilterModel::~FmFilterModel()
{
    Clear();
}


void FmFilterModel::Clear()
{
    // notify
    FilterClearingHint aClearedHint;
    Broadcast( aClearedHint );

    // lose endings
    if (m_pAdapter.is())
    {
        m_pAdapter->dispose();
        m_pAdapter.clear();
    }

    m_pCurrentItems  = nullptr;
    m_xController    = nullptr;
    m_xControllers   = nullptr;

    m_aChildren.clear();
}


void FmFilterModel::Update(const Reference< XIndexAccess > & xControllers, const Reference< XFormController > & xCurrent)
{
    if ( xCurrent == m_xController )
        return;

    if (!xControllers.is())
    {
        Clear();
        return;
    }

    // there is only a new current controller
    if ( m_xControllers != xControllers )
    {
        Clear();

        m_xControllers = xControllers;
        Update(m_xControllers, this);

        DBG_ASSERT(xCurrent.is(), "FmFilterModel::Update(...) no current controller");

        // Listening for TextChanges
        m_pAdapter = new FmFilterAdapter(this, xControllers);

        SetCurrentController(xCurrent);
        EnsureEmptyFilterRows( *this );
    }
    else
        SetCurrentController(xCurrent);
}


void FmFilterModel::Update(const Reference< XIndexAccess > & xControllers, FmParentData* pParent)
{
    try
    {
        sal_Int32 nCount = xControllers->getCount();
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            Reference< XFormController > xController( xControllers->getByIndex(i), UNO_QUERY_THROW );

            Reference< XPropertySet > xFormProperties( xController->getModel(), UNO_QUERY_THROW );
            OUString aName;
            OSL_VERIFY( xFormProperties->getPropertyValue( FM_PROP_NAME ) >>= aName );

            // Insert a new item for the form
            FmFormItem* pFormItem = new FmFormItem( pParent, xController, aName );
            Insert( pParent->GetChildren().end(), std::unique_ptr<FmFilterData>(pFormItem) );

            Reference< XFilterController > xFilterController( pFormItem->GetFilterController(), UNO_SET_THROW );

            // insert the existing filters for the form
            OUString aTitle(SvxResId(RID_STR_FILTER_FILTER_FOR));

            const Sequence< Sequence< OUString > > aExpressions = xFilterController->getPredicateExpressions();
            for ( auto const & conjunctionTerm : aExpressions )
            {
                // we always display one row, even if there's no term to be displayed
                FmFilterItems* pFilterItems = new FmFilterItems( pFormItem, aTitle );
                Insert( pFormItem->GetChildren().end(), std::unique_ptr<FmFilterData>(pFilterItems) );

                const Sequence< OUString >& rDisjunction( conjunctionTerm );
                sal_Int32 nComponentIndex = -1;
                for ( const OUString& rDisjunctiveTerm : rDisjunction )
                {
                    ++nComponentIndex;

                    if ( rDisjunctiveTerm.isEmpty() )
                        // no condition for this particular component in this particular conjunction term
                        continue;

                    // determine the display name of the control
                    const Reference< XControl > xFilterControl( xFilterController->getFilterComponent( nComponentIndex ) );
                    const OUString sDisplayName( lcl_getLabelName_nothrow( xFilterControl ) );

                    // insert a new entry
                    std::unique_ptr<FmFilterItem> pANDCondition(new FmFilterItem( pFilterItems, sDisplayName, rDisjunctiveTerm, nComponentIndex ));
                    Insert( pFilterItems->GetChildren().end(), std::move(pANDCondition) );
                }

                // title for the next conditions
                aTitle = SvxResId( RID_STR_FILTER_FILTER_OR );
            }

            // now add dependent controllers
            Update( xController, pFormItem );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}


FmFormItem* FmFilterModel::Find(const ::std::vector<std::unique_ptr<FmFilterData>>& rItems, const Reference< XFormController > & xController) const
{
    for (const auto& rItem : rItems)
    {
        FmFormItem* pForm = dynamic_cast<FmFormItem*>( rItem.get() );
        if (pForm)
        {
            if ( xController == pForm->GetController() )
                return pForm;
            else
            {
                pForm = Find(pForm->GetChildren(), xController);
                if (pForm)
                    return pForm;
            }
        }
    }
    return nullptr;
}


FmFormItem* FmFilterModel::Find(const ::std::vector<std::unique_ptr<FmFilterData>>& rItems, const Reference< XForm >& xForm) const
{
    for (const auto& rItem : rItems)
    {
        FmFormItem* pForm = dynamic_cast<FmFormItem*>( rItem.get() );
        if (pForm)
        {
            if (xForm == pForm->GetController()->getModel())
                return pForm;
            else
            {
                pForm = Find(pForm->GetChildren(), xForm);
                if (pForm)
                    return pForm;
            }
        }
    }
    return nullptr;
}

void FmFilterModel::SetCurrentController(const Reference< XFormController > & xCurrent)
{
    if ( xCurrent == m_xController )
        return;

    m_xController = xCurrent;

    FmFormItem* pItem = Find( m_aChildren, xCurrent );
    if ( !pItem )
        return;

    try
    {
        Reference< XFilterController > xFilterController( m_xController, UNO_QUERY_THROW );
        const sal_Int32 nActiveTerm( xFilterController->getActiveTerm() );
        if (nActiveTerm != -1 && pItem->GetChildren().size() > o3tl::make_unsigned(nActiveTerm))
        {
            SetCurrentItems( static_cast< FmFilterItems* >( pItem->GetChildren()[ nActiveTerm ].get() ) );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}

void FmFilterModel::AppendFilterItems( FmFormItem& _rFormItem )
{
    // insert the condition behind the last filter items
    auto iter = std::find_if(_rFormItem.GetChildren().rbegin(), _rFormItem.GetChildren().rend(),
        [](const std::unique_ptr<FmFilterData>& rChild) { return dynamic_cast<const FmFilterItems*>(rChild.get()) != nullptr; });

    sal_Int32 nInsertPos = iter.base() - _rFormItem.GetChildren().begin();
    // delegate this to the FilterController, it will notify us, which will let us update our model
    try
    {
        Reference< XFilterController > xFilterController( _rFormItem.GetFilterController(), UNO_SET_THROW );
        if ( nInsertPos >= xFilterController->getDisjunctiveTerms() )
            xFilterController->appendEmptyDisjunctiveTerm();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}

void FmFilterModel::Insert(const ::std::vector<std::unique_ptr<FmFilterData>>::iterator& rPos, std::unique_ptr<FmFilterData> pData)
{
    auto pTemp = pData.get();
    size_t nPos;
    ::std::vector<std::unique_ptr<FmFilterData>>& rItems = pData->GetParent()->GetChildren();
    if (rPos == rItems.end())
    {
        nPos = rItems.size();
        rItems.push_back(std::move(pData));
    }
    else
    {
        nPos = rPos - rItems.begin();
        rItems.insert(rPos, std::move(pData));
    }

    // notify the UI
    FmFilterInsertedHint aInsertedHint(pTemp, nPos);
    Broadcast( aInsertedHint );
}

void FmFilterModel::Remove(FmFilterData* pData)
{
    FmParentData* pParent = pData->GetParent();
    ::std::vector<std::unique_ptr<FmFilterData>>& rItems = pParent->GetChildren();

    // erase the item from the model
    auto i = ::std::find_if(rItems.begin(), rItems.end(),
            [&](const std::unique_ptr<FmFilterData>& p) { return p.get() == pData; } );
    DBG_ASSERT(i != rItems.end(), "FmFilterModel::Remove(): unknown Item");
    // position within the parent
    sal_Int32 nPos = i - rItems.begin();
    if (dynamic_cast<const FmFilterItems*>( pData) !=  nullptr)
    {
        FmFormItem* pFormItem = static_cast<FmFormItem*>(pParent);

        try
        {
            Reference< XFilterController > xFilterController( pFormItem->GetFilterController(), UNO_SET_THROW );

            bool bEmptyLastTerm = ( ( nPos == 0 ) && xFilterController->getDisjunctiveTerms() == 1 );
            if ( bEmptyLastTerm )
            {
                // remove all children (by setting an empty predicate expression)
                ::std::vector< std::unique_ptr<FmFilterData> >& rChildren = static_cast<FmFilterItems*>(pData)->GetChildren();
                while ( !rChildren.empty() )
                {
                    auto removePos = rChildren.end() - 1;
                    if (FmFilterItem* pFilterItem = dynamic_cast<FmFilterItem*>( removePos->get() ))
                    {
                        FmFilterAdapter::setText( nPos, pFilterItem, OUString() );
                    }
                    Remove( removePos );
                }
            }
            else
            {
                xFilterController->removeDisjunctiveTerm( nPos );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }
    else // FormItems can not be deleted
    {
        FmFilterItem& rFilterItem = dynamic_cast<FmFilterItem&>(*pData);

        // if it's the last condition remove the parent
        if (rItems.size() == 1)
            Remove(rFilterItem.GetParent());
        else
        {
            // find the position of the father within his father
            ::std::vector<std::unique_ptr<FmFilterData>>& rParentParentItems = pData->GetParent()->GetParent()->GetChildren();
            auto j = ::std::find_if(rParentParentItems.begin(), rParentParentItems.end(),
                [&](const std::unique_ptr<FmFilterData>& p) { return p.get() == rFilterItem.GetParent(); });
            DBG_ASSERT(j != rParentParentItems.end(), "FmFilterModel::Remove(): unknown Item");
            sal_Int32 nParentPos = j - rParentParentItems.begin();

            // EmptyText removes the filter
            FmFilterAdapter::setText(nParentPos, &rFilterItem, OUString());
            Remove( i );
        }
    }
}

void FmFilterModel::Remove( const ::std::vector<std::unique_ptr<FmFilterData>>::iterator& rPos )
{
    // remove from parent's child list
    std::unique_ptr<FmFilterData> pData = std::move(*rPos);
    pData->GetParent()->GetChildren().erase( rPos );

    // notify the view, this will remove the actual SvTreeListEntry
    FmFilterRemovedHint aRemoveHint( pData.get() );
    Broadcast( aRemoveHint );
}


bool FmFilterModel::ValidateText(FmFilterItem const * pItem, OUString& rText, OUString& rErrorMsg) const
{
    FmFormItem* pFormItem = dynamic_cast<FmFormItem*>( pItem->GetParent()->GetParent()  );
    assert(pFormItem);
    try
    {
        Reference< XFormController > xFormController( pFormItem->GetController() );
        // obtain the connection of the form belonging to the controller
        Reference< XRowSet > xRowSet( xFormController->getModel(), UNO_QUERY_THROW );
        Reference< XConnection > xConnection( getConnection( xRowSet ) );

        // obtain a number formatter for this connection
        // TODO: shouldn't this be cached?
        Reference< XNumberFormatsSupplier > xFormatSupplier = getNumberFormats( xConnection, true );
        Reference< XNumberFormatter > xFormatter( NumberFormatter::create( comphelper::getProcessComponentContext() ), UNO_QUERY_THROW );
        xFormatter->attachNumberFormatsSupplier( xFormatSupplier );

        // get the field (database column) which the item is responsible for
        Reference< XFilterController > xFilterController( xFormController, UNO_QUERY_THROW );
        Reference< XPropertySet > xField( lcl_getBoundField_nothrow( xFilterController->getFilterComponent( pItem->GetComponentIndex() ) ), UNO_SET_THROW );

        // parse the given text as filter predicate
        OUString aErr, aTxt( rText );
        std::unique_ptr< OSQLParseNode > pParseNode = predicateTree( aErr, aTxt, xFormatter, xField );
        rErrorMsg = aErr;
        rText = aTxt;
        if ( pParseNode != nullptr )
        {
            OUString aPreparedText;
            Locale aAppLocale = Application::GetSettings().GetUILanguageTag().getLocale();
            pParseNode->parseNodeToPredicateStr(
                aPreparedText, xConnection, xFormatter, xField, OUString(), aAppLocale, OUString("."), getParseContext() );
            rText = aPreparedText;
            return true;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }

    return false;
}


void FmFilterModel::Append(FmFilterItems* pItems, std::unique_ptr<FmFilterItem> pFilterItem)
{
    Insert(pItems->GetChildren().end(), std::move(pFilterItem));
}


void FmFilterModel::SetTextForItem(FmFilterItem* pItem, const OUString& rText)
{
    ::std::vector<std::unique_ptr<FmFilterData>>& rItems = pItem->GetParent()->GetParent()->GetChildren();
    auto i = ::std::find_if(rItems.begin(), rItems.end(),
                [&](const std::unique_ptr<FmFilterData>& p) { return p.get() == pItem->GetParent(); });
    sal_Int32 nParentPos = i - rItems.begin();

    FmFilterAdapter::setText(nParentPos, pItem, rText);

    if (rText.isEmpty())
        Remove(pItem);
    else
    {
        // Change the text
        pItem->SetText(rText);
        FmFilterTextChangedHint aChangeHint(pItem);
        Broadcast( aChangeHint );
    }
}


void FmFilterModel::SetCurrentItems(FmFilterItems* pCurrent)
{
    if (m_pCurrentItems == pCurrent)
        return;

    // search for the condition
    if (pCurrent)
    {
        FmFormItem* pFormItem = static_cast<FmFormItem*>(pCurrent->GetParent());
        ::std::vector<std::unique_ptr<FmFilterData>>& rItems = pFormItem->GetChildren();
        auto i = ::std::find_if(rItems.begin(), rItems.end(),
                    [&](const std::unique_ptr<FmFilterData>& p) { return p.get() == pCurrent; });

        if (i != rItems.end())
        {
            // determine the filter position
            sal_Int32 nPos = i - rItems.begin();
            try
            {
                Reference< XFilterController > xFilterController( pFormItem->GetFilterController(), UNO_SET_THROW );
                xFilterController->setActiveTerm( nPos );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("svx");
            }

            if ( m_xController != pFormItem->GetController() )
                // calls SetCurrentItems again
                SetCurrentController( pFormItem->GetController() );
            else
                m_pCurrentItems = pCurrent;
        }
        else
            m_pCurrentItems = nullptr;
    }
    else
        m_pCurrentItems = nullptr;


    // notify the UI
    FmFilterCurrentChangedHint aHint;
    Broadcast( aHint );
}


void FmFilterModel::EnsureEmptyFilterRows( FmParentData& _rItem )
{
    // checks whether for each form there's one free level for input
    ::std::vector< std::unique_ptr<FmFilterData> >& rChildren = _rItem.GetChildren();
    bool bAppendLevel = dynamic_cast<const FmFormItem*>(&_rItem) !=  nullptr;

    for ( const auto& rpChild : rChildren )
    {
        FmFilterItems* pItems = dynamic_cast<FmFilterItems*>( rpChild.get() );
        if ( pItems && pItems->GetChildren().empty() )
        {
            bAppendLevel = false;
            break;
        }

        FmFormItem* pFormItem = dynamic_cast<FmFormItem*>( rpChild.get() );
        if (pFormItem)
        {
            EnsureEmptyFilterRows( *pFormItem );
            continue;
        }
    }

    if ( bAppendLevel )
    {
        FmFormItem* pFormItem = dynamic_cast<FmFormItem*>( &_rItem  );
        OSL_ENSURE( pFormItem, "FmFilterModel::EnsureEmptyFilterRows: no FmFormItem, but a FmFilterItems child?" );
        if ( pFormItem )
            AppendFilterItems( *pFormItem );
    }
}

const int nxD = 4;
const int nxDBmp = 12;

IMPL_STATIC_LINK(FmFilterNavigator, CustomGetSizeHdl, weld::TreeView::get_size_args, aPayload, Size)
{
    vcl::RenderContext& rRenderContext = aPayload.first;
    const OUString& rId = aPayload.second;

    Size aSize;

    FmFilterData* pData = reinterpret_cast<FmFilterData*>(rId.toUInt64());
    OUString sText = pData->GetText();

    if (FmFilterItem* pItem = dynamic_cast<FmFilterItem*>(pData))
    {
        rRenderContext.Push(PushFlags::FONT);
        vcl::Font aFont(rRenderContext.GetFont());
        aFont.SetWeight(WEIGHT_BOLD);
        rRenderContext.SetFont(aFont);

        OUString sName = pItem->GetFieldName() + ": ";
        aSize = Size(rRenderContext.GetTextWidth(sName), rRenderContext.GetTextHeight());

        rRenderContext.Pop();

        aSize.AdjustWidth(rRenderContext.GetTextWidth(sText) + nxD);
    }
    else
    {
        aSize = Size(rRenderContext.GetTextWidth(sText), rRenderContext.GetTextHeight());
        if (dynamic_cast<FmFilterItems*>(pData))
            aSize.AdjustWidth(nxDBmp);
    }

    return aSize;
}

IMPL_STATIC_LINK(FmFilterNavigator, CustomRenderHdl, weld::TreeView::render_args, aPayload, void)
{
    vcl::RenderContext& rRenderContext = std::get<0>(aPayload);
    const ::tools::Rectangle& rRect = std::get<1>(aPayload);
    ::tools::Rectangle aRect(rRect.TopLeft(), Size(rRenderContext.GetOutputSize().Width() - rRect.Left(), rRect.GetHeight()));
    bool bSelected = std::get<2>(aPayload);
    const OUString& rId = std::get<3>(aPayload);

    rRenderContext.Push(PushFlags::TEXTCOLOR);
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    if (bSelected)
        rRenderContext.SetTextColor(rStyleSettings.GetHighlightTextColor());
    else
        rRenderContext.SetTextColor(rStyleSettings.GetDialogTextColor());

    FmFilterData* pData = reinterpret_cast<FmFilterData*>(rId.toUInt64());
    OUString sText = pData->GetText();
    Point aPos(aRect.TopLeft());

    if (FmFilterItem* pFilter = dynamic_cast<FmFilterItem*>(pData))
    {
        vcl::Font aFont(rRenderContext.GetFont());
        aFont.SetWeight(WEIGHT_BOLD);

        rRenderContext.Push(PushFlags::FONT);
        rRenderContext.SetFont(aFont);

        OUString sName = pFilter->GetFieldName() + ": ";
        rRenderContext.DrawText(aPos, sName);

        // position for the second text
        aPos.AdjustX(rRenderContext.GetTextWidth(sName) + nxD);
        rRenderContext.Pop();

        rRenderContext.DrawText(aPos, sText);
    }
    else if (FmFilterItems* pRow = dynamic_cast<FmFilterItems*>(pData))
    {
        FmFormItem* pForm = static_cast<FmFormItem*>(pRow->GetParent());

        // current filter is significant painted
        const bool bIsCurrentFilter = pForm->GetChildren()[ pForm->GetFilterController()->getActiveTerm() ].get() == pRow;
        if (bIsCurrentFilter)
        {
            rRenderContext.Push(PushFlags::LINECOLOR);
            rRenderContext.SetLineColor(rRenderContext.GetTextColor());

            Point aFirst(aPos.X(), aRect.Bottom() - 6);
            Point aSecond(aFirst .X() + 2, aFirst.Y() + 3);

            rRenderContext.DrawLine(aFirst, aSecond);

            aFirst = aSecond;
            aFirst.AdjustX(1);
            aSecond.AdjustX(6);
            aSecond.AdjustY(-5);

            rRenderContext.DrawLine(aFirst, aSecond);
            rRenderContext.Pop();
        }

        rRenderContext.DrawText(Point(aPos.X() + nxDBmp, aPos.Y()), sText);
    }
    else
        rRenderContext.DrawText(aPos, sText);

    rRenderContext.Pop();
}

FmFilterNavigatorDropTarget::FmFilterNavigatorDropTarget(FmFilterNavigator& rTreeView)
    : DropTargetHelper(rTreeView.get_widget().get_drop_target())
    , m_rTreeView(rTreeView)
{
}

sal_Int8 FmFilterNavigatorDropTarget::AcceptDrop(const AcceptDropEvent& rEvt)
{
    sal_Int8 nAccept = m_rTreeView.AcceptDrop(rEvt);

    if (nAccept != DND_ACTION_NONE)
    {
        // to enable the autoscroll when we're close to the edges
        weld::TreeView& rWidget = m_rTreeView.get_widget();
        rWidget.get_dest_row_at_pos(rEvt.maPosPixel, nullptr, true);
    }

    return nAccept;
}

sal_Int8 FmFilterNavigatorDropTarget::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    return m_rTreeView.ExecuteDrop(rEvt);
}

FmFilterNavigator::FmFilterNavigator(vcl::Window* pTopLevel, std::unique_ptr<weld::TreeView> xTreeView)
    : m_xTopLevel(pTopLevel)
    , m_xTreeView(std::move(xTreeView))
    , m_aDropTargetHelper(*this)
    , m_aControlExchange()
    , m_nAsyncRemoveEvent(nullptr)
{
    m_xTreeView->set_help_id(HID_FILTER_NAVIGATOR);

    m_xTreeView->set_selection_mode(SelectionMode::Multiple);

    m_pModel.reset( new FmFilterModel() );
    StartListening( *m_pModel );

    m_xTreeView->connect_custom_get_size(LINK(this, FmFilterNavigator, CustomGetSizeHdl));
    m_xTreeView->connect_custom_render(LINK(this, FmFilterNavigator, CustomRenderHdl));
    m_xTreeView->set_column_custom_renderer(0, true);

    m_xTreeView->connect_changed(LINK(this, FmFilterNavigator, SelectHdl));
    m_xTreeView->connect_key_press(LINK(this, FmFilterNavigator, KeyInputHdl));
    m_xTreeView->connect_popup_menu(LINK(this, FmFilterNavigator, PopupMenuHdl));
    m_xTreeView->connect_editing(LINK(this, FmFilterNavigator, EditingEntryHdl),
                                 LINK(this, FmFilterNavigator, EditedEntryHdl));
    m_xTreeView->connect_drag_begin(LINK(this, FmFilterNavigator, DragBeginHdl));
}

FmFilterNavigator::~FmFilterNavigator()
{
    if (m_nAsyncRemoveEvent)
        Application::RemoveUserEvent(m_nAsyncRemoveEvent);
    EndListening(*m_pModel);
    m_pModel.reset();
}

void FmFilterNavigator::UpdateContent(const Reference< XIndexAccess > & xControllers, const Reference< XFormController > & xCurrent)
{
    if (xCurrent == m_pModel->GetCurrentController())
        return;

    m_pModel->Update(xControllers, xCurrent);

    // expand the filters for the current controller
    std::unique_ptr<weld::TreeIter> xEntry = FindEntry(m_pModel->GetCurrentForm());
    if (!xEntry || m_xTreeView->get_row_expanded(*xEntry))
        return;

    m_xTreeView->unselect_all();

    m_xTreeView->expand_row(*xEntry);

    xEntry = FindEntry(m_pModel->GetCurrentItems());
    if (xEntry)
    {
        if (!m_xTreeView->get_row_expanded(*xEntry))
            m_xTreeView->expand_row(*xEntry);
        m_xTreeView->select(*xEntry);
        SelectHdl(*m_xTreeView);
    }
}

IMPL_LINK(FmFilterNavigator, EditingEntryHdl, const weld::TreeIter&, rIter, bool)
{
    // returns true to allow editing
    if (dynamic_cast<const FmFilterItem*>(reinterpret_cast<FmFilterData*>(m_xTreeView->get_id(rIter).toUInt64())))
    {
        m_xEditingCurrently = m_xTreeView->make_iterator(&rIter);
        return true;
    }
    m_xEditingCurrently.reset();
    return false;
}

IMPL_LINK(FmFilterNavigator, EditedEntryHdl, const IterString&, rIterString, bool)
{
    const weld::TreeIter& rIter = rIterString.first;
    const OUString& rNewText = rIterString.second;

    assert(m_xEditingCurrently && m_xTreeView->iter_compare(rIter, *m_xEditingCurrently) == 0 &&
               "FmFilterNavigator::EditedEntry: suspicious entry!");
    m_xEditingCurrently.reset();

    FmFilterData* pData = reinterpret_cast<FmFilterData*>(m_xTreeView->get_id(rIter).toUInt64());

    DBG_ASSERT(dynamic_cast<const FmFilterItem*>(pData) != nullptr,
                    "FmFilterNavigator::EditedEntry() wrong entry");

    OUString aText(comphelper::string::strip(rNewText, ' '));
    if (aText.isEmpty())
    {
        // deleting the entry asynchron
        m_nAsyncRemoveEvent = Application::PostUserEvent(LINK(this, FmFilterNavigator, OnRemove), pData);
    }
    else
    {
        OUString aErrorMsg;

        if (m_pModel->ValidateText(static_cast<FmFilterItem*>(pData), aText, aErrorMsg))
        {
            // this will set the text at the FmFilterItem, as well as update any filter controls
            // which are connected to this particular entry
            m_pModel->SetTextForItem(static_cast<FmFilterItem*>(pData), aText);
            m_xTreeView->set_text(rIter, aText);
        }
        else
        {
            // display the error and return sal_False
            SQLContext aError;
            aError.Message = SvxResId(RID_STR_SYNTAXERROR);
            aError.Details = aErrorMsg;
            displayException(aError, m_xTopLevel);

            return false;
        }
    }
    return true;
}

IMPL_LINK( FmFilterNavigator, OnRemove, void*, p, void )
{
    m_nAsyncRemoveEvent = nullptr;
    // now remove the entry
    m_pModel->Remove(static_cast<FmFilterData*>(p));
}

sal_Int8 FmFilterNavigator::AcceptDrop( const AcceptDropEvent& rEvt )
{
    if (!m_aControlExchange.isDragSource())
        return DND_ACTION_NONE;

    if (!OFilterItemExchange::hasFormat(m_aDropTargetHelper.GetDataFlavorExVector()))
        return DND_ACTION_NONE;

    // do we contain the formitem?
    if (!FindEntry(m_aControlExchange->getFormItem()))
        return DND_ACTION_NONE;

    Point aDropPos = rEvt.maPosPixel;
    std::unique_ptr<weld::TreeIter> xDropTarget(m_xTreeView->make_iterator());
    // get_dest_row_at_pos with false cause we must drop exactly "on" a node to paste a condition into it
    if (!m_xTreeView->get_dest_row_at_pos(aDropPos, xDropTarget.get(), false))
        xDropTarget.reset();

    if (!xDropTarget)
        return DND_ACTION_NONE;

    FmFilterData* pData = reinterpret_cast<FmFilterData*>(m_xTreeView->get_id(*xDropTarget).toUInt64());
    FmFormItem* pForm = nullptr;
    if (dynamic_cast<const FmFilterItem*>(pData) !=  nullptr)
    {
        pForm = dynamic_cast<FmFormItem*>( pData->GetParent()->GetParent() );
        if (pForm != m_aControlExchange->getFormItem())
            return DND_ACTION_NONE;
    }
    else if (dynamic_cast<const FmFilterItems*>( pData) !=  nullptr)
    {
        pForm = dynamic_cast<FmFormItem*>( pData->GetParent() );
        if (pForm != m_aControlExchange->getFormItem())
            return DND_ACTION_NONE;
    }
    else
        return DND_ACTION_NONE;

    return rEvt.mnAction;
}

namespace
{
    FmFilterItems* getTargetItems(const weld::TreeView& rTreeView, const weld::TreeIter& rTarget)
    {
        FmFilterData* pData = reinterpret_cast<FmFilterData*>(rTreeView.get_id(rTarget).toUInt64());
        FmFilterItems*  pTargetItems = dynamic_cast<FmFilterItems*>(pData);
        if (!pTargetItems)
            pTargetItems = dynamic_cast<FmFilterItems*>(pData->GetParent());
        return pTargetItems;
    }
}

sal_Int8 FmFilterNavigator::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    if (!m_aControlExchange.isDragSource())
        return DND_ACTION_NONE;

    Point aDropPos = rEvt.maPosPixel;
    std::unique_ptr<weld::TreeIter> xDropTarget(m_xTreeView->make_iterator());
    // get_dest_row_at_pos with false cause we must drop exactly "on" a node to paste a condition into it
    if (!m_xTreeView->get_dest_row_at_pos(aDropPos, xDropTarget.get(), false))
        xDropTarget.reset();
    if (!xDropTarget)
        return DND_ACTION_NONE;

    // search the container where to add the items
    FmFilterItems* pTargetItems = getTargetItems(*m_xTreeView, *xDropTarget);
    m_xTreeView->unselect_all();
    std::unique_ptr<weld::TreeIter> xEntry = FindEntry(pTargetItems);
    m_xTreeView->select(*xEntry);
    m_xTreeView->set_cursor(*xEntry);

    insertFilterItem(m_aControlExchange->getDraggedEntries(),pTargetItems,DND_ACTION_COPY == rEvt.mnAction);

    return DND_ACTION_COPY;
}

IMPL_LINK_NOARG(FmFilterNavigator, SelectHdl, weld::TreeView&, void)
{
    std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_selected(xIter.get()))
        return;

    FmFilterData* pData = reinterpret_cast<FmFilterData*>(m_xTreeView->get_id(*xIter).toUInt64());

    FmFormItem* pFormItem = nullptr;
    if (FmFilterItem* pItem = dynamic_cast<FmFilterItem*>(pData))
        pFormItem = static_cast<FmFormItem*>(pItem->GetParent()->GetParent());
    else if (FmFilterItems* pItems = dynamic_cast<FmFilterItems*>(pData))
        pFormItem = static_cast<FmFormItem*>(pItems->GetParent()->GetParent());
    else
        pFormItem = dynamic_cast<FmFormItem*>(pData);

    if (pFormItem)
    {
        // will the controller be exchanged?
        if (FmFilterItem* pItem = dynamic_cast<FmFilterItem*>(pData))
            m_pModel->SetCurrentItems(static_cast<FmFilterItems*>(pItem->GetParent()));
        else if (FmFilterItems* pItems = dynamic_cast<FmFilterItems*>(pData))
            m_pModel->SetCurrentItems(pItems);
        else
            m_pModel->SetCurrentController(pFormItem->GetController());
    }
}

void FmFilterNavigator::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if (const FmFilterInsertedHint* pInsertHint = dynamic_cast<const FmFilterInsertedHint*>(&rHint))
    {
        Insert(pInsertHint->GetData(), pInsertHint->GetPos());
    }
    else if( dynamic_cast<const FilterClearingHint*>(&rHint) )
    {
        m_xTreeView->clear();
    }
    else if (const FmFilterRemovedHint* pRemoveHint = dynamic_cast<const FmFilterRemovedHint*>(&rHint))
    {
        Remove(pRemoveHint->GetData());
    }
    else if (const FmFilterTextChangedHint *pChangeHint = dynamic_cast<const FmFilterTextChangedHint*>(&rHint))
    {
        std::unique_ptr<weld::TreeIter> xEntry = FindEntry(pChangeHint->GetData());
        if (xEntry)
            m_xTreeView->set_text(*xEntry, pChangeHint->GetData()->GetText());
    }
    else if( dynamic_cast<const FmFilterCurrentChangedHint*>(&rHint) )
    {
        m_xTreeView->queue_draw();
    }
}

std::unique_ptr<weld::TreeIter> FmFilterNavigator::FindEntry(const FmFilterData* pItem) const
{
    if (!pItem)
        return nullptr;
    std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
    if (!m_xTreeView->get_iter_first(*xEntry))
        return nullptr;
    do
    {
        FmFilterData* pEntryItem = reinterpret_cast<FmFilterData*>(m_xTreeView->get_id(*xEntry).toUInt64());
        if (pEntryItem == pItem)
            return xEntry;
    }
    while (m_xTreeView->iter_next(*xEntry));

    return nullptr;
}

void FmFilterNavigator::Insert(FmFilterData* pItem, int nPos)
{
    const FmParentData* pParent = pItem->GetParent() ? pItem->GetParent() : m_pModel.get();

    // insert the item
    std::unique_ptr<weld::TreeIter> xParentEntry = FindEntry(pParent);

    OUString sId(OUString::number(reinterpret_cast<sal_uIntPtr>(pItem)));
    std::unique_ptr<weld::TreeIter> xRet(m_xTreeView->make_iterator());
    m_xTreeView->insert(xParentEntry.get(), nPos, &pItem->GetText(), &sId,
                        nullptr, nullptr, false, xRet.get());
    m_xTreeView->set_image(*xRet, pItem->GetImage());

    if (!xParentEntry)
        return;
    m_xTreeView->expand_row(*xParentEntry);
}

void FmFilterNavigator::EndEditing()
{
    if (m_xEditingCurrently)
    {
        // end editing
        m_xTreeView->end_editing();
        m_xEditingCurrently.reset();
    }
}

void FmFilterNavigator::Remove(FmFilterData const * pItem)
{
    // the entry for the data
    std::unique_ptr<weld::TreeIter> xEntry = FindEntry(pItem);
    if (!xEntry)
        return;

    if (m_xEditingCurrently && m_xTreeView->iter_compare(*xEntry, *m_xEditingCurrently) == 0)
        EndEditing();

    m_xTreeView->remove(*xEntry);
}

FmFormItem* FmFilterNavigator::getSelectedFilterItems(::std::vector<FmFilterItem*>& _rItemList)
{
    // be sure that the data is only used within only one form!
    FmFormItem* pFirstItem = nullptr;

    bool bHandled = true;
    bool bFoundSomething = false;

    m_xTreeView->selected_foreach([this, &bHandled, &bFoundSomething, &pFirstItem, &_rItemList](weld::TreeIter& rEntry) {
        FmFilterData* pFilterEntry = reinterpret_cast<FmFilterData*>(m_xTreeView->get_id(rEntry).toInt64());
        FmFilterItem* pFilter = dynamic_cast<FmFilterItem*>(pFilterEntry);
        if (pFilter)
        {
            FmFormItem* pForm = dynamic_cast<FmFormItem*>( pFilter->GetParent()->GetParent() );
            if (!pForm)
                bHandled = false;
            else if (!pFirstItem)
                pFirstItem = pForm;
            else if (pFirstItem != pForm)
                bHandled = false;

            if (bHandled)
            {
                _rItemList.push_back(pFilter);
                bFoundSomething = true;
            }
        }
        return !bHandled;
    });

    if ( !bHandled || !bFoundSomething )
        pFirstItem = nullptr;
    return pFirstItem;
}

void FmFilterNavigator::insertFilterItem(const ::std::vector<FmFilterItem*>& _rFilterList,FmFilterItems* _pTargetItems,bool _bCopy)
{
    for (FmFilterItem* pLookupItem : _rFilterList)
    {
        if ( pLookupItem->GetParent() == _pTargetItems )
            continue;

        FmFilterItem* pFilterItem = _pTargetItems->Find( pLookupItem->GetComponentIndex() );
        OUString aText = pLookupItem->GetText();
        if ( !pFilterItem )
        {
            pFilterItem = new FmFilterItem( _pTargetItems, pLookupItem->GetFieldName(), aText, pLookupItem->GetComponentIndex() );
            m_pModel->Append( _pTargetItems, std::unique_ptr<FmFilterItem>(pFilterItem) );
        }

        if ( !_bCopy )
            m_pModel->Remove( pLookupItem );

        // now set the text for the new dragged item
        m_pModel->SetTextForItem( pFilterItem, aText );
    }

    m_pModel->EnsureEmptyFilterRows( *_pTargetItems->GetParent() );
}

IMPL_LINK(FmFilterNavigator, DragBeginHdl, bool&, rUnsetDragIcon, bool)
{
    rUnsetDragIcon = false;

    // be sure that the data is only used within an only one form!
    m_aControlExchange.prepareDrag();

    ::std::vector<FmFilterItem*> aItemList;
    if (FmFormItem* pFirstItem = getSelectedFilterItems(aItemList))
    {
        m_aControlExchange->setDraggedEntries(aItemList);
        m_aControlExchange->setFormItem(pFirstItem);

        OFilterItemExchange& rExchange = *m_aControlExchange;
        rtl::Reference<TransferDataContainer> xHelper(&rExchange);
        m_xTreeView->enable_drag_source(xHelper, DND_ACTION_COPYMOVE);
        rExchange.setDragging(true);

        return false;
    }
    return true;
}

IMPL_LINK(FmFilterNavigator, PopupMenuHdl, const CommandEvent&, rEvt, bool)
{
    bool bHandled = false;
    switch (rEvt.GetCommand())
    {
        case CommandEventId::ContextMenu:
        {
            // the place where it was clicked
            Point aWhere;
            std::unique_ptr<weld::TreeIter> xClicked(m_xTreeView->make_iterator());
            if (rEvt.IsMouseEvent())
            {
                aWhere = rEvt.GetMousePosPixel();
                if (!m_xTreeView->get_dest_row_at_pos(aWhere, xClicked.get(), false))
                    break;

                if (!m_xTreeView->is_selected(*xClicked))
                {
                    m_xTreeView->unselect_all();
                    m_xTreeView->select(*xClicked);
                    m_xTreeView->set_cursor(*xClicked);
                }
            }
            else
            {
                if (!m_xTreeView->get_cursor(xClicked.get()))
                    break;
                aWhere = m_xTreeView->get_row_area(*xClicked).Center();
            }

            ::std::vector<FmFilterData*> aSelectList;
            m_xTreeView->selected_foreach([this, &aSelectList](weld::TreeIter& rEntry) {
                FmFilterData* pFilterEntry = reinterpret_cast<FmFilterData*>(m_xTreeView->get_id(rEntry).toInt64());

                // don't delete forms
                FmFormItem* pForm = dynamic_cast<FmFormItem*>(pFilterEntry);
                if (!pForm)
                    aSelectList.push_back(pFilterEntry);

                return false;
            });

            if (aSelectList.size() == 1)
            {
                // don't delete the only empty row of a form
                FmFilterItems* pFilterItems = dynamic_cast<FmFilterItems*>( aSelectList[0] );
                if (pFilterItems && pFilterItems->GetChildren().empty()
                    && pFilterItems->GetParent()->GetChildren().size() == 1)
                    aSelectList.clear();
            }

            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xTreeView.get(), "svx/ui/filtermenu.ui"));
            std::unique_ptr<weld::Menu> xContextMenu(xBuilder->weld_menu("menu"));

            // every condition could be deleted except the first one if it's the only one
            bool bNoDelete = false;
            if (aSelectList.empty())
            {
                bNoDelete = true;
                xContextMenu->remove("delete");
            }

            FmFilterData* pFilterEntry = reinterpret_cast<FmFilterData*>(m_xTreeView->get_id(*xClicked).toInt64());
            bool bEdit = dynamic_cast<FmFilterItem*>(pFilterEntry) != nullptr &&
                m_xTreeView->is_selected(*xClicked) && m_xTreeView->count_selected_rows() == 1;

            if (bNoDelete && !bEdit)
            {
                // nothing is in the menu, don't bother
                return true;
            }

            if (!bEdit)
            {
                xContextMenu->remove("edit");
                xContextMenu->remove("isnull");
                xContextMenu->remove("isnotnull");
            }

            OString sIdent = xContextMenu->popup_at_rect(m_xTreeView.get(), tools::Rectangle(aWhere, ::Size(1, 1)));
            if (sIdent == "edit")
            {
                m_xTreeView->start_editing(*xClicked);
            }
            else if (sIdent == "isnull")
            {
                OUString aErrorMsg;
                OUString aText = "IS NULL";
                m_pModel->ValidateText(static_cast<FmFilterItem*>(pFilterEntry),
                                        aText, aErrorMsg);
                m_pModel->SetTextForItem(static_cast<FmFilterItem*>(pFilterEntry), aText);
            }
            else if (sIdent == "isnotnull")
            {
                OUString aErrorMsg;
                OUString aText = "IS NOT NULL";

                m_pModel->ValidateText(static_cast<FmFilterItem*>(pFilterEntry),
                                        aText, aErrorMsg);
                m_pModel->SetTextForItem(static_cast<FmFilterItem*>(pFilterEntry), aText);
            }
            else if (sIdent == "delete")
            {
                DeleteSelection();
            }
            bHandled = true;
        }
        break;
        default: break;
    }

    return bHandled;
}

typedef std::vector<std::unique_ptr<weld::TreeIter>> iter_vector;

bool FmFilterNavigator::getNextEntry(weld::TreeIter& rEntry)
{
    bool bEntry = m_xTreeView->iter_next(rEntry);
    // we need the next filter entry
    if (bEntry)
    {
        while (!m_xTreeView->iter_has_child(rEntry))
        {
            std::unique_ptr<weld::TreeIter> xNext = m_xTreeView->make_iterator(&rEntry);
            if (!m_xTreeView->iter_next(*xNext))
                break;
            m_xTreeView->copy_iterator(*xNext, rEntry);
        }
    }
    return bEntry;
}

bool FmFilterNavigator::getPrevEntry(weld::TreeIter& rEntry)
{
    bool bEntry = m_xTreeView->iter_previous(rEntry);
    // check if the previous entry is a filter, if so get the next prev
    if (bEntry && m_xTreeView->iter_has_child(rEntry))
    {
        bEntry = m_xTreeView->iter_previous(rEntry);
        // if the entry is still no leaf return
        if (bEntry && m_xTreeView->iter_has_child(rEntry))
            bEntry = false;
    }
    return bEntry;
}
IMPL_LINK(FmFilterNavigator, KeyInputHdl, const ::KeyEvent&, rKEvt, bool)
{
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();

    switch ( rKeyCode.GetCode() )
    {
        case KEY_UP:
        case KEY_DOWN:
        {
            if ( !rKeyCode.IsMod1() || !rKeyCode.IsMod2() || rKeyCode.IsShift() )
                break;

            ::std::vector<FmFilterItem*> aItemList;
            if ( !getSelectedFilterItems( aItemList ) )
                break;


            iter_vector aSelected;
            m_xTreeView->selected_foreach([this, &aSelected](weld::TreeIter& rEntry){
                aSelected.emplace_back(m_xTreeView->make_iterator(&rEntry));
                return false;
            });

            std::unique_ptr<weld::TreeIter> xTarget;
            ::std::function<bool(FmFilterNavigator*, weld::TreeIter&)> getter;

            if (rKeyCode.GetCode() == KEY_UP)
            {
                xTarget = m_xTreeView->make_iterator(aSelected.front().get());
                getter = ::std::mem_fn(&FmFilterNavigator::getPrevEntry);
            }
            else
            {
                xTarget = m_xTreeView->make_iterator(aSelected.back().get());
                getter = ::std::mem_fn(&FmFilterNavigator::getNextEntry);
            }

            bool bTarget = getter(this, *xTarget);
            if (!bTarget)
                break;

            FmFilterItems* pTargetItems = getTargetItems(*m_xTreeView, *xTarget);
            if (!pTargetItems)
                break;

            ::std::vector<FmFilterItem*>::const_iterator aEnd = aItemList.end();
            bool bNextTargetItem = true;
            while ( bNextTargetItem )
            {
                ::std::vector<FmFilterItem*>::const_iterator i = aItemList.begin();
                for (; i != aEnd; ++i)
                {
                    if ( (*i)->GetParent() == pTargetItems )
                    {
                        bTarget = getter(this, *xTarget);
                        if (!bTarget)
                            return true;
                        pTargetItems = getTargetItems(*m_xTreeView, *xTarget);
                        break;
                    }
                    else
                    {
                        FmFilterItem* pFilterItem = pTargetItems->Find( (*i)->GetComponentIndex() );
                        // we found the text component so jump above
                        if ( pFilterItem )
                        {
                            bTarget = getter(this, *xTarget);
                            if (!bTarget)
                                return true;

                            pTargetItems = getTargetItems(*m_xTreeView, *xTarget);
                            break;
                        }
                    }
                }
                bNextTargetItem = i != aEnd && pTargetItems;
            }

            if ( pTargetItems )
            {
                insertFilterItem( aItemList, pTargetItems, false );
                return true;
            }
        }
        break;

        case KEY_DELETE:
        {
            if ( rKeyCode.GetModifier() )
                break;

            std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
            if (m_xTreeView->get_iter_first(*xEntry) && !m_xTreeView->is_selected(*xEntry))
                DeleteSelection();

            return true;
        }
    }

    return false;
}

void FmFilterNavigator::DeleteSelection()
{
    // to avoid the deletion of an entry twice (e.g. deletion of a parent and afterward
    // the deletion of its child, I have to shrink the selection list
    std::vector<FmFilterData*> aEntryList;

    m_xTreeView->selected_foreach([this, &aEntryList](weld::TreeIter& rEntry) {
        FmFilterData* pFilterEntry = reinterpret_cast<FmFilterData*>(m_xTreeView->get_id(rEntry).toInt64());

        if (dynamic_cast<FmFilterItem*>(pFilterEntry))
        {
            std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator(&rEntry));
            if (m_xTreeView->iter_parent(*xParent) && m_xTreeView->is_selected(*xParent))
                return false;
        }

        FmFormItem* pForm = dynamic_cast<FmFormItem*>(pFilterEntry);
        if (!pForm)
            aEntryList.emplace_back(pFilterEntry);

        return false;
    });

    // Remove the selection
    m_xTreeView->unselect_all();

    for (auto i = aEntryList.rbegin(); i != aEntryList.rend(); ++i)
        m_pModel->Remove(*i);
}

FmFilterNavigatorWin::FmFilterNavigatorWin(SfxBindings* _pBindings, SfxChildWindow* _pMgr,
                                           vcl::Window* _pParent)
    : SfxDockingWindow(_pBindings, _pMgr, _pParent, "FilterNavigator", "svx/ui/filternavigator.ui")
    , SfxControllerItem( SID_FM_FILTER_NAVIGATOR_CONTROL, *_pBindings )
    , m_xNavigatorTree(new FmFilterNavigator(this, m_xBuilder->weld_tree_view("treeview")))
{
    SetHelpId( HID_FILTER_NAVIGATOR_WIN );

    SetText( SvxResId(RID_STR_FILTER_NAVIGATOR) );
    SfxDockingWindow::SetFloatingSize( Size(200,200) );
}

FmFilterNavigatorWin::~FmFilterNavigatorWin()
{
    disposeOnce();
}

void FmFilterNavigatorWin::dispose()
{
    m_xNavigatorTree.reset();
    ::SfxControllerItem::dispose();
    SfxDockingWindow::dispose();
}

void FmFilterNavigatorWin::UpdateContent(FmFormShell const * pFormShell)
{
    if (!m_xNavigatorTree)
        return;

    if (!pFormShell)
        m_xNavigatorTree->UpdateContent( nullptr, nullptr );
    else
    {
        Reference<XFormController> const xController(pFormShell->GetImpl()->getActiveInternalController_Lock());
        Reference< XIndexAccess >   xContainer;
        if (xController.is())
        {
            Reference< XChild >  xChild = xController;
            for (Reference< XInterface >  xParent(xChild->getParent());
                 xParent.is();
                 xParent = xChild.is() ? xChild->getParent() : Reference< XInterface > ())
            {
                xContainer.set(xParent, UNO_QUERY);
                xChild.set(xParent, UNO_QUERY);
            }
        }
        m_xNavigatorTree->UpdateContent(xContainer, xController);
    }
}

void FmFilterNavigatorWin::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if( !pState  || SID_FM_FILTER_NAVIGATOR_CONTROL != nSID )
        return;

    if( eState >= SfxItemState::DEFAULT )
    {
        FmFormShell* pShell = dynamic_cast<FmFormShell*>( static_cast<const SfxObjectItem*>(pState)->GetShell()  );
        UpdateContent( pShell );
    }
    else
        UpdateContent( nullptr );
}

bool FmFilterNavigatorWin::Close()
{
    if (m_xNavigatorTree)
        m_xNavigatorTree->EndEditing();

    UpdateContent( nullptr );
    return SfxDockingWindow::Close();
}

void FmFilterNavigatorWin::FillInfo( SfxChildWinInfo& rInfo ) const
{
    SfxDockingWindow::FillInfo( rInfo );
    rInfo.bVisible = false;
}

Size FmFilterNavigatorWin::CalcDockingSize( SfxChildAlignment eAlign )
{
    if ( ( eAlign == SfxChildAlignment::TOP ) || ( eAlign == SfxChildAlignment::BOTTOM ) )
        return Size();

    return SfxDockingWindow::CalcDockingSize( eAlign );
}

SfxChildAlignment FmFilterNavigatorWin::CheckAlignment( SfxChildAlignment eActAlign, SfxChildAlignment eAlign )
{
    switch (eAlign)
    {
        case SfxChildAlignment::LEFT:
        case SfxChildAlignment::RIGHT:
        case SfxChildAlignment::NOALIGNMENT:
            return eAlign;
        default:
            break;
    }

    return eActAlign;
}

void FmFilterNavigatorWin::GetFocus()
{
    // oj #97405#
    if (m_xNavigatorTree)
        m_xNavigatorTree->GrabFocus();
    else
        SfxDockingWindow::GetFocus();
}

SFX_IMPL_DOCKINGWINDOW( FmFilterNavigatorWinMgr, SID_FM_FILTER_NAVIGATOR )


FmFilterNavigatorWinMgr::FmFilterNavigatorWinMgr( vcl::Window *_pParent, sal_uInt16 _nId,
                                    SfxBindings *_pBindings, SfxChildWinInfo* _pInfo )
                 :SfxChildWindow( _pParent, _nId )
{
    SetWindow( VclPtr<FmFilterNavigatorWin>::Create( _pBindings, this, _pParent ) );
    static_cast<SfxDockingWindow*>(GetWindow())->Initialize( _pInfo );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
