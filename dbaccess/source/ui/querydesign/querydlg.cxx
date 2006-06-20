/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: querydlg.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:30:08 $
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
#ifndef DBAUI_QUERYDLG_HXX
#include "querydlg.hxx"
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif
#ifndef DBAUI_QUERYDLG_HRC
#include "querydlg.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_QTABLECONNECTIONDATA_HXX
#include "QTableConnectionData.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef DBAUI_RELATIONCONTROL_HXX
#include "RelationControl.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;

DBG_NAME(DlgQryJoin)
DlgQryJoin::DlgQryJoin( Window * pParent,
                       OQueryTableConnectionData* _pData,
                       OJoinTableView::OTableWindowMap* _pTableMap,
                       const Reference< XConnection >& _xConnection,
                       BOOL _bAllowTableSelect)
    :ModalDialog( pParent, ModuleRes(DLG_QRY_JOIN) )
    ,aFL_Join( this, ResId( FL_JOIN ) )
    ,aFT_Title( this, ResId(FT_LISTBOXTITLE) )
    ,aLB_JoinType( this, ResId(LB_JOINTYPE) )
    ,aML_HelpText( this, ResId(ML_HELPTEXT) )
    ,aPB_OK( this, ResId( PB_OK ) )
    ,aPB_CANCEL( this, ResId( PB_CANCEL ) )
    ,aPB_HELP( this, ResId( PB_HELP ) )
    ,m_pTableControl( NULL )
    ,m_pTableMap(_pTableMap)
    ,eJoinType(_pData->GetJoinType())
    ,m_pConnData(NULL)
    ,m_pOrigConnData(_pData)
    ,m_xConnection(_xConnection)
{
    DBG_CTOR(DlgQryJoin,NULL);

    aML_HelpText.SetControlBackground( GetSettings().GetStyleSettings().GetFaceColor() );
    //////////////////////////////////////////////////////////////////////
    // Connection kopieren
    m_pConnData = static_cast<OQueryTableConnectionData*>(_pData->NewInstance());
    m_pConnData->CopyFrom(*_pData);

    m_pTableControl = new OTableListBoxControl(this,ModuleRes(WND_CONTROL),m_pTableMap,this);

    if( _bAllowTableSelect )
    {
        m_pTableControl->Init( m_pConnData );
        m_pTableControl->fillListBoxes();
    }
    else
    {
        m_pTableControl->fillAndDisable(m_pConnData);
        m_pTableControl->Init( m_pConnData );
    }

    m_pTableControl->lateInit();

    sal_Bool bFull = sal_False;
    sal_Bool bOuter = sal_False;
    Reference<XDatabaseMetaData> xMeta;
    try
    {
        xMeta = m_xConnection->getMetaData();
        if ( xMeta.is() )
            bFull = xMeta->supportsFullOuterJoins();
    }
    catch(SQLException&)
    {
    }
    try
    {
        if ( xMeta.is() )
            bOuter= xMeta->supportsOuterJoins();
    }
    catch(SQLException&)
    {
    }

    setJoinType(eJoinType);

    aPB_OK.SetClickHdl( LINK(this, DlgQryJoin, OKClickHdl) );

    aLB_JoinType.SetSelectHdl(LINK(this,DlgQryJoin,LBChangeHdl));

    if ( static_cast<OQueryTableView*>(pParent)->getDesignView()->getController()->isReadOnly() )
    {
        aLB_JoinType.Disable();
        m_pTableControl->Disable();
    }
    else if ( !bFull && !bOuter )
        aLB_JoinType.Disable();
    else
    {
        if ( !bFull )
            aLB_JoinType.RemoveEntry(3);

        if ( !bOuter )
        {
            aLB_JoinType.RemoveEntry(0);
            aLB_JoinType.RemoveEntry(0);
            aLB_JoinType.RemoveEntry(0);
        }

        m_pTableControl->NotifyCellChange();
    }

    FreeResource();
}

