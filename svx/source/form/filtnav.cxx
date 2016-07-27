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

#include "filtnav.hxx"
#include "fmexch.hxx"
#include "fmhelp.hrc"
#include "fmitems.hxx"
#include "fmprop.hrc"
#include "svx/fmresids.hrc"

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>
#include <comphelper/uno3.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/sqlnode.hxx>
#include <cppuhelper/implbase.hxx>
#include <fmservs.hxx>
#include <fmshimp.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <svx/dialmgr.hxx>
#include <svx/fmshell.hxx>
#include <svx/fmtools.hxx>
#include <svx/svxids.hrc>
#include <vcl/wrkwin.hxx>
#include <vcl/settings.hxx>
#include <tools/diagnose_ex.h>
#include <svtools/svlbitm.hxx>
#include "svtools/treelistentry.hxx"
#include "svtools/viewdataentry.hxx"

#include <functional>

#define DROP_ACTION_TIMER_INITIAL_TICKS     10
    // solange dauert es, bis das Scrollen anspringt
#define DROP_ACTION_TIMER_SCROLL_TICKS      3
    // in diesen Intervallen wird jeweils eine Zeile gescrollt
#define DROP_ACTION_TIMER_TICK_BASE         10
    // das ist die Basis, mit der beide Angaben multipliziert werden (in ms)

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
    using ::com::sun::star::uno::RuntimeException;
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
    static SotClipboardFormatId s_nFormat = static_cast<SotClipboardFormatId>(-1);
    if (static_cast<SotClipboardFormatId>(-1) == s_nFormat)
    {
        s_nFormat = SotExchange::RegisterFormatName("application/x-openoffice;windows_formatname=\"form.FilterControlExchange\"");
        DBG_ASSERT(static_cast<SotClipboardFormatId>(-1) != s_nFormat, "OFilterExchangeHelper::getFormatId: bad exchange id!");
    }
    return s_nFormat;
}


OLocalExchange* OFilterExchangeHelper::createExchange() const
{
    return new OFilterItemExchange;
}


Image FmFilterData::GetImage() const
{
    return Image();
}


FmParentData::~FmParentData()
{
    for (::std::vector<FmFilterData*>::const_iterator i = m_aChildren.begin();
         i != m_aChildren.end(); ++i)
        delete (*i);
}


Image FmFormItem::GetImage() const
{
    ImageList aNavigatorImages( SVX_RES( RID_SVXIMGLIST_FMEXPL ) );
    return aNavigatorImages.GetImage( RID_SVXIMG_FORM );
}


FmFilterItem* FmFilterItems::Find( const ::sal_Int32 _nFilterComponentIndex ) const
{
    for (   ::std::vector< FmFilterData* >::const_iterator i = m_aChildren.begin();
            i != m_aChildren.end();
            ++i
        )
    {
        FmFilterData* pData = *i;
        FmFilterItem& rCondition = dynamic_cast<FmFilterItem&>(*pData);
        if ( _nFilterComponentIndex == rCondition.GetComponentIndex() )
            return &rCondition;
    }
    return nullptr;
}


Image FmFilterItems::GetImage() const
{
    ImageList aNavigatorImages( SVX_RES( RID_SVXIMGLIST_FMEXPL ) );
    return aNavigatorImages.GetImage( RID_SVXIMG_FILTER );
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


Image FmFilterItem::GetImage() const
{
    ImageList aNavigatorImages( SVX_RES( RID_SVXIMGLIST_FMEXPL ) );
    return aNavigatorImages.GetImage( RID_SVXIMG_FIELD );
}


// Hints for communication between model and view

class FmFilterHint : public SfxHint
{
    FmFilterData*   m_pData;

public:
    explicit FmFilterHint(FmFilterData* pData):m_pData(pData){}
    FmFilterData* GetData() const { return m_pData; }
};


class FmFilterInsertedHint : public FmFilterHint
{
    sal_uLong m_nPos;   // Position relative to the parent of the data

public:
    FmFilterInsertedHint(FmFilterData* pData, sal_uLong nRelPos)
        :FmFilterHint(pData)
        ,m_nPos(nRelPos){}

    sal_uLong GetPos() const { return m_nPos; }
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


// class FmFilterAdapter, Listener an den FilterControls

class FmFilterAdapter : public ::cppu::WeakImplHelper< XFilterControllerListener >
{
    FmFilterModel*              m_pModel;
    Reference< XIndexAccess >   m_xControllers;

public:
    FmFilterAdapter(FmFilterModel* pModel, const Reference< XIndexAccess >& xControllers);

// XEventListener
    virtual void SAL_CALL disposing(const EventObject& Source) throw( RuntimeException, std::exception ) override;

// XFilterControllerListener
    virtual void SAL_CALL predicateExpressionChanged( const FilterEvent& Event ) throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL disjunctiveTermRemoved( const FilterEvent& Event ) throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL disjunctiveTermAdded( const FilterEvent& Event ) throw (RuntimeException, std::exception) override;

// helpers
    void dispose() throw( RuntimeException );

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


void FmFilterAdapter::dispose() throw( RuntimeException )
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

    try
    {
        Reference< XFilterController > xController( pFormItem->GetController(), UNO_QUERY_THROW );
        xController->setPredicateExpression( pFilterItem->GetComponentIndex(), nRowPos, rText );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}


// XEventListener

void SAL_CALL FmFilterAdapter::disposing(const EventObject& /*e*/) throw( RuntimeException, std::exception )
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
            DBG_UNHANDLED_EXCEPTION();
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
            DBG_UNHANDLED_EXCEPTION();
        }
        return xField;
    }
}

// XFilterControllerListener

void FmFilterAdapter::predicateExpressionChanged( const FilterEvent& Event ) throw( RuntimeException, std::exception )
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

    FmFilterData* pData = pFormItem->GetChildren()[nActiveTerm];
    FmFilterItems& rFilter = dynamic_cast<FmFilterItems&>(*pData);
    FmFilterItem* pFilterItem = rFilter.Find( Event.FilterComponent );
    if ( pFilterItem )
    {
        if ( !Event.PredicateExpression.isEmpty())
        {
            pFilterItem->SetText( Event.PredicateExpression );
            // UI benachrichtigen
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

        pFilterItem = new FmFilterItem(&rFilter, aFieldName, Event.PredicateExpression, Event.FilterComponent);
        m_pModel->Insert(rFilter.GetChildren().end(), pFilterItem);
    }

    // ensure there's one empty term in the filter, just in case the active term was previously empty
    m_pModel->EnsureEmptyFilterRows( *pFormItem );
}


