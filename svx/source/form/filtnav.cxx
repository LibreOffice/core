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

#include "filtnav.hxx"
#include "fmexch.hxx"
#include "fmhelp.hrc"
#include "fmitems.hxx"
#include "fmprop.hrc"
#include "svx/fmresids.hrc"

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>
#include <comphelper/uno3.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/implbase1.hxx>
#include <fmservs.hxx>
#include <fmshimp.hxx>
#include <rtl/logfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <svx/dialmgr.hxx>
#include <svx/fmshell.hxx>
#include <svx/fmtools.hxx>
#include <svx/svxids.hrc>
#include <tools/shl.hxx>
#include <vcl/wrkwin.hxx>
#include <tools/diagnose_ex.h>
#include <svtools/svlbitm.hxx>

#include <functional>

#define DROP_ACTION_TIMER_INITIAL_TICKS     10
    // solange dauert es, bis das Scrollen anspringt
#define DROP_ACTION_TIMER_SCROLL_TICKS      3
    // in diesen Intervallen wird jeweils eine Zeile gescrollt
#define DROP_ACTION_TIMER_TICK_BASE         10
    // das ist die Basis, mit der beide Angaben multipliziert werden (in ms)

using namespace ::svxform;
using namespace ::connectivity::simple;
using namespace ::connectivity;


//........................................................................
namespace svxform
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::awt::TextEvent;
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
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::util::XNumberFormatter;
    using ::com::sun::star::sdbc::XRowSet;
    using ::com::sun::star::lang::Locale;
    using ::com::sun::star::sdb::SQLContext;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::awt::XTextComponent;
    using ::com::sun::star::uno::Sequence;
    /** === end UNO using === **/

//========================================================================
OFilterItemExchange::OFilterItemExchange()
{
}

//------------------------------------------------------------------------
void OFilterItemExchange::AddSupportedFormats()
{
    AddFormat(getFormatId());
}

//------------------------------------------------------------------------
sal_uInt32 OFilterItemExchange::getFormatId()
{
    static sal_uInt32 s_nFormat = (sal_uInt32)-1;
    if ((sal_uInt32)-1 == s_nFormat)
    {
        s_nFormat = SotExchange::RegisterFormatName(rtl::OUString("application/x-openoffice;windows_formatname=\"form.FilterControlExchange\""));
        DBG_ASSERT((sal_uInt32)-1 != s_nFormat, "OFilterExchangeHelper::getFormatId: bad exchange id!");
    }
    return s_nFormat;
}

//------------------------------------------------------------------------
OLocalExchange* OFilterExchangeHelper::createExchange() const
{
    return new OFilterItemExchange;
}

//========================================================================
TYPEINIT0(FmFilterData);
Image FmFilterData::GetImage() const
{
    return Image();
}

//========================================================================
TYPEINIT1(FmParentData, FmFilterData);
//------------------------------------------------------------------------
FmParentData::~FmParentData()
{
    for (::std::vector<FmFilterData*>::const_iterator i = m_aChildren.begin();
         i != m_aChildren.end(); ++i)
        delete (*i);
}

//========================================================================
TYPEINIT1(FmFormItem, FmParentData);
//------------------------------------------------------------------------
Image FmFormItem::GetImage() const
{
    static Image aImage;

    if (!aImage)
    {
        ImageList aNavigatorImages( SVX_RES( RID_SVXIMGLIST_FMEXPL ) );
        aImage = aNavigatorImages.GetImage( RID_SVXIMG_FORM );
    }
    return aImage;
}

//========================================================================
TYPEINIT1(FmFilterItems, FmParentData);
//------------------------------------------------------------------------
FmFilterItem* FmFilterItems::Find( const ::sal_Int32 _nFilterComponentIndex ) const
{
    for (   ::std::vector< FmFilterData* >::const_iterator i = m_aChildren.begin();
            i != m_aChildren.end();
            ++i
        )
    {
        FmFilterItem* pCondition = PTR_CAST( FmFilterItem, *i );
        DBG_ASSERT( pCondition, "FmFilterItems::Find: Wrong element in container!" );
        if ( _nFilterComponentIndex == pCondition->GetComponentIndex() )
            return pCondition;
    }
    return NULL;
}

//------------------------------------------------------------------------
Image FmFilterItems::GetImage() const
{
    static Image aImage;

    if (!aImage)
    {
        ImageList aNavigatorImages( SVX_RES( RID_SVXIMGLIST_FMEXPL ) );
        aImage = aNavigatorImages.GetImage( RID_SVXIMG_FILTER );
    }
    return aImage;
}

//========================================================================
TYPEINIT1(FmFilterItem, FmFilterData);
//------------------------------------------------------------------------
FmFilterItem::FmFilterItem( const Reference< XMultiServiceFactory >& _rxFactory,
                            FmFilterItems* pParent,
                            const ::rtl::OUString& aFieldName,
                            const ::rtl::OUString& aText,
                            const sal_Int32 _nComponentIndex )
          :FmFilterData(_rxFactory,pParent, aText)
          ,m_aFieldName(aFieldName)
          ,m_nComponentIndex( _nComponentIndex )
{
}

//------------------------------------------------------------------------
Image FmFilterItem::GetImage() const
{
    static Image aImage;

    if (!aImage)
    {
        ImageList aNavigatorImages( SVX_RES( RID_SVXIMGLIST_FMEXPL ) );
        aImage = aNavigatorImages.GetImage( RID_SVXIMG_FIELD );
    }
    return aImage;
}

//========================================================================
// Hints for communicatition between model and view
//========================================================================
class FmFilterHint : public SfxHint
{
    FmFilterData*   m_pData;

public:
    TYPEINFO();
    FmFilterHint(FmFilterData* pData):m_pData(pData){}
    FmFilterData* GetData() const { return m_pData; }
};
TYPEINIT1( FmFilterHint, SfxHint );

//========================================================================
class FmFilterInsertedHint : public FmFilterHint
{
    sal_uLong m_nPos;   // Position relative to the parent of the data

public:
    TYPEINFO();
    FmFilterInsertedHint(FmFilterData* pData, sal_uLong nRelPos)
        :FmFilterHint(pData)
        ,m_nPos(nRelPos){}

    sal_uLong GetPos() const { return m_nPos; }
};
TYPEINIT1( FmFilterInsertedHint, FmFilterHint );

//========================================================================
class FmFilterRemovedHint : public FmFilterHint
{
public:
    TYPEINFO();
    FmFilterRemovedHint(FmFilterData* pData)
        :FmFilterHint(pData){}

};
TYPEINIT1( FmFilterRemovedHint, FmFilterHint );

//========================================================================
class FmFilterTextChangedHint : public FmFilterHint
{
public:
    TYPEINFO();
    FmFilterTextChangedHint(FmFilterData* pData)
        :FmFilterHint(pData){}

};
TYPEINIT1( FmFilterTextChangedHint, FmFilterHint );

//========================================================================
class FilterClearingHint : public SfxHint
{
public:
    TYPEINFO();
    FilterClearingHint(){}
};
TYPEINIT1( FilterClearingHint, SfxHint );

//========================================================================
class FmFilterCurrentChangedHint : public SfxHint
{
public:
    TYPEINFO();
    FmFilterCurrentChangedHint(){}
};
TYPEINIT1( FmFilterCurrentChangedHint, SfxHint );

