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

#ifndef RPTUI_CONDITION_HXX
#define RPTUI_CONDITION_HXX

#include "conditionalexpression.hxx"

#include <com/sun/star/report/XFormatCondition.hpp>

#include <dbaccess/ToolBoxHelper.hxx>

#include <svx/fntctrl.hxx>

#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>

#include <memory>

namespace svx { class ToolboxButtonColorUpdater; }

namespace rptui
{
    class OColorPopup;
    class OReportController;
    class IConditionalFormatAction;
    class Condition;

    class ConditionField : public Edit
    {
        Condition*  m_pParent;
        Edit*       m_pSubEdit;
        PushButton  m_aFormula;

        DECL_LINK( OnFormula,   Button* );
    public:
        ConditionField( Condition* pParent, const ResId& rResId );
        virtual ~ConditionField();
        virtual void Resize();
    };

    //========================================================================
    //= Condition
    //========================================================================
    class Condition :public Control
                    ,public dbaui::OToolBoxHelper
    {
        ::rptui::OReportController& m_rController;
        IConditionalFormatAction&   m_rAction;
        FixedLine                   m_aHeader;
        ListBox                     m_aConditionType;
        ListBox                     m_aOperationList;
        ConditionField              m_aCondLHS;
        FixedText                   m_aOperandGlue;
        ConditionField              m_aCondRHS;
        ToolBox                     m_aActions;
        SvxFontPrevWindow           m_aPreview;
        ImageButton                 m_aMoveUp;
        ImageButton                 m_aMoveDown;
        PushButton                  m_aAddCondition;
        PushButton                  m_aRemoveCondition;
        OColorPopup*                m_pColorFloat;

        ::svx::ToolboxButtonColorUpdater*   m_pBtnUpdaterFontColor; // updates the color below the toolbar icon
        ::svx::ToolboxButtonColorUpdater*   m_pBtnUpdaterBackgroundColor;


        size_t                          m_nCondIndex;
        long                            m_nLastKnownWindowWidth;
        bool                            m_bInDestruction;

        ConditionalExpressions          m_aConditionalExpressions;

        DECL_LINK( OnFormatAction,      ToolBox* );
        DECL_LINK( DropdownClick,       ToolBox* );
        DECL_LINK( OnConditionAction,   Button* );

    public:
        Condition( Window* _pParent, IConditionalFormatAction& _rAction, ::rptui::OReportController& _rController );
        virtual ~Condition();

        /** will be called when the id of the image list is needed.
            @param  _eBitmapSet
                <svtools/imgdef.hxx>
        */
        virtual ImageList getImageList(sal_Int16 _eBitmapSet) const;

        /** will be called when the controls need to be resized.
        */
        virtual void resizeControls(const Size& _rDiff);

        /** sets the props at the control
            @param  _xCondition the source
        */
        void setCondition(const com::sun::star::uno::Reference< com::sun::star::report::XFormatCondition >& _xCondition);

        /** fills from the control
            _xCondition the destination
        */
        void fillFormatCondition(const com::sun::star::uno::Reference< com::sun::star::report::XFormatCondition >& _xCondition);

        /** updates the toolbar
            _xCondition the destination
        */
        void updateToolbar(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportControlFormat >& _xCondition);

        /// tells the condition its new index within the dialog's condition array
        void setConditionIndex( size_t _nCondIndex, size_t _nCondCount );

        /// returns the condition's index within the dialog's condition array
        size_t  getConditionIndex() const { return m_nCondIndex; }

        /** determines whether the condition is actually empty
        */
        bool    isEmpty() const;

        /** forward to the parent class
        */
        void    ApplyCommand(sal_uInt16 _nCommandId, const ::Color& _aColor );

        inline ::rptui::OReportController& getController() const { return m_rController; }

    protected:
        virtual void StateChanged( StateChangedType nStateChange );
        virtual void DataChanged( const DataChangedEvent& rDCEvt );
        virtual void Paint( const Rectangle& rRect );
        virtual void Resize();
        virtual void GetFocus();

    private:
        void    impl_layoutAll();
        void    impl_layoutOperands();

        /// determines the rectangle to be occupied by the toolbar, including the border drawn around it
        Rectangle   impl_getToolBarBorderRect() const;

        inline  ConditionType
                    impl_getCurrentConditionType() const;

        inline  ComparisonOperation
                    impl_getCurrentComparisonOperation() const;

        void    impl_setCondition( const ::rtl::OUString& _rConditionFormula );

    private:
        DECL_LINK( OnTypeSelected, ListBox* );
        DECL_LINK( OnOperationSelected, ListBox* );
    };

    // -------------------------------------------------------------------------
    inline ConditionType Condition::impl_getCurrentConditionType() const
    {
        return sal::static_int_cast< ConditionType >( m_aConditionType.GetSelectEntryPos() );
    }

    // -------------------------------------------------------------------------
    inline ComparisonOperation Condition::impl_getCurrentComparisonOperation() const
    {
        return sal::static_int_cast< ComparisonOperation >( m_aOperationList.GetSelectEntryPos() );
    }

// =============================================================================
} // namespace rptui
// =============================================================================
#endif // RPTUI_CONDITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