void SAL_CALL FmFilterAdapter::disjunctiveTermRemoved( const FilterEvent& Event ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Reference< XFormController > xController( Event.Source, UNO_QUERY_THROW );
    Reference< XFilterController > xFilterController( Event.Source, UNO_QUERY_THROW );
    Reference< XForm > xForm( xController->getModel(), UNO_QUERY_THROW );

    FmFormItem* pFormItem = m_pModel->Find( m_pModel->m_aChildren, xForm );
    OSL_ENSURE( pFormItem, "FmFilterAdapter::disjunctiveTermRemoved: don't know this form!" );
    if ( !pFormItem )
        return;

    ::std::vector< FmFilterData* >& rTermItems = pFormItem->GetChildren();
    const bool bValidIndex = ( Event.DisjunctiveTerm >= 0 ) && ( (size_t)Event.DisjunctiveTerm < rTermItems.size() );
    OSL_ENSURE( bValidIndex, "FmFilterAdapter::disjunctiveTermRemoved: invalid term index!" );
    if ( !bValidIndex )
        return;

    // if the first term was removed, then the to-be first term needs its text updated
    if ( Event.DisjunctiveTerm == 0 )
    {
        rTermItems[1]->SetText( SVX_RESSTR(RID_STR_FILTER_FILTER_FOR));
        FmFilterTextChangedHint aChangeHint( rTermItems[1] );
        m_pModel->Broadcast( aChangeHint );
    }

    // finally remove the entry from the model
    m_pModel->Remove( rTermItems.begin() + Event.DisjunctiveTerm );

    // ensure there's one empty term in the filter, just in case the currently removed one was the last empty one
    m_pModel->EnsureEmptyFilterRows( *pFormItem );
}


void SAL_CALL FmFilterAdapter::disjunctiveTermAdded( const FilterEvent& Event ) throw (RuntimeException, std::exception)
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
    bool bValidIndex = ( nInsertPos >= 0 ) && ( (size_t)nInsertPos <= pFormItem->GetChildren().size() );
    if ( !bValidIndex )
    {
        OSL_FAIL( "FmFilterAdapter::disjunctiveTermAdded: invalid index!" );
        return;
    }

    const ::std::vector< FmFilterData* >::iterator insertPos = pFormItem->GetChildren().begin() + nInsertPos;

    FmFilterItems* pFilterItems = new FmFilterItems(pFormItem, SVX_RESSTR(RID_STR_FILTER_FILTER_OR));
    m_pModel->Insert( insertPos, pFilterItems );
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

    for (::std::vector<FmFilterData*>::const_iterator i = m_aChildren.begin();
         i != m_aChildren.end(); ++i)
        delete (*i);

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
            Insert( pParent->GetChildren().end(), pFormItem );

            Reference< XFilterController > xFilterController( pFormItem->GetFilterController(), UNO_SET_THROW );

            // insert the existing filters for the form
            OUString aTitle(SVX_RESSTR(RID_STR_FILTER_FILTER_FOR));

            Sequence< Sequence< OUString > > aExpressions = xFilterController->getPredicateExpressions();
            for (   const Sequence< OUString >* pConjunctionTerm = aExpressions.getConstArray();
                    pConjunctionTerm != aExpressions.getConstArray() + aExpressions.getLength();
                    ++pConjunctionTerm
                )
            {
                // we always display one row, even if there's no term to be displayed
                FmFilterItems* pFilterItems = new FmFilterItems( pFormItem, aTitle );
                Insert( pFormItem->GetChildren().end(), pFilterItems );

                const Sequence< OUString >& rDisjunction( *pConjunctionTerm );
                for (   const OUString* pDisjunctiveTerm = rDisjunction.getConstArray();
                        pDisjunctiveTerm != rDisjunction.getConstArray() + rDisjunction.getLength();
                        ++pDisjunctiveTerm
                    )
                {
                    if ( pDisjunctiveTerm->isEmpty() )
                        // no condition for this particular component in this particular conjunction term
                        continue;

                    const sal_Int32 nComponentIndex = pDisjunctiveTerm - rDisjunction.getConstArray();

                    // determine the display name of the control
                    const Reference< XControl > xFilterControl( xFilterController->getFilterComponent( nComponentIndex ) );
                    const OUString sDisplayName( lcl_getLabelName_nothrow( xFilterControl ) );

                    // insert a new entry
                    FmFilterItem* pANDCondition = new FmFilterItem( pFilterItems, sDisplayName, *pDisjunctiveTerm, nComponentIndex );
                    Insert( pFilterItems->GetChildren().end(), pANDCondition );
                }

                // title for the next conditions
                aTitle = SVX_RESSTR( RID_STR_FILTER_FILTER_OR );
            }

            // now add dependent controllers
            Update( xController, pFormItem );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}