//========================================================================
// class FmFilterAdapter, Listener an den FilterControls
//========================================================================
class FmFilterAdapter : public ::cppu::WeakImplHelper1< XFilterControllerListener >
{
    FmFilterModel*              m_pModel;
    Reference< XIndexAccess >   m_xControllers;

public:
    FmFilterAdapter(FmFilterModel* pModel, const Reference< XIndexAccess >& xControllers);

// XEventListener
    virtual void SAL_CALL disposing(const EventObject& Source) throw( RuntimeException );

// XFilterControllerListener
    virtual void SAL_CALL predicateExpressionChanged( const FilterEvent& _Event ) throw (RuntimeException);
    virtual void SAL_CALL disjunctiveTermRemoved( const FilterEvent& _Event ) throw (RuntimeException);
    virtual void SAL_CALL disjunctiveTermAdded( const FilterEvent& _Event ) throw (RuntimeException);

// helpers
    void dispose() throw( RuntimeException );

    void AddOrRemoveListener( const Reference< XIndexAccess >& _rxControllers, const bool _bAdd );

    void setText(sal_Int32 nPos,
        const FmFilterItem* pFilterItem,
        const ::rtl::OUString& rText);
};

//------------------------------------------------------------------------
FmFilterAdapter::FmFilterAdapter(FmFilterModel* pModel, const Reference< XIndexAccess >& xControllers)
    :m_pModel( pModel )
    ,m_xControllers( xControllers )
{
    AddOrRemoveListener( m_xControllers, true );
}

//------------------------------------------------------------------------
void FmFilterAdapter::dispose() throw( RuntimeException )
{
    AddOrRemoveListener( m_xControllers, false );
}

