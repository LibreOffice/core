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

#ifndef RPTUI_CONDFORMAT_HXX
#define RPTUI_CONDFORMAT_HXX

#include "ModuleHelper.hxx"

#include <com/sun/star/report/XReportControlModel.hpp>

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/scrbar.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <vector>

// .............................................................................
namespace rptui
{
// .............................................................................

    #define MAX_CONDITIONS  (size_t)3

    class OReportController;
    class Condition;

    //=========================================================================
    //= IConditionalFormatAction
    //=========================================================================
    class SAL_NO_VTABLE IConditionalFormatAction
    {
    public:
        virtual void            addCondition( size_t _nAddAfterIndex ) = 0;
        virtual void            deleteCondition( size_t _nCondIndex ) = 0;
        virtual void            applyCommand( size_t _nCondIndex, sal_uInt16 _nCommandId, const ::Color _aColor ) = 0;
        virtual void            moveConditionUp( size_t _nCondIndex ) = 0;
        virtual void            moveConditionDown( size_t _nCondIndex ) = 0;
        virtual ::rtl::OUString getDataField() const = 0;
    };

    /*************************************************************************
    |*
    |* Conditional formatting dialog
    |*
    \************************************************************************/
    class ConditionalFormattingDialog  :public ModalDialog
                                        ,public IConditionalFormatAction
                                        ,private ::boost::noncopyable
    {
        typedef ::boost::shared_ptr< Condition >    ConditionPtr;
        typedef ::std::vector< ConditionPtr >       Conditions;

        OModuleClient   m_aModuleClient;
        Window          m_aConditionPlayground;
        Conditions      m_aConditions;
        FixedLine       m_aSeparator;
        OKButton        m_aPB_OK;
        CancelButton    m_aPB_CANCEL;
        HelpButton      m_aPB_Help;
        ScrollBar       m_aCondScroll;

        ::rptui::OReportController&                         m_rController;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportControlModel >
                                                            m_xFormatConditions;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportControlModel >
                                                            m_xCopy;

        bool    m_bDeletingCondition;

    public:
        ConditionalFormattingDialog(
            Window* pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportControlModel>& _xHoldAlive,
            ::rptui::OReportController& _rController
        );
        virtual ~ConditionalFormattingDialog();

        // Dialog overridables
        virtual short   Execute();

        // IConditionalFormatAction overridables
        virtual void addCondition( size_t _nAddAfterIndex );
        virtual void deleteCondition( size_t _nCondIndex );
        virtual void applyCommand( size_t _nCondIndex, sal_uInt16 _nCommandId, const ::Color _aColor );
        virtual void moveConditionUp( size_t _nCondIndex );
        virtual void moveConditionDown( size_t _nCondIndex );
        virtual ::rtl::OUString getDataField() const;

    protected:
        virtual long        PreNotify( NotifyEvent& rNEvt );

    private:
        DECL_LINK( OnScroll, ScrollBar* );

    private:
        /// returns the current number of conditions
        size_t  impl_getConditionCount() const { return m_aConditions.size(); }

        /** adds a condition
            @param _nNewCondIndex
                the index of the to-be-inserted condition
        */
        void    impl_addCondition_nothrow( size_t _nNewCondIndex );

        /// deletes the condition with the given index
        void    impl_deleteCondition_nothrow( size_t _nCondIndex );

        /// moves the condition with the given index one position
        void    impl_moveCondition_nothrow( size_t _nCondIndex, bool _bMoveUp );

        /// does the dialog layouting
        void    impl_layoutAll();

        /// does the layout for the condition windows
        void    impl_layoutConditions( Point& _out_rBelowLastVisible );

        /// called when the number of conditions has changed in any way
        void    impl_conditionCountChanged();

        /// initializes the conditions from m_xCopy
        void    impl_initializeConditions();

        /// tells all our Condition instances their new index
        void    impl_updateConditionIndicies();

        /// returns the number of the condition which has the (child path) focus
        size_t  impl_getFocusedConditionIndex( sal_Int32 _nFallBackIfNone ) const;

        /// returns the index of the first visible condition
        size_t  impl_getFirstVisibleConditionIndex() const;

        /// returns the index of the last visible condition
        size_t  impl_getLastVisibleConditionIndex() const;

        /// determines the width of a Condition
        long    impl_getConditionWidth() const;

        /// focuses the condition with the given index, making it visible if necessary
        void    impl_focusCondition( size_t _nCondIndex );

        /// updates the scrollbar range. (does not update the scrollbar visibility)
        void    impl_updateScrollBarRange();

        /// determines whether we need a scrollbar for the conditions
        bool    impl_needScrollBar() const { return m_aConditions.size() > MAX_CONDITIONS; }

        /// scrolls the condition with the given index to the top position
        void    impl_scrollTo( size_t _nTopCondIndex );

        /// ensures the condition with the given index is visible
        void    impl_ensureConditionVisible( size_t _nCondIndex );
    };

// .............................................................................
} // namespace rptui
// .............................................................................

#endif // RPTUI_CONDFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
