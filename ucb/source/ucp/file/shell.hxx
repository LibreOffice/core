/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#ifndef _SHELL_HXX_
#define _SHELL_HXX_


#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <list>
#include <osl/file.hxx>

#include "osl/mutex.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertiesChangeNotifier.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ucb/XPropertySetRegistryFactory.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/ContentInfo.hpp>
#include "filtask.hxx"
#include "filnot.hxx"

namespace fileaccess {

    class FileProvider;
    class XPropertySetInfo_impl;
    class XCommandInfo_impl;
    class XResultSet_impl;
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
            inline const rtl::OUString& SAL_CALL getPropertyName() const { return PropertyName; }
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

        typedef boost::unordered_set< MyProperty,hMyProperty,eMyProperty > PropertySet;
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

        typedef boost::unordered_map< UniquePath,UnqPathData,hUniquePath,eUniquePath > ContentMap;

    public:

        // MethodenDefinitionen
        shell( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMultiServiceFactory,
               FileProvider* pProvider,sal_Bool bWithConfig );

        virtual ~shell();



        /**
         *  This two methods register and deregister a change listener for the content belonging
         *  to URL aUnqPath
         */

        void SAL_CALL registerNotifier( const rtl::OUString& aUnqPath,Notifier* pNotifier );

        void SAL_CALL deregisterNotifier( const rtl::OUString& aUnqPath,Notifier* pNotifier );



        /**
         *  Used to associate and deassociate a new property with
         *  the content belonging to URL UnqPath.
         *  The default value and the the attributes are input
         */

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



        //
        //  Every method having a command id is not allowed to throw anything,
        //  but instead must install every error code in the task handler
        //


        /**
         *  Given an xOutputStream, this method writes the content of the file belonging to
         *  URL aUnqPath into the XOutputStream
         */

        void SAL_CALL page( sal_Int32 CommandId,
                            const rtl::OUString& aUnqPath,
                            const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xOutputStream )
            throw();


        /**
         *  Given a file URL aUnqPath, this methods returns a XInputStream which reads from the open file.
         */

        com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL
        open( sal_Int32 CommandId,
              const rtl::OUString& aUnqPath,
              sal_Bool bLock )
            throw();


        /**
         *  Given a file URL aUnqPath, this methods returns a XStream which can be used
         *  to read and write from/to the file.
         */

        com::sun::star::uno::Reference< com::sun::star::io::XStream > SAL_CALL
        open_rw( sal_Int32 CommandId,
                 const rtl::OUString& aUnqPath,
                 sal_Bool bLock )
            throw();


        /**
         *  This method returns the result set containing the the children of the directory belonging
         *  to file URL aUnqPath
         */

        com::sun::star::uno::Reference< com::sun::star::ucb::XDynamicResultSet > SAL_CALL
        ls( sal_Int32 CommandId,
            const rtl::OUString& aUnqPath,
            const sal_Int32 OpenMode,
            const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& sProperty,
            const com::sun::star::uno::Sequence< com::sun::star::ucb::NumberedSortingInfo > & sSortingInfo )
            throw();


        /**
         *  Info methods
         */

        // Info for commands
        com::sun::star::uno::Reference< com::sun::star::ucb::XCommandInfo > SAL_CALL
        info_c()
            throw();

        // Info for the properties
        com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL
        info_p( const rtl::OUString& aUnqPath )
            throw();


        /**
         *  Sets the values of the properties belonging to fileURL aUnqPath
         */

        com::sun::star::uno::Sequence< com::sun::star::uno::Any > SAL_CALL
        setv( const rtl::OUString& aUnqPath,
              const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& values )
            throw();


        /**
         *  Reads the values of the properties belonging to fileURL aUnqPath;
         *  Returns an XRow object containing the values in the requested order.
         */

        com::sun::star::uno::Reference< com::sun::star::sdbc::XRow > SAL_CALL
        getv( sal_Int32 CommandId,
              const rtl::OUString& aUnqPath,
              const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& properties )
            throw();


        /********************************************************************************/
        /*                         transfer-commands                                    */
        /********************************************************************************/

        /**
         *  Moves the content belonging to fileURL srcUnqPath to fileURL dstUnqPath( files and directories )
         */

        void SAL_CALL
        move( sal_Int32 CommandId,
              const rtl::OUString srcUnqPath,   // Full file(folder)-path
              const rtl::OUString dstUnqPath,   // Path to the destination-directory
              const sal_Int32 NameClash )
            throw();

