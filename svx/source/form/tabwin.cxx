/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabwin.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:26:25 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "tabwin.hxx"
#include "fmtools.hxx"
#include "fmservs.hxx"
#include "stringlistresource.hxx"

#include <svx/svxids.hrc>
#include <svx/dbaexchange.hxx>

#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPREPAREDSTATEMENT_HPP_
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XLOCALIZEDALIASES_HPP_
#include <com/sun/star/util/XLocalizedAliases.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _SVX_FMHELP_HRC
#include "fmhelp.hrc"
#endif

#ifndef _SVX_FMSHELL_HXX
#include <svx/fmshell.hxx>
#endif
#ifndef _SVX_FMSHIMP_HXX
#include "fmshimp.hxx"
#endif

#ifndef SVX_DBTOOLSCLIENT_HXX
#include "dbtoolsclient.hxx"
#endif

#ifndef _SVX_FMPAGE_HXX
#include <svx/fmpage.hxx>
#endif

#ifndef _SVX_FMPGEIMP_HXX
#include "fmpgeimp.hxx"
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif

#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif

#ifndef _SVX_DIALMGR_HXX
#include <svx/dialmgr.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif

#ifndef _SFX_OBJITEM_HXX //autogen
#include <sfx2/objitem.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _SFXFRAME_HXX
#include <sfx2/frame.hxx>
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif

const long STD_WIN_POS_X = 50;
const long STD_WIN_POS_Y = 50;

const long STD_WIN_SIZE_X = 120;
const long STD_WIN_SIZE_Y = 150;

const long MIN_WIN_SIZE_X = 50;
const long MIN_WIN_SIZE_Y = 50;

const long LISTBOX_BORDER = 2;

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::container;
using namespace ::svxform;
using namespace ::svx;

//==================================================================
// class FmFieldWinListBox
//==================================================================
DBG_NAME(FmFieldWinListBox)
//------------------------------------------------------------------------------
FmFieldWinListBox::FmFieldWinListBox( FmFieldWin* pParent )
    :SvTreeListBox( pParent, WB_HASBUTTONS|WB_BORDER )
    ,pTabWin( pParent )
{
    DBG_CTOR(FmFieldWinListBox,NULL);
    SetHelpId( HID_FIELD_SEL );

    SetHighlightRange( );
}

//------------------------------------------------------------------------------
FmFieldWinListBox::~FmFieldWinListBox()
{
    DBG_DTOR(FmFieldWinListBox,NULL);
}

//------------------------------------------------------------------------------
sal_Int8 FmFieldWinListBox::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}

//------------------------------------------------------------------------------
sal_Int8 FmFieldWinListBox::ExecuteDrop( const ExecuteDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}

//------------------------------------------------------------------------------
BOOL FmFieldWinListBox::DoubleClickHdl()
{
    if ( pTabWin->createSelectionControls() )
        return sal_True;

    return SvTreeListBox::DoubleClickHdl();
}

//------------------------------------------------------------------------------
void FmFieldWinListBox::StartDrag( sal_Int8 /*_nAction*/, const Point& /*_rPosPixel*/ )
{
    SvLBoxEntry* pSelected = FirstSelected();
    if (!pSelected)
        // no drag without a field
        return;

    ::svx::ODataAccessDescriptor aDescriptor;
    aDescriptor[ daDataSource ] <<= pTabWin->GetDatabaseName();
    aDescriptor[ daConnection ] <<= pTabWin->GetConnection().getTyped();
    aDescriptor[ daCommand ]    <<= pTabWin->GetObjectName();
    aDescriptor[ daCommandType ]<<= pTabWin->GetObjectType();
    aDescriptor[ daColumnName ] <<= ::rtl::OUString( GetEntryText( pSelected ) );

    TransferableHelper* pTransferColumn = new OColumnTransferable(
        aDescriptor, CTF_FIELD_DESCRIPTOR | CTF_CONTROL_EXCHANGE | CTF_COLUMN_DESCRIPTOR
    );
    Reference< XTransferable> xEnsureDelete = pTransferColumn;
    if (pTransferColumn)
    {
        EndSelection();
        pTransferColumn->StartDrag( this, DND_ACTION_COPY );
    }
}

//========================================================================
// class FmFieldWinData
//========================================================================
DBG_NAME(FmFieldWinData);
//-----------------------------------------------------------------------
FmFieldWinData::FmFieldWinData()
{
    DBG_CTOR(FmFieldWinData,NULL);
}