//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
void FmFilterAdapter::setText(sal_Int32 nRowPos,
                              const FmFilterItem* pFilterItem,
                              const ::rtl::OUString& rText)
{
    FmFormItem* pFormItem = PTR_CAST( FmFormItem, pFilterItem->GetParent()->GetParent() );

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
//------------------------------------------------------------------------
void SAL_CALL FmFilterAdapter::disposing(const EventObject& /*e*/) throw( RuntimeException )
{
}

//------------------------------------------------------------------------
namespace
{
    ::rtl::OUString lcl_getLabelName_nothrow( const Reference< XControl >& _rxControl )
    {
        ::rtl::OUString sLabelName;
        try
        {
            Reference< XControl > xControl( _rxControl, UNO_SET_THROW );
            Reference< XPropertySet > xModel( xControl->getModel(), UNO_QUERY_THROW );
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
            Reference< XControl > xControl( _rxControl, UNO_SET_THROW );
            Reference< XPropertySet > xModelProps( xControl->getModel(), UNO_QUERY_THROW );
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
//------------------------------------------------------------------------
void FmFilterAdapter::predicateExpressionChanged( const FilterEvent& _Event ) throw( RuntimeException )
{
    SolarMutexGuard aGuard;

    if ( !m_pModel )
        return;

    // the controller which sent the event
    Reference< XFormController > xController( _Event.Source, UNO_QUERY_THROW );
    Reference< XFilterController > xFilterController( _Event.Source, UNO_QUERY_THROW );
    Reference< XForm > xForm( xController->getModel(), UNO_QUERY_THROW );

    FmFormItem* pFormItem = m_pModel->Find( m_pModel->m_aChildren, xForm );
    OSL_ENSURE( pFormItem, "FmFilterAdapter::predicateExpressionChanged: don't know this form!" );
    if ( !pFormItem )
        return;

    const sal_Int32 nActiveTerm( xFilterController->getActiveTerm() );

    FmFilterItems* pFilter = PTR_CAST( FmFilterItems, pFormItem->GetChildren()[ nActiveTerm ] );
    FmFilterItem* pFilterItem = pFilter->Find( _Event.FilterComponent );
    if ( pFilterItem )
    {
        if ( !_Event.PredicateExpression.isEmpty())
        {
            pFilterItem->SetText( _Event.PredicateExpression );
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
        ::rtl::OUString aFieldName( lcl_getLabelName_nothrow( xFilterController->getFilterComponent( _Event.FilterComponent ) ) );

        pFilterItem = new FmFilterItem( m_pModel->getORB(), pFilter, aFieldName, _Event.PredicateExpression, _Event.FilterComponent );
        m_pModel->Insert(pFilter->GetChildren().end(), pFilterItem);
    }

    // ensure there's one empty term in the filter, just in case the active term was previously empty
    m_pModel->EnsureEmptyFilterRows( *pFormItem );
}

//------------------------------------------------------------------------
void SAL_CALL FmFilterAdapter::disjunctiveTermRemoved( const FilterEvent& _Event ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    Reference< XFormController > xController( _Event.Source, UNO_QUERY_THROW );
    Reference< XFilterController > xFilterController( _Event.Source, UNO_QUERY_THROW );
    Reference< XForm > xForm( xController->getModel(), UNO_QUERY_THROW );

    FmFormItem* pFormItem = m_pModel->Find( m_pModel->m_aChildren, xForm );
    OSL_ENSURE( pFormItem, "FmFilterAdapter::disjunctiveTermRemoved: don't know this form!" );
    if ( !pFormItem )
        return;

    ::std::vector< FmFilterData* >& rTermItems = pFormItem->GetChildren();
    const bool bValidIndex = ( _Event.DisjunctiveTerm >= 0 ) && ( (size_t)_Event.DisjunctiveTerm < rTermItems.size() );
    OSL_ENSURE( bValidIndex, "FmFilterAdapter::disjunctiveTermRemoved: invalid term index!" );
    if ( !bValidIndex )
        return;

    // if the first term was removed, then the to-be first term needs its text updated
    if ( _Event.DisjunctiveTerm == 0 )
    {
        rTermItems[1]->SetText( String( SVX_RES( RID_STR_FILTER_FILTER_FOR ) ) );
        FmFilterTextChangedHint aChangeHint( rTermItems[1] );
        m_pModel->Broadcast( aChangeHint );
    }

    // finally remove the entry from the model
    m_pModel->Remove( rTermItems.begin() + _Event.DisjunctiveTerm );

    // ensure there's one empty term in the filter, just in case the currently removed one was the last empty one
    m_pModel->EnsureEmptyFilterRows( *pFormItem );
}

//------------------------------------------------------------------------
void SAL_CALL FmFilterAdapter::disjunctiveTermAdded( const FilterEvent& _Event ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    Reference< XFormController > xController( _Event.Source, UNO_QUERY_THROW );
    Reference< XFilterController > xFilterController( _Event.Source, UNO_QUERY_THROW );
    Reference< XForm > xForm( xController->getModel(), UNO_QUERY_THROW );

    FmFormItem* pFormItem = m_pModel->Find( m_pModel->m_aChildren, xForm );
    OSL_ENSURE( pFormItem, "FmFilterAdapter::disjunctiveTermAdded: don't know this form!" );
    if ( !pFormItem )
        return;

    const sal_Int32 nInsertPos = _Event.DisjunctiveTerm;
    bool bValidIndex = ( nInsertPos >= 0 ) && ( (size_t)nInsertPos <= pFormItem->GetChildren().size() );
    if ( !bValidIndex )
    {
        OSL_FAIL( "FmFilterAdapter::disjunctiveTermAdded: invalid index!" );
        return;
    }

    const ::std::vector< FmFilterData* >::iterator insertPos = pFormItem->GetChildren().begin() + nInsertPos;

    FmFilterItems* pFilterItems = new FmFilterItems( m_pModel->getORB(), pFormItem, String( SVX_RES( RID_STR_FILTER_FILTER_OR ) ) );
    m_pModel->Insert( insertPos, pFilterItems );
}

//========================================================================
// class FmFilterModel
//========================================================================
TYPEINIT1(FmFilterModel, FmParentData);
//------------------------------------------------------------------------
FmFilterModel::FmFilterModel(const Reference< XMultiServiceFactory >& _rxFactory)
              :FmParentData(_rxFactory,NULL, ::rtl::OUString())
              ,OSQLParserClient(_rxFactory)
              ,m_xORB(_rxFactory)
              ,m_pAdapter(NULL)
              ,m_pCurrentItems(NULL)
{
}

//------------------------------------------------------------------------
FmFilterModel::~FmFilterModel()
{
    Clear();
}

//------------------------------------------------------------------------
void FmFilterModel::Clear()
{
    // notify
    FilterClearingHint aClearedHint;
    Broadcast( aClearedHint );

    // loose endings
    if (m_pAdapter)
    {
        m_pAdapter->dispose();
        m_pAdapter->release();
        m_pAdapter= NULL;
    }

    m_pCurrentItems  = NULL;
    m_xController    = NULL;
    m_xControllers   = NULL;

    for (::std::vector<FmFilterData*>::const_iterator i = m_aChildren.begin();
         i != m_aChildren.end(); ++i)
        delete (*i);

    m_aChildren.clear();
}

//------------------------------------------------------------------------
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
        m_pAdapter->acquire();

        SetCurrentController(xCurrent);
        EnsureEmptyFilterRows( *this );
    }
    else
        SetCurrentController(xCurrent);
}

//------------------------------------------------------------------------
void FmFilterModel::Update(const Reference< XIndexAccess > & xControllers, FmParentData* pParent)
{
    try
    {
        sal_Int32 nCount = xControllers->getCount();
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            Reference< XFormController > xController( xControllers->getByIndex(i), UNO_QUERY_THROW );

            Reference< XPropertySet > xFormProperties( xController->getModel(), UNO_QUERY_THROW );
            ::rtl::OUString aName;
            OSL_VERIFY( xFormProperties->getPropertyValue( FM_PROP_NAME ) >>= aName );

            // Insert a new item for the form
            FmFormItem* pFormItem = new FmFormItem( m_xORB, pParent, xController, aName );
            Insert( pParent->GetChildren().end(), pFormItem );

            Reference< XFilterController > xFilterController( pFormItem->GetFilterController(), UNO_SET_THROW );

            // insert the existing filters for the form
            String aTitle( SVX_RES( RID_STR_FILTER_FILTER_FOR ) );

            Sequence< Sequence< ::rtl::OUString > > aExpressions = xFilterController->getPredicateExpressions();
            for (   const Sequence< ::rtl::OUString >* pConjunctionTerm = aExpressions.getConstArray();
                    pConjunctionTerm != aExpressions.getConstArray() + aExpressions.getLength();
                    ++pConjunctionTerm
                )
            {
                // we always display one row, even if there's no term to be displayed
                FmFilterItems* pFilterItems = new FmFilterItems( m_xORB, pFormItem, aTitle );
                Insert( pFormItem->GetChildren().end(), pFilterItems );

                const Sequence< ::rtl::OUString >& rDisjunction( *pConjunctionTerm );
                for (   const ::rtl::OUString* pDisjunctiveTerm = rDisjunction.getConstArray();
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
                    const ::rtl::OUString sDisplayName( lcl_getLabelName_nothrow( xFilterControl ) );

                    // insert a new entry
                    FmFilterItem* pANDCondition = new FmFilterItem( m_xORB, pFilterItems, sDisplayName, *pDisjunctiveTerm, nComponentIndex );
                    Insert( pFilterItems->GetChildren().end(), pANDCondition );
                }

                // title for the next conditions
                aTitle = SVX_RESSTR( RID_STR_FILTER_FILTER_OR );
            }

            // now add dependent controllers
            Reference< XIndexAccess > xControllerAsIndex( xController, UNO_QUERY );
            Update( xControllerAsIndex, pFormItem );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//------------------------------------------------------------------------
FmFormItem* FmFilterModel::Find(const ::std::vector<FmFilterData*>& rItems, const Reference< XFormController > & xController) const
{
    for (::std::vector<FmFilterData*>::const_iterator i = rItems.begin();
         i != rItems.end(); ++i)
    {
        FmFormItem* pForm = PTR_CAST(FmFormItem,*i);
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
    return NULL;
}

//------------------------------------------------------------------------
FmFormItem* FmFilterModel::Find(const ::std::vector<FmFilterData*>& rItems, const Reference< XForm >& xForm) const
{
    for (::std::vector<FmFilterData*>::const_iterator i = rItems.begin();
         i != rItems.end(); ++i)
    {
        FmFormItem* pForm = PTR_CAST(FmFormItem,*i);
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
    return NULL;
}

//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
void FmFilterModel::AppendFilterItems( FmFormItem& _rFormItem )
{
    // insert the condition behind the last filter items
    ::std::vector<FmFilterData*>::reverse_iterator iter;
    for (   iter = _rFormItem.GetChildren().rbegin();
            iter != _rFormItem.GetChildren().rend();
            ++iter
        )
    {
        if ((*iter)->ISA(FmFilterItems))
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

//------------------------------------------------------------------------
void FmFilterModel::Insert(const ::std::vector<FmFilterData*>::iterator& rPos, FmFilterData* pData)
{
    ::std::vector<FmFilterData*>& rItems = pData->GetParent()->GetChildren();
    sal_uLong nPos = rPos == rItems.end() ? LIST_APPEND : rPos - rItems.begin();
    rItems.insert(rPos, pData);

    // UI benachrichtigen
    FmFilterInsertedHint aInsertedHint(pData, nPos);
    Broadcast( aInsertedHint );
}

//------------------------------------------------------------------------
void FmFilterModel::Remove(FmFilterData* pData)
{
    FmParentData* pParent = pData->GetParent();
    ::std::vector<FmFilterData*>& rItems = pParent->GetChildren();

    // erase the item from the model
    ::std::vector<FmFilterData*>::iterator i = ::std::find(rItems.begin(), rItems.end(), pData);
    DBG_ASSERT(i != rItems.end(), "FmFilterModel::Remove(): unknown Item");
    // position within the parent
    sal_Int32 nPos = i - rItems.begin();
    if (pData->ISA(FmFilterItems))
    {
        FmFormItem* pFormItem = (FmFormItem*)pParent;

        try
        {
            Reference< XFilterController > xFilterController( pFormItem->GetFilterController(), UNO_SET_THROW );

            bool bEmptyLastTerm = ( ( nPos == 0 ) && xFilterController->getDisjunctiveTerms() == 1 );
            if ( bEmptyLastTerm )
            {
                // remove all children (by setting an empty predicate expression)
                ::std::vector< FmFilterData* >& rChildren = ((FmFilterItems*)pData)->GetChildren();
                while ( !rChildren.empty() )
                {
                    ::std::vector< FmFilterData* >::iterator removePos = rChildren.end() - 1;
                    FmFilterItem* pFilterItem = PTR_CAST( FmFilterItem, *removePos );
                    m_pAdapter->setText( nPos, pFilterItem, ::rtl::OUString() );
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
        FmFilterItem* pFilterItem = PTR_CAST(FmFilterItem, pData);

        // if its the last condition remove the parent
        if (rItems.size() == 1)
            Remove(pFilterItem->GetParent());
        else
        {
            // find the position of the father within his father
            ::std::vector<FmFilterData*>& rParentParentItems = pData->GetParent()->GetParent()->GetChildren();
            ::std::vector<FmFilterData*>::iterator j = ::std::find(rParentParentItems.begin(), rParentParentItems.end(), pFilterItem->GetParent());
            DBG_ASSERT(j != rParentParentItems.end(), "FmFilterModel::Remove(): unknown Item");
            sal_Int32 nParentPos = j - rParentParentItems.begin();

            // EmptyText removes the filter
            m_pAdapter->setText(nParentPos, pFilterItem, ::rtl::OUString());
            Remove( i );
        }
    }
}

//------------------------------------------------------------------------
void FmFilterModel::Remove( const ::std::vector<FmFilterData*>::iterator& rPos )
{
    // remove from parent's child list
    FmFilterData* pData = *rPos;
    pData->GetParent()->GetChildren().erase( rPos );

    // notify the view, this will remove the actual SvLBoxEntry
    FmFilterRemovedHint aRemoveHint( pData );
    Broadcast( aRemoveHint );

    delete pData;
}

//------------------------------------------------------------------------
sal_Bool FmFilterModel::ValidateText(FmFilterItem* pItem, UniString& rText, UniString& rErrorMsg) const
{
    FmFormItem* pFormItem = PTR_CAST( FmFormItem, pItem->GetParent()->GetParent() );
    try
    {
        Reference< XFormController > xFormController( pFormItem->GetController() );
        // obtain the connection of the form belonging to the controller
        OStaticDataAccessTools aStaticTools;
        Reference< XRowSet > xRowSet( xFormController->getModel(), UNO_QUERY_THROW );
        Reference< XConnection > xConnection( aStaticTools.getRowSetConnection( xRowSet ) );

        // obtain a number formatter for this connection
        // TODO: shouldn't this be cached?
        Reference< XNumberFormatsSupplier > xFormatSupplier = aStaticTools.getNumberFormats( xConnection, sal_True );
        Reference< XNumberFormatter > xFormatter( m_xORB->createInstance( FM_NUMBER_FORMATTER ), UNO_QUERY );
        xFormatter->attachNumberFormatsSupplier( xFormatSupplier );

        // get the field (database column) which the item is responsible for
        Reference< XFilterController > xFilterController( xFormController, UNO_QUERY_THROW );
        Reference< XPropertySet > xField( lcl_getBoundField_nothrow( xFilterController->getFilterComponent( pItem->GetComponentIndex() ) ), UNO_SET_THROW );

        // parse the given text as filter predicate
        ::rtl::OUString aErr, aTxt( rText );
        ::rtl::Reference< ISQLParseNode > xParseNode = predicateTree( aErr, aTxt, xFormatter, xField );
        rErrorMsg = aErr;
        rText = aTxt;
        if ( xParseNode.is() )
        {
            ::rtl::OUString aPreparedText;
            Locale aAppLocale = Application::GetSettings().GetUILocale();
            xParseNode->parseNodeToPredicateStr(
                aPreparedText, xConnection, xFormatter, xField, aAppLocale, '.', getParseContext() );
            rText = aPreparedText;
            return sal_True;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return sal_False;
}

//------------------------------------------------------------------------
void FmFilterModel::Append(FmFilterItems* pItems, FmFilterItem* pFilterItem)
{
    Insert(pItems->GetChildren().end(), pFilterItem);
}

//------------------------------------------------------------------------
void FmFilterModel::SetTextForItem(FmFilterItem* pItem, const ::rtl::OUString& rText)
{
    ::std::vector<FmFilterData*>& rItems = pItem->GetParent()->GetParent()->GetChildren();
    ::std::vector<FmFilterData*>::iterator i = ::std::find(rItems.begin(), rItems.end(), pItem->GetParent());
    sal_Int32 nParentPos = i - rItems.begin();

    m_pAdapter->setText(nParentPos, pItem, rText);

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

//------------------------------------------------------------------------
void FmFilterModel::SetCurrentItems(FmFilterItems* pCurrent)
{
    if (m_pCurrentItems == pCurrent)
        return;

    // search for the condition
    if (pCurrent)
    {
        FmFormItem* pFormItem = (FmFormItem*)pCurrent->GetParent();
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
            m_pCurrentItems = NULL;
    }
    else
        m_pCurrentItems = NULL;


    // UI benachrichtigen
    FmFilterCurrentChangedHint aHint;
    Broadcast( aHint );
}

//------------------------------------------------------------------------
void FmFilterModel::EnsureEmptyFilterRows( FmParentData& _rItem )
{
    // checks whether for each form there's one free level for input
    ::std::vector< FmFilterData* >& rChildren = _rItem.GetChildren();
    sal_Bool bAppendLevel = _rItem.ISA( FmFormItem );

    for (   ::std::vector<FmFilterData*>::iterator i = rChildren.begin();
            i != rChildren.end();
            ++i
        )
    {
        FmFilterItems* pItems = PTR_CAST(FmFilterItems, *i);
        if ( pItems && pItems->GetChildren().empty() )
        {
            bAppendLevel = sal_False;
            break;
        }

        FmFormItem* pFormItem = PTR_CAST(FmFormItem, *i);
        if (pFormItem)
        {
            EnsureEmptyFilterRows( *pFormItem );
            continue;
        }
    }

    if ( bAppendLevel )
    {
        FmFormItem* pFormItem = PTR_CAST( FmFormItem, &_rItem );
        OSL_ENSURE( pFormItem, "FmFilterModel::EnsureEmptyFilterRows: no FmFormItem, but a FmFilterItems child?" );
        if ( pFormItem )
            AppendFilterItems( *pFormItem );
    }
}

//========================================================================
// class FmFilterItemsString
//========================================================================
class FmFilterItemsString : public SvLBoxString
{
public:
    FmFilterItemsString( SvLBoxEntry* pEntry, sal_uInt16 nFlags,    const XubString& rStr )
        :SvLBoxString(pEntry,nFlags,rStr){}

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry);
    virtual void InitViewData( SvTreeListBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* pViewData);
};

const int nxDBmp = 12;
//------------------------------------------------------------------------
void FmFilterItemsString::Paint(const Point& rPos, SvTreeListBox& rDev, sal_uInt16 /*nFlags*/, SvLBoxEntry* pEntry )
{
    FmFilterItems* pRow = (FmFilterItems*)pEntry->GetUserData();
    FmFormItem* pForm = (FmFormItem*)pRow->GetParent();

    // current filter is significant painted
    const bool bIsCurrentFilter = pForm->GetChildren()[ pForm->GetFilterController()->getActiveTerm() ] == pRow;
    if ( bIsCurrentFilter )
    {
        rDev.Push( PUSH_LINECOLOR );

        rDev.SetLineColor( rDev.GetTextColor() );

        Rectangle aRect( rPos, GetSize( &rDev, pEntry ) );
        Point aFirst( rPos.X(), aRect.Bottom() - 6 );
        Point aSecond(aFirst .X() + 2, aFirst.Y() + 3 );

        rDev.DrawLine( aFirst, aSecond );

        aFirst = aSecond;
        aFirst.X() += 1;
        aSecond.X() += 6;
        aSecond.Y() -= 5;

        rDev.DrawLine( aFirst, aSecond );

        rDev.Pop();
    }

    rDev.DrawText( Point(rPos.X() + nxDBmp, rPos.Y()), GetText() );
}

//------------------------------------------------------------------------
void FmFilterItemsString::InitViewData( SvTreeListBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );

    Size aSize(pView->GetTextWidth(GetText()), pView->GetTextHeight());
    aSize.Width() += nxDBmp;
    pViewData->aSize = aSize;
}

//========================================================================
// class FmFilterString
//========================================================================
class FmFilterString : public SvLBoxString
{
    UniString m_aName;

public:
    FmFilterString( SvLBoxEntry* pEntry, sal_uInt16 nFlags, const XubString& rStr, const UniString& aName)
        :SvLBoxString(pEntry,nFlags,rStr)
        ,m_aName(aName)
    {
        m_aName.AppendAscii(": ");
    }

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry);
    virtual void InitViewData( SvTreeListBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* pViewData);
};

const int nxD = 4;

//------------------------------------------------------------------------
void FmFilterString::InitViewData( SvTreeListBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );

    Font aOldFont( pView->GetFont());
    Font aFont( aOldFont );
    aFont.SetWeight(WEIGHT_BOLD);
    pView->Control::SetFont( aFont );

    Size aSize(pView->GetTextWidth(m_aName), pView->GetTextHeight());
    pView->Control::SetFont( aOldFont );
    aSize.Width() += pView->GetTextWidth(GetText()) + nxD;
    pViewData->aSize = aSize;
}

//------------------------------------------------------------------------
void FmFilterString::Paint(const Point& rPos, SvTreeListBox& rDev, sal_uInt16 /*nFlags*/, SvLBoxEntry* /*pEntry*/ )
{
    Font aOldFont( rDev.GetFont());
    Font aFont( aOldFont );
    aFont.SetWeight(WEIGHT_BOLD);
    rDev.SetFont( aFont );

    Point aPos(rPos);
    rDev.DrawText( aPos, m_aName );

    // position for the second text
    aPos.X() += rDev.GetTextWidth(m_aName) + nxD;
    rDev.SetFont( aOldFont );
    rDev.DrawText( aPos, GetText() );
}

//========================================================================
// class FmFilterNavigator
//========================================================================
FmFilterNavigator::FmFilterNavigator( Window* pParent )
                  :SvTreeListBox( pParent, WB_HASBUTTONS|WB_HASLINES|WB_BORDER|WB_HASBUTTONSATROOT )
                  ,m_pModel( NULL )
                  ,m_pEditingCurrently( NULL )
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

    m_pModel = new FmFilterModel(comphelper::getProcessServiceFactory());
    StartListening( *m_pModel );

    EnableInplaceEditing( sal_True );
    SetSelectionMode(MULTIPLE_SELECTION);

    SetDragDropMode(0xFFFF);

    m_aDropActionTimer.SetTimeoutHdl(LINK(this, FmFilterNavigator, OnDropActionTimer));
}

//------------------------------------------------------------------------
FmFilterNavigator::~FmFilterNavigator()
{
    EndListening( *m_pModel );
    delete m_pModel;
}

//------------------------------------------------------------------------
void FmFilterNavigator::UpdateContent(const Reference< XIndexAccess > & xControllers, const Reference< XFormController > & xCurrent)
{
    if (xCurrent == m_pModel->GetCurrentController())
        return;

    m_pModel->Update(xControllers, xCurrent);

    // expand the filters for the current controller
    SvLBoxEntry* pEntry = FindEntry(m_pModel->GetCurrentForm());
    if (pEntry && !IsExpanded(pEntry))
    {
        SelectAll(sal_False);

        if (!IsExpanded(pEntry))
            Expand(pEntry);

        pEntry = FindEntry(m_pModel->GetCurrentItems());
        if (pEntry)
        {
            if (!IsExpanded(pEntry))
                Expand(pEntry);
            Select(pEntry, sal_True);
        }
    }
}

//------------------------------------------------------------------------
sal_Bool FmFilterNavigator::EditingEntry( SvLBoxEntry* pEntry, Selection& rSelection )
{
    m_pEditingCurrently = pEntry;
    if (!SvTreeListBox::EditingEntry( pEntry, rSelection ))
        return sal_False;

    return pEntry && ((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItem);
}

//------------------------------------------------------------------------
sal_Bool FmFilterNavigator::EditedEntry( SvLBoxEntry* pEntry, const rtl::OUString& rNewText )
{
    DBG_ASSERT(pEntry == m_pEditingCurrently, "FmFilterNavigator::EditedEntry: suspicious entry!");
    m_pEditingCurrently = NULL;

    if (EditingCanceled())
        return sal_True;

    DBG_ASSERT(((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItem),
                    "FmFilterNavigator::EditedEntry() wrong entry");

    UniString aText(comphelper::string::strip(rNewText, ' '));
    if (aText.Len() == 0)
    {
        // deleting the entry asynchron
        sal_uLong nEvent;
        PostUserEvent(nEvent, LINK(this, FmFilterNavigator, OnRemove), pEntry);
    }
    else
    {
        UniString aErrorMsg;

        if (m_pModel->ValidateText((FmFilterItem*)pEntry->GetUserData(), aText, aErrorMsg))
        {
            GrabFocus();
            // this will set the text at the FmFilterItem, as well as update any filter controls
            // which are connected to this particular entry
            m_pModel->SetTextForItem( static_cast< FmFilterItem* >( pEntry->GetUserData() ), aText );

            SetCursor( pEntry, sal_True );
            SetEntryText( pEntry, aText );
        }
        else
        {
            // display the error and return sal_False
            SQLContext aError;
            aError.Message = String(SVX_RES(RID_STR_SYNTAXERROR));
            aError.Details = aErrorMsg;
            displayException(aError, this);

            return sal_False;
        }
    }
    return sal_True;
}

//------------------------------------------------------------------------
IMPL_LINK( FmFilterNavigator, OnRemove, SvLBoxEntry*, pEntry )
{
    // now remove the entry
    m_pModel->Remove((FmFilterData*) pEntry->GetUserData());
    return 0L;
}

//------------------------------------------------------------------------
IMPL_LINK_NOARG(FmFilterNavigator, OnDropActionTimer)
{
    if (--m_aTimerCounter > 0)
        return 0L;

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
            SvLBoxEntry* pToExpand = GetEntry(m_aTimerTriggered);
            if (pToExpand && (GetChildCount(pToExpand) > 0) &&  !IsExpanded(pToExpand))
                // tja, eigentlich muesste ich noch testen, ob die Node nicht schon expandiert ist, aber ich
                // habe dazu weder in den Basisklassen noch im Model eine Methode gefunden ...
                // aber ich denke, die BK sollte es auch so vertragen
                Expand(pToExpand);

            // nach dem Expand habe ich im Gegensatz zum Scrollen natuerlich nix mehr zu tun
            m_aDropActionTimer.Stop();
        }
        break;
    }
    return 0L;
}


//------------------------------------------------------------------------
sal_Int8 FmFilterNavigator::AcceptDrop( const AcceptDropEvent& rEvt )
{
    Point aDropPos = rEvt.maPosPixel;

    // kuemmern wir uns erst mal um moeglich DropActions (Scrollen und Aufklappen)
    if (rEvt.mbLeaving)
    {
        if (m_aDropActionTimer.IsActive())
            m_aDropActionTimer.Stop();
    }
    else
    {
        sal_Bool bNeedTrigger = sal_False;
        // auf dem ersten Eintrag ?
        if ((aDropPos.Y() >= 0) && (aDropPos.Y() < GetEntryHeight()))
        {
            m_aDropActionType = DA_SCROLLUP;
            bNeedTrigger = sal_True;
        }
        else
        {
            // auf dem letzten (bzw. in dem Bereich, den ein Eintrag einnehmen wuerde, wenn er unten genau buendig
            // abschliessen wuerde) ?
            if ((aDropPos.Y() < GetSizePixel().Height()) && (aDropPos.Y() >= GetSizePixel().Height() - GetEntryHeight()))
            {
                m_aDropActionType = DA_SCROLLDOWN;
                bNeedTrigger = sal_True;
            }
            else
            {   // is it an entry whith children, and not yet expanded?
                SvLBoxEntry* pDropppedOn = GetEntry(aDropPos);
                if (pDropppedOn && (GetChildCount(pDropppedOn) > 0) && !IsExpanded(pDropppedOn))
                {
                    // -> aufklappen
                    m_aDropActionType = DA_EXPANDNODE;
                    bNeedTrigger = sal_True;
                }
            }
        }
        if (bNeedTrigger && (m_aTimerTriggered != aDropPos))
        {
            // neu anfangen zu zaehlen
            m_aTimerCounter = DROP_ACTION_TIMER_INITIAL_TICKS;
            // die Pos merken, da ich auch QueryDrops bekomme, wenn sich die Maus gar nicht bewegt hat
            m_aTimerTriggered = aDropPos;
            // und den Timer los
            if (!m_aDropActionTimer.IsActive()) // gibt es den Timer schon ?
            {
                m_aDropActionTimer.SetTimeout(DROP_ACTION_TIMER_TICK_BASE);
                m_aDropActionTimer.Start();
            }
        }
        else if (!bNeedTrigger)
            m_aDropActionTimer.Stop();
    }


    // Hat das Object das richtige Format?
    if (!m_aControlExchange.isDragSource())
        return DND_ACTION_NONE;

    if (!m_aControlExchange->hasFormat(GetDataFlavorExVector()))
        return DND_ACTION_NONE;

    // do we conain the formitem?
    if (!FindEntry(m_aControlExchange->getFormItem()))
        return DND_ACTION_NONE;

    SvLBoxEntry* pDropTarget = GetEntry(aDropPos);
    if (!pDropTarget)
        return DND_ACTION_NONE;

    FmFilterData* pData = (FmFilterData*)pDropTarget->GetUserData();
    FmFormItem* pForm = NULL;
    if (pData->ISA(FmFilterItem))
    {
        pForm = PTR_CAST(FmFormItem,pData->GetParent()->GetParent());
        if (pForm != m_aControlExchange->getFormItem())
            return DND_ACTION_NONE;
    }
    else if (pData->ISA(FmFilterItems))
    {
        pForm = PTR_CAST(FmFormItem,pData->GetParent());
        if (pForm != m_aControlExchange->getFormItem())
            return DND_ACTION_NONE;
    }
    else
        return DND_ACTION_NONE;

    return rEvt.mnAction;
}
// -----------------------------------------------------------------------------
namespace
{
    FmFilterItems* getTargetItems(SvLBoxEntry* _pTarget)
    {
        FmFilterData*   pData = static_cast<FmFilterData*>(_pTarget->GetUserData());
        FmFilterItems*  pTargetItems = pData->ISA(FmFilterItems)
                                        ?
                                        PTR_CAST(FmFilterItems,pData)
                                        :
                                    PTR_CAST(FmFilterItems,pData->GetParent());
        return pTargetItems;
    }
}
//------------------------------------------------------------------------
sal_Int8 FmFilterNavigator::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    // ware schlecht, wenn nach dem Droppen noch gescrollt wird ...
    if (m_aDropActionTimer.IsActive())
        m_aDropActionTimer.Stop();

    // Format-Ueberpruefung
    if (!m_aControlExchange.isDragSource())
        return DND_ACTION_NONE;

    // das Ziel des Drop sowie einige Daten darueber
    Point aDropPos = rEvt.maPosPixel;
    SvLBoxEntry* pDropTarget = GetEntry( aDropPos );
    if (!pDropTarget)
        return DND_ACTION_NONE;

    // search the container where to add the items
    FmFilterItems*  pTargetItems = getTargetItems(pDropTarget);
    SelectAll(sal_False);
    SvLBoxEntry* pEntry = FindEntry(pTargetItems);
    Select(pEntry, sal_True);
    SetCurEntry(pEntry);

    insertFilterItem(m_aControlExchange->getDraggedEntries(),pTargetItems,DND_ACTION_COPY == rEvt.mnAction);

    return sal_True;
}

//------------------------------------------------------------------------
void FmFilterNavigator::InitEntry(SvLBoxEntry* pEntry,
                                  const XubString& rStr,
                                  const Image& rImg1,
                                  const Image& rImg2,
                                                                  SvLBoxButtonKind eButtonKind)
{
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2, eButtonKind );
    SvLBoxString* pString = NULL;

    if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItem))
        pString = new FmFilterString(pEntry, 0, rStr, ((FmFilterItem*)pEntry->GetUserData())->GetFieldName());
    else if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItems))
        pString = new FmFilterItemsString(pEntry, 0, rStr );

    if (pString)
        pEntry->ReplaceItem( pString, 1 );
}

//------------------------------------------------------------------------
sal_Bool FmFilterNavigator::Select( SvLBoxEntry* pEntry, sal_Bool bSelect )
{
    if (bSelect == IsSelected(pEntry))  // das passiert manchmal, ich glaube, die Basisklasse geht zu sehr auf Nummer sicher ;)
        return sal_True;

    if (SvTreeListBox::Select(pEntry, bSelect))
    {
        if (bSelect)
        {
            FmFormItem* pFormItem = NULL;
            if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItem))
                pFormItem = (FmFormItem*)((FmFilterItem*)pEntry->GetUserData())->GetParent()->GetParent();
            else if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItems))
                pFormItem = (FmFormItem*)((FmFilterItem*)pEntry->GetUserData())->GetParent()->GetParent();
            else if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFormItem))
                pFormItem = (FmFormItem*)pEntry->GetUserData();

            if (pFormItem)
            {
                // will the controller be exchanged?
                if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItem))
                    m_pModel->SetCurrentItems((FmFilterItems*)((FmFilterItem*)pEntry->GetUserData())->GetParent());
                else if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFilterItems))
                    m_pModel->SetCurrentItems((FmFilterItems*)pEntry->GetUserData());
                else if (((FmFilterData*)pEntry->GetUserData())->ISA(FmFormItem))
                    m_pModel->SetCurrentController(((FmFormItem*)pEntry->GetUserData())->GetController());
            }
        }
        return sal_True;
    }
    else
        return sal_False;
}

