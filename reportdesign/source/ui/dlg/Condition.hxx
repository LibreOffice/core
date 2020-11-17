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

#include <svx/colorwindow.hxx>
#include <svx/fntctrl.hxx>

#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>

#include <memory>

namespace rptui
{
    class OColorPopup;
    class OReportController;
    class IConditionalFormatAction;
    class Condition;

    class ConditionField
    {
        Condition*  m_pParent;
        std::unique_ptr<weld::Entry> m_xSubEdit;
        std::unique_ptr<weld::Button> m_xFormula;

        DECL_LINK(OnFormula, weld::Button&, void);
    public:
        ConditionField(Condition* pParent, std::unique_ptr<weld::Entry> xSubEdit, std::unique_ptr<weld::Button> xFormula);
        void grab_focus() { m_xSubEdit->grab_focus(); }
        void set_visible(bool bShow) { m_xSubEdit->set_visible(bShow); m_xFormula->set_visible(bShow); }
        void set_text(const OUString& rText) { m_xSubEdit->set_text(rText); }
        OUString get_text() const { return m_xSubEdit->get_text(); }
    };

    class ConditionColorWrapper
    {
    public:
        ConditionColorWrapper(Condition* pControl, sal_uInt16 nSlotId);
        void operator()(const OUString& rCommand, const NamedColor& rColor);
            // ColorSelectFunction signature
    private:
        Condition* mpControl;
        sal_uInt16 mnSlotId;
    };

    //= Condition

    class Condition
    {
        std::shared_ptr<PaletteManager> m_xPaletteManager;
        ColorStatus                 m_aColorStatus;
        ConditionColorWrapper       m_aBackColorWrapper;
        ConditionColorWrapper       m_aForeColorWrapper;

        ::rptui::OReportController& m_rController;
        IConditionalFormatAction&   m_rAction;

        size_t                          m_nCondIndex;

        ConditionalExpressions          m_aConditionalExpressions;

        SvxFontPrevWindow m_aPreview;
        weld::Window* m_pDialog;
        std::unique_ptr<weld::Builder>     m_xBuilder;
        std::unique_ptr<weld::Container>   m_xContainer;
        std::unique_ptr<weld::Label>       m_xHeader;
        std::unique_ptr<weld::ComboBox>    m_xConditionType;
        std::unique_ptr<weld::ComboBox>    m_xOperationList;
        std::unique_ptr<ConditionField>    m_xCondLHS;
        std::unique_ptr<weld::Label>       m_xOperandGlue;
        std::unique_ptr<ConditionField>    m_xCondRHS;
        std::unique_ptr<weld::Toolbar>     m_xActions;
        std::unique_ptr<weld::CustomWeld>  m_xPreview;
        std::unique_ptr<weld::Button>      m_xMoveUp;
        std::unique_ptr<weld::Button>      m_xMoveDown;
        std::unique_ptr<weld::Button>      m_xAddCondition;
        std::unique_ptr<weld::Button>      m_xRemoveCondition;
        std::unique_ptr<ColorWindow>       m_xBackColorFloat;
        std::unique_ptr<ColorWindow>       m_xForeColorFloat;

        void SetBackgroundDropdownClick();
        void SetForegroundDropdownClick();

        DECL_LINK( OnFormatAction, const OString&, void );
        DECL_LINK( OnConditionAction, weld::Button&, void );

    public:
        Condition(weld::Container* pParent, weld::Window* pDialog, IConditionalFormatAction& rAction, ::rptui::OReportController& rController);
        ~Condition();

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
        void    ApplyCommand( sal_uInt16 _nCommandId, const NamedColor& rNamedColor );

        ::rptui::OReportController& getController() const { return m_rController; }

        static sal_uInt16 mapToolbarItemToSlotId(const OString& rItemId);

        css::uno::Reference<css::awt::XWindow> GetXWindow() const { return m_pDialog->GetXWindow(); }

        void grab_focus() { m_xContainer->grab_focus(); }
        bool has_focus() const { return m_xContainer->has_focus(); }
        Size get_preferred_size() const { return m_xContainer->get_preferred_size(); }
        weld::Widget* get_widget() const { return m_xContainer.get(); }

    private:
        void    impl_layoutOperands();

        inline  ConditionType
                    impl_getCurrentConditionType() const;

        inline  ComparisonOperation
                    impl_getCurrentComparisonOperation() const;

        void    impl_setCondition( const OUString& _rConditionFormula );

    private:
        DECL_LINK( OnTypeSelected, weld::ComboBox&, void );
        DECL_LINK( OnOperationSelected, weld::ComboBox&, void );
    };

    inline ConditionType Condition::impl_getCurrentConditionType() const
    {
        return sal::static_int_cast<ConditionType>(m_xConditionType->get_active());
    }

    inline ComparisonOperation Condition::impl_getCurrentComparisonOperation() const
    {
        return sal::static_int_cast<ComparisonOperation>(m_xOperationList->get_active());
    }

} // namespace rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_DLG_CONDITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