//-----------------------------------------------------------------------
FmFieldWinData::~FmFieldWinData()
{
    DBG_DTOR(FmFieldWinData,NULL);
}

//========================================================================
// class FmFieldWin
//========================================================================
DBG_NAME(FmFieldWin);
//-----------------------------------------------------------------------
FmFieldWin::FmFieldWin(SfxBindings* _pBindings, SfxChildWindow* _pMgr, Window* _pParent)
            :SfxFloatingWindow(_pBindings, _pMgr, _pParent, WinBits(WB_STDMODELESS|WB_SIZEABLE))
            ,SfxControllerItem(SID_FM_FIELDS_CONTROL, *_pBindings)
            ,::comphelper::OPropertyChangeListener(m_aMutex)
            ,pData(new FmFieldWinData)
            ,m_nObjectType(0)
            ,m_pChangeListener(NULL)
{
    DBG_CTOR(FmFieldWin,NULL);
    SetHelpId( HID_FIELD_SEL_WIN );

    SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor()) );
    pListBox = new FmFieldWinListBox( this );
    pListBox->Show();
    UpdateContent(NULL);
    SetSizePixel(Size(STD_WIN_SIZE_X,STD_WIN_SIZE_Y));
}

//-----------------------------------------------------------------------
FmFieldWin::~FmFieldWin()
{
    if (m_pChangeListener)
    {
        m_pChangeListener->dispose();
        m_pChangeListener->release();
        //  delete m_pChangeListener;
    }
    delete pListBox;
    delete pData;
    DBG_DTOR(FmFieldWin,NULL);
}

//-----------------------------------------------------------------------
void FmFieldWin::GetFocus()
{
    if ( pListBox )
        pListBox->GrabFocus();
    else
        SfxFloatingWindow::GetFocus();
}

//-----------------------------------------------------------------------
sal_Bool FmFieldWin::createSelectionControls( )
{
    SvLBoxEntry* pSelected = pListBox->FirstSelected();
    if ( pSelected )
    {
        // build a descriptor for the currently selected field
        ODataAccessDescriptor aDescr;
        aDescr.setDataSource(GetDatabaseName());

        aDescr[ daConnection ]  <<= GetConnection().getTyped();

        aDescr[ daCommand ]     <<= GetObjectName();
        aDescr[ daCommandType ] <<= GetObjectType();
        aDescr[ daColumnName ]  <<= ::rtl::OUString( pListBox->GetEntryText( pSelected) );

        // transfer this to the SFX world
        SfxUnoAnyItem aDescriptorItem( SID_FM_DATACCESS_DESCRIPTOR, makeAny( aDescr.createPropertyValueSequence() ) );
        const SfxPoolItem* pArgs[] =
        {
            &aDescriptorItem, NULL
        };

        // execute the create slot
        GetBindings().Execute( SID_FM_CREATE_FIELDCONTROL, pArgs );
    }

    return NULL != pSelected;
}

//-----------------------------------------------------------------------
long FmFieldWin::PreNotify( NotifyEvent& _rNEvt )
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

    return SfxFloatingWindow::PreNotify( _rNEvt );
}

//-----------------------------------------------------------------------
sal_Bool FmFieldWin::Close()
{
    return SfxFloatingWindow::Close();
}

//-----------------------------------------------------------------------
void FmFieldWin::_propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xForm(evt.Source, ::com::sun::star::uno::UNO_QUERY);
    UpdateContent(xForm);
}

//-----------------------------------------------------------------------
void FmFieldWin::StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    if (!pState  || SID_FM_FIELDS_CONTROL != nSID)
        return;

    if (eState >= SFX_ITEM_AVAILABLE)
    {
        FmFormShell* pShell = PTR_CAST(FmFormShell,((SfxObjectItem*)pState)->GetShell());
        UpdateContent(pShell);
    }
    else
        UpdateContent(NULL);
}

//-----------------------------------------------------------------------
void FmFieldWin::UpdateContent(FmFormShell* pShell)
{
    pListBox->Clear();
    String aTitle( SVX_RES( RID_STR_FIELDSELECTION ) );
    SetText( aTitle );

    if (!pShell || !pShell->GetImpl())
        return;

    Reference< XForm >  xForm = pShell->GetImpl()->getCurrentForm();
    if ( xForm.is() )
        UpdateContent( xForm );
}

