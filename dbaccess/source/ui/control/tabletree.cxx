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

#include "tabletree.hxx"
#include "imageprovider.hxx"
#include "moduledbu.hxx"
#include "dbu_control.hrc"
#include <vcl/layout.hxx>
#include <vcl/menu.hxx>
#include <vcl/builderfactory.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/types.hxx>
#include "dbustrings.hrc"
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
#include <com/sun/star/sdb/application/DatabaseObjectContainer.hpp>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "commontypes.hxx"
#include "listviewitems.hxx"
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <connectivity/dbmetadata.hxx>
#include "svtools/treelistentry.hxx"

#include <algorithm>

namespace dbaui
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb::application;

using namespace ::dbtools;
using namespace ::comphelper;

namespace DatabaseObject = ::com::sun::star::sdb::application::DatabaseObject;
namespace DatabaseObjectContainer = ::com::sun::star::sdb::application::DatabaseObjectContainer;

// OTableTreeListBox
OTableTreeListBox::OTableTreeListBox(vcl::Window* pParent, WinBits nWinStyle)
    :OMarkableTreeListBox(pParent, nWinStyle)
    ,m_xImageProvider( new ImageProvider )
    ,m_bVirtualRoot(false)
    ,m_bNoEmptyFolders( false )
{
    implSetDefaultImages();
}

VCL_BUILDER_DECL_FACTORY(OTableTreeListBox)
{
    WinBits nWinStyle = 0;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    rRet = VclPtr<OTableTreeListBox>::Create(pParent, nWinStyle);
}

void OTableTreeListBox::implSetDefaultImages()
{
    ImageProvider aImageProvider;
    SetDefaultExpandedEntryBmp(  ImageProvider::getFolderImage( DatabaseObject::TABLE ) );
    SetDefaultCollapsedEntryBmp( ImageProvider::getFolderImage( DatabaseObject::TABLE ) );
}

bool  OTableTreeListBox::isFolderEntry( const SvTreeListEntry* _pEntry )
{
    sal_Int32 nEntryType = reinterpret_cast< sal_IntPtr >( _pEntry->GetUserData() );
    if  (   ( nEntryType == DatabaseObjectContainer::TABLES )
        ||  ( nEntryType == DatabaseObjectContainer::CATALOG )
        ||  ( nEntryType == DatabaseObjectContainer::SCHEMA )
        )
        return true;
    return false;
}

void OTableTreeListBox::notifyHiContrastChanged()
{
    implSetDefaultImages();

    SvTreeListEntry* pEntryLoop = First();
    while (pEntryLoop)
    {
        size_t nCount = pEntryLoop->ItemCount();
        for (size_t i=0;i<nCount;++i)
        {
            SvLBoxItem& rItem = pEntryLoop->GetItem(i);
            if (rItem.GetType() == SV_ITEM_ID_LBOXCONTEXTBMP)
            {
                SvLBoxContextBmp& rContextBitmapItem = static_cast< SvLBoxContextBmp& >( rItem );

                Image aImage;
                if ( isFolderEntry( pEntryLoop ) )
                {
                    aImage = ImageProvider::getFolderImage( DatabaseObject::TABLE );
                }
                else
                {
                    OUString sCompleteName( getQualifiedTableName( pEntryLoop ) );
                    m_xImageProvider->getImages( sCompleteName, DatabaseObject::TABLE, aImage );
                }

                rContextBitmapItem.SetBitmap1( aImage );
                rContextBitmapItem.SetBitmap2( aImage );
                break;
            }
        }
        pEntryLoop = Next(pEntryLoop);
    }
}

void OTableTreeListBox::implOnNewConnection( const Reference< XConnection >& _rxConnection )
{
    m_xConnection = _rxConnection;
    m_xImageProvider.reset( new ImageProvider( m_xConnection  ) );
}