FmFormItem* FmFilterModel::Find(const ::std::vector<FmFilterData*>& rItems, const Reference< XFormController > & xController) const
{
    for (::std::vector<FmFilterData*>::const_iterator i = rItems.begin();
         i != rItems.end(); ++i)
    {
        FmFormItem* pForm = dynamic_cast<FmFormItem*>( *i );
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


FmFormItem* FmFilterModel::Find(const ::std::vector<FmFilterData*>& rItems, const Reference< XForm >& xForm) const
{
    for (::std::vector<FmFilterData*>::const_iterator i = rItems.begin();
         i != rItems.end(); ++i)
    {
        FmFormItem* pForm = dynamic_cast<FmFormItem*>( *i );
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
        if ( pItem->GetChildren().size() > (size_t)nActiveTerm )
        {
            SetCurrentItems( static_cast< FmFilterItems* >( pItem->GetChildren()[ nActiveTerm ] ) );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}


void FmFilterModel::AppendFilterItems( FmFormItem& _rFormItem )
{
    // insert the condition behind the last filter items
    ::std::vector<FmFilterData*>::const_reverse_iterator aEnd = _rFormItem.GetChildren().rend();
    ::std::vector<FmFilterData*>::reverse_iterator iter;
    for (   iter = _rFormItem.GetChildren().rbegin();
            iter != aEnd;
            ++iter
        )
    {
        if (dynamic_cast<const FmFilterItems*>(*iter) !=  nullptr)
            break;
    }

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
        DBG_UNHANDLED_EXCEPTION();
    }
}


void FmFilterModel::Insert(const ::std::vector<FmFilterData*>::iterator& rPos, FmFilterData* pData)
{
    ::std::vector<FmFilterData*>& rItems = pData->GetParent()->GetChildren();
    sal_uLong nPos = rPos == rItems.end() ? CONTAINER_APPEND : rPos - rItems.begin();
    if (nPos == CONTAINER_APPEND)
    {
        rItems.push_back(pData);
        nPos = rItems.size() - 1;
    }
    else
    {
        rItems.insert(rPos, pData);
    }

    // UI benachrichtigen
    FmFilterInsertedHint aInsertedHint(pData, nPos);
    Broadcast( aInsertedHint );
}

void FmFilterModel::Remove(FmFilterData* pData)
{
    FmParentData* pParent = pData->GetParent();
    ::std::vector<FmFilterData*>& rItems = pParent->GetChildren();

    // erase the item from the model
    ::std::vector<FmFilterData*>::iterator i = ::std::find(rItems.begin(), rItems.end(), pData);
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
                ::std::vector< FmFilterData* >& rChildren = static_cast<FmFilterItems*>(pData)->GetChildren();
                while ( !rChildren.empty() )
                {
                    ::std::vector< FmFilterData* >::iterator removePos = rChildren.end() - 1;
                    if (FmFilterItem* pFilterItem = dynamic_cast<FmFilterItem*>( *removePos))
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
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    else // FormItems can not be deleted
    {
        FmFilterItem& rFilterItem = dynamic_cast<FmFilterItem&>(*pData);

        // if its the last condition remove the parent
        if (rItems.size() == 1)
            Remove(rFilterItem.GetParent());
        else
        {
            // find the position of the father within his father
            ::std::vector<FmFilterData*>& rParentParentItems = pData->GetParent()->GetParent()->GetChildren();
            ::std::vector<FmFilterData*>::iterator j = ::std::find(rParentParentItems.begin(), rParentParentItems.end(), rFilterItem.GetParent());
            DBG_ASSERT(j != rParentParentItems.end(), "FmFilterModel::Remove(): unknown Item");
            sal_Int32 nParentPos = j - rParentParentItems.begin();

            // EmptyText removes the filter
            FmFilterAdapter::setText(nParentPos, &rFilterItem, OUString());
            Remove( i );
        }
    }
}

void FmFilterModel::Remove( const ::std::vector<FmFilterData*>::iterator& rPos )
{
    // remove from parent's child list
    FmFilterData* pData = *rPos;
    pData->GetParent()->GetChildren().erase( rPos );

    // notify the view, this will remove the actual SvTreeListEntry
    FmFilterRemovedHint aRemoveHint( pData );
    Broadcast( aRemoveHint );

    delete pData;
}


bool FmFilterModel::ValidateText(FmFilterItem* pItem, OUString& rText, OUString& rErrorMsg) const
{
    FmFormItem* pFormItem = dynamic_cast<FmFormItem*>( pItem->GetParent()->GetParent()  );
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
        std::shared_ptr< OSQLParseNode > pParseNode = predicateTree( aErr, aTxt, xFormatter, xField );
        rErrorMsg = aErr;
        rText = aTxt;
        if ( pParseNode != nullptr )
        {
            OUString aPreparedText;
            Locale aAppLocale = Application::GetSettings().GetUILanguageTag().getLocale();
            pParseNode->parseNodeToPredicateStr(
                aPreparedText, xConnection, xFormatter, xField, OUString(), aAppLocale, '.', getParseContext() );
            rText = aPreparedText;
            return true;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return false;
}


void FmFilterModel::Append(FmFilterItems* pItems, FmFilterItem* pFilterItem)
{
    Insert(pItems->GetChildren().end(), pFilterItem);
}


void FmFilterModel::SetTextForItem(FmFilterItem* pItem, const OUString& rText)
{
    ::std::vector<FmFilterData*>& rItems = pItem->GetParent()->GetParent()->GetChildren();
    ::std::vector<FmFilterData*>::const_iterator i = ::std::find(rItems.begin(), rItems.end(), pItem->GetParent());
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
        ::std::vector<FmFilterData*>& rItems = pFormItem->GetChildren();
        ::std::vector<FmFilterData*>::const_iterator i = ::std::find(rItems.begin(), rItems.end(), pCurrent);

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
                DBG_UNHANDLED_EXCEPTION();
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


    // UI benachrichtigen
    FmFilterCurrentChangedHint aHint;
    Broadcast( aHint );
}


void FmFilterModel::EnsureEmptyFilterRows( FmParentData& _rItem )
{
    // checks whether for each form there's one free level for input
    ::std::vector< FmFilterData* >& rChildren = _rItem.GetChildren();
    bool bAppendLevel = dynamic_cast<const FmFormItem*>(&_rItem) !=  nullptr;
    ::std::vector<FmFilterData*>::const_iterator aEnd = rChildren.end();

    for (   ::std::vector<FmFilterData*>::iterator i = rChildren.begin();
            i != aEnd;
            ++i
        )
    {
        FmFilterItems* pItems = dynamic_cast<FmFilterItems*>( *i );
        if ( pItems && pItems->GetChildren().empty() )
        {
            bAppendLevel = false;
            break;
        }

        FmFormItem* pFormItem = dynamic_cast<FmFormItem*>( *i );
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

class FmFilterItemsString : public SvLBoxString
{
public:
    explicit FmFilterItemsString(const OUString& rStr)
        : SvLBoxString(rStr)
    {
    }

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
    virtual void InitViewData( SvTreeListBox* pView,SvTreeListEntry* pEntry, SvViewDataItem* pViewData = nullptr) override;
};

const int nxDBmp = 12;

void FmFilterItemsString::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                                const SvViewDataEntry* /*pView*/, const SvTreeListEntry& rEntry)
{
    FmFilterItems* pRow = static_cast<FmFilterItems*>(rEntry.GetUserData());
    FmFormItem* pForm = static_cast<FmFormItem*>(pRow->GetParent());

    // current filter is significant painted
    const bool bIsCurrentFilter = pForm->GetChildren()[ pForm->GetFilterController()->getActiveTerm() ] == pRow;
    if (bIsCurrentFilter)
    {
        rRenderContext.Push(PushFlags::LINECOLOR);
        rRenderContext.SetLineColor(rRenderContext.GetTextColor());

        Rectangle aRect(rPos, GetSize(&rDev, &rEntry));
        Point aFirst(rPos.X(), aRect.Bottom() - 6);
        Point aSecond(aFirst .X() + 2, aFirst.Y() + 3);

        rRenderContext.DrawLine(aFirst, aSecond);

        aFirst = aSecond;
        aFirst.X() += 1;
        aSecond.X() += 6;
        aSecond.Y() -= 5;

        rRenderContext.DrawLine(aFirst, aSecond);
        rRenderContext.Pop();
    }

    rRenderContext.DrawText(Point(rPos.X() + nxDBmp, rPos.Y()), GetText());
}


void FmFilterItemsString::InitViewData( SvTreeListBox* pView,SvTreeListEntry* pEntry, SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );

    Size aSize(pView->GetTextWidth(GetText()), pView->GetTextHeight());
    aSize.Width() += nxDBmp;
    pViewData->maSize = aSize;
}

class FmFilterString : public SvLBoxString
{
    OUString m_aName;

public:
    FmFilterString( const OUString& rStr, const OUString& aName)
        : SvLBoxString(rStr)
        , m_aName(aName)
    {
        m_aName += ": ";
    }

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
    virtual void InitViewData( SvTreeListBox* pView,SvTreeListEntry* pEntry, SvViewDataItem* pViewData = nullptr) override;
};

const int nxD = 4;


void FmFilterString::InitViewData( SvTreeListBox* pView,SvTreeListEntry* pEntry, SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );

    vcl::Font aOldFont( pView->GetFont());
    vcl::Font aFont( aOldFont );
    aFont.SetWeight(WEIGHT_BOLD);
    pView->Control::SetFont( aFont );

    Size aSize(pView->GetTextWidth(m_aName), pView->GetTextHeight());
    pView->Control::SetFont( aOldFont );
    aSize.Width() += pView->GetTextWidth(GetText()) + nxD;
    pViewData->maSize = aSize;
}


void FmFilterString::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                           const SvViewDataEntry* /*pView*/, const SvTreeListEntry& /*rEntry*/)
{
    rRenderContext.Push(PushFlags::FONT);
    vcl::Font aFont(rRenderContext.GetFont());
    aFont.SetWeight(WEIGHT_BOLD);
    rRenderContext.SetFont(aFont);

    Point aPos(rPos);
    rRenderContext.DrawText(aPos, m_aName);

    // position for the second text
    aPos.X() += rDev.GetTextWidth(m_aName) + nxD;
    rRenderContext.Pop();
    rDev.DrawText(aPos, GetText());
}

FmFilterNavigator::FmFilterNavigator( vcl::Window* pParent )
                  :SvTreeListBox( pParent, WB_HASBUTTONS|WB_HASLINES|WB_BORDER|WB_HASBUTTONSATROOT )
                  ,m_pModel( nullptr )
                  ,m_pEditingCurrently( nullptr )
                  ,m_aControlExchange( this )
                  ,m_aTimerCounter( 0 )
                  ,m_aDropActionType( DA_SCROLLUP )
{
    SetHelpId( HID_FILTER_NAVIGATOR );

    {
        ImageList aNavigatorImages( SVX_RES( RID_SVXIMGLIST_FMEXPL ) );
        SetNodeBitmaps(
            aNavigatorImages.GetImage( RID_SVXIMG_COLLAPSEDNODE ),
            aNavigatorImages.GetImage( RID_SVXIMG_EXPANDEDNODE )
        );
    }

    m_pModel = new FmFilterModel();
    StartListening( *m_pModel );

    EnableInplaceEditing( true );
    SetSelectionMode(SelectionMode::Multiple);

    SetDragDropMode(DragDropMode::ALL);

    m_aDropActionTimer.SetTimeoutHdl(LINK(this, FmFilterNavigator, OnDropActionTimer));
}


FmFilterNavigator::~FmFilterNavigator()
{
    disposeOnce();
}

void FmFilterNavigator::dispose()
{
    EndListening( *m_pModel );
    delete m_pModel;
    SvTreeListBox::dispose();
}


void FmFilterNavigator::UpdateContent(const Reference< XIndexAccess > & xControllers, const Reference< XFormController > & xCurrent)
{
    if (xCurrent == m_pModel->GetCurrentController())
        return;

    m_pModel->Update(xControllers, xCurrent);

    // expand the filters for the current controller
    SvTreeListEntry* pEntry = FindEntry(m_pModel->GetCurrentForm());
    if (pEntry && !IsExpanded(pEntry))
    {
        SelectAll(false);

        if (!IsExpanded(pEntry))
            Expand(pEntry);

        pEntry = FindEntry(m_pModel->GetCurrentItems());
        if (pEntry)
        {
            if (!IsExpanded(pEntry))
                Expand(pEntry);
            Select(pEntry);
        }
    }
}


bool FmFilterNavigator::EditingEntry( SvTreeListEntry* pEntry, Selection& rSelection )
{
    m_pEditingCurrently = pEntry;
    if (!SvTreeListBox::EditingEntry( pEntry, rSelection ))
        return false;

    return pEntry && dynamic_cast<const FmFilterItem*>(static_cast<FmFilterData*>(pEntry->GetUserData())) == nullptr;
}


bool FmFilterNavigator::EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText )
{
    DBG_ASSERT(pEntry == m_pEditingCurrently, "FmFilterNavigator::EditedEntry: suspicious entry!");
    m_pEditingCurrently = nullptr;

    if (EditingCanceled())
        return true;

    DBG_ASSERT(dynamic_cast<const FmFilterItem*>(static_cast<FmFilterData*>(pEntry->GetUserData())) != nullptr,
                    "FmFilterNavigator::EditedEntry() wrong entry");

    OUString aText(comphelper::string::strip(rNewText, ' '));
    if (aText.isEmpty())
    {
        // deleting the entry asynchron
        PostUserEvent(LINK(this, FmFilterNavigator, OnRemove), pEntry, true);
    }
    else
    {
        OUString aErrorMsg;

        if (m_pModel->ValidateText(static_cast<FmFilterItem*>(pEntry->GetUserData()), aText, aErrorMsg))
        {
            GrabFocus();
            // this will set the text at the FmFilterItem, as well as update any filter controls
            // which are connected to this particular entry
            m_pModel->SetTextForItem( static_cast< FmFilterItem* >( pEntry->GetUserData() ), aText );

            SetCursor( pEntry, true );
            SetEntryText( pEntry, aText );
        }
        else
        {
            // display the error and return sal_False
            SQLContext aError;
            aError.Message = SVX_RESSTR(RID_STR_SYNTAXERROR);
            aError.Details = aErrorMsg;
            displayException(aError, this);

            return false;
        }
    }
    return true;
}


IMPL_LINK_TYPED( FmFilterNavigator, OnRemove, void*, p, void )
{
    SvTreeListEntry* pEntry = static_cast<SvTreeListEntry*>(p);
    // now remove the entry
    m_pModel->Remove(static_cast<FmFilterData*>(pEntry->GetUserData()));
}


IMPL_LINK_NOARG_TYPED(FmFilterNavigator, OnDropActionTimer, Timer *, void)
{
    if (--m_aTimerCounter > 0)
        return;

    switch (m_aDropActionType)
    {
        case DA_SCROLLUP :
            ScrollOutputArea(1);
            m_aTimerCounter = DROP_ACTION_TIMER_SCROLL_TICKS;
            break;
        case DA_SCROLLDOWN :
            ScrollOutputArea(-1);
            m_aTimerCounter = DROP_ACTION_TIMER_SCROLL_TICKS;
            break;
        case DA_EXPANDNODE:
        {
            SvTreeListEntry* pToExpand = GetEntry(m_aTimerTriggered);
            if (pToExpand && (GetChildCount(pToExpand) > 0) &&  !IsExpanded(pToExpand))
                Expand(pToExpand);
            m_aDropActionTimer.Stop();
        }
        break;
    }
}


sal_Int8 FmFilterNavigator::AcceptDrop( const AcceptDropEvent& rEvt )
{
    Point aDropPos = rEvt.maPosPixel;

    // possible DropActions scroll and expand
    if (rEvt.mbLeaving)
    {
        if (m_aDropActionTimer.IsActive())
            m_aDropActionTimer.Stop();
    }
    else
    {
        bool bNeedTrigger = false;
        // first entry ?
        if ((aDropPos.Y() >= 0) && (aDropPos.Y() < GetEntryHeight()))
        {
            m_aDropActionType = DA_SCROLLUP;
            bNeedTrigger = true;
        }
        else
        {
            if ((aDropPos.Y() < GetSizePixel().Height()) && (aDropPos.Y() >= GetSizePixel().Height() - GetEntryHeight()))
            {
                m_aDropActionType = DA_SCROLLDOWN;
                bNeedTrigger = true;
            }
            else
            {   // is it an entry with children, and not yet expanded?
                SvTreeListEntry* pDropppedOn = GetEntry(aDropPos);
                if (pDropppedOn && (GetChildCount(pDropppedOn) > 0) && !IsExpanded(pDropppedOn))
                {
                    // -> expand
                    m_aDropActionType = DA_EXPANDNODE;
                    bNeedTrigger = true;
                }
            }
        }
        if (bNeedTrigger && (m_aTimerTriggered != aDropPos))
        {
            m_aTimerCounter = DROP_ACTION_TIMER_INITIAL_TICKS;
            // remember DropPos because there are QueryDrops even though the mouse was not moved
            m_aTimerTriggered = aDropPos;
            if (!m_aDropActionTimer.IsActive())
            {
                m_aDropActionTimer.SetTimeout(DROP_ACTION_TIMER_TICK_BASE);
                m_aDropActionTimer.Start();
            }
        }
        else if (!bNeedTrigger)
            m_aDropActionTimer.Stop();
    }

    if (!m_aControlExchange.isDragSource())
        return DND_ACTION_NONE;

    if (!OFilterItemExchange::hasFormat(GetDataFlavorExVector()))
        return DND_ACTION_NONE;

    // do we contain the formitem?
    if (!FindEntry(m_aControlExchange->getFormItem()))
        return DND_ACTION_NONE;

    SvTreeListEntry* pDropTarget = GetEntry(aDropPos);
    if (!pDropTarget)
        return DND_ACTION_NONE;

    FmFilterData* pData = static_cast<FmFilterData*>(pDropTarget->GetUserData());
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
    FmFilterItems* getTargetItems(SvTreeListEntry* _pTarget)
    {
        FmFilterData*   pData = static_cast<FmFilterData*>(_pTarget->GetUserData());
        FmFilterItems*  pTargetItems = dynamic_cast<FmFilterItems*>(pData);
        if (!pTargetItems)
            pTargetItems = dynamic_cast<FmFilterItems*>(pData->GetParent());
        return pTargetItems;
    }
}

sal_Int8 FmFilterNavigator::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    // you can't scroll after dropping...
    if (m_aDropActionTimer.IsActive())
        m_aDropActionTimer.Stop();

    if (!m_aControlExchange.isDragSource())
        return DND_ACTION_NONE;

    Point aDropPos = rEvt.maPosPixel;
    SvTreeListEntry* pDropTarget = GetEntry( aDropPos );
    if (!pDropTarget)
        return DND_ACTION_NONE;

    // search the container where to add the items
    FmFilterItems*  pTargetItems = getTargetItems(pDropTarget);
    SelectAll(false);
    SvTreeListEntry* pEntry = FindEntry(pTargetItems);
    Select(pEntry);
    SetCurEntry(pEntry);

    insertFilterItem(m_aControlExchange->getDraggedEntries(),pTargetItems,DND_ACTION_COPY == rEvt.mnAction);

    return DND_ACTION_COPY;
}


void FmFilterNavigator::InitEntry(SvTreeListEntry* pEntry,
                                  const OUString& rStr,
                                  const Image& rImg1,
                                  const Image& rImg2,
                                  SvLBoxButtonKind eButtonKind)
{
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2, eButtonKind );
    std::unique_ptr<SvLBoxString> pString;

    if (dynamic_cast<const FmFilterItem*>(static_cast<FmFilterData*>(pEntry->GetUserData())) != nullptr)
        pString.reset(new FmFilterString(rStr,
            static_cast<FmFilterItem*>(pEntry->GetUserData())->GetFieldName()));
    else if (dynamic_cast<const FmFilterItems*>(static_cast<FmFilterData*>(pEntry->GetUserData())) != nullptr)
        pString.reset(new FmFilterItemsString(rStr));

    if (pString)
        pEntry->ReplaceItem(std::move(pString), 1 );
}


