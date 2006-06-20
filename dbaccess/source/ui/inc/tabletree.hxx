/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabletree.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:19:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _DBAUI_TABLETREE_HXX_
#define _DBAUI_TABLETREE_HXX_

#ifndef _DBAUI_MARKTREE_HXX_
#include "marktree.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVER_HPP_
#include <com/sun/star/sdbc/XDriver.hpp>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

//========================================================================
//= OTableTreeListBox
//========================================================================
class OTableTreeListBox : public OMarkableTreeListBox
{
protected:
    Image           m_aTableImage;
    Image           m_aViewImage;

    sal_Bool        m_bVirtualRoot; // should the first entry be visible

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

    typedef ::std::pair< ::rtl::OUString,sal_Bool>  TTableViewName;
    typedef ::std::vector< TTableViewName >         TNames;
    /** call when HiContrast change.
    */
    virtual void notifyHiContrastChanged();

    /** fill the table list with the tables belonging to the connection described by the parameters
        @param _xConnection
            the connection, which must support the service com.sun.star.sdb.Connection
        @throws
            <type scope="com::sun::star::sdbc">SQLException</type> if no connection could be created
    */
    void
            UpdateTableList(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection
            )   throw(::com::sun::star::sdbc::SQLException);

    /** fill the table list with the tables and views determined by the two given containers.
        The views sequence is used to determine which table is of type view.
        @param      _rxConnMetaData meta data describing the connection where you got the object names from. Must not be NULL.
                                    Used to split the full qualified names into it's parts.
        @param      _rTables        table/view sequence
        @param      _rViews         view sequence
    */
    void    UpdateTableList(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxConnMetaData,
                const ::com::sun::star::uno::Sequence< ::rtl::OUString>& _rTables,
                const ::com::sun::star::uno::Sequence< ::rtl::OUString>& _rViews
            );

    /** to be used if a foreign instance added a table
    */
    SvLBoxEntry* addedTable(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn,
                const ::rtl::OUString& _rName,
                const ::com::sun::star::uno::Any& _rObject
            );

    /** to be used if a foreign instance removed a table
    */
    void    removedTable(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn,
                const ::rtl::OUString& _rName
            );

    SvLBoxEntry*    getEntryByQualifiedName(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn,
                const ::rtl::OUString& _rName
            );

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
    virtual void InitEntry(SvLBoxEntry* _pEntry, const XubString& _rString, const Image& _rCollapsedBitmap, const Image& _rExpandedBitmap);

    virtual void checkedButton_noBroadcast(SvLBoxEntry* _pEntry);

    void implEmphasize(SvLBoxEntry* _pEntry, sal_Bool _bChecked, sal_Bool _bUpdateDescendants = sal_True, sal_Bool _bUpdateAncestors = sal_True);

    SvLBoxEntry* implAddEntry(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxConnMetaData,
            const ::rtl::OUString& _rTableName,
            const Image& _rImage,
            SvLBoxEntry* _pParentEntry,
            sal_Int32 _nType,
            sal_Bool _bCheckName = sal_True
        );

    sal_Bool haveVirtualRoot() const { return m_bVirtualRoot; }

    /** fill the table list with the tables and views determined by the two given containers
        @param      _rxConnMetaData meta data describing the connection where you got the object names from. Must not be NULL.
                                    Used to split the full qualified names into it's parts.
        @param      _rTables        table/view sequence, the second argument is <TRUE/> if it is a table, otherwise it is a view.
    */
    void    UpdateTableList(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxConnMetaData,
                const TNames& _rTables
            );

};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_TABLETREE_HXX_

