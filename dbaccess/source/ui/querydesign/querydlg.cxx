/*************************************************************************
 *
 *  $RCSfile: querydlg.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 15:49:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

DBG_NAME(DlgQryJoin);
DlgQryJoin::DlgQryJoin( Window * pParent,
                       OQueryTableConnectionData* _pData,
                       OJoinTableView::OTableWindowMap* _pTableMap,
                       const Reference< XConnection >& _xConnection,
                       BOOL _bAllowTableSelect)
    : ModalDialog( pParent, ModuleRes(DLG_QRY_JOIN) ),
    aFL_Join( this, ResId( FL_JOIN ) ),
    aML_HelpText( this, ResId(ML_HELPTEXT) ),
    aFT_Title( this, ResId(FT_LISTBOXTITLE) ),
    aLB_JoinType( this, ResId(LB_JOINTYPE) ),
    aPB_OK( this, ResId( PB_OK ) ),
    aPB_CANCEL( this, ResId( PB_CANCEL ) ),
    aPB_HELP( this, ResId( PB_HELP ) ),
    eJoinType(_pData->GetJoinType()),
    m_pConnData(NULL),
    m_xConnection(_xConnection),
    m_pTableMap(_pTableMap),
    m_pOrigConnData(_pData)

{
    DBG_CTOR(DlgQryJoin,NULL);

    aML_HelpText.SetControlBackground( GetSettings().GetStyleSettings().GetFaceColor() );
    //////////////////////////////////////////////////////////////////////
    // Connection kopieren
    m_pConnData = static_cast<OQueryTableConnectionData*>(_pData->NewInstance());
    m_pConnData->CopyFrom(*_pData);

    m_pTableControl = new OTableListBoxControl(this,ModuleRes(LB_CONTROL),m_pTableMap,this);

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
    try
    {
        Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
        bFull = xMeta->supportsFullOuterJoins();
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
    else if ( !(bFull && bOuter) )
        aLB_JoinType.Disable();
    else
    {
        if ( !bFull )
            aLB_JoinType.RemoveEntry(3);

        if ( !(bFull || bOuter) )
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
IMPL_LINK( DlgQryJoin, LBChangeHdl, ListBox*, pListBox )
{
    DBG_CHKTHIS(DlgQryJoin,NULL);
    aML_HelpText.SetText(String());

    String sFirstWinName,sSecondWinName;
    USHORT nResId = 0;
    USHORT nPos = aLB_JoinType.GetSelectEntryPos();
    switch ( nPos )
    {
        default:
        case 0:
            nResId = STR_QUERY_INNER_JOIN;
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

    String sStr = String(ModuleRes(nResId));
    if( sFirstWinName.Len() )
    {
        sStr.SearchAndReplace(String(RTL_CONSTASCII_STRINGPARAM("%1")),sFirstWinName);
        sStr.SearchAndReplace(String(RTL_CONSTASCII_STRINGPARAM("%2")),sSecondWinName);
    }
    aML_HelpText.SetText(sStr);
    return 1;
}
// -----------------------------------------------------------------------------

IMPL_LINK( DlgQryJoin, OKClickHdl, Button*, pButton )
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
void DlgQryJoin::notifyConnectionChange(OTableConnectionData* _pConnectionData)
{
    setJoinType(m_pConnData->GetJoinType());
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
    }
    aLB_JoinType.SelectEntryPos(nPos);
    LBChangeHdl(&aLB_JoinType);
}
// -----------------------------------------------------------------------------



