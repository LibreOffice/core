/*************************************************************************
 *
 *  $RCSfile: dbadmin.hxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:39 $
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
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
//#ifndef _SV_BUTTON_HXX
//#include <vcl/imagebtn.hxx>
//#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
#ifndef DBAUI_DATASOURCEMAP_HXX
#include "datasourcemap.hxx"
#endif
#ifndef DBAUI_DATASOURCESELECTOR_HXX
#include "datasourceselector.hxx"
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

class ODsnTypeCollection;

//=========================================================================
//= ODbAdminDialog
//=========================================================================
class OGeneralPage;
struct OPageSettings;
/** tab dialog for administrating the office wide registered data sources
*/
class ODbAdminDialog : public SfxTabDialog
{
public:
    /** modes the dialog can be operated in
    */
    enum OperationMode
    {
        omFull,                 // full administration of data sources
        omSingleEdit,           // edit a single data source only
        omSingleEditFixedType   // edit a single data source only, don't allow a type change
    };

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                            m_xORB;                 /// service factory
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                            m_xDatabaseContext;     /// database context we're working in
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XNamingService >
                            m_xDynamicContext;      /// just another interface of the context ...
    ODatasourceMap          m_aDatasources;

    typedef ::std::stack< sal_Int32 > PageStack;
    PageStack               m_aCurrentDetailPages;  // ids of all currently enabled (type-dependent) detail pages

    ::rtl::OUString         m_sCurrentDatasource;   /// name of the currently selected data source, empty if deleted
    typedef ::std::set< ::rtl::OUString >   StringSet;
    typedef StringSet::const_iterator       ConstStringSetIterator;
    StringSet               m_aValidDatasources;        /// names of all data sources except the currently selected one

    DECLARE_STL_MAP(sal_Int32, ::rtl::OUString, ::std::less< sal_Int32 >, MapInt2String);
    MapInt2String           m_aDirectPropTranslator;    /// translating property id's into names (direct properties of a data source)
    MapInt2String           m_aIndirectPropTranslator;  /// translating property id's into names (indirect properties of a data source)

    sal_Bool                m_bResetting : 1;   /// sal_True while we're resetting the pages
    sal_Bool                m_bApplied : 1;     /// sal_True if any changes have been applied while the dialog was executing
    sal_Bool                m_bUIEnabled : 1;   /// <TRUE/> if the UI is enabled, false otherwise. Cannot be switched back to <TRUE/>, once it is <FALSE/>

    sal_Int32               m_nCurrentDeletedDataSource;
    sal_Int16               m_nPostApplyPage;           // the page to be activated after an async apply operation
    const OPageSettings*    m_pPostApplyPageSettings;   // the page data to pass to this page

    OperationMode           m_eMode;        // the mode we're working in

private:
    ODatasourceSelector     m_aSelector;

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
    void    selectDataSource(const ::rtl::OUString& _rName);

    /** inserts a new data source with the given name.
        <p>The name must not already exist in the list of available, non-deleted data sources.</p>
        @return
            <FALSE/> if the data source could not be inserted. Possible Reasons for this include:
            <ul><li>the currently selected data source could not be deselected (because it's data as entered by
                    the user is inconsistent)</li>
                <li>the name given is invalid</li>
            </ul>
    */
    sal_Bool insertDataSource(const ::rtl::OUString& _rName);

    /// retrieves the current operation mode
    OperationMode   getMode() const { return m_eMode; }

    /** sets a new operation mode
        <p><em>Must</em> not be called if the dialog is beeind executed.</p>
    */
    void            setMode(const OperationMode _eMode);

