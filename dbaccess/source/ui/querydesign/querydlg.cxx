/*************************************************************************
 *
 *  $RCSfile: querydlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fme $ $Date: 2001-06-21 15:32:06 $
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
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
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

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;

DBG_NAME(DlgQryJoin);
DlgQryJoin::DlgQryJoin( Window * pParent,OQueryTableConnectionData* pData,
                       const Reference< XDatabaseMetaData >& _rxMetaData)
    : ModalDialog( pParent, ModuleRes(DLG_QRY_JOIN) ),
    aFL_Join( this, ResId( FL_JOIN ) ),
    aRB_Inner( this, ResId( RB_INNER ) ),
    aRB_Left( this, ResId( RB_LEFT ) ),
    aRB_Right( this, ResId( RB_RIGHT ) ),
    aRB_Full( this, ResId( RB_FULL ) ),
    aML_HelpText( this, ResId(ML_HELPTEXT) ),

    aPB_OK( this, ResId( PB_OK ) ),
    aPB_CANCEL( this, ResId( PB_CANCEL ) ),
    aPB_HELP( this, ResId( PB_HELP ) ),
    eJoinType(pData->GetJoinType()),
    pConnData(pData)

{
    DBG_CTOR(DlgQryJoin,NULL);

    aRB_Left.Disable();
    aRB_Right.Disable();
    aRB_Full.Disable();

    if(_rxMetaData->supportsFullOuterJoins())
    {
        aRB_Left.Enable();
        aRB_Right.Enable();
        aRB_Full.Enable();
    }
    else if(_rxMetaData->supportsOuterJoins())
    {
        aRB_Left.Enable();
        aRB_Right.Enable();
    }

    switch(eJoinType)
    {
        case INNER_JOIN:
            aRB_Inner.Check(sal_True);
            RBTogleHdl(&aRB_Inner);
            break;
        case LEFT_JOIN:
            aRB_Left.Check(sal_True);
            RBTogleHdl(&aRB_Left);
            break;
        case RIGHT_JOIN:
            aRB_Right.Check(sal_True);
            RBTogleHdl(&aRB_Right);
            break;
        case FULL_JOIN:
            aRB_Full.Check(sal_True);
            RBTogleHdl(&aRB_Full);
            break;
    }

    aPB_OK.SetClickHdl( LINK(this, DlgQryJoin, OKClickHdl) );

    aRB_Inner.SetToggleHdl(LINK(this,DlgQryJoin,RBTogleHdl));
    aRB_Left.SetToggleHdl(LINK(this,DlgQryJoin,RBTogleHdl));
    aRB_Right.SetToggleHdl(LINK(this,DlgQryJoin,RBTogleHdl));
    aRB_Full.SetToggleHdl(LINK(this,DlgQryJoin,RBTogleHdl));

    if (static_cast<OQueryTableView*>(pParent)->getDesignView()->getController()->isReadOnly())
    {
        aRB_Inner.Disable();
        aRB_Left.Disable();
        aRB_Right.Disable();
        aRB_Full.Disable();
    }

    FreeResource();
}

//------------------------------------------------------------------------
DlgQryJoin::~DlgQryJoin()
{
    DBG_DTOR(DlgQryJoin,NULL);
}
IMPL_LINK( DlgQryJoin, RBTogleHdl, RadioButton*, pButton )
{
    DBG_CHKTHIS(DlgQryJoin,NULL);
    aML_HelpText.SetText(String());
    String aStr;
    if(pButton == &aRB_Inner)
    {
        aStr = String(ModuleRes(STR_QUERY_INNER_JOIN));
    }
    else if(pButton == &aRB_Left)
    {
        aStr = String(ModuleRes(STR_QUERY_LEFTRIGHT_JOIN));
        aStr.SearchAndReplace(String::CreateFromAscii("%1"),pConnData->GetSourceWinName());
        aStr.SearchAndReplace(String::CreateFromAscii("%2"),pConnData->GetDestWinName());
    }
    else if(pButton == &aRB_Right)
    {
        aStr = String(ModuleRes(STR_QUERY_LEFTRIGHT_JOIN));
        aStr.SearchAndReplace(String::CreateFromAscii("%1"),pConnData->GetDestWinName());
        aStr.SearchAndReplace(String::CreateFromAscii("%2"),pConnData->GetSourceWinName());
    }
    else if(pButton == &aRB_Full)
    {
        aStr = String(ModuleRes(STR_QUERY_FULL_JOIN));
        aStr.SearchAndReplace(String::CreateFromAscii("%1"),pConnData->GetDestWinName());
        aStr.SearchAndReplace(String::CreateFromAscii("%2"),pConnData->GetSourceWinName());
    }
    aML_HelpText.SetText(aStr);
    return 1;
}

IMPL_LINK( DlgQryJoin, OKClickHdl, Button*, pButton )
{
    DBG_CHKTHIS(DlgQryJoin,NULL);
    if( aRB_Inner.IsChecked() )
        eJoinType = INNER_JOIN;
    else if( aRB_Left.IsChecked() )
        eJoinType = LEFT_JOIN;
    else if( aRB_Right.IsChecked() )
        eJoinType = RIGHT_JOIN;
    else if( aRB_Full.IsChecked() )
        eJoinType = FULL_JOIN;
    EndDialog();
    return 1;
}


