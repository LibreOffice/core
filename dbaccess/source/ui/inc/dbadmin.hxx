/*************************************************************************
 *
 *  $RCSfile: dbadmin.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-28 11:45:46 $
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

#ifndef _DBAUI_DBADMIN_HXX_
#define _DBAUI_DBADMIN_HXX_

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/imagebtn.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef __SGI_STL_SET
#include <set>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

class ODsnTypeCollection;

//=========================================================================
//= ODatasourceSelector
//=========================================================================
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

    /** create a new (floating) datasource<p/>
        The ds is inserted it into the map under the given name and returned.<br/>
        If no object could be created, nothing is inserted into the map.<br/>
        The item pool and the item ranges are used to create the initial SfxItemSet for the new ds.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                createNew(const ::rtl::OUString& _rName, SfxItemPool* _pPool, const USHORT* _pRanges);

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
    /// check if the datasource is to be created
    sal_Bool        isNew() const;
    /// get the name the datasource is registered under
    ::rtl::OUString getName() const { return m_sName; }
    /// get the original name of a datasource (may habe been renamed)
    ::rtl::OUString getOriginalName() const;
    /// get the real name of the datasource, which is the name which is in the item set
    ::rtl::OUString getRealName() const;
    /// check if the datasource should is about to be renamed (which means the original name does not equal the real name
    sal_Bool        isRenamed() const { return !isNew() && !getRealName().equals(getOriginalName()); }
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

//=========================================================================
//= DatasourceState
//=========================================================================
enum DatasourceState
{
    CLEAN,
    MODIFIED,
    NEW,
    DELETED
};

/** administrates the listbox for displaying the data sources</p>
    The list may contain normal, modified, new and "deleted" entries. Excluding the deleted ones,
    the entry names must be unique, that's why accessing entries which are not deleted is done by
    specifying a name.<br/>
    To access entries which are deleted, a special access key has to be supplied by the user of this class.
    This key has to be unique, so it can be used for non-ambiguous access.
*/
//=========================================================================
//= ODatasourceSelector
//=========================================================================
class ODatasourceSelector : public Window
{
protected:
    PushButton  m_aNewDatasource;
    ListBox     m_aDatasourceList;

    Link    m_aNewHandler;
    Link    m_aDeleteHandler;
    Link    m_aRestoreHandler;

    struct EntryData
    {
        DatasourceState     eState;     // state of the data source
        sal_Int32           nAccessKey; // access key, relevant only if DELETED == eState

        EntryData() :eState(CLEAN), nAccessKey(-1) { }
        EntryData(DatasourceState _eState) :eState(_eState), nAccessKey(-1) { }
    };

protected:
    /// set the state of a data source specified by position
    void                setEntryState(sal_uInt16 _nPos, DatasourceState _eState);
    /// get the state of a data source specified by position
    DatasourceState     getEntryState(sal_uInt16 _nPos) const;
    /// get the resource id for imgages to be used for the given data source state
    sal_uInt16          getImageId(DatasourceState _eState);
    /** get the position of an entry given by name, excluding the entries marked as deleted
        (with respect to these deleted entries the names are unique)
    */
    sal_uInt16          getValidEntryPos(const String& _rName);
    /// get the position of an (deleted) entry for a given access key
    sal_uInt16          getDeletedEntryPos(sal_Int32 _nAccessKey);

    /// get the access key for a entry given by position
    sal_Int32           getAccessKey(sal_uInt16 _nPos) const;
    /// set the access key for a entry given by position, to be used for entry which are in state DELETED only
    void                setAccessKey(sal_uInt16 _nPos, sal_Int32 _nAccessKey);

    /// remove an entry (given by pos) from the list, auto-select the next one
    void                implDeleted(sal_uInt16 _nPos);

public:
    ODatasourceSelector(Window* _pParent, const ResId& _rResId);
    ~ODatasourceSelector();

    /// mark the given data source as modified
    void        modified(const String& _sName);
    /** tell the selector object that a data source has been renamed.
        @param      _rOldName       the old entry name
        @param      _rNewName       the new entry name
        @param      _bIsNew         indicates whether or not the entry refers to a new datasource
    */
    void        renamed(const String& _rOldName, const String& _rNewName);

    /** tell the selector that a data source is scheduled to be deleted.
        @param  _rName          name of the entry. Must be an entry which's DatasourceState is not DELETED and not NEW
        @param  _nAccessKey     Needed for accessing the entry after it has been marked as deleted
    */
    void        markDeleted(const String& _rName, sal_Int32 _nAccessKey);

    /** restore an entry previously marked as deleted<p/>
        After returning from this method, the access key used is invalid.
        @param      _nAccessKey     the access key used in <method>markDeleted</method>
        @param      _eState         the new state of the datasource. Must not be DELETED
    */
    void        restoreDeleted(sal_Int32 _nAccessKey, DatasourceState _eState);

    /** tell the selector to really delete an entry from the list.
        @param  _rName  name of the entry. Must be an entry which's DatasourceState is not DELETED
    */
    void        deleted(const String& _rName);

