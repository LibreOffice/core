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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TABLETREE_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TABLETREE_HXX

#include "imageprovider.hxx"
#include "marktree.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdb/application/NamedDatabaseObject.hpp>
#include <memory>

namespace dbaui
{

// OTableTreeListBox
class OTableTreeListBox : public OMarkableTreeListBox
{
protected:
    css::uno::Reference< css::sdbc::XConnection >
                    m_xConnection;      // the connection we're working for, set in implOnNewConnection, called by UpdateTableList
    std::unique_ptr< ImageProvider >
                    m_xImageProvider;   // provider for our images
    bool            m_bVirtualRoot;     // should the first entry be visible
    bool            m_bNoEmptyFolders;  // should empty catalogs/schematas be prevented from being displayed?

public:
    OTableTreeListBox(vcl::Window* pParent, WinBits nWinStyle);

    void init(bool bVirtualRoot) { m_bVirtualRoot = bVirtualRoot; }

    typedef ::std::pair< OUString, bool > TTableViewName;
    typedef ::std::vector< TTableViewName >         TNames;

    void    suppressEmptyFolders() { m_bNoEmptyFolders = true; }

    /** call when HiContrast change.
    */
    void notifyHiContrastChanged();

    /** determines whether the given entry denotes a tables folder
    */
    static bool isFolderEntry( const SvTreeListEntry* _pEntry );

    /** fill the table list with the tables belonging to the connection described by the parameters
        @param _rxConnection
            the connection, which must support the service com.sun.star.sdb.Connection
        @throws
            <type scope="css::sdbc">SQLException</type> if no connection could be created
    */
    void    UpdateTableList(
                const css::uno::Reference< css::sdbc::XConnection >& _rxConnection
            )   throw(css::sdbc::SQLException, std::exception);

    /** fill the table list with the tables and views determined by the two given containers.
        The views sequence is used to determine which table is of type view.
        @param      _rxConnection   the connection where you got the object names from. Must not be NULL.
                                    Used to split the full qualified names into it's parts.
        @param      _rTables        table/view sequence
        @param      _rViews         view sequence
    */
    void    UpdateTableList(
                const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
                const css::uno::Sequence< OUString>& _rTables,
                const css::uno::Sequence< OUString>& _rViews
            );

    /** returns a NamedDatabaseObject record which describes the given entry
    */
    css::sdb::application::NamedDatabaseObject
            describeObject( SvTreeListEntry* _pEntry );

    /** to be used if a foreign instance added a table
    */
    SvTreeListEntry* addedTable( const OUString& _rName );

    /** to be used if a foreign instance removed a table
    */
    void    removedTable( const OUString& _rName );

    /** returns the fully qualified name of a table entry
        @param _pEntry
            the entry whose name is to be obtained. Must not denote a folder entry.
    */
    OUString getQualifiedTableName( SvTreeListEntry* _pEntry ) const;

    SvTreeListEntry*    getEntryByQualifiedName( const OUString& _rName );

    SvTreeListEntry*    getAllObjectsEntry() const;

    /** does a wildcard check of the given entry
        <p>There are two different 'checked' states: If the user checks all children of an entry, this is different
        from checking the entry itself. The second is called 'wildcard' checking, 'cause in the resulting
        table filter it's represented by a wildcard.</p>
    */
    void            checkWildcard(SvTreeListEntry* _pEntry);

    /** determine if the given entry is 'wildcard checked'
        @see checkWildcard
    */
    static bool     isWildcardChecked(SvTreeListEntry* _pEntry);

protected:
    virtual void InitEntry(SvTreeListEntry* _pEntry, const OUString& _rString, const Image& _rCollapsedBitmap, const Image& _rExpandedBitmap, SvLBoxButtonKind _eButtonKind) SAL_OVERRIDE;

    virtual void checkedButton_noBroadcast(SvTreeListEntry* _pEntry) SAL_OVERRIDE;

    void implEmphasize(SvTreeListEntry* _pEntry, bool _bChecked, bool _bUpdateDescendants = true, bool _bUpdateAncestors = true);

    /** adds the given entry to our list
        @precond
            our image provider must already have been reset to the connection to which the meta data
            belong.
    */
    SvTreeListEntry* implAddEntry(
            const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _rxMeta,
            const OUString& _rTableName,
            bool _bCheckName = true
        );

    void    implSetDefaultImages();

    void    implOnNewConnection( const css::uno::Reference< css::sdbc::XConnection >& _rxConnection );

    bool    impl_getAndAssertMetaData( css::uno::Reference< css::sdbc::XDatabaseMetaData >& _out_rMetaData ) const;

    bool haveVirtualRoot() const { return m_bVirtualRoot; }

    /** fill the table list with the tables and views determined by the two given containers
        @param      _rxConnection   the connection where you got the object names from. Must not be NULL.
                                    Used to split the full qualified names into it's parts.
        @param      _rTables        table/view sequence, the second argument is <TRUE/> if it is a table, otherwise it is a view.
    */
    void    UpdateTableList(
                const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
                const TNames& _rTables
            );

};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TABLETREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
