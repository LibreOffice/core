/*************************************************************************
 *
 *  $RCSfile: RelationDlg.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: oj $ $Date: 2002-08-19 07:43:44 $
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
//#ifndef _SVX_TABWIN_HXX
//#include "tabwin.hxx"
//#endif
#ifndef DBAUI_RELATIONDIALOG_HRC
#include "RelationDlg.hrc"
#endif
#ifndef DBAUI_RELATIONDIALOG_HXX
#include "RelationDlg.hxx"
#endif

#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBAUI_JOINDESIGNVIEW_HXX
#include "JoinDesignView.hxx"
#endif
#ifndef DBAUI_JOINCONTROLLER_HXX
#include "JoinController.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef DBAUI_RTABLECONNECTIONDATA_HXX
#include "RTableConnectionData.hxx"
#endif
#ifndef DBAUI_RELATIONCONTROL_HXX
#include "RelationControl.hxx"
#endif

#include <algorithm>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::dbaui;
using namespace ::dbtools;

//========================================================================
// class ORelationDialog
//========================================================================
ORelationDialog::ORelationDialog( OJoinTableView* pParent,
                                 ORelationTableConnectionData* pConnectionData,
                                 BOOL bAllowTableSelect )
    : ModalDialog( pParent, ModuleRes(DLG_REL_PROPERTIES) )
    ,aFL_CascUpd(           this, ModuleRes(FL_CASC_UPD) )
    ,aRB_NoCascUpd(         this, ModuleRes(RB_NO_CASC_UPD) )
    ,aRB_CascUpd(           this, ModuleRes(RB_CASC_UPD) )
    ,aRB_CascUpdNull(       this, ModuleRes(RB_CASC_UPD_NULL) )
    ,aRB_CascUpdDefault(    this, ModuleRes(RB_CASC_UPD_DEFAULT) )
    ,aFL_CascDel(           this, ModuleRes(FL_CASC_DEL) )
    ,aRB_NoCascDel(         this, ModuleRes(RB_NO_CASC_DEL) )
    ,aRB_CascDel(           this, ModuleRes(RB_CASC_DEL) )
    ,aRB_CascDelNull(       this, ModuleRes(RB_CASC_DEL_NULL) )
    ,aRB_CascDelDefault(    this, ModuleRes(RB_CASC_DEL_DEFAULT) )
    ,m_pOrigConnData( pConnectionData )

    ,aPB_OK( this, ResId( PB_OK ) )
    ,aPB_CANCEL( this, ResId( PB_CANCEL ) )
    ,aPB_HELP( this, ResId( PB_HELP ) )
    ,m_bTriedOneUpdate(FALSE)
    ,m_pTableMap(pParent->GetTabWinMap())
    ,m_pConnData(NULL)
{
    m_xConnection = pParent->getDesignView()->getController()->getConnection();

    //////////////////////////////////////////////////////////////////////
    // Connection kopieren
    m_pConnData = static_cast<ORelationTableConnectionData*>(pConnectionData->NewInstance());
    m_pConnData->CopyFrom( *pConnectionData );

    m_pTableControl = new OTableListBoxControl(this,ModuleRes(LB_CONTROL),m_pTableMap,this);
    Init(m_pConnData);
    m_pTableControl->Init( m_pConnData );


    aPB_OK.SetClickHdl( LINK(this, ORelationDialog, OKClickHdl) );

    if (bAllowTableSelect)
        m_pTableControl->fillListBoxes();
    else
        m_pTableControl->fillAndDisable(pConnectionData);

    m_pTableControl->NotifyCellChange();

    FreeResource();
}

//------------------------------------------------------------------------
void ORelationDialog::Init(ORelationTableConnectionData* _m_pConnData)
{
    // Update Rules
    switch (_m_pConnData->GetUpdateRules())
    {
    case KeyRule::NO_ACTION:
    case KeyRule::RESTRICT:
        aRB_NoCascUpd.Check( TRUE );
        break;

    case KeyRule::CASCADE:
        aRB_CascUpd.Check( TRUE );
        break;

    case KeyRule::SET_NULL:
        aRB_CascUpdNull.Check( TRUE );
        break;
    case KeyRule::SET_DEFAULT:
        aRB_CascUpdDefault.Check( TRUE );
        break;
    }

    // Delete Rules
    switch (_m_pConnData->GetDeleteRules())
    {
    case KeyRule::NO_ACTION:
    case KeyRule::RESTRICT:
        aRB_NoCascDel.Check( TRUE );
        break;

    case KeyRule::CASCADE:
        aRB_CascDel.Check( TRUE );
        break;

    case KeyRule::SET_NULL:
        aRB_CascDelNull.Check( TRUE );
        break;
    case KeyRule::SET_DEFAULT:
        aRB_CascDelDefault.Check( TRUE );
        break;
    }
}

//------------------------------------------------------------------------
ORelationDialog::~ORelationDialog()
{
    delete m_pTableControl;
    delete m_pConnData;
}

//------------------------------------------------------------------------


//------------------------------------------------------------------------
IMPL_LINK( ORelationDialog, OKClickHdl, Button*, pButton )
{
    //////////////////////////////////////////////////////////////////////
    // RadioButtons auslesen
    UINT16 nAttrib = 0;

    // Delete Rules
    if( aRB_NoCascDel.IsChecked() )
        nAttrib |= KeyRule::NO_ACTION;
    if( aRB_CascDel.IsChecked() )
        nAttrib |= KeyRule::CASCADE;
    if( aRB_CascDelNull.IsChecked() )
        nAttrib |= KeyRule::SET_NULL;
    if( aRB_CascDelDefault.IsChecked() )
        nAttrib |= KeyRule::SET_DEFAULT;

    m_pConnData->SetDeleteRules( nAttrib );


    // Update Rules
    nAttrib = 0;
    if( aRB_NoCascUpd.IsChecked() )
        nAttrib |= KeyRule::NO_ACTION;
    if( aRB_CascUpd.IsChecked() )
        nAttrib |= KeyRule::CASCADE;
    if( aRB_CascUpdNull.IsChecked() )
        nAttrib |= KeyRule::SET_NULL;
    if( aRB_CascUpdDefault.IsChecked() )
        nAttrib |= KeyRule::SET_DEFAULT;
    m_pConnData->SetUpdateRules( nAttrib );

    m_pTableControl->SaveModified();

    // wenn die ComboBoxen fuer die Tabellenauswahl enabled sind (Constructor mit bAllowTableSelect==TRUE), dann muss ich in die
    // Connection auch die Tabellennamen stecken
    m_pConnData->SetSourceWinName(m_pTableControl->getSourceWinName());
    m_pConnData->SetDestWinName(m_pTableControl->getDestWinName());

    // try to create the relation
    try
    {
        if (m_pConnData->Update())
        {
            m_pOrigConnData->CopyFrom( *m_pConnData );
            EndDialog( RET_OK );
            return 0L;
        }
    }
    catch(SQLException& e)
    {
        ::dbaui::showError( SQLExceptionInfo(e),
                            this,
                            static_cast<OJoinTableView*>(GetParent())->getDesignView()->getController()->getORB());
    }
    catch(Exception&)
    {
        OSL_ENSURE(sal_False, "ORelationDialog, OKClickHdl: caught an exception!");
    }

    m_bTriedOneUpdate = TRUE;
    // this means that the original connection may be lost (if m_pConnData was not a newly created but an
    // existent conn to be modified), which we reflect by returning RET_NO (see ::Execute)

    // try again
    Init(m_pConnData);
    m_pTableControl->Init( m_pConnData );

    return 0;
}


//------------------------------------------------------------------------
short ORelationDialog::Execute()
{
    short nResult = ModalDialog::Execute();
    if ((nResult != RET_OK) && m_bTriedOneUpdate)
        return RET_NO;

    return nResult;
}
// -----------------------------------------------------------------------------
OTableConnectionData* ORelationDialog::getConnectionData() const
{
    return m_pConnData;
}
// -----------------------------------------------------------------------------
void ORelationDialog::setValid(sal_Bool _bValid)
{
    aPB_OK.Enable(_bValid);
}
// -----------------------------------------------------------------------------
void ORelationDialog::notifyConnectionChange(OTableConnectionData* _pConnectionData)
{
    Init(m_pConnData);
}
// -----------------------------------------------------------------------------


