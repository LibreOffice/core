/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DBAUI_TABLETREE_HXX_
#define _DBAUI_TABLETREE_HXX_

#include "marktree.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdb/application/NamedDatabaseObject.hpp>

#include <memory>

//.........................................................................
namespace dbaui
{
//.........................................................................

class ImageProvider;

//========================================================================
//= OTableTreeListBox
//========================================================================
class OTableTreeListBox : public OMarkableTreeListBox
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    m_xConnection;      // the connection we're working for, set in implOnNewConnection, called by UpdateTableList
    ::std::auto_ptr< ImageProvider >
                    m_pImageProvider;   // provider for our images
    sal_Bool        m_bVirtualRoot; // should the first entry be visible
    bool            m_bNoEmptyFolders;  // should empty catalogs/schematas be prevented from being displayed?

public:
    OTableTreeListBox(
        Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
        WinBits nWinStyle,
        sal_Bool _bVirtualRoot );

    OTableTreeListBox(
        Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
        const ResId& rResId,
        sal_Bool _bVirtualRoot );

    ~OTableTreeListBox();

    typedef ::std::pair< ::rtl::OUString,sal_Bool>  TTableViewName;
    typedef ::std::vector< TTableViewName >         TNames;

    void    suppressEmptyFolders() { m_bNoEmptyFolders = true; }

    /** call when HiContrast change.
    */
    void notifyHiContrastChanged();

    /** determines whether the given entry denotes a tables folder
    */
    bool    isFolderEntry( const SvLBoxEntry* _pEntry ) const;

    /** determines whether the given entry denotes a table or view
    */
    bool    isTableOrViewEntry( const SvLBoxEntry* _pEntry ) const
    {
        return !isFolderEntry( _pEntry );
    }

    /** fill the table list with the tables belonging to the connection described by the parameters
        @param _rxConnection
            the connection, which must support the service com.sun.star.sdb.Connection
        @throws
            <type scope="com::sun::star::sdbc">SQLException</type> if no connection could be created
    */
    void    UpdateTableList(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
            )   throw(::com::sun::star::sdbc::SQLException);

    /** fill the table list with the tables and views determined by the two given containers.
        The views sequence is used to determine which table is of type view.
        @param      _rxConnection   the connection where you got the object names from. Must not be NULL.
                                    Used to split the full qualified names into it's parts.
        @param      _rTables        table/view sequence
        @param      _rViews         view sequence
    */
    void    UpdateTableList(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const ::com::sun::star::uno::Sequence< ::rtl::OUString>& _rTables,
                const ::com::sun::star::uno::Sequence< ::rtl::OUString>& _rViews
            );

    /** returns a NamedDatabaseObject record which describes the given entry
    */
    ::com::sun::star::sdb::application::NamedDatabaseObject
            describeObject( SvLBoxEntry* _pEntry );

    /** to be used if a foreign instance added a table
    */
    SvLBoxEntry* addedTable( const ::rtl::OUString& _rName );

    /** to be used if a foreign instance removed a table
    */
    void    removedTable( const ::rtl::OUString& _rName );

    /** returns the fully qualified name of a table entry
        @param _pEntry
            the entry whose name is to be obtained. Must not denote a folder entry.
    */
    String getQualifiedTableName( SvLBoxEntry* _pEntry ) const;

    SvLBoxEntry*    getEntryByQualifiedName( const ::rtl::OUString& _rName );

    SvLBoxEntry*    getAllObjectsEntry() const;

    /** does a wildcard check of the given entry
        <p>There are two different 'checked' states: If the user checks all children of an entry, this is different
        from checking the entry itself. The second is called 'wildcard' checking, 'cause in the resulting
        table filter it's represented by a wildcard.</p>
    */
    void            checkWildcard(SvLBoxEntry* _pEntry);

    /** determine if the given entry is 'wildcard checked'
        @see checkWildcard
    */
    sal_Bool        isWildcardChecked(SvLBoxEntry* _pEntry) const;

protected:
    virtual void InitEntry(SvLBoxEntry* _pEntry, const XubString& _rString, const Image& _rCollapsedBitmap, const Image& _rExpandedBitmap, SvLBoxButtonKind _eButtonKind);

    virtual void checkedButton_noBroadcast(SvLBoxEntry* _pEntry);

    void implEmphasize(SvLBoxEntry* _pEntry, sal_Bool _bChecked, sal_Bool _bUpdateDescendants = sal_True, sal_Bool _bUpdateAncestors = sal_True);

    /** adds the given entry to our list
        @precond
            our image provider must already have been reset to the connection to which the meta data
            belong.
    */
    SvLBoxEntry* implAddEntry(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxMeta,
            const ::rtl::OUString& _rTableName,
            sal_Bool _bCheckName = sal_True
        );

    void    implSetDefaultImages();

    void    implOnNewConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection );

    bool    impl_getAndAssertMetaData( ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _out_rMetaData ) const;

    sal_Bool haveVirtualRoot() const { return m_bVirtualRoot; }

    /** fill the table list with the tables and views determined by the two given containers
        @param      _rxConnection   the connection where you got the object names from. Must not be NULL.
                                    Used to split the full qualified names into it's parts.
        @param      _rTables        table/view sequence, the second argument is <TRUE/> if it is a table, otherwise it is a view.
    */
    void    UpdateTableList(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const TNames& _rTables
            );

};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_TABLETREE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
