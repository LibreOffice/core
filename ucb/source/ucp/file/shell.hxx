/*************************************************************************
 *
 *  $RCSfile: shell.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:53:36 $
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


#ifndef _SHELL_HXX_
#define _SHELL_HXX_


#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef __SGI_STL_VECTOR
#include <stl/vector>
#endif
#ifndef __SGI_STL_HASH_MAP
#include <stl/hash_map>
#endif
#ifndef __SGI_STL_HASH_SET
#include <stl/hash_set>
#endif
#ifndef _SGI_STL_LIST
#include <stl/list>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_protected
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGENOTIFIER_HPP_
#include <com/sun/star/beans/XPropertiesChangeNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NUMBEREDSORTINGINFO_HPP_
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XDYNAMICRESULTSET_HPP__
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROPERTYSETREGISTRYFACTORY_HPP_
#include <com/sun/star/ucb/XPropertySetRegistryFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _FILTASK_HXX_
#include "filtask.hxx"
#endif
#ifndef _FILNOT_HXX_
#include "filnot.hxx"
#endif

namespace fileaccess {

    class FileProvider;
    class XPropertySetInfo_impl;
    class XCommandInfo_impl;
    class XResultSet_impl;
    class BaseContent;
    class shell;

    class shell
        : public virtual TaskManager
    {
        friend class XPropertySetInfo_impl;
        friend class XResultSet_impl;
        friend class XCommandInfo_impl;
    public:
        // Type definitions

        typedef rtl::OUString UniquePath;
        typedef equalOUString eUniquePath;
        typedef hashOUString hUniquePath;

        class MyProperty
        {
        private:
            rtl::OUString                          PropertyName;
            sal_Int32                              Handle;
            sal_Bool                               isNative;
            com::sun::star::uno::Type              Typ;        // Duplicates information in Value
            com::sun::star::uno::Any               Value;
            com::sun::star::beans::PropertyState   State;
            sal_Int16                              Attributes;
        public:
            MyProperty();
            MyProperty( const rtl::OUString&                         __PropertyName );
            MyProperty( const sal_Bool&                              __isNative,
                        const rtl::OUString&                         __PropertyName,
                        const sal_Int32&                             __Handle,
                        const com::sun::star::uno::Type&             __Typ,
                        const com::sun::star::uno::Any&              __Value,
                        const com::sun::star::beans::PropertyState&  __State,
                        const sal_Int16&                             __Attributes );

            ~MyProperty();
            inline const sal_Bool& SAL_CALL IsNative() const;
            inline const rtl::OUString& SAL_CALL getPropertyName() const;
            inline const sal_Int32& SAL_CALL getHandle() const;
            inline const com::sun::star::uno::Type& SAL_CALL getType() const;
            inline const com::sun::star::uno::Any& SAL_CALL getValue() const;
            inline const com::sun::star::beans::PropertyState& SAL_CALL getState() const;
            inline const sal_Int16& SAL_CALL getAttributes() const;

            // The set* functions are declared const, because the key of "this" stays intact
            inline void SAL_CALL setHandle( const sal_Int32&  __Handle ) const;
            inline void SAL_CALL setType( const com::sun::star::uno::Type& __Type ) const;
            inline void SAL_CALL setValue( const com::sun::star::uno::Any& __Value ) const;
            inline void SAL_CALL setState( const com::sun::star::beans::PropertyState& __State ) const;
            inline void SAL_CALL setAttributes( const sal_Int16& __Attributes ) const;
        };

        struct eMyProperty
        {
            bool operator()( const MyProperty& rKey1, const MyProperty& rKey2 ) const
            {
                return !!(  rKey1.getPropertyName() == rKey2.getPropertyName() );
            }
        };

        struct hMyProperty
        {
            size_t operator()( const MyProperty& rName ) const
            {
                return rName.getPropertyName().hashCode();
            }
        };

        typedef std::hash_set< MyProperty,hMyProperty,eMyProperty > PropertySet;
        typedef std::list< Notifier* >                              NotifierList;


        class UnqPathData
        {
        public:
            UnqPathData();
            ~UnqPathData();
            UnqPathData( const UnqPathData& );
            UnqPathData& operator=( UnqPathData& );

            PropertySet*               properties;
            NotifierList*              notifier;

            // Three views on the PersistentPropertySet
            com::sun::star::uno::Reference< com::sun::star::ucb::XPersistentPropertySet >   xS;
            com::sun::star::uno::Reference< com::sun::star::beans::XPropertyContainer >     xC;
            com::sun::star::uno::Reference< com::sun::star::beans::XPropertyAccess >        xA;
        };

        typedef std::hash_map< UniquePath,UnqPathData,hUniquePath,eUniquePath > ContentMap;

    public:
        // MethodenDefinitionen
        shell( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMultiServiceFactory,
               FileProvider* pProvider );

        virtual ~shell();

        void SAL_CALL registerNotifier( const rtl::OUString& aUnqPath,Notifier* pNotifier );
        void SAL_CALL deregisterNotifier( const rtl::OUString& aUnqPath,Notifier* pNotifier );


        void SAL_CALL associate( const rtl::OUString& UnqPath,
                                         const rtl::OUString& PropertyName,
                                         const com::sun::star::uno::Any& DefaultValue,
                                 const sal_Int16 Attributes )
            throw( com::sun::star::beans::PropertyExistException,
                   com::sun::star::beans::IllegalTypeException,
                   com::sun::star::uno::RuntimeException);


        void SAL_CALL deassociate( const rtl::OUString& UnqPath,
                                   const rtl::OUString& PropertyName )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::beans::NotRemoveableException,
                   com::sun::star::uno::RuntimeException);

        com::sun::star::uno::Reference< com::sun::star::ucb::XDynamicResultSet > SAL_CALL
        ls( sal_Int32 CommandId,
            const rtl::OUString& aUnqPath,
            const sal_Int32 OpenMode,
            const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& sProperty,
            const com::sun::star::uno::Sequence< com::sun::star::ucb::NumberedSortingInfo > & sSortingInfo );


        com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL
        open( sal_Int32 CommandId,
              const rtl::OUString& aUnqPath );

        com::sun::star::uno::Reference< com::sun::star::io::XStream > SAL_CALL
        open_rw( sal_Int32 CommandId,
                 const rtl::OUString& aUnqPath );

        void SAL_CALL page( sal_Int32 CommandId,
                            const rtl::OUString& aUnqPath,
                            const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xOutputStream )
            throw( com::sun::star::ucb::CommandAbortedException );

        // Info for the properties
        com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL
        info_p( sal_Int32 CommandId,
                const rtl::OUString& aUnqPath );

        // Info for commands
        com::sun::star::uno::Reference< com::sun::star::ucb::XCommandInfo > SAL_CALL
        info_c( sal_Int32 CommandId,
                const rtl::OUString& aUnqPath );


        // Setting values
        void SAL_CALL setv( sal_Int32 CommandId,
                            const rtl::OUString& aUnqPath,
                            const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& values );

        // Reading values
        com::sun::star::uno::Reference< com::sun::star::sdbc::XRow > SAL_CALL
        getv( sal_Int32 CommandId,
              const rtl::OUString& aUnqPath,
              const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& properties );

        //
        void SAL_CALL
        move( sal_Int32 CommandId,
              const rtl::OUString srcUnqPath,   // Full file(folder)-path
              const rtl::OUString dstUnqPath,   // Path to the destination-directory
              const sal_Int32 NameClash )
            throw( com::sun::star::ucb::CommandAbortedException );

        void SAL_CALL
        copy( sal_Int32 CommandId,               // See "move"
              const rtl::OUString srcUnqPath,
              const rtl::OUString dstUnqPath,
              sal_Int32 NameClash )
            throw( com::sun::star::ucb::CommandAbortedException );

#define RemoveFolder   1
#define RemoveFile    -1
#define RemoveUnknown  0

        void SAL_CALL
        remove( sal_Int32 CommandId,
                const rtl::OUString& aUnqPath,
                sal_Int32 TypeToMove = RemoveUnknown );

#undef RemoveUnknown
#undef RemoveFile
#undef RemoveFolder


        sal_Bool SAL_CALL                                 // Creates new directory
        mkdir( sal_Int32 CommandId,                       // returns success
               const rtl::OUString& aDirectoryName );


        sal_Bool SAL_CALL                             // Returns success
        mkfil( sal_Int32 CommandId,                   // Creates new file
               const rtl::OUString& aFileName,      // returns success
               sal_Bool OverWrite,
               const com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& aInputStream );

        sal_Bool SAL_CALL                             // Returns success
        write( sal_Int32 CommandId,
               const rtl::OUString& aUnqPath,
               sal_Bool OverWrite,
               const com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& aInputStream );


        void SAL_CALL InsertDefaultProperties( const rtl::OUString& aUnqPath );
        static void SAL_CALL getScheme( rtl::OUString& Scheme );
        sal_Bool SAL_CALL getUnqFromUrl( const rtl::OUString& Url, rtl::OUString& Unq );
        sal_Bool SAL_CALL getUrlFromUnq( const rtl::OUString& Unq, rtl::OUString& Url );
        rtl::OUString SAL_CALL getParentName( const rtl::OUString& aFileName );




        // Methods for "writeComponentInfo" and "createComponentFactory"
        static rtl::OUString SAL_CALL getImplementationName_static( void );
        static com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames_static( void );


        FileProvider*                                                                   m_pProvider;
        com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >    m_xMultiServiceFactory;
        com::sun::star::uno::Reference< com::sun::star::ucb::XPropertySetRegistry >     m_xFileRegistry;
    private:
        // Get EventListeners
        std::list< ContentEventNotifier* >* SAL_CALL getContentEventListeners( const rtl::OUString& aName );
        std::list< ContentEventNotifier* >* SAL_CALL getContentDeletedEventListeners( const rtl::OUString& aName );
        std::vector< std::list< ContentEventNotifier* >* >* SAL_CALL getContentExchangedEventListeners(
            const rtl::OUString aOldPrefix,
            const rtl::OUString aNewPrefix,
            sal_Bool withChilds );
        std::list< PropertyChangeNotifier* >* SAL_CALL getPropertyChangeNotifier( const rtl::OUString& aName );
        std::list< PropertySetInfoChangeNotifier* >* SAL_CALL getPropertySetListeners( const rtl::OUString& aName );


        // Notifications
        void SAL_CALL notifyPropertyChanges( std::list< PropertyChangeNotifier* >* listeners,
                                             const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyChangeEvent >&
                                             seqChanged );

        void SAL_CALL notifyContentExchanged( std::vector< std::list< ContentEventNotifier* >* >* listeners_vec );

        void SAL_CALL notifyInsert( std::list< ContentEventNotifier* >* listeners,const rtl::OUString& aChildName );

        void SAL_CALL notifyContentDeleted( std::list< ContentEventNotifier* >* listeners );

        void SAL_CALL notifyContentRemoved( std::list< ContentEventNotifier* >* listeners,
                                            const rtl::OUString& aChildName );



        void SAL_CALL notifyPropertyAdded( std::list< PropertySetInfoChangeNotifier* >* listeners,
                                           const rtl::OUString& aPropertyName );

        void SAL_CALL notifyPropertyRemoved( std::list< PropertySetInfoChangeNotifier* >* listeners,
                                             const rtl::OUString& aPropertyName );

        // Methods

        void SAL_CALL erasePersistentSet( const rtl::OUString& aUnqPath,
                                          sal_Bool withChilds = false );

        void SAL_CALL copyPersistentSet( const rtl::OUString& srcUnqPath,
                                         const rtl::OUString& dstUnqPath,
                                         sal_Bool withChilds = false );

        // Special optimized method for getting the properties of a directoryitem
        com::sun::star::uno::Reference< com::sun::star::sdbc::XRow > SAL_CALL
        getv( sal_Int32 CommandId,
              Notifier* pNotifier,
              const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& properties,
              osl::DirectoryItem& DirItem,
              rtl::OUString& aUnqPath,
              sal_Bool&      bIsRegular );


        void SAL_CALL
        getMaskFromProperties(
            sal_Int32& n_Mask,
            const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& seq );

        void SAL_CALL
        load( const shell::ContentMap::iterator& it, sal_Bool create );

        void SAL_CALL
        commit(
            const shell::ContentMap::iterator& it,
            const osl::FileStatus& aFileStatus );

        void SAL_CALL
        setFileProperties( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& values,
                           sal_Int32 numberOfValues );

        osl::FileBase::RC SAL_CALL copy_recursive( const rtl::OUString& srcUnqPath,
                                                   const rtl::OUString& dstUnqPath,
                                                   sal_Int32 TypeToCopy );

        // General
        vos::OMutex m_aMutex;
        ContentMap  m_aContent;

        // Default properties

        const rtl::OUString Title;
        const rtl::OUString IsDocument;
        const rtl::OUString IsFolder;
        const rtl::OUString DateCreated;
        const rtl::OUString DateModified;
        const rtl::OUString Size;
        const rtl::OUString FolderCount;
        const rtl::OUString DocumentCount;
        const rtl::OUString ContentType;
        const rtl::OUString IsReadOnly;

    public:
        const rtl::OUString FolderContentType;
        const rtl::OUString FileContentType;

    private:
        PropertySet                                                         m_aDefaultProperties;

        // Commands
        com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >   m_sCommandInfo;

    public:

        sal_Bool m_bFaked;

        struct MountPoint
        {
            MountPoint( const rtl::OUString& aMountPoint,
                        const rtl::OUString& aDirectory );

            rtl::OUString m_aTitle;
            rtl::OUString m_aMountPoint;
            rtl::OUString m_aDirectory;
        };

        std::vector< MountPoint > m_vecMountPoint;

        sal_Bool SAL_CALL checkMountPoint( const rtl::OUString&  aUnqPath,
                                           rtl::OUString&        aRedirectedPath );

        sal_Bool SAL_CALL uncheckMountPoint( const rtl::OUString&  aUnqPath,
                                             rtl::OUString&        aRedirectedPath );

    };
}             // end namespace fileaccess

#endif

