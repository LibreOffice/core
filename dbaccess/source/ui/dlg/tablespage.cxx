/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <vcl/settings.hxx>
#include <vcl/waitobj.hxx>
#include <com/sun/star/i18n/Collator.hpp>
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
#include "svtools/treelistentry.hxx"
#include "TablesSingleDlg.hxx"
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include "AutoControls.hrc"

namespace dbaui
{

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

    
    OTableSubscriptionPage::OTableSubscriptionPage(Window* pParent, const SfxItemSet& _rCoreAttrs,
        OTableSubscriptionDialog* _pTablesDlg)
        : OGenericAdministrationPage(pParent, "TablesFilterPage",
            "dbaccess/ui/tablesfilterpage.ui", _rCoreAttrs)
        , m_bCatalogAtStart(true)
        , m_pTablesDlg(_pTablesDlg)
    {
        get(m_pTables, "TablesFilterPage");

        get(m_pTablesList, "treeview");
        m_pTablesList->set_width_request(56 * m_pTablesList->approximate_char_width());
        m_pTablesList->set_height_request(12 * m_pTablesList->GetTextHeight());

        m_pTablesList->SetCheckHandler(getControlModifiedLink());

        
        m_pTablesList->SetSelectionMode( MULTIPLE_SELECTION );
        m_pTablesList->SetDragDropMode( 0 );
        m_pTablesList->EnableInplaceEditing( false );
        m_pTablesList->SetStyle(m_pTablesList->GetStyle() | WB_BORDER | WB_HASLINES | WB_HASLINESATROOT | WB_SORT | WB_HASBUTTONS | WB_HSCROLL |WB_HASBUTTONSATROOT);

        m_pTablesList->Clear();

        m_pTablesList->SetCheckButtonHdl(LINK(this, OTableSubscriptionPage, OnTreeEntryChecked));
        m_pTablesList->SetCheckHandler(LINK(this, OTableSubscriptionPage, OnTreeEntryChecked));
    }

    OTableSubscriptionPage::~OTableSubscriptionPage()
    {
        
        try
        {
            ::comphelper::disposeComponent(m_xCurrentConnection);
        }
        catch (RuntimeException&) { }

    }

    void OTableSubscriptionPage::StateChanged( StateChangedType nType )
    {
        OGenericAdministrationPage::StateChanged( nType );

        if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        {
            
            m_pTablesList->notifyHiContrastChanged();
        }
    }
    void OTableSubscriptionPage::DataChanged( const DataChangedEvent& rDCEvt )
    {
        OGenericAdministrationPage::DataChanged( rDCEvt );

        if ((( rDCEvt.GetType() == DATACHANGED_SETTINGS )   ||
            ( rDCEvt.GetType() == DATACHANGED_DISPLAY   ))  &&
            ( rDCEvt.GetFlags() & SETTINGS_STYLE        ))
        {
            
            m_pTablesList->notifyHiContrastChanged();
        }
    }
    void OTableSubscriptionPage::resizeControls(const Size& _rDiff)
    {
        if ( _rDiff.Height() )
        {
            Size aOldSize = m_pTablesList->GetSizePixel();
            aOldSize.Height() -= _rDiff.Height();
            m_pTablesList->SetPosSizePixel(
                    m_pTablesList->GetPosPixel()+Point(0,_rDiff.Height()),
                    aOldSize
                    );
        }
    }
    void OTableSubscriptionPage::implCheckTables(const Sequence< OUString >& _rTables)
    {
        
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

        
        CheckAll(sal_False);

        
        OUString sCatalog, sSchema, sName;

        SvTreeListEntry* pRootEntry = m_pTablesList->getAllObjectsEntry();
        sal_Bool bAllTables = sal_False;
        sal_Bool bAllSchemas = sal_False;

        const OUString* pIncludeTable = _rTables.getConstArray();
        for (sal_Int32 i=0; i<_rTables.getLength(); ++i, ++pIncludeTable)
        {
            if (xMeta.is())
                qualifiedNameComponents(xMeta, pIncludeTable->getStr(), sCatalog, sSchema, sName,::dbtools::eInDataManipulation);
            else
                sName = pIncludeTable->getStr();

            bAllTables = (1 == sName.getLength()) && ('%' == sName[0]);
            bAllSchemas = (1 == sSchema.getLength()) && ('%' == sSchema[0]);

            
            SvTreeListEntry* pCatalog = m_pTablesList->GetEntryPosByName(sCatalog, pRootEntry);
            if (!(pCatalog || sCatalog.isEmpty()))
                
                continue;

            if (bAllSchemas && pCatalog)
            {
                m_pTablesList->checkWildcard(pCatalog);
                continue;
            }

            
            SvTreeListEntry* pSchema = m_pTablesList->GetEntryPosByName(sSchema, (pCatalog ? pCatalog : pRootEntry));
            if (!(pSchema || sSchema.isEmpty()))
                
                continue;

            if (bAllTables && pSchema)
            {
                m_pTablesList->checkWildcard(pSchema);
                continue;
            }

            SvTreeListEntry* pEntry = m_pTablesList->GetEntryPosByName(sName, pSchema ? pSchema : (pCatalog ? pCatalog : pRootEntry) );
            if (pEntry)
                m_pTablesList->SetCheckButtonState(pEntry, SV_BUTTON_CHECKED);
        }
        m_pTablesList->CheckButtons();
    }

