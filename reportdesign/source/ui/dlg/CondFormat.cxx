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

#include "CondFormat.hxx"
#include "CondFormat.hrc"

#include "uistrings.hrc"
#include "RptResId.hrc"
#include "rptui_slotid.hrc"
#include "ModuleHelper.hxx"
#include "helpids.hrc"
#include "UITools.hxx"
#include "uistrings.hrc"
#include "ReportController.hxx"
#include "Condition.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <toolkit/helper/vclunohelper.hxx>

#include <vcl/msgbox.hxx>

#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

#include <comphelper/property.hxx>

#include <algorithm>
#include "UndoActions.hxx"

// .............................................................................
namespace rptui
{
// .............................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::uno::Any;
    /** === end UNO using === **/
    using namespace ::com::sun::star::report;

    //========================================================================
    // UpdateLocker
    //========================================================================
    class UpdateLocker
    {
        Window& m_rWindow;

    public:
        UpdateLocker( Window& _rWindow )
            :m_rWindow( _rWindow )
        {
            _rWindow.SetUpdateMode( sal_False );
        }
        ~UpdateLocker()
        {
            m_rWindow.SetUpdateMode( sal_True );
        }
    };

    //========================================================================
    // class ConditionalFormattingDialog
    //========================================================================
    DBG_NAME(rpt_ConditionalFormattingDialog)
    ConditionalFormattingDialog::ConditionalFormattingDialog(
            Window* _pParent, const Reference< XReportControlModel >& _rxFormatConditions, ::rptui::OReportController& _rController )
        :ModalDialog( _pParent, ModuleRes(RID_CONDFORMAT) )
        ,m_aConditionPlayground( this, ModuleRes( WND_COND_PLAYGROUND ) )
        ,m_aSeparator(this,     ModuleRes(FL_SEPARATOR1))
        ,m_aPB_OK(this,         ModuleRes(PB_OK))
        ,m_aPB_CANCEL(this,     ModuleRes(PB_CANCEL))
        ,m_aPB_Help(this,       ModuleRes(PB_HELP))
        ,m_aCondScroll( this,   ModuleRes( SB_ALL_CONDITIONS ) )
        ,m_rController( _rController )
        ,m_xFormatConditions( _rxFormatConditions )
        ,m_bDeletingCondition( false )
    {
        DBG_CTOR(rpt_ConditionalFormattingDialog,NULL);
        OSL_ENSURE( m_xFormatConditions.is(), "ConditionalFormattingDialog::ConditionalFormattingDialog: ReportControlModel is NULL -> Prepare for GPF!" );

        m_xCopy.set( m_xFormatConditions->createClone(), UNO_QUERY_THROW );

        m_aCondScroll.SetScrollHdl( LINK( this, ConditionalFormattingDialog, OnScroll ) );

        impl_initializeConditions();

        FreeResource();
    }

    //------------------------------------------------------------------------
    ConditionalFormattingDialog::~ConditionalFormattingDialog()
    {
        m_aConditions.clear();
        DBG_DTOR(rpt_ConditionalFormattingDialog,NULL);
    }

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::impl_updateConditionIndicies()
    {
        sal_Int32 nIndex = 0;
        for (   Conditions::const_iterator cond = m_aConditions.begin();
                cond != m_aConditions.end();
                ++cond, ++nIndex
            )
        {
            (*cond)->setConditionIndex( nIndex, impl_getConditionCount() );
        }
    }

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::impl_conditionCountChanged()
    {
        if ( m_aConditions.empty() )
            impl_addCondition_nothrow( 0 );

        impl_updateScrollBarRange();
        impl_updateConditionIndicies();
        impl_layoutAll();
    }

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::addCondition( size_t _nAddAfterIndex )
    {
        OSL_PRECOND( _nAddAfterIndex < impl_getConditionCount(), "ConditionalFormattingDialog::addCondition: illegal condition index!" );
        impl_addCondition_nothrow( _nAddAfterIndex + 1 );
    }

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::deleteCondition( size_t _nCondIndex )
    {
        impl_deleteCondition_nothrow( _nCondIndex );
    }

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::impl_addCondition_nothrow( size_t _nNewCondIndex )
    {
        UpdateLocker aLockUpdates( *this );

        try
        {
            if ( _nNewCondIndex > (size_t)m_xCopy->getCount() )
                throw IllegalArgumentException();

            Reference< XFormatCondition > xCond = m_xCopy->createFormatCondition();
            ::comphelper::copyProperties(m_xCopy.get(),xCond.get());
            m_xCopy->insertByIndex( _nNewCondIndex, makeAny( xCond ) );

            ConditionPtr pCon( new Condition( &m_aConditionPlayground, *this, m_rController ) );
            pCon->setCondition( xCond );
            m_aConditions.insert( m_aConditions.begin() + _nNewCondIndex, pCon );

            pCon->setPosSizePixel( 0, 0, impl_getConditionWidth(), 0, WINDOW_POSSIZE_WIDTH );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        impl_conditionCountChanged();

        impl_ensureConditionVisible( _nNewCondIndex );
    }

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::impl_focusCondition( size_t _nCondIndex )
    {
        OSL_PRECOND( _nCondIndex < impl_getConditionCount(),
            "ConditionalFormattingDialog::impl_focusCondition: illegal index!" );

        impl_ensureConditionVisible( _nCondIndex );
        m_aConditions[ _nCondIndex ]->GrabFocus();
    }

    // -----------------------------------------------------------------------------
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
                xFormatCondition->setFormula( ::rtl::OUString() );
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
            DBG_UNHANDLED_EXCEPTION();
        }

