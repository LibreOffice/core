/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableController.hxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 14:02:00 $
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
#ifndef DBUI_TABLECONTROLLER_HXX
#define DBUI_TABLECONTROLLER_HXX

#ifndef DBAUI_SINGLEDOCCONTROLLER_HXX
#include "singledoccontroller.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XKEYSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

class FixedText;
namespace dbaui
{
    class OTableRow;
    class OFieldDescription;
    typedef OSingleDocumentController   OTableController_BASE;
    class OTableController : public OTableController_BASE
    {
    private:
        OModuleClient        m_aModuleClient;
        ::std::vector< ::boost::shared_ptr<OTableRow> > m_vRowList;
        OTypeInfoMap                m_aTypeInfo;
        ::std::vector<OTypeInfoMap::iterator> m_aTypeInfoIndex;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xTable;

        ::rtl::OUString m_sCatalogName;         // catalog for update data
        ::rtl::OUString m_sSchemaName;          // schema for update data
        ::rtl::OUString m_sName;                // table for update data
        ::rtl::OUString m_sAutoIncrementValue;  // the autoincrement value set in the datasource
        String          m_sTypeNames;           // these type names are the ones out of the resource file
        TOTypeInfoSP    m_pTypeInfo;            // fall back when type is unkown because database driver has a failure

        sal_Bool        m_bAllowAutoIncrementValue; // no : 1 NO BIT , is true when the datasource has a AutoIncrementValue property in their info property
        sal_Bool        m_bNew      : 1;        // is true when we create a new table


        void reSyncRows();
        void assignTable();                 // set the table if a name is given
        void loadData();
        sal_Bool checkColumns(sal_Bool _bNew) throw(::com::sun::star::sdbc::SQLException);      // check if we have double column names
        String createUniqueName(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxTables,const String& _rDefault);
        void appendColumns(::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>& _rxColSup,sal_Bool _bNew,sal_Bool _bKeyColumns = sal_False);
        void appendPrimaryKey(::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XKeysSupplier>& _rxSup,sal_Bool _bNew);
        void alterColumns();
        void dropPrimaryKey();
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getKeyColumns() const;
        ::rtl::OUString createUniqueName(const ::rtl::OUString& _rName);

        void reload();

        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures();
        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

        virtual void losingConnection( );

        virtual ::rtl::OUString getPrivateTitle( ) const;

        void        doEditIndexes();
        sal_Bool    doSaveDoc(sal_Bool _bSaveAs);

        virtual ~OTableController();
    public:
        OTableController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       getTable() { return m_xTable;}

        sal_Bool isAddAllowed()     const;
        sal_Bool isDropAllowed()    const;
        sal_Bool isAlterAllowed()   const;


        inline sal_Bool                 isAutoIncrementValueEnabled()   const { return m_bAllowAutoIncrementValue; }
        inline const ::rtl::OUString&   getAutoIncrementValue()         const { return m_sAutoIncrementValue; }

        virtual void setModified(sal_Bool _bModified=sal_True);

        //  const ::connectivity::OSQLParseNode* getParseTree() const { return m_aSqlIterator.getParseTree();}
        // need for undo's and redo's
        SfxUndoManager* getUndoMgr();
        inline ::std::vector< ::boost::shared_ptr<OTableRow> >* getRows() { return &m_vRowList; }

        /// returns the postion of the the first empty row
        sal_Int32                           getFirstEmptyRowPosition() const;

        inline const OTypeInfoMap*          getTypeInfo() const { return &m_aTypeInfo; }

        inline TOTypeInfoSP                 getTypeInfo(sal_Int32 _nPos) const { return m_aTypeInfoIndex[_nPos]->second; }
        TOTypeInfoSP                        getTypeInfoByType(sal_Int32 _nDataType) const;

        inline TOTypeInfoSP                 getTypeInfoFallBack() const { return m_pTypeInfo; }

        virtual sal_Bool                    Construct(Window* pParent);
        // XEventListener
        virtual void SAL_CALL               disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL           suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::lang::XComponent
        virtual void        SAL_CALL disposing();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
        // need by registration
        static ::rtl::OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

        //
        virtual void Load(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxIn);
        virtual void Save(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOut);

    protected:
        void startTableListening();
        void stopTableListening();
        virtual void impl_initialize();
    };
}
#endif // DBUI_TABLECONTROLLER_HXX


