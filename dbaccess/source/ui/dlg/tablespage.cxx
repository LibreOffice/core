/*************************************************************************
 *
 *  $RCSfile: tablespage.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:28 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_TABLESPAGE_HXX_
#include "tablespage.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _DBAUI_DBADMIN_HRC_
#include "dbadmin.hrc"
#endif
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif
#ifndef _DBAUI_COMMONPAGES_HXX_
#include "commonpages.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _DBAUI_DATASOURCECONNECTOR_HXX_
#include "datasourceconnector.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _DBAUI_STRINGLISTITEM_HXX_
#include "stringlistitem.hxx"
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _DBAUI_DBADMIN_HXX_
#include "dbadmin.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDROP_HPP_
#include <com/sun/star/sdbcx/XDrop.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADEFINITIONSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBAUI_QUERYDESIGNACCESS_HXX_
#include "querydesignaccess.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SVTOOLS_IMGDEF_HXX
#include <svtools/imgdef.hxx>
#endif

#define RET_ALL     10

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::i18n;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::frame;
    using namespace ::dbtools;

    //========================================================================
    //= OTableSubscriptionPage
    //========================================================================
    //------------------------------------------------------------------------
    OTableSubscriptionPage::OTableSubscriptionPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OGenericAdministrationPage( pParent, ModuleRes(PAGE_TABLESUBSCRIPTION), _rCoreAttrs )
        ,OContainerListener( m_aNotifierMutex )
        ,m_aTables              (this, ResId(FL_SEPARATOR1))
        ,m_aActions             (this, ResId(TLB_ACTIONS))
        ,m_aTablesList          (this, ResId(CTL_TABLESUBSCRIPTION),sal_False)
        ,m_aExplanation         (this, ResId(FT_FILTER_EXPLANATION))
        ,m_aColumnsLine         (this, ResId(FL_SEPARATOR2))
        ,m_aSuppressVersionColumns(this, ResId(CB_SUPPRESVERSIONCL))
        ,m_bCheckedAll          ( sal_False )
        ,m_bCatalogAtStart      ( sal_True )
        ,m_pAdminDialog         ( NULL )
        ,m_bCanAddTables        ( sal_False )
        ,m_bCanDropTables       ( sal_False )
        ,m_bConnectionWriteable ( sal_False )
    {
        m_aTablesList.SetCheckHandler(getControlModifiedLink());
        m_aSuppressVersionColumns.SetClickHdl(getControlModifiedLink());

        m_aActions.SetSelectHdl(LINK(this, OTableSubscriptionPage, OnToolboxClicked));
        lcl_removeToolboxItemShortcuts(m_aActions);

        // initialize the TabListBox
        m_aTablesList.SetSelectionMode( MULTIPLE_SELECTION );
        m_aTablesList.SetDragDropMode( 0 );
        m_aTablesList.EnableInplaceEditing( sal_False );
        m_aTablesList.SetWindowBits(WB_BORDER | WB_HASLINES | WB_HASLINESATROOT | WB_SORT | WB_HASBUTTONS | WB_HSCROLL |WB_HASBUTTONSATROOT);
        m_aTablesList.SetSelectHdl(LINK(this, OTableSubscriptionPage, OnTreeEntrySelected));
        m_aTablesList.SetDeselectHdl(LINK(this, OTableSubscriptionPage, OnTreeEntrySelected));

        m_aTablesList.Clear();

        FreeResource();

        setToolBox(&m_aActions);

        m_aTablesList.SetCheckButtonHdl(LINK(this, OTableSubscriptionPage, OnTreeEntryChecked));
        m_aTablesList.SetCheckHandler(LINK(this, OTableSubscriptionPage, OnTreeEntryChecked));

        enableToolBoxAcceleration( &m_aActions );
        addToolboxAccelerator( ID_DROP_TABLE, KeyCode( KEY_DELETE ) );
    }

    //------------------------------------------------------------------------
    OTableSubscriptionPage::~OTableSubscriptionPage()
    {
        // just to make sure that our connection will be removed
        try
        {
            ::comphelper::disposeComponent(m_xCurrentConnection);
        }
        catch (RuntimeException&) { }
        m_bConnectionWriteable = m_bCanAddTables = m_bCanDropTables = sal_False;

        retireNotifiers();
    }

    // -----------------------------------------------------------------------------
    void OTableSubscriptionPage::StateChanged( StateChangedType nType )
    {
        OGenericAdministrationPage::StateChanged( nType );

        if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        {
            // Check if we need to get new images for normal/high contrast mode
            checkImageList();
            m_aTablesList.notifyHiContrastChanged();
        }
        else if ( nType == STATE_CHANGE_TEXT )
        {
            // The physical toolbar changed its outlook and shows another logical toolbar!
            // We have to set the correct high contrast mode on the new tbx manager.
            //  checkImageList();
        }
    }
    // -----------------------------------------------------------------------------
    void OTableSubscriptionPage::DataChanged( const DataChangedEvent& rDCEvt )
    {
        OGenericAdministrationPage::DataChanged( rDCEvt );

        if ((( rDCEvt.GetType() == DATACHANGED_SETTINGS )   ||
            ( rDCEvt.GetType() == DATACHANGED_DISPLAY   ))  &&
            ( rDCEvt.GetFlags() & SETTINGS_STYLE        ))
        {
            // Check if we need to get new images for normal/high contrast mode
            checkImageList();
            m_aTablesList.notifyHiContrastChanged();
        }
    }
    //------------------------------------------------------------------
    sal_Int16 OTableSubscriptionPage::getImageListId(sal_Int16 _eBitmapSet,sal_Bool _bHiContast) const
    {
        sal_Int16 nN = IMG_TABLESUBCRIPTION_SC;
        sal_Int16 nH = IMG_TABLESUBCRIPTION_SCH;
        if ( _eBitmapSet == SFX_SYMBOLS_LARGE )
        {
            nN = IMG_TABLESUBCRIPTION_LC;
            nH = IMG_TABLESUBCRIPTION_LCH;
        }

        return _bHiContast ? nH : nN;
    }
    //------------------------------------------------------------------
    void OTableSubscriptionPage::resizeControls(const Size& _rDiff)
    {
        if ( _rDiff.Height() )
        {
            Size aOldSize = m_aTablesList.GetSizePixel();
            aOldSize.Height() -= _rDiff.Height();
            m_aTablesList.SetPosSizePixel(
                    m_aTablesList.GetPosPixel()+Point(0,_rDiff.Height()),
                    aOldSize
                    );
        }
    }
    //------------------------------------------------------------------------
    void OTableSubscriptionPage::retireNotifiers()
    {
        for (   AdapterArrayIterator aLoop = m_aNotifiers.begin();
                aLoop != m_aNotifiers.end();
                ++aLoop
            )
        {
            if ( *aLoop )
            {
                ( *aLoop )->dispose();
                ( *aLoop )->release();
                ( *aLoop ) = NULL;
            }
        }
        m_aNotifiers.clear( );
    }

    //------------------------------------------------------------------------
    SfxTabPage* OTableSubscriptionPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
    {
        return ( new OTableSubscriptionPage( pParent, rAttrSet ) );
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::implCheckTables(const Sequence< ::rtl::OUString >& _rTables)
    {
        // the meta data for the current connection, used for splitting up table names
        Reference< XDatabaseMetaData > xMeta;
        try
        {
            if (m_xCurrentConnection.is())
                xMeta = m_xCurrentConnection->getMetaData();
        }
        catch(SQLException&)
        {
            DBG_ERROR("OTableSubscriptionPage::implCheckTables : could not retrieve the current connection's meta data!");
        }

        // uncheck all
        CheckAll(sal_False);

        // check the ones which are in the list
        String aListBoxTable;
        ::rtl::OUString sCatalog, sSchema, sName;

        SvLBoxEntry* pRootEntry = m_aTablesList.getAllObjectsEntry();
        sal_Bool bAllTables = sal_False;
        sal_Bool bAllSchemas = sal_False;

        const ::rtl::OUString* pIncludeTable = _rTables.getConstArray();
        for (sal_Int32 i=0; i<_rTables.getLength(); ++i, ++pIncludeTable)
        {
            if (xMeta.is())
                qualifiedNameComponents(xMeta, pIncludeTable->getStr(), sCatalog, sSchema, sName,::dbtools::eInDataManipulation);
            else
                sName = pIncludeTable->getStr();

            bAllTables = (1 == sName.getLength()) && ('%' == sName[0]);
            bAllSchemas = (1 == sSchema.getLength()) && ('%' == sSchema[0]);

            // the catalog entry
            SvLBoxEntry* pCatalog = m_aTablesList.GetEntryPosByName(sCatalog, pRootEntry);
            if (!pCatalog && sCatalog.getLength())
                // the table (resp. its catalog) refered in this filter entry does not exist anymore
                continue;

            if (bAllSchemas && pCatalog)
            {
                m_aTablesList.checkWildcard(pCatalog);
                continue;
            }

            // the schema entry
            SvLBoxEntry* pSchema = m_aTablesList.GetEntryPosByName(sSchema, pCatalog);
            if (!pSchema && sSchema.getLength())
                // the table (resp. its schema) refered in this filter entry does not exist anymore
                continue;

            if (bAllTables && pSchema)
            {
                m_aTablesList.checkWildcard(pSchema);
                continue;
            }

            SvLBoxEntry* pEntry = m_aTablesList.GetEntryPosByName(sName, pSchema);
            if (pEntry)
                m_aTablesList.SetCheckButtonState(pEntry, SV_BUTTON_CHECKED);
        }
        m_aTablesList.CheckButtons();
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::implCompleteTablesCheck( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableFilter )
    {
        if (!_rTableFilter.getLength())
        {   // no tables visible
            CheckAll(sal_False);
        }
        else
        {
            if ((1 == _rTableFilter.getLength()) && _rTableFilter[0].equalsAsciiL("%", 1))
            {   // all tables visible
                CheckAll(sal_True);
            }
            else
                implCheckTables( _rTableFilter );
        }
    }

    //-------------------------------------------------------------------------
    void OTableSubscriptionPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        bValid = bValid && m_xCurrentConnection.is();
        bReadonly = bReadonly || !bValid;

        m_aTables.Enable(!bReadonly);
        m_aActions.Enable(!bReadonly);
        m_aTablesList.Enable(!bReadonly);
        m_aExplanation.Enable(!bReadonly);
        m_aColumnsLine.Enable(!bReadonly);
        m_aSuppressVersionColumns.Enable(!bReadonly);

        // get the current table filter
        SFX_ITEMSET_GET(_rSet, pTableFilter, OStringListItem, DSID_TABLEFILTER, sal_True);
        SFX_ITEMSET_GET(_rSet, pSuppress, SfxBoolItem, DSID_SUPPRESSVERSIONCL, sal_True);
        Sequence< ::rtl::OUString > aTableFilter;
        sal_Bool bSuppressVersionColumns = sal_True;
        if (pTableFilter)
            aTableFilter = pTableFilter->getList();
        if (pSuppress)
            bSuppressVersionColumns = pSuppress->GetValue();

        implCompleteTablesCheck( aTableFilter );

        // expand the first entry by default
        SvLBoxEntry* pExpand = m_aTablesList.getAllObjectsEntry();
        while (pExpand)
        {
            m_aTablesList.Expand(pExpand);
            pExpand = m_aTablesList.FirstChild(pExpand);
            if (pExpand && m_aTablesList.NextSibling(pExpand))
                pExpand = NULL;
        }

        // update the toolbox according the the current selection and check state
        implUpdateToolbox();

        if (!bValid)
            m_aSuppressVersionColumns.Check(!bSuppressVersionColumns);
        if (_bSaveValue)
            m_aSuppressVersionColumns.SaveValue();
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::CheckAll( sal_Bool _bCheck )
    {
        SvButtonState eState = _bCheck ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED;
        SvLBoxEntry* pEntry = m_aTablesList.First();
        while (pEntry)
        {
            m_aTablesList.SetCheckButtonState( pEntry, eState);
            pEntry = m_aTablesList.Next(pEntry);
        }

        if (_bCheck && m_aTablesList.getAllObjectsEntry())
            m_aTablesList.checkWildcard(m_aTablesList.getAllObjectsEntry());
    }

    //------------------------------------------------------------------------
    int OTableSubscriptionPage::DeactivatePage(SfxItemSet* _pSet)
    {
        int nResult = OGenericAdministrationPage::DeactivatePage(_pSet);

        // dispose the connection, we don't need it anymore, so we're not wasting resources
        try
        {
            ::comphelper::disposeComponent(m_xCurrentConnection);
        }
        catch (RuntimeException&) { }
        m_bConnectionWriteable = m_bCanAddTables = m_bCanDropTables = sal_False;

        retireNotifiers();

        return nResult;
    }

    //------------------------------------------------------------------------
    namespace {
        static void lcl_addHint( String& _rItemText, sal_Bool _bActuallyNeedHint, const String& _rHint )
        {
            xub_StrLen nCurrentHintPos = _rItemText.SearchAscii( "  " );
            sal_Bool bHaveHint = ( STRING_NOTFOUND != nCurrentHintPos );

            if ( bHaveHint )
            {   // remove the hint in any case - even if there currently is one, the new one may be different
                _rItemText = _rItemText.Copy( 0, nCurrentHintPos );
                bHaveHint = sal_False;
            }

            if ( !bHaveHint && _bActuallyNeedHint )
            {
                _rItemText.AppendAscii( "  " );
                _rItemText += _rHint;
            }
        }
        static void lcl_updateHint( ToolBox& _rTB, sal_uInt16 _nItemId, sal_Bool _bNeedHint, sal_uInt16 _nHintId )
        {
            // the current item text
            String sText = _rTB.GetItemText( _nItemId );
            // the hint (add or remove)
            lcl_addHint( sText, _bNeedHint, String( ModuleRes( _nHintId ) ) );
            // set as new item text
            _rTB.SetItemText( _nItemId, sText );
        }
    }
    //........................................................................
    void OTableSubscriptionPage::implAdjustToolBoxTexts()
    {
        // in general, if the connection is read-only, all is disabled
        lcl_updateHint( m_aActions, ID_NEW_TABLE_DESIGN,    !m_bConnectionWriteable, STR_HINT_READONLY_CONNECTION );
        lcl_updateHint( m_aActions, ID_DROP_TABLE,          !m_bConnectionWriteable, STR_HINT_READONLY_CONNECTION );
        lcl_updateHint( m_aActions, ID_EDIT_TABLE,          !m_bConnectionWriteable, STR_HINT_READONLY_CONNECTION );

        if ( m_bConnectionWriteable )
        {   // for add and drop, in case the connection is writeable in general, there are more options
            lcl_updateHint( m_aActions, ID_DROP_TABLE, !m_bCanDropTables, STR_HINT_CONNECTION_NOT_CAPABLE );
            lcl_updateHint( m_aActions, ID_EDIT_TABLE, !m_bCanAddTables, STR_HINT_CONNECTION_NOT_CAPABLE );
        }
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::ActivatePage(const SfxItemSet& _rSet)
    {
        DBG_ASSERT(!m_xCurrentConnection.is(), "OTableSubscriptionPage::ActivatePage: already have an active connection! ");

        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        // get the name of the data source we're working for
        SFX_ITEMSET_GET(_rSet, pNameItem, SfxStringItem, DSID_NAME, sal_True);
        DBG_ASSERT(pNameItem, "OTableSubscriptionPage::ActivatePage: missing the name attribute!");
        m_sDSName = pNameItem->GetValue();

        if (bValid)
        {   // get the current table list from the connection for the current settings

            // the PropertyValues for the current dialog settings
            Sequence< PropertyValue > aConnectionParams;
            DBG_ASSERT(m_pAdminDialog, "OTableSubscriptionPage::ActivatePage : need a parent dialog doing the translation!");
            if (m_pAdminDialog)
            {
                if (!m_pAdminDialog->getCurrentSettings(aConnectionParams))
                {
                    OGenericAdministrationPage::ActivatePage(_rSet);
                    m_aTablesList.Clear();
                    return;
                }
            }

            if (!m_xCollator.is())
            {
                // the collator for the string compares
                m_xCollator = Reference< XCollator >(m_xORB->createInstance(SERVICE_I18N_COLLATOR), UNO_QUERY);
                if (m_xCollator.is())
                    m_xCollator->loadDefaultCollator(Application::GetSettings().GetLocale(), 0);
            }

            // fill the table list with this connection information
            SQLExceptionInfo aErrorInfo;
            // the current DSN
            if(!m_xCurrentConnection.is())
            {
                String sURL;
                SFX_ITEMSET_GET(_rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);
                sURL = pUrlItem->GetValue();

                try
                {
                    WaitObject aWaitCursor(this);
                    m_aTablesList.GetModel()->SetSortMode(SortAscending);
                    m_aTablesList.GetModel()->SetCompareHdl(LINK(this, OTableSubscriptionPage, OnTreeEntryCompare));

                    Reference< XDriver > xDriver;
                    m_xCurrentConnection = m_aTablesList.UpdateTableList( sURL, aConnectionParams, xDriver );
                    if ( m_xCurrentConnection.is() )
                    {
                        if (m_pAdminDialog)
                            m_pAdminDialog->successfullyConnected();
                    }

                    // collect some meta data about the connection
                    Reference< XDatabaseMetaData > xMetaData;
                    if ( m_xCurrentConnection.is() )
                        xMetaData = m_xCurrentConnection->getMetaData();

                    // is the connection writeable in general?
                    m_bConnectionWriteable = xMetaData.is() && !xMetaData->isReadOnly();

                    // for other infos we need to check the tables supplier
                    Reference< XTablesSupplier > xSuppTables( m_xCurrentConnection, UNO_QUERY );
                    if ( !xSuppTables.is() )
                    {
                        Reference< XDataDefinitionSupplier > xDataDefSupp( xDriver, UNO_QUERY );
                        if ( xDataDefSupp.is() )
                            xSuppTables = xSuppTables.query( xDataDefSupp->getDataDefinitionByConnection( m_xCurrentConnection ) );
                    }

                    if ( !xSuppTables.is() )
                    {   // assume that we can do anything
                        // The point is, we have a low-level connection here, not necessarily an SDB level connection
                        // But when the user connects later on (using the XDataSource of a data source), (s)he gets
                        // a SDB level connection which may support adding and dropping tables, though the underlying
                        // low level connection isn't
                        m_bCanDropTables = sal_True;
                        m_bCanAddTables = sal_True;
                    }
                    else
                    {
                        // can we drop tables?
                        Reference< XDrop > xDropTables;
                        if ( xSuppTables.is() )
                            xDropTables = xDropTables.query( xSuppTables->getTables() );
                        m_bCanDropTables = xDropTables.is();

                        // can we add tables?
                        Reference< XAppend > xAppendTables;
                        if ( xSuppTables.is() )
                            xAppendTables = xAppendTables.query( xSuppTables->getTables() );
                        m_bCanAddTables = xAppendTables.is();
                    }
                }
                catch (SQLContext& e) { aErrorInfo = SQLExceptionInfo(e); }
                catch (SQLWarning& e) { aErrorInfo = SQLExceptionInfo(e); }
                catch (SQLException& e) { aErrorInfo = SQLExceptionInfo(e); }

                // adjust the toolbox texts according
                implAdjustToolBoxTexts();
            }

            if (aErrorInfo.isValid())
            {
                // establishing the connection failed. Show an error window and exit.
                OSQLMessageBox aMessageBox(GetParent(), aErrorInfo, WB_OK | WB_DEF_OK, OSQLMessageBox::Error);
                aMessageBox.Execute();
                m_aTables.Enable(sal_False);
                m_aActions.Enable(sal_False);
                m_aTablesList.Enable(sal_False);
                m_aExplanation.Enable(sal_False);
                m_aColumnsLine.Enable(sal_False);
                m_aSuppressVersionColumns.Enable(sal_False);
                m_aTablesList.Clear();

                if (m_pAdminDialog)
                    m_pAdminDialog->clearPassword();
            }
            else
            {
                // in addition, we need some infos about the connection used
                m_sCatalogSeparator = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));    // (default)
                m_bCatalogAtStart = sal_True;   // (default)
                try
                {
                    Reference< XDatabaseMetaData > xMeta;
                    if (m_xCurrentConnection.is())
                        xMeta = m_xCurrentConnection->getMetaData();
                    if (xMeta.is() && xMeta->supportsCatalogsInDataManipulation())
                    {
                        m_sCatalogSeparator = xMeta->getCatalogSeparator();
                        m_bCatalogAtStart = xMeta->isCatalogAtStart();
                    }
                }
                catch(SQLException&)
                {
                    DBG_ERROR("OTableSubscriptionPage::ActivatePage : could not retrieve the qualifier separator for the used connection !");
                }
            }
        }

        // if we're (resp. the dialog) is in a mode where only editing of a single data source is allowed ...
        const sal_Bool bAnySingleEditMode = ( ODbAdminDialog::omFull != m_pAdminDialog->getMode() );
        const sal_Bool bPreviouslySingleEditMode = !m_aActions.IsVisible();
        if ( bAnySingleEditMode != bPreviouslySingleEditMode )
        {
            // ... we don't offer the toolbox to the user
            m_aActions.Show( !bAnySingleEditMode );

            // resize the listbox (below the toolbox) accordingly
            Size aSize = m_aTablesList.GetSizePixel();
            Point aPos = m_aTablesList.GetPosPixel();

            sal_Int32 nResizeY = m_aTablesList.GetPosPixel().Y() - m_aActions.GetPosPixel().Y();

            aPos.Y() -= nResizeY;
            aSize.Height() += nResizeY;

            m_aTablesList.SetPosSizePixel( aPos, aSize );
        }

        OGenericAdministrationPage::ActivatePage(_rSet);
    }

    //------------------------------------------------------------------------
    IMPL_LINK( OTableSubscriptionPage, OnTreeEntrySelected, void*, NOTINTERESTEDIN )
    {
        implUpdateToolbox();
        return 0L;
    }

    //------------------------------------------------------------------------
    IMPL_LINK( OTableSubscriptionPage, OnTreeEntryChecked, Control*, _pControl )
    {
        implUpdateToolbox();
        return OnControlModified(_pControl);
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::implUpdateToolbox()
    {
        // is the page connected?
        sal_Bool bConnected = m_xCurrentConnection.is();

        // is there _any_ selected entry
        SvLBoxEntry* pSelected = m_aTablesList.FirstSelected();
        sal_Bool bSelectedAnything = (NULL != pSelected);

        // is exactly one entry selected?
        sal_Bool bSelectedOne = (NULL != pSelected) && (NULL == m_aTablesList.NextSelected(pSelected));

        // are there tables only?
        sal_Bool bLeafsOnly = bSelectedAnything;
        // all tables which are selected are checked, too?
        sal_Bool bAllLeafsChecked = bSelectedAnything;
        while (pSelected)
        {
            if (0 != m_aTablesList.GetChildCount(pSelected))
                // it's a container which is selected here
                bLeafsOnly = sal_False;
            else
            {   // it's a leaf (ergo a table or view)
                SvButtonState eState = m_aTablesList.GetCheckButtonState(pSelected);
                OSL_ENSURE(SV_BUTTON_TRISTATE != eState, "OTableSubscriptionPage::implUpdateToolbox: a tristate table?");
                bAllLeafsChecked = bAllLeafsChecked && (SV_BUTTON_CHECKED == eState);
            }

            pSelected = m_aTablesList.NextSelected(pSelected);
        }

        Reference< XDatabaseMetaData > xMetaData;
        if ( m_xCurrentConnection.is() )
            xMetaData = m_xCurrentConnection->getMetaData();

        // TODO: disable the EDIT for views

        m_aActions.EnableItem(ID_NEW_TABLE_DESIGN,  bConnected && m_bCanAddTables  && m_bConnectionWriteable);
        m_aActions.EnableItem(ID_DROP_TABLE,        bConnected && m_bCanDropTables && m_bConnectionWriteable &&                 bLeafsOnly && bAllLeafsChecked);
        m_aActions.EnableItem(ID_EDIT_TABLE,        bConnected &&                     m_bConnectionWriteable && bSelectedOne && bLeafsOnly && bAllLeafsChecked);
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::collectEntryPaths(StringArray& _rFillInPaths, EntryPredicateCheck _pPredicateCheck)
    {
        _rFillInPaths.clear();

        SvLBoxEntry* pRoot = m_aTablesList.getAllObjectsEntry();
        SvLBoxEntry* pLoop = pRoot ? m_aTablesList.FirstChild(pRoot) : NULL;

        StringArray aCurrentEntryPath;
        const ::rtl::OUString sPathSeparator = ::rtl::OUString::createFromAscii(".");

        sal_Bool bAlreadyVisitedCurrent = sal_False;
        while (pLoop)
        {
            // is this entry expanded ?
            if (!bAlreadyVisitedCurrent && (m_aTablesList.*_pPredicateCheck)(pLoop))
            {   // -> add it to the view settings' lits
                ::rtl::OUString sThisEntryPath;
                for (   ConstStringArrayIterator aPathElements = aCurrentEntryPath.begin();
                        aPathElements != aCurrentEntryPath.end();
                        ++aPathElements
                    )
                {
                    sThisEntryPath += *aPathElements;
                    sThisEntryPath += sPathSeparator;
                }
                sThisEntryPath += m_aTablesList.GetEntryText(pLoop);
                _rFillInPaths.push_back(sThisEntryPath);
            }

            // if the entry has children, step down
            SvLBoxEntry* pCandidate = m_aTablesList.FirstChild(pLoop);
            if (pCandidate && !bAlreadyVisitedCurrent)
            {
                // remember the text for this level
                aCurrentEntryPath.push_back(m_aTablesList.GetEntryText(pLoop));
                // step down
                pLoop  = pCandidate;
                bAlreadyVisitedCurrent = sal_False;
            }
            else
            {
                pCandidate = m_aTablesList.NextSibling(pLoop);
                if (pCandidate)
                {
                    pLoop = pCandidate;
                    bAlreadyVisitedCurrent = sal_False;
                }
                else
                {   // step up
                    pCandidate = m_aTablesList.GetParent(pLoop);
                    if (pCandidate == pRoot)
                        // don't go further
                        pCandidate = NULL;

                    DBG_ASSERT( (NULL != pCandidate) == (0 != aCurrentEntryPath.size()),
                        "OTableSubscriptionPage::collectEntryPaths: inconsistence!");
                    if (aCurrentEntryPath.size())
                        aCurrentEntryPath.pop_back();

                    // don't step down again
                    bAlreadyVisitedCurrent = sal_True;

                    pLoop = pCandidate;
                }
            }
        }
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::doExpand(SvLBoxEntry* _pEntry)
    {
        m_aTablesList.Expand(_pEntry);
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::doSelect(SvLBoxEntry* _pEntry)
    {
        m_aTablesList.Select(_pEntry, sal_True);
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::actOnEntryPaths(const StringArray& _rPaths, EntryAction _pAction)
    {
        // TODO: this whole stuff is not really performant ....
        // (but hey, this is no critical area ....)

        for (   ConstStringArrayIterator aLoop = _rPaths.begin();
                aLoop != _rPaths.end();
                ++aLoop
            )
        {
            SvLBoxEntry* pEntry = getEntryFromPath(*aLoop);
            if (pEntry)
                (this->*_pAction)(pEntry);
        }
    }

    //------------------------------------------------------------------------
    SvLBoxEntry* OTableSubscriptionPage::getEntryFromPath(const ::rtl::OUString& _rPath)
    {
        const sal_Unicode aSeparator = '.';

        SvLBoxEntry* pParent = m_aTablesList.getAllObjectsEntry();

        sal_Int32 nSepPos = -1;
        sal_Int32 nPreviousSegmentStart = 0;

        nSepPos = _rPath.indexOf(aSeparator, nPreviousSegmentStart);
        while ((nPreviousSegmentStart >= 0) && pParent)
        {
            String sSegmentName = _rPath.copy(
                nPreviousSegmentStart,
                (nSepPos > nPreviousSegmentStart ? nSepPos : _rPath.getLength()) - nPreviousSegmentStart);

            SvLBoxEntry* pChildSearch = m_aTablesList.FirstChild(pParent);
            String sChildText;
            while (pChildSearch)
            {
                sChildText = m_aTablesList.GetEntryText(pChildSearch);
                if (sChildText == sSegmentName)
                    break;

                pChildSearch = m_aTablesList.NextSibling(pChildSearch);
            }

            if (!pChildSearch)
            {   // did not find this levels child
                pParent = NULL;
                break;
            }

            pParent = pChildSearch;

            if (nSepPos > 0)
                nSepPos = _rPath.indexOf(aSeparator, nPreviousSegmentStart = (nSepPos + 1));
            else
                nPreviousSegmentStart = -1;
        }

        return pParent;
    }

    //------------------------------------------------------------------------
    OPageSettings* OTableSubscriptionPage::createViewSettings()
    {
        return new OTablePageViewSettings;
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::fillViewSettings(OPageSettings* _pSettings)
    {
        OTablePageViewSettings* pMySettings = static_cast<OTablePageViewSettings*>(_pSettings);
        if (!pMySettings)
            return;

        // collect the names of the expanded extries
        EntryPredicateCheck aExpandedCheck =&SvListView::IsExpanded;
        collectEntryPaths(pMySettings->aExpandedEntries, aExpandedCheck);

        // collect the names of the selected extries
        EntryPredicateCheck aSelectedCheck = &SvListView::IsSelected;
        collectEntryPaths(pMySettings->aSelectedEntries, aSelectedCheck);

        SvLBoxEntry* pCurEntry = m_aTablesList.GetCurEntry();
        if (pCurEntry)
        {
            StringArray aLocalNames;
            while (pCurEntry && (pCurEntry != m_aTablesList.getAllObjectsEntry()))
            {
                aLocalNames.push_back(m_aTablesList.GetEntryText(pCurEntry));
                pCurEntry = m_aTablesList.GetParent(pCurEntry);
            }

            const ::rtl::OUString sSeparator = ::rtl::OUString::createFromAscii(".");

            // assemble the name
            pMySettings->sFocusEntry = ::rtl::OUString();
            for (   StringArray::reverse_iterator aSegments = aLocalNames.rbegin();
                    aSegments != aLocalNames.rend();

                )
            {
                pMySettings->sFocusEntry += *aSegments;
                if (++aSegments != aLocalNames.rend())
                    pMySettings->sFocusEntry += sSeparator;
            }
        }
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::restoreViewSettings(const OPageSettings* _pSettings)
    {
        const OTablePageViewSettings* pMySettings = static_cast<const OTablePageViewSettings*>(_pSettings);
        if (!pMySettings)
            return;

        // expand the entries
        // TODO: some kind of collapse all
        actOnEntryPaths(pMySettings->aExpandedEntries, (void (OTableSubscriptionPage::*)(SvLBoxEntry*))doExpand);

        // select the entries
        m_aTablesList.SelectAll(sal_False);
        actOnEntryPaths(pMySettings->aSelectedEntries, (void (OTableSubscriptionPage::*)(SvLBoxEntry*))doSelect);

        SvLBoxEntry* pFocusEntry = getEntryFromPath(pMySettings->sFocusEntry);
        if (pFocusEntry)
            m_aTablesList.SetCurEntry(pFocusEntry);

        if (pMySettings->nDelayedToolboxAction)
            onToolBoxAction(pMySettings->nDelayedToolboxAction);
    }

    //------------------------------------------------------------------------
    ::rtl::OUString OTableSubscriptionPage::getComposedEntryName(SvLBoxEntry* _pEntry)
    {
        SvLBoxEntry* pSchema = NULL;
        SvLBoxEntry* pCatalog = NULL;
        SvLBoxEntry* pAllObjectsEntry = m_aTablesList.getAllObjectsEntry();
        ::rtl::OUString sCatalog;
        ::rtl::OUString sComposedName;
        if (m_aTablesList.GetModel()->HasParent(_pEntry))
        {
            pSchema = m_aTablesList.GetModel()->GetParent(_pEntry);
            if (pAllObjectsEntry == pSchema)
                // do not want to have the root entry
                pSchema = NULL;

            if (pSchema)
            {   // it's a real schema entry, not the "all objects" root
                if (m_aTablesList.GetModel()->HasParent(pSchema))
                {
                    pCatalog = m_aTablesList.GetModel()->GetParent(pSchema);
                    if (pAllObjectsEntry == pCatalog)
                        // do not want to have the root entry
                        pCatalog = NULL;

                    if (pCatalog)
                    {   // it's a real catalog entry, not the "all objects" root
                        if (m_bCatalogAtStart)
                        {
                            sComposedName += m_aTablesList.GetEntryText( pCatalog );
                            sComposedName += m_sCatalogSeparator;
                        }
                        else
                        {
                            sCatalog += m_sCatalogSeparator;
                            sCatalog += m_aTablesList.GetEntryText( pCatalog );
                        }
                    }
                }
                sComposedName += m_aTablesList.GetEntryText( pSchema );
                sComposedName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));
            }
        }
        sComposedName += m_aTablesList.GetEntryText( _pEntry );
        if (!m_bCatalogAtStart)
            sComposedName += sCatalog;

        return sComposedName;
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::dropSelection()
    {
        DBG_ASSERT(!m_pAdminDialog->isCurrentModified(), "OTableSubscriptionPage::dropSelection: invalid call!");

        // get a connection for the data source we're working for
        Reference< XConnection > xConnection;
        ODatasourceConnector aConnector(m_xORB, GetParent());
        xConnection = aConnector.connect(m_sDSName);

        if (!xConnection.is())
            // handled by the connector (should have shown an error message)
            return;

        Reference< XTablesSupplier > xSuppTables(xConnection, UNO_QUERY);
        Reference< XNameAccess > xTables;
        if (xSuppTables.is())
            xTables = xSuppTables->getTables();

        Reference< XDrop > xDropTable(xTables, UNO_QUERY);
        if (!xDropTable.is())
        {
            String sMessage(ModuleRes(STR_MISSING_TABLES_XDROP));
            ErrorBox aError(GetParent(), WB_OK, sMessage);
            aError.Execute();
            return;
        }
        else
        {
            sal_Bool bConfirm = sal_True;
            ::std::vector< void* > aSelection;
            SvLBoxEntry* pSelected = m_aTablesList.FirstSelected();
            while (pSelected)
            {
                aSelection.push_back(pSelected);
                pSelected = m_aTablesList.NextSelected(pSelected);
            }

            if ( !aSelection.empty() )
            {
                ::std::vector< void* >::const_iterator aLast = aSelection.end();
                --aLast;

                for (   ::std::vector< void* >::const_iterator aLoop = aSelection.begin();
                        aLoop != aSelection.end();
                        ++aLoop
                    )
                {
                    SvLBoxEntry* pSelected = static_cast<SvLBoxEntry*>(*aLoop);
                    // the composed table name
                    String sCompleteTableName = getComposedEntryName(pSelected);

                    // let the user confirm this
                    sal_Int32 nResult = RET_YES;
                    if (bConfirm)
                    {
                        // let the user confirm this
                        String sMessage(ModuleRes(STR_QUERY_DELETE_TABLE));
                        sMessage.SearchAndReplace(String::CreateFromAscii("%1"), sCompleteTableName);

                        QueryBox aAsk(GetParent(), WB_YES_NO | WB_DEF_YES, sMessage);
                        aAsk.SetText(String(ModuleRes(STR_TITLE_CONFIRM_DELETION)));

                        // add an "all" button
                        if ( aLast != aLoop )
                        {
                            aAsk.AddButton(String(ModuleRes(STR_BUTTON_TEXT_ALL)), RET_ALL, 0);
                            aAsk.GetPushButton(RET_ALL)->SetHelpId(HID_CONFIRM_DROP_BUTTON_ALL);
                        }

                        nResult = aAsk.Execute();
                    }

                    if ((RET_YES == nResult) || (RET_ALL == nResult))
                    {
                        SQLExceptionInfo aErrorInfo;
                        try
                        {
                            xDropTable->dropByName(sCompleteTableName);

                            // remove the entry from the list
                            m_aTablesList.GetModel()->Remove(pSelected);
                        }
                        catch(SQLContext& e) { aErrorInfo = e; }
                        catch(SQLWarning& e) { aErrorInfo = e; }
                        catch(SQLException& e) { aErrorInfo = e; }
                        catch(Exception&)
                        {
                            DBG_ERROR("OTableSubscriptionPage::dropSelection: suspicious exception caught!");
                        }
                        if (aErrorInfo.isValid())
                            showError(aErrorInfo, GetParent(), m_xORB);
                    }

                    if (RET_ALL == nResult)
                        bConfirm = sal_False;
                }
            }
        }

        ::comphelper::disposeComponent(xConnection);
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::onToolBoxAction(sal_uInt16 _nId)
    {
        if (m_pAdminDialog->isCurrentModified())
        {
            // get the current view settings
            OTablePageViewSettings* pMySettings = new OTablePageViewSettings;
            fillViewSettings(pMySettings);
            pMySettings->nDelayedToolboxAction = _nId;

            OPageSettings* pTypedSettings = pMySettings;

            if (!prepareConnectionAction(m_pAdminDialog, m_aActions.GetItemText(_nId), &pTypedSettings))
                return;
        }

        // get the name of the selected entry
        SvLBoxEntry* pSelected;
        String sSelectedEntry;
        if ((ID_DROP_TABLE == _nId) || (ID_EDIT_TABLE == _nId))
        {
            pSelected = m_aTablesList.FirstSelected();
            if (!pSelected)
            {
                DBG_ERROR("OTableSubscriptionPage::onToolBoxAction: to be called if at least one entry is selected!");
                return;
            }

            if ((m_aTablesList.NextSelected(pSelected)) && (ID_DROP_TABLE != _nId))
            {
                DBG_ERROR("OTableSubscriptionPage::onToolBoxAction: EDIT can't be applied to more than one table!");
                return;
            }

            sSelectedEntry = getComposedEntryName(pSelected);
        }

        switch (_nId)
        {
            case ID_NEW_TABLE_DESIGN:
            {
                OTableDesignAccess aDispatcher(m_xORB);
                Reference< XComponent > xComp = aDispatcher.create(m_sDSName, Reference< XConnection >());
                OSL_ENSURE( xComp.is(), "OTableSubscriptionPage::onToolBoxAction: could not load the component!" );

                if ( xComp.is() )
                {   // successfully loaded

                    // add a container listener to the tables container the component is about to extend ....
                    try
                    {
                        // get the property set of the controller we just loaded
                        Reference< XPropertySet > xCompProps( xComp, UNO_QUERY );
                        Reference< XPropertySetInfo > xPSI;
                        if ( xCompProps.is() ) xPSI = xCompProps->getPropertySetInfo();
                        OSL_ENSURE( xPSI.is() && xPSI->hasPropertyByName( PROPERTY_ACTIVECONNECTION ),
                            "OTableSubscriptionPage::onToolBoxAction: invalid controller!" );

                        // get the connection the controller is working with
                        if ( xPSI.is() && xPSI->hasPropertyByName( PROPERTY_ACTIVECONNECTION ) )
                        {
                            Reference< XTablesSupplier > xSuppTables;
                            xCompProps->getPropertyValue( PROPERTY_ACTIVECONNECTION ) >>= xSuppTables;
                            OSL_ENSURE( xSuppTables.is(), "OTableSubscriptionPage::onToolBoxAction: the controller has an invalid connection!" );
                            if ( xSuppTables.is() )
                            {
                                Reference< XContainer > xTables( xSuppTables->getTables(), UNO_QUERY );
                                OSL_ENSURE( xTables.is(), "OTableSubscriptionPage::onToolBoxAction: invalid tables container!" );
                                // create a notifier for the container so we know if a table is inserted
                                if ( xTables.is() )
                                {
                                    OContainerListenerAdapter* pNotifier = new OContainerListenerAdapter( this, xTables );
                                    pNotifier->acquire( );
                                    m_aNotifiers.push_back( pNotifier );
                                }
                            }
                        }
                    }
                    catch( const Exception& )
                    {
                    }
                }
            }
            break;

            case ID_EDIT_TABLE:
            {
                OTableDesignAccess aDispatcher(m_xORB);
                aDispatcher.edit(m_sDSName, sSelectedEntry, Reference< XConnection >());
            }
            break;

            case ID_DROP_TABLE:
                dropSelection();
                break;

        }
    }

    //------------------------------------------------------------------------
    IMPL_LINK( OTableSubscriptionPage, OnToolboxClicked, void*, NOTINTERESTEDIN )
    {
        onToolBoxAction(m_aActions.GetCurItemId());
        return 0L;
    }

    //------------------------------------------------------------------------
    IMPL_LINK( OTableSubscriptionPage, OnTreeEntryCompare, const SvSortData*, _pSortData )
    {
        SvLBoxEntry* pLHS = static_cast<SvLBoxEntry*>(_pSortData->pLeft);
        SvLBoxEntry* pRHS = static_cast<SvLBoxEntry*>(_pSortData->pRight);
        DBG_ASSERT(pLHS && pRHS, "SbaTableQueryBrowser::OnTreeEntryCompare: invalid tree entries!");

        SvLBoxString* pLeftTextItem = static_cast<SvLBoxString*>(pLHS->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
        SvLBoxString* pRightTextItem = static_cast<SvLBoxString*>(pRHS->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
        DBG_ASSERT(pLeftTextItem && pRightTextItem, "SbaTableQueryBrowser::OnTreeEntryCompare: invalid text items!");

        String sLeftText = pLeftTextItem->GetText();
        String sRightText = pRightTextItem->GetText();

        sal_Int32 nCompareResult = 0;   // equal by default

        if (m_xCollator.is())
        {
            try
            {
                nCompareResult = m_xCollator->compareString(sLeftText, sRightText);
            }
            catch(Exception&)
            {
            }
        }
        else
            // default behaviour if we do not have a collator -> do the simple string compare
            nCompareResult = sLeftText.CompareTo(sRightText);

        return nCompareResult;
    }

    //------------------------------------------------------------------------
    Sequence< ::rtl::OUString > OTableSubscriptionPage::collectDetailedSelection() const
    {
        Sequence< ::rtl::OUString > aTableFilter;
        static const ::rtl::OUString sDot(RTL_CONSTASCII_USTRINGPARAM("."));
        static const ::rtl::OUString sWildcard(RTL_CONSTASCII_USTRINGPARAM("%"));

        ::rtl::OUString sComposedName;
        const SvLBoxEntry* pAllObjectsEntry = m_aTablesList.getAllObjectsEntry();
        if (!pAllObjectsEntry)
            return aTableFilter;
        SvLBoxEntry* pEntry = m_aTablesList.GetModel()->Next(const_cast<SvLBoxEntry*>(pAllObjectsEntry));
        while(pEntry)
        {
            sal_Bool bCatalogWildcard = sal_False;
            sal_Bool bSchemaWildcard =  sal_False;
            SvLBoxEntry* pSchema = NULL;
            SvLBoxEntry* pCatalog = NULL;

            if (m_aTablesList.GetCheckButtonState(pEntry) == SV_BUTTON_CHECKED && !m_aTablesList.GetModel()->HasChilds(pEntry))
            {   // checked and a leaf, which means it's no catalog, no schema, but a real table
                ::rtl::OUString sCatalog;
                if(m_aTablesList.GetModel()->HasParent(pEntry))
                {
                    pSchema = m_aTablesList.GetModel()->GetParent(pEntry);
                    if (pAllObjectsEntry == pSchema)
                        // do not want to have the root entry
                        pSchema = NULL;

                    if (pSchema)
                    {   // it's a real schema entry, not the "all objects" root
                        if(m_aTablesList.GetModel()->HasParent(pSchema))
                        {
                            pCatalog = m_aTablesList.GetModel()->GetParent(pSchema);
                            if (pAllObjectsEntry == pCatalog)
                                // do not want to have the root entry
                                pCatalog = NULL;

                            if (pCatalog)
                            {   // it's a real catalog entry, not the "all objects" root
                                bCatalogWildcard = m_aTablesList.isWildcardChecked(pCatalog);
                                if (m_bCatalogAtStart)
                                {
                                    sComposedName += m_aTablesList.GetEntryText( pCatalog );
                                    sComposedName += m_sCatalogSeparator;
                                    if (bCatalogWildcard)
                                        sComposedName += sWildcard;
                                }
                                else
                                {
                                    if (bCatalogWildcard)
                                        sCatalog = sWildcard;
                                    else
                                        sCatalog = ::rtl::OUString();
                                    sCatalog += m_sCatalogSeparator;
                                    sCatalog += m_aTablesList.GetEntryText( pCatalog );
                                }
                            }
                        }
                        bSchemaWildcard = m_aTablesList.isWildcardChecked(pSchema);
                        sComposedName += m_aTablesList.GetEntryText( pSchema );
                        sComposedName += sDot;
                    }

                    if (bSchemaWildcard)
                        sComposedName += sWildcard;
                }
                if (!bSchemaWildcard && !bCatalogWildcard)
                    sComposedName += m_aTablesList.GetEntryText( pEntry );

                if (!m_bCatalogAtStart && !bCatalogWildcard)
                    sComposedName += sCatalog;

                // need some space
                sal_Int32 nOldLen = aTableFilter.getLength();
                aTableFilter.realloc(nOldLen + 1);
                // add the new name
                aTableFilter[nOldLen] = sComposedName;

                // reset the composed name
                sComposedName = ::rtl::OUString();
            }

            if (bCatalogWildcard)
                pEntry = implNextSibling(pCatalog);
            else if (bSchemaWildcard)
                pEntry = implNextSibling(pSchema);
            else
                pEntry = m_aTablesList.GetModel()->Next(pEntry);
        }

        return aTableFilter;
    }

    //------------------------------------------------------------------------
    SvLBoxEntry* OTableSubscriptionPage::implNextSibling(SvLBoxEntry* _pEntry) const
    {
        SvLBoxEntry* pReturn = NULL;
        if (_pEntry)
        {
            pReturn = m_aTablesList.NextSibling(_pEntry);
            if (!pReturn)
                pReturn = implNextSibling(m_aTablesList.GetParent(_pEntry));
        }
        return pReturn;
    }

    //------------------------------------------------------------------------
    sal_Bool OTableSubscriptionPage::FillItemSet( SfxItemSet& _rCoreAttrs )
    {
        sal_Bool bValid, bReadonly;
        getFlags(_rCoreAttrs, bValid, bReadonly);

        if (!bValid || bReadonly)
            // don't store anything if the data we're working with is invalid or readonly
            return sal_True;

        /////////////////////////////////////////////////////////////////////////
        // create the output string which contains all the table names
        if ( m_xCurrentConnection.is() )
        {   // collect the table filter data only if we have a connection - else no tables are displayed at all
            Sequence< ::rtl::OUString > aTableFilter;
            if (m_aTablesList.isWildcardChecked(m_aTablesList.getAllObjectsEntry()))
            {
                aTableFilter.realloc(1);
                aTableFilter[0] = ::rtl::OUString("%", 1, RTL_TEXTENCODING_ASCII_US);
            }
            else
            {
                aTableFilter = collectDetailedSelection();
            }
            _rCoreAttrs.Put( OStringListItem(DSID_TABLEFILTER, aTableFilter) );
        }

        if (m_aSuppressVersionColumns.IsChecked() != m_aSuppressVersionColumns.GetSavedValue())
            _rCoreAttrs.Put( SfxBoolItem(DSID_SUPPRESSVERSIONCL, !m_aSuppressVersionColumns.IsChecked()) );

        return sal_True;
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::_elementInserted( const ContainerEvent& _rEvent ) throw(RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        ::rtl::OUString sName;
        _rEvent.Accessor >>= sName;
        DBG_ASSERT( 0 != sName.getLength(), "OTableSubscriptionPage::_elementInserted: invalid accessor!" );

        m_aTablesList.addedTable( m_xCurrentConnection, sName, _rEvent.Element );

        // update the checks from the table filter set on the data source
        try
        {
            Reference< XPropertySet > xDS = m_pAdminDialog->getCurrentDataSource();
            if ( xDS.is() )
            {
                Sequence< ::rtl::OUString > aTableFilter;
                xDS->getPropertyValue( PROPERTY_TABLEFILTER ) >>= aTableFilter;
                implCompleteTablesCheck( aTableFilter );
            }
        }
        catch( const Exception& )
        {
        }

        // update the check states
        m_aTablesList.CheckButtons();
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::_elementRemoved( const ContainerEvent& _rEvent ) throw(RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        ::rtl::OUString sName;
        _rEvent.Accessor >>= sName;
        DBG_ASSERT( 0 != sName.getLength(), "OTableSubscriptionPage::_elementRemoved: invalid accessor!" );

        m_aTablesList.removedTable( m_xCurrentConnection, sName );

        m_aTablesList.CheckButtons();
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::_elementReplaced( const ContainerEvent& _rEvent ) throw(RuntimeException)
    {
        DBG_ERROR( "OTableSubscriptionPage::_elementReplaced: not implemented!" );
    }

    //------------------------------------------------------------------------
    void OTableSubscriptionPage::_disposing(const EventObject& _rSource) throw( RuntimeException)
    {
        Reference< XContainer > xSource( _rSource.Source, UNO_QUERY );

        // look for the notifier which caused this
        for (   AdapterArrayIterator aSearch = m_aNotifiers.begin();
                aSearch != m_aNotifiers.end();
                ++aSearch
            )
        {
            if  (   *aSearch
                &&  ( *aSearch )->getContainer().get() == xSource.get()
                )
            {
                ( *aSearch )->release();
                m_aNotifiers.erase( aSearch );
                break;
            }
        }
        // not interested in
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................

