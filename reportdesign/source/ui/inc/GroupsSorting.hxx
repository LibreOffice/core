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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_GROUPSSORTING_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_GROUPSSORTING_HXX

#include <com/sun/star/report/XGroups.hpp>
#include <com/sun/star/report/XGroup.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/propmultiplex.hxx>
#include <cppuhelper/basemutex.hxx>
#include <rtl/ref.hxx>
#include <vcl/weld.hxx>
#include <osl/diagnose.h>

class Control;

namespace comphelper
{
    class OPropertyChangeMultiplexer;
}
namespace rptui
{
class OFieldExpressionControl;
class OReportController;
/*************************************************************************
|*
|* Groups and Sorting dialog
|*
\************************************************************************/

class OGroupsSortingDialog : public weld::GenericDialogController
                           , public ::cppu::BaseMutex
                           , public ::comphelper::OPropertyChangeListener
{
    friend class OFieldExpressionControl;

    ::rptui::OReportController*                    m_pController;
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                       m_pCurrentGroupListener;
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                       m_pReportListener;
    css::uno::Reference< css::report::XGroups>            m_xGroups;
    css::uno::Reference< css::container::XNameAccess >    m_xColumns;
    bool                                                  m_bReadOnly;

    std::unique_ptr<weld::Toolbar>          m_xToolBox;
    std::unique_ptr<weld::Widget>           m_xProperties;
    std::unique_ptr<weld::ComboBox>         m_xOrderLst;
    std::unique_ptr<weld::ComboBox>         m_xHeaderLst;
    std::unique_ptr<weld::ComboBox>         m_xFooterLst;
    std::unique_ptr<weld::ComboBox>         m_xGroupOnLst;
    std::unique_ptr<weld::SpinButton>       m_xGroupIntervalEd;
    std::unique_ptr<weld::ComboBox>         m_xKeepTogetherLst;
    std::unique_ptr<weld::Label>            m_xHelpWindow;
    std::unique_ptr<weld::Container>        m_xBox;
    css::uno::Reference<css::awt::XWindow>  m_xTableCtrlParent;
    VclPtr<OFieldExpressionControl>         m_xFieldExpression;

private:
    DECL_LINK( OnWidgetFocusLost, weld::Widget&, void );
    DECL_LINK( OnWidgetFocusGot, weld::Widget&, void );

    DECL_LINK( OnControlFocusGot, LinkParamNone*, void );

    DECL_LINK( LBChangeHdl, weld::ComboBox&, void );
    DECL_LINK( OnFormatAction, const OString&, void );

    /** returns the groups
        @return the groups which now have to check which one changes
    */
    css::uno::Reference< css::report::XGroups>& getGroups() { return m_xGroups; }

    css::uno::Reference< css::report::XGroup> getGroup(sal_Int32 _nPos)
    {
        OSL_ENSURE(_nPos >= 0 && _nPos < m_xGroups->getCount(),"Invalid count!");
        return css::uno::Reference< css::report::XGroup>(m_xGroups->getByIndex(_nPos),css::uno::UNO_QUERY);
    }

    /** updates the listboxes with the new group properties
        @param  _nRow   the new group pos
    */
    void DisplayData( sal_Int32 _nRow );

    /** saves the values from the listboxes into the group at position _nRow
        @param  _nRow   the group pos to store in
    */
    void SaveData( sal_Int32 _nRow );

    /** returns <TRUE/> when the dialog should be read only
    */
    bool isReadOnly( ) const { return m_bReadOnly;}

    /** returns the data type for the given column name
        @param _sColumnName
    */
    sal_Int32 getColumnDataType(const OUString& _sColumnName);

    /** display the group props
        @param  _xGroup the group to display
    */
    void displayGroup(const css::uno::Reference< css::report::XGroup>& _xGroup);

    /** enables or disables the up and down button
        @param  _nRow   the row which will be active
    */
    void checkButtons(sal_Int32 _nRow);

    /** clears the m_xColumns member and reset the fields
    *
    */
    void fillColumns();
    OGroupsSortingDialog(OGroupsSortingDialog const &) = delete;
    void operator =(OGroupsSortingDialog const &) = delete;
protected:
    // OPropertyChangeListener
    virtual void    _propertyChanged(const css::beans::PropertyChangeEvent& _rEvent) override;
public:
    OGroupsSortingDialog(weld::Window* pParent,
                         bool _bReadOnly,
                         ::rptui::OReportController* _pController);
    virtual ~OGroupsSortingDialog() override;

    /* updates the current view
    */
    void UpdateData( );
};

} // namespace rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_GROUPSSORTING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
