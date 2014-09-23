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

#include <vcl/floatwin.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <com/sun/star/report/XGroups.hpp>
#include <com/sun/star/report/XGroup.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include "GroupProperties.hxx"
#include <comphelper/propmultiplex.hxx>
#include <cppuhelper/basemutex.hxx>
#include <svtools/svmedit.hxx>
#include <rtl/ref.hxx>

#include <vector>

#include <dbaccess/ToolBoxHelper.hxx>

#include <vcl/ImageListProvider.hxx>

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

class OGroupsSortingDialog :    public FloatingWindow
                           ,    public ::cppu::BaseMutex
                           ,    public ::comphelper::OPropertyChangeListener
{
    friend class OFieldExpressionControl;

    ToolBox*                                m_pToolBox;
    sal_uInt16                              m_nMoveUpId;
    sal_uInt16                              m_nMoveDownId;
    sal_uInt16                              m_nDeleteId;

    VclContainer*                           m_pProperties;
    ListBox*                                m_pOrderLst;
    ListBox*                                m_pHeaderLst;
    ListBox*                                m_pFooterLst;
    ListBox*                                m_pGroupOnLst;
    NumericField*                           m_pGroupIntervalEd;
    ListBox*                                m_pKeepTogetherLst;
    FixedText*                              m_pHelpWindow;

    OFieldExpressionControl*                m_pFieldExpression;
    ::rptui::OReportController*             m_pController;
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                       m_pCurrentGroupListener;
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                       m_pReportListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroups>            m_xGroups;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xColumns;
    bool                                m_bReadOnly;
private:
    DECL_LINK( OnControlFocusLost, Control* );
    DECL_LINK( OnControlFocusGot, Control* );
    DECL_LINK( LBChangeHdl, ListBox* );
    DECL_LINK( OnFormatAction,      ToolBox* );

    /** returns the groups
        @return the groups which now have to check which one changes
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroups>& getGroups() { return m_xGroups; }

    ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup> getGroup(sal_Int32 _nPos)
    {
        OSL_ENSURE(_nPos >= 0 && _nPos < m_xGroups->getCount(),"Invalid count!");
        return ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup>(m_xGroups->getByIndex(_nPos),::com::sun::star::uno::UNO_QUERY);
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

    /** shows the text given by the id in the multiline edit
        @param  _nResId the string id
    */
    void showHelpText(sal_uInt16 _nResId);
    /** display the group props
        @param  _xGroup the group to display
    */
    void displayGroup(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup>& _xGroup);

    /** enables or diables the up and down button
        @param  _nRow   the row which will be active
    */
    void checkButtons(sal_Int32 _nRow);

    /** clears the m_xColumns member and reset the fields
    *
    */
    void fillColumns();
    OGroupsSortingDialog(OGroupsSortingDialog&);
    void operator =(OGroupsSortingDialog&);
protected:
    // OPropertyChangeListener
    virtual void    _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& _rEvent) throw( ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
public:
    OGroupsSortingDialog( vcl::Window* pParent
                        ,bool _bReadOnly
                        ,::rptui::OReportController* _pController);
    virtual ~OGroupsSortingDialog();

    /** sets the newe columns at the groups dialog.
        @param  _xColumns the new columns
    */
    void setColumns(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xColumns);

    /* updates the current view
    */
    void UpdateData( );
};

} // namespace rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_GROUPSSORTING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