//------------------------------------------------------------------------
void FmFilterNavigator::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if (rHint.ISA(FmFilterInsertedHint))
    {
        FmFilterInsertedHint* pHint = (FmFilterInsertedHint*)&rHint;
        Insert(pHint->GetData(), pHint->GetPos());
    }
    else if( rHint.ISA(FilterClearingHint) )
    {
        SvTreeListBox::Clear();
    }
    else if( rHint.ISA(FmFilterRemovedHint) )
    {
        FmFilterRemovedHint* pHint = (FmFilterRemovedHint*)&rHint;
        Remove(pHint->GetData());
    }
    else if( rHint.ISA(FmFilterTextChangedHint) )
    {
        FmFilterTextChangedHint* pHint = (FmFilterTextChangedHint*)&rHint;
        SvLBoxEntry* pEntry = FindEntry(pHint->GetData());
        if (pEntry)
            SetEntryText( pEntry, pHint->GetData()->GetText());
    }
    else if( rHint.ISA(FmFilterCurrentChangedHint) )
    {
        // invalidate the entries
        for (SvLBoxEntry* pEntry = First(); pEntry != NULL;
             pEntry = Next(pEntry))
            GetModel()->InvalidateEntry( pEntry );
    }
}

//------------------------------------------------------------------------
SvLBoxEntry* FmFilterNavigator::FindEntry(const FmFilterData* pItem) const
{
    SvLBoxEntry* pEntry = NULL;
    if (pItem)
    {
        for (pEntry = First(); pEntry != NULL; pEntry = Next( pEntry ))
        {
            FmFilterData* pEntryItem = (FmFilterData*)pEntry->GetUserData();
            if (pEntryItem == pItem)
                break;
        }
    }
    return pEntry;
}

