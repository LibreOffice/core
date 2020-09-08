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
#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_FILTASK_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_FILTASK_HXX

#include <osl/file.hxx>
#include <rtl/ustring.hxx>

#include <osl/mutex.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ucb/ContentInfo.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <com/sun/star/ucb/XPropertySetRegistry.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include "filerror.hxx"
#include "filnot.hxx"
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fileaccess
{
    class BaseContent;
    class FileProvider;
    class XPropertySetInfo_impl;
    class XCommandInfo_impl;
    class XResultSet_impl;

    /*
     * The relevant methods in this class all have as first argument the CommandId,
     * so if necessary, every method has access to its relevant XInteractionHandler and
     * XProgressHandler.
     */


    class TaskManager
    {
        friend class XPropertySetInfo_impl;
        friend class XResultSet_impl;
        friend class XCommandInfo_impl;

    private:

        class TaskHandling
        {
        private:

            bool m_bHandled;
            sal_Int32 m_nErrorCode,m_nMinorCode;
            css::uno::Reference< css::task::XInteractionHandler > m_xInteractionHandler;
            css::uno::Reference< css::ucb::XCommandEnvironment >  m_xCommandEnvironment;


        public:

            explicit TaskHandling(
                const css::uno::Reference< css::ucb::XCommandEnvironment >&  xCommandEnv )
                : m_bHandled( false ),
                  m_nErrorCode( TASKHANDLER_NO_ERROR ),
                  m_nMinorCode( TASKHANDLER_NO_ERROR ),
                  m_xCommandEnvironment( xCommandEnv )
            {
            }

            void setHandled()
            {
                m_bHandled = true;
            }

            bool isHandled() const
            {
                return m_bHandled;
            }

            void clearError()
            {
                m_nErrorCode = TASKHANDLER_NO_ERROR;
                m_nMinorCode =  TASKHANDLER_NO_ERROR;
            }

            void installError( sal_Int32 nErrorCode,
                                        sal_Int32 nMinorCode )
            {
                m_nErrorCode = nErrorCode;
                m_nMinorCode = nMinorCode;
            }

            sal_Int32 getInstalledError() const
            {
                return m_nErrorCode;
            }

            sal_Int32 getMinorErrorCode() const
            {
                return m_nMinorCode;
            }

            css::uno::Reference< css::task::XInteractionHandler > const &
            getInteractionHandler()
            {
                if( ! m_xInteractionHandler.is() && m_xCommandEnvironment.is() )
                    m_xInteractionHandler = m_xCommandEnvironment->getInteractionHandler();

                return m_xInteractionHandler;
            }

            const css::uno::Reference< css::ucb::XCommandEnvironment >&
            getCommandEnvironment() const
            {
                return m_xCommandEnvironment;
            }

        };  // end class TaskHandling


        typedef std::unordered_map< sal_Int32,TaskHandling > TaskMap;
    private:

        osl::Mutex                                                         m_aMutex;
        sal_Int32                                                           m_nCommandId;
        TaskMap                                                             m_aTaskMap;


    public:
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
            explicit MyProperty( const OUString&  thePropertyName );
            MyProperty( bool                              theIsNative,
                        const OUString&                   thePropertyName,
                        sal_Int32                         theHandle,
                        const css::uno::Type&             theTyp,
                        const css::uno::Any&              theValue,
                        const css::beans::PropertyState&  theState,
                        sal_Int16                         theAttributes );

            inline const bool& IsNative() const;
            const OUString& getPropertyName() const { return PropertyName; }
            inline const sal_Int32& getHandle() const;
            inline const css::uno::Type& getType() const;
            inline const css::uno::Any& getValue() const;
            inline const css::beans::PropertyState& getState() const;
            inline const sal_Int16& getAttributes() const;

            // The set* functions are declared const, because the key of "this" stays intact
            inline void setValue( const css::uno::Any& theValue ) const;
            inline void setState( const css::beans::PropertyState& theState ) const;
        };

        struct eMyProperty
        {
            bool operator()( const MyProperty& rKey1, const MyProperty& rKey2 ) const
            {
                return rKey1.getPropertyName() == rKey2.getPropertyName();
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

        class UnqPathData
        {
        public:
            UnqPathData();
            UnqPathData(UnqPathData&&);
            ~UnqPathData();

            PropertySet properties;
            std::vector< Notifier* > notifier;

            // Three views on the PersistentPropertySet
            css::uno::Reference< css::ucb::XPersistentPropertySet >   xS;
            css::uno::Reference< css::beans::XPropertyContainer >     xC;
            css::uno::Reference< css::beans::XPropertyAccess >        xA;
        };

        typedef std::unordered_map< OUString,UnqPathData > ContentMap;

        TaskManager( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
               FileProvider* pProvider, bool bWithConfig );
        ~TaskManager();

        /// @throws css::ucb::DuplicateCommandIdentifierException
        void startTask(
            sal_Int32 CommandId,
            const css::uno::Reference< css::ucb::XCommandEnvironment >&  xCommandEnv );

        sal_Int32 getCommandId();


        /**
         *  The error code may be one of the error codes defined in
         *  filerror.hxx.
         *  The minor code refines the information given in ErrorCode.
         */

        void installError( sal_Int32 CommandId,
                                    sal_Int32 ErrorCode,
                                    sal_Int32 minorCode = TASKHANDLER_NO_ERROR );

        void retrieveError( sal_Int32 CommandId,
                                     sal_Int32 &ErrorCode,
                                     sal_Int32 &minorCode);

        /**
         *  Deinstalls the task and evaluates a possibly set error code.
         *  "endTask" throws in case an error code is set the corresponding exception.
         */

        void endTask( sal_Int32 CommandId,
                               // the physical URL of the object
                               const OUString& aUnqPath,
                               BaseContent* pContent);


        /**
         *  Handles an interactionrequest
         */

        void handleTask( sal_Int32 CommandId,
                                  const css::uno::Reference< css::task::XInteractionRequest >& request );

        /**
         *  Clears any error which are set on the commandid
         */

        void clearError( sal_Int32 );

        /**
         *  This two methods register and deregister a change listener for the content belonging
         *  to URL aUnqPath
         */

        void registerNotifier( const OUString& aUnqPath,Notifier* pNotifier );

        void deregisterNotifier( const OUString& aUnqPath,Notifier* pNotifier );


        /**
         *  Used to associate and deassociate a new property with
         *  the content belonging to URL UnqPath.
         *  The default value and the attributes are input
         *
         *  @throws css::beans::PropertyExistException
         *  @throws css::beans::IllegalTypeException
         *  @throws css::uno::RuntimeException
         */

        void associate( const OUString& UnqPath,
                                 const OUString& PropertyName,
                                 const css::uno::Any& DefaultValue,
                                 const sal_Int16 Attributes );

        /// @throws css::beans::UnknownPropertyException
        /// @throws css::beans::NotRemoveableException
        /// @throws css::uno::RuntimeException
        void deassociate( const OUString& UnqPath,
                                   const OUString& PropertyName );


        //  Every method having a command id is not allowed to throw anything,
        //  but instead must install every error code in the task handler


        /**
         *  Given an xOutputStream, this method writes the content of the file belonging to
         *  URL aUnqPath into the XOutputStream
         */

        void page( sal_Int32 CommandId,
                            const OUString& aUnqPath,
                            const css::uno::Reference< css::io::XOutputStream >& xOutputStream );


        /**
         *  Given a file URL aUnqPath, this methods returns a XInputStream which reads from the open file.
         */

        css::uno::Reference< css::io::XInputStream >
        open( sal_Int32 CommandId,
              const OUString& aUnqPath,
              bool bLock );


        /**
         *  Given a file URL aUnqPath, this methods returns a XStream which can be used
         *  to read and write from/to the file.
         */

        css::uno::Reference< css::io::XStream >
        open_rw( sal_Int32 CommandId,
                 const OUString& aUnqPath,
                 bool bLock );


        /**
         *  This method returns the result set containing the children of the directory belonging
         *  to file URL aUnqPath
         */

        css::uno::Reference< css::ucb::XDynamicResultSet >
        ls( sal_Int32 CommandId,
            const OUString& aUnqPath,
            const sal_Int32 OpenMode,
            const css::uno::Sequence< css::beans::Property >& sProperty,
            const css::uno::Sequence< css::ucb::NumberedSortingInfo > & sSortingInfo );


        /**
         *  Info methods
         */

        // Info for commands
        css::uno::Reference< css::ucb::XCommandInfo >
        info_c();

        // Info for the properties
        css::uno::Reference< css::beans::XPropertySetInfo >
        info_p( const OUString& aUnqPath );


        /**
         *  Sets the values of the properties belonging to fileURL aUnqPath
         */

        css::uno::Sequence< css::uno::Any >
        setv( const OUString& aUnqPath,
              const css::uno::Sequence< css::beans::PropertyValue >& values );


        /**
         *  Reads the values of the properties belonging to fileURL aUnqPath;
         *  Returns an XRow object containing the values in the requested order.
         */

        css::uno::Reference< css::sdbc::XRow >
        getv( sal_Int32 CommandId,
              const OUString& aUnqPath,
              const css::uno::Sequence< css::beans::Property >& properties );


        /********************************************************************************/
        /*                         transfer-commands                                    */
        /********************************************************************************/

        /**
         *  Moves the content belonging to fileURL srcUnqPath to fileURL dstUnqPath( files and directories )
         */

        void
        move( sal_Int32 CommandId,
              const OUString& srcUnqPath,   // Full file(folder)-path
              const OUString& dstUnqPath,   // Path to the destination-directory
              const sal_Int32 NameClash );

        /**
         *  Copies the content belonging to fileURL srcUnqPath to fileURL dstUnqPath ( files and directories )
         */

        void
        copy( sal_Int32 CommandId,               // See "move"
              const OUString& srcUnqPath,
              const OUString& dstUnqPath,
              sal_Int32 NameClash );

        enum class FileUrlType { Folder = 1, File = -1, Unknown = 0 };

        /**
         *  Deletes the content belonging to fileURL aUnqPath( recursively in case of directory )
         */

        bool
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

        bool
        mkdir( sal_Int32 CommandId,
               const OUString& aDirectoryName,
               bool OverWrite );


        /**
         *  Creates new file with given URL.
         *  The content of aInputStream becomes the content of the file
         *  Return:: success of operation
         */

        bool
        mkfil( sal_Int32 CommandId,
               const OUString& aFileName,
               bool OverWrite,
               const css::uno::Reference< css::io::XInputStream >& aInputStream );


        /**
         *  writes to the file with given URL.
         *  The content of aInputStream becomes the content of the file
         *  Return:: success of operation
         */
        bool
        write( sal_Int32 CommandId,
               const OUString& aUnqPath,
               bool OverWrite,
               const css::uno::Reference< css::io::XInputStream >& aInputStream );


        void insertDefaultProperties( const OUString& aUnqPath );

        static css::uno::Sequence< css::ucb::ContentInfo >
        queryCreatableContentsInfo();


        /******************************************************************************/
        /*                                                                            */
        /*                          mapping of file urls                              */
        /*                          to uncpath and vice versa                         */
        /*                                                                            */
        /******************************************************************************/

        static bool getUnqFromUrl( const OUString& Url, OUString& Unq );

        static bool getUrlFromUnq( const OUString& Unq, OUString& Url );


        FileProvider*                                             m_pProvider;
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        css::uno::Reference< css::ucb::XPropertySetRegistry >     m_xFileRegistry;

    private:

        /********************************************************************************/
        /*                              get eventListeners                              */
        /********************************************************************************/

        std::vector< std::unique_ptr< ContentEventNotifier > >
        getContentEventListeners( const OUString& aName );

        std::vector< std::unique_ptr< ContentEventNotifier > >
        getContentDeletedEventListeners( const OUString& aName );

        std::vector< std::unique_ptr < ContentEventNotifier > >
        getContentExchangedEventListeners( const OUString& aOldPrefix,
                                           const OUString& aNewPrefix,
                                           bool withChildren );

        std::vector< std::unique_ptr< PropertyChangeNotifier > >
        getPropertyChangeNotifier( const OUString& aName );

        std::vector< std::unique_ptr< PropertySetInfoChangeNotifier > >
        getPropertySetListeners( const OUString& aName );


        /********************************************************************************/
        /*                              notify eventListeners                           */
        /********************************************************************************/

        static void notifyPropertyChanges(
            const std::vector<std::unique_ptr<PropertyChangeNotifier>>& listeners,
            const css::uno::Sequence<css::beans::PropertyChangeEvent>& seqChanged);

        static void notifyContentExchanged(
            const std::vector<std::unique_ptr<ContentEventNotifier>>& listeners_vec);

        static void
        notifyInsert(const std::vector<std::unique_ptr<ContentEventNotifier>>& listeners,
                     const OUString& aChildName);

        static void
        notifyContentDeleted(const std::vector<std::unique_ptr<ContentEventNotifier>>& listeners);

        static void
        notifyContentRemoved(const std::vector<std::unique_ptr<ContentEventNotifier>>& listeners,
                             const OUString& aChildName);

        static void notifyPropertyAdded(
            const std::vector<std::unique_ptr<PropertySetInfoChangeNotifier>>& listeners,
            const OUString& aPropertyName);

        static void notifyPropertyRemoved(
            const std::vector<std::unique_ptr<PropertySetInfoChangeNotifier>>& listeners,
            const OUString& aPropertyName);

        /********************************************************************************/
        /*                       remove persistent propertyset                          */
        /********************************************************************************/

        void erasePersistentSetWithoutChildren( const OUString& aUnqPath );
        void erasePersistentSet( const OUString& aUnqPath,
                                          bool withChildren = false );

        /********************************************************************************/
        /*                       copy persistent propertyset                            */
        /*                       from srcUnqPath to dstUnqPath                          */
        /********************************************************************************/

        void copyPersistentSetWithoutChildren( const OUString& srcUnqPath,
                                const OUString& dstUnqPath );
        void copyPersistentSet( const OUString& srcUnqPath,
                                         const OUString& dstUnqPath,
                                         bool withChildren );


        // Special optimized method for getting the properties of a directoryitem, which
        // is returned by osl::DirectoryItem::getNextItem()

        bool
        getv( const css::uno::Sequence< css::beans::Property >& properties,
              osl::DirectoryItem& DirItem,
              OUString& aUnqPath,
              bool&      bIsRegular,
              css::uno::Reference< css::sdbc::XRow > & row );


        /**
         *  Load the properties from configuration, if create == true create them.
         *  The Properties are stored under the url belonging to it->first.
         */

        void load( const TaskManager::ContentMap::iterator& it,
                            bool create );

        /**
         *  Commit inserts the determined properties in the filestatus object into
         *  the internal map, so that is possible to determine on a subsequent
         *  setting of file properties which properties have changed without filestat
         */

        void
        commit(
            const TaskManager::ContentMap::iterator& it,
            const osl::FileStatus& aFileStatus );

        /**
         *  Given a Sequence of properties seq, this method determines the mask
         *  used to instantiate an osl::FileStatus, so that a call to
         *  osl::DirectoryItem::getFileStatus fills the required fields.
         */

        static void
        getMaskFromProperties(
            sal_Int32& n_Mask,
            const css::uno::Sequence< css::beans::Property >& seq );


        // Helper function for public copy

        osl::FileBase::RC
        copy_recursive(
            const OUString& srcUnqPath,
            const OUString& dstUnqPath,
            FileUrlType TypeToCopy,
            bool  testExistence );


        // Helper function for mkfil,mkdir and write
        // Creates whole path
        // returns success of the operation
        // The call determines the errorCode, which should be used to install
        // any error

        bool
        ensuredir( sal_Int32 CommandId,
                   const OUString& aDirectoryName,
                   sal_Int32 errorCode );

        // General
        ContentMap  m_aContent;


    public:

        static constexpr OUStringLiteral FolderContentType =
            u"application/vnd.sun.staroffice.fsys-folder";
        static constexpr OUStringLiteral FileContentType =
            u"application/vnd.sun.staroffice.fsys-file";


    private:

        PropertySet                                   m_aDefaultProperties;
        css::uno::Sequence< css::ucb::CommandInfo >   m_sCommandInfo;

    public:
        // Miscellaneous:
        // Methods for "writeComponentInfo" and "createComponentFactory"

        static void getScheme( OUString& Scheme );
    };

} // end namespace TaskHandling

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
