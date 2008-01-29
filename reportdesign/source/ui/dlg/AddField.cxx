/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AddField.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:46:35 $
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
#include "precompiled_reportdesign.hxx"

#ifndef RPTUI_ADDFIELDWINDOW_HXX
#include "AddField.hxx"
#endif
#include "UITools.hxx"

#ifndef _SVX_DBAEXCHANGE_HXX_
#include <svx/dbaexchange.hxx>
#endif

#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XDOCUMENTDATASOURCE_HPP_
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _RPTUI_SLOTID_HRC_
#include "rptui_slotid.hrc"
#endif
#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef RTPUI_REPORTDESIGN_HELPID_HRC
#include "helpids.hrc"
#endif
#ifndef _RPTUI_DLGRESID_HRC
#include "RptResId.hrc"
#endif
#ifndef _RPTUI_MODULE_HELPER_RPT_HXX_
#include "ModuleHelper.hxx"
#endif
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

namespace rptui
{
const long STD_WIN_SIZE_X = 120;
const long STD_WIN_SIZE_Y = 150;

const long LISTBOX_BORDER = 2;

using namespace ::com::sun::star;
using namespace sdbc;
using namespace sdb;
using namespace uno;
using namespace datatransfer;
using namespace beans;
using namespace lang;
using namespace container;
using namespace ::svx;
class OAddFieldWindowListBox    : public SvTreeListBox
{
    OAddFieldWindow* m_pTabWin;

    OAddFieldWindowListBox(const OAddFieldWindowListBox&);
    void operator =(const OAddFieldWindowListBox&);
protected:
//  virtual void Command( const CommandEvent& rEvt );

public:
    OAddFieldWindowListBox( OAddFieldWindow* pParent );
    virtual ~OAddFieldWindowListBox();

    sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );
    sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );

    uno::Sequence< beans::PropertyValue > getSelectedFieldDescriptor();

protected:
    // DragSourceHelper
    virtual void StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    // SvLBox
    virtual BOOL DoubleClickHdl();
private:
    using SvTreeListBox::ExecuteDrop;
};
// -----------------------------------------------------------------------------
uno::Sequence< beans::PropertyValue > OAddFieldWindowListBox::getSelectedFieldDescriptor()
{
    uno::Sequence< beans::PropertyValue > aArgs(GetSelectionCount());
    sal_Int32 i = 0;
    SvLBoxEntry* pSelected = FirstSelected();
    while( pSelected )
    {
        // build a descriptor for the currently selected field
        ::svx::ODataAccessDescriptor aDescriptor;
        m_pTabWin->fillDescriptor(pSelected,aDescriptor);
        aArgs[i++].Value <<= aDescriptor.createPropertyValueSequence();
        pSelected = NextSelected(pSelected);
    }
    return aArgs;
}
//==================================================================
// class OAddFieldWindowListBox
//==================================================================
DBG_NAME( rpt_OAddFieldWindowListBox );
//------------------------------------------------------------------------------
OAddFieldWindowListBox::OAddFieldWindowListBox( OAddFieldWindow* pParent )
    :SvTreeListBox( pParent, WB_HASBUTTONS|WB_BORDER )
    ,m_pTabWin( pParent )
{
    DBG_CTOR( rpt_OAddFieldWindowListBox,NULL);
    SetHelpId( HID_RPT_FIELD_SEL );
    SetSelectionMode(MULTIPLE_SELECTION);
    SetHighlightRange( );
}

//------------------------------------------------------------------------------
OAddFieldWindowListBox::~OAddFieldWindowListBox()
{
    DBG_DTOR( rpt_OAddFieldWindowListBox,NULL);
}

//------------------------------------------------------------------------------
sal_Int8 OAddFieldWindowListBox::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}

//------------------------------------------------------------------------------
sal_Int8 OAddFieldWindowListBox::ExecuteDrop( const ExecuteDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}

//------------------------------------------------------------------------------
BOOL OAddFieldWindowListBox::DoubleClickHdl()
{
    if ( m_pTabWin->createSelectionControls() )
        return sal_True;

    return SvTreeListBox::DoubleClickHdl();
}

