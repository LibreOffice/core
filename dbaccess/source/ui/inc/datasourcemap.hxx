/*************************************************************************
 *
 *  $RCSfile: datasourcemap.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:57:03 $
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

#ifndef DBAUI_DATASOURCEMAP_HXX
#define DBAUI_DATASOURCEMAP_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

class SfxItemPool;
class SfxItemSet;
//.........................................................................
namespace dbaui
{
//.........................................................................

    //=====================================================================
    //= ODatasourceMap
    //=====================================================================
    class ODatasourceMap
    {
        struct DatasourceInfo
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                xDatasource;
            SfxItemSet*         pModifications;

            DatasourceInfo() :pModifications (NULL) {  }
            DatasourceInfo(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDS,
                    SfxItemSet* _pMods = NULL)
                :xDatasource(_rxDS), pModifications(_pMods) { }
        };

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                m_xORB;                 /// service factory
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                                m_xDatabaseContext;     /// database context we're working in

        DECLARE_STL_USTRINGACCESS_MAP(DatasourceInfo, DatasourceInfos);
        DatasourceInfos         m_aDatasources;         /// known infos about data sources

        // deleted data sources, not necessarily with distinct names, that's why accessed via unique ids
        DECLARE_STL_MAP(sal_Int32, DatasourceInfo, ::std::less< sal_Int32 >, MapInt2Info);
        MapInt2Info             m_aDeletedDatasources;

    public:
        /// iterating through all data sources
        class Iterator;
        friend class ODatasourceMap::Iterator;

        /// encapsulates the infos about a data source for access from outside the class
        class ODatasourceInfo;
        friend class ODatasourceMap::ODatasourceInfo;

        ODatasourceInfo operator[](const ::rtl::OUString _rName);

        ODatasourceMap(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB);

        // get the database context
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    getContext() { return m_xDatabaseContext; }

        /// first element for iterating through the datasources
        Iterator    begin();
        /// last element for iterating through the datasources
        Iterator    end();

        /// first element for iterating through the deleted datasources
        Iterator    beginDeleted();
        /// last element for iterating through the deleted datasources
        Iterator    endDeleted();

        /// check if the object contains a valid datasource enumeration
        sal_Bool    isValid() const { return m_xDatabaseContext.is(); }
        /// check if a datasource with the given name exists
        sal_Bool    exists(const ::rtl::OUString& _rName) const;
        /// return the number of datasources available
        sal_Int32   size() const { return m_aDatasources.size(); }
        /// clear the map (non-deleted <em>and</em> deleted items)
        void        clear();
        /// clear the map (deleted items only)
        void        clearDeleted();

        /// clear the modification items for a datasource
        void        clearModifiedFlag(const ::rtl::OUString& _rName);

        /** tell the map that a data source is scheduled to be deleted.
            @return     id for accessing the deleted data source later. -1 if no free id existed or an error occured
        */
        sal_Int32   markDeleted(const ::rtl::OUString& _rName);

        /** restores a datasource which has previously been marked as deleted.<p/>
            @param      _nAccessId      the access id as got from <method>markDeleted</method>
            @param      _rName          contains, upon return, the name of the datasource the access key refers to
            @return     sal_True if the datasource was successfully restored, sal_False if it could not be restored
                        because of a naming conflict (e.g. because another data source now has the name of the
                        to-be-restored one).
            @see    renamed
            @see    markDeleted
        */
        sal_Bool    restoreDeleted(sal_Int32 _nAccessId, ::rtl::OUString& _rName);

        /// remove an element from the map
        void        deleted(const ::rtl::OUString& _rName);
            // (should be an erase(const Iterator&), but this is way to general ...

        /// update the infos for a data source with a given item set
        void        update(const ::rtl::OUString& _rName, SfxItemSet& _rSet);
        /** Tells the map that an entry has been renamed in a sense that it should be accessible under
            a new name. This does not necesssarily mean that the data source has been renamed within
            it's database context
        */
        void        renamed(const ::rtl::OUString& _rOldName, const ::rtl::OUString& _rNewName);

        /** adjust the registration name if necessary<p/>
            The real name of the data source (as indicated in the SfxItemSet for this ds) may be another
            one than the name the ds is registered for. This method corrects this, the ds will become registered
            under it's real name.
            @param      _rName      the name the ds is registered for
            @return                 the real name of the data source
        */
        ::rtl::OUString adjustRealName(const ::rtl::OUString& _rName);

    protected:
        /** ensure that the DatabaseInfo for the named object is filled<p/>
            This method allows us lazy access to the data sources: They're retrieved from the context
            only if they're accessed by somebody.
        */
        void    ensureObject(const ::rtl::OUString& _rName);
    };

    //-------------------------------------------------------------------------
    //- ODatasourceMap::ODatasourceInfo
    //-------------------------------------------------------------------------
    class ODatasourceMap::ODatasourceInfo
    {
        friend class ODatasourceMap;
        friend class ODatasourceMap::Iterator;

    private:
        ODatasourceMap*                             m_pOwner;
        const ODatasourceMap::DatasourceInfo&       m_rInfoImpl;
        ::rtl::OUString                             m_sName;
        sal_Int32                                   m_nAccessKey;

    public:
        ODatasourceInfo(const ODatasourceInfo& _rSource)
            :m_pOwner(_rSource.m_pOwner), m_sName(_rSource.m_sName), m_rInfoImpl(_rSource.m_rInfoImpl), m_nAccessKey(_rSource.m_nAccessKey) { }

        /// check if the datasource settings are modified
        sal_Bool        isModified() const;
        /// get the name the datasource is registered under
        ::rtl::OUString getName() const { return m_sName; }
        /// get the original name of a datasource (may habe been renamed)
        ::rtl::OUString getOriginalName() const;
        /// get the real name of the datasource, which is the name which is in the item set
        ::rtl::OUString getRealName() const;
        /// check if the datasource should is about to be renamed (which means the original name does not equal the real name
        sal_Bool        isRenamed() const { return !getRealName().equals(getOriginalName()); }
        /// get the key used to acces the object in the data source map
        sal_Int32       getAccessKey() const { return m_nAccessKey; }

        /// return the datasource the object represents
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        getDatasource() const;
        /** return the modifications for the data source<p/>
            The return value is non-NULL if and only if <method>isModified</method> returned sal_True
        */
        const SfxItemSet*
                        getModifications() const { return m_rInfoImpl.pModifications; }

        operator ::rtl::OUString() const { return getName(); }
        operator String() const { return getName().getStr(); }

        const ODatasourceInfo* const operator->() const { return this; }

    protected:
        ODatasourceInfo(
                ODatasourceMap* _pOwner, const ::rtl::OUString& _rName,
                const ODatasourceMap::DatasourceInfo& _rSource, sal_Int32 _nAccessKey)
            :m_pOwner(_pOwner), m_sName(_rName), m_rInfoImpl(_rSource), m_nAccessKey(_nAccessKey) { }
    };

    //-------------------------------------------------------------------------
    //- ODatasourceMap::Iterator
    //-------------------------------------------------------------------------
    class ODatasourceMap::Iterator
    {
        friend class ODatasourceMap;
    protected:
        ODatasourceMap*                                 m_pOwner;
        ODatasourceMap::ConstDatasourceInfosIterator    m_aPos;
        ODatasourceMap::ConstMapInt2InfoIterator        m_aPosDeleted;
        sal_Bool                                        m_bLoopingDeleted;

    public:
        Iterator(const Iterator& _rSource);

        ODatasourceInfo operator->() const;
        ODatasourceInfo operator*() const;

        /// prefix increment
        const Iterator& operator++();
        /// postfix increment
        const Iterator  operator++(int) { Iterator hold(*this); ++*this; return hold; }

        /// prefix decrement
        const Iterator& operator--();
        /// postfix decrement
        const Iterator  operator--(int) { Iterator hold(*this); --*this; return hold; }


    // compare two iterators
        friend bool operator==(const Iterator& lhs, const Iterator& rhs)
        {
            if (lhs.m_bLoopingDeleted)
                return lhs.m_aPosDeleted == rhs.m_aPosDeleted;
            else
                return lhs.m_aPos == rhs.m_aPos;
        }

        friend bool operator!=(const Iterator& lhs, const Iterator& rhs) { return !(lhs == rhs); }

    protected:
        Iterator(ODatasourceMap* _pOwner, ODatasourceMap::ConstDatasourceInfosIterator _rPos);
        Iterator(ODatasourceMap* _pOwner, ODatasourceMap::ConstMapInt2InfoIterator _rPos);

    protected:
        ::rtl::OUString implGetName(const ODatasourceMap::DatasourceInfo& _rInfo) const;
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBAUI_DATASOURCEMAP_HXX

