/*************************************************************************
 *
 *  $RCSfile: dlgsave.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-14 10:35:10 $
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
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif


using namespace dbaui;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;

//==================================================================
OSaveAsDlg::OSaveAsDlg( Window * pParent,
                        const sal_Int32& _rType,
                        const Reference<XNameAccess>& _rxNames,
                        const Reference< XDatabaseMetaData>& _rxMetaData,
                        const String& rDefault,
                        sal_Bool _bOverWrite)
             :ModalDialog( pParent, ModuleRes(DLG_SAVE_AS))
             ,m_aCatalogLbl(this, ResId (FT_CATALOG))
             ,m_aCatalog(this, ResId (ET_CATALOG))
             ,m_aSchemaLbl(this, ResId (FT_SCHEMA))
             ,m_aSchema(this, ResId (ET_SCHEMA))
             ,m_aLabel(this, ResId (FT_TITLE))
             ,m_aTitle(this, ResId (ET_TITLE))
             ,m_aPB_OK(this, ResId( PB_OK ) )
             ,m_aPB_CANCEL(this, ResId( PB_CANCEL ))
             ,m_aPB_HELP(this, ResId( PB_HELP))
             ,m_aQryLabel(ResId(STR_QRY_LABEL))
             ,m_sTblLabel(ResId(STR_TBL_LABEL))
             ,m_aExists(ResId(STR_OBJECT_EXISTS_ALREADY))
             ,m_aExistsOverwrite(ResId(STR_OBJECT_EXISTS_ALREADY_OVERWRITE))
             ,m_aName(rDefault)
             ,m_xNames(_rxNames)
             ,m_xMetaData(_rxMetaData)
             ,m_nType(_rType)
             ,m_bOverWrite(_bOverWrite)
{
    switch (_rType)
    {
        case CommandType::QUERY:
            {
                m_aLabel.SetText(m_aQryLabel);
                m_aCatalogLbl.Hide();
                m_aCatalog.Hide();
                m_aSchemaLbl.Hide();
                m_aSchema.Hide();

                Point aPos(m_aSchemaLbl.GetPosPixel());
                m_aLabel.SetPosPixel(m_aCatalogLbl.GetPosPixel());
                m_aTitle.SetPosPixel(m_aCatalog.GetPosPixel());

                m_aPB_OK.SetPosPixel(Point(m_aPB_OK.GetPosPixel().X(),aPos.Y()));
                m_aPB_CANCEL.SetPosPixel(Point(m_aPB_CANCEL.GetPosPixel().X(),aPos.Y()));
                m_aPB_HELP.SetPosPixel(Point(m_aPB_HELP.GetPosPixel().X(),aPos.Y()));

                SetSizePixel(Size(GetSizePixel().Width(),aPos.Y()+m_aPB_OK.GetSizePixel().Height()+m_aTitle.GetSizePixel().Height()*0.5));
                m_aTitle.SetText(m_aName);
            }
            break;
        case CommandType::TABLE:
            {
                OSL_ENSURE(m_xMetaData.is(),"The metadata can not be null!");
                if(m_aName.Search('.') != STRING_NOTFOUND)
                {
                    ::rtl::OUString sCatalog,sSchema,sTable;
                    ::dbtools::qualifiedNameComponents(_rxMetaData,
                                                        m_aName,
                                                        sCatalog,
                                                        sSchema,
                                                        sTable);

                    m_aCatalog.SetText(sCatalog);
                    m_aSchema.SetText(sSchema);
                    m_aTitle.SetText(sTable);
                }
                else
                    m_aTitle.SetText(m_aName);


                m_aLabel.SetText(m_sTblLabel);
                Point aPos(m_aPB_OK.GetPosPixel());
                if(!_rxMetaData->supportsCatalogsInTableDefinitions())
                {
                    m_aCatalogLbl.Hide();
                    m_aCatalog.Hide();

                    aPos = m_aLabel.GetPosPixel();

                    m_aLabel.SetPosPixel(m_aSchemaLbl.GetPosPixel());
                    m_aTitle.SetPosPixel(m_aSchema.GetPosPixel());

                    m_aSchemaLbl.SetPosPixel(m_aCatalogLbl.GetPosPixel());
                    m_aSchema.SetPosPixel(m_aCatalog.GetPosPixel());
                }

                if(!_rxMetaData->supportsSchemasInTableDefinitions())
                {
                    m_aSchemaLbl.Hide();
                    m_aSchema.Hide();

                    aPos = m_aLabel.GetPosPixel();

                    m_aLabel.SetPosPixel(m_aSchemaLbl.GetPosPixel());
                    m_aTitle.SetPosPixel(m_aSchema.GetPosPixel());
                }
                m_aPB_OK.SetPosPixel(Point(m_aPB_OK.GetPosPixel().X(),aPos.Y()));
                m_aPB_CANCEL.SetPosPixel(Point(m_aPB_CANCEL.GetPosPixel().X(),aPos.Y()));
                m_aPB_HELP.SetPosPixel(Point(m_aPB_HELP.GetPosPixel().X(),aPos.Y()));

                SetSizePixel(Size(GetSizePixel().Width(),aPos.Y()+m_aPB_OK.GetSizePixel().Height()+m_aTitle.GetSizePixel().Height()*0.5));
            }
            break;
        default:
            OSL_ENSURE(0,"Type not supported yet!");
    }


    m_aPB_OK.SetClickHdl(LINK(this,OSaveAsDlg,ButtonClickHdl));
    m_aTitle.SetModifyHdl(LINK(this,OSaveAsDlg,EditModifyHdl));
    m_aTitle.GrabFocus();
    FreeResource();
}
// -----------------------------------------------------------------------------

IMPL_LINK(OSaveAsDlg, ButtonClickHdl, Button *, pButton)
{
    if (pButton == &m_aPB_OK)
    {
        m_aName = m_aTitle.GetText();
        sal_Bool bError = m_xNames->hasByName(m_aName);
        if(m_nType == CommandType::TABLE)
        {
            OSL_ENSURE(m_xMetaData.is(),"The metadata can not be null!");
            ::rtl::OUString sComposedName;
            ::dbtools::composeTableName(m_xMetaData,getCatalog(),getSchema(),m_aName,sComposedName,sal_False);
            bError = m_xNames->hasByName(sComposedName);
        }
        if(bError)
        {
            m_aTitle.GrabFocus();
            String aText(m_bOverWrite ? m_aExistsOverwrite : m_aExists);
            aText.SearchAndReplace(String::CreateFromAscii("$Name: not supported by cvs2svn $"),m_aName);
            OSQLMessageBox aDlg(this, String(ModuleRes(STR_OBJECT_ALREADY_EXSISTS)), aText, m_bOverWrite ? WB_YES_NO : WB_OK, OSQLMessageBox::Query);
            if(aDlg.Execute() == RET_YES)
                EndDialog(RET_OK);
        }
        else
            EndDialog(RET_OK);
    }
    return 0;
}
// -----------------------------------------------------------------------------

IMPL_LINK(OSaveAsDlg, EditModifyHdl, Edit *, pEdit )
{
    if (pEdit == &m_aTitle)
        m_aPB_OK.Enable(m_aTitle.GetText().Len());
    return 0;
}
// -----------------------------------------------------------------------------