bool FmFilterNavigator::Select( SvTreeListEntry* pEntry, bool bSelect )
{
    if (bSelect == IsSelected(pEntry))  // das passiert manchmal, ich glaube, die Basisklasse geht zu sehr auf Nummer sicher ;)
        return true;

    if (SvTreeListBox::Select(pEntry, bSelect))
    {
        if (bSelect)
        {
            FmFormItem* pFormItem = nullptr;
            if ( dynamic_cast<const FmFilterItem*>(static_cast<FmFilterData*>(pEntry->GetUserData())) != nullptr)
                pFormItem = static_cast<FmFormItem*>(static_cast<FmFilterItem*>(pEntry->GetUserData())->GetParent()->GetParent());
            else if (dynamic_cast<const FmFilterItems*>(static_cast<FmFilterData*>(pEntry->GetUserData())) != nullptr)
                pFormItem = static_cast<FmFormItem*>(static_cast<FmFilterItem*>(pEntry->GetUserData())->GetParent()->GetParent());
            else if (dynamic_cast<const FmFormItem*>(static_cast<FmFilterData*>(pEntry->GetUserData())) != nullptr)
                pFormItem = static_cast<FmFormItem*>(pEntry->GetUserData());

            if (pFormItem)
            {
                // will the controller be exchanged?
                if (dynamic_cast<const FmFilterItem*>(static_cast<FmFilterData*>(pEntry->GetUserData())) != nullptr)
                    m_pModel->SetCurrentItems(static_cast<FmFilterItems*>(static_cast<FmFilterItem*>(pEntry->GetUserData())->GetParent()));
                else if (dynamic_cast<const FmFilterItems*>(static_cast<FmFilterData*>(pEntry->GetUserData())) != nullptr)
                    m_pModel->SetCurrentItems(static_cast<FmFilterItems*>(pEntry->GetUserData()));
                else if (dynamic_cast<const FmFormItem*>(static_cast<FmFilterData*>(pEntry->GetUserData())) != nullptr)
                    m_pModel->SetCurrentController(static_cast<FmFormItem*>(pEntry->GetUserData())->GetController());
            }
        }
        return true;
    }
    else
        return false;
}