//------------------------------------------------------------------------
void FmFilterNavigator::Insert(FmFilterData* pItem, sal_uLong nPos)
{
    const FmParentData* pParent = pItem->GetParent() ? pItem->GetParent() : GetFilterModel();

    // insert the item
    SvLBoxEntry* pParentEntry = FindEntry( pParent );
    InsertEntry( pItem->GetText(), pItem->GetImage(), pItem->GetImage(), pParentEntry, sal_False, nPos, pItem );
    if ( pParentEntry )
        Expand( pParentEntry );
}

//------------------------------------------------------------------------
void FmFilterNavigator::Remove(FmFilterData* pItem)
{
    // der Entry zu den Daten
    SvLBoxEntry* pEntry = FindEntry(pItem);

    if (pEntry == m_pEditingCurrently)
        // cancel editing
        EndEditing(sal_True);

    if (pEntry)
        GetModel()->Remove( pEntry );
}
// -----------------------------------------------------------------------------
FmFormItem* FmFilterNavigator::getSelectedFilterItems(::std::vector<FmFilterItem*>& _rItemList)
{
    // be sure that the data is only used within only one form!
    FmFormItem* pFirstItem = NULL;

    sal_Bool bHandled = sal_True;
    sal_Bool bFoundSomething = sal_False;
    for (SvLBoxEntry* pEntry = FirstSelected();
         bHandled && pEntry != NULL;
         pEntry = NextSelected(pEntry))
    {
        FmFilterItem* pFilter = PTR_CAST(FmFilterItem, (FmFilterData*)pEntry->GetUserData());
        if (pFilter)
        {
            FmFormItem* pForm = PTR_CAST(FmFormItem,pFilter->GetParent()->GetParent());
            if (!pForm)
                bHandled = sal_False;
            else if (!pFirstItem)
                pFirstItem = pForm;
            else if (pFirstItem != pForm)
                bHandled = sal_False;

            if (bHandled)
            {
                _rItemList.push_back(pFilter);
                bFoundSomething = sal_True;
            }
        }
    }
    if ( !bHandled || !bFoundSomething )
        pFirstItem = NULL;
    return pFirstItem;
}
// -----------------------------------------------------------------------------
void FmFilterNavigator::insertFilterItem(const ::std::vector<FmFilterItem*>& _rFilterList,FmFilterItems* _pTargetItems,sal_Bool _bCopy)
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
        String aText = pLookupItem->GetText();
        if ( !pFilterItem )
        {
            pFilterItem = new FmFilterItem( m_pModel->getORB(), _pTargetItems, pLookupItem->GetFieldName(), aText, pLookupItem->GetComponentIndex() );
            m_pModel->Append( _pTargetItems, pFilterItem );
        }

        if ( !_bCopy )
            m_pModel->Remove( pLookupItem );

        // now set the text for the new dragged item
        m_pModel->SetTextForItem( pFilterItem, aText );
    }

    m_pModel->EnsureEmptyFilterRows( *_pTargetItems->GetParent() );
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void FmFilterNavigator::Command( const CommandEvent& rEvt )
{
    sal_Bool bHandled = sal_False;
    switch (rEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            // die Stelle, an der geklickt wurde
            Point aWhere;
            SvLBoxEntry* pClicked = NULL;
            if (rEvt.IsMouseEvent())
            {
                aWhere = rEvt.GetMousePosPixel();
                pClicked = GetEntry(aWhere);
                if (pClicked == NULL)
                    break;

                if (!IsSelected(pClicked))
                {
                    SelectAll(sal_False);
                    Select(pClicked, sal_True);
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
            for (SvLBoxEntry* pEntry = FirstSelected();
                 pEntry != NULL;
                 pEntry = NextSelected(pEntry))
            {
                // don't delete forms
                FmFormItem* pForm = PTR_CAST(FmFormItem, (FmFilterData*)pEntry->GetUserData());
                if (!pForm)
                    aSelectList.push_back((FmFilterData*)pEntry->GetUserData());
            }
            if (aSelectList.size() == 1)
            {
                // don't delete the only empty row of a form
                FmFilterItems* pFilterItems = PTR_CAST(FmFilterItems, aSelectList[0]);
                if (pFilterItems && pFilterItems->GetChildren().empty()
                    && pFilterItems->GetParent()->GetChildren().size() == 1)
                    aSelectList.clear();
            }

            PopupMenu aContextMenu(SVX_RES(RID_FM_FILTER_MENU));

            // every condition could be deleted except the first one if its the only one
            aContextMenu.EnableItem( SID_FM_DELETE, !aSelectList.empty() );

            //
            sal_Bool bEdit = PTR_CAST(FmFilterItem, (FmFilterData*)pClicked->GetUserData()) != NULL &&
                IsSelected(pClicked) && GetSelectionCount() == 1;

            aContextMenu.EnableItem( SID_FM_FILTER_EDIT,
                bEdit );
            aContextMenu.EnableItem( SID_FM_FILTER_IS_NULL,
                bEdit );
            aContextMenu.EnableItem( SID_FM_FILTER_IS_NOT_NULL,
                bEdit );

            aContextMenu.RemoveDisabledEntries(sal_True, sal_True);
            sal_uInt16 nSlotId = aContextMenu.Execute( this, aWhere );
            switch( nSlotId )
            {
                case SID_FM_FILTER_EDIT:
                {
                    EditEntry( pClicked );
                }   break;
                case SID_FM_FILTER_IS_NULL:
                case SID_FM_FILTER_IS_NOT_NULL:
                {
                    UniString aErrorMsg;
                    UniString aText;
                    if (nSlotId == SID_FM_FILTER_IS_NULL)
                        aText.AssignAscii("IS NULL");
                    else
                        aText.AssignAscii("IS NOT NULL");

                    m_pModel->ValidateText((FmFilterItem*)pClicked->GetUserData(),
                                            aText, aErrorMsg);
                    m_pModel->SetTextForItem((FmFilterItem*)pClicked->GetUserData(), aText);
                }   break;
                case SID_FM_DELETE:
                {
                    DeleteSelection();
                }   break;
            }
            bHandled = sal_True;
        } break;
    }

    if (!bHandled)
        SvTreeListBox::Command( rEvt );
}
// -----------------------------------------------------------------------------
SvLBoxEntry* FmFilterNavigator::getNextEntry(SvLBoxEntry* _pStartWith)
{
    SvLBoxEntry* pEntry = _pStartWith ? _pStartWith : LastSelected();
    pEntry = Next(pEntry);
    // we need the next filter entry
    while( pEntry && GetChildCount( pEntry ) == 0 && pEntry != Last() )
        pEntry = Next(pEntry);
    return pEntry;
}
// -----------------------------------------------------------------------------
SvLBoxEntry* FmFilterNavigator::getPrevEntry(SvLBoxEntry* _pStartWith)
{
    SvLBoxEntry* pEntry = _pStartWith ? _pStartWith : FirstSelected();
    pEntry = Prev(pEntry);
    // check if the previous entry is a filter, if so get the next prev
    if ( pEntry && GetChildCount( pEntry ) != 0 )
    {
        pEntry = Prev(pEntry);
        // if the entry is still no leaf return
        if ( pEntry && GetChildCount( pEntry ) != 0 )
            pEntry = NULL;
    }
    return pEntry;
}
//------------------------------------------------------------------------
void FmFilterNavigator::KeyInput(const KeyEvent& rKEvt)
{
    const KeyCode&  rKeyCode = rKEvt.GetKeyCode();

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

        ::std::mem_fun1_t<SvLBoxEntry*,FmFilterNavigator,SvLBoxEntry*> getter = ::std::mem_fun(&FmFilterNavigator::getNextEntry);
        if ( rKeyCode.GetCode() == KEY_UP )
            getter = ::std::mem_fun(&FmFilterNavigator::getPrevEntry);

        SvLBoxEntry* pTarget = getter( this, NULL );
        if ( !pTarget )
            break;

        FmFilterItems* pTargetItems = getTargetItems( pTarget );
        if ( !pTargetItems )
            break;

        ::std::vector<FmFilterItem*>::const_iterator aEnd = aItemList.end();
        sal_Bool bNextTargetItem = sal_True;
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
            insertFilterItem( aItemList, pTargetItems );
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

//------------------------------------------------------------------------------
void FmFilterNavigator::DeleteSelection()
{
    // to avoid the deletion of an entry twice (e.g. deletion of a parent and afterward
    // the deletion of it's child, i have to shrink the selecton list
    ::std::vector<SvLBoxEntry*> aEntryList;
    for (SvLBoxEntry* pEntry = FirstSelected();
         pEntry != NULL;
         pEntry = NextSelected(pEntry))
    {
        FmFilterItem* pFilterItem = PTR_CAST(FmFilterItem, (FmFilterData*)pEntry->GetUserData());
        if (pFilterItem && IsSelected(GetParent(pEntry)))
            continue;

        FmFormItem* pForm = PTR_CAST(FmFormItem, (FmFilterData*)pEntry->GetUserData());
        if (!pForm)
            aEntryList.push_back(pEntry);
    }

    // Remove the selection
    SelectAll(sal_False);

    for (::std::vector<SvLBoxEntry*>::reverse_iterator i = aEntryList.rbegin();
        // link problems with operator ==
        i.base() != aEntryList.rend().base(); ++i)
    {
        m_pModel->Remove((FmFilterData*)(*i)->GetUserData());
    }
}
// -----------------------------------------------------------------------------

//========================================================================
// class FmFilterNavigatorWin
//========================================================================
FmFilterNavigatorWin::FmFilterNavigatorWin( SfxBindings* _pBindings, SfxChildWindow* _pMgr,
                              Window* _pParent )
                     :SfxDockingWindow( _pBindings, _pMgr, _pParent, WinBits(WB_STDMODELESS|WB_SIZEABLE|WB_ROLLABLE|WB_3DLOOK|WB_DOCKABLE) )
                     ,SfxControllerItem( SID_FM_FILTER_NAVIGATOR_CONTROL, *_pBindings )
{
    SetHelpId( HID_FILTER_NAVIGATOR_WIN );

    m_pNavigator = new FmFilterNavigator( this );
    m_pNavigator->Show();
    SetText( SVX_RES(RID_STR_FILTER_NAVIGATOR) );
    SfxDockingWindow::SetFloatingSize( Size(200,200) );
}

//------------------------------------------------------------------------
FmFilterNavigatorWin::~FmFilterNavigatorWin()
{
    delete m_pNavigator;
}

//-----------------------------------------------------------------------
void FmFilterNavigatorWin::UpdateContent(FmFormShell* pFormShell)
{
    if (!pFormShell)
        m_pNavigator->UpdateContent( NULL, NULL );
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
                xContainer = Reference< XIndexAccess > (xParent, UNO_QUERY);
                xChild = Reference< XChild > (xParent, UNO_QUERY);
            }
        }
        m_pNavigator->UpdateContent(xContainer, xController);
    }
}

