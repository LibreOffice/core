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

#ifndef RPTUI_ADDFIELDWINDOW_HXX
#define RPTUI_ADDFIELDWINDOW_HXX

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <svtools/transfer.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <svtools/svtreebx.hxx>
#include <svtools/transfer.hxx>
#include <vcl/floatwin.hxx>
#include <comphelper/propmultiplex.hxx>
#include <comphelper/containermultiplexer.hxx>
#include <vcl/button.hxx>

#include <svx/dataaccessdescriptor.hxx>
#include "cppuhelper/basemutex.hxx"
#include <dbaccess/ToolBoxHelper.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>

#include <rtl/ref.hxx>

namespace rptui
{
//==================================================================
class OAddFieldWindow;
class OAddFieldWindowListBox;
//========================================================================
class  OAddFieldWindow  :public FloatingWindow
                    ,   public ::cppu::BaseMutex
                    ,   public ::comphelper::OPropertyChangeListener
                    ,   public ::comphelper::OContainerListener
                    ,   public dbaui::OToolBoxHelper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       m_xHoldAlive;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> m_xColumns;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xRowSet;

    ToolBox                                                                     m_aActions;

    ::std::auto_ptr<OAddFieldWindowListBox>                                     m_pListBox;
    FixedLine                                                                   m_aFixedLine;
    FixedText                                                                   m_aHelpText;

    PushButton                                                                  m_aInsertButton;
    Link                                                                        m_aCreateLink;
    ::rtl::OUString                                                             m_aCommandName;
    ::rtl::OUString                                                             m_sFilter;
    sal_Int32                                                                   m_nCommandType;
    sal_Bool                                                                    m_bEscapeProcessing;
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                   m_pChangeListener;
    ::rtl::Reference< comphelper::OContainerListenerAdapter>                    m_pContainerListener;

    DECL_LINK( OnDoubleClickHdl, void* );
    DECL_LINK( OnSelectHdl,      void* );
    DECL_LINK( OnSortAction,     ToolBox* );

    OAddFieldWindow(const OAddFieldWindow&);
    void operator =(const OAddFieldWindow&);
public:
    OAddFieldWindow(Window* pParent
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xRowSet);

    virtual ~OAddFieldWindow();
    virtual void Resize();
    virtual void GetFocus();
    virtual long PreNotify( NotifyEvent& _rNEvt );

    inline const ::rtl::OUString&       GetCommand()            const { return m_aCommandName; }
    inline sal_Int32                    GetCommandType()        const { return m_nCommandType; }
    inline sal_Bool                     GetEscapeProcessing()   const { return m_bEscapeProcessing; }
    inline void SetCreateHdl(const Link& _aCreateLink) { m_aCreateLink = _aCreateLink; }

    inline ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>  getColumns() const { return m_xColumns; }
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>              getConnection() const;

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > getSelectedFieldDescriptors();

    /** will be called when the id of the image list is needed.
        @param  _eBitmapSet
            <svtools/imgdef.hxx>
    */
    virtual ImageList getImageList(sal_Int16 _eBitmapSet) const;

    /** will be called when the controls need to be resized.
    */
    virtual void resizeControls(const Size& _rDiff);

    /// Updates the current field list
    void Update();

    /** fills the descriptor with the column name, column object, command and command type
    *
    * \param _pSelected the currently selected
    * \param _rDescriptor the descriptor will be filled
    */
    void fillDescriptor(SvLBoxEntry* _pSelected,::svx::ODataAccessDescriptor& _rDescriptor);

private:
    // FmXChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw( ::com::sun::star::uno::RuntimeException );
    // OContainerListener
    virtual void _elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
    virtual void _elementRemoved( const  ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
    virtual void _elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
};
//==============================================================================
} // rptui
//==============================================================================
#endif // RPTUI_ADDFIELDWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