void FmFilterNavigator::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if (const FmFilterInsertedHint* pInsertHint = dynamic_cast<const FmFilterInsertedHint*>(&rHint))
    {
        Insert(pInsertHint->GetData(), pInsertHint->GetPos());
    }
    else if( dynamic_cast<const FilterClearingHint*>(&rHint) )
    {
        SvTreeListBox::Clear();
    }
    else if (const FmFilterRemovedHint* pRemoveHint = dynamic_cast<const FmFilterRemovedHint*>(&rHint))
    {
        Remove(pRemoveHint->GetData());
    }
    else if (const FmFilterTextChangedHint *pChangeHint = dynamic_cast<const FmFilterTextChangedHint*>(&rHint))
    {
        SvTreeListEntry* pEntry = FindEntry(pChangeHint->GetData());
        if (pEntry)
            SetEntryText( pEntry, pChangeHint->GetData()->GetText());
    }
    else if( dynamic_cast<const FmFilterCurrentChangedHint*>(&rHint) )
    {
        // invalidate the entries
        for (SvTreeListEntry* pEntry = First(); pEntry != nullptr;
             pEntry = Next(pEntry))
            GetModel()->InvalidateEntry( pEntry );
    }
}

SvTreeListEntry* FmFilterNavigator::FindEntry(const FmFilterData* pItem) const
{
    SvTreeListEntry* pEntry = nullptr;
    if (pItem)
    {
        for (pEntry = First(); pEntry != nullptr; pEntry = Next( pEntry ))
        {
            FmFilterData* pEntryItem = static_cast<FmFilterData*>(pEntry->GetUserData());
            if (pEntryItem == pItem)
                break;
        }
    }
    return pEntry;
}