//-----------------------------------------------------------------------
void FmFilterNavigatorWin::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if( !pState  || SID_FM_FILTER_NAVIGATOR_CONTROL != nSID )
        return;

    if( eState >= SFX_ITEM_AVAILABLE )
    {
        FmFormShell* pShell = PTR_CAST( FmFormShell,((SfxObjectItem*)pState)->GetShell() );
        UpdateContent( pShell );
    }
    else
        UpdateContent( NULL );
}

//-----------------------------------------------------------------------
sal_Bool FmFilterNavigatorWin::Close()
{
    if ( m_pNavigator && m_pNavigator->IsEditingActive() )
        m_pNavigator->EndEditing();

    if ( m_pNavigator && m_pNavigator->IsEditingActive() )
        // the EndEditing was vetoed (perhaps of an syntax error or such)
        return sal_False;

    UpdateContent( NULL );
    return SfxDockingWindow::Close();
}

//-----------------------------------------------------------------------
void FmFilterNavigatorWin::FillInfo( SfxChildWinInfo& rInfo ) const
{
    SfxDockingWindow::FillInfo( rInfo );
    rInfo.bVisible = sal_False;
}

//-----------------------------------------------------------------------
Size FmFilterNavigatorWin::CalcDockingSize( SfxChildAlignment eAlign )
{
    if ( ( eAlign == SFX_ALIGN_TOP ) || ( eAlign == SFX_ALIGN_BOTTOM ) )
        return Size();

    return SfxDockingWindow::CalcDockingSize( eAlign );
}

