/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "tablespage.hxx"
#include "dbu_dlg.hrc"
#include "dbadmin.hrc"
#include "dsitems.hxx"
#include "browserids.hxx"
#include "datasourceconnector.hxx"
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include "stringlistitem.hxx"
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include "dbustrings.hrc"
#include <vcl/svapp.hxx>
#include <vcl/waitobj.hxx>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include "sqlmessage.hxx"
#include <vcl/msgbox.hxx>
#include "dbaccess_helpid.hrc"
#include "UITools.hxx"
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <svtools/imgdef.hxx>
#include "TablesSingleDlg.hxx"
#include <tools/diagnose_ex.h>
#include <cppuhelper/exc_hlp.hxx>
#include "AutoControls.hrc"

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
    using namespace ::com::sun::star::util;
    using namespace ::dbtools;
    using namespace ::comphelper;

    //========================================================================
    //= OTableSubscriptionPage
    //========================================================================
DBG_NAME(OTableSubscriptionPage)
//------------------------------------------------------------------------
    OTableSubscriptionPage::OTableSubscriptionPage( Window* pParent, const SfxItemSet& _rCoreAttrs,OTableSubscriptionDialog* _pTablesDlg )
        :OGenericAdministrationPage( pParent, ModuleRes(PAGE_TABLESUBSCRIPTION), _rCoreAttrs )
        ,m_aTables              (this, ModuleRes(FL_SEPARATOR1))
        ,m_aTablesList          (this, NULL,ModuleRes(CTL_TABLESUBSCRIPTION),sal_True)
        ,m_aExplanation         (this, ModuleRes(FT_FILTER_EXPLANATION))
        ,m_bCatalogAtStart      ( sal_True )
        ,m_pTablesDlg(_pTablesDlg)
    {
        DBG_CTOR(OTableSubscriptionPage,NULL);

        m_aTablesList.SetCheckHandler(getControlModifiedLink());

        // initialize the TabListBox
        m_aTablesList.SetSelectionMode( MULTIPLE_SELECTION );
        m_aTablesList.SetDragDropMode( 0 );
        m_aTablesList.EnableInplaceEditing( sal_False );
        m_aTablesList.SetStyle(m_aTablesList.GetStyle() | WB_BORDER | WB_HASLINES | WB_HASLINESATROOT | WB_SORT | WB_HASBUTTONS | WB_HSCROLL |WB_HASBUTTONSATROOT);

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

        DBG_DTOR(OTableSubscriptionPage,NULL);
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
            OSL_FAIL("OTableSubscriptionPage::implCheckTables : could not retrieve the current connection's meta data!");
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
            if (!(pCatalog || sCatalog.isEmpty()))
                // the table (resp. its catalog) refered in this filter entry does not exist anymore
                continue;

            if (bAllSchemas && pCatalog)
            {
                m_aTablesList.checkWildcard(pCatalog);
                continue;
            }

            // the schema entry
            SvLBoxEntry* pSchema = m_aTablesList.GetEntryPosByName(sSchema, (pCatalog ? pCatalog : pRootEntry));
            if (!(pSchema || sSchema.isEmpty()))
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
        OSL_ENSURE(pNameItem, "OTableSubscriptionPage::implInitControls: missing the name attribute!");
        String sDSName = pNameItem->GetValue();

        if (bValid && sDSName.Len() && !m_xCurrentConnection.is() )
        {   // get the current table list from the connection for the current settings

            // the PropertyValues for the current dialog settings
            Sequence< PropertyValue > aConnectionParams;
            OSL_ENSURE(m_pTablesDlg, "OTableSubscriptionPage::implInitControls: need a parent dialog doing the translation!");
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
                catch(const Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
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
                Reference<XPropertySet> xProp = m_pTablesDlg->getCurrentDataSource();
                OSL_ENSURE(xProp.is(),"No data source set!");
                if ( xProp.is() )
                {
                    Any aTableFilter = xProp->getPropertyValue(PROPERTY_TABLEFILTER);
                    Any aTableTypeFilter = xProp->getPropertyValue(PROPERTY_TABLETYPEFILTER);

                    Reference<XModifiable> xModi(getDataSourceOrModel(xProp),UNO_QUERY);
                    sal_Bool bModified = ( xModi.is() && xModi->isModified() );

                    Sequence< ::rtl::OUString > aNewTableFilter(1);
                    aNewTableFilter[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%"));
                    xProp->setPropertyValue(PROPERTY_TABLEFILTER,makeAny(aNewTableFilter));

                    xProp->setPropertyValue( PROPERTY_TABLETYPEFILTER, makeAny( Sequence< ::rtl::OUString >() ) );
                    Reference< ::com::sun::star::lang::XEventListener> xEvt;
                    aErrorInfo = ::dbaui::createConnection(xProp,m_xORB,xEvt,m_xCurrentConnection);

                    xProp->setPropertyValue(PROPERTY_TABLEFILTER,aTableFilter);
                    xProp->setPropertyValue(PROPERTY_TABLETYPEFILTER,aTableTypeFilter);

                    if ( xModi.is() && !bModified )
                        xModi->setModified(sal_False);

                }

                if ( m_xCurrentConnection.is() )
                {
                    m_aTablesList.UpdateTableList( m_xCurrentConnection );
                    if (m_pTablesDlg)
                        m_pTablesDlg->successfullyConnected();
                }
            }
            catch (const SQLException&)
            {
                aErrorInfo = ::cppu::getCaughtException();
            }

            if (aErrorInfo.isValid())
            {
                // establishing the connection failed. Show an error window and exit.
                OSQLMessageBox aMessageBox( GetParentDialog(), aErrorInfo );
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
                catch(Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }

        bValid = bValid && m_xCurrentConnection.is();
        bReadonly = bReadonly || !bValid;

        // get the current table filter
        SFX_ITEMSET_GET(_rSet, pTableFilter, OStringListItem, DSID_TABLEFILTER, sal_True);
        Sequence< ::rtl::OUString > aTableFilter;
        if (pTableFilter)
            aTableFilter = pTableFilter->getList();

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

        return nResult;
    }
    //------------------------------------------------------------------------
    IMPL_LINK( OTableSubscriptionPage, OnTreeEntryChecked, Control*, _pControl )
    {
        return OnControlModified(_pControl);
    }
    //------------------------------------------------------------------------
    IMPL_LINK( OTableSubscriptionPage, OnTreeEntryCompare, const SvSortData*, _pSortData )
    {
        SvLBoxEntry* pLHS = static_cast<SvLBoxEntry*>(_pSortData->pLeft);
        SvLBoxEntry* pRHS = static_cast<SvLBoxEntry*>(_pSortData->pRight);
        OSL_ENSURE(pLHS && pRHS, "SbaTableQueryBrowser::OnTreeEntryCompare: invalid tree entries!");

        SvLBoxString* pLeftTextItem = static_cast<SvLBoxString*>(pLHS->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
        SvLBoxString* pRightTextItem = static_cast<SvLBoxString*>(pRHS->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
        OSL_ENSURE(pLeftTextItem && pRightTextItem, "SbaTableQueryBrowser::OnTreeEntryCompare: invalid text items!");

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

            if (m_aTablesList.GetCheckButtonState(pEntry) == SV_BUTTON_CHECKED && !m_aTablesList.GetModel()->HasChildren(pEntry))
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

        return sal_True;
    }

    // -----------------------------------------------------------------------
    void OTableSubscriptionPage::fillControls(::std::vector< ISaveValueWrapper* >& /*_rControlList*/)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
