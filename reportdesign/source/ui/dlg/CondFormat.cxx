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

#include <CondFormat.hxx>

#include <strings.hxx>
#include <strings.hrc>
#include <rptui_slotid.hrc>
#include <core_resource.hxx>
#include <UITools.hxx>
#include <ReportController.hxx>
#include "Condition.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <toolkit/helper/vclunohelper.hxx>

#include <vcl/settings.hxx>

#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

#include <comphelper/property.hxx>

#include <algorithm>
#include <UndoActions.hxx>


namespace rptui
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::uno::Any;

    using namespace ::com::sun::star::report;


    // UpdateLocker

    class UpdateLocker
    {
        vcl::Window& m_rWindow;

    public:
        explicit UpdateLocker( vcl::Window& _rWindow )
            :m_rWindow( _rWindow )
        {
            _rWindow.SetUpdateMode( false );
        }
        ~UpdateLocker()
        {
            m_rWindow.SetUpdateMode( true );
        }
    };

    void ConditionalFormattingDialog::impl_setPrefHeight(bool bFirst)
    {
        if (!m_bConstructed && !bFirst)
            return;

        //allow dialog to resize itself
        size_t nCount = impl_getConditionCount();
        if (nCount)
        {
            long nHeight = m_aConditions[0]->get_preferred_size().Height();
            size_t nVisibleConditions = ::std::min(nCount, MAX_CONDITIONS);
            nHeight *= nVisibleConditions;
            if (nHeight != m_pScrollWindow->get_height_request())
            {
                m_pScrollWindow->set_height_request(nHeight);
                if (!isCalculatingInitialLayoutSize() && !bFirst)
                    setOptimalLayoutSize();
            }
        }
    }

    // class ConditionalFormattingDialog
    ConditionalFormattingDialog::ConditionalFormattingDialog(
            vcl::Window* _pParent, const Reference< XReportControlModel >& _rxFormatConditions, ::rptui::OReportController& _rController )
        :ModalDialog( _pParent, "CondFormat", "modules/dbreport/ui/condformatdialog.ui" )
        ,m_rController( _rController )
        ,m_xFormatConditions( _rxFormatConditions )
        ,m_bDeletingCondition( false )
        ,m_bConstructed( false )
    {
        get(m_pConditionPlayground, "condPlaygroundDrawingarea");
        get(m_pScrollWindow, "scrolledwindow");
        m_pScrollWindow->setUserManagedScrolling(true);
        m_pCondScroll = &(m_pScrollWindow->getVertScrollBar());

        OSL_ENSURE( m_xFormatConditions.is(), "ConditionalFormattingDialog::ConditionalFormattingDialog: ReportControlModel is NULL -> Prepare for GPF!" );

        m_xCopy.set( m_xFormatConditions->createClone(), UNO_QUERY_THROW );

        m_pCondScroll->SetScrollHdl( LINK( this, ConditionalFormattingDialog, OnScroll ) );

        impl_initializeConditions();

        impl_setPrefHeight(true);

        m_bConstructed = true;
    }

    ConditionalFormattingDialog::~ConditionalFormattingDialog()
    {
        disposeOnce();
    }

    void ConditionalFormattingDialog::dispose()
    {

        for (auto& rxCondition : m_aConditions)
        {
            rxCondition.disposeAndClear();
        }

        m_aConditions.clear();
        m_pConditionPlayground.clear();
        m_pScrollWindow.clear();
        m_pCondScroll.clear();
        ModalDialog::dispose();
    }

    void ConditionalFormattingDialog::impl_updateConditionIndicies()
    {
        sal_Int32 nIndex = 0;
        for (const auto& rxCondition : m_aConditions)
        {
            rxCondition->setConditionIndex( nIndex, impl_getConditionCount() );
            ++nIndex;
        }
    }

    void ConditionalFormattingDialog::impl_conditionCountChanged()
    {
        if ( m_aConditions.empty() )
            impl_addCondition_nothrow( 0 );

        impl_setPrefHeight(false);

        impl_updateScrollBarRange();
        impl_updateConditionIndicies();
        impl_layoutAll();
    }

    void ConditionalFormattingDialog::addCondition( size_t _nAddAfterIndex )
    {
        OSL_PRECOND( _nAddAfterIndex < impl_getConditionCount(), "ConditionalFormattingDialog::addCondition: illegal condition index!" );
        impl_addCondition_nothrow( _nAddAfterIndex + 1 );
    }


    void ConditionalFormattingDialog::deleteCondition( size_t _nCondIndex )
    {
        impl_deleteCondition_nothrow( _nCondIndex );
    }


    void ConditionalFormattingDialog::impl_addCondition_nothrow( size_t _nNewCondIndex )
    {
        UpdateLocker aLockUpdates( *this );

        try
        {
            if ( _nNewCondIndex > static_cast<size_t>(m_xCopy->getCount()) )
                throw IllegalArgumentException();

            Reference< XFormatCondition > xCond = m_xCopy->createFormatCondition();
            ::comphelper::copyProperties(m_xCopy.get(),xCond.get());
            m_xCopy->insertByIndex( _nNewCondIndex, makeAny( xCond ) );
            VclPtrInstance<Condition> pCon( m_pConditionPlayground, *this, m_rController );
            pCon->setCondition( xCond );
            pCon->reorderWithinParent(_nNewCondIndex);
            m_aConditions.insert( m_aConditions.begin() + _nNewCondIndex, pCon );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
        }

        impl_conditionCountChanged();

        impl_ensureConditionVisible( _nNewCondIndex );
    }


    void ConditionalFormattingDialog::impl_focusCondition( size_t _nCondIndex )
    {
        OSL_PRECOND( _nCondIndex < impl_getConditionCount(),
            "ConditionalFormattingDialog::impl_focusCondition: illegal index!" );

        impl_ensureConditionVisible( _nCondIndex );
        m_aConditions[ _nCondIndex ]->GrabFocus();
    }


    void ConditionalFormattingDialog::impl_deleteCondition_nothrow( size_t _nCondIndex )
    {
        UpdateLocker aLockUpdates( *this );

        OSL_PRECOND( _nCondIndex < impl_getConditionCount(),
            "ConditionalFormattingDialog::impl_deleteCondition_nothrow: illegal index!" );

        bool bLastCondition = ( impl_getConditionCount() == 1 );

        bool bSetNewFocus = false;
        size_t nNewFocusIndex( _nCondIndex );
        try
        {
            if ( !bLastCondition )
                m_xCopy->removeByIndex( _nCondIndex );

            Conditions::iterator pos = m_aConditions.begin() + _nCondIndex;
            if ( bLastCondition )
            {
                Reference< XFormatCondition > xFormatCondition( m_xCopy->getByIndex( 0 ), UNO_QUERY_THROW );
                xFormatCondition->setFormula( OUString() );
                (*pos)->setCondition( xFormatCondition );
            }
            else
            {
                bSetNewFocus = (*pos)->HasChildPathFocus();
                m_bDeletingCondition = true;
                m_aConditions.erase( pos );
                m_bDeletingCondition = false;
            }

            if ( bSetNewFocus )
            {
                if ( nNewFocusIndex >= impl_getConditionCount() )
                    nNewFocusIndex = impl_getConditionCount() - 1;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
        }

        impl_conditionCountChanged();
        if ( bSetNewFocus )
            impl_focusCondition( nNewFocusIndex );
    }


    void ConditionalFormattingDialog::impl_moveCondition_nothrow( size_t _nCondIndex, bool _bMoveUp )
    {
        size_t nOldConditionIndex( _nCondIndex );
        size_t nNewConditionIndex( _bMoveUp ? _nCondIndex - 1 : _nCondIndex + 1 );

        // do this in two steps, so we don't become inconsistent if any of the UNO actions fails
        Any aMovedCondition;
        Condition *pMovedCondition;
        try
        {
            aMovedCondition = m_xCopy->getByIndex( static_cast<sal_Int32>(nOldConditionIndex) );
            m_xCopy->removeByIndex( static_cast<sal_Int32>(nOldConditionIndex) );

            Conditions::iterator aRemovePos( m_aConditions.begin() + nOldConditionIndex );
            pMovedCondition = aRemovePos->get();
            m_aConditions.erase( aRemovePos );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
            return;
        }

        try
        {
            m_xCopy->insertByIndex( static_cast<sal_Int32>(nNewConditionIndex), aMovedCondition );
            m_aConditions.insert( m_aConditions.begin() + nNewConditionIndex, pMovedCondition );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
        }

        // at least the two swapped conditions need to know their new index
        impl_updateConditionIndicies();

        // re-layout all conditions
        impl_layoutConditions();

        // ensure the moved condition is visible
        impl_ensureConditionVisible( nNewConditionIndex );
    }

    IMPL_LINK( ConditionalFormattingDialog, OnScroll, ScrollBar*, /*_pNotInterestedIn*/, void )
    {
        size_t nFirstCondIndex( impl_getFirstVisibleConditionIndex() );
        size_t nFocusCondIndex = impl_getFocusedConditionIndex( nFirstCondIndex );

        impl_layoutConditions();

        if ( nFocusCondIndex < nFirstCondIndex )
            impl_focusCondition( nFirstCondIndex );
        else if ( nFocusCondIndex >= nFirstCondIndex + MAX_CONDITIONS )
            impl_focusCondition( nFirstCondIndex + MAX_CONDITIONS - 1 );
    }

    void ConditionalFormattingDialog::impl_layoutConditions()
    {
        if (m_aConditions.empty())
            return;
        long nConditionHeight = m_aConditions[0]->get_preferred_size().Height();
        Point aConditionPos(0, -1 * nConditionHeight * impl_getFirstVisibleConditionIndex());
        m_pConditionPlayground->SetPosPixel(aConditionPos);
    }

    void ConditionalFormattingDialog::impl_layoutAll()
    {
        // condition's positions
        impl_layoutConditions();

        // scrollbar visibility
        if ( m_aConditions.size() <= MAX_CONDITIONS )
            // normalize the position, so it can, in all situations, be used as top index
            m_pCondScroll->SetThumbPos( 0 );
    }

    void ConditionalFormattingDialog::impl_initializeConditions()
    {
        try
        {
            sal_Int32 nCount = m_xCopy->getCount();
            for ( sal_Int32 i = 0; i < nCount ; ++i )
            {
                VclPtrInstance<Condition> pCon( m_pConditionPlayground, *this, m_rController );
                Reference< XFormatCondition > xCond( m_xCopy->getByIndex(i), UNO_QUERY );
                pCon->reorderWithinParent(i);
                pCon->setCondition( xCond );
                pCon->updateToolbar( xCond.get() );
                m_aConditions.push_back( pCon );
            }
        }
        catch(Exception&)
        {
            OSL_FAIL("Can not access format condition!");
        }

        impl_conditionCountChanged();
    }

    void ConditionalFormattingDialog::applyCommand(size_t _nCondIndex, sal_uInt16 _nCommandId, const ::Color& rColor)
    {
        OSL_PRECOND( _nCommandId, "ConditionalFormattingDialog::applyCommand: illegal command id!" );
        try
        {
            Reference< XReportControlFormat > xReportControlFormat( m_xCopy->getByIndex( _nCondIndex ), UNO_QUERY_THROW );

            Sequence< PropertyValue > aArgs(3);

            aArgs[0].Name = REPORTCONTROLFORMAT;
            aArgs[0].Value <<= xReportControlFormat;

            aArgs[1].Name = CURRENT_WINDOW;
            aArgs[1].Value <<= VCLUnoHelper::GetInterface(this);

            aArgs[2].Name = PROPERTY_FONTCOLOR;
            aArgs[2].Value <<= rColor;

            // we use this way to create undo actions
            m_rController.executeUnChecked(_nCommandId,aArgs);
            m_aConditions[ _nCondIndex ]->updateToolbar(xReportControlFormat);
        }
        catch( Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
        }
    }


    void ConditionalFormattingDialog::moveConditionUp( size_t _nCondIndex )
    {
        OSL_PRECOND( _nCondIndex > 0, "ConditionalFormattingDialog::moveConditionUp: cannot move up the first condition!" );
        if ( _nCondIndex > 0 )
            impl_moveCondition_nothrow( _nCondIndex, true );
    }


    void ConditionalFormattingDialog::moveConditionDown( size_t _nCondIndex )
    {
        OSL_PRECOND( _nCondIndex < impl_getConditionCount(), "ConditionalFormattingDialog::moveConditionDown: cannot move down the last condition!" );
        if ( _nCondIndex < impl_getConditionCount() )
            impl_moveCondition_nothrow( _nCondIndex, false );
    }


    OUString ConditionalFormattingDialog::getDataField() const
    {
        OUString sDataField;
        try
        {
            sDataField = m_xFormatConditions->getDataField();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
        }
        return sDataField;
    }


    short ConditionalFormattingDialog::Execute()
    {
        short nRet = ModalDialog::Execute();
        if ( nRet == RET_OK )
        {
            const OUString sUndoAction( RptResId( RID_STR_UNDO_CONDITIONAL_FORMATTING ) );
            const UndoContext aUndoContext( m_rController.getUndoManager(), sUndoAction );
            try
            {
                sal_Int32 j(0), i(0);
                for (   Conditions::const_iterator cond = m_aConditions.begin();
                        cond != m_aConditions.end();
                        ++cond, ++i
                    )
                {
                    Reference< XFormatCondition > xCond( m_xCopy->getByIndex(i), UNO_QUERY_THROW );
                    (*cond)->fillFormatCondition( xCond );

                    if ( (*cond)->isEmpty() )
                        continue;

                    Reference< XFormatCondition > xNewCond;
                    bool bAppend = j >= m_xFormatConditions->getCount();
                    if ( bAppend )
                    {
                        xNewCond = m_xFormatConditions->createFormatCondition();
                        m_xFormatConditions->insertByIndex( i, makeAny( xNewCond ) );
                    }
                    else
                        xNewCond.set( m_xFormatConditions->getByIndex(j), UNO_QUERY );
                    ++j;

                    ::comphelper::copyProperties(xCond.get(),xNewCond.get());
                }

                for ( sal_Int32 k = m_xFormatConditions->getCount()-1; k >= j; --k )
                    m_xFormatConditions->removeByIndex(k);

                ::comphelper::copyProperties( m_xCopy.get(), m_xFormatConditions.get() );
            }
            catch ( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("reportdesign");
                nRet = RET_NO;
            }
        }
        return nRet;
    }


    bool ConditionalFormattingDialog::PreNotify( NotifyEvent& _rNEvt )
    {
        switch ( _rNEvt.GetType() )
        {
            case MouseNotifyEvent::KEYINPUT:
            {
                const KeyEvent* pKeyEvent( _rNEvt.GetKeyEvent() );
                const vcl::KeyCode& rKeyCode = pKeyEvent->GetKeyCode();
                if ( rKeyCode.IsMod1() && rKeyCode.IsMod2() )
                {
                    if ( rKeyCode.GetCode() == 0x0508 )
                    {
                        impl_deleteCondition_nothrow( impl_getFocusedConditionIndex( 0 ) );
                        return true;
                    }
                    if ( rKeyCode.GetCode() == 0x0507 ) // +
                    {
                        impl_addCondition_nothrow( impl_getFocusedConditionIndex( impl_getConditionCount() - 1 ) + 1 );
                        return true;
                    }
                }
                break;
            }
            case MouseNotifyEvent::GETFOCUS:
            {
                if (m_bDeletingCondition)
                    break;

                if (!m_pConditionPlayground) //e.g. during dispose
                    break;

                const vcl::Window* pGetFocusWindow( _rNEvt.GetWindow() );

                // determine whether the new focus window is part of an (currently invisible) condition
                const vcl::Window* pConditionCandidate = pGetFocusWindow->GetParent();
                const vcl::Window* pPlaygroundCandidate = pConditionCandidate ? pConditionCandidate->GetParent() : nullptr;
                while   (     pPlaygroundCandidate
                        &&  ( pPlaygroundCandidate != this )
                        &&  ( pPlaygroundCandidate != m_pConditionPlayground )
                        )
                {
                    pConditionCandidate = pConditionCandidate->GetParent();
                    pPlaygroundCandidate = pConditionCandidate ? pConditionCandidate->GetParent() : nullptr;
                }
                if (pConditionCandidate && pPlaygroundCandidate == m_pConditionPlayground)
                {
                    impl_ensureConditionVisible( dynamic_cast< const Condition& >( *pConditionCandidate ).getConditionIndex() );
                }
                break;
            }
            default:
                break;
        }

        return ModalDialog::PreNotify( _rNEvt );
    }


    size_t ConditionalFormattingDialog::impl_getFirstVisibleConditionIndex() const
    {
        return static_cast<size_t>(m_pCondScroll->GetThumbPos());
    }


    size_t ConditionalFormattingDialog::impl_getLastVisibleConditionIndex() const
    {
        return ::std::min( impl_getFirstVisibleConditionIndex() + MAX_CONDITIONS, impl_getConditionCount() ) - 1;
    }


    size_t ConditionalFormattingDialog::impl_getFocusedConditionIndex( sal_Int32 _nFallBackIfNone ) const
    {
        auto cond = std::find_if(m_aConditions.begin(), m_aConditions.end(),
            [](const VclPtr<Condition>& rxCondition) { return rxCondition->HasChildPathFocus(); });
        if (cond != m_aConditions.end())
            return static_cast<size_t>(std::distance(m_aConditions.begin(), cond));
        return _nFallBackIfNone;
    }


    void ConditionalFormattingDialog::impl_updateScrollBarRange()
    {
        long nMax = ( impl_getConditionCount() > MAX_CONDITIONS ) ? impl_getConditionCount() - MAX_CONDITIONS + 1 : 0;

        m_pCondScroll->SetRangeMin( 0 );
        m_pCondScroll->SetRangeMax( nMax );
        m_pCondScroll->SetVisibleSize( 1 );
    }


    void ConditionalFormattingDialog::impl_scrollTo( size_t _nTopCondIndex )
    {
        OSL_PRECOND( _nTopCondIndex + MAX_CONDITIONS <= impl_getConditionCount(),
            "ConditionalFormattingDialog::impl_scrollTo: illegal index!" );
        m_pCondScroll->SetThumbPos( _nTopCondIndex );
        OnScroll( m_pCondScroll );
    }


    void ConditionalFormattingDialog::impl_ensureConditionVisible( size_t _nCondIndex )
    {
        OSL_PRECOND( _nCondIndex < impl_getConditionCount(),
            "ConditionalFormattingDialog::impl_ensureConditionVisible: illegal index!" );

        if ( _nCondIndex < impl_getFirstVisibleConditionIndex() )
            impl_scrollTo( _nCondIndex );
        else if ( _nCondIndex > impl_getLastVisibleConditionIndex() )
            impl_scrollTo( _nCondIndex - MAX_CONDITIONS + 1 );
    }


} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
