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

#ifndef INCLUDED_BASCTL_SOURCE_INC_SCRIPTDOCUMENT_HXX
#define INCLUDED_BASCTL_SOURCE_INC_SCRIPTDOCUMENT_HXX

#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/io/XInputStreamProvider.hpp>

#include <memory>
#include <vector>

class SfxListener;

class BasicManager;


namespace basctl
{

    enum LibraryContainerType
    {
        E_SCRIPTS,
        E_DIALOGS
    };

    enum LibraryLocation
    {
        LIBRARY_LOCATION_UNKNOWN,
        LIBRARY_LOCATION_USER,
        LIBRARY_LOCATION_SHARE,
        LIBRARY_LOCATION_DOCUMENT
    };

    enum LibraryType
    {
        LIBRARY_TYPE_UNKNOWN,
        LIBRARY_TYPE_MODULE,
        LIBRARY_TYPE_DIALOG,
        LIBRARY_TYPE_ALL
    };

    class ScriptDocument;
    typedef ::std::vector< ScriptDocument >  ScriptDocuments;

    /** encapsulates a document which contains Basic scripts and dialogs
    */
    class ScriptDocument
    {
    private:
        class Impl;
        std::shared_ptr<Impl> m_pImpl;

    private:
        /** creates a ScriptDocument instance which operates on the application-wide
            scripts and dialogs
        */
                    ScriptDocument();

    public:
        enum SpecialDocument { NoDocument };
        /** creates a ScriptDocument instance which does refers to neither the application-wide,
            nor a specific real document's scripts.

            This constructor might come handy when you need some kind of uninitialized
            ScriptDocument, which you do not want to operate on (yet), but initialize later
            by assignment.

            <member>isValid</member> will return <FALSE/> for a ScriptDocument constructed
            this way.
        */
        explicit    ScriptDocument( SpecialDocument _eType );

        /** creates a ScriptDocument instance which refers to a document given as
            XModel

            @param _rxDocument
                the document. Must not be <NULL/>.
        */
        explicit    ScriptDocument( const css::uno::Reference< css::frame::XModel >& _rxDocument );

        /// copy constructor
                    ScriptDocument( const ScriptDocument& _rSource );

        /// destructor
                    ~ScriptDocument();

        /** returns a reference to a shared ScriptDocument instance which
            operates on the application-wide scripts and dialogs
        */
        static const ScriptDocument&
                    getApplicationScriptDocument();

        /** returns a (newly created) ScriptDocument instance for the document to
            which a given BasicManager belongs

            If the basic manager is the application's basic manager, then the (shared)
            ScriptDocument instance which is responsible for the application is returned.

            @see getApplicationScriptDocument
        */
        static ScriptDocument
                    getDocumentForBasicManager( const BasicManager* _pManager );

        /** returns a (newly created) ScriptDocument instance for the document
            with a given caption or URL

            If there is no document with the given caption, then the (shared)
            ScriptDocument instance which is responsible for the application is returned.

            @see getApplicationScriptDocument
        */
        static ScriptDocument
                    getDocumentWithURLOrCaption( const OUString& _rUrlOrCaption );

        /** operation mode for getAllScriptDocuments
        */
        enum ScriptDocumentList
        {
            /** all ScriptDocuments, including the dedicated one which represents
                the application-wide scripts/dialogs.
            */
            AllWithApplication,
            /** real documents only
            */
            DocumentsOnly,
            /** real documents only, sorted lexicographically by their title (using the sys locale's default
                collator)
            */
            DocumentsSorted
        };

        /** returns the set of ScriptDocument instances, one for each open document which
            contains Basic/Dialog containers; plus an additional instance for
            the application, if desired

            Documents which are not visible - i.e. do not have a visible frame.

            @param _bIncludingApplication
                <TRUE/> if the application-wide scripts/dialogs should also be represented
                by a ScriptDocument
        */
        static ScriptDocuments
                    getAllScriptDocuments( ScriptDocumentList _eListType );

        // comparison
                bool operator==( const ScriptDocument& _rhs ) const;
        inline  bool operator!=( const ScriptDocument& _rhs ) const { return !( *this == _rhs ); }

        /// retrieves a (pretty simple) hash code for the document
        sal_Int32   hashCode() const;

        /** determines whether the document is actually able to contain Basic/Dialog libraries

            Note that validity does not automatically imply the document can be used for active
            work. Instead, it is possible the document is closed already (or being closed currently).
            In this case, isValid will return <TRUE/>, but isAlive will return <FALSE/>.

            @return
                <TRUE/> if the instance refers to a document which contains Basic/Dialog libraries,
                or the application as a whole, <FALSE/> otherwise.

            @see isAlive
        */
        bool        isValid() const;