void FmFilterNavigator::Insert(FmFilterData* pItem, sal_uLong nPos)
{
    const FmParentData* pParent = pItem->GetParent() ? pItem->GetParent() : GetFilterModel();

    // insert the item
    SvTreeListEntry* pParentEntry = FindEntry( pParent );
    InsertEntry( pItem->GetText(), pItem->GetImage(), pItem->GetImage(), pParentEntry, false, nPos, pItem );
    if ( pParentEntry )
        Expand( pParentEntry );
}


void FmFilterNavigator::Remove(FmFilterData* pItem)
{
    // der Entry zu den Daten
    SvTreeListEntry* pEntry = FindEntry(pItem);

    if (pEntry == m_pEditingCurrently)
        // cancel editing
        EndEditing(true);

    if (pEntry)
        GetModel()->Remove( pEntry );
}

FmFormItem* FmFilterNavigator::getSelectedFilterItems(::std::vector<FmFilterItem*>& _rItemList)
{
    // be sure that the data is only used within only one form!
    FmFormItem* pFirstItem = nullptr;

    bool bHandled = true;
    bool bFoundSomething = false;
    for (SvTreeListEntry* pEntry = FirstSelected();
         bHandled && pEntry != nullptr;
         pEntry = NextSelected(pEntry))
    {
        FmFilterItem* pFilter = dynamic_cast<FmFilterItem*>( static_cast<FmFilterData*>(pEntry->GetUserData()) );
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
    }
    if ( !bHandled || !bFoundSomething )
        pFirstItem = nullptr;
    return pFirstItem;
}

