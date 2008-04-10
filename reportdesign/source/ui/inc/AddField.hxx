/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AddField.hxx,v $
 * $Revision: 1.4 $
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

#ifndef RPTUI_ADDFIELDWINDOW_HXX
#define RPTUI_ADDFIELDWINDOW_HXX

#include <svtools/svtreebx.hxx>
#include <vcl/floatwin.hxx>
#include <comphelper/propmultiplex.hxx>
#include <svtools/transfer.hxx>
#include "ReportController.hxx"
#include <svx/dataaccessdescriptor.hxx>
#include "cppuhelper/basemutex.hxx"
#include <comphelper/containermultiplexer.hxx>
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
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       m_xHoldAlive;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> m_xColumns;
    ::std::auto_ptr<OAddFieldWindowListBox>                                     m_pListBox;
    ::rptui::OReportController&                                                 m_rController;
    ::rtl::OUString                                                             m_aCommandName;
    ::rtl::OUString                                                             m_sFilter;
    sal_Int32                                                                   m_nCommandType;
    sal_Bool                                                                    m_bEscapeProcessing;
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                   m_pChangeListener;
    ::rtl::Reference< comphelper::OContainerListenerAdapter>                    m_pContainerListener;

    OAddFieldWindow(const OAddFieldWindow&);
    void operator =(const OAddFieldWindow&);
public:
    OAddFieldWindow(::rptui::OReportController& _rController,Window* pParent);

    virtual ~OAddFieldWindow();
    virtual void Resize();
    virtual void GetFocus();
    virtual long PreNotify( NotifyEvent& _rNEvt );

    inline const ::rtl::OUString&       GetCommand()            const { return m_aCommandName; }
    inline sal_Int32                    GetCommandType()        const { return m_nCommandType; }
    inline sal_Bool                     GetEscapeProcessing()   const { return m_bEscapeProcessing; }
    inline ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>  getColumns() const { return m_xColumns; }
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>              getConnection() const;

    sal_Bool    createSelectionControls( );

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

