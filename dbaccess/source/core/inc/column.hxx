/*************************************************************************
 *
 *  $RCSfile: column.hxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 15:48:23 $
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

#ifndef _DBA_COREAPI_COLUMN_HXX_
#define _DBA_COREAPI_COLUMN_HXX_

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDROP_HPP_
#include <com/sun/star/sdbcx/XDrop.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif
#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef CONNECTIVITY_COLUMNSHELPER_HXX
#include <connectivity/TColumnsHelper.hxx>
#endif
#ifndef _CONNECTIVITY_FILE_VALUE_HXX_
#include <connectivity/FValue.hxx>
#endif
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif
#ifndef _CONNECTIVITY_SDBCX_IREFRESHABLE_HXX_
#include <connectivity/sdbcx/IRefreshable.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

namespace dbaccess
{

    //************************************************************
    //  OColumn
    //************************************************************
    typedef ::cppu::WeakComponentImplHelper3<   ::com::sun::star::lang::XServiceInfo,
                                                ::com::sun::star::container::XNamed,
                                                ::com::sun::star::lang::XUnoTunnel > OColumnBase;

    //------------------------------------------------------------
    class OColumnSettings;
    class OColumn   :public comphelper::OBaseMutex
                    ,public OColumnBase
                    ,public ::cppu::OPropertySetHelper

    {
        friend class OColumns;

    protected:
        ::rtl::OUString m_sName;

    protected:
        OColumn();

    public:
        virtual ~OColumn();

    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) = 0;

    // com::sun::star::uno::XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();

    // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::cppu::OPropertySetHelper
        virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                ::com::sun::star::uno::Any & rConvertedValue,
                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue )
                                    throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                                     )
                                                     throw (::com::sun::star::uno::Exception);

        // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

    // cppu::OComponentHelper
        virtual void SAL_CALL disposing(void);

    // ::com::sun::star::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        // XNamed
        virtual ::rtl::OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setName( const ::rtl::OUString& _rName ) throw(::com::sun::star::uno::RuntimeException);

        /** return a pointer to the object which holds the UI-settings for this column, if any.
            @see    OColumnSettings
            @see    OColumns::loadSettings
        */
        virtual OColumnSettings*    getSettings();
        virtual void fireValueChange(const ::connectivity::ORowSetValue& _rOldValue);
    };

    //************************************************************
    //  OColumnSettings
    //************************************************************
    class OColumnSettings
    {
    //  <properties>
        ::com::sun::star::uno::Any  m_aWidth;               // sal_Int32 or void
        ::com::sun::star::uno::Any  m_aFormatKey;           // sal_Int32 or void
        ::com::sun::star::uno::Any  m_aRelativePosition;    // sal_Int32 or void
        ::com::sun::star::uno::Any  m_aAlignment;           // sal_Int32 (::com::sun::star::awt::TextAlign) or void
        ::com::sun::star::uno::Any  m_aHelpText;            // the description of the column which is visible in the helptext of the column
        ::com::sun::star::uno::Any  m_aControlDefault;      // the default value which should be displayed as by a control when moving to a new row
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                    m_xControlModel;

        sal_Bool                    m_bHidden;
    //  </properties>

    // Setting of values
    public:
        OColumnSettings();
        ~OColumnSettings();

        sal_Bool SAL_CALL convertFastPropertyValue(
                                ::com::sun::star::uno::Any & rConvertedValue,
                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue )
                                    throw (::com::sun::star::lang::IllegalArgumentException);
        void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                                     )
                                                     throw (::com::sun::star::uno::Exception);
        void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;

    public:
        /** write the connection independent information (i.e. the hidden flag or the column width) to the given stream.
            @param _rxConfigNode
                the configuratoin node to write to
            @param _rxFormats
                the number formats to use. In the configuration, for user defined formats the format descriptor (string/locale pair) is
                made persistent, not the format key. To obtain this descriptor, the number formatter is necessary.
            @return
                <TRUE/>, if anything has been written (i.e. there is at least one non-default property)
                <FALSE/> else
        */
        sal_Bool    writeUITo( const ::utl::OConfigurationNode& _rConfigNode, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& _rxFormats );
        /** read the connection independent information (i.e. te hidden flag or the column width) from the given configuration node.
            @param _rxConfigNode
                the configuratoin node to read from
            @param _rxFormats
                the number formats to use. In the configuration, for user defined formats the format descriptor (string/locale pair) is
                made persistent, not the format key. To obtain this descriptor, the number formatter is necessary.
        */
        void        readUIFrom(const ::utl::OConfigurationNode& _rConfigNode, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& _rxFormats);

        /** check if the persistent settings have their default value
        */
        sal_Bool    isDefaulted() const;
    };

    //------------------------------------------------------------
    DECLARE_STL_USTRINGACCESS_MAP( OColumnSettings*, MapName2Settings );

    //============================================================
    //= IColumnFactory - used by OColumns for creating new columns
    //============================================================
    class IColumnFactory
    {
    public:
        virtual OColumn*    createColumn(const ::rtl::OUString& _rName) const = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createEmptyObject() = 0;
    };

    //============================================================
    //= OColumns
    //= general columns map, could be used for readonly access
    //= no appending and dropping is supported
    //============================================================
    typedef ::std::hash_map<rtl::OUString, OColumn*, ::comphelper::UStringMixHash, ::comphelper::UStringMixEqual> OColumnMap;
    typedef ::std::vector<OColumn*> OColumnArray;

    class ODBTable;
    typedef connectivity::OColumnsHelper OColumns_BASE;
    //------------------------------------------------------------
    class OColumns : public OColumns_BASE
    {
        MapName2Settings            m_aSettings;
            // in case we are asked to load our UI, but do not have the columns, yet, we don't create them
            // instead we store the column settings and merge them into the real columns as soon as they're
            // inserted

    protected:
        // configuration
        ::utl::OConfigurationNode   m_aConfigurationNode;
        // comes from the driver can be null
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xDrvColumns;
        IColumnFactory*                             m_pColFactoryImpl;
        ::connectivity::sdbcx::IRefreshableColumns* m_pRefreshColumns;

        sal_Bool                                    m_bInitialized  : 1;
        sal_Bool                                    m_bAddColumn    : 1;
        sal_Bool                                    m_bDropColumn   : 1;

        virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > createObject(const ::rtl::OUString& _rName);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createEmptyObject();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > cloneObject(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xDescriptor);
        virtual void appendObject( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
        virtual void dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName);

    public:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > createBaseObject(const ::rtl::OUString& _rName)
        {
            return OColumns_BASE::createObject(_rName);
        }
        /** flag which determines whether the container is filled or not
        */
        sal_Bool    isInitialized() const { return m_bInitialized; }
        void        setInitialized() {m_bInitialized = sal_True;}

    public:
        /** constructs an empty container without configuration location.
            @param      rParent             the parent object. This instance will be used for refcounting, so the parent
                                            cannot die before the container does.
            @param      _rMutex             the mutex of the parent.
            @param      _bCaseSensitive     the initial case sensitivity flag
            @see        setCaseSensitive
        */
        OColumns(
                ::cppu::OWeakObject& _rParent,
                ::osl::Mutex& _rMutex,
                sal_Bool _bCaseSensitive,
                const ::std::vector< ::rtl::OUString>& _rVector,
                IColumnFactory* _pColFactory,
                ::connectivity::sdbcx::IRefreshableColumns* _pRefresh,
                sal_Bool _bAddColumn = sal_False,
                sal_Bool _bDropColumn = sal_False);

        OColumns(
            ::cppu::OWeakObject& _rParent,
            ::osl::Mutex& _rMutex,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxDrvColumns,
            sal_Bool _bCaseSensitive,
            const ::std::vector< ::rtl::OUString> &_rVector,
            IColumnFactory* _pColFactory,
            ::connectivity::sdbcx::IRefreshableColumns* _pRefresh,
            sal_Bool _bAddColumn = sal_False,
            sal_Bool _bDropColumn = sal_False);
        ~OColumns();

        //XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
        //XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // column settings persistence
        /** loads the columns UI information from the given configuration node.
            <p>If the container does contain any columns when this method is called, the informations are merged
            by name, i.e. for each column under the node the routine tries to find a column with the same name
            and overwrites it's settings, else it creates a new one.</p>

            @param _rxLocation
                the configuration node where the column UI informations are stored
            @see    OColumn::readUIFrom
            @see    storeSettings
        */
        virtual void    loadSettings(
            const ::utl::OConfigurationNode& _rLocation,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& _rxNumberFormats
        );

        /** store the columns configuration information under the current configuration node.
            @param  _rCommitLocation        Since the current configuration does not support different types of
                                            operations in one transaction, we have to commit before and after we
                                            create new nodes, thus ensuring that every transaction we do contains
                                            only one type of operation (insert/remove/update). Thus this commit node
                                            needs to be given, so every creation of a new set child (which is an insert
                                            operation) is flanked by two commits.
            @see    OColumn::writeUITo
            @see    loadSettings
        */
        virtual void    storeSettings(
            const ::utl::OConfigurationNode& _rLocation,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& _rxNumberFormats
        );

        void append(const ::rtl::OUString& rName, OColumn*);
        void clearColumns();
        // only the name is identical to ::cppu::OComponentHelper
        virtual void SAL_CALL disposing(void);

    private:
        void clearColumnSettings();
    };
}
#endif // _DBA_COREAPI_COLUMN_HXX_