    void OTableSubscriptionPage::implCompleteTablesCheck( const ::com::sun::star::uno::Sequence< OUString >& _rTableFilter )
    {
        if (!_rTableFilter.getLength())
        {   
            CheckAll(sal_False);
        }
        else
        {
            if ((1 == _rTableFilter.getLength()) && _rTableFilter[0] == "%")
            {   
                CheckAll(sal_True);
            }
            else
                implCheckTables( _rTableFilter );
        }
    }

    void OTableSubscriptionPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        
        SFX_ITEMSET_GET(_rSet, pNameItem, SfxStringItem, DSID_NAME, true);
        OSL_ENSURE(pNameItem, "OTableSubscriptionPage::implInitControls: missing the name attribute!");
        OUString sDSName = pNameItem->GetValue();

        if (bValid && !sDSName.isEmpty() && !m_xCurrentConnection.is() )
        {   

            
            Sequence< PropertyValue > aConnectionParams;
            OSL_ENSURE(m_pTablesDlg, "OTableSubscriptionPage::implInitControls: need a parent dialog doing the translation!");
            if ( m_pTablesDlg )
            {
                if (!m_pTablesDlg->getCurrentSettings(aConnectionParams))
                {
                    m_pTablesList->Clear();
                    m_pTablesDlg->endExecution();
                    return;
                }
            }

            if (!m_xCollator.is())
            {
                
                try
                {
                    m_xCollator = Collator::create(m_xORB);
                    m_xCollator->loadDefaultCollator(Application::GetSettings().GetLanguageTag().getLocale(), 0);
                }
                catch(const Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }

            
            SQLExceptionInfo aErrorInfo;
            
            OUString sURL;
            if ( m_pTablesDlg )
                sURL = m_pTablesDlg->getConnectionURL();

            try
            {
                WaitObject aWaitCursor(this);
                m_pTablesList->GetModel()->SetSortMode(SortAscending);
                m_pTablesList->GetModel()->SetCompareHdl(LINK(this, OTableSubscriptionPage, OnTreeEntryCompare));

                Reference< XDriver > xDriver;
                Reference<XPropertySet> xProp = m_pTablesDlg->getCurrentDataSource();
                OSL_ENSURE(xProp.is(),"No data source set!");
                if ( xProp.is() )
                {
                    Any aTableFilter = xProp->getPropertyValue(PROPERTY_TABLEFILTER);
                    Any aTableTypeFilter = xProp->getPropertyValue(PROPERTY_TABLETYPEFILTER);

                    Reference<XModifiable> xModi(getDataSourceOrModel(xProp),UNO_QUERY);
                    sal_Bool bModified = ( xModi.is() && xModi->isModified() );

                    Sequence< OUString > aNewTableFilter(1);
                    aNewTableFilter[0] = "%";
                    xProp->setPropertyValue(PROPERTY_TABLEFILTER,makeAny(aNewTableFilter));

                    xProp->setPropertyValue( PROPERTY_TABLETYPEFILTER, makeAny( Sequence< OUString >() ) );
                    Reference< ::com::sun::star::lang::XEventListener> xEvt;
                    aErrorInfo = ::dbaui::createConnection(xProp, m_xORB, xEvt, m_xCurrentConnection);

                    xProp->setPropertyValue(PROPERTY_TABLEFILTER,aTableFilter);
                    xProp->setPropertyValue(PROPERTY_TABLETYPEFILTER,aTableTypeFilter);

                    if ( xModi.is() && !bModified )
                        xModi->setModified(sal_False);

                }

                if ( m_xCurrentConnection.is() )
                {
                    m_pTablesList->UpdateTableList( m_xCurrentConnection );
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
                
                OSQLMessageBox aMessageBox( GetParentDialog(), aErrorInfo );
                aMessageBox.Execute();
                m_pTables->Enable(false);
                m_pTablesList->Clear();

                if ( m_pTablesDlg )
                {
                    m_pTablesDlg->clearPassword();
                    m_pTablesDlg->endExecution();
                }
            }
            else
            {
                
                m_sCatalogSeparator = ".";    
                m_bCatalogAtStart = sal_True;   
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

        
        SFX_ITEMSET_GET(_rSet, pTableFilter, OStringListItem, DSID_TABLEFILTER, true);
        Sequence< OUString > aTableFilter;
        if (pTableFilter)
            aTableFilter = pTableFilter->getList();

        implCompleteTablesCheck( aTableFilter );

        
        SvTreeListEntry* pExpand = m_pTablesList->getAllObjectsEntry();
        while (pExpand)
        {
            m_pTablesList->Expand(pExpand);
            pExpand = m_pTablesList->FirstChild(pExpand);
            if (pExpand && m_pTablesList->NextSibling(pExpand))
                pExpand = NULL;
        }

        
        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
    }

    void OTableSubscriptionPage::CheckAll( sal_Bool _bCheck )
    {
        SvButtonState eState = _bCheck ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED;
        SvTreeListEntry* pEntry = m_pTablesList->First();
        while (pEntry)
        {
            m_pTablesList->SetCheckButtonState( pEntry, eState);
            pEntry = m_pTablesList->Next(pEntry);
        }

        if (_bCheck && m_pTablesList->getAllObjectsEntry())
            m_pTablesList->checkWildcard(m_pTablesList->getAllObjectsEntry());
    }

    int OTableSubscriptionPage::DeactivatePage(SfxItemSet* _pSet)
    {
        int nResult = OGenericAdministrationPage::DeactivatePage(_pSet);

        
        try
        {
            ::comphelper::disposeComponent(m_xCurrentConnection);
        }
        catch (RuntimeException&) { }

        return nResult;
    }
    IMPL_LINK( OTableSubscriptionPage, OnTreeEntryChecked, Control*, _pControl )
    {
        return OnControlModified(_pControl);
    }
    IMPL_LINK( OTableSubscriptionPage, OnTreeEntryCompare, const SvSortData*, _pSortData )
    {
        const SvTreeListEntry* pLHS = static_cast<const SvTreeListEntry*>(_pSortData->pLeft);
        const SvTreeListEntry* pRHS = static_cast<const SvTreeListEntry*>(_pSortData->pRight);
        OSL_ENSURE(pLHS && pRHS, "SbaTableQueryBrowser::OnTreeEntryCompare: invalid tree entries!");

        const SvLBoxString* pLeftTextItem = static_cast<const SvLBoxString*>(pLHS->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
        const SvLBoxString* pRightTextItem = static_cast<const SvLBoxString*>(pRHS->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
        OSL_ENSURE(pLeftTextItem && pRightTextItem, "SbaTableQueryBrowser::OnTreeEntryCompare: invalid text items!");

        OUString sLeftText = pLeftTextItem->GetText();
        OUString sRightText = pRightTextItem->GetText();

        sal_Int32 nCompareResult = 0;   

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
            
            nCompareResult = sLeftText.compareTo(sRightText);

        return nCompareResult;
    }

    Sequence< OUString > OTableSubscriptionPage::collectDetailedSelection() const
    {
        Sequence< OUString > aTableFilter;
        static const OUString sDot(".");
        static const OUString sWildcard("%");

        OUString sComposedName;
        const SvTreeListEntry* pAllObjectsEntry = m_pTablesList->getAllObjectsEntry();
        if (!pAllObjectsEntry)
            return aTableFilter;
        SvTreeListEntry* pEntry = m_pTablesList->GetModel()->Next(const_cast<SvTreeListEntry*>(pAllObjectsEntry));
        while(pEntry)
        {
            sal_Bool bCatalogWildcard = sal_False;
            sal_Bool bSchemaWildcard =  sal_False;
            SvTreeListEntry* pSchema = NULL;
            SvTreeListEntry* pCatalog = NULL;

            if (m_pTablesList->GetCheckButtonState(pEntry) == SV_BUTTON_CHECKED && !m_pTablesList->GetModel()->HasChildren(pEntry))
            {   
                OUString sCatalog;
                if(m_pTablesList->GetModel()->HasParent(pEntry))
                {
                    pSchema = m_pTablesList->GetModel()->GetParent(pEntry);
                    if (pAllObjectsEntry == pSchema)
                        
                        pSchema = NULL;

                    if (pSchema)
                    {   
                        if(m_pTablesList->GetModel()->HasParent(pSchema))
                        {
                            pCatalog = m_pTablesList->GetModel()->GetParent(pSchema);
                            if (pAllObjectsEntry == pCatalog)
                                
                                pCatalog = NULL;

                            if (pCatalog)
                            {   
                                bCatalogWildcard = m_pTablesList->isWildcardChecked(pCatalog);
                                if (m_bCatalogAtStart)
                                {
                                    sComposedName += m_pTablesList->GetEntryText( pCatalog );
                                    sComposedName += m_sCatalogSeparator;
                                    if (bCatalogWildcard)
                                        sComposedName += sWildcard;
                                }
                                else
                                {
                                    if (bCatalogWildcard)
                                        sCatalog = sWildcard;
                                    else
                                        sCatalog = "";
                                    sCatalog += m_sCatalogSeparator;
                                    sCatalog += m_pTablesList->GetEntryText( pCatalog );
                                }
                            }
                        }
                        bSchemaWildcard = m_pTablesList->isWildcardChecked(pSchema);
                        sComposedName += m_pTablesList->GetEntryText( pSchema );
                        sComposedName += sDot;
                    }

                    if (bSchemaWildcard)
                        sComposedName += sWildcard;
                }
                if (!bSchemaWildcard && !bCatalogWildcard)
                    sComposedName += m_pTablesList->GetEntryText( pEntry );

                if (!m_bCatalogAtStart && !bCatalogWildcard)
                    sComposedName += sCatalog;

                
                sal_Int32 nOldLen = aTableFilter.getLength();
                aTableFilter.realloc(nOldLen + 1);
                
                aTableFilter[nOldLen] = sComposedName;

                
                sComposedName = "";
            }

            if (bCatalogWildcard)
                pEntry = implNextSibling(pCatalog);
            else if (bSchemaWildcard)
                pEntry = implNextSibling(pSchema);
            else
                pEntry = m_pTablesList->GetModel()->Next(pEntry);
        }

        return aTableFilter;
    }

    SvTreeListEntry* OTableSubscriptionPage::implNextSibling(SvTreeListEntry* _pEntry) const
    {
        SvTreeListEntry* pReturn = NULL;
        if (_pEntry)
        {
            pReturn = m_pTablesList->NextSibling(_pEntry);
            if (!pReturn)
                pReturn = implNextSibling(m_pTablesList->GetParent(_pEntry));
        }
        return pReturn;
    }

    sal_Bool OTableSubscriptionPage::FillItemSet( SfxItemSet& _rCoreAttrs )
    {
        sal_Bool bValid, bReadonly;
        getFlags(_rCoreAttrs, bValid, bReadonly);

        if (!bValid || bReadonly)
            
            return sal_True;

        
        if ( m_xCurrentConnection.is() )
        {   
            Sequence< OUString > aTableFilter;
            if (m_pTablesList->isWildcardChecked(m_pTablesList->getAllObjectsEntry()))
            {
                aTableFilter.realloc(1);
                aTableFilter[0] = "%";
            }
            else
            {
                aTableFilter = collectDetailedSelection();
            }
            _rCoreAttrs.Put( OStringListItem(DSID_TABLEFILTER, aTableFilter) );
        }

        return sal_True;
    }

    void OTableSubscriptionPage::fillControls(::std::vector< ISaveValueWrapper* >& /*_rControlList*/)
    {
    }

    void OTableSubscriptionPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<VclContainer>(m_pTables));
    }
}   

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