//------------------------------------------------------------------------------
void OAddFieldWindowListBox::StartDrag( sal_Int8 /*_nAction*/, const Point& /*_rPosPixel*/ )
{
    if ( GetSelectionCount() != 1 )
        // no drag without a field or with more than one
        return;

    ::svx::ODataAccessDescriptor aDescriptor;
    m_pTabWin->fillDescriptor(FirstSelected(),aDescriptor);

    TransferableHelper* pTransferColumn = new ::svx::OColumnTransferable(aDescriptor, CTF_FIELD_DESCRIPTOR | CTF_CONTROL_EXCHANGE | CTF_COLUMN_DESCRIPTOR   );
    Reference< XTransferable> xEnsureDelete = pTransferColumn;
    EndSelection();
    pTransferColumn->StartDrag( this, DND_ACTION_COPYMOVE | DND_ACTION_LINK );
}
//========================================================================
// class OAddFieldWindow
//========================================================================
DBG_NAME( rpt_OAddFieldWindow );
//-----------------------------------------------------------------------
OAddFieldWindow::OAddFieldWindow(::rptui::OReportController& _rController,Window* pParent)
            :FloatingWindow(pParent, WinBits(WB_STDMODELESS|WB_SIZEABLE))
            ,::comphelper::OPropertyChangeListener(m_aMutex)
            ,::comphelper::OContainerListener(m_aMutex)
            ,m_pListBox(new OAddFieldWindowListBox( this ))
            ,m_rController( _rController )
            ,m_nCommandType(0)
            ,m_bEscapeProcessing(sal_False)
            ,m_pChangeListener(NULL)
{
    DBG_CTOR( rpt_OAddFieldWindow,NULL);
    SetHelpId( HID_RPT_FIELD_SEL_WIN );
    SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor()) );

    m_pListBox->Show();

    SetSizePixel(Size(STD_WIN_SIZE_X,STD_WIN_SIZE_Y));
    Show();

    try
    {
        // be notified when the settings of report definition change
        uno::Reference< beans::XPropertySet > xRowSetProps( m_rController.getRowSet(), uno::UNO_QUERY_THROW );
        m_pChangeListener = new ::comphelper::OPropertyChangeMultiplexer( this, xRowSetProps );
        m_pChangeListener->addProperty( PROPERTY_COMMAND );
        m_pChangeListener->addProperty( PROPERTY_COMMANDTYPE );
        m_pChangeListener->addProperty( PROPERTY_ESCAPEPROCESSING );
        m_pChangeListener->addProperty( PROPERTY_FILTER );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//-----------------------------------------------------------------------
OAddFieldWindow::~OAddFieldWindow()
{
    if (m_pChangeListener.is())
        m_pChangeListener->dispose();
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();
    DBG_DTOR( rpt_OAddFieldWindow,NULL);
}

//-----------------------------------------------------------------------
void OAddFieldWindow::GetFocus()
{
    if ( m_pListBox.get() )
        m_pListBox->GrabFocus();
    else
        FloatingWindow::GetFocus();
}
//-----------------------------------------------------------------------
sal_Bool OAddFieldWindow::createSelectionControls( )
{
    WaitObject aObj(this);
    uno::Sequence< beans::PropertyValue > aArgs = m_pListBox->getSelectedFieldDescriptor();
    // we use this way to create undo actions
    if ( aArgs.getLength() )
        m_rController.executeChecked(SID_ADD_CONTROL_PAIR,aArgs);

    return aArgs.getLength() != 0;
}

//-----------------------------------------------------------------------
long OAddFieldWindow::PreNotify( NotifyEvent& _rNEvt )
{
    if ( EVENT_KEYINPUT == _rNEvt.GetType() )
    {
        const KeyCode& rKeyCode = _rNEvt.GetKeyEvent()->GetKeyCode();
        if ( ( 0 == rKeyCode.GetModifier() ) && ( KEY_RETURN == rKeyCode.GetCode() ) )
        {
            if ( createSelectionControls() )
                return 1;
        }
    }

    return FloatingWindow::PreNotify( _rNEvt );
}
//-----------------------------------------------------------------------
void OAddFieldWindow::_propertyChanged( const beans::PropertyChangeEvent& _evt ) throw( uno::RuntimeException )
{
    OSL_ENSURE( _evt.Source == m_rController.getRowSet(), "OAddFieldWindow::_propertyChanged: where did this come from?" );
    (void)_evt;
    Update();
}

//-----------------------------------------------------------------------
namespace
{
    void lcl_addToList( OAddFieldWindowListBox& _rListBox, const uno::Sequence< ::rtl::OUString >& _rEntries )
    {
        const ::rtl::OUString* pEntries = _rEntries.getConstArray();
        sal_Int32 nEntries = _rEntries.getLength();
        for ( sal_Int32 i = 0; i < nEntries; ++i, ++pEntries )
            _rListBox.InsertEntry( *pEntries );
    }
}

//-----------------------------------------------------------------------
void OAddFieldWindow::Update()
{
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();
    m_pContainerListener = NULL;
    m_xColumns.clear();

    try
    {
        // ListBox loeschen
        m_pListBox->Clear();
        String aTitle(ModuleRes(RID_STR_FIELDSELECTION));
        SetText(aTitle);

        uno::Reference< beans::XPropertySet > xRowSetProps( m_rController.getRowSet(), uno::UNO_QUERY_THROW );

        ::rtl::OUString sCommand( m_aCommandName );
        sal_Int32       nCommandType( m_nCommandType );
        sal_Bool        bEscapeProcessing( m_bEscapeProcessing );
        ::rtl::OUString sFilter( m_sFilter );

        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_COMMAND ) >>= sCommand );
        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_COMMANDTYPE ) >>= nCommandType );
        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_ESCAPEPROCESSING ) >>= bEscapeProcessing );
        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_FILTER ) >>= sFilter );

        if  (   ( sCommand == m_aCommandName )
            &&  ( nCommandType == m_nCommandType  )
            &&  ( bEscapeProcessing == m_bEscapeProcessing )
            &&  ( sFilter == m_sFilter )
            )
            return;

        m_aCommandName  = sCommand;
        m_nCommandType  = nCommandType;
        m_bEscapeProcessing = bEscapeProcessing;
        m_sFilter = sFilter;

        // add the columns to the list
        uno::Reference< sdbc::XConnection> xCon = getConnection();
        if ( xCon.is() && m_aCommandName.getLength() )
            m_xColumns = dbtools::getFieldsByCommandDescriptor( xCon, GetCommandType(), GetCommand(), m_xHoldAlive );
        if ( m_xColumns.is() )
        {
            lcl_addToList( *m_pListBox, m_xColumns->getElementNames() );
            uno::Reference< container::XContainer> xContainer(m_xColumns,uno::UNO_QUERY);
            if ( xContainer.is() )
                m_pContainerListener = new ::comphelper::OContainerListenerAdapter(this,xContainer);
        }

        // add the parameter columns to the list
        Sequence< ::rtl::OUString > aParamNames( getParameterNames( m_rController.getRowSet() ) );
        lcl_addToList( *m_pListBox, aParamNames );

        // set title
        aTitle.AppendAscii(" ");
        aTitle += m_aCommandName.getStr();
        SetText( aTitle );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//-----------------------------------------------------------------------