void FmFilterNavigator::insertFilterItem(const ::std::vector<FmFilterItem*>& _rFilterList,FmFilterItems* _pTargetItems,bool _bCopy)
{
    ::std::vector<FmFilterItem*>::const_iterator aEnd = _rFilterList.end();
    for (   ::std::vector< FmFilterItem* >::const_iterator i = _rFilterList.begin();
            i != aEnd;
            ++i
        )
    {
        FmFilterItem* pLookupItem( *i );
        if ( pLookupItem->GetParent() == _pTargetItems )
            continue;

        FmFilterItem* pFilterItem = _pTargetItems->Find( pLookupItem->GetComponentIndex() );
        OUString aText = pLookupItem->GetText();
        if ( !pFilterItem )
        {
            pFilterItem = new FmFilterItem( _pTargetItems, pLookupItem->GetFieldName(), aText, pLookupItem->GetComponentIndex() );
            m_pModel->Append( _pTargetItems, pFilterItem );
        }

        if ( !_bCopy )
            m_pModel->Remove( pLookupItem );

        // now set the text for the new dragged item
        m_pModel->SetTextForItem( pFilterItem, aText );
    }

    m_pModel->EnsureEmptyFilterRows( *_pTargetItems->GetParent() );
}


void FmFilterNavigator::StartDrag( sal_Int8 /*_nAction*/, const Point& /*_rPosPixel*/ )
{
    EndSelection();

    // be sure that the data is only used within a only one form!
    m_aControlExchange.prepareDrag();

    ::std::vector<FmFilterItem*> aItemList;
    if ( FmFormItem* pFirstItem = getSelectedFilterItems(aItemList) )
    {
        m_aControlExchange->setDraggedEntries(aItemList);
        m_aControlExchange->setFormItem(pFirstItem);
        m_aControlExchange.startDrag( DND_ACTION_COPYMOVE );
    }
}


void FmFilterNavigator::Command( const CommandEvent& rEvt )
{
    bool bHandled = false;
    switch (rEvt.GetCommand())
    {
        case CommandEventId::ContextMenu:
        {
            // die Stelle, an der geklickt wurde
            Point aWhere;
            SvTreeListEntry* pClicked = nullptr;
            if (rEvt.IsMouseEvent())
            {
                aWhere = rEvt.GetMousePosPixel();
                pClicked = GetEntry(aWhere);
                if (pClicked == nullptr)
                    break;

                if (!IsSelected(pClicked))
                {
                    SelectAll(false);
                    Select(pClicked);
                    SetCurEntry(pClicked);
                }
            }
            else
            {
                pClicked = GetCurEntry();
                if (!pClicked)
                    break;
                aWhere = GetEntryPosition( pClicked );
            }

            ::std::vector<FmFilterData*> aSelectList;
            for (SvTreeListEntry* pEntry = FirstSelected();
                 pEntry != nullptr;
                 pEntry = NextSelected(pEntry))
            {
                // don't delete forms
                FmFormItem* pForm = dynamic_cast<FmFormItem*>( static_cast<FmFilterData*>(pEntry->GetUserData()) );
                if (!pForm)
                    aSelectList.push_back(static_cast<FmFilterData*>(pEntry->GetUserData()));
            }
            if (aSelectList.size() == 1)
            {
                // don't delete the only empty row of a form
                FmFilterItems* pFilterItems = dynamic_cast<FmFilterItems*>( aSelectList[0] );
                if (pFilterItems && pFilterItems->GetChildren().empty()
                    && pFilterItems->GetParent()->GetChildren().size() == 1)
                    aSelectList.clear();
            }

            ScopedVclPtrInstance<PopupMenu> aContextMenu(SVX_RES(RID_FM_FILTER_MENU));

            // every condition could be deleted except the first one if its the only one
            aContextMenu->EnableItem( SID_FM_DELETE, !aSelectList.empty() );


            bool bEdit = dynamic_cast<FmFilterItem*>( static_cast<FmFilterData*>(pClicked->GetUserData()) ) != nullptr &&
                IsSelected(pClicked) && GetSelectionCount() == 1;

            aContextMenu->EnableItem( SID_FM_FILTER_EDIT, bEdit );
            aContextMenu->EnableItem( SID_FM_FILTER_IS_NULL, bEdit );
            aContextMenu->EnableItem( SID_FM_FILTER_IS_NOT_NULL, bEdit );

            aContextMenu->RemoveDisabledEntries(true, true);
            sal_uInt16 nSlotId = aContextMenu->Execute( this, aWhere );
            switch( nSlotId )
            {
                case SID_FM_FILTER_EDIT:
                {
                    EditEntry( pClicked );
                }   break;
                case SID_FM_FILTER_IS_NULL:
                case SID_FM_FILTER_IS_NOT_NULL:
                {
                    OUString aErrorMsg;
                    OUString aText;
                    if (nSlotId == SID_FM_FILTER_IS_NULL)
                        aText = "IS NULL";
                    else
                        aText = "IS NOT NULL";

                    m_pModel->ValidateText(static_cast<FmFilterItem*>(pClicked->GetUserData()),
                                            aText, aErrorMsg);
                    m_pModel->SetTextForItem(static_cast<FmFilterItem*>(pClicked->GetUserData()), aText);
                }   break;
                case SID_FM_DELETE:
                {
                    DeleteSelection();
                }   break;
            }
            bHandled = true;
        }
        break;
        default: break;
    }

    if (!bHandled)
        SvTreeListBox::Command( rEvt );
}

SvTreeListEntry* FmFilterNavigator::getNextEntry(SvTreeListEntry* _pStartWith)
{
    SvTreeListEntry* pEntry = _pStartWith ? _pStartWith : LastSelected();
    pEntry = Next(pEntry);
    // we need the next filter entry
    while( pEntry && GetChildCount( pEntry ) == 0 && pEntry != Last() )
        pEntry = Next(pEntry);
    return pEntry;
}

SvTreeListEntry* FmFilterNavigator::getPrevEntry(SvTreeListEntry* _pStartWith)
{
    SvTreeListEntry* pEntry = _pStartWith ? _pStartWith : FirstSelected();
    pEntry = Prev(pEntry);
    // check if the previous entry is a filter, if so get the next prev
    if ( pEntry && GetChildCount( pEntry ) != 0 )
    {
        pEntry = Prev(pEntry);
        // if the entry is still no leaf return
        if ( pEntry && GetChildCount( pEntry ) != 0 )
            pEntry = nullptr;
    }
    return pEntry;
}