//------------------------------------------------------------------------
DlgQryJoin::~DlgQryJoin()
{
    DBG_DTOR(DlgQryJoin,NULL);
    delete m_pTableControl;
    delete m_pConnData;
}
// -----------------------------------------------------------------------------
IMPL_LINK( DlgQryJoin, LBChangeHdl, ListBox*, /*pListBox*/ )
{
    DBG_CHKTHIS(DlgQryJoin,NULL);
    aML_HelpText.SetText(String());

    String sFirstWinName,sSecondWinName;
    USHORT nResId = 0;
    USHORT nPos = aLB_JoinType.GetSelectEntryPos();
    sal_Bool bAddHint = sal_True;
    switch ( nPos )
    {
        default:
        case 0:
            nResId = STR_QUERY_INNER_JOIN;
            bAddHint = sal_False;
            break;
        case 1:
            nResId = STR_QUERY_LEFTRIGHT_JOIN;
            sFirstWinName   = m_pConnData->GetSourceWinName();
            sSecondWinName  = m_pConnData->GetDestWinName();
            break;
        case 2:
        case 3:
            nResId = (nPos == 2) ? STR_QUERY_LEFTRIGHT_JOIN : STR_QUERY_FULL_JOIN;

            sFirstWinName   = m_pConnData->GetDestWinName();
            sSecondWinName  = m_pConnData->GetSourceWinName();
            break;
    }

    String sHelpText = String( ModuleRes( nResId ) );
    if( sFirstWinName.Len() )
    {
        sHelpText.SearchAndReplace( String( RTL_CONSTASCII_STRINGPARAM( "%1" ) ), sFirstWinName );
        sHelpText.SearchAndReplace( String( RTL_CONSTASCII_STRINGPARAM( "%2" ) ), sSecondWinName );
    }
    if ( bAddHint )
    {
        sHelpText += String( RTL_CONSTASCII_STRINGPARAM( "\n" ) );
        sHelpText += String( ModuleRes( STR_JOIN_TYPE_HINT ) );
    }

    aML_HelpText.SetText( sHelpText );
    return 1;
}
// -----------------------------------------------------------------------------

IMPL_LINK( DlgQryJoin, OKClickHdl, Button*, /*pButton*/ )
{
    DBG_CHKTHIS(DlgQryJoin,NULL);
    USHORT nPos = aLB_JoinType.GetSelectEntryPos();
    switch ( nPos )
    {
        case 0:
            eJoinType = INNER_JOIN;
            break;
        case 1:
            eJoinType = LEFT_JOIN;
            break;
        case 2:
            eJoinType = RIGHT_JOIN;
            break;
        case 3:
            eJoinType = FULL_JOIN;
            break;
    }

    m_pConnData->Update();
    m_pOrigConnData->CopyFrom( *m_pConnData );

    EndDialog(RET_OK);
    return 1;
}
// -----------------------------------------------------------------------------
OTableConnectionData* DlgQryJoin::getConnectionData() const
{
    return m_pConnData;
}
// -----------------------------------------------------------------------------
void DlgQryJoin::setValid(sal_Bool _bValid)
{
    LBChangeHdl(&aLB_JoinType);

    aPB_OK.Enable(_bValid);
}
// -----------------------------------------------------------------------------
void DlgQryJoin::notifyConnectionChange( )
{
    setJoinType( m_pConnData->GetJoinType() );
}
// -----------------------------------------------------------------------------
void DlgQryJoin::setJoinType(EJoinType _eNewJoinType)
{
    eJoinType = _eNewJoinType;
    USHORT nPos = 0;
    switch(eJoinType)
    {
        case INNER_JOIN:
            break;
        case LEFT_JOIN:
            nPos = 1;
            break;
        case RIGHT_JOIN:
            nPos = 2;
            break;
        case FULL_JOIN:
            nPos = 3;
            break;
        default:
            break;
    }
    aLB_JoinType.SelectEntryPos(nPos);
    LBChangeHdl(&aLB_JoinType);
}
// -----------------------------------------------------------------------------



