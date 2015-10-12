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

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <svtools/transfer.hxx>
#include <svtools/treelistbox.hxx>
#include <vcl/floatwin.hxx>
#include <comphelper/propmultiplex.hxx>
#include <comphelper/containermultiplexer.hxx>
#include <vcl/button.hxx>

#include <svx/dataaccessdescriptor.hxx>
#include <cppuhelper/basemutex.hxx>
#include <dbaccess/ToolBoxHelper.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>

#include <rtl/ref.hxx>

namespace rptui
{

class OAddFieldWindow;
class OAddFieldWindowListBox;

class  OAddFieldWindow  :public FloatingWindow
                    ,   public ::cppu::BaseMutex
                    ,   public ::comphelper::OPropertyChangeListener
                    ,   public ::comphelper::OContainerListener
                    ,   public dbaui::OToolBoxHelper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       m_xHoldAlive;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> m_xColumns;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xRowSet;

    VclPtr<ToolBox>                                                             m_aActions;

    VclPtr<OAddFieldWindowListBox>                                              m_pListBox;
    VclPtr<FixedLine>                                                           m_aFixedLine;
    VclPtr<FixedText>                                                           m_aHelpText;

    VclPtr<PushButton>                                                          m_aInsertButton;
    Link<OAddFieldWindow&,void>                                                 m_aCreateLink;
    OUString                                                                    m_aCommandName;
    OUString                                                                    m_sFilter;
    sal_Int32                                                                   m_nCommandType;
    bool                                                                        m_bEscapeProcessing;
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                   m_pChangeListener;
    ::rtl::Reference< comphelper::OContainerListenerAdapter>                    m_pContainerListener;

    DECL_LINK_TYPED( OnClickHdl, Button*, void );
    DECL_LINK_TYPED( OnDoubleClickHdl, SvTreeListBox*, bool );
    DECL_LINK_TYPED( OnSelectHdl, SvTreeListBox*, void );
    DECL_LINK_TYPED( OnSortAction, ToolBox*, void );

    OAddFieldWindow(const OAddFieldWindow&) = delete;
    void operator =(const OAddFieldWindow&) = delete;
public:
    OAddFieldWindow(vcl::Window* pParent
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xRowSet);

    virtual ~OAddFieldWindow();
    virtual void dispose() override;
    virtual void Resize() override;
    virtual void GetFocus() override;
    virtual bool PreNotify( NotifyEvent& _rNEvt ) override;

    inline const OUString&       GetCommand()            const { return m_aCommandName; }
    inline sal_Int32                    GetCommandType()        const { return m_nCommandType; }
    inline bool                     GetEscapeProcessing()   const { return m_bEscapeProcessing; }
    inline void SetCreateHdl(const Link<OAddFieldWindow&,void>& _aCreateLink) { m_aCreateLink = _aCreateLink; }

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>              getConnection() const;

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > getSelectedFieldDescriptors();

    /** will be called when the id of the image list needs to change.
        @param  _eBitmapSet
            <svtools/imgdef.hxx>
    */
    virtual void setImageList(sal_Int16 _eBitmapSet) override;

    /** will be called when the controls need to be resized.
    */
    virtual void resizeControls(const Size& _rDiff) override;

    /// Updates the current field list
    void Update();

    /** fills the descriptor with the column name, column object, command and command type
    *
    * \param _pSelected the currently selected
    * \param _rDescriptor the descriptor will be filled
    */
    void fillDescriptor(SvTreeListEntry* _pSelected,svx::ODataAccessDescriptor& _rDescriptor);

private:
    // FmXChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    // OContainerListener
    virtual void _elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void _elementRemoved( const  ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void _elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

} // rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_ADDFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