void FmFilterNavigator::KeyInput(const KeyEvent& rKEvt)
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

        ::std::mem_fun1_t<SvTreeListEntry*,FmFilterNavigator,SvTreeListEntry*> getter = ::std::mem_fun(&FmFilterNavigator::getNextEntry);
        if ( rKeyCode.GetCode() == KEY_UP )
            getter = ::std::mem_fun(&FmFilterNavigator::getPrevEntry);

        SvTreeListEntry* pTarget = getter( this, nullptr );
        if ( !pTarget )
            break;

        FmFilterItems* pTargetItems = getTargetItems( pTarget );
        if ( !pTargetItems )
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
                    pTarget = getter(this,pTarget);
                    if ( !pTarget )
                        return;
                    pTargetItems = getTargetItems( pTarget );
                    break;
                }
                else
                {
                    FmFilterItem* pFilterItem = pTargetItems->Find( (*i)->GetComponentIndex() );
                    // we found the text component so jump above
                    if ( pFilterItem )
                    {
                        pTarget = getter( this, pTarget );
                        if ( !pTarget )
                            return;

                        pTargetItems = getTargetItems( pTarget );
                        break;
                    }
                }
            }
            bNextTargetItem = i != aEnd && pTargetItems;
        }

        if ( pTargetItems )
        {
            insertFilterItem( aItemList, pTargetItems, false );
            return;
        }
    }
    break;

    case KEY_DELETE:
    {
        if ( rKeyCode.GetModifier() )
            break;

        if ( !IsSelected( First() ) || GetEntryCount() > 1 )
            DeleteSelection();
        return;
    }
    }

    SvTreeListBox::KeyInput(rKEvt);
}


void FmFilterNavigator::DeleteSelection()
{
    // to avoid the deletion of an entry twice (e.g. deletion of a parent and afterward
    // the deletion of its child, I have to shrink the selecton list
    ::std::vector<SvTreeListEntry*> aEntryList;
    for (SvTreeListEntry* pEntry = FirstSelected();
         pEntry != nullptr;
         pEntry = NextSelected(pEntry))
    {
        FmFilterItem* pFilterItem = dynamic_cast<FmFilterItem*>( static_cast<FmFilterData*>(pEntry->GetUserData()) );
        if (pFilterItem && IsSelected(GetParent(pEntry)))
            continue;

        FmFormItem* pForm = dynamic_cast<FmFormItem*>( static_cast<FmFilterData*>(pEntry->GetUserData()) );
        if (!pForm)
            aEntryList.push_back(pEntry);
    }

    // Remove the selection
    SelectAll(false);

    for (::std::vector<SvTreeListEntry*>::reverse_iterator i = aEntryList.rbegin();
        // link problems with operator ==
        i.base() != aEntryList.rend().base(); ++i)
    {
        m_pModel->Remove(static_cast<FmFilterData*>((*i)->GetUserData()));
    }
}

FmFilterNavigatorWin::FmFilterNavigatorWin( SfxBindings* _pBindings, SfxChildWindow* _pMgr,
                              vcl::Window* _pParent )
                     :SfxDockingWindow( _pBindings, _pMgr, _pParent, WinBits(WB_STDMODELESS|WB_SIZEABLE|WB_ROLLABLE|WB_3DLOOK|WB_DOCKABLE) )
                     ,SfxControllerItem( SID_FM_FILTER_NAVIGATOR_CONTROL, *_pBindings )
{
    SetHelpId( HID_FILTER_NAVIGATOR_WIN );

    m_pNavigator = VclPtr<FmFilterNavigator>::Create( this );
    m_pNavigator->Show();
    SetText( SVX_RES(RID_STR_FILTER_NAVIGATOR) );
    SfxDockingWindow::SetFloatingSize( Size(200,200) );
}


FmFilterNavigatorWin::~FmFilterNavigatorWin()
{
    disposeOnce();
}

void FmFilterNavigatorWin::dispose()
{
    m_pNavigator.disposeAndClear();
    ::SfxControllerItem::dispose();
    SfxDockingWindow::dispose();
}


void FmFilterNavigatorWin::UpdateContent(FmFormShell* pFormShell)
{
    if (!m_pNavigator)
        return;

    if (!pFormShell)
        m_pNavigator->UpdateContent( nullptr, nullptr );
    else
    {
        Reference< XFormController >  xController(pFormShell->GetImpl()->getActiveInternalController());
        Reference< XIndexAccess >   xContainer;
        if (xController.is())
        {
            Reference< XChild >  xChild(xController, UNO_QUERY);
            for (Reference< XInterface >  xParent(xChild->getParent());
                 xParent.is();
                 xParent = xChild.is() ? xChild->getParent() : Reference< XInterface > ())
            {
                xContainer.set(xParent, UNO_QUERY);
                xChild.set(xParent, UNO_QUERY);
            }
        }
        m_pNavigator->UpdateContent(xContainer, xController);
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
    if ( m_pNavigator && m_pNavigator->IsEditingActive() )
        m_pNavigator->EndEditing();

    if ( m_pNavigator && m_pNavigator->IsEditingActive() )
        // the EndEditing was vetoed (perhaps of an syntax error or such)
        return false;

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


void FmFilterNavigatorWin::Resize()
{
    SfxDockingWindow::Resize();

    Size aLogOutputSize = PixelToLogic( GetOutputSizePixel(), MAP_APPFONT );
    Size aLogExplSize = aLogOutputSize;
    aLogExplSize.Width() -= 6;
    aLogExplSize.Height() -= 6;

    Point aExplPos = LogicToPixel( Point(3,3), MAP_APPFONT );
    Size aExplSize = LogicToPixel( aLogExplSize, MAP_APPFONT );

    m_pNavigator->SetPosSizePixel( aExplPos, aExplSize );
}

void FmFilterNavigatorWin::GetFocus()
{
    // oj #97405#
    if ( m_pNavigator )
        m_pNavigator->GrabFocus();
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