        /** determines whether the document instance is alive

            If the instance is not valid, <FALSE/> is returned.

            If the instance refers to a real document, which is already closed, or just being closed,
            the method returns <FALSE/>.

            If the instance refers to the application, <TRUE/> is returned.

            @see isValid
        */
        bool        isAlive() const;

        bool        isInVBAMode() const;
        /// returns the BasicManager associated with this instance
        BasicManager*
                    getBasicManager() const;

        /** returns the UNO component representing the document which the instance operates on

            Must not be used when the instance operates on the application-wide
            Basic/Dialog libraries.
        */
        css::uno::Reference< css::frame::XModel >
                    getDocument() const;

        /** returns the UNO component representing the document which the instance operates on

            May be used when the instance operates on the application-wide
            Basic/Dialog libraries, in this case it returns <NULL/>.
        */
        css::uno::Reference< css::frame::XModel >
                    getDocumentOrNull() const;

        /** returns the Basic or Dialog library container of the document

            If the document is not valid, <NULL/> is returned.
        */
        css::uno::Reference< css::script::XLibraryContainer >
                    getLibraryContainer( LibraryContainerType _eType ) const;

        /** determines whether there exists a library of the given type, with the given name
        */
        bool        hasLibrary( LibraryContainerType _eType, const OUString& _rLibName ) const;

        /** returns a script or dialog library given by name

            @param _eType
                the type of library to load
            @param _rLibName
                the name of the script library
            @param _bLoadLibrary
                <TRUE/> if and only if the library should be loaded.

            @throws NoSuchElementException
                if there is no script library with the given name
        */
        css::uno::Reference< css::container::XNameContainer >
                    getLibrary( LibraryContainerType _eType, const OUString& _rLibName, bool _bLoadLibrary ) const;

        /** creates a script or dialog library in the document, or returns an existing one

            If <code>_rLibName</code> denotes an existing library which does not need to be created,
            then this library will automatically be loaded, and then returned.
        */
        css::uno::Reference< css::container::XNameContainer >
                    getOrCreateLibrary( LibraryContainerType _eType, const OUString& _rLibName ) const;

        /** returns the names of the modules in a given script or dialog library of the document
        */
        css::uno::Sequence< OUString >
                    getObjectNames( LibraryContainerType _eType, const OUString& _rLibName ) const;

        /** retrieves a name for a newly to be created module or dialog
        */
        OUString    createObjectName( LibraryContainerType _eType, const OUString& _rLibName ) const;

        /** loads a script or dialog library given by name, if there is such a library
        */
        void        loadLibraryIfExists( LibraryContainerType _eType, const OUString& _rLibrary );

        /// retrieves the (combined) names of all script and dialog libraries
        css::uno::Sequence< OUString >
                    getLibraryNames() const;

        /** removes a given script module from the document

            @return
                <TRUE/> if and only if the removal was successful. When <FALSE/> is returned,
                this will reported as assertion in a non-product build.
        */
        bool        removeModule( const OUString& _rLibName, const OUString& _rModuleName ) const;

        /** creates a module with the given name in the given library
            @param  _rLibName
                the library name
            @param  _rModName
                the name of the to-be-created module
            @param  _bCreateMain
                determines whether or not a function Main should be created
            @param  _out_rNewModuleCode
                the source code of the newly created module
            @return
                <TRUE/> if and only if the creation was successful
        */
        bool        createModule( const OUString& _rLibName, const OUString& _rModName, bool _bCreateMain, OUString& _out_rNewModuleCode ) const;

        /** inserts a given piece as code as module
            @param  _rLibName
                the name of the library to insert the module into. If a library with this name does
                not yet exist, it will be created.
            @param  _rModName
                the name of the module to insert the code as. Must denote a name which is not yet
                used in the module library.
            @param  _rModuleCode
                the code of the new module
            @return
                <TRUE/> if and only if the insertion was successful.
        */
        bool        insertModule( const OUString& _rLibName, const OUString& _rModName, const OUString& _rModuleCode ) const;

        /** updates a given module with new code
            @param  _rLibName
                the name of the library the modules lives in. Must denote an existing module library.
            @param  _rModName
                the name of the module to update. Must denote an existing module in the given library.
            @param  _rModuleCode
                the new module code.
            @return
                <TRUE/> if and only if the insertion was successful.
        */
        bool        updateModule( const OUString& _rLibName, const OUString& _rModName, const OUString& _rModuleCode ) const;

        /// determines whether a module with the given name exists in the given library
        bool        hasModule( const OUString& _rLibName, const OUString& _rModName ) const;

        /** retrieves a module's source
            @param  _rLibName
                the library name where the module is located
            @param  _rModName
                the module name
            @param  _out_rModuleSource
                takes the module's source upon successful return
            @return
                <TRUE/> if and only if the code could be successfully retrieved, <FALSE/> otherwise
        */
        bool        getModule( const OUString& _rLibName, const OUString& _rModName, OUString& _rModuleSource ) const;