    /** tell the selector to really delete an entry from the list.
        @param  _nAccessKey     access key of the entry. Must be an entry which's DatasourceState is DELETED
    */
    void        deleted(sal_Int32 _nAccessKey);

    /// insert the name of an existent data source
    sal_uInt16  insert(const String& _rName);

    /// insert the name of a daza source which is newly created
    void        insertNew(const String& _rName);

    /** tell the selector that a data source has been flushed.<p/>
        This means that the state a formerly modified or new entry changes to <em>clean</em>.
    */
    void        flushed(const String& _rName);

    /// get the name of the currently selected data source
    String      getSelected() const { return m_aDatasourceList.GetSelectEntry(); }

    /// get the state of the currently selected data source
    DatasourceState getSelectedState() const { return getEntryState(m_aDatasourceList.GetSelectEntryPos()); }
    /// get the access key of the currently selected data source
    sal_Int32       getSelectedAccessKey() const { return getAccessKey(m_aDatasourceList.GetSelectEntryPos()); }

    /// select a data source by name, valid only for non-deleted data sources
    void        select(const String& _rName);
    /// select a data source by acces key
    void        select(sal_Int32 _nAccessKey);

    /// set a handler to be called whenever the selection changes
    void        setSelectHandler(const Link& _rHdl) { m_aDatasourceList.SetSelectHdl(_rHdl); }

    /// set a handler to be called if the user chooses to create a new data source
    void        setNewHandler(const Link& _rHdl) { m_aNewHandler = _rHdl; }

    /// set a handler to be called if the user chooses to delete a new data source
    void        setDeleteHandler(const Link& _rHdl) { m_aDeleteHandler = _rHdl; }

    /// set a handler to be called if the user chooses to restore a delete data source
    void        setRestoreHandler(const Link& _rHdl) { m_aRestoreHandler = _rHdl; }

protected:
    virtual void    Resize();
    virtual long    Notify(NotifyEvent& _rNEvt);

    DECL_LINK(OnButtonPressed, Button*);
};

//=========================================================================
//= ODbAdminDialog
//=========================================================================
class OGeneralPage;
/** tab dialog for administrating the office wide registered data sources
*/
class ODbAdminDialog : public SfxTabDialog
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                            m_xORB;                 /// service factory
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                            m_xDatabaseContext;     /// database context we're working in
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XNamingService >
                            m_xDynamicContext;      /// just another interface of the context ...
    ODatasourceMap          m_aDatasources;

    typedef ::std::stack< sal_uInt16 > PageStack;
    PageStack               m_aCurrentDetailPages;  // ids of all currently enabled (type-dependent) detail pages

    ::rtl::OUString         m_sCurrentDatasource;   /// name of the currently selected data source, empty if deleted
    typedef ::std::set< ::rtl::OUString >   StringSet;
    typedef StringSet::const_iterator       ConstStringSetIterator;
    StringSet               m_aValidDatasources;        /// names of all data sources except the currently selected one

    DECLARE_STL_MAP(sal_uInt16, ::rtl::OUString, ::std::less< sal_uInt16 >, MapInt2String);
    MapInt2String           m_aDirectPropTranslator;    /// translating property id's into names (direct properties of a data source)
    MapInt2String           m_aIndirectPropTranslator;  /// translating property id's into names (indirect properties of a data source)

    sal_Bool                m_bResetting : 1;   /// sal_True while we're resetting the pages

    sal_Int32               m_nCurrentDeletedDataSource;
private:
    ODatasourceSelector m_aSelector;

