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


#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_SHELL_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_SHELL_HXX


#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/typeprovider.hxx>
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
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ucb/XPropertySetRegistryFactory.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/ContentInfo.hpp>
#include "filtask.hxx"
#include "filnot.hxx"
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

        class MyProperty
        {
        private:
            OUString                    PropertyName;
            sal_Int32                   Handle;
            bool                        isNative;
            css::uno::Type              Typ;        // Duplicates information in Value
            css::uno::Any               Value;
            css::beans::PropertyState   State;
            sal_Int16                   Attributes;
        public:
            MyProperty();
            explicit MyProperty( const OUString&  __PropertyName );
            MyProperty( const bool&                       __isNative,
                        const OUString&                   __PropertyName,
                        const sal_Int32&                  __Handle,
                        const css::uno::Type&             __Typ,
                        const css::uno::Any&              __Value,
                        const css::beans::PropertyState&  __State,
                        const sal_Int16&                  __Attributes );

            ~MyProperty();
            inline const bool& SAL_CALL IsNative() const;
            inline const OUString& SAL_CALL getPropertyName() const { return PropertyName; }
            inline const sal_Int32& SAL_CALL getHandle() const;
            inline const css::uno::Type& SAL_CALL getType() const;
            inline const css::uno::Any& SAL_CALL getValue() const;
            inline const css::beans::PropertyState& SAL_CALL getState() const;
            inline const sal_Int16& SAL_CALL getAttributes() const;

            // The set* functions are declared const, because the key of "this" stays intact
            inline void SAL_CALL setValue( const css::uno::Any& __Value ) const;
            inline void SAL_CALL setState( const css::beans::PropertyState& __State ) const;
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

        typedef std::unordered_set< MyProperty,hMyProperty,eMyProperty > PropertySet;
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
            css::uno::Reference< css::ucb::XPersistentPropertySet >   xS;
            css::uno::Reference< css::beans::XPropertyContainer >     xC;
            css::uno::Reference< css::beans::XPropertyAccess >        xA;
        };

        typedef std::unordered_map< OUString,UnqPathData,OUStringHash > ContentMap;

    public:

        // MethodenDefinitionen
        shell( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
               FileProvider* pProvider, bool bWithConfig );

        virtual ~shell();



        /**
         *  This two methods register and deregister a change listener for the content belonging
         *  to URL aUnqPath
         */

        void SAL_CALL registerNotifier( const OUString& aUnqPath,Notifier* pNotifier );

        void SAL_CALL deregisterNotifier( const OUString& aUnqPath,Notifier* pNotifier );



        /**
         *  Used to associate and deassociate a new property with
         *  the content belonging to URL UnqPath.
         *  The default value and the attributes are input
         */

        void SAL_CALL associate( const OUString& UnqPath,
                                 const OUString& PropertyName,
                                 const css::uno::Any& DefaultValue,
                                 const sal_Int16 Attributes )
            throw( css::beans::PropertyExistException,
                   css::beans::IllegalTypeException,
                   css::uno::RuntimeException);


        void SAL_CALL deassociate( const OUString& UnqPath,
                                   const OUString& PropertyName )
            throw( css::beans::UnknownPropertyException,
                   css::beans::NotRemoveableException,
                   css::uno::RuntimeException);




        //  Every method having a command id is not allowed to throw anything,
        //  but instead must install every error code in the task handler



        /**
         *  Given an xOutputStream, this method writes the content of the file belonging to
         *  URL aUnqPath into the XOutputStream
         */

        void SAL_CALL page( sal_Int32 CommandId,
                            const OUString& aUnqPath,
                            const css::uno::Reference< css::io::XOutputStream >& xOutputStream );


        /**
         *  Given a file URL aUnqPath, this methods returns a XInputStream which reads from the open file.
         */

        css::uno::Reference< css::io::XInputStream > SAL_CALL
        open( sal_Int32 CommandId,
              const OUString& aUnqPath,
              bool bLock );


        /**
         *  Given a file URL aUnqPath, this methods returns a XStream which can be used
         *  to read and write from/to the file.
         */

        css::uno::Reference< css::io::XStream > SAL_CALL
        open_rw( sal_Int32 CommandId,
                 const OUString& aUnqPath,
                 bool bLock );


        /**
         *  This method returns the result set containing the children of the directory belonging
         *  to file URL aUnqPath
         */

        css::uno::Reference< css::ucb::XDynamicResultSet > SAL_CALL
        ls( sal_Int32 CommandId,
            const OUString& aUnqPath,
            const sal_Int32 OpenMode,
            const css::uno::Sequence< css::beans::Property >& sProperty,
            const css::uno::Sequence< css::ucb::NumberedSortingInfo > & sSortingInfo );


        /**
         *  Info methods
         */

        // Info for commands
        css::uno::Reference< css::ucb::XCommandInfo > SAL_CALL
        info_c();

        // Info for the properties
        css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        info_p( const OUString& aUnqPath );


        /**
         *  Sets the values of the properties belonging to fileURL aUnqPath
         */

        css::uno::Sequence< css::uno::Any > SAL_CALL
        setv( const OUString& aUnqPath,
              const css::uno::Sequence< css::beans::PropertyValue >& values );


        /**
         *  Reads the values of the properties belonging to fileURL aUnqPath;
         *  Returns an XRow object containing the values in the requested order.
         */

        css::uno::Reference< css::sdbc::XRow > SAL_CALL
        getv( sal_Int32 CommandId,
              const OUString& aUnqPath,
              const css::uno::Sequence< css::beans::Property >& properties );


        /********************************************************************************/
        /*                         transfer-commands                                    */
        /********************************************************************************/

        /**
         *  Moves the content belonging to fileURL srcUnqPath to fileURL dstUnqPath( files and directories )
         */

        void SAL_CALL
        move( sal_Int32 CommandId,
              const OUString& srcUnqPath,   // Full file(folder)-path
              const OUString& dstUnqPath,   // Path to the destination-directory
              const sal_Int32 NameClash );

        /**
         *  Copies the content belonging to fileURL srcUnqPath to fileURL dstUnqPath ( files and directories )
         */

        void SAL_CALL
        copy( sal_Int32 CommandId,               // See "move"
              const OUString& srcUnqPath,
              const OUString& dstUnqPath,
              sal_Int32 NameClash );

        enum class FileUrlType { Folder = 1, File = -1, Unknown = 0 };

        /**
         *  Deletes the content belonging to fileURL aUnqPath( recursively in case of directory )
         */

        bool SAL_CALL
        remove( sal_Int32 CommandId,
                const OUString& aUnqPath,
                FileUrlType eTypeToMove = FileUrlType::Unknown,
                bool  MustExist  = true );


        /********************************************************************************/
        /*                         write and create - commandos                         */
        /********************************************************************************/

        /**
         *  Creates new directory with given URL, recursively if necessary
         *  Return:: success of operation
         */

        bool SAL_CALL
        mkdir( sal_Int32 CommandId,
               const OUString& aDirectoryName,
               bool OverWrite );


        /**
         *  Creates new file with given URL.
         *  The content of aInputStream becomes the content of the file
         *  Return:: success of operation
         */

        bool SAL_CALL
        mkfil( sal_Int32 CommandId,
               const OUString& aFileName,
               bool OverWrite,
               const css::uno::Reference< css::io::XInputStream >& aInputStream );


        /**
         *  writes to the file with given URL.
         *  The content of aInputStream becomes the content of the file
         *  Return:: success of operation
         */
        bool SAL_CALL
        write( sal_Int32 CommandId,
               const OUString& aUnqPath,
               bool OverWrite,
               const css::uno::Reference< css::io::XInputStream >& aInputStream );



        void SAL_CALL insertDefaultProperties( const OUString& aUnqPath );

        css::uno::Sequence< css::ucb::ContentInfo >
        queryCreatableContentsInfo();


        /******************************************************************************/
        /*                                                                            */
        /*                          mapping of file urls                              */
        /*                          to uncpath and vice versa                         */
        /*                                                                            */
        /******************************************************************************/

        static bool SAL_CALL getUnqFromUrl( const OUString& Url, OUString& Unq );

        static bool SAL_CALL getUrlFromUnq( const OUString& Unq, OUString& Url );


        bool m_bWithConfig;
        FileProvider*                                             m_pProvider;
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        css::uno::Reference< css::ucb::XPropertySetRegistry >     m_xFileRegistry;

    private:

        /********************************************************************************/
        /*                              get eventListeners                              */
        /********************************************************************************/

        std::list< ContentEventNotifier* >* SAL_CALL
        getContentEventListeners( const OUString& aName );

        std::list< ContentEventNotifier* >* SAL_CALL
        getContentDeletedEventListeners( const OUString& aName );

        std::vector< std::list< ContentEventNotifier* >* >* SAL_CALL
        getContentExchangedEventListeners( const OUString& aOldPrefix,
                                           const OUString& aNewPrefix,
                                           bool withChildren );

        std::list< PropertyChangeNotifier* >* SAL_CALL
        getPropertyChangeNotifier( const OUString& aName );

        std::list< PropertySetInfoChangeNotifier* >* SAL_CALL
        getPropertySetListeners( const OUString& aName );


        /********************************************************************************/
        /*                              notify eventListeners                           */
        /********************************************************************************/

        static void SAL_CALL notifyPropertyChanges(
            std::list< PropertyChangeNotifier* >* listeners,
            const css::uno::Sequence< css::beans::PropertyChangeEvent >& seqChanged );

        static void SAL_CALL notifyContentExchanged(
            std::vector< std::list< ContentEventNotifier* >* >* listeners_vec );

        static void SAL_CALL notifyInsert(
            std::list< ContentEventNotifier* >* listeners,const OUString& aChildName );

        static void SAL_CALL notifyContentDeleted(
            std::list< ContentEventNotifier* >* listeners );

        static void SAL_CALL notifyContentRemoved(
            std::list< ContentEventNotifier* >* listeners,
            const OUString& aChildName );

        static void SAL_CALL notifyPropertyAdded(
            std::list< PropertySetInfoChangeNotifier* >* listeners,
            const OUString& aPropertyName );

        static void SAL_CALL notifyPropertyRemoved(
            std::list< PropertySetInfoChangeNotifier* >* listeners,
            const OUString& aPropertyName );


        /********************************************************************************/
        /*                       remove persistent propertyset                          */
        /********************************************************************************/

        void SAL_CALL erasePersistentSet( const OUString& aUnqPath,
                                          bool withChildren = false );

        /********************************************************************************/
        /*                       copy persistent propertyset                            */
        /*                       from srcUnqPath to dstUnqPath                          */
        /********************************************************************************/

        void SAL_CALL copyPersistentSet( const OUString& srcUnqPath,
                                         const OUString& dstUnqPath,
                                         bool withChildren = false );


        // Special optimized method for getting the properties of a directoryitem, which
        // is returned by osl::DirectoryItem::getNextItem()

        css::uno::Reference< css::sdbc::XRow > SAL_CALL
        getv( Notifier* pNotifier,
              const css::uno::Sequence< css::beans::Property >& properties,
              osl::DirectoryItem& DirItem,
              OUString& aUnqPath,
              bool&      bIsRegular );


        /**
         *  Load the properties from configuration, if create == true create them.
         *  The Properties are stored under the url belonging to it->first.
         */

        void SAL_CALL load( const shell::ContentMap::iterator& it,
                            bool create );

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
            const css::uno::Sequence< css::beans::Property >& seq );


        // Helper function for public copy

        osl::FileBase::RC SAL_CALL
        copy_recursive(
            const OUString& srcUnqPath,
            const OUString& dstUnqPath,
            FileUrlType TypeToCopy,
            bool  testExistence );


        // Helper function for mkfil,mkdir and write
        // Creates whole path
        // returns success of the operation
        // The calle determines the errorCode, which should be used to install
        // any error

        bool SAL_CALL
        ensuredir( sal_Int32 CommandId,
                   const OUString& aDirectoryName,
                   sal_Int32 errorCode );

        // General
        osl::Mutex m_aMutex;
        ContentMap  m_aContent;

        // Default properties

        const OUString Title;
        const OUString CasePreservingURL;
        const OUString IsDocument;
        const OUString IsFolder;
        const OUString DateModified;
        const OUString Size;
        const OUString IsVolume;
        const OUString IsRemoveable;
        const OUString IsRemote;
        const OUString IsCompactDisc;
        const OUString IsFloppy;
        const OUString IsHidden;
        const OUString ContentType;
        const OUString IsReadOnly;
        const OUString CreatableContentsInfo;

    public:

        const OUString FolderContentType;
        const OUString FileContentType;


    private:

        PropertySet                                   m_aDefaultProperties;
        css::uno::Sequence< css::ucb::CommandInfo >   m_sCommandInfo;

    public:
        // Misceancellous:
        // Methods for "writeComponentInfo" and "createComponentFactory"

        static void SAL_CALL getScheme( OUString& Scheme );

        static OUString SAL_CALL getImplementationName_static();

        static css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static();

    };    // end class shell

}             // end namespace fileaccess

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
