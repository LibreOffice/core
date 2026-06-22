/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <config_features.h>
#include <core_resource.hxx>
#include "dsnItem.hxx"
#include "generalpage.hxx"
#include <IItemSetHelper.hxx>
#include <strings.hrc>
#include <dsitems.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfilt.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svl/stritem.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <UITools.hxx>
#include <officecfg/Office/Common.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/confignode.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

namespace dbaui
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;

    // OGeneralPage
    OGeneralPage::OGeneralPage(weld::Container* pPage, weld::DialogController* pController, const OUString& _rUIXMLDescription, const SfxItemSet& _rItems)
        : OGenericAdministrationPage(pPage, pController, _rUIXMLDescription, u"PageGeneral"_ustr, _rItems)
        , m_xSpecialMessage(m_xBuilder->weld_label(u"specialMessage"_ustr))
        , m_eLastMessage(smNone)
        , m_bInitTypeList(true)
        , m_xDatasourceType(m_xBuilder->weld_combo_box(u"datasourceType"_ustr))
        , m_pCollection(nullptr)
    {
        // extract the datasource type collection from the item set
        const DbuTypeCollectionItem* pCollectionItem = dynamic_cast<const DbuTypeCollectionItem*>( _rItems.GetItem(DSID_TYPECOLLECTION) );
        if (pCollectionItem)
            m_pCollection = pCollectionItem->getCollection();
        SAL_WARN_IF(!m_pCollection, "dbaccess.ui.generalpage", "OGeneralPage::OGeneralPage : really need a DSN type collection !");

        // do some knittings
        m_xDatasourceType->connect_changed(LINK(this, OGeneralPage, OnDatasourceTypeSelected));
    }

    OGeneralPage::~OGeneralPage()
    {
    }

    namespace
    {
        struct DisplayedType
        {
            OUString eType;
            OUString sDisplayName;

            DisplayedType( OUString _eType, OUString _sDisplayName ) : eType(std::move( _eType )), sDisplayName(std::move( _sDisplayName )) { }
        };
        typedef std::vector< DisplayedType > DisplayedTypes;

        struct DisplayedTypeLess
        {
            bool operator() ( const DisplayedType& _rLHS, const DisplayedType& _rRHS )
            {
                return _rLHS.eType < _rRHS.eType;
            }
        };
    }

    void OGeneralPage::initializeTypeList()
    {
        if ( !m_bInitTypeList )
            return;

        m_bInitTypeList = false;
        m_xDatasourceType->clear();

        if ( !m_pCollection )
            return;

        DisplayedTypes aDisplayedTypes;

        ::dbaccess::ODsnTypeCollection::TypeIterator aEnd = m_pCollection->end();
        for (   ::dbaccess::ODsnTypeCollection::TypeIterator aTypeLoop =  m_pCollection->begin();
                aTypeLoop != aEnd;
                ++aTypeLoop
            )
        {
            const OUString& sURLPrefix = aTypeLoop.getURLPrefix();
            if ( !sURLPrefix.isEmpty() )
            {
                // skip mysql connection variations. It is handled in another window.
                if(sURLPrefix.startsWith("sdbc:mysql:") && !sURLPrefix.startsWith("sdbc:mysql:jdbc:"))
                    continue;

                OUString sDisplayName = aTypeLoop.getDisplayName();
                if (m_xDatasourceType->find_text(sDisplayName) == -1 &&
                    approveDatasourceType(sURLPrefix, sDisplayName))
                {
                    aDisplayedTypes.emplace_back( sURLPrefix, sDisplayName );
                }
            }
        }
        std::sort( aDisplayedTypes.begin(), aDisplayedTypes.end(), DisplayedTypeLess() );
        for ( const auto& rDisplayedType : aDisplayedTypes )
            insertDatasourceTypeEntryData( rDisplayedType.eType, rDisplayedType.sDisplayName );
    }

    void OGeneralPage::setParentTitle(const OUString&)
    {
    }

    void OGeneralPage::switchMessage(std::u16string_view _sURLPrefix)
    {
        SPECIAL_MESSAGE eMessage = smNone;
        if ( _sURLPrefix.empty()/*_eType == m_eNotSupportedKnownType*/ )
        {
            eMessage = smUnsupportedType;
        }

        if ( eMessage != m_eLastMessage )
        {
            TranslateId pResId;
            if ( smUnsupportedType == eMessage )
                pResId = STR_UNSUPPORTED_DATASOURCE_TYPE;
            OUString sMessage;
            if ( pResId )
                sMessage = DBA_RES(pResId);

            m_xSpecialMessage->set_label( sMessage );
            m_eLastMessage = eMessage;
        }
    }

    void OGeneralPage::onTypeSelected(const OUString& _sURLPrefix)
    {
        // the new URL text as indicated by the selection history
        implSetCurrentType( _sURLPrefix );

        switchMessage(_sURLPrefix);

        m_aTypeSelectHandler.Call(*this);
    }

    void OGeneralPage::implInitControls( const SfxItemSet& _rSet, bool _bSaveValue )
    {
        initializeTypeList();

        m_xDatasourceType->set_active_text(getDatasourceName(_rSet));

        // notify our listener that our type selection has changed (if so)
        // FIXME: how to detect that it did not changed? (fdo#62937)
        setParentTitle( m_eCurrentSelection );
        onTypeSelected( m_eCurrentSelection );

        // a special message for the current page state
        switchMessage( m_eCurrentSelection );

        OGenericAdministrationPage::implInitControls( _rSet, _bSaveValue );
    }

    OUString OGeneralPage::getDatasourceName( const SfxItemSet& _rSet )
    {
        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags( _rSet, bValid, bReadonly );

        // if the selection is invalid, disable everything
        OUString sConnectURL;
        if ( bValid )
        {
            // collect some items and some values
            const SfxStringItem* pUrlItem = _rSet.GetItem<SfxStringItem>(DSID_CONNECTURL);
            assert( pUrlItem );
            sConnectURL = pUrlItem->GetValue();
        }

        implSetCurrentType(  OUString() );

        // compare the DSN prefix with the registered ones
        OUString sDisplayName;

        if (m_pCollection && bValid)
        {
            implSetCurrentType( m_pCollection->getPrefix( sConnectURL ) );
            sDisplayName = m_pCollection->getTypeDisplayName( m_eCurrentSelection );
        }

        // select the correct datasource type
        if  ( approveDatasourceType( m_eCurrentSelection, sDisplayName )
            &&  m_xDatasourceType->find_text(sDisplayName) == -1 )
        {   // this indicates it's really a type which is known in general, but not supported on the current platform
            // show a message saying so
            //  eSpecialMessage = smUnsupportedType;
            insertDatasourceTypeEntryData( m_eCurrentSelection, sDisplayName );
        }

        return sDisplayName;
    }

    // For the databaseWizard we only have one entry for the MySQL Database,
    // because we have a separate tabpage to retrieve the respective datasource type
    // ( ::dbaccess::DST_MYSQL_ODBC ||  ::dbaccess::DST_MYSQL_JDBC). Therefore we use  ::dbaccess::DST_MYSQL_JDBC as a temporary
    // representative for all MySQl databases)
    // Also, embedded databases (embedded HSQL, at the moment), are not to appear in the list of
    // databases to connect to.
    bool OGeneralPage::approveDatasourceType( std::u16string_view _sURLPrefix, OUString& _inout_rDisplayName )
    {
        return approveDatasourceType( m_pCollection->determineType(_sURLPrefix), _inout_rDisplayName );
    }

    bool OGeneralPage::approveDatasourceType( ::dbaccess::DATASOURCE_TYPE eType, OUString& _inout_rDisplayName )
    {
        if ( eType == ::dbaccess::DST_MYSQL_NATIVE_DIRECT )
        {
            // do not display the Connector/OOo driver itself, it is always wrapped via the MySQL-Driver, if
            // this driver is installed
            if ( m_pCollection->hasDriver( u"sdbc:mysql:mysqlc:" ) )
                _inout_rDisplayName.clear();
        }

        return _inout_rDisplayName.getLength() > 0;
    }

    void OGeneralPage::insertDatasourceTypeEntryData(const OUString& _sType, const OUString& sDisplayName)
    {
        // insert a (temporary) entry
        m_xDatasourceType->append_text(sDisplayName);
        m_aURLPrefixes.push_back(_sType);
    }

    void OGeneralPage::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xSpecialMessage.get()));
    }

    void OGeneralPage::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ComboBox>(m_xDatasourceType.get()));
    }

    void OGeneralPage::implSetCurrentType( const OUString& _eType )
    {
        if ( _eType == m_eCurrentSelection )
            return;

        m_eCurrentSelection = _eType;
    }

    void OGeneralPage::Reset(const SfxItemSet* _rCoreAttrs)
    {
        // reset all locale data
        implSetCurrentType(  OUString() );
            // this ensures that our type selection link will be called, even if the new one is the same as the
            // current one
        OGenericAdministrationPage::Reset(_rCoreAttrs);
    }

    IMPL_LINK( OGeneralPage, OnDatasourceTypeSelected, weld::ComboBox&, _rBox, void )
    {
        // get the type from the entry data
        const sal_Int32 nSelected = _rBox.get_active();
        if (nSelected == -1)
            return;
        if (o3tl::make_unsigned(nSelected) >= m_aURLPrefixes.size() )
        {
            SAL_WARN("dbaccess.ui.generalpage", "Got out-of-range value '" << nSelected <<  "' from the DatasourceType selection ListBox's GetSelectedEntryPos(): no corresponding URL prefix");
            return;
        }
        const OUString sURLPrefix = m_aURLPrefixes[ nSelected ];

        setParentTitle( sURLPrefix );
        // let the impl method do all the stuff
        onTypeSelected( sURLPrefix );
        // tell the listener we were modified
        callModifiedHdl();
    }

    // OGeneralPageDialog
    OGeneralPageDialog::OGeneralPageDialog(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rItems)
        : OGeneralPage(pPage, pController, u"dbaccess/ui/generalpagedialog.ui"_ustr, _rItems)
    {
    }

    void OGeneralPageDialog::setParentTitle( const OUString& _sURLPrefix )
    {
        const OUString sName = m_pCollection->getTypeDisplayName( _sURLPrefix );
        if ( m_pAdminDialog )
        {
            OUString sMessage = DBA_RES(STR_PARENTTITLE_GENERAL);
            m_pAdminDialog->setTitle( sMessage.replaceAll( "#", sName ) );
        }
    }

    void OGeneralPageDialog::implInitControls( const SfxItemSet& _rSet, bool _bSaveValue )
    {
        OGeneralPage::implInitControls( _rSet, _bSaveValue );

        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly );

        m_xDatasourceType->set_sensitive( bValid );
    }

    bool OGeneralPageDialog::FillItemSet( SfxItemSet* _rCoreAttrs )
    {
        bool bChangedSomething = false;

        const sal_Int32 nEntry = m_xDatasourceType->get_active();
        OUString sURLPrefix = m_aURLPrefixes[ nEntry ];

        if (m_xDatasourceType->get_value_changed_from_saved())
        {
            _rCoreAttrs->Put( SfxStringItem( DSID_CONNECTURL, sURLPrefix ) );
            bChangedSomething = true;
        }

        return bChangedSomething;
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
