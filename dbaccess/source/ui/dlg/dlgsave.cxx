/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgsave.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:26:32 $
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
#ifndef DBACCESS_SOURCE_UI_INC_OBJECTNAMECHECK_HXX
#include "objectnamecheck.hxx"
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif


using namespace dbaui;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;

//==================================================================
namespace
{
    typedef Reference< XResultSet > (SAL_CALL XDatabaseMetaData::*FGetMetaStrings)();

    void lcl_fillComboList( ComboBox& _rList, const Reference< XConnection >& _rxConnection,
        FGetMetaStrings _GetAll, const ::rtl::OUString& _rCurrent )
    {
        try
        {
            Reference< XDatabaseMetaData > xMetaData( _rxConnection->getMetaData(), UNO_QUERY_THROW );

            Reference< XResultSet > xRes = (xMetaData.get()->*_GetAll)();
            Reference< XRow > xRow( xRes, UNO_QUERY_THROW );
            ::rtl::OUString sValue;
            while ( xRes->next() )
            {
                sValue = xRow->getString( 1 );
                if ( !xRow->wasNull() )
                    _rList.InsertEntry( sValue );
            }

            USHORT nPos = _rList.GetEntryPos( String( _rCurrent ) );
            if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
                _rList.SelectEntryPos( nPos );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

//==================================================================
OSaveAsDlg::OSaveAsDlg( Window * pParent,
                        const sal_Int32& _rType,
                        const Reference< XMultiServiceFactory >& _rxORB,
                        const Reference< XConnection>& _xConnection,
                        const String& rDefault,
                        const IObjectNameCheck& _rObjectNameCheck,
                        sal_Int32 _nFlags)
    :ModalDialog( pParent, ModuleRes(DLG_SAVE_AS))
    ,m_aDescription(this, ResId (FT_DESCRIPTION))
    ,m_aCatalogLbl(this, ResId (FT_CATALOG))
    ,m_aCatalog(this, ResId (ET_CATALOG), ::rtl::OUString())
    ,m_aSchemaLbl(this, ResId (FT_SCHEMA))
    ,m_aSchema(this, ResId (ET_SCHEMA), ::rtl::OUString())
    ,m_aLabel(this, ResId (FT_TITLE))
    ,m_aTitle(this, ResId (ET_TITLE), ::rtl::OUString())
    ,m_aPB_OK(this, ResId( PB_OK ) )
    ,m_aPB_CANCEL(this, ResId( PB_CANCEL ))
    ,m_aPB_HELP(this, ResId( PB_HELP))
    ,m_aQryLabel(ResId(STR_QRY_LABEL))
    ,m_sTblLabel(ResId(STR_TBL_LABEL))
    ,m_aName(rDefault)
    ,m_rObjectNameCheck( _rObjectNameCheck )
    ,m_xORB( _rxORB )
    ,m_nType(_rType)
    ,m_nFlags(_nFlags)
{
    if ( _xConnection.is() )
        m_xMetaData = _xConnection->getMetaData();

    if ( m_xMetaData.is() )
    {
        ::rtl::OUString sExtraNameChars( m_xMetaData->getExtraNameCharacters() );
        m_aCatalog.setAllowedChars( sExtraNameChars );
        m_aSchema.setAllowedChars( sExtraNameChars );
        m_aTitle.setAllowedChars( sExtraNameChars );
    }

    switch (_rType)
    {
        case CommandType::QUERY:
            implInitOnlyTitle(m_aQryLabel);
            break;

        case CommandType::TABLE:
            OSL_ENSURE( m_xMetaData.is(), "OSaveAsDlg::OSaveAsDlg: no meta data for entering table names: this will crash!" );
            {
                m_aLabel.SetText(m_sTblLabel);
                Point aPos(m_aPB_OK.GetPosPixel());
                if ( !m_xMetaData->supportsCatalogsInTableDefinitions() )
                {
                    m_aCatalogLbl.Hide();
                    m_aCatalog.Hide();

                    aPos = m_aLabel.GetPosPixel();

                    m_aLabel.SetPosPixel(m_aSchemaLbl.GetPosPixel());
                    m_aTitle.SetPosPixel(m_aSchema.GetPosPixel());

                    m_aSchemaLbl.SetPosPixel(m_aCatalogLbl.GetPosPixel());
                    m_aSchema.SetPosPixel(m_aCatalog.GetPosPixel());
                }
                else
                {
                    // now fill the catalogs
                    lcl_fillComboList( m_aCatalog, _xConnection,
                        &XDatabaseMetaData::getCatalogs, _xConnection->getCatalog() );
                }

                if ( !m_xMetaData->supportsSchemasInTableDefinitions())
                {
                    m_aSchemaLbl.Hide();
                    m_aSchema.Hide();

                    aPos = m_aLabel.GetPosPixel();

                    m_aLabel.SetPosPixel(m_aSchemaLbl.GetPosPixel());
                    m_aTitle.SetPosPixel(m_aSchema.GetPosPixel());
                }
                else
                {
                    lcl_fillComboList( m_aSchema, _xConnection,
                        &XDatabaseMetaData::getSchemas, m_xMetaData->getUserName() );
                }

                ::rtl::OUString sCatalog,sSchema,sTable;
                ::dbtools::qualifiedNameComponents( m_xMetaData,
                                                    m_aName,
                                                    sCatalog,
                                                    sSchema,
                                                    sTable,
                                                    ::dbtools::eInDataManipulation);

                USHORT nPos = m_aCatalog.GetEntryPos( String( sCatalog ) );
                if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
                    m_aCatalog.SelectEntryPos( nPos );

                if ( sSchema.getLength() )
                {
                    nPos = m_aSchema.GetEntryPos( String( sSchema ) );
                    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
                        m_aSchema.SelectEntryPos( nPos );
                }
                m_aTitle.SetText(sTable);

                m_aPB_OK.SetPosPixel(Point(m_aPB_OK.GetPosPixel().X(),aPos.Y()));
                m_aPB_CANCEL.SetPosPixel(Point(m_aPB_CANCEL.GetPosPixel().X(),aPos.Y()));
                m_aPB_HELP.SetPosPixel(Point(m_aPB_HELP.GetPosPixel().X(),aPos.Y()));

                USHORT nLength =  m_xMetaData.is() ? static_cast<USHORT>(m_xMetaData->getMaxTableNameLength()) : 0;
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
            OSL_ENSURE( false, "OSaveAsDlg::OSaveAsDlg: Type not supported yet!" );
    }

    implInit();
}
// -----------------------------------------------------------------------------
OSaveAsDlg::OSaveAsDlg( Window * pParent,
                        const Reference< XMultiServiceFactory >& _rxORB,
                        const String& rDefault,
                        const String& _sLabel,
                        const IObjectNameCheck& _rObjectNameCheck,
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
             ,m_aName(rDefault)
             ,m_rObjectNameCheck( _rObjectNameCheck )
             ,m_xORB( _rxORB )
             ,m_nType(CommandType::COMMAND)
             ,m_nFlags(_nFlags)
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
        ::rtl::OUString sNameToCheck( m_aName );

        if ( m_nType == CommandType::TABLE )
        {
            m_aName = ::dbtools::composeTableName(
                m_xMetaData,
                getCatalog(),
                getSchema(),
                sNameToCheck,
                sal_False,  // no quoting
                ::dbtools::eInDataManipulation
            );
        }

        SQLExceptionInfo aNameError;
        if ( m_rObjectNameCheck.isNameValid( sNameToCheck, aNameError ) )
            EndDialog( RET_OK );

        showError( aNameError, this, m_xORB );
        m_aTitle.GrabFocus();
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