        impl_conditionCountChanged();
        if ( bSetNewFocus )
            impl_focusCondition( nNewFocusIndex );
    }

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::impl_moveCondition_nothrow( size_t _nCondIndex, bool _bMoveUp )
    {
        size_t nOldConditionIndex( _nCondIndex );
        size_t nNewConditionIndex( _bMoveUp ? _nCondIndex - 1 : _nCondIndex + 1 );

        // do this in two steps, so we don't become inconsistent if any of the UNO actions fails
        Any aMovedCondition;
        ConditionPtr pMovedCondition;
        try
        {
            aMovedCondition = m_xCopy->getByIndex( (sal_Int32)nOldConditionIndex );
            m_xCopy->removeByIndex( (sal_Int32)nOldConditionIndex );

            Conditions::iterator aRemovePos( m_aConditions.begin() + nOldConditionIndex );
            pMovedCondition = *aRemovePos;
            m_aConditions.erase( aRemovePos );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            return;
        }

        try
        {
            m_xCopy->insertByIndex( (sal_Int32)nNewConditionIndex, aMovedCondition );
            m_aConditions.insert( m_aConditions.begin() + nNewConditionIndex, pMovedCondition );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        // at least the two swapped conditions need to know their new index
        impl_updateConditionIndicies();

        // re-layout all conditions
        Point aDummy;
        impl_layoutConditions( aDummy );

        // ensure the moved condition is visible
        impl_ensureConditionVisible( nNewConditionIndex );
    }

    // -----------------------------------------------------------------------------
    long ConditionalFormattingDialog::impl_getConditionWidth() const
    {
        const Size aDialogSize( GetOutputSizePixel() );
        const Size aScrollBarWidth( LogicToPixel( Size( SCROLLBAR_WIDTH + UNRELATED_CONTROLS, 0 ), MAP_APPFONT ) );
        return aDialogSize.Width() - aScrollBarWidth.Width();
    }

    // -----------------------------------------------------------------------------
    IMPL_LINK( ConditionalFormattingDialog, OnScroll, ScrollBar*, /*_pNotInterestedIn*/ )
    {
        size_t nFirstCondIndex( impl_getFirstVisibleConditionIndex() );
        size_t nFocusCondIndex = impl_getFocusedConditionIndex( nFirstCondIndex );

        Point aDummy;
        impl_layoutConditions( aDummy );

        if ( nFocusCondIndex < nFirstCondIndex )
            impl_focusCondition( nFirstCondIndex );
        else if ( nFocusCondIndex >= nFirstCondIndex + MAX_CONDITIONS )
            impl_focusCondition( nFirstCondIndex + MAX_CONDITIONS - 1 );

        return 0;
    }

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::impl_layoutConditions( Point& _out_rBelowLastVisible )
    {
        // position the condition's playground
        long nConditionWidth = impl_getConditionWidth();
        long nConditionHeight = LogicToPixel( Size( 0, CONDITION_HEIGHT ), MAP_APPFONT ).Height();
        size_t nVisibleConditions = ::std::min( impl_getConditionCount(), MAX_CONDITIONS );
        Size aPlaygroundSize( nConditionWidth, nVisibleConditions * nConditionHeight );
        m_aConditionPlayground.SetSizePixel( aPlaygroundSize );
        _out_rBelowLastVisible = Point( 0, aPlaygroundSize.Height() );

        // position the single conditions
        Point aConditionPos( 0, -1 * nConditionHeight * impl_getFirstVisibleConditionIndex() );
        for (   Conditions::const_iterator cond = m_aConditions.begin();
                cond != m_aConditions.end();
                ++cond
            )
        {
            (*cond)->setPosSizePixel( aConditionPos.X(), aConditionPos.Y(), nConditionWidth, nConditionHeight );
            aConditionPos.Move( 0, nConditionHeight );
        }
    }

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::impl_layoutAll()
    {
        // condition's positions
        Point aPos;
        impl_layoutConditions( aPos );

        // scrollbar size and visibility
        m_aCondScroll.setPosSizePixel( 0, 0, 0, aPos.Y(), WINDOW_POSSIZE_HEIGHT );
        if ( !impl_needScrollBar() )
            // normalize the position, so it can, in all situations, be used as top index
            m_aCondScroll.SetThumbPos( 0 );

        // the separator and the buttons below it
        aPos += LogicToPixel( Point( 0 , RELATED_CONTROLS ), MAP_APPFONT );
        m_aSeparator.setPosSizePixel( 0, aPos.Y(), 0, 0, WINDOW_POSSIZE_Y );

        aPos += LogicToPixel( Point( 0 , UNRELATED_CONTROLS ), MAP_APPFONT );
        Window* pWindows[] = { &m_aPB_OK, &m_aPB_CANCEL, &m_aPB_Help };
        for ( size_t i= 0; i < sizeof(pWindows)/sizeof(pWindows[0]); ++i )
        {
            pWindows[i]->setPosSizePixel( 0, aPos.Y(), 0, 0, WINDOW_POSSIZE_Y );
        }

        aPos += LogicToPixel( Point( 0, BUTTON_HEIGHT + RELATED_CONTROLS ), MAP_APPFONT );
        setPosSizePixel( 0, 0, 0, aPos.Y(), WINDOW_POSSIZE_HEIGHT );
    }
    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::impl_initializeConditions()
    {
        try
        {
            sal_Int32 nCount = m_xCopy->getCount();
            for ( sal_Int32 i = 0; i < nCount ; ++i )
            {
                ConditionPtr pCon( new Condition( &m_aConditionPlayground, *this, m_rController ) );
                Reference< XFormatCondition > xCond( m_xCopy->getByIndex(i), UNO_QUERY );
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

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::applyCommand( size_t _nCondIndex, sal_uInt16 _nCommandId, const ::Color _aColor )
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
            aArgs[2].Value <<= (sal_uInt32)_aColor.GetColor();

            // we use this way to create undo actions
            m_rController.executeUnChecked(_nCommandId,aArgs);
            m_aConditions[ _nCondIndex ]->updateToolbar(xReportControlFormat);
        }
        catch( Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::moveConditionUp( size_t _nCondIndex )
    {
        OSL_PRECOND( _nCondIndex > 0, "ConditionalFormattingDialog::moveConditionUp: cannot move up the first condition!" );
        if ( _nCondIndex > 0 )
            impl_moveCondition_nothrow( _nCondIndex, true );
    }

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::moveConditionDown( size_t _nCondIndex )
    {
        OSL_PRECOND( _nCondIndex < impl_getConditionCount(), "ConditionalFormattingDialog::moveConditionDown: cannot move down the last condition!" );
        if ( _nCondIndex < impl_getConditionCount() )
            impl_moveCondition_nothrow( _nCondIndex, false );
    }

    // -----------------------------------------------------------------------------
    ::rtl::OUString ConditionalFormattingDialog::getDataField() const
    {
        ::rtl::OUString sDataField;
        try
        {
            sDataField = m_xFormatConditions->getDataField();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return sDataField;
    }

    // -----------------------------------------------------------------------------
    short ConditionalFormattingDialog::Execute()
    {
        short nRet = ModalDialog::Execute();
        if ( nRet == RET_OK )
        {
            const String sUndoAction( ModuleRes( RID_STR_UNDO_CONDITIONAL_FORMATTING ) );
            const UndoContext aUndoContext( m_rController.getUndoManager(), sUndoAction );
            try
            {
                sal_Int32 j(0), i(0);;
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
                    sal_Bool bAppend = j >= m_xFormatConditions->getCount();
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
                DBG_UNHANDLED_EXCEPTION();
                nRet = RET_NO;
            }
        }
        return nRet;
    }

    // -----------------------------------------------------------------------------
    long ConditionalFormattingDialog::PreNotify( NotifyEvent& _rNEvt )
    {
        switch ( _rNEvt.GetType() )
        {
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKeyEvent( _rNEvt.GetKeyEvent() );
            const KeyCode& rKeyCode = pKeyEvent->GetKeyCode();
            if ( rKeyCode.IsMod1() && rKeyCode.IsMod2() )
            {
                if ( rKeyCode.GetCode() == 0x0508 ) // -
                {
                    impl_deleteCondition_nothrow( impl_getFocusedConditionIndex( 0 ) );
                    return 1;
                }
                if ( rKeyCode.GetCode() == 0x0507 ) // +
                {
                    impl_addCondition_nothrow( impl_getFocusedConditionIndex( impl_getConditionCount() - 1 ) + 1 );
                    return 1;
                }
            }
        }
        break;
        case EVENT_GETFOCUS:
        {
            if ( m_bDeletingCondition )
                break;

            const Window* pGetFocusWindow( _rNEvt.GetWindow() );

            // determine whether the new focus window is part of an (currently invisible) condition
            const Window* pConditionCandidate = pGetFocusWindow->GetParent();
            const Window* pPlaygroundCandidate = pConditionCandidate ? pConditionCandidate->GetParent() : NULL;
            while   (   ( pPlaygroundCandidate )
                    &&  ( pPlaygroundCandidate != this )
                    &&  ( pPlaygroundCandidate != &m_aConditionPlayground )
                    )
            {
                pConditionCandidate = pConditionCandidate->GetParent();
                pPlaygroundCandidate = pConditionCandidate ? pConditionCandidate->GetParent() : NULL;
            }
            if ( pPlaygroundCandidate == &m_aConditionPlayground )
            {
                impl_ensureConditionVisible( dynamic_cast< const Condition& >( *pConditionCandidate ).getConditionIndex() );
            }
        }
        break;
        }

        return ModalDialog::PreNotify( _rNEvt );
    }

    // -----------------------------------------------------------------------------
    size_t ConditionalFormattingDialog::impl_getFirstVisibleConditionIndex() const
    {
        return (size_t)m_aCondScroll.GetThumbPos();
    }

    // -----------------------------------------------------------------------------
    size_t ConditionalFormattingDialog::impl_getLastVisibleConditionIndex() const
    {
        return ::std::min( impl_getFirstVisibleConditionIndex() + MAX_CONDITIONS, impl_getConditionCount() ) - 1;
    }

    // -----------------------------------------------------------------------------
    size_t ConditionalFormattingDialog::impl_getFocusedConditionIndex( sal_Int32 _nFallBackIfNone ) const
    {
        size_t nIndex( 0 );
        for (   Conditions::const_iterator cond = m_aConditions.begin();
                cond != m_aConditions.end();
                ++cond, ++nIndex
            )
        {
            if ( (*cond)->HasChildPathFocus() )
                return nIndex;
        }
        return _nFallBackIfNone;
    }

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::impl_updateScrollBarRange()
    {
        long nMax = ( impl_getConditionCount() > MAX_CONDITIONS ) ? impl_getConditionCount() - MAX_CONDITIONS + 1 : 0;

        m_aCondScroll.SetRangeMin( 0 );
        m_aCondScroll.SetRangeMax( nMax );
        m_aCondScroll.SetVisibleSize( 1 );
    }

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::impl_scrollTo( size_t _nTopCondIndex )
    {
        OSL_PRECOND( _nTopCondIndex + MAX_CONDITIONS <= impl_getConditionCount(),
            "ConditionalFormattingDialog::impl_scrollTo: illegal index!" );
        m_aCondScroll.SetThumbPos( _nTopCondIndex );
        OnScroll( &m_aCondScroll );
    }

    // -----------------------------------------------------------------------------
    void ConditionalFormattingDialog::impl_ensureConditionVisible( size_t _nCondIndex )
    {
        OSL_PRECOND( _nCondIndex < impl_getConditionCount(),
            "ConditionalFormattingDialog::impl_ensureConditionVisible: illegal index!" );

        if ( _nCondIndex < impl_getFirstVisibleConditionIndex() )
            impl_scrollTo( _nCondIndex );
        else if ( _nCondIndex > impl_getLastVisibleConditionIndex() )
            impl_scrollTo( _nCondIndex - MAX_CONDITIONS + 1 );
    }

// .............................................................................
} // rptui
// .............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
