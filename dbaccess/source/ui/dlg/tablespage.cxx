/*************************************************************************
 *
 *  $RCSfile: tablespage.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:50:58 $
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
#ifndef _DBAUI_TABLESSINGLEDLG_HXX_
#include "TablesSingleDlg.hxx"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
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
    OTableSubscriptionPage::OTableSubscriptionPage( Window* pParent, const SfxItemSet& _rCoreAttrs,OTableSubscriptionDialog* _pTablesDlg )
        :OGenericAdministrationPage( pParent, ModuleRes(PAGE_TABLESUBSCRIPTION), _rCoreAttrs )
        ,OContainerListener( m_aNotifierMutex )
        ,m_aTables              (this, ResId(FL_SEPARATOR1))
        ,m_aTablesList          (this, NULL,ResId(CTL_TABLESUBSCRIPTION),sal_False)
        ,m_aExplanation         (this, ResId(FT_FILTER_EXPLANATION))
        ,m_bCheckedAll          ( sal_False )
        ,m_bCatalogAtStart      ( sal_True )
        ,m_pTablesDlg(_pTablesDlg)
    {
        m_aTablesList.SetCheckHandler(getControlModifiedLink());

        // initialize the TabListBox
        m_aTablesList.SetSelectionMode( MULTIPLE_SELECTION );
        m_aTablesList.SetDragDropMode( 0 );
        m_aTablesList.EnableInplaceEditing( sal_False );
        m_aTablesList.SetWindowBits(WB_BORDER | WB_HASLINES | WB_HASLINESATROOT | WB_SORT | WB_HASBUTTONS | WB_HSCROLL |WB_HASBUTTONSATROOT);

        m_aTablesList.Clear();

        FreeResource();

        m_aTablesList.SetCheckButtonHdl(LINK(this, OTableSubscriptionPage, OnTreeEntryChecked));
        m_aTablesList.SetCheckHandler(LINK(this, OTableSubscriptionPage, OnTreeEntryChecked));
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

        retireNotifiers();
    }

    // -----------------------------------------------------------------------------
    void OTableSubscriptionPage::StateChanged( StateChangedType nType )
    {
        OGenericAdministrationPage::StateChanged( nType );

        if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        {
            // Check if we need to get new images for normal/high contrast mode
            m_aTablesList.notifyHiContrastChanged();
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
            SvLBoxEntry* pSchema = m_aTablesList.GetEntryPosByName(sSchema, (pCatalog ? pCatalog : pRootEntry));
            if (!pSchema && sSchema.getLength())
                // the table (resp. its schema) refered in this filter entry does not exist anymore
                continue;

            if (bAllTables && pSchema)
            {
                m_aTablesList.checkWildcard(pSchema);
                continue;
            }

            SvLBoxEntry* pEntry = m_aTablesList.GetEntryPosByName(sName, pSchema ? pSchema : (pCatalog ? pCatalog : pRootEntry) );
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

        // get the name of the data source we're working for
        SFX_ITEMSET_GET(_rSet, pNameItem, SfxStringItem, DSID_NAME, sal_True);
        DBG_ASSERT(pNameItem, "OTableSubscriptionPage::ActivatePage: missing the name attribute!");
        String sDSName = pNameItem->GetValue();

        if (bValid && sDSName.Len() && !m_xCurrentConnection.is() )
        {   // get the current table list from the connection for the current settings

            // the PropertyValues for the current dialog settings
            Sequence< PropertyValue > aConnectionParams;
            DBG_ASSERT(m_pTablesDlg, "OTableSubscriptionPage::ActivatePage : need a parent dialog doing the translation!");
            if ( m_pTablesDlg )
            {
                if (!m_pTablesDlg->getCurrentSettings(aConnectionParams))
                {
                    m_aTablesList.Clear();
                    m_pTablesDlg->endExecution();
                    return;
                }
            }

            if (!m_xCollator.is())
            {
                // the collator for the string compares
                try
                {
                    m_xCollator = Reference< XCollator >(m_xORB->createInstance(SERVICE_I18N_COLLATOR), UNO_QUERY);
                    if (m_xCollator.is())
                        m_xCollator->loadDefaultCollator(Application::GetSettings().GetLocale(), 0);
                }
                catch(Exception&)
                {
                    OSL_ENSURE(0,"Exception catched!");
                }
            }

            // fill the table list with this connection information
            SQLExceptionInfo aErrorInfo;
            // the current DSN
            String sURL;
            if ( m_pTablesDlg )
                sURL = m_pTablesDlg->getConnectionURL();

            try
            {
                WaitObject aWaitCursor(this);
                m_aTablesList.GetModel()->SetSortMode(SortAscending);
                m_aTablesList.GetModel()->SetCompareHdl(LINK(this, OTableSubscriptionPage, OnTreeEntryCompare));

                Reference< XDriver > xDriver;
                m_aTablesList.setORB(m_xORB);
                m_xCurrentConnection = m_aTablesList.UpdateTableList( sURL, aConnectionParams, xDriver );
                if ( m_xCurrentConnection.is() )
                {
                    if (m_pTablesDlg)
                        m_pTablesDlg->successfullyConnected();
                }
            }
            catch (SQLContext& e) { aErrorInfo = SQLExceptionInfo(e); }
            catch (SQLWarning& e) { aErrorInfo = SQLExceptionInfo(e); }
            catch (SQLException& e) { aErrorInfo = SQLExceptionInfo(e); }

            if (aErrorInfo.isValid())
            {
                // establishing the connection failed. Show an error window and exit.
                OSQLMessageBox aMessageBox(GetParent()->GetParent(), aErrorInfo, WB_OK | WB_DEF_OK, OSQLMessageBox::Error);
                aMessageBox.Execute();
                m_aTables.Enable(sal_False);
                m_aTablesList.Enable(sal_False);
                m_aExplanation.Enable(sal_False);
                m_aTablesList.Clear();

                if ( m_pTablesDlg )
                {
                    m_pTablesDlg->clearPassword();
                    m_pTablesDlg->endExecution();
                }
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
                catch(Exception&)
                {
                    OSL_ENSURE(0,"Exception catched!");
                }
            }
        }

        bValid = bValid && m_xCurrentConnection.is();
        bReadonly = bReadonly || !bValid;

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
        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
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

        retireNotifiers();

        return nResult;
    }
    //------------------------------------------------------------------------
    IMPL_LINK( OTableSubscriptionPage, OnTreeEntryChecked, Control*, _pControl )
    {
        return OnControlModified(_pControl);
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
        actOnEntryPaths(pMySettings->aExpandedEntries, &OTableSubscriptionPage::doExpand);

        // select the entries
        m_aTablesList.SelectAll(sal_False);
        actOnEntryPaths(pMySettings->aSelectedEntries, &OTableSubscriptionPage::doSelect);

        SvLBoxEntry* pFocusEntry = getEntryFromPath(pMySettings->sFocusEntry);
        if (pFocusEntry)
            m_aTablesList.SetCurEntry(pFocusEntry);
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
    BOOL OTableSubscriptionPage::FillItemSet( SfxItemSet& _rCoreAttrs )
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
            Reference< XPropertySet > xDS = m_pTablesDlg->getCurrentDataSource();
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
    // -----------------------------------------------------------------------
    void OTableSubscriptionPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
    }
    // -----------------------------------------------------------------------
    void OTableSubscriptionPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<OTableTreeListBox>(&m_aTablesList));
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aTables));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aExplanation));
    }
    // -----------------------------------------------------------------------
//.........................................................................
}   // namespace dbaui
//.........................................................................