        /** renames a module
            @param  _rLibName
                the library where the module lives in. Must denote an existing library.
            @param  _rOldName
                the old module name. Must denote an existing module.
            @param  _rNewName
                the new module name
            @return
                <TRUE/> if and only if renaming was successful.
        */
        bool        renameModule( const OUString& _rLibName, const OUString& _rOldName, const OUString& _rNewName ) const;

        /** removes a given dialog from the document

            @return
                <TRUE/> if and only if the removal was successful. When <FALSE/> is returned,
                this will reported as assertion in a non-product build.
        */
        bool        removeDialog( const OUString& _rLibName, const OUString& _rDialogName ) const;

        /// determines whether a dialog with the given name exists in the given library
        bool        hasDialog( const OUString& _rLibName, const OUString& _rDialogName ) const;

        /** retrieves a dialog
            @param  _rLibName
                the library name where the module is located
            @param  _rDialogName
                the dialog's name
            @param  _out_rDialogSource
                takes the provider for the dialog's description, upon successful return
            @return
                <TRUE/> if and only if the dialog could be successfully retrieved, <FALSE/> otherwise
        */
        bool        getDialog(
                        const OUString& _rLibName,
                        const OUString& _rDialogName,
                        css::uno::Reference< css::io::XInputStreamProvider >& _out_rDialogProvider
                    ) const;

        /** renames a dialog
            @param  _rLibName
                the library where the dialog lives in. Must denote an existing library.
            @param  _rOldName
                the old dialog name. Must denote an existing dialog.
            @param  _rNewName
                the new dialog name
            @param _rxExistingDialogModel
                the existing model of the dialog, if already loaded in the IDE
            @return
                <TRUE/> if and only if renaming was successful.
        */
        bool        renameDialog(
                        const OUString& _rLibName,
                        const OUString& _rOldName,
                        const OUString& _rNewName,
                        const css::uno::Reference< css::container::XNameContainer >& _rxExistingDialogModel
                    ) const;

        /** create a dialog
            @param  _rLibName
                the library name where the module is located
            @param  _rDialogName
                the dialog's name
            @param  _out_rDialogSource
                takes the provider for the dialog's description, upon successful return
            @return
                <TRUE/> if and only if the dialog could be successfully retrieved, <FALSE/> otherwise
        */
        bool        createDialog(
                        const OUString& _rLibName,
                        const OUString& _rDialogName,
                        css::uno::Reference< css::io::XInputStreamProvider >& _out_rDialogProvider
                    ) const;

        /** inserts a given dialog into a given library

            @param  _rLibName
                the name of the library to insert the dialog into. If a library with this name does
                not yet exist, it will be created.
            @param  _rModName
                the name of the dialog to insert. Must denote a name which is not yet
                used in the dialog library.
            @param  _rDialogProvider
                the provider of the dialog's description
            @return
                <TRUE/> if and only if the insertion was successful.
        */
        bool        insertDialog(
                        const OUString& _rLibName,
                        const OUString& _rDialogName,
                        const css::uno::Reference< css::io::XInputStreamProvider >& _rDialogProvider
                    ) const;

        /** determines whether the document is read-only

            cannot be called if the document operates on the application-wide scripts
        */
        bool        isReadOnly() const;

        /** determines whether the ScriptDocument instance operates on the whole application,
            as opposed to a real document
        */
        bool        isApplication() const;

        /** determines whether the ScriptDocument instance operates on a real document,
            as opposed to the whole application
        */
        bool        isDocument() const { return isValid() && !isApplication(); }

        /** marks the document as modified
            @precond
                the instance operates on a real document, not on the application
            @see isDocument
        */
        void        setDocumentModified() const;

        /** determines whether the document is modified
            @precond
                the instance operates on a real document, not on the application
            @see isDocument
        */
        bool        isDocumentModified() const;

        /** saves the document, if the instance refers to a real document
            @precond
                <code>isApplication</code> returns <FALSE/>
        */
        bool        saveDocument(
                        const css::uno::Reference< css::task::XStatusIndicator >& _rxStatusIndicator
                    ) const;

        /// returns the location of a library given by name
        LibraryLocation
                    getLibraryLocation( const OUString& _rLibName ) const;

        /// returns the title for the document
        OUString    getTitle( LibraryLocation _eLocation, LibraryType _eType = LIBRARY_TYPE_ALL ) const;

        /** returns the title of the document

            to be used for valid documents only
        */
        OUString    getTitle() const;

        /** returns the URL of the document

            to be used for valid documents only
        */
        OUString    getURL() const;

        /** determines whether the document is currently the one-and-only application-wide active document
        */
        bool        isActive() const;

        /** determines whether macro execution for this document is allowed

            only to be called for real documents (->isDocument)
        */
        bool    allowMacros() const;
    };


} // namespace basctl


#endif // INCLUDED_BASCTL_SOURCE_INC_SCRIPTDOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