void OTableTreeListBox::UpdateTableList( const Reference< XConnection >& _rxConnection ) throw(SQLException, std::exception)
{
    Sequence< OUString > sTables, sViews;

    OUString sCurrentActionError;
    try
    {
        Reference< XTablesSupplier > xTableSupp( _rxConnection, UNO_QUERY_THROW );
        sCurrentActionError = ModuleRes(STR_NOTABLEINFO);

        Reference< XNameAccess > xTables,xViews;

        Reference< XViewsSupplier > xViewSupp( _rxConnection, UNO_QUERY );
        if ( xViewSupp.is() )
        {
            xViews = xViewSupp->getViews();
            if (xViews.is())
                sViews = xViews->getElementNames();
        }

        xTables = xTableSupp->getTables();
        if (xTables.is())
            sTables = xTables->getElementNames();
    }
    catch(RuntimeException&)
    {
        OSL_FAIL("OTableTreeListBox::UpdateTableList : caught an RuntimeException!");
    }
    catch ( const SQLException& )
    {
        throw;
    }
    catch(Exception&)
    {
        // a non-SQLException exception occurred ... simply throw an SQLException
        SQLException aInfo;
        aInfo.Message = sCurrentActionError;
        throw aInfo;
    }

    UpdateTableList( _rxConnection, sTables, sViews );
}

namespace
{
    struct OViewSetter : public ::std::unary_function< OTableTreeListBox::TNames::value_type, bool>
    {
        const Sequence< OUString> m_aViews;
        ::comphelper::UStringMixEqual m_aEqualFunctor;

        OViewSetter(const Sequence< OUString>& _rViews,bool _bCase) : m_aViews(_rViews),m_aEqualFunctor(_bCase){}
        OTableTreeListBox::TNames::value_type operator() (const OUString& lhs)
        {
            OTableTreeListBox::TNames::value_type aRet;
            aRet.first = lhs;
            const OUString* pIter = m_aViews.getConstArray();
            const OUString* pEnd = m_aViews.getConstArray() + m_aViews.getLength();
            aRet.second = ::std::any_of(pIter,pEnd,::std::bind2nd(m_aEqualFunctor,lhs));

            return aRet;
        }
    };

}

