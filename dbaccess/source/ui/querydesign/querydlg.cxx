/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
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
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef DBAUI_QTABLECONNECTIONDATA_HXX
#include "QTableConnectionData.hxx"
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
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;

namespace dbaui
{
class OJoinControl  : public Window
{
public:
    FixedLine               aFL_Join;
    FixedText               aFT_Title;
    ListBox                 aLB_JoinType;
    CheckBox                m_aCBNatural;

    OJoinControl(Window* _pParent,const ResId& _rResId);
};
OJoinControl::OJoinControl(Window* _pParent,const ResId& _rResId)
    : Window(_pParent,_rResId)
    ,aFL_Join( this, ResId( FL_JOIN,*_rResId.GetResMgr() ) )
    ,aFT_Title( this, ResId(FT_LISTBOXTITLE,*_rResId.GetResMgr()) )
    ,aLB_JoinType( this, ResId(LB_JOINTYPE,*_rResId.GetResMgr()) )
    ,m_aCBNatural( this, ResId(CB_NATURAL,*_rResId.GetResMgr()) )
{
    FreeResource();
}
// -----------------------------------------------------------------------------
} // dbaui
// -----------------------------------------------------------------------------
DBG_NAME(DlgQryJoin)
DlgQryJoin::DlgQryJoin( OQueryTableView * pParent,
                       const TTableConnectionData::value_type& _pData,
                       OJoinTableView::OTableWindowMap* _pTableMap,
                       const Reference< XConnection >& _xConnection,
                       sal_Bool _bAllowTableSelect)
    :ModalDialog( pParent, ModuleRes(DLG_QRY_JOIN) )
    ,aML_HelpText( this, ModuleRes(ML_HELPTEXT) )
    ,aPB_OK( this, ModuleRes( PB_OK ) )
    ,aPB_CANCEL( this, ModuleRes( PB_CANCEL ) )
    ,aPB_HELP( this, ModuleRes( PB_HELP ) )
    ,m_pJoinControl( NULL )
    ,m_pTableControl( NULL )
    ,m_pTableMap(_pTableMap)
    ,m_pTableView(pParent)
    ,eJoinType(static_cast<OQueryTableConnectionData*>(_pData.get())->GetJoinType())
    ,m_pOrigConnData(_pData)
    ,m_xConnection(_xConnection)
{
    DBG_CTOR(DlgQryJoin,NULL);

    aML_HelpText.SetControlBackground( GetSettings().GetStyleSettings().GetFaceColor() );
    //////////////////////////////////////////////////////////////////////
    // Connection kopieren
    m_pConnData.reset(_pData->NewInstance());
    m_pConnData->CopyFrom(*_pData);

    m_pTableControl = new OTableListBoxControl(this,ModuleRes(WND_CONTROL),m_pTableMap,this);

    m_pJoinControl = new OJoinControl(m_pTableControl,ModuleRes(WND_JOIN_CONTROL));

    m_pJoinControl->Show();
    m_pJoinControl->m_aCBNatural.Check(static_cast<OQueryTableConnectionData*>(m_pConnData.get())->isNatural());
    m_pTableControl->Show();

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

    m_pTableControl->lateUIInit(m_pJoinControl);

    sal_Bool bSupportFullJoin = sal_False;
    Reference<XDatabaseMetaData> xMeta;
    try
    {
        xMeta = m_xConnection->getMetaData();
        if ( xMeta.is() )
            bSupportFullJoin = xMeta->supportsFullOuterJoins();
    }
    catch(SQLException&)
    {
    }
    sal_Bool bSupportOuterJoin = sal_False;
    try
    {
        if ( xMeta.is() )
            bSupportOuterJoin= xMeta->supportsOuterJoins();
    }
    catch(SQLException&)
    {
    }

    setJoinType(eJoinType);

    aPB_OK.SetClickHdl( LINK(this, DlgQryJoin, OKClickHdl) );

    m_pJoinControl->aLB_JoinType.SetSelectHdl(LINK(this,DlgQryJoin,LBChangeHdl));
    m_pJoinControl->m_aCBNatural.SetToggleHdl(LINK(this,DlgQryJoin,NaturalToggleHdl));

    if ( static_cast<OQueryTableView*>(pParent)->getDesignView()->getController().isReadOnly() )
    {
        m_pJoinControl->aLB_JoinType.Disable();
        m_pJoinControl->m_aCBNatural.Disable();
        m_pTableControl->Disable();
    }
    else
    {
        const sal_uInt16 nCount = m_pJoinControl->aLB_JoinType.GetEntryCount();
        for (sal_uInt16 i = 0; i < nCount; ++i)
        {
            const long nJoinTyp = reinterpret_cast<long>(m_pJoinControl->aLB_JoinType.GetEntryData(i));
            if ( !bSupportFullJoin && nJoinTyp == ID_FULL_JOIN )
                m_pJoinControl->aLB_JoinType.RemoveEntry(i);
            else if ( !bSupportOuterJoin && (nJoinTyp == ID_LEFT_JOIN || nJoinTyp == ID_RIGHT_JOIN) )
                m_pJoinControl->aLB_JoinType.RemoveEntry(i);
        }

        m_pTableControl->NotifyCellChange();
        m_pTableControl->enableRelation(!static_cast<OQueryTableConnectionData*>(m_pConnData.get())->isNatural() && eJoinType != CROSS_JOIN );
    }

    FreeResource();
}

