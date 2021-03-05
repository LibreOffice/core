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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_ADDFIELD_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_ADDFIELD_HXX

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <comphelper/propmultiplex.hxx>
#include <comphelper/containermultiplexer.hxx>

#include <svx/dataaccessdescriptor.hxx>
#include <svx/dbaexchange.hxx>
#include <cppuhelper/basemutex.hxx>

#include <vcl/weld.hxx>

#include <rtl/ref.hxx>

#include "ColumnInfo.hxx"

namespace rptui
{

class OAddFieldWindow;

class  OAddFieldWindow : public weld::GenericDialogController
                       , public ::cppu::BaseMutex
                       , public ::comphelper::OPropertyChangeListener
                       , public ::comphelper::OContainerListener
{
    css::uno::Reference< css::lang::XComponent>                                 m_xHoldAlive;
    css::uno::Reference< css::container::XNameAccess>                           m_xColumns;
    css::uno::Reference< css::beans::XPropertySet >                             m_xRowSet;

    std::unique_ptr<weld::Toolbar>                                              m_xActions;
    std::unique_ptr<weld::TreeView>                                             m_xListBox;
    std::unique_ptr<weld::Label>                                                m_xHelpText;

    Link<OAddFieldWindow&,void>                                                 m_aCreateLink;
    OUString                                                                    m_aCommandName;
    OUString                                                                    m_sFilter;
    sal_Int32                                                                   m_nCommandType;
    bool                                                                        m_bEscapeProcessing;
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                   m_pChangeListener;
    ::rtl::Reference< comphelper::OContainerListenerAdapter>                    m_pContainerListener;
    ::rtl::Reference< svx::OMultiColumnTransferable >                           m_xHelper;

    std::vector<std::unique_ptr<ColumnInfo>> m_aListBoxData;

    DECL_LINK( OnDoubleClickHdl, weld::TreeView&, bool );
    DECL_LINK( OnSelectHdl, weld::TreeView&, void );
    DECL_LINK( DragBeginHdl, bool&, bool );
    DECL_LINK( OnSortAction, const OString&, void );
    DECL_LINK( FocusChangeHdl, weld::Container&, void );

    void addToList(const css::uno::Sequence<OUString>& rEntries);
    void addToList(const css::uno::Reference<css::container::XNameAccess>& i_xColumns);

    OAddFieldWindow(const OAddFieldWindow&) = delete;
    void operator =(const OAddFieldWindow&) = delete;
public:
    OAddFieldWindow(weld::Window* pParent,
                    const css::uno::Reference< css::beans::XPropertySet >& xRowSet);

    virtual ~OAddFieldWindow() override;

    const OUString&       GetCommand()            const { return m_aCommandName; }
    sal_Int32                    GetCommandType()        const { return m_nCommandType; }
    void SetCreateHdl(const Link<OAddFieldWindow&,void>& _aCreateLink) { m_aCreateLink = _aCreateLink; }

    css::uno::Reference< css::sdbc::XConnection>              getConnection() const;

    css::uno::Sequence< css::beans::PropertyValue > getSelectedFieldDescriptors();

    /// Updates the current field list
    void Update();

    /** fills the descriptor with the column name, column object, command and command type
    *
    * \param rSelected the currently selected
    * \param rDescriptor the descriptor will be filled
    */
    void fillDescriptor(const weld::TreeIter& rSelected, svx::ODataAccessDescriptor& rDescriptor);

private:
    // FmXChangeListener
    virtual void _propertyChanged(const css::beans::PropertyChangeEvent& evt) override;
    // OContainerListener
    virtual void _elementInserted( const css::container::ContainerEvent& _rEvent ) override;
    virtual void _elementRemoved( const  css::container::ContainerEvent& _rEvent ) override;
    virtual void _elementReplaced( const css::container::ContainerEvent& _rEvent ) override;
};

} // rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_ADDFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