void OTableTreeListBox::UpdateTableList(
                const Reference< XConnection >& _rxConnection,
                const Sequence< OUString>& _rTables,
                const Sequence< OUString>& _rViews
            )
{
    TNames aTables;
    aTables.resize(_rTables.getLength());
    const OUString* pIter = _rTables.getConstArray();
    const OUString* pEnd = _rTables.getConstArray() + _rTables.getLength();
    try
    {
        Reference< XDatabaseMetaData > xMeta( _rxConnection->getMetaData(), UNO_QUERY_THROW );
        ::std::transform( pIter, pEnd,
            aTables.begin(), OViewSetter( _rViews, xMeta->supportsMixedCaseQuotedIdentifiers() ) );
    }
    catch(Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    UpdateTableList( _rxConnection, aTables );
}

namespace
{
    ::std::vector< OUString > lcl_getMetaDataStrings_throw( const Reference< XResultSet >& _rxMetaDataResult, sal_Int32 _nColumnIndex )
    {
        ::std::vector< OUString > aStrings;
        Reference< XRow > xRow( _rxMetaDataResult, UNO_QUERY_THROW );
        while ( _rxMetaDataResult->next() )
            aStrings.push_back( xRow->getString( _nColumnIndex ) );
        return aStrings;
    }

    bool lcl_shouldDisplayEmptySchemasAndCatalogs( const Reference< XConnection >& _rxConnection )
    {
        ::dbtools::DatabaseMetaData aMetaData( _rxConnection );
        return aMetaData.displayEmptyTableFolders();
    }
}

void OTableTreeListBox::UpdateTableList( const Reference< XConnection >& _rxConnection, const TNames& _rTables )
{
    implOnNewConnection( _rxConnection );

    // throw away all the old stuff
    Clear();

    try
    {
        if (haveVirtualRoot())
        {
            OUString sRootEntryText;
            if ( ::std::none_of(_rTables.begin(),_rTables.end(),
                                [] (const TNames::value_type& name) { return !name.second; }) )
                sRootEntryText  = ModuleRes(STR_ALL_TABLES);
            else if ( ::std::none_of(_rTables.begin(),_rTables.end(),
                                     [] (const TNames::value_type& name) { return name.second; }) )
                sRootEntryText  = ModuleRes(STR_ALL_VIEWS);
            else
                sRootEntryText  = ModuleRes(STR_ALL_TABLES_AND_VIEWS);
            InsertEntry( sRootEntryText, nullptr, false, TREELIST_APPEND, reinterpret_cast< void* >( DatabaseObjectContainer::TABLES ) );
        }

        if ( _rTables.empty() )
            // nothing to do (besides inserting the root entry)
            return;

        // get the table/view names
        TNames::const_iterator aIter = _rTables.begin();
        TNames::const_iterator aEnd = _rTables.end();

        Reference< XDatabaseMetaData > xMeta( _rxConnection->getMetaData(), UNO_QUERY_THROW );
        for ( ; aIter != aEnd; ++aIter )
        {
            // add the entry
            implAddEntry(
                xMeta,
                aIter->first,
                false
            );
        }

        if ( !m_bNoEmptyFolders && lcl_shouldDisplayEmptySchemasAndCatalogs( _rxConnection ) )
        {
            bool bSupportsCatalogs = xMeta->supportsCatalogsInDataManipulation();
            bool bSupportsSchemas = xMeta->supportsSchemasInDataManipulation();

            if ( bSupportsCatalogs || bSupportsSchemas )
            {
                // we display empty catalogs if the DB supports catalogs, and they're noted at the beginning of a
                // composed name. Otherwise, we display empty schematas. (also see the tree structure explained in
                // implAddEntry)
                bool bCatalogs = bSupportsCatalogs && xMeta->isCatalogAtStart();

                ::std::vector< OUString > aFolderNames( lcl_getMetaDataStrings_throw(
                    bCatalogs ? xMeta->getCatalogs() : xMeta->getSchemas(), 1 ) );
                sal_Int32 nFolderType = bCatalogs ? DatabaseObjectContainer::CATALOG : DatabaseObjectContainer::SCHEMA;

                SvTreeListEntry* pRootEntry = getAllObjectsEntry();
                for (   ::std::vector< OUString >::const_iterator folder = aFolderNames.begin();
                        folder != aFolderNames.end();
                        ++folder
                    )
                {
                    SvTreeListEntry* pFolder = GetEntryPosByName( *folder, pRootEntry );
                    if ( !pFolder )
                        pFolder = InsertEntry( *folder, pRootEntry, false, TREELIST_APPEND, reinterpret_cast< void* >( nFolderType ) );
                }
            }
        }
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

bool OTableTreeListBox::isWildcardChecked(SvTreeListEntry* _pEntry)
{
    if (_pEntry)
    {
        OBoldListboxString* pTextItem = static_cast<OBoldListboxString*>(_pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING));
        if (pTextItem)
            return pTextItem->isEmphasized();
    }
    return false;
}

void OTableTreeListBox::checkWildcard(SvTreeListEntry* _pEntry)
{
    SetCheckButtonState(_pEntry, SV_BUTTON_CHECKED);
    checkedButton_noBroadcast(_pEntry);
}

SvTreeListEntry* OTableTreeListBox::getAllObjectsEntry() const
{
    return haveVirtualRoot() ? First() : nullptr;
}

void OTableTreeListBox::checkedButton_noBroadcast(SvTreeListEntry* _pEntry)
{
    OMarkableTreeListBox::checkedButton_noBroadcast(_pEntry);

    // if an entry has children, it makes a difference if the entry is checked
    // because all children are checked or if the user checked it explicitly.
    // So we track explicit (un)checking

    SvButtonState eState = GetCheckButtonState(_pEntry);
    OSL_ENSURE(SV_BUTTON_TRISTATE != eState, "OTableTreeListBox::CheckButtonHdl: user action which lead to TRISTATE?");
    implEmphasize(_pEntry, SV_BUTTON_CHECKED == eState);
}

void OTableTreeListBox::implEmphasize(SvTreeListEntry* _pEntry, bool _bChecked, bool _bUpdateDescendants, bool _bUpdateAncestors)
{
    OSL_ENSURE(_pEntry, "OTableTreeListBox::implEmphasize: invalid entry (NULL)!");

    // special emphasizing handling for the "all objects" entry
    bool bAllObjectsEntryAffected = haveVirtualRoot() && (getAllObjectsEntry() == _pEntry);
    if  (   GetModel()->HasChildren(_pEntry)              // the entry has children
        ||  bAllObjectsEntryAffected                    // or it is the "all objects" entry
        )
    {
        OBoldListboxString* pTextItem = static_cast<OBoldListboxString*>(_pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING));
        if (pTextItem)
            pTextItem->emphasize(_bChecked);

        if (bAllObjectsEntryAffected)
            InvalidateEntry(_pEntry);
    }

    if (_bUpdateDescendants)
    {
        // remove the mark for all children of the checked entry
        SvTreeListEntry* pChildLoop = FirstChild(_pEntry);
        while (pChildLoop)
        {
            if (GetModel()->HasChildren(pChildLoop))
                implEmphasize(pChildLoop, false, true, false);
            pChildLoop = NextSibling(pChildLoop);
        }
    }

    if (_bUpdateAncestors)
    {
        // remove the mark for all ancestors of the entry
        if (GetModel()->HasParent(_pEntry))
            implEmphasize(GetParent(_pEntry), false, false);
    }
}

void OTableTreeListBox::InitEntry(SvTreeListEntry* _pEntry, const OUString& _rString, const Image& _rCollapsedBitmap, const Image& _rExpandedBitmap, SvLBoxButtonKind _eButtonKind)
{
    OMarkableTreeListBox::InitEntry(_pEntry, _rString, _rCollapsedBitmap, _rExpandedBitmap, _eButtonKind);

    // replace the text item with our own one
    SvLBoxItem* pTextItem = _pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING);
    OSL_ENSURE(pTextItem, "OTableTreeListBox::InitEntry: no text item!?");
    size_t nTextPos = _pEntry->GetPos(pTextItem);
    OSL_ENSURE(SvTreeListEntry::ITEM_NOT_FOUND != nTextPos, "OTableTreeListBox::InitEntry: no text item pos!");

    _pEntry->ReplaceItem(std::unique_ptr<OBoldListboxString>(
                new OBoldListboxString(_pEntry, 0, _rString)), nTextPos);
}

