/*************************************************************************
 *
 *  $RCSfile: dlgsave.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:46:25 $
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
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
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
                        const Reference< XConnection>& _xConnection,
                        const String& rDefault,
                        sal_Int32 _nFlags)
             :ModalDialog( pParent, ModuleRes(DLG_SAVE_AS))
             ,m_aDescription(this, ResId (FT_DESCRIPTION))
             ,m_aCatalogLbl(this, ResId (FT_CATALOG))
             ,m_aCatalog(this, ResId (ET_CATALOG),_rxMetaData.is() ? _rxMetaData->getExtraNameCharacters() : ::rtl::OUString())
             ,m_aSchemaLbl(this, ResId (FT_SCHEMA))
             ,m_aSchema(this, ResId (ET_SCHEMA),_rxMetaData.is() ? _rxMetaData->getExtraNameCharacters() : ::rtl::OUString())
             ,m_aLabel(this, ResId (FT_TITLE))
             ,m_aTitle(this, ResId (ET_TITLE), _rxMetaData.is() ? _rxMetaData->getExtraNameCharacters() : ::rtl::OUString())
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
             ,m_nFlags(_nFlags)
{
    switch (_rType)
    {
        case CommandType::QUERY:
            implInitOnlyTitle(m_aQryLabel);
            break;
        case CommandType::TABLE:
            {
                m_aLabel.SetText(m_sTblLabel);
                Point aPos(m_aPB_OK.GetPosPixel());
                if(_rxMetaData.is() && !_rxMetaData->supportsCatalogsInTableDefinitions())
                {
                    m_aCatalogLbl.Hide();
                    m_aCatalog.Hide();

                    aPos = m_aLabel.GetPosPixel();

                    m_aLabel.SetPosPixel(m_aSchemaLbl.GetPosPixel());
                    m_aTitle.SetPosPixel(m_aSchema.GetPosPixel());

                    m_aSchemaLbl.SetPosPixel(m_aCatalogLbl.GetPosPixel());
                    m_aSchema.SetPosPixel(m_aCatalog.GetPosPixel());
                }
                else if ( _rxMetaData.is() )
                {
                    // now fill the catalogs
                    try
                    {
                        Reference<XResultSet> xRes = m_xMetaData->getCatalogs();
                        Reference<XRow> xRow(xRes,UNO_QUERY);
                        ::rtl::OUString sCatalog;
                        while(xRes.is() && xRes->next())
                        {
                            sCatalog = xRow->getString(1);
                            if(!xRow->wasNull())
                                m_aCatalog.InsertEntry(sCatalog);
                        }
                        if ( _xConnection.is() )
                        {
                            String sCatalog = _xConnection->getCatalog();
                            USHORT nPos = m_aCatalog.GetEntryPos(sCatalog);
                            if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
                                m_aCatalog.SelectEntryPos(nPos);
                        }
                        else
                            m_aCatalog.SetText(String());
                    }
                    catch(const SQLException&)
                    {
                    }
                }

                if(_rxMetaData.is() && !_rxMetaData->supportsSchemasInTableDefinitions())
                {
                    m_aSchemaLbl.Hide();
                    m_aSchema.Hide();

                    aPos = m_aLabel.GetPosPixel();

                    m_aLabel.SetPosPixel(m_aSchemaLbl.GetPosPixel());
                    m_aTitle.SetPosPixel(m_aSchema.GetPosPixel());
                }
                else if ( _rxMetaData.is() )
                {
                    // now fill the schemata
                    try
                    {
                        Reference<XResultSet> xRes = m_xMetaData->getSchemas();
                        Reference<XRow> xRow(xRes,UNO_QUERY);
                        ::rtl::OUString sSchema;
                        while(xRes.is() && xRes->next())
                        {
                            sSchema = xRow->getString(1);
                            if(!xRow->wasNull() && m_aSchema.GetEntryPos(XubString(sSchema)) == COMBOBOX_ENTRY_NOTFOUND)
                                m_aSchema.InsertEntry(sSchema);
                        }
                        String sTemp = m_xMetaData->getUserName();
                        USHORT nPos = m_aSchema.GetEntryPos(sTemp);
                        if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
                            m_aSchema.SelectEntryPos(nPos);
                    }
                    catch(const SQLException&)
                    {
                    }
                }

                OSL_ENSURE(m_xMetaData.is(),"The metadata can not be null!");
                if(m_aName.Search('.') != STRING_NOTFOUND)
                {
                    ::rtl::OUString sCatalog,sSchema,sTable;
                    ::dbtools::qualifiedNameComponents(_rxMetaData,
                                                        m_aName,
                                                        sCatalog,
                                                        sSchema,
                                                        sTable,
                                                        ::dbtools::eInDataManipulation);

                    USHORT nPos = m_aCatalog.GetEntryPos(String(sCatalog));
                    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
                        m_aCatalog.SelectEntryPos(nPos);

                    if ( sSchema.getLength() )
                    {
                        nPos = m_aSchema.GetEntryPos(String(sSchema));
                        if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
                            m_aSchema.SelectEntryPos(nPos);
                    }
                    m_aTitle.SetText(sTable);
                }
                else
                    m_aTitle.SetText(m_aName);

                m_aPB_OK.SetPosPixel(Point(m_aPB_OK.GetPosPixel().X(),aPos.Y()));
                m_aPB_CANCEL.SetPosPixel(Point(m_aPB_CANCEL.GetPosPixel().X(),aPos.Y()));
                m_aPB_HELP.SetPosPixel(Point(m_aPB_HELP.GetPosPixel().X(),aPos.Y()));


                USHORT nLength =  _rxMetaData.is() ? static_cast<USHORT>(_rxMetaData->getMaxTableNameLength()) : 0;
                nLength = nLength ? nLength : EDIT_NOLIMIT;

                m_aTitle.SetMaxTextLen(nLength);
                m_aSchema.SetMaxTextLen(nLength);
                m_aCatalog.SetMaxTextLen(nLength);

                sal_Bool bCheck = _xConnection.is() && isSQL92CheckEnabled(_xConnection);
                m_aTitle.setCheck(bCheck); // enable non valid sql chars as well
                m_aSchema.setCheck(bCheck); // enable non valid sql chars as well
                m_aCatalog.setCheck(bCheck); // enable non valid sql chars as well


                Size aSize = GetSizePixel();
                aSize.Height() =
                    aPos.Y() + m_aPB_OK.GetSizePixel().Height() + m_aTitle.GetSizePixel().Height() / 2;
                SetSizePixel(aSize);
            }
            break;
        default:
            OSL_ENSURE(0,"Type not supported yet!");
    }

    implInit();
}
// -----------------------------------------------------------------------------
OSaveAsDlg::OSaveAsDlg( Window * pParent,
                        const Reference<XNameAccess>& _rxNames,
                        const String& rDefault,
                        const String& _sLabel,
                        sal_Int32 _nFlags)
             :ModalDialog( pParent, ModuleRes(DLG_SAVE_AS))
             ,m_aDescription(this, ResId (FT_DESCRIPTION))
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
             ,m_nFlags(_nFlags)
             ,m_nType(CommandType::COMMAND)
{
    implInitOnlyTitle(_sLabel);
    implInit();
}
// -----------------------------------------------------------------------------
OSaveAsDlg::OSaveAsDlg( Window * pParent,
                        const Reference<XHierarchicalNameAccess>& _rxNames,
                        const String& rDefault,
                        const String& _sLabel,
                        const String& _sParentURL,
                        sal_Int32 _nFlags)
             :ModalDialog( pParent, ModuleRes(DLG_SAVE_AS))
             ,m_aDescription(this, ResId (FT_DESCRIPTION))
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
             ,m_xHierarchyNames(_rxNames)
             ,m_nFlags(_nFlags)
             ,m_nType(CommandType::COMMAND)
             ,m_sParentURL(_sParentURL)
{
    implInitOnlyTitle(_sLabel);
    implInit();
}
// -----------------------------------------------------------------------------

IMPL_LINK(OSaveAsDlg, ButtonClickHdl, Button *, pButton)
{
    if (pButton == &m_aPB_OK)
    {
        m_aName = m_aTitle.GetText();
        sal_Bool bError = sal_False;
        if ( m_xHierarchyNames.is() )
        {
            String sTest;
            if ( m_sParentURL.Len() )
                sTest = m_sParentURL + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
            sTest += m_aName;
            bError = m_xHierarchyNames->hasByHierarchicalName(sTest);
        }
        else
            bError = m_xNames->hasByName(m_aName);
        if ( m_nType == CommandType::TABLE )
        {
            OSL_ENSURE(m_xNames.is(),"Who try this one with a hierarchical access!");
            OSL_ENSURE(m_xMetaData.is(),"The metadata can not be null!");
            ::rtl::OUString sComposedName;
            ::dbtools::composeTableName(m_xMetaData,getCatalog(),getSchema(),m_aName,sComposedName,sal_False,::dbtools::eInDataManipulation);
            bError = m_xNames->hasByName(sComposedName);
        }
        if(bError)
        {
            m_aTitle.GrabFocus();

            sal_Bool bOverwrite = ( (m_nFlags & SAD_OVERWRITE) == SAD_OVERWRITE );
            String aText( bOverwrite ? m_aExistsOverwrite : m_aExists);
            aText.SearchAndReplace(String::CreateFromAscii("$#$"),m_aName);
            OSQLMessageBox aDlg(this, String(ModuleRes(STR_OBJECT_ALREADY_EXSISTS)), aText, bOverwrite ? WB_YES_NO : WB_OK, OSQLMessageBox::Query);

            if ( aDlg.Execute() == RET_YES && bOverwrite )
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
        m_aPB_OK.Enable(0 != m_aTitle.GetText().Len());
    return 0;
}
// -----------------------------------------------------------------------------
void OSaveAsDlg::implInitOnlyTitle(const String& _rLabel)
{
    m_aLabel.SetText(_rLabel);
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

    sal_Int32 nNewHeight =
        aPos.Y() + m_aPB_OK.GetSizePixel().Height() + m_aTitle.GetSizePixel().Height() / 2;

    SetSizePixel(Size(GetSizePixel().Width(), nNewHeight));

    m_aTitle.SetText(m_aName);
    m_aTitle.setCheck(sal_False); // enable non valid sql chars as well
}
// -----------------------------------------------------------------------------
void OSaveAsDlg::implInit()
{
    if ( 0 == ( m_nFlags & SAD_ADDITIONAL_DESCRIPTION ) )
    {
        // hide the description window
        m_aDescription.Hide();

        // the number of pixels we have to move the other controls
        sal_Int32 nMoveUp = m_aCatalog.GetPosPixel().Y() - m_aDescription.GetPosPixel().Y();

        // loop to all controls and move them ...
        for (   Window* pChildControl = GetWindow( WINDOW_FIRSTCHILD );
                pChildControl;
                pChildControl= pChildControl->GetWindow( WINDOW_NEXT )
            )
        {
            if ( &m_aDescription != pChildControl )
            {
                Point aPos = pChildControl->GetPosPixel();
                aPos.Y() -= nMoveUp;
                pChildControl->SetPosPixel(aPos);
            }
        }

        // change our own size accordingly
        Size aSize = GetSizePixel();
        aSize.Height() -= nMoveUp;
        SetSizePixel(aSize);
    }

    if ( SAD_TITLE_PASTE_AS == ( m_nFlags & SAD_TITLE_PASTE_AS ) )
        SetText( String( ModuleRes( STR_TITLE_PASTE_AS ) ) );
    else if ( SAD_TITLE_RENAME == ( m_nFlags & SAD_TITLE_RENAME ) )
    {
        SetText( String( ModuleRes( STR_TITLE_RENAME ) ) );
        m_aTitle.SetHelpId(HID_DLG_RENAME);
    }

    m_aPB_OK.SetClickHdl(LINK(this,OSaveAsDlg,ButtonClickHdl));
    m_aTitle.SetModifyHdl(LINK(this,OSaveAsDlg,EditModifyHdl));
    m_aTitle.GrabFocus();
    FreeResource();
}
// -----------------------------------------------------------------------------