//-----------------------------------------------------------------------
SfxChildAlignment FmFilterNavigatorWin::CheckAlignment( SfxChildAlignment eActAlign, SfxChildAlignment eAlign )
{
    switch (eAlign)
    {
        case SFX_ALIGN_LEFT:
        case SFX_ALIGN_RIGHT:
        case SFX_ALIGN_NOALIGNMENT:
            return (eAlign);
        default:
            break;
    }

    return (eActAlign);
}

//------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
void FmFilterNavigatorWin::GetFocus()
{
    // oj #97405#
    if ( m_pNavigator )
        m_pNavigator->GrabFocus();
}
// -----------------------------------------------------------------------------


//========================================================================
// class FmFilterNavigatorWinMgr
//========================================================================
SFX_IMPL_DOCKINGWINDOW( FmFilterNavigatorWinMgr, SID_FM_FILTER_NAVIGATOR )

//-----------------------------------------------------------------------
FmFilterNavigatorWinMgr::FmFilterNavigatorWinMgr( Window *_pParent, sal_uInt16 _nId,
                                    SfxBindings *_pBindings, SfxChildWinInfo* _pInfo )
                 :SfxChildWindow( _pParent, _nId )
{
    pWindow = new FmFilterNavigatorWin( _pBindings, this, _pParent );
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    ((SfxDockingWindow*)pWindow)->Initialize( _pInfo );
}

//........................................................................
}   // namespace svxform
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