SvTreeListEntry* OTableTreeListBox::implAddEntry(
        const Reference< XDatabaseMetaData >& _rxMeta,
        const OUString& _rTableName,
        bool _bCheckName
    )
{
    OSL_PRECOND( _rxMeta.is(), "OTableTreeListBox::implAddEntry: invalid meta data!" );
    if ( !_rxMeta.is() )
        return nullptr;

    // split the complete name into its components
    OUString sCatalog, sSchema, sName;
    qualifiedNameComponents( _rxMeta, _rTableName, sCatalog, sSchema, sName, ::dbtools::eInDataManipulation );

    SvTreeListEntry* pParentEntry = getAllObjectsEntry();

    // if the DB uses catalog at the start of identifiers, then our hierarchy is
    //   catalog
    //   +- schema
    //      +- table
    // else it is
    //   schema
    //   +- catalog
    //      +- table
    bool bCatalogAtStart = _rxMeta->isCatalogAtStart();
    const OUString& rFirstName  = bCatalogAtStart ? sCatalog : sSchema;
    const sal_Int32 nFirstFolderType   = bCatalogAtStart ? DatabaseObjectContainer::CATALOG : DatabaseObjectContainer::SCHEMA;
    const OUString& rSecondName = bCatalogAtStart ? sSchema : sCatalog;
    const sal_Int32 nSecondFolderType  = bCatalogAtStart ? DatabaseObjectContainer::SCHEMA : DatabaseObjectContainer::CATALOG;

    if ( !rFirstName.isEmpty() )
    {
        SvTreeListEntry* pFolder = GetEntryPosByName( rFirstName, pParentEntry );
        if ( !pFolder )
            pFolder = InsertEntry( rFirstName, pParentEntry, false, TREELIST_APPEND, reinterpret_cast< void* >( nFirstFolderType ) );
        pParentEntry = pFolder;
    }

    if ( !rSecondName.isEmpty() )
    {
        SvTreeListEntry* pFolder = GetEntryPosByName( rSecondName, pParentEntry );
        if ( !pFolder )
            pFolder = InsertEntry( rSecondName, pParentEntry, false, TREELIST_APPEND, reinterpret_cast< void* >( nSecondFolderType ) );
        pParentEntry = pFolder;
    }

    SvTreeListEntry* pRet = nullptr;
    if ( !_bCheckName || !GetEntryPosByName( sName, pParentEntry ) )
    {
        pRet = InsertEntry( sName, pParentEntry );

        Image aImage;
        m_xImageProvider->getImages( _rTableName, DatabaseObject::TABLE, aImage );

        SetExpandedEntryBmp( pRet, aImage );
        SetCollapsedEntryBmp( pRet, aImage );
    }
    return pRet;
}