//-----------------------------------------------------------------------
void FmFieldWin::UpdateContent(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > & xForm)
{
    try
    {
        // ListBox loeschen
        pListBox->Clear();
        UniString aTitle(SVX_RES(RID_STR_FIELDSELECTION));
        SetText(aTitle);

        if (!xForm.is())
            return;

        Reference< XPreparedStatement >  xStatement;
        Reference< XPropertySet >  xSet(xForm, UNO_QUERY);

        m_aObjectName   = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_COMMAND));
        m_aDatabaseName = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_DATASOURCE));
        m_nObjectType   = ::comphelper::getINT32(xSet->getPropertyValue(FM_PROP_COMMANDTYPE));

        // get the connection of the form
        OStaticDataAccessTools aTools;
        m_aConnection.reset(
            aTools.connectRowset( Reference< XRowSet >( xForm, UNO_QUERY ), ::comphelper::getProcessServiceFactory(), sal_True ),
            SharedConnection::NoTakeOwnership
        );
        // TODO: When incompatible changes (such as extending the "virtualdbtools" interface by ensureRowSetConnection)
        // are allowed, again, we should change this: dbtools should consistently use SharedConnection all over
        // the place, and connectRowset should be replaced with ensureRowSetConnection

        // get the fields of the object
        Sequence< ::rtl::OUString> aFieldNames;
        if ( m_aConnection.is() && m_aObjectName.getLength() )
            aFieldNames = getFieldNamesByCommandDescriptor( m_aConnection, m_nObjectType, m_aObjectName );

        // put them into the list
        const ::rtl::OUString* pFieldNames = aFieldNames.getConstArray();
        sal_Int32 nFieldsCount = aFieldNames.getLength();
        for ( sal_Int32 i = 0; i < nFieldsCount; ++i, ++pFieldNames)
            pListBox->InsertEntry( * pFieldNames);

        // Prefix setzen
        UniString  aPrefix;
        StringListResource aPrefixes( SVX_RES( RID_RSC_TABWIN_PREFIX ) );

        switch (m_nObjectType)
        {
            case CommandType::TABLE:
                aPrefix = aPrefixes[0];
                break;
            case CommandType::QUERY:
                aPrefix = aPrefixes[1];
                break;
            default:
                aPrefix = aPrefixes[2];
                break;
        }

        // an dem PropertySet nach Aenderungen der ControlSource lauschen
        if (m_pChangeListener)
        {
            m_pChangeListener->dispose();
            m_pChangeListener->release();
        }
        m_pChangeListener = new ::comphelper::OPropertyChangeMultiplexer(this, xSet);
        m_pChangeListener->acquire();
        m_pChangeListener->addProperty(FM_PROP_DATASOURCE);
        m_pChangeListener->addProperty(FM_PROP_COMMAND);
        m_pChangeListener->addProperty(FM_PROP_COMMANDTYPE);

        // Titel setzen
        aTitle.AppendAscii(" ");
        aTitle += aPrefix;
        aTitle.AppendAscii(" ");
        aTitle += m_aObjectName.getStr();
        SetText( aTitle );
    }
    catch( const Exception& )
    {
        DBG_ERROR( "FmTabWin::UpdateContent: caught an exception!" );
    }
}

//-----------------------------------------------------------------------
void FmFieldWin::Resize()
{
    SfxFloatingWindow::Resize();

    Point aPos(GetPosPixel());
    Size aOutputSize( GetOutputSizePixel() );

    //////////////////////////////////////////////////////////////////////

    // Groesse der ::com::sun::star::form::ListBox anpassen
    Point aLBPos( LISTBOX_BORDER, LISTBOX_BORDER );
    Size aLBSize( aOutputSize );
    aLBSize.Width() -= (2*LISTBOX_BORDER);
    aLBSize.Height() -= (2*LISTBOX_BORDER);

    pListBox->SetPosSizePixel( aLBPos, aLBSize );
}

//-----------------------------------------------------------------------
void FmFieldWin::FillInfo( SfxChildWinInfo& rInfo ) const
{
    rInfo.bVisible = sal_False;
}

//-----------------------------------------------------------------------
SFX_IMPL_FLOATINGWINDOW(FmFieldWinMgr, SID_FM_ADD_FIELD)

//-----------------------------------------------------------------------
FmFieldWinMgr::FmFieldWinMgr(Window* _pParent, sal_uInt16 _nId,
               SfxBindings* _pBindings, SfxChildWinInfo* _pInfo)
              :SfxChildWindow(_pParent, _nId)
{
    pWindow = new FmFieldWin(_pBindings, this, _pParent);
    SetHideNotDelete(sal_True);
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    ((SfxFloatingWindow*)pWindow)->Initialize( _pInfo );
}


