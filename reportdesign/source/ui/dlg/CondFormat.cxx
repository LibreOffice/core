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
#include <core_resource.hxx>
#include <ReportController.hxx>
#include "Condition.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <o3tl/safeint.hxx>
#include <toolkit/helper/vclunohelper.hxx>

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

    void ConditionalFormattingDialog::impl_setPrefHeight(bool bFirst)
    {
        if (!m_bConstructed && !bFirst)
            return;

        //allow dialog to resize itself
        size_t nCount = impl_getConditionCount();
        if (!nCount)
            return;

        auto nHeight = m_aConditions[0]->get_preferred_size().Height();
        size_t nVisibleConditions = ::std::min(nCount, MAX_CONDITIONS);
        nHeight *= nVisibleConditions;
        nHeight += 2;
        if (nHeight != m_xScrollWindow->get_size_request().Height())
        {
            m_xScrollWindow->set_size_request(-1, nHeight);
            if (!bFirst)
                m_xDialog->resize_to_request();
        }
    }

    ConditionalFormattingDialog::ConditionalFormattingDialog(
            weld::Window* _pParent, const Reference< XReportControlModel >& _rxFormatConditions, ::rptui::OReportController& _rController)
        : GenericDialogController(_pParent, "modules/dbreport/ui/condformatdialog.ui", "CondFormat")
        , m_rController(_rController)
        , m_xFormatConditions(_rxFormatConditions)
        , m_bConstructed(false)
        , m_xScrollWindow(m_xBuilder->weld_scrolled_window("scrolledwindow"))
        , m_xConditionPlayground(m_xBuilder->weld_box("condPlaygroundDrawingarea"))
    {
        OSL_ENSURE( m_xFormatConditions.is(), "ConditionalFormattingDialog::ConditionalFormattingDialog: ReportControlModel is NULL -> Prepare for GPF!" );

        m_xCopy.set( m_xFormatConditions->createClone(), UNO_QUERY_THROW );

        m_xScrollWindow->connect_vadjustment_changed(LINK(this, ConditionalFormattingDialog, OnScroll));

        impl_initializeConditions();

        impl_setPrefHeight(true);

        m_bConstructed = true;
    }

    ConditionalFormattingDialog::~ConditionalFormattingDialog()
    {
    }

    void ConditionalFormattingDialog::impl_updateConditionIndicies()
    {
        sal_Int32 nIndex = 0;
        for (const auto& rxCondition : m_aConditions)
        {
            rxCondition->setConditionIndex( nIndex, impl_getConditionCount() );
            m_xConditionPlayground->reorder_child(rxCondition->get_widget(), nIndex);
            ++nIndex;
        }
    }

    void ConditionalFormattingDialog::impl_conditionCountChanged()
    {
        if ( m_aConditions.empty() )
            impl_addCondition_nothrow( 0 );

        impl_setPrefHeight(false);

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
        try
        {
            if ( _nNewCondIndex > o3tl::make_unsigned(m_xCopy->getCount()) )
                throw IllegalArgumentException();

            Reference< XFormatCondition > xCond = m_xCopy->createFormatCondition();
            ::comphelper::copyProperties(m_xCopy, xCond);
            m_xCopy->insertByIndex( _nNewCondIndex, makeAny( xCond ) );
            auto xCon = std::make_unique<Condition>(m_xConditionPlayground.get(), m_xDialog.get(), *this, m_rController);
            xCon->setCondition(xCond);
            m_xConditionPlayground->reorder_child(xCon->get_widget(), _nNewCondIndex);
            m_aConditions.insert(m_aConditions.begin() + _nNewCondIndex, std::move(xCon));
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
        m_aConditions[ _nCondIndex ]->grab_focus();
    }

    void ConditionalFormattingDialog::impl_deleteCondition_nothrow( size_t _nCondIndex )
    {
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
                bSetNewFocus = (*pos)->has_focus();

                auto xMovedCondition = std::move(*pos);
                m_aConditions.erase(pos);
                m_xConditionPlayground->move(xMovedCondition->get_widget(), nullptr);
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
        std::unique_ptr<Condition> xMovedCondition;
        try
        {
            aMovedCondition = m_xCopy->getByIndex( static_cast<sal_Int32>(nOldConditionIndex) );
            m_xCopy->removeByIndex( static_cast<sal_Int32>(nOldConditionIndex) );

            Conditions::iterator aRemovePos( m_aConditions.begin() + nOldConditionIndex );
            xMovedCondition = std::move(*aRemovePos);
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
            m_aConditions.insert(m_aConditions.begin() + nNewConditionIndex, std::move(xMovedCondition));
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
        }

        // at least the two swapped conditions need to know their new index
        impl_updateConditionIndicies();

        // ensure the moved condition is visible
        impl_ensureConditionVisible( nNewConditionIndex );
    }

    IMPL_LINK_NOARG(ConditionalFormattingDialog, OnScroll, weld::ScrolledWindow&, void)
    {
        size_t nFirstCondIndex( impl_getFirstVisibleConditionIndex() );
        size_t nFocusCondIndex = impl_getFocusedConditionIndex( nFirstCondIndex );

        if ( nFocusCondIndex < nFirstCondIndex )
            impl_focusCondition( nFirstCondIndex );
        else if ( nFocusCondIndex >= nFirstCondIndex + MAX_CONDITIONS )
            impl_focusCondition( nFirstCondIndex + MAX_CONDITIONS - 1 );
    }

    void ConditionalFormattingDialog::impl_layoutAll()
    {
        // scrollbar visibility
        if ( m_aConditions.size() <= MAX_CONDITIONS )
            // normalize the position, so it can, in all situations, be used as top index
            m_xScrollWindow->vadjustment_set_value(0);
    }

    void ConditionalFormattingDialog::impl_initializeConditions()
    {
        try
        {
            sal_Int32 nCount = m_xCopy->getCount();
            for ( sal_Int32 i = 0; i < nCount ; ++i )
            {
                auto xCon = std::make_unique<Condition>(m_xConditionPlayground.get(), m_xDialog.get(), *this, m_rController);
                Reference< XFormatCondition > xCond( m_xCopy->getByIndex(i), UNO_QUERY );
                m_xConditionPlayground->reorder_child(xCon->get_widget(), i);
                xCon->setCondition(xCond);
                xCon->updateToolbar(xCond);
                m_aConditions.push_back(std::move(xCon));
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
            aArgs[1].Value <<= m_xDialog->GetXWindow();

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

    short ConditionalFormattingDialog::run()
    {
        short nRet = GenericDialogController::run();
        if (nRet == RET_OK)
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

                    ::comphelper::copyProperties(xCond, xNewCond);
                }

                for ( sal_Int32 k = m_xFormatConditions->getCount()-1; k >= j; --k )
                    m_xFormatConditions->removeByIndex(k);

                ::comphelper::copyProperties( m_xCopy, m_xFormatConditions );
            }
            catch ( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("reportdesign");
                nRet = RET_NO;
            }
        }
        return nRet;
    }

    size_t ConditionalFormattingDialog::impl_getFirstVisibleConditionIndex() const
    {
        auto nHeight = m_aConditions[0]->get_preferred_size().Height();
        return m_xScrollWindow->vadjustment_get_value() / nHeight;
    }

    size_t ConditionalFormattingDialog::impl_getLastVisibleConditionIndex() const
    {
        return ::std::min( impl_getFirstVisibleConditionIndex() + MAX_CONDITIONS, impl_getConditionCount() ) - 1;
    }

    size_t ConditionalFormattingDialog::impl_getFocusedConditionIndex( sal_Int32 _nFallBackIfNone ) const
    {
        auto cond = std::find_if(m_aConditions.begin(), m_aConditions.end(),
            [](const std::unique_ptr<Condition>& rxCondition) { return rxCondition->has_focus(); });
        if (cond != m_aConditions.end())
            return static_cast<size_t>(std::distance(m_aConditions.begin(), cond));
        return _nFallBackIfNone;
    }

    void ConditionalFormattingDialog::impl_scrollTo( size_t nTopCondIndex )
    {
        OSL_PRECOND( nTopCondIndex + MAX_CONDITIONS <= impl_getConditionCount(),
            "ConditionalFormattingDialog::impl_scrollTo: illegal index!" );

        auto nHeight = m_aConditions[0]->get_preferred_size().Height();
        m_xScrollWindow->vadjustment_set_value(nTopCondIndex * nHeight);
        OnScroll(*m_xScrollWindow);
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