NamedDatabaseObject OTableTreeListBox::describeObject( SvTreeListEntry* _pEntry )
{
    NamedDatabaseObject aObject;

    sal_Int32 nEntryType = reinterpret_cast< sal_IntPtr >( _pEntry->GetUserData() );

    if  ( nEntryType == DatabaseObjectContainer::TABLES )
    {
        aObject.Type = DatabaseObjectContainer::TABLES;
    }
    else if (   ( nEntryType == DatabaseObjectContainer::CATALOG )
            ||  ( nEntryType == DatabaseObjectContainer::SCHEMA )
            )
    {
        SvTreeListEntry* pParent = GetParent( _pEntry );
        sal_Int32 nParentEntryType = pParent ? reinterpret_cast< sal_IntPtr >( pParent->GetUserData() ) : -1;

        OUStringBuffer buffer;
        if  ( nEntryType == DatabaseObjectContainer::CATALOG )
        {
            if ( nParentEntryType == DatabaseObjectContainer::SCHEMA )
            {
                buffer.append( GetEntryText( pParent ) );
                buffer.append( '.' );
            }
            buffer.append( GetEntryText( _pEntry ) );
        }
        else if ( nEntryType == DatabaseObjectContainer::SCHEMA )
        {
            if ( nParentEntryType == DatabaseObjectContainer::CATALOG )
            {
                buffer.append( GetEntryText( pParent ) );
                buffer.append( '.' );
            }
            buffer.append( GetEntryText( _pEntry ) );
        }
    }
    else
    {
        aObject.Type = DatabaseObject::TABLE;
        aObject.Name = getQualifiedTableName( _pEntry );
    }

    return aObject;
}

SvTreeListEntry* OTableTreeListBox::addedTable( const OUString& _rName )
{
    try
    {
        Reference< XDatabaseMetaData > xMeta;
        if ( impl_getAndAssertMetaData( xMeta ) )
            return implAddEntry( xMeta, _rName );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return nullptr;
}

bool OTableTreeListBox::impl_getAndAssertMetaData( Reference< XDatabaseMetaData >& _out_rMetaData ) const
{
    if ( m_xConnection.is() )
        _out_rMetaData = m_xConnection->getMetaData();
    OSL_PRECOND( _out_rMetaData.is(), "OTableTreeListBox::impl_getAndAssertMetaData: invalid current connection!" );
    return _out_rMetaData.is();
}

OUString OTableTreeListBox::getQualifiedTableName( SvTreeListEntry* _pEntry ) const
{
    OSL_PRECOND( !isFolderEntry( _pEntry ), "OTableTreeListBox::getQualifiedTableName: folder entries not allowed here!" );

    try
    {
        Reference< XDatabaseMetaData > xMeta;
        if ( !impl_getAndAssertMetaData( xMeta ) )
            return OUString();

        OUString sCatalog;
        OUString sSchema;
        OUString sTable;

        SvTreeListEntry* pSchema = GetParent( _pEntry );
        if ( pSchema )
        {
            SvTreeListEntry* pCatalog = GetParent( pSchema );
            if  (   pCatalog
                ||  (   xMeta->supportsCatalogsInDataManipulation()
                    &&  !xMeta->supportsSchemasInDataManipulation()
                    )   // here we support catalog but no schema
                )
            {
                if ( pCatalog == nullptr )
                {
                    pCatalog = pSchema;
                    pSchema = nullptr;
                }
                sCatalog = GetEntryText( pCatalog );
            }
            if ( pSchema )
                sSchema = GetEntryText(pSchema);
        }
        sTable = GetEntryText( _pEntry );

        return ::dbtools::composeTableName( xMeta, sCatalog, sSchema, sTable, false, ::dbtools::eInDataManipulation );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return OUString();
}

SvTreeListEntry* OTableTreeListBox::getEntryByQualifiedName( const OUString& _rName )
{
    try
    {
        Reference< XDatabaseMetaData > xMeta;
        if ( !impl_getAndAssertMetaData( xMeta ) )
            return nullptr;

        // split the complete name into its components
        OUString sCatalog, sSchema, sName;
        qualifiedNameComponents(xMeta, _rName, sCatalog, sSchema, sName,::dbtools::eInDataManipulation);

        SvTreeListEntry* pParent = getAllObjectsEntry();
        SvTreeListEntry* pCat = nullptr;
        SvTreeListEntry* pSchema = nullptr;
        if ( !sCatalog.isEmpty() )
        {
            pCat = GetEntryPosByName(sCatalog, pParent);
            if ( pCat )
                pParent = pCat;
        }

        if ( !sSchema.isEmpty() )
        {
            pSchema = GetEntryPosByName(sSchema, pParent);
            if ( pSchema )
                pParent = pSchema;
        }

        return GetEntryPosByName(sName, pParent);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return nullptr;
}

void OTableTreeListBox::removedTable( const OUString& _rName )
{
    try
    {
        SvTreeListEntry* pEntry = getEntryByQualifiedName( _rName );
        if ( pEntry )
            GetModel()->Remove( pEntry );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