public:
    /** ctor. The itemset given should have been created by <method>createItemSet</method> and should be destroyed
        after the dialog has been destroyed
    */
    ODbAdminDialog(Window* pParent,
        SfxItemSet* _pItems,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
    ~ODbAdminDialog();

    // SfxTabDialog overridables
    virtual short Execute();

    /** select a data source given by name<p/>
        This method is used by the UNO wrapper for the dialog to set the initial selection.<br/>
        If no data source with the given name exists, nothing is changed at all.
    */
    void selectDataSource(const ::rtl::OUString& _rName);

    /** create and return an item set for use with the dialog.
        @param      _pTypeCollection        pointer to an <type>ODatasourceMap</type>. May be NULL, in this case
                                            the pool will not contain a typecollection default.
    */
    static SfxItemSet*  createItemSet(SfxItemSet*& _rpSet, SfxItemPool*& _rpPool, SfxPoolItem**& _rppDefaults, ODsnTypeCollection* _pTypeCollection);
    /** destroy and item set / item pool / pool defaults previously created by <method>createItemSet</method>
    */
    static void         destroyItemSet(SfxItemSet*& _rpSet, SfxItemPool*& _rpPool, SfxPoolItem**& _rppDefaults);

    /// translate the current dialog SfxItems into driver relevant PropertyValues
    sal_Bool    getCurrentSettings(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rDriverParams);

protected:
    virtual void PageCreated(USHORT _nId, SfxTabPage& _rPage);
    virtual short Ok();

protected:
    /// select a (scheduled-to-be-)deleted data source, given by it's access key
    void implSelectDeleted(sal_Int32 _nKey);
    /// select a datasource with a given name, adjust the item set accordingly, and everything like that ..
    void implSelectDatasource(const ::rtl::OUString& _rRegisteredName);
    /// reset the tag pages according to m_sCurrentDatasource and <arg>_rxDatasource</arg>
    void resetPages(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDatasource, sal_Bool _bDeleted);

    /** translate properties of an UNO data source into SfxItems
    */
    void translateProperties(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxSource,
            SfxItemSet& _rDest);

    /** translate SfxItems into properties of an UNO data source
    */
    void translateProperties(
            const SfxItemSet& _rSource,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDest);

    /** fill a data source info array with the settings from a given item set
    */
    void fillDatasourceInfo(const SfxItemSet& _rSource, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo);

    /** get the property set for the data source with the given name
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getDatasource(const ::rtl::OUString& _rName);

    /** get a unique new datasource name
    */
    ::rtl::OUString getUniqueName() const;

    /// translate the given value into an SfxPoolItem, put this into the given set under the given id
    void        implTranslateProperty(SfxItemSet& _rSet, sal_uInt16  _nId, const ::com::sun::star::uno::Any& _rValue);
    /// translate the given SfxPoolItem into an <type scope="com.sun.star.Any">uno</type>
    ::com::sun::star::uno::Any
                implTranslateProperty(const SfxPoolItem* _pItem);
    /// translate the given SfxPoolItem into an <type scope="com.sun.star.Any">uno</type>, set it (under the given name) on the given property set
    void        implTranslateProperty(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxSet, const ::rtl::OUString& _rName, const SfxPoolItem* _pItem);

    enum ApplyResult
    {
        AR_LEAVE_MODIFIED,      // somthing was modified and has successfully been committed
        AR_LEAVE_UNCHANGED,     // no changes were made
        AR_KEEP                 // don't leave the page (e.g. because an error occured)
    };
    /** apply all changes made
    */
    ApplyResult implApplyChanges();

    /** extracts the connection type from the given set<p/>
        The connection type is determined by the value of the DSN item, analyzed by the TypeCollection item.
    */
    DATASOURCE_TYPE     getDatasourceType(const SfxItemSet& _rSet) const;

    /** get a list of items relevant for the curently selected type.<p/>
        The type is calculated from the DSN item in the given set.<br/>
        The usual items such as DSN, NAME etc. are not returned, only the special ones not common to all types.
    */
    const sal_uInt16*   getRelevantItems(const SfxItemSet& _rSet) const;

    /** check if the data source described by the given set needs authentication<p/>
        The return value depends on the data source type only.
    */
    sal_Bool            hasAuthentication(const SfxItemSet& _rSet) const;

private:
    DECL_LINK(OnDatasourceSelected, ListBox*);
    DECL_LINK(OnTypeSelected, OGeneralPage*);
    DECL_LINK(OnNameModified, OGeneralPage*);
    DECL_LINK(OnDatasourceModifed, SfxTabPage*);
    DECL_LINK(OnNewDatasource, Window*);
    DECL_LINK(OnDeleteDatasource, Window*);
    DECL_LINK(OnRestoreDatasource, Window*);
    DECL_LINK(OnApplyChanges, PushButton*);
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DBADMIN_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.6  2000/11/23 02:04:35  svesik
 *  Remove stl/ from #include statement
 *
 *  Revision 1.5  2000/11/10 17:36:50  fs
 *  small bug fixes
 *
 *  Revision 1.4  2000/10/31 08:03:33  fs
 *  +selectDataSource - supporting an initial selecting when creating as service
 *
 *  Revision 1.3  2000/10/30 08:00:23  fs
 *  +getDatasourceType / +getRelevantItems / +hasAuthentication
 *
 *  Revision 1.2  2000/10/26 07:33:19  fs
 *  fillDatasourceInfo
 *
 *  Revision 1.1  2000/10/25 12:49:01  fs
 *  moved herein from ..\dlg
 *
 *  Revision 1.7  2000/10/24 12:12:26  fs
 *  ODatasourceMap::update takes a non-constant set (to reset the ORIGINALNAME item)
 *
 *  Revision 1.6  2000/10/23 12:56:50  fs
 *  added apply functionality
 *
 *  Revision 1.5  2000/10/13 16:06:21  fs
 *  implemented the usage if the 'Info' property of the data sources / allow key usage in the data source list
 *
 *  Revision 1.4  2000/10/12 16:20:42  fs
 *  new implementations ... still under construction
 *
 *  Revision 1.3  2000/10/11 11:31:03  fs
 *  new implementations - still under construction
 *
 *  Revision 1.2  2000/10/09 12:39:29  fs
 *  some (a lot of) new imlpementations - still under development
 *
 *  Revision 1.1  2000/10/05 10:04:54  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 20.09.00 13:09:03  fs
 ************************************************************************/