        /**
         *  Copies the content belonging to fileURL srcUnqPath to fileURL dstUnqPath ( files and directories )
         */

        void SAL_CALL
        copy( sal_Int32 CommandId,               // See "move"
              const rtl::OUString srcUnqPath,
              const rtl::OUString dstUnqPath,
              sal_Int32 NameClash )
            throw();

#define RemoveFolder   1
#define RemoveFile    -1
#define RemoveUnknown  0

        /**
         *  Deletes the content belonging to fileURL aUnqPath( recursively in case of directory )
         */

        sal_Bool SAL_CALL
        remove( sal_Int32 CommandId,
                const rtl::OUString& aUnqPath,
                sal_Int32 TypeToMove = RemoveUnknown,
                sal_Bool  MustExist  = sal_True )
            throw();

#undef RemoveUnknown
#undef RemoveFile
#undef RemoveFolder


        /********************************************************************************/
        /*                         write and create - commandos                         */
        /********************************************************************************/

        /**
         *  Creates new directory with given URL, recursively if necessary
         *  Return:: success of operation
         */

        sal_Bool SAL_CALL
        mkdir( sal_Int32 CommandId,
               const rtl::OUString& aDirectoryName,
               sal_Bool OverWrite )
            throw();


        /**
         *  Creates new file with given URL.
         *  The content of aInputStream becomes the content of the file
         *  Return:: success of operation
         */

        sal_Bool SAL_CALL
        mkfil( sal_Int32 CommandId,
               const rtl::OUString& aFileName,
               sal_Bool OverWrite,
               const com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& aInputStream )
            throw();


        /**
         *  writes to the file with given URL.
         *  The content of aInputStream becomes the content of the file
         *  Return:: success of operation
         */
        sal_Bool SAL_CALL
        write( sal_Int32 CommandId,
               const rtl::OUString& aUnqPath,
               sal_Bool OverWrite,
               const com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& aInputStream )
            throw();



        void SAL_CALL insertDefaultProperties( const rtl::OUString& aUnqPath );

        com::sun::star::uno::Sequence< com::sun::star::ucb::ContentInfo >
        queryCreatableContentsInfo();


        /******************************************************************************/
        /*                                                                            */
        /*                          mapping of file urls                              */
        /*                          to uncpath and vice versa                         */
        /*                                                                            */
        /******************************************************************************/

        sal_Bool SAL_CALL getUnqFromUrl( const rtl::OUString& Url, rtl::OUString& Unq );

        sal_Bool SAL_CALL getUrlFromUnq( const rtl::OUString& Unq, rtl::OUString& Url );


        sal_Bool m_bWithConfig;
        FileProvider*                                                                   m_pProvider;
        com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >    m_xMultiServiceFactory;
        com::sun::star::uno::Reference< com::sun::star::ucb::XPropertySetRegistry >     m_xFileRegistry;

    private:

        /********************************************************************************/
        /*                              get eventListeners                              */
        /********************************************************************************/

        std::list< ContentEventNotifier* >* SAL_CALL
        getContentEventListeners( const rtl::OUString& aName );

        std::list< ContentEventNotifier* >* SAL_CALL
        getContentDeletedEventListeners( const rtl::OUString& aName );

        std::vector< std::list< ContentEventNotifier* >* >* SAL_CALL
        getContentExchangedEventListeners( const rtl::OUString aOldPrefix,
                                           const rtl::OUString aNewPrefix,
                                           sal_Bool withChildren );

        std::list< PropertyChangeNotifier* >* SAL_CALL
        getPropertyChangeNotifier( const rtl::OUString& aName );

        std::list< PropertySetInfoChangeNotifier* >* SAL_CALL
        getPropertySetListeners( const rtl::OUString& aName );


        /********************************************************************************/
        /*                              notify eventListeners                           */
        /********************************************************************************/

        void SAL_CALL notifyPropertyChanges(
            std::list< PropertyChangeNotifier* >* listeners,
            const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyChangeEvent >& seqChanged );

        void SAL_CALL notifyContentExchanged(
            std::vector< std::list< ContentEventNotifier* >* >* listeners_vec );

        void SAL_CALL notifyInsert(
            std::list< ContentEventNotifier* >* listeners,const rtl::OUString& aChildName );

        void SAL_CALL notifyContentDeleted(
            std::list< ContentEventNotifier* >* listeners );