//------------------------------------------------------------------------
DlgQryJoin::~DlgQryJoin()
{
    DBG_DTOR(DlgQryJoin,NULL);
    delete m_pJoinControl;
    delete m_pTableControl;
}
// -----------------------------------------------------------------------------
IMPL_LINK( DlgQryJoin, LBChangeHdl, ListBox*, /*pListBox*/ )
{
    DBG_CHKTHIS(DlgQryJoin,NULL);
    if (m_pJoinControl->aLB_JoinType.GetSelectEntryPos() == m_pJoinControl->aLB_JoinType.GetSavedValue() )
        return 1;

    m_pJoinControl->aLB_JoinType.SaveValue();
    aML_HelpText.SetText(String());

    m_pTableControl->enableRelation(true);

    String sFirstWinName    = m_pConnData->getReferencingTable()->GetWinName();
    String sSecondWinName   = m_pConnData->getReferencedTable()->GetWinName();
    const EJoinType eOldJoinType = eJoinType;
    sal_uInt16 nResId = 0;
    const sal_uInt16 nPos = m_pJoinControl->aLB_JoinType.GetSelectEntryPos();
    const long nJoinType = reinterpret_cast<long>(m_pJoinControl->aLB_JoinType.GetEntryData(nPos));
    sal_Bool bAddHint = sal_True;
    switch ( nJoinType )
    {
        default:
        case ID_INNER_JOIN:
            nResId = STR_QUERY_INNER_JOIN;
            bAddHint = sal_False;
            eJoinType = INNER_JOIN;
            break;
        case ID_LEFT_JOIN:
            nResId = STR_QUERY_LEFTRIGHT_JOIN;
            eJoinType = LEFT_JOIN;
            break;
        case ID_RIGHT_JOIN:
            {
                nResId = STR_QUERY_LEFTRIGHT_JOIN;
                eJoinType = RIGHT_JOIN;
                String sTemp = sFirstWinName;
                sFirstWinName = sSecondWinName;
                sSecondWinName = sTemp;
            }
            break;
        case ID_FULL_JOIN:
            nResId = STR_QUERY_FULL_JOIN;
            eJoinType = FULL_JOIN;
            break;
        case ID_CROSS_JOIN:
            {
                nResId = STR_QUERY_CROSS_JOIN;
                eJoinType = CROSS_JOIN;

                m_pConnData->ResetConnLines();
                m_pTableControl->lateInit();
                m_pJoinControl->m_aCBNatural.Check(sal_False);
                m_pTableControl->enableRelation(false);
                ::rtl::OUString sEmpty;
                m_pConnData->AppendConnLine(sEmpty,sEmpty);
                aPB_OK.Enable(sal_True);
            }
            break;
    }

    m_pJoinControl->m_aCBNatural.Enable(eJoinType != CROSS_JOIN);

    if ( eJoinType != eOldJoinType && eOldJoinType == CROSS_JOIN )
    {
        m_pConnData->ResetConnLines();
    }
    if ( eJoinType != CROSS_JOIN )
    {
        m_pTableControl->NotifyCellChange();
        NaturalToggleHdl(&m_pJoinControl->m_aCBNatural);
    }

    m_pTableControl->Invalidate();

    String sHelpText = String( ModuleRes( nResId ) );
    if( nPos )
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

    m_pConnData->Update();
    m_pOrigConnData->CopyFrom( *m_pConnData );

    EndDialog(RET_OK);
    return 1;
}
// -----------------------------------------------------------------------------

