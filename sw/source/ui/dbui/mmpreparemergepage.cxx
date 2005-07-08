/*************************************************************************
 *
 *  $RCSfile: mmpreparemergepage.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-08 10:30:20 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif

#pragma hdrstop
#ifndef _MAILMERGEPREPAREMERGEPAGE_HXX
#include <mmpreparemergepage.hxx>
#endif
#ifndef _MAILMERGEWIZARD_HXX
#include <mailmergewizard.hxx>
#endif
#ifndef _MMCONFIGITEM_HXX
#include <mmconfigitem.hxx>
#endif
#ifndef _DBUI_HRC
#include <dbui.hrc>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _SW_ABSTDLG_HXX
#include <swabstdlg.hxx>
#endif


#include <mmpreparemergepage.hrc>
#include <dbui.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::rtl;
#define C2U(cChar) OUString::createFromAscii(cChar)

/*-- 02.04.2004 16:42:49---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergePrepareMergePage::SwMailMergePrepareMergePage( SwMailMergeWizard* _pParent) :
    svt::OWizardPage( _pParent, SW_RES(DLG_MM_PREPAREMERGE_PAGE)),
#pragma warning (disable : 4355)
    m_aHeaderFI(this,  ResId(     FI_HEADER ) ),
    m_aPreviewFI(this, ResId(     FI_PREVIEW ) ),
    m_aRecipientFT(this, ResId(   FT_RECIPIENT ) ),
    m_aFirstPB(this, ResId(       PB_FIRST   ) ),
    m_aPrevPB(this, ResId(        PB_PREV    ) ),
    m_aRecordED(this, ResId(      ED_RECORD  ) ),
    m_aNextPB(this, ResId(        PB_NEXT    ) ),
    m_aLastPB(this, ResId(        PB_LAST    ) ),
    m_ExcludeCB(this, ResId(      CB_EXCLUDE    ) ),
    m_aNoteHeaderFL(this, ResId(  FL_NOTEHEADER ) ),
    m_aEditFI(this, ResId(        FI_EDIT       ) ),
    m_aEditPB(this, ResId(        PB_EDIT       ) ),
#pragma warning (disable : 4355)
    m_pWizard(_pParent)
{
    FreeResource();
    m_aEditPB.SetClickHdl( LINK( this, SwMailMergePrepareMergePage, EditDocumentHdl_Impl));
    Link aMoveLink(LINK( this, SwMailMergePrepareMergePage, MoveHdl_Impl));
    m_aFirstPB.SetClickHdl( aMoveLink );
    m_aPrevPB.SetClickHdl( aMoveLink );
    m_aNextPB.SetClickHdl( aMoveLink );
    m_aLastPB.SetClickHdl( aMoveLink );
    m_aRecordED.SetActionHdl( aMoveLink );
    m_ExcludeCB.SetClickHdl(LINK(this, SwMailMergePrepareMergePage, ExcludeHdl_Impl));
    aMoveLink.Call(&m_aRecordED);
}
/*-- 02.04.2004 16:42:49---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergePrepareMergePage::~SwMailMergePrepareMergePage()
{
}
/*-- 13.05.2004 15:36:48---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwMailMergePrepareMergePage, EditDocumentHdl_Impl, PushButton*, pButton)
{
    m_pWizard->SetRestartPage(MM_PREPAREMERGEPAGE);
    m_pWizard->EndDialog(RET_EDIT_DOC);
    return 0;
}
/*-- 27.05.2004 14:16:37---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwMailMergePrepareMergePage, MoveHdl_Impl, void*, pCtrl)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    sal_Int32 nPos = rConfigItem.GetResultSetPosition();
    if(pCtrl == &m_aFirstPB)
    {
        rConfigItem.MoveResultSet(1);
    }
    else if(pCtrl == &m_aPrevPB)
    {
        rConfigItem.MoveResultSet(nPos - 1);
    }
    else if(pCtrl == &m_aRecordED)
    {
        rConfigItem.MoveResultSet(m_aRecordED.GetValue());
    }
    else if(pCtrl == &m_aNextPB)
        rConfigItem.MoveResultSet(nPos + 1);
    else if(pCtrl == &m_aLastPB)
        rConfigItem.MoveResultSet(-1);

    nPos = rConfigItem.GetResultSetPosition();
    m_aRecordED.SetValue(nPos);
    bool bIsFirst;
    bool bIsLast;
    bool bValid = rConfigItem.IsResultSetFirstLast(bIsFirst, bIsLast);
    m_aFirstPB.Enable(bValid && !bIsFirst);
    m_aPrevPB.Enable(bValid && !bIsFirst);
    m_aNextPB.Enable(bValid && !bIsLast);
    m_aLastPB.Enable(bValid && !bIsLast);
    m_ExcludeCB.Check(rConfigItem.IsRecordExcluded( rConfigItem.GetResultSetPosition() ));
    //now the record has to be merged into the source document
    SwView* pView = m_pWizard->GetSwView();

/*
            { CONST_CHAR("ActiveConnection"),   daConnection,       &::getCppuType( static_cast< Reference< XConnection >* >(NULL) ),   PropertyAttribute::TRANSIENT, 0 },
            { CONST_CHAR("BookmarkSelection"),  daBookmarkSelection,&::getBooleanCppuType( ),                                           PropertyAttribute::TRANSIENT, 0 },
            { CONST_CHAR("Column"),             daColumnObject,     &::getCppuType( static_cast< Reference< XPropertySet >* >(NULL) ),  PropertyAttribute::TRANSIENT, 0 },
            { CONST_CHAR("ColumnName"),         daColumnName,       &::getCppuType( static_cast< ::rtl::OUString* >(NULL) ),            PropertyAttribute::TRANSIENT, 0 },
            { CONST_CHAR("Command"),            daCommand,          &::getCppuType( static_cast< ::rtl::OUString* >(NULL) ),            PropertyAttribute::TRANSIENT, 0 },
            { CONST_CHAR("CommandType"),        daCommandType,      &::getCppuType( static_cast< sal_Int32* >(NULL) ),                  PropertyAttribute::TRANSIENT, 0 },
            { CONST_CHAR("Cursor"),             daCursor,           &::getCppuType( static_cast< Reference< XResultSet>* >(NULL) ),     PropertyAttribute::TRANSIENT, 0 },
            { CONST_CHAR("DataSourceName"),     daDataSource,       &::getCppuType( static_cast< ::rtl::OUString* >(NULL) ),            PropertyAttribute::TRANSIENT, 0 },
            { CONST_CHAR("EscapeProcessing"),   daEscapeProcessing, &::getBooleanCppuType( ),                                           PropertyAttribute::TRANSIENT, 0 },
            { CONST_CHAR("Filter"),             daFilter,           &::getCppuType( static_cast< ::rtl::OUString* >(NULL) ),            PropertyAttribute::TRANSIENT, 0 },
            { CONST_CHAR("Selection"),          daSelection,        &::getCppuType( static_cast< Sequence< Any >* >(NULL) ),            PropertyAttribute::TRANSIENT, 0 },

*/
    const SwDBData& rDBData = rConfigItem.GetCurrentDBData();

    Sequence< PropertyValue > aArgs(7);
    Sequence<Any> aSelection(1);
    aSelection[0] <<= rConfigItem.GetResultSetPosition();
    aArgs[0].Name = C2U("Selection");
    aArgs[0].Value <<= aSelection;
    aArgs[1].Name = C2U("DataSourceName");
    aArgs[1].Value <<= rDBData.sDataSource;
    aArgs[2].Name = C2U("Command");
    aArgs[2].Value <<= rDBData.sCommand;
    aArgs[3].Name = C2U("CommandType");
    aArgs[3].Value <<= rDBData.nCommandType;
    aArgs[4].Name = C2U("ActiveConnection");
    aArgs[4].Value <<=  rConfigItem.GetConnection().getTyped();
    aArgs[5].Name = C2U("Filter");
    aArgs[5].Value <<= rConfigItem.GetFilter();
    aArgs[6].Name = C2U("Cursor");
    aArgs[6].Value <<= rConfigItem.GetResultSet();

    ::svx::ODataAccessDescriptor aDescriptor(aArgs);
    SwWrtShell& rSh = m_pWizard->GetSwView()->GetWrtShell();
    SwMergeDescriptor aMergeDesc( DBMGR_MERGE, rSh, aDescriptor );
    rSh.GetNewDBMgr()->MergeNew(aMergeDesc);
    return 0;
}
/*-- 27.05.2004 14:46:28---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwMailMergePrepareMergePage, ExcludeHdl_Impl, CheckBox*, pBox)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    rConfigItem.ExcludeRecord( rConfigItem.GetResultSetPosition(), pBox->IsChecked());
    return 0;
};
/*-- 18.08.2004 10:36:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SwMailMergePrepareMergePage::ActivatePage()
{
    MoveHdl_Impl(&m_aRecordED);
}
/*-- 13.05.2004 15:38:32---------------------------------------------------
    merge the data into a new file
  -----------------------------------------------------------------------*/
sal_Bool  SwMailMergePrepareMergePage::commitPage(COMMIT_REASON _eReason)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    if(CR_TRAVEL_NEXT == _eReason && !rConfigItem.IsMergeDone())
    {
        m_pWizard->CreateTargetDocument();
        m_pWizard->SetRestartPage(MM_MERGEPAGE);
        m_pWizard->EndDialog(RET_TARGET_CREATED);
    }
    return sal_True;
}
