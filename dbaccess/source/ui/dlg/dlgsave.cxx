/*************************************************************************
 *
 *  $RCSfile: dlgsave.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-01-26 16:12:56 $
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

#ifndef DBAUI_DLGSAVE_HXX
#include "dlgsave.hxx"
#endif
#ifndef DBAUI_DLGSAVE_HRC
#include "dlgsave.hrc"
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef DBAUI_DLGERR_HXX
#include "dlgerr.hxx"
#endif


using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;

//==================================================================
OSaveAsDlg::OSaveAsDlg( Window * pParent,
                        const sal_Int32& _rType,
                        const Reference<XNameAccess>& _rxNames,
                        const String& rDefault)
             :ModalDialog( pParent, ModuleRes(DLG_SAVE_AS))
             ,m_aLabel(this, ResId (FT_TITLE))
             ,m_aTitle(this, ResId (ET_TITLE))
             ,m_aPB_OK(this, ResId( PB_OK ) )
             ,m_aPB_CANCEL(this, ResId( PB_CANCEL ))
             ,m_aPB_HELP(this, ResId( PB_HELP))
             ,m_aQryLabel(ResId(STR_QRY_LABEL))
             ,m_aExists(ResId(STR_OBJECT_EXISTS_ALREADY))
             ,m_aName(rDefault)
             ,m_xNames(_rxNames)
{
    switch (_rType)
    {
        case CommandType::QUERY:
            m_aLabel.SetText(m_aQryLabel);
            break;
        default:
            OSL_ENSURE(0,"Type not supported yet!");
    }

    m_aTitle.SetText(m_aName);
    m_aPB_OK.SetClickHdl(LINK(this,OSaveAsDlg,ButtonClickHdl));
    m_aTitle.SetModifyHdl(LINK(this,OSaveAsDlg,EditModifyHdl));
    FreeResource();
}

IMPL_LINK(OSaveAsDlg, ButtonClickHdl, Button *, pButton)
{
    if (pButton == &m_aPB_OK)
    {
        m_aName = m_aTitle.GetText();

        if(m_xNames->hasByName(m_aName))
        {
            m_aTitle.GrabFocus();
            String aText(m_aExists);
            aText.SearchAndReplace(String::CreateFromAscii("'$Name: not supported by cvs2svn $'"),m_aName);
            OMsgBox aDlg(this,String(ModuleRes(STR_OBJECT_ALREADY_EXSISTS)),aText,WB_OK,OMsgBox::Query);
        }
        else
            EndDialog(RET_OK);
    }
    return 0;
}

IMPL_LINK(OSaveAsDlg, EditModifyHdl, Edit *, pEdit )
{
    if (pEdit == &m_aTitle)
        m_aPB_OK.Enable(m_aTitle.GetText().Len());
    return 0;
}