    /** create and return an item set for use with the dialog.
        @param      _pTypeCollection        pointer to an <type>ODatasourceMap</type>. May be NULL, in this case
                                            the pool will not contain a typecollection default.
    */
    static SfxItemSet*  createItemSet(SfxItemSet*& _rpSet, SfxItemPool*& _rpPool, SfxPoolItem**& _rppDefaults, ODsnTypeCollection* _pTypeCollection);
    /** destroy and item set / item pool / pool defaults previously created by <method>createItemSet</method>
    */
    static void         destroyItemSet(SfxItemSet*& _rpSet, SfxItemPool*& _rpPool, SfxPoolItem**& _rppDefaults);

    /** translate the current dialog SfxItems into driver relevant PropertyValues
        @see successfullyConnected
    */
    sal_Bool    getCurrentSettings(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rDriverParams);

    /** to be called if the settings got from getCurrentSettings have been used for successfully connecting
        @see getCurrentSettings
    */
    void        successfullyConnected();

    /// clear the password in the current data source's item set
    void        clearPassword();

    /// apply the current changes, return sal_True if successfull
    void        applyChangesAsync(const OPageSettings* _pUseTheseSettings = NULL);

    /// return <TRUE/> if in the current state, the changes can be saved (i.e. they produce no conflict)
    sal_Bool    isApplyable() const;

    /// return <TRUE/> if the currently selected data (if any) source is modified
    sal_Bool    isCurrentModified() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getORB() const { return m_xORB; }

    /** creates a new connection. The caller is responsible to dispose it !!!!
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >     createConnection();

    /** return the corresponding driver for the selected URL
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >         getDriver();

    /** returns the data source the dialog is currently working with
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   getCurrentDataSource();

protected:
    // adds a new detail page and remove all the old ones
    void addDetailPage(USHORT _nPageId,USHORT _nTextId,CreateTabPage pCreateFunc);
    // removes all detail pages
    void removeDetailPages();

    virtual void PageCreated(USHORT _nId, SfxTabPage& _rPage);
    virtual short Ok();

protected:
    sal_Bool    isUIEnabled() const { return m_bUIEnabled; }
    void    disabledUI() { m_bUIEnabled = sal_False; }

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
    void        implTranslateProperty(SfxItemSet& _rSet, sal_Int32  _nId, const ::com::sun::star::uno::Any& _rValue);
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
    const sal_Int32*    getRelevantItems(const SfxItemSet& _rSet) const;

    /** check if the data source described by the given set needs authentication<p/>
        The return value depends on the data source type only.
    */
    sal_Bool            hasAuthentication(const SfxItemSet& _rSet) const;

    /// prepares switching to another data source
    sal_Bool            prepareSwitchDatasource();

    /** checks if the name given can be used as name for a new data source
        <p>The method does not check if the name is empty for performance reasons: If so, <TRUE/> is returned,
        though the name is not valid at all.</p>
    */
    sal_Bool            isValidNewName(const ::rtl::OUString& _rName) const;

    /** inserts a new data source and selects it
        <p>no checks are made if the name is valid, or the currently selected data source can be left. This has
        to be done by the caller.</p>
        @return
            <FALSE/> in case no new com.sun.star.sdb.DataSource could be created
    */
    sal_Bool            implInsertNew_noCheck(const ::rtl::OUString& _rName);

#ifdef DBG_UTIL
    ::rtl::OString translatePropertyId( sal_Int32 _nId );
#endif

private:
    DECL_LINK(OnDatasourceSelected, ListBox*);
    DECL_LINK(OnTypeSelected, OGeneralPage*);
    DECL_LINK(OnNameModified, OGeneralPage*);
    DECL_LINK(OnValidateName, OGeneralPage*);
    DECL_LINK(OnDatasourceModifed, SfxTabPage*);
    DECL_LINK(OnNewDatasource, Window*);
    DECL_LINK(OnDeleteDatasource, Window*);
    DECL_LINK(OnRestoreDatasource, Window*);
    DECL_LINK(OnApplyChanges, PushButton*);
    DECL_LINK(OnAsyncApplyChanges, void*);
    DECL_LINK(OnAsyncSelectDetailsPage, void*);

    String getConnectionURL() const;
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DBADMIN_HXX_

