/*************************************************************************
 *
 *  $RCSfile: TableController.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 14:36:53 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBUI_TABLECONTROLLER_HXX
#define DBUI_TABLECONTROLLER_HXX

#ifndef DBAUI_GENERICCONTROLLER_HXX
#include "genericcontroller.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _UNDO_HXX
#include <svtools/undo.hxx>
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


namespace dbaui
{
    class OTableRow;
    class OFieldDescription;
    class OTableController : public OGenericUnoController
    {
    private:
        SfxUndoManager  m_aUndoManager;
        ::std::vector<OTableRow*>   m_vRowList;
        OTypeInfoMap                m_aTypeInfo;
        ::std::vector<OTypeInfoMap::iterator> m_aTypeInfoIndex;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xTable;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier

        ::rtl::OUString m_sDataSourceName;  // is set in initialze
        ::rtl::OUString m_sCatalogName;     // catalog for update data
        ::rtl::OUString m_sSchemaName;      // schema for update data
        ::rtl::OUString m_sName;            // table for update data
        String          m_sTypeNames;       // these type names are the ones out of the resource file

        sal_Bool        m_bEditable;        // is the control readonly or not
        sal_Bool        m_bModified;        // is the data modified
        sal_Bool        m_bOwnConnection;   // is true when we created our own connection

        void loadData();
        void fillTypeInfo();        // fills a vector with type info
        void checkColumns() throw(::com::sun::star::sdbc::SQLException);;       // check if we have double column names
        String createUniqueName(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxTables,const String& _rDefault);
        void appendColumns(::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>& _rxColSup,sal_Bool _bKeyColumns=sal_False);
        void appendKey(::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XKeysSupplier>& _rxSup);
        void alterColumns();
        void dropKey();
        void setColumnProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxColumn,const OFieldDescription* _pFieldDesc);
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getKeyColumns() const;
    protected:
        // all the features which should be handled by this class
        virtual void            AddSupportedFeatures();
        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId);
        // execute a feature
        virtual void            Execute(sal_uInt16 nId);
        virtual ToolBox* CreateToolBox(Window* pParent);

        void createNewConnection(sal_Bool _bUI = sal_False);

    public:
        OTableController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        ~OTableController();
        // removes the connection from the vector and delete it
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         getConnection() { return m_xConnection; }
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       getTable() { return m_xTable;}
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    getNumberFormatter() const  { return m_xFormatter; }

        // should the statement be parsed by our own sql parser
        sal_Bool        isReadOnly()            const { return !m_bEditable; }
        sal_Bool        isModified()            const { return m_bModified; }

        void            setModified(sal_Bool _bModified=sal_True);

        //  const ::connectivity::OSQLParseNode* getParseTree() const { return m_aSqlIterator.getParseTree();}
        // need for undo's and redo's
        SfxUndoManager* getUndoMgr();
        ::std::vector<OTableRow*>*  getRows() { return &m_vRowList; }
        const OTypeInfoMap*         getTypeInfo() const { return &m_aTypeInfo; }

        const OTypeInfo* getTypeInfo(sal_Int32 _nPos) const { return m_aTypeInfoIndex[_nPos]->second; }
        const OTypeInfo* getTypeInfoByType(sal_Int32 _nDataType) const;

        virtual sal_Bool Construct(Window* pParent);
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt);
        // ::com::sun::star::container::XContainerListener
        virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );
        // ::com::sun::star::lang::XComponent
        virtual void        SAL_CALL dispose();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
        // need by registration
        static ::rtl::OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
        // lang::XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        //
        virtual void Load(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxIn);
        virtual void Save(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOut);

    };
}
#endif // DBUI_TABLECONTROLLER_HXX


