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
#include "dbtreelistbox.hxx"

#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/application/NamedDatabaseObject.hpp>
#include <memory>

namespace dbaui
{

// OTableTreeListBox
class OTableTreeListBox : public TreeListBox
{
    css::uno::Reference< css::sdbc::XConnection >
                    m_xConnection;      // the connection we're working for, set in implOnNewConnection, called by UpdateTableList
    std::unique_ptr< ImageProvider >
                    m_xImageProvider;   // provider for our images
    bool            m_bVirtualRoot;     // should the first entry be visible
    bool            m_bNoEmptyFolders;  // should empty catalogs/schematas be prevented from being displayed?
    bool            m_bShowToggles;     // show toggle buttons

public:
    OTableTreeListBox(std::unique_ptr<weld::TreeView> xTreeView, bool bShowToggles);

    void init() { m_bVirtualRoot = true; }

    typedef std::pair< OUString, bool > TTableViewName;
    typedef std::vector< TTableViewName >         TNames;

    void    SuppressEmptyFolders() { m_bNoEmptyFolders = true; }

    /** determines whether the given entry denotes a tables folder
    */
    bool isFolderEntry(const weld::TreeIter& rEntry) const;

    /** fill the table list with the tables belonging to the connection described by the parameters
        @param _rxConnection
            the connection, which must support the service com.sun.star.sdb.Connection
        @throws
            <type scope="css::sdbc">SQLException</type> if no connection could be created
    */
    void    UpdateTableList(
                const css::uno::Reference< css::sdbc::XConnection >& _rxConnection
            );

    /** fill the table list with the tables and views determined by the two given containers.
        The views sequence is used to determine which table is of type view.
        @param      _rxConnection   the connection where you got the object names from. Must not be NULL.
                                    Used to split the full qualified names into its parts.
        @param      _rTables        table/view sequence
        @param      _rViews         view sequence
    */
    void    UpdateTableList(
                const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
                const css::uno::Sequence< OUString>& _rTables,
                const css::uno::Sequence< OUString>& _rViews
            );

    /** to be used if a foreign instance added a table
    */
    std::unique_ptr<weld::TreeIter> addedTable( const OUString& _rName );

    /** to be used if a foreign instance removed a table
    */
    void    removedTable( const OUString& _rName );

    std::unique_ptr<weld::TreeIter>    getAllObjectsEntry() const;

    /** does a wildcard check of the given entry
        <p>There are two different 'checked' states: If the user checks all children of an entry, this is different
        from checking the entry itself. The second is called 'wildcard' checking, 'cause in the resulting
        table filter it's represented by a wildcard.</p>
    */
    void            checkWildcard(weld::TreeIter& rEntry);

    /** determine if the given entry is 'wildcard checked'
        @see checkWildcard
    */
    bool            isWildcardChecked(const weld::TreeIter& rEntry);

    void            CheckButtons();     // make the button states consistent (bottom-up)

    void            checkedButton_noBroadcast(const weld::TreeIter& rEntry);
private:
    TriState implDetermineState(weld::TreeIter& rEntry);

    void implEmphasize(const weld::TreeIter& rEntry, bool _bChecked, bool _bUpdateDescendants = true, bool _bUpdateAncestors = true);

    /** adds the given entry to our list
        @precond
            our image provider must already have been reset to the connection to which the meta data
            belong.
    */
    std::unique_ptr<weld::TreeIter> implAddEntry(
            const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _rxMeta,
            const OUString& _rTableName,
            bool _bCheckName = true
        );

    void    implOnNewConnection( const css::uno::Reference< css::sdbc::XConnection >& _rxConnection );

    bool    impl_getAndAssertMetaData( css::uno::Reference< css::sdbc::XDatabaseMetaData >& _out_rMetaData ) const;

    bool haveVirtualRoot() const { return m_bVirtualRoot; }

public:
    /** fill the table list with the tables and views determined by the two given containers
        @param      _rxConnection   the connection where you got the object names from. Must not be NULL.
                                    Used to split the full qualified names into its parts.
        @param      _rTables        table/view sequence, the second argument is <TRUE/> if it is a table, otherwise it is a view.
    */
    void    UpdateTableList(
                const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
                const TNames& _rTables
            );

    /** returns a NamedDatabaseObject record which describes the given entry
    */
    css::sdb::application::NamedDatabaseObject
            describeObject(weld::TreeIter& rEntry);

    /** returns the fully qualified name of a table entry
        @param _pEntry
            the entry whose name is to be obtained. Must not denote a folder entry.
    */
    OUString getQualifiedTableName(weld::TreeIter& rEntry) const;

    std::unique_ptr<weld::TreeIter> getEntryByQualifiedName(const OUString& rName);
};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TABLETREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