        void SAL_CALL notifyContentRemoved(
            std::list< ContentEventNotifier* >* listeners,
            const rtl::OUString& aChildName );

        void SAL_CALL notifyPropertyAdded(
            std::list< PropertySetInfoChangeNotifier* >* listeners,
            const rtl::OUString& aPropertyName );

        void SAL_CALL notifyPropertyRemoved(
            std::list< PropertySetInfoChangeNotifier* >* listeners,
            const rtl::OUString& aPropertyName );


        /********************************************************************************/
        /*                       remove persistent propertyset                          */
        /********************************************************************************/

        void SAL_CALL erasePersistentSet( const rtl::OUString& aUnqPath,
                                          sal_Bool withChildren = false );

        /********************************************************************************/
        /*                       copy persistent propertyset                            */
        /*                       from srcUnqPath to dstUnqPath                          */
        /********************************************************************************/

        void SAL_CALL copyPersistentSet( const rtl::OUString& srcUnqPath,
                                         const rtl::OUString& dstUnqPath,
                                         sal_Bool withChildren = false );


        // Special optimized method for getting the properties of a directoryitem, which
        // is returned by osl::DirectoryItem::getNextItem()

        com::sun::star::uno::Reference< com::sun::star::sdbc::XRow > SAL_CALL
        getv( Notifier* pNotifier,
              const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& properties,
              osl::DirectoryItem& DirItem,
              rtl::OUString& aUnqPath,
              sal_Bool&      bIsRegular );


        /**
         *  Load the properties from configuration, if create == true create them.
         *  The Properties are stored under the url belonging to it->first.
         */

        void SAL_CALL load( const shell::ContentMap::iterator& it,
                            sal_Bool create );

        /**
         *  Commit inserts the determined properties in the filestatus object into
         *  the internal map, so that is possible to determine on a subsequent
         *  setting of file properties which properties have changed without filestat
         */

        void SAL_CALL
        commit(
            const shell::ContentMap::iterator& it,
            const osl::FileStatus& aFileStatus );

        /**
         *  Given a Sequence of properties seq, this method determines the mask
         *  used to instantiate a osl::FileStatus, so that a call to
         *  osl::DirectoryItem::getFileStatus fills the required fields.
         */

        void SAL_CALL
        getMaskFromProperties(
            sal_Int32& n_Mask,
            const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& seq );


        void SAL_CALL
        setFileProperties(
            const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& values,
            sal_Int32 numberOfValues )
            throw();


        // Helper function for public copy

        osl::FileBase::RC SAL_CALL
        copy_recursive(
            const rtl::OUString& srcUnqPath,
            const rtl::OUString& dstUnqPath,
            sal_Int32 TypeToCopy,
            sal_Bool  testExistence )
            throw();


        // Helper function for mkfil,mkdir and write
        // Creates whole path
        // returns success of the operation
        // The calle determines the errorCode, which should be used to install
        // any error

        sal_Bool SAL_CALL
        ensuredir( sal_Int32 CommandId,
                   const rtl::OUString& aDirectoryName,
                   sal_Int32 errorCode )
            throw();

        // General
        osl::Mutex m_aMutex;
        ContentMap  m_aContent;

        // Default properties

        const rtl::OUString Title;
        const rtl::OUString CasePreservingURL;
        const rtl::OUString IsDocument;
        const rtl::OUString IsFolder;
        const rtl::OUString DateModified;
        const rtl::OUString Size;
        const rtl::OUString IsVolume;
        const rtl::OUString IsRemoveable;
        const rtl::OUString IsRemote;
        const rtl::OUString IsCompactDisc;
        const rtl::OUString IsFloppy;
        const rtl::OUString IsHidden;
        const rtl::OUString ContentType;
        const rtl::OUString IsReadOnly;
        const rtl::OUString CreatableContentsInfo;

    public:

        const rtl::OUString FolderContentType;
        const rtl::OUString FileContentType;


    private:

        PropertySet                                                         m_aDefaultProperties;
        com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >   m_sCommandInfo;

    public:
        // Misceancellous:
        // Methods for "writeComponentInfo" and "createComponentFactory"

        static void SAL_CALL getScheme( rtl::OUString& Scheme );

        static rtl::OUString SAL_CALL getImplementationName_static( void );

        static com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames_static( void );

    };    // end class shell

}             // end namespace fileaccess

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