IMPL_LINK( DlgQryJoin, NaturalToggleHdl, CheckBox*, /*pButton*/ )
{
    DBG_CHKTHIS(DlgQryJoin,NULL);
    sal_Bool bChecked = m_pJoinControl->m_aCBNatural.IsChecked();
    static_cast<OQueryTableConnectionData*>(m_pConnData.get())->setNatural(bChecked);
    m_pTableControl->enableRelation(!bChecked);
    if ( bChecked )
    {
        m_pConnData->ResetConnLines();
        try
        {
            Reference<XNameAccess> xReferencedTableColumns(m_pConnData->getReferencedTable()->getColumns());
            Sequence< ::rtl::OUString> aSeq = m_pConnData->getReferencingTable()->getColumns()->getElementNames();
            const ::rtl::OUString* pIter = aSeq.getConstArray();
            const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
            for(;pIter != pEnd;++pIter)
            {
                if ( xReferencedTableColumns->hasByName(*pIter) )
                    m_pConnData->AppendConnLine(*pIter,*pIter);
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        m_pTableControl->NotifyCellChange();
        m_pTableControl->Invalidate();
    }

    return 1;
}
// -----------------------------------------------------------------------------
TTableConnectionData::value_type DlgQryJoin::getConnectionData() const
{
    return m_pConnData;
}
// -----------------------------------------------------------------------------
void DlgQryJoin::setValid(sal_Bool _bValid)
{
    //LBChangeHdl(&aLB_JoinType);

    aPB_OK.Enable(_bValid || eJoinType == CROSS_JOIN );
}
// -----------------------------------------------------------------------------
void DlgQryJoin::notifyConnectionChange( )
{
    setJoinType( static_cast<OQueryTableConnectionData*>(m_pConnData.get())->GetJoinType() );
    m_pJoinControl->m_aCBNatural.Check(static_cast<OQueryTableConnectionData*>(m_pConnData.get())->isNatural());
    NaturalToggleHdl(&m_pJoinControl->m_aCBNatural);
}
// -----------------------------------------------------------------------------
void DlgQryJoin::setJoinType(EJoinType _eNewJoinType)
{
    eJoinType = _eNewJoinType;
    m_pJoinControl->m_aCBNatural.Enable(eJoinType != CROSS_JOIN);

    long nJoinType = 0;
    switch ( eJoinType )
    {
        default:
        case INNER_JOIN:
            nJoinType = ID_INNER_JOIN;
            break;
        case LEFT_JOIN:
            nJoinType = ID_LEFT_JOIN;
            break;
        case RIGHT_JOIN:
            nJoinType = ID_RIGHT_JOIN;
            break;
        case FULL_JOIN:
            nJoinType = ID_FULL_JOIN;
            break;
        case CROSS_JOIN:
            nJoinType = ID_CROSS_JOIN;
            break;
    }

    const sal_uInt16 nCount = m_pJoinControl->aLB_JoinType.GetEntryCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        if ( nJoinType == reinterpret_cast<long>(m_pJoinControl->aLB_JoinType.GetEntryData(i)) )
        {
            m_pJoinControl->aLB_JoinType.SelectEntryPos(i);
            break;
        }
    }

    LBChangeHdl(&m_pJoinControl->aLB_JoinType);
}
// -----------------------------------------------------------------------------



