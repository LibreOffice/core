/*************************************************************************
 *
 *  $RCSfile: tabletree.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:03:08 $
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
    OTableTreeListBox( Window* pParent
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
                        ,sal_Bool _bHiContrast,WinBits nWinStyle = NULL,sal_Bool _bVirtualRoot=sal_True );
    OTableTreeListBox( Window* pParent
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
                        , const ResId& rResId,sal_Bool _bHiContrast,sal_Bool _bVirtualRoot=sal_True );
    /** call when HiContrast change.
    */
    virtual void notifyHiContrastChanged();

    /** fill the table list with the tables belonging to the connection described by the parameters
        <BR>
        The given URL is used to obtain a <type scope="com.sun.star.sdbc">Driver</type> object which can
        handle it. After this, the driver is asked (<type scope="com.sun.star.sdbcx">XDataDefinitionSupplier</type>)
        for a <type scope="com.sun.star.sdbcx">DatabaseDefinition</type> object for the connection.
        @param _rConnectionURL
            the connection URL
        @param _rProperties
            additional properties for creating the connection
        @param _rxCreator
            will, upon return, contain the XDriver which was used to create the connection
        @return
            the connection which was created for the given URL and the given params.
        @throws
            <type scope="com::sun::star::sdbc">SQLException</type> if no connection could be created
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
            UpdateTableList(
                const   ::rtl::OUString& _rConnectionURL,
                const   ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > _rProperties,
                        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >& _rxCreator
            )   throw(::com::sun::star::sdbc::SQLException);

    /** fill the table list with the tables and views determined by the two given containers
        @param      _rxConnMetaData meta data describing the connection where you got the object names from. Must not be NULL.
                                    Used to split the full qualified names into it's parts.
        @param      _rxTables       table container. May be NULL.
        @param      _rxView         view container. May be NULL.
    */
    void    UpdateTableList(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxConnMetaData,
                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTables,
                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rViews
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
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_TABLETREE_HXX_

