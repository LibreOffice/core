/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AddField.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:09:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef RPTUI_ADDFIELDWINDOW_HXX
#define RPTUI_ADDFIELDWINDOW_HXX

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SV_FLOATWIN_HXX //autogen
#include <vcl/floatwin.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef RPTUI_REPORTCONTROLLER_HXX
#include "ReportController.hxx"
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#ifndef _CPPUHELPER_BASEMUTEX_HXX_
#include "cppuhelper/basemutex.hxx"
#endif
#include <comphelper/containermultiplexer.hxx>
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

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

