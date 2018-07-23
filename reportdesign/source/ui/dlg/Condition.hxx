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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_DLG_CONDITION_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_DLG_CONDITION_HXX

#include <conditionalexpression.hxx>

#include <com/sun/star/report/XFormatCondition.hpp>

#include <dbaccess/ToolBoxHelper.hxx>

#include <svx/colorwindow.hxx>
#include <svx/fntctrl.hxx>

#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/builder.hxx>
#include <vcl/layout.hxx>

#include <memory>

namespace svx { class ToolboxButtonColorUpdater; }

namespace rptui
{
    class OColorPopup;
    class OReportController;
    class IConditionalFormatAction;
    class Condition;

    class ConditionField
    {
        VclPtr<Condition>  m_pParent;
        VclPtr<Edit>       m_pSubEdit;
        VclPtr<PushButton> m_pFormula;

        DECL_LINK( OnFormula,   Button*, void );
    public:
        ConditionField(Condition* pParent, Edit* pSubEdit, PushButton *pFormula);
        void GrabFocus() { m_pSubEdit->GrabFocus(); }
        void Show(bool bShow) { m_pSubEdit->Show(bShow); m_pFormula->Show(bShow); }
        void SetText(const OUString& rText) { m_pSubEdit->SetText(rText); }
        OUString GetText() const { return m_pSubEdit->GetText(); }
    };

    class ConditionColorWrapper
    {
    public:
        ConditionColorWrapper(Condition* pControl);
        void SetSlotId(sal_uInt16 nSlotId) { mnSlotId = nSlotId; }
        void operator()(const OUString& rCommand, const NamedColor& rColor);
        void dispose();
    private:
        VclPtr<Condition> mxControl;
        sal_uInt16 mnSlotId;
    };

    //= Condition

    class Condition :public VclHBox
                    ,public dbaui::OToolBoxHelper
                    ,public VclBuilderContainer
    {
        sal_uInt16                  m_nBoldId;
        sal_uInt16                  m_nItalicId;
        sal_uInt16                  m_nUnderLineId;
        sal_uInt16                  m_nBackgroundColorId;
        sal_uInt16                  m_nFontColorId;
        sal_uInt16                  m_nFontDialogId;
        std::shared_ptr<PaletteManager> m_xPaletteManager;
        BorderColorStatus           m_aBorderColorStatus;
        ConditionColorWrapper       m_aColorWrapper;

        ::rptui::OReportController& m_rController;
        IConditionalFormatAction&   m_rAction;
        VclPtr<FixedText>                  m_pHeader;
        VclPtr<ListBox>                    m_pConditionType;
        VclPtr<ListBox>                    m_pOperationList;
        std::unique_ptr<ConditionField>    m_pCondLHS;
        VclPtr<FixedText>                  m_pOperandGlue;
        std::unique_ptr<ConditionField>    m_pCondRHS;
        VclPtr<ToolBox>                    m_pActions;
        VclPtr<SvxFontPrevWindow>          m_pPreview;
        VclPtr<PushButton>                 m_pMoveUp;
        VclPtr<PushButton>                 m_pMoveDown;
        VclPtr<PushButton>                 m_pAddCondition;
        VclPtr<PushButton>                 m_pRemoveCondition;
        VclPtr<SvxColorWindow>             m_pColorFloat;

        std::unique_ptr<svx::ToolboxButtonColorUpdater> m_pBtnUpdaterFontColor; // updates the color below the toolbar icon
        std::unique_ptr<svx::ToolboxButtonColorUpdater> m_pBtnUpdaterBackgroundColor;


        size_t                          m_nCondIndex;
        bool                            m_bInDestruction;

        ConditionalExpressions          m_aConditionalExpressions;

        DECL_LINK( OnFormatAction, ToolBox*, void );
        DECL_LINK( DropdownClick, ToolBox*, void );
        DECL_LINK( OnConditionAction, Button*, void );

    public:
        Condition( vcl::Window* _pParent, IConditionalFormatAction& _rAction, ::rptui::OReportController& _rController );
        virtual ~Condition() override;
        virtual void dispose() override;

        /** will be called when the id of the image list needs to change.
            @param  _eBitmapSet
                <svtools/imgdef.hxx>
        */
        virtual void setImageList(sal_Int16 _eBitmapSet) override;

        /** will be called when the controls need to be resized.
        */
        virtual void resizeControls(const Size& _rDiff) override;

        /** sets the props at the control
            @param  _xCondition the source
        */
        void setCondition(const css::uno::Reference< css::report::XFormatCondition >& _xCondition);

        /** fills from the control
            _xCondition the destination
        */
        void fillFormatCondition(const css::uno::Reference< css::report::XFormatCondition >& _xCondition);

        /** updates the toolbar
            _xCondition the destination
        */
        void updateToolbar(const css::uno::Reference< css::report::XReportControlFormat >& _xCondition);

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

        ::rptui::OReportController& getController() const { return m_rController; }

        sal_uInt16 mapToolbarItemToSlotId(sal_uInt16 nItemId) const;

    protected:
        virtual void GetFocus() override;

    private:
        void    impl_layoutOperands();

        inline  ConditionType
                    impl_getCurrentConditionType() const;

        inline  ComparisonOperation
                    impl_getCurrentComparisonOperation() const;

        void    impl_setCondition( const OUString& _rConditionFormula );

    private:
        DECL_LINK( OnTypeSelected, ListBox&, void );
        DECL_LINK( OnOperationSelected, ListBox&, void );
    };


    inline ConditionType Condition::impl_getCurrentConditionType() const
    {
        return sal::static_int_cast< ConditionType >( m_pConditionType->GetSelectedEntryPos() );
    }


    inline ComparisonOperation Condition::impl_getCurrentComparisonOperation() const
    {
        return sal::static_int_cast< ComparisonOperation >( m_pOperationList->GetSelectedEntryPos() );
    }


} // namespace rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_DLG_CONDITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