void OAddFieldWindow::Resize()
{
    FloatingWindow::Resize();

    Point aPos(GetPosPixel());
    Size aSize( GetOutputSizePixel() );

    //////////////////////////////////////////////////////////////////////

    // Groesse der form::ListBox anpassen
    Point aLBPos( LISTBOX_BORDER, LISTBOX_BORDER );
    Size aLBSize( aSize );
    aLBSize.Width() -= (2*LISTBOX_BORDER);
    aLBSize.Height() -= (2*LISTBOX_BORDER);

    m_pListBox->SetPosSizePixel( aLBPos, aLBSize );
}
// -----------------------------------------------------------------------------
uno::Reference< sdbc::XConnection> OAddFieldWindow::getConnection() const
{
    return m_rController.getConnection();
}
// -----------------------------------------------------------------------------
void OAddFieldWindow::fillDescriptor(SvLBoxEntry* _pSelected,::svx::ODataAccessDescriptor& _rDescriptor)
{
    if ( _pSelected && m_xColumns.is() )
    {
        uno::Reference<sdb::XDocumentDataSource> xDocument( m_rController.getDataSource(), uno::UNO_QUERY );
        if ( xDocument.is() )
        {
            uno::Reference<frame::XModel> xModel(xDocument->getDatabaseDocument(),uno::UNO_QUERY);
            if ( xModel.is() )
                _rDescriptor[ daDatabaseLocation ] <<= xModel->getURL();
        }

        _rDescriptor[ ::svx::daCommand ]            <<= GetCommand();
        _rDescriptor[ ::svx::daCommandType ]        <<= GetCommandType();
        _rDescriptor[ ::svx::daEscapeProcessing ]   <<= GetEscapeProcessing();
        _rDescriptor[ ::svx::daConnection ]         <<= getConnection();

        ::rtl::OUString sColumnName = m_pListBox->GetEntryText( _pSelected );
        _rDescriptor[ ::svx::daColumnName ]         <<= sColumnName;
        if ( m_xColumns->hasByName( sColumnName ) )
            _rDescriptor[ ::svx::daColumnObject ] <<= m_xColumns->getByName(sColumnName);
    }
}
// -----------------------------------------------------------------------------
void OAddFieldWindow::_elementInserted( const container::ContainerEvent& _rEvent )  throw(::com::sun::star::uno::RuntimeException)
{
    if ( m_pListBox.get() )
    {
        ::rtl::OUString sName;
        if ( _rEvent.Accessor >>= sName )
            m_pListBox->InsertEntry(sName);
    }
}
// -----------------------------------------------------------------------------
void OAddFieldWindow::_elementRemoved( const container::ContainerEvent& /*_rEvent*/ ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( m_pListBox.get() )
    {
        m_pListBox->Clear();
        if ( m_xColumns.is() )
            lcl_addToList( *m_pListBox, m_xColumns->getElementNames() );
    }
}
// -----------------------------------------------------------------------------
void OAddFieldWindow::_elementReplaced( const container::ContainerEvent& /*_rEvent*/ ) throw(::com::sun::star::uno::RuntimeException)
{
}
// =============================================================================
} // namespace rptui
// =============================================================================
