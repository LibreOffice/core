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

#ifndef RPTUI_CONDITION_HXX
#define RPTUI_CONDITION_HXX

#include "conditionalexpression.hxx"

#include <com/sun/star/report/XFormatCondition.hpp>

#include <dbaccess/ToolBoxHelper.hxx>

#include <svx/fntctrl.hxx>

#include <svtools/valueset.hxx>

#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>

#include <memory>

namespace svx { class ToolboxButtonColorUpdater; }

namespace rptui
{
    class ConditionalFormattingDialog;
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
            @param  _bHiContast
                <TRUE/> when in high contrast mode.
        */
        virtual ImageList getImageList(sal_Int16 _eBitmapSet,sal_Bool _bHiContast) const;

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

