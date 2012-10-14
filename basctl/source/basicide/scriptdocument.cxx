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

#include "scriptdocument.hxx"
#include "basobj.hxx"
#include "basidesh.hrc"
#include "iderid.hxx"
#include "dlgeddef.hxx"
#include "doceventnotifier.hxx"
#include "documentenumeration.hxx"

#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>

#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/docfile.hxx>

#include <vcl/svapp.hxx>

#include <basic/basicmanagerrepository.hxx>

#include <xmlscript/xmldlg_imexp.hxx>

#include <unotools/syslocale.hxx>

#include <unotools/collatorwrapper.hxx>

#include <tools/diagnose_ex.h>

#include <comphelper/processfactory.hxx>
#include <comphelper/documentinfo.hxx>

#include <osl/mutex.hxx>

#include <cppuhelper/implbase1.hxx>

#include <rtl/uri.hxx>
#include <rtl/bootstrap.hxx>

#include <osl/process.h>
#include <osl/file.hxx>

#include <set>

namespace basctl
{
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::script::XLibraryContainer;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::container::XNameContainer;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::task::XStatusIndicator;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::script::XLibraryContainer2;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::uri::XUriReferenceFactory;
    using ::com::sun::star::uri::XUriReference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::util::XMacroExpander;
    using ::com::sun::star::io::XInputStreamProvider;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::io::XInputStream;
    using ::com::sun::star::frame::XStorable;
    using ::com::sun::star::util::XModifiable;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::util::URL;
    using ::com::sun::star::frame::XDispatchProvider;
    using ::com::sun::star::frame::XDispatch;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::frame::XDesktop;
    using ::com::sun::star::container::XEnumerationAccess;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::frame::XModel2;
    using ::com::sun::star::awt::XWindow2;
    using ::com::sun::star::document::XEventListener;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::document::XEventBroadcaster;
    using ::com::sun::star::document::XEmbeddedScripts;
    using ::com::sun::star::script::ModuleInfo;
    using ::com::sun::star::script::vba::XVBACompatibility;
    using ::com::sun::star::script::vba::XVBAModuleInfo;
    /** === end UNO using === **/
    namespace MacroExecMode = ::com::sun::star::document::MacroExecMode;
    namespace FrameSearchFlag = ::com::sun::star::frame::FrameSearchFlag;

    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        static bool StringCompareLessThan( const String& lhs, const String& rhs )
        {
            return ( lhs.CompareIgnoreCaseToAscii( rhs ) == COMPARE_LESS );
        }

        class FilterDocuments : public docs::IDocumentDescriptorFilter
        {
        public:
            FilterDocuments( bool _bFilterInvisible ) : m_bFilterInvisible( _bFilterInvisible ) { }

            virtual ~FilterDocuments() {}

            virtual bool    includeDocument( const docs::DocumentDescriptor& _rDocument ) const;

        private:
            bool    impl_isDocumentVisible_nothrow( const docs::DocumentDescriptor& _rDocument ) const;

        private:
            bool    m_bFilterInvisible;
        };

        bool FilterDocuments::impl_isDocumentVisible_nothrow( const docs::DocumentDescriptor& _rDocument ) const
        {
            try
            {
                for (   docs::Controllers::const_iterator controller = _rDocument.aControllers.begin();
                        controller != _rDocument.aControllers.end();
                        ++controller
                    )
                {
                    Reference< XFrame > xFrame( (*controller)->getFrame(), UNO_SET_THROW );
                    Reference< XWindow2 > xContainer( xFrame->getContainerWindow(), UNO_QUERY_THROW );
                    if ( xContainer->isVisible() )
                        return true;
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return false;
        }

        bool FilterDocuments::includeDocument( const docs::DocumentDescriptor& _rDocument ) const
        {
            Reference< XEmbeddedScripts > xScripts( _rDocument.xModel, UNO_QUERY );
            if ( !xScripts.is() )
                return false;
            if ( !m_bFilterInvisible || impl_isDocumentVisible_nothrow( _rDocument ) )
                return true;
            return false;
        }

        void lcl_getAllModels_throw( docs::Documents& _out_rModels, bool _bVisibleOnly )
        {
            _out_rModels.clear();

            FilterDocuments aFilter( _bVisibleOnly );
            docs::DocumentEnumeration aEnum(
                comphelper::getProcessComponentContext(), &aFilter );

            aEnum.getDocuments( _out_rModels );
        }
    }

    class ScriptDocument::Impl : public DocumentEventListener
    {
    private:
        bool                            m_bIsApplication;
        bool                            m_bValid;
        bool                            m_bDocumentClosed;
        Reference< XModel >             m_xDocument;
        Reference< XModifiable >        m_xDocModify;
        Reference< XEmbeddedScripts >   m_xScriptAccess;
        ::std::auto_ptr< DocumentEventNotifier >
                                        m_pDocListener;

    public:
        Impl ();
        Impl (Reference<XModel> const& rxDocument);
        ~Impl ();

        /** determines whether the instance refers to a valid "document" with script and
            dialog libraries
        */
        inline  bool    isValid()       const   { return m_bValid; }
        /** determines whether the instance refers to a non-closed document
        */
        inline  bool    isAlive()       const   { return m_bValid ? ( m_bIsApplication ? true : !m_bDocumentClosed ) : false; }
        /// determines whether the "document" refers to the application in real
        inline  bool    isApplication() const   { return m_bValid && m_bIsApplication; }
        /// determines whether the document refers to a real document (instead of the application)
        inline  bool    isDocument()    const   { return m_bValid && !m_bIsApplication; }

        /** invalidates the instance
        */
        void    invalidate();

        const Reference< XModel >&
                        getDocumentRef() const { return m_xDocument; }

        /// returns a library container belonging to the document
        Reference< XLibraryContainer >
                    getLibraryContainer( LibraryContainerType _eType ) const;

        /// determines whether a given library is part of the shared installation
        bool        isLibraryShared( const OUString& _rLibName, LibraryContainerType _eType );

        /** returns the current frame of the document

            To be called for documents only, not for the application.

            If <FALSE/> is returned, an assertion will be raised in non-product builds.
        */
        bool        getCurrentFrame( Reference< XFrame >& _out_rxFrame ) const;

        // versions with the same signature/semantics as in ScriptDocument itself
        bool        isReadOnly() const;
        bool        isInVBAMode() const;
        BasicManager*
                    getBasicManager() const;
        Reference< XModel >
                    getDocument() const;
        void        setDocumentModified() const;
        bool        isDocumentModified() const;
        bool        saveDocument( const Reference< XStatusIndicator >& _rxStatusIndicator ) const;

        OUString    getTitle() const;
        OUString    getURL() const;

        bool        allowMacros() const;

        Reference< XNameContainer >
                    getLibrary( LibraryContainerType _eType, const OUString& _rLibName, bool _bLoadLibrary ) const
                        SAL_THROW((NoSuchElementException));
        bool        hasLibrary( LibraryContainerType _eType, const OUString& _rLibName ) const;
        Reference< XNameContainer >
                    getOrCreateLibrary( LibraryContainerType _eType, const OUString& _rLibName ) const;

        void        loadLibraryIfExists( LibraryContainerType _eType, const OUString& _rLibrary );

        bool        removeModuleOrDialog( LibraryContainerType _eType, const OUString& _rLibName, const OUString& _rModuleName );
        bool        hasModuleOrDialog( LibraryContainerType _eType, const OUString& _rLibName, const OUString& _rModName ) const;
        bool        getModuleOrDialog( LibraryContainerType _eType, const OUString& _rLibName, const OUString& _rObjectName, Any& _out_rModuleOrDialog );
        bool        renameModuleOrDialog( LibraryContainerType _eType, const OUString& _rLibName, const OUString& _rOldName, const OUString& _rNewName, const Reference< XNameContainer >& _rxExistingDialogModel );
        bool        createModule( const OUString& _rLibName, const OUString& _rModName, bool _bCreateMain, OUString& _out_rNewModuleCode ) const;
        bool        insertModuleOrDialog( LibraryContainerType _eType, const OUString& _rObjectName, const OUString& _rModName, const Any& _rElement ) const;
        bool        updateModule( const OUString& _rLibName, const OUString& _rModName, const OUString& _rModuleCode ) const;
        bool        createDialog( const OUString& _rLibName, const OUString& _rDialogName, Reference< XInputStreamProvider >& _out_rDialogProvider ) const;

    protected:
        // DocumentEventListener
        virtual void onDocumentCreated( const ScriptDocument& _rDocument );
        virtual void onDocumentOpened( const ScriptDocument& _rDocument );
        virtual void onDocumentSave( const ScriptDocument& _rDocument );
        virtual void onDocumentSaveDone( const ScriptDocument& _rDocument );
        virtual void onDocumentSaveAs( const ScriptDocument& _rDocument );
        virtual void onDocumentSaveAsDone( const ScriptDocument& _rDocument );
        virtual void onDocumentClosed( const ScriptDocument& _rDocument );
        virtual void onDocumentTitleChanged( const ScriptDocument& _rDocument );
        virtual void onDocumentModeChanged( const ScriptDocument& _rDocument );

    private:
        bool        impl_initDocument_nothrow( const Reference< XModel >& _rxModel );
    };

    //====================================================================
    //= ScriptDocument::Impl - implementation
    //====================================================================
    ScriptDocument::Impl::Impl()
        :m_bIsApplication( true )
        ,m_bValid( true )
        ,m_bDocumentClosed( false )
    {
    }

    ScriptDocument::Impl::Impl( const Reference< XModel >& _rxDocument )
        :m_bIsApplication( false )
        ,m_bValid( false )
        ,m_bDocumentClosed( false )
    {
        if ( _rxDocument.is() )
        {
            if ( impl_initDocument_nothrow( _rxDocument ) )
            {
            }
        }
    }

    ScriptDocument::Impl::~Impl()
    {
        invalidate();
    }

    void ScriptDocument::Impl::invalidate()
    {
        m_bIsApplication = false;
        m_bValid = false;
        m_bDocumentClosed = false;

        m_xDocument.clear();
        m_xDocModify.clear();
        m_xScriptAccess.clear();

        if ( m_pDocListener.get() )
            m_pDocListener->dispose();
    }

    bool ScriptDocument::Impl::impl_initDocument_nothrow( const Reference< XModel >& _rxModel )
    {
        try
        {
            m_xDocument.set     ( _rxModel, UNO_SET_THROW );
            m_xDocModify.set    ( _rxModel, UNO_QUERY_THROW );
            m_xScriptAccess.set ( _rxModel, UNO_QUERY );

            m_bValid = m_xScriptAccess.is();

            if ( m_bValid )
                m_pDocListener.reset( new DocumentEventNotifier( *this, _rxModel ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            m_bValid = false;
        }

        if ( !m_bValid )
        {
            invalidate();
        }

        return m_bValid;
    }

    Reference< XLibraryContainer > ScriptDocument::Impl::getLibraryContainer( LibraryContainerType _eType ) const
    {
        OSL_ENSURE( isValid(), "ScriptDocument::Impl::getLibraryContainer: invalid!" );

        Reference< XLibraryContainer > xContainer;
        if ( !isValid() )
            return xContainer;

        try
        {
            if ( isApplication() )
                xContainer.set( _eType == E_SCRIPTS ? SFX_APP()->GetBasicContainer() : SFX_APP()->GetDialogContainer(), UNO_QUERY_THROW );
            else
            {
                xContainer.set(
                    _eType == E_SCRIPTS ? m_xScriptAccess->getBasicLibraries() : m_xScriptAccess->getDialogLibraries(),
                    UNO_QUERY_THROW );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return xContainer;
    }

    bool ScriptDocument::Impl::isReadOnly() const
    {
        OSL_ENSURE( isValid(), "ScriptDocument::Impl::isReadOnly: invalid state!" );
        OSL_ENSURE( !isApplication(), "ScriptDocument::Impl::isReadOnly: not allowed to be called for the application!" );

        bool bIsReadOnly = true;
        if ( isValid() && !isApplication() )
        {
            try
            {
                // note that XStorable is required by the OfficeDocument service
                Reference< XStorable > xDocStorable( m_xDocument, UNO_QUERY_THROW );
                bIsReadOnly = xDocStorable->isReadonly();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return bIsReadOnly;
    }

    bool ScriptDocument::Impl::isInVBAMode() const
    {
        bool bResult = false;
        if ( !isApplication() )
        {
            Reference< XVBACompatibility > xVBACompat( getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
            if ( xVBACompat.is() )
                bResult = xVBACompat->getVBACompatibilityMode();
        }
        return bResult;
    }


    BasicManager* ScriptDocument::Impl::getBasicManager() const
    {
        OSL_ENSURE( isValid(), "ScriptDocument::Impl::getBasicManager: invalid state!" );
        if ( !isValid() )
            return NULL;

        if ( isApplication() )
            return SFX_APP()->GetBasicManager();

        return ::basic::BasicManagerRepository::getDocumentBasicManager( m_xDocument );
    }


    Reference< XModel > ScriptDocument::Impl::getDocument() const
    {
        OSL_ENSURE( isValid(), "ScriptDocument::Impl::getDocument: invalid state!" );
        OSL_ENSURE( isDocument(), "ScriptDocument::Impl::getDocument: for documents only!" );
        if ( !isValid() || !isDocument() )
            return NULL;

        return m_xDocument;
    }


    Reference< XNameContainer > ScriptDocument::Impl::getLibrary( LibraryContainerType _eType, const OUString& _rLibName, bool _bLoadLibrary ) const
        SAL_THROW((NoSuchElementException))
    {
        OSL_ENSURE( isValid(), "ScriptDocument::Impl::getLibrary: invalid state!" );

        Reference< XNameContainer > xContainer;
        try
        {
            Reference< XLibraryContainer > xLibContainer = getLibraryContainer( _eType );
            if ( isValid() )
            {
                if ( xLibContainer.is() )
                    xContainer.set( xLibContainer->getByName( _rLibName ), UNO_QUERY_THROW );
            }

            if ( !xContainer.is() )
                throw NoSuchElementException();

            // load library
            if ( _bLoadLibrary && !xLibContainer->isLibraryLoaded( _rLibName ) )
                xLibContainer->loadLibrary( _rLibName );
        }
        catch( const NoSuchElementException& )
        {
            throw;  // allowed to leave
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return xContainer;
    }


    bool ScriptDocument::Impl::hasLibrary( LibraryContainerType _eType, const OUString& _rLibName ) const
    {
        bool bHas = false;
        try
        {
            Reference< XLibraryContainer > xLibContainer = getLibraryContainer( _eType );
            bHas = xLibContainer.is() && xLibContainer->hasByName( _rLibName );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return bHas;
    }


    Reference< XNameContainer > ScriptDocument::Impl::getOrCreateLibrary( LibraryContainerType _eType, const OUString& _rLibName ) const
    {
        Reference< XNameContainer > xLibrary;
        try
        {
            Reference< XLibraryContainer > xLibContainer( getLibraryContainer( _eType ), UNO_QUERY_THROW );
            if ( xLibContainer->hasByName( _rLibName ) )
                xLibrary.set( xLibContainer->getByName( _rLibName ), UNO_QUERY_THROW );
            else
                xLibrary.set( xLibContainer->createLibrary( _rLibName ), UNO_QUERY_THROW );

            if ( !xLibContainer->isLibraryLoaded( _rLibName ) )
                xLibContainer->loadLibrary( _rLibName );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return xLibrary;
    }


    void ScriptDocument::Impl::loadLibraryIfExists( LibraryContainerType _eType, const OUString& _rLibrary )
    {
        try
        {
            Reference< XLibraryContainer > xLibContainer( getLibraryContainer( _eType ) );
            if ( xLibContainer.is() && xLibContainer->hasByName( _rLibrary ) && !xLibContainer->isLibraryLoaded( _rLibrary ) )
                xLibContainer->loadLibrary( _rLibrary );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    bool ScriptDocument::Impl::removeModuleOrDialog( LibraryContainerType _eType, const OUString& _rLibName, const OUString& _rModuleName )
    {
        OSL_ENSURE( isValid(), "ScriptDocument::Impl::removeModuleOrDialog: invalid!" );
        if ( isValid() )
        {
            try
            {
                Reference< XNameContainer > xLib( getLibrary( _eType, _rLibName, true ) );
                if ( xLib.is() )
                {
                    xLib->removeByName( _rModuleName );
                    return true;
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return false;
    }


    bool ScriptDocument::Impl::hasModuleOrDialog( LibraryContainerType _eType, const OUString& _rLibName, const OUString& _rModName ) const
    {
        OSL_ENSURE( isValid(), "ScriptDocument::Impl::hasModuleOrDialog: invalid!" );
        if ( !isValid() )
            return false;

        try
        {
            Reference< XNameContainer > xLib( getLibrary( _eType, _rLibName, true ) );
            if ( xLib.is() )
                return xLib->hasByName( _rModName );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }


    bool ScriptDocument::Impl::getModuleOrDialog( LibraryContainerType _eType, const OUString& _rLibName, const OUString& _rObjectName, Any& _out_rModuleOrDialog )
    {
        OSL_ENSURE( isValid(), "ScriptDocument::Impl::getModuleOrDialog: invalid!" );
        if ( !isValid() )
            return false;

        _out_rModuleOrDialog.clear();
        try
        {
            Reference< XNameContainer > xLib( getLibrary( _eType, _rLibName, true ), UNO_QUERY_THROW );
            if ( xLib->hasByName( _rObjectName ) )
            {
                _out_rModuleOrDialog = xLib->getByName( _rObjectName );
                return true;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }


    bool ScriptDocument::Impl::renameModuleOrDialog( LibraryContainerType _eType, const OUString& _rLibName,
        const OUString& _rOldName, const OUString& _rNewName, const Reference< XNameContainer >& _rxExistingDialogModel )
    {
        OSL_ENSURE( isValid(), "ScriptDocument::Impl::renameModuleOrDialog: invalid!" );
        if ( !isValid() )
            return false;

        try
        {
            Reference< XNameContainer > xLib( getLibrary( _eType, _rLibName, true ), UNO_QUERY_THROW );

            // get element
            Any aElement( xLib->getByName( _rOldName ) );

            // remove element from container
            xLib->removeByName( _rOldName );

            // if it's a dialog, import and export, to reflect the new name
            if ( _eType == E_DIALOGS )
            {
                // create dialog model
                Reference< XComponentContext > aContext(
                    comphelper::getProcessComponentContext() );
                Reference< XNameContainer > xDialogModel;
                if ( _rxExistingDialogModel.is() )
                    xDialogModel = _rxExistingDialogModel;
                else
                    xDialogModel.set(
                        ( aContext->getServiceManager()->
                          createInstanceWithContext(
                              "com.sun.star.awt.UnoControlDialogModel",
                              aContext ) ),
                        UNO_QUERY_THROW );

                // import dialog model
                Reference< XInputStreamProvider > xISP( aElement, UNO_QUERY_THROW );
                if ( !_rxExistingDialogModel.is() )
                {
                    Reference< XInputStream > xInput( xISP->createInputStream(), UNO_QUERY_THROW );
                    ::xmlscript::importDialogModel( xInput, xDialogModel, aContext, isDocument() ? getDocument() : Reference< XModel >() );
                }

                // set new name as property
                Reference< XPropertySet > xDlgPSet( xDialogModel, UNO_QUERY_THROW );
                xDlgPSet->setPropertyValue( DLGED_PROP_NAME, makeAny( _rNewName ) );

                // export dialog model
                xISP = ::xmlscript::exportDialogModel( xDialogModel, aContext, isDocument() ? getDocument() : Reference< XModel >() );
                aElement <<= xISP;
            }

            // insert element by new name in container
            if ( _eType == E_SCRIPTS )
            {
                Reference< XVBAModuleInfo > xVBAModuleInfo( xLib, UNO_QUERY );
                if ( xVBAModuleInfo->hasModuleInfo( _rOldName ) )
                {
                    ModuleInfo sModuleInfo = xVBAModuleInfo->getModuleInfo( _rOldName );
                    xVBAModuleInfo->removeModuleInfo( _rOldName );
                    xVBAModuleInfo->insertModuleInfo( _rNewName, sModuleInfo );
                }
            }
            xLib->insertByName( _rNewName, aElement );
            return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }


    bool ScriptDocument::Impl::createModule( const OUString& _rLibName, const OUString& _rModName, bool _bCreateMain, OUString& _out_rNewModuleCode ) const
    {
        _out_rNewModuleCode = OUString();
        try
        {
            Reference< XNameContainer > xLib( getLibrary( E_SCRIPTS, _rLibName, true ) );
            if ( !xLib.is() || xLib->hasByName( _rModName ) )
                return false;

            // create new module
            _out_rNewModuleCode = "REM  *****  BASIC  *****\n\n" ;
            if ( _bCreateMain )
                _out_rNewModuleCode += "Sub Main\n\nEnd Sub\n" ;

            // insert module into library
            xLib->insertByName( _rModName, makeAny( _out_rNewModuleCode ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            return false;
        }

        return true;
    }


    bool ScriptDocument::Impl::insertModuleOrDialog( LibraryContainerType _eType, const OUString& _rLibName, const OUString& _rObjectName, const Any& _rElement ) const
    {
        try
        {
            Reference< XNameContainer > xLib( getOrCreateLibrary( _eType, _rLibName ), UNO_QUERY_THROW );
            if ( xLib->hasByName( _rObjectName ) )
                return false;

            xLib->insertByName( _rObjectName, _rElement );
            return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }


    bool ScriptDocument::Impl::updateModule( const OUString& _rLibName, const OUString& _rModName, const OUString& _rModuleCode ) const
    {
        try
        {
            Reference< XNameContainer > xLib( getOrCreateLibrary( E_SCRIPTS, _rLibName ), UNO_QUERY_THROW );
            if ( !xLib->hasByName( _rModName ) )
                return false;
            xLib->replaceByName( _rModName, makeAny( _rModuleCode ) );
            return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }


    bool ScriptDocument::Impl::createDialog( const OUString& _rLibName, const OUString& _rDialogName, Reference< XInputStreamProvider >& _out_rDialogProvider ) const
    {
        try
        {
            Reference< XNameContainer > xLib( getLibrary( E_DIALOGS, _rLibName, true ), UNO_QUERY_THROW );

            // create dialog
            _out_rDialogProvider.clear();
            if ( xLib->hasByName( _rDialogName ) )
                return false;

            // create new dialog model
            Reference< XComponentContext > aContext(
                comphelper::getProcessComponentContext() );
            Reference< XNameContainer > xDialogModel(
                aContext->getServiceManager()->createInstanceWithContext(
                    "com.sun.star.awt.UnoControlDialogModel", aContext ),
                UNO_QUERY_THROW );

            // set name property
            Reference< XPropertySet > xDlgPSet( xDialogModel, UNO_QUERY_THROW );
            xDlgPSet->setPropertyValue( DLGED_PROP_NAME, makeAny( _rDialogName ) );

            // export dialog model
            _out_rDialogProvider = ::xmlscript::exportDialogModel( xDialogModel, aContext, isDocument() ? getDocument() : Reference< XModel >() );

            // insert dialog into library
            xLib->insertByName( _rDialogName, makeAny( _out_rDialogProvider ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return _out_rDialogProvider.is();
    }


    void ScriptDocument::Impl::setDocumentModified() const
    {
        OSL_ENSURE( isValid() && isDocument(), "ScriptDocument::Impl::setDocumentModified: only to be called for real documents!" );
        if ( isValid() && isDocument() )
        {
            try
            {
                m_xDocModify->setModified( true );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }


    bool ScriptDocument::Impl::isDocumentModified() const
    {
        OSL_ENSURE( isValid() && isDocument(), "ScriptDocument::Impl::isDocumentModified: only to be called for real documents!" );
        bool bIsModified = false;
        if ( isValid() && isDocument() )
        {
            try
            {
                bIsModified = m_xDocModify->isModified();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return bIsModified;
    }


    bool ScriptDocument::Impl::saveDocument( const Reference< XStatusIndicator >& _rxStatusIndicator ) const
    {
        Reference< XFrame > xFrame;
        if ( !getCurrentFrame( xFrame ) )
            return false;

        Sequence< PropertyValue > aArgs;
        if ( _rxStatusIndicator.is() )
        {
            aArgs.realloc(1);
            aArgs[0].Name = "StatusIndicator" ;
            aArgs[0].Value <<= _rxStatusIndicator;
        }

        try
        {
            URL aURL;
            aURL.Complete = ".uno:Save" ;
            aURL.Main = aURL.Complete;
            aURL.Protocol = ".uno:" ;
            aURL.Path = "Save" ;

            Reference< XDispatchProvider > xDispProv( xFrame, UNO_QUERY_THROW );
            Reference< XDispatch > xDispatch(
                xDispProv->queryDispatch( aURL, "_self", FrameSearchFlag::AUTO ),
                UNO_SET_THROW );

            xDispatch->dispatch( aURL, aArgs );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            return false;
        }

        return true;
    }


    OUString ScriptDocument::Impl::getTitle() const
    {
        OSL_PRECOND( isValid() && isDocument(), "ScriptDocument::Impl::getTitle: for documents only!" );

        OUString sTitle;
        if ( isValid() && isDocument() )
        {
            sTitle = ::comphelper::DocumentInfo::getDocumentTitle( m_xDocument );
        }
        return sTitle;
    }


    OUString ScriptDocument::Impl::getURL() const
    {
        OSL_PRECOND( isValid() && isDocument(), "ScriptDocument::Impl::getURL: for documents only!" );

        OUString sURL;
        if ( isValid() && isDocument() )
        {
            try
            {
                sURL = m_xDocument->getURL();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return sURL;
    }


    bool ScriptDocument::Impl::allowMacros() const
    {
        OSL_ENSURE( isValid() && isDocument(), "ScriptDocument::Impl::allowMacros: for documents only!" );
        bool bAllow = false;
        if ( isValid() && isDocument() )
        {
            try
            {
                bAllow = m_xScriptAccess->getAllowMacroExecution();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return bAllow;
    }


    bool ScriptDocument::Impl::getCurrentFrame( Reference< XFrame >& _out_rxFrame ) const
    {
        _out_rxFrame.clear();
        OSL_PRECOND( isValid() && isDocument(), "ScriptDocument::Impl::getCurrentFrame: documents only!" );
        if ( !isValid() || !isDocument() )
            return false;

        try
        {
            Reference< XModel > xDocument( m_xDocument, UNO_SET_THROW );
            Reference< XController > xController( xDocument->getCurrentController(), UNO_SET_THROW );
            _out_rxFrame.set( xController->getFrame(), UNO_SET_THROW );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return _out_rxFrame.is();
    }


    bool ScriptDocument::Impl::isLibraryShared( const OUString& _rLibName, LibraryContainerType _eType )
    {
        bool bIsShared = false;
        try
        {
            Reference< XLibraryContainer2 > xLibContainer( getLibraryContainer( _eType ), UNO_QUERY_THROW );

            if ( !xLibContainer->hasByName( _rLibName ) || !xLibContainer->isLibraryLink( _rLibName ) )
                return false;
            OUString aFileURL;
            Reference< XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
            Reference< XUriReferenceFactory > xUriFac;
            if ( xMSF.is() )
            {
                xUriFac.set(
                    xMSF->createInstance( "com.sun.star.uri.UriReferenceFactory" ),
                    UNO_QUERY_THROW );
            }

            OUString aLinkURL( xLibContainer->getLibraryLinkURL( _rLibName ) );
            Reference< XUriReference > xUriRef( xUriFac->parse( aLinkURL ), UNO_QUERY_THROW );

            OUString aScheme = xUriRef->getScheme();
            if ( aScheme.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("file")) )
            {
                aFileURL = aLinkURL;
            }
            else if ( aScheme.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.pkg")) )
            {
                OUString aAuthority = xUriRef->getAuthority();
                if ( aAuthority.matchIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.expand:")) )
                {
                    OUString aDecodedURL( aAuthority.copy( sizeof ( "vnd.sun.star.expand:" ) - 1 ) );
                    aDecodedURL = ::rtl::Uri::decode( aDecodedURL, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
                    Reference< XComponentContext > xContext(
                        comphelper::getComponentContext( xMSF ) );
                    Reference< XMacroExpander > xMacroExpander(
                        xContext->getValueByName(
                        "/singletons/com.sun.star.util.theMacroExpander" ),
                        UNO_QUERY_THROW );
                    aFileURL = xMacroExpander->expandMacros( aDecodedURL );
                }
            }

            if ( !aFileURL.isEmpty() )
            {
                ::osl::DirectoryItem aFileItem;
                ::osl::FileStatus aFileStatus( osl_FileStatus_Mask_FileURL );
                OSL_VERIFY( ::osl::DirectoryItem::get( aFileURL, aFileItem ) == ::osl::FileBase::E_None );
                OSL_VERIFY( aFileItem.getFileStatus( aFileStatus ) == ::osl::FileBase::E_None );
                OUString aCanonicalFileURL( aFileStatus.getFileURL() );

                OUString aSearchURL1( "share/basic" );
                OUString aSearchURL2( "share/uno_packages" );
                OUString aSearchURL3( "share/extensions" );
                if( aCanonicalFileURL.indexOf( aSearchURL1 ) >= 0 ||
                    aCanonicalFileURL.indexOf( aSearchURL2 ) >= 0 ||
                    aCanonicalFileURL.indexOf( aSearchURL3 ) >= 0 )
                        bIsShared = true;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return bIsShared;
    }


    void ScriptDocument::Impl::onDocumentCreated( const ScriptDocument& /*_rDocument*/ )
    {
        // not interested in
    }

    void ScriptDocument::Impl::onDocumentOpened( const ScriptDocument& /*_rDocument*/ )
    {
        // not interested in
    }

    void ScriptDocument::Impl::onDocumentSave( const ScriptDocument& /*_rDocument*/ )
    {
        // not interested in
    }

    void ScriptDocument::Impl::onDocumentSaveDone( const ScriptDocument& /*_rDocument*/ )
    {
        // not interested in
    }

    void ScriptDocument::Impl::onDocumentSaveAs( const ScriptDocument& /*_rDocument*/ )
    {
        // not interested in
    }

    void ScriptDocument::Impl::onDocumentSaveAsDone( const ScriptDocument& /*_rDocument*/ )
    {
        // not interested in
    }

    void ScriptDocument::Impl::onDocumentClosed( const ScriptDocument& _rDocument )
    {
        DBG_TESTSOLARMUTEX();
        OSL_PRECOND( isValid(), "ScriptDocument::Impl::onDocumentClosed: should not be listening if I'm not valid!" );

        bool bMyDocument = m_xDocument == _rDocument.getDocument();
        OSL_PRECOND( bMyDocument, "ScriptDocument::Impl::onDocumentClosed: didn't want to know *this*!" );
        if ( bMyDocument )
        {
            m_bDocumentClosed = true;
        }
    }


    void ScriptDocument::Impl::onDocumentTitleChanged( const ScriptDocument& /*_rDocument*/ )
    {
        // not interested in
    }

    void ScriptDocument::Impl::onDocumentModeChanged( const ScriptDocument& /*_rDocument*/ )
    {
        // not interested in
    }

    //====================================================================
    //= ScriptDocument
    //====================================================================
    ScriptDocument::ScriptDocument()
        :m_pImpl(new Impl)
    { }


    ScriptDocument::ScriptDocument( ScriptDocument::SpecialDocument _eType )
        :m_pImpl( new Impl( Reference< XModel >() ) )
    {
        OSL_ENSURE( _eType == NoDocument, "ScriptDocument::ScriptDocument: unknown SpecialDocument type!" );
        (void)_eType;
    }


    ScriptDocument::ScriptDocument( const Reference< XModel >& _rxDocument )
        :m_pImpl( new Impl( _rxDocument ) )
    {
        OSL_ENSURE( _rxDocument.is(), "ScriptDocument::ScriptDocument: document must not be NULL!" );
            // a NULL document results in an uninitialized instance, and for this
            // purpose, there is a dedicated constructor
    }


    ScriptDocument::ScriptDocument( const ScriptDocument& _rSource )
        :m_pImpl( _rSource.m_pImpl )
    {
    }


    ScriptDocument::~ScriptDocument()
    {
    }


    const ScriptDocument& ScriptDocument::getApplicationScriptDocument()
    {
        static ScriptDocument s_aApplicationScripts;
        return s_aApplicationScripts;
    }


    ScriptDocument ScriptDocument::getDocumentForBasicManager( const BasicManager* _pManager )
    {
        if ( _pManager == SFX_APP()->GetBasicManager() )
            return getApplicationScriptDocument();

        docs::Documents aDocuments;
        lcl_getAllModels_throw( aDocuments, false );

        for (   docs::Documents::const_iterator doc = aDocuments.begin();
                doc != aDocuments.end();
                ++doc
            )
        {
            const BasicManager* pDocBasicManager = ::basic::BasicManagerRepository::getDocumentBasicManager( doc->xModel );
            if  (   ( pDocBasicManager != SFX_APP()->GetBasicManager() )
                &&  ( pDocBasicManager == _pManager )
                )
            {
                return ScriptDocument( doc->xModel );
            }
        }

        OSL_FAIL( "ScriptDocument::getDocumentForBasicManager: did not find a document for this manager!" );
        return ScriptDocument( NoDocument );
    }


    ScriptDocument ScriptDocument::getDocumentWithURLOrCaption( const OUString& _rUrlOrCaption )
    {
        ScriptDocument aDocument( getApplicationScriptDocument() );
        if ( _rUrlOrCaption.isEmpty() )
            return aDocument;

        docs::Documents aDocuments;
        lcl_getAllModels_throw( aDocuments, false );

        for (   docs::Documents::const_iterator doc = aDocuments.begin();
                doc != aDocuments.end();
                ++doc
            )
        {
            const ScriptDocument aCheck = ScriptDocument( doc->xModel );
            if  (   _rUrlOrCaption == aCheck.getTitle()
                ||  _rUrlOrCaption == aCheck.getURL()
                )
            {
                aDocument = aCheck;
                break;
            }
        }

        return aDocument;
    }


    namespace
    {
        struct DocumentTitleLess : public ::std::binary_function< ScriptDocument, ScriptDocument, bool >
        {
            DocumentTitleLess( const CollatorWrapper& _rCollator )
                :m_aCollator( _rCollator )
            {
            }

            bool operator()( const ScriptDocument& _lhs, const ScriptDocument& _rhs ) const
            {
                return m_aCollator.compareString( _lhs.getTitle(), _rhs.getTitle() ) < 0;
            }
        private:
            const CollatorWrapper   m_aCollator;
        };
    }


    ScriptDocuments ScriptDocument::getAllScriptDocuments( ScriptDocument::ScriptDocumentList _eListType )
    {
        ScriptDocuments aScriptDocs;

        // include application?
        if ( _eListType == AllWithApplication )
            aScriptDocs.push_back( getApplicationScriptDocument() );

        // obtain documents
        try
        {
            docs::Documents aDocuments;
            lcl_getAllModels_throw( aDocuments, true /* exclude invisible */ );

            for (   docs::Documents::const_iterator doc = aDocuments.begin();
                    doc != aDocuments.end();
                    ++doc
                )
            {
                // exclude documents without script/library containers
                ScriptDocument aDoc( doc->xModel );
                if ( !aDoc.isValid() )
                    continue;

                aScriptDocs.push_back( aDoc );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        // sort document list by doc title?
        if ( _eListType == DocumentsSorted )
        {
            CollatorWrapper aCollator( ::comphelper::getProcessServiceFactory() );
            aCollator.loadDefaultCollator( SvtSysLocale().GetLocaleData().getLocale(), 0 );
            ::std::sort( aScriptDocs.begin(), aScriptDocs.end(), DocumentTitleLess( aCollator ) );
        }

        return aScriptDocs;
    }


    bool ScriptDocument::operator==( const ScriptDocument& _rhs ) const
    {
        return m_pImpl->getDocumentRef() == _rhs.m_pImpl->getDocumentRef();
    }


    sal_Int32 ScriptDocument::hashCode() const
    {
        return sal::static_int_cast<sal_Int32>(reinterpret_cast< sal_IntPtr >( m_pImpl->getDocumentRef().get() ));
    }


    bool ScriptDocument::isValid() const
    {
        return m_pImpl->isValid();
    }


    bool ScriptDocument::isAlive() const
    {
        return m_pImpl->isAlive();
    }


    Reference< XLibraryContainer > ScriptDocument::getLibraryContainer( LibraryContainerType _eType ) const
    {
        return m_pImpl->getLibraryContainer( _eType );
    }


    Reference< XNameContainer > ScriptDocument::getLibrary( LibraryContainerType _eType, const OUString& _rLibName, bool _bLoadLibrary ) const
        SAL_THROW((NoSuchElementException))
    {
        return m_pImpl->getLibrary( _eType, _rLibName, _bLoadLibrary );
    }


    bool ScriptDocument::hasLibrary( LibraryContainerType _eType, const OUString& _rLibName ) const
    {
        return m_pImpl->hasLibrary( _eType, _rLibName );
    }


    Reference< XNameContainer > ScriptDocument::getOrCreateLibrary( LibraryContainerType _eType, const OUString& _rLibName ) const
    {
        return m_pImpl->getOrCreateLibrary( _eType, _rLibName );
    }


    void ScriptDocument::loadLibraryIfExists( LibraryContainerType _eType, const OUString& _rLibrary )
    {
        m_pImpl->loadLibraryIfExists( _eType, _rLibrary );
    }


    Sequence< OUString > ScriptDocument::getObjectNames( LibraryContainerType _eType, const OUString& _rLibName ) const
    {
        Sequence< OUString > aModuleNames;

        try
        {
            if ( hasLibrary( _eType, _rLibName ) )
            {
                Reference< XNameContainer > xLib( getLibrary( _eType, _rLibName, false ) );
                if ( xLib.is() )
                    aModuleNames = xLib->getElementNames();
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        // sort
        ::std::sort( aModuleNames.getArray() , aModuleNames.getArray() + aModuleNames.getLength() , StringCompareLessThan );

        return aModuleNames;
    }


    OUString ScriptDocument::createObjectName( LibraryContainerType _eType, const OUString& _rLibName ) const
    {
        OUString aObjectName;

        OUString aBaseName = _eType == E_SCRIPTS ? "Module" : "Dialog";

        Sequence< OUString > aUsedNames( getObjectNames( _eType, _rLibName ) );
        ::std::set< OUString > aUsedNamesCheck;
        ::std::copy( aUsedNames.getConstArray(), aUsedNames.getConstArray() + aUsedNames.getLength(),
            ::std::insert_iterator< ::std::set< OUString > >( aUsedNamesCheck, aUsedNamesCheck.begin() ) );

        bool bValid = false;
        sal_Int32 i = 1;
        while ( !bValid )
        {
            aObjectName = aBaseName;
            aObjectName += OUString::valueOf( i );

            if ( aUsedNamesCheck.find( aObjectName ) == aUsedNamesCheck.end() )
                bValid = true;

            ++i;
        }

        return aObjectName;
    }


    Sequence< OUString > ScriptDocument::getLibraryNames() const
    {
        return GetMergedLibraryNames( getLibraryContainer( E_SCRIPTS ), getLibraryContainer( E_DIALOGS ) );
    }


    bool ScriptDocument::isReadOnly() const
    {
        return m_pImpl->isReadOnly();
    }


    bool ScriptDocument::isApplication() const
    {
        return m_pImpl->isApplication();
    }

    bool ScriptDocument::isInVBAMode() const
    {
        return m_pImpl->isInVBAMode();
    }


    BasicManager* ScriptDocument::getBasicManager() const
    {
        return m_pImpl->getBasicManager();
    }


    Reference< XModel > ScriptDocument::getDocument() const
    {
        return m_pImpl->getDocument();
    }


    Reference< XModel > ScriptDocument::getDocumentOrNull() const
    {
        if ( isDocument() )
            return m_pImpl->getDocument();
        return NULL;
    }


    bool ScriptDocument::removeModule( const OUString& _rLibName, const OUString& _rModuleName ) const
    {
        return m_pImpl->removeModuleOrDialog( E_SCRIPTS, _rLibName, _rModuleName );
    }


    bool ScriptDocument::hasModule( const OUString& _rLibName, const OUString& _rModuleName ) const
    {
        return m_pImpl->hasModuleOrDialog( E_SCRIPTS, _rLibName, _rModuleName );
    }


    bool ScriptDocument::getModule( const OUString& _rLibName, const OUString& _rModName, OUString& _out_rModuleSource ) const
    {
        Any aCode;
        if ( !m_pImpl->getModuleOrDialog( E_SCRIPTS, _rLibName, _rModName, aCode ) )
            return false;
        OSL_VERIFY( aCode >>= _out_rModuleSource );
        return true;
    }


    bool ScriptDocument::renameModule( const OUString& _rLibName, const OUString& _rOldName, const OUString& _rNewName ) const
    {
        return m_pImpl->renameModuleOrDialog( E_SCRIPTS, _rLibName, _rOldName, _rNewName, NULL );
    }


    bool ScriptDocument::createModule( const OUString& _rLibName, const OUString& _rModName, bool _bCreateMain, OUString& _out_rNewModuleCode ) const
    {
        if ( !m_pImpl->createModule( _rLibName, _rModName, _bCreateMain, _out_rNewModuleCode ) )
            return false;

        // doc shell modified
        MarkDocumentModified( *const_cast< ScriptDocument* >( this ) );    // here?
        return true;
    }


    bool ScriptDocument::insertModule( const OUString& _rLibName, const OUString& _rModName, const OUString& _rModuleCode ) const
    {
        return m_pImpl->insertModuleOrDialog( E_SCRIPTS, _rLibName, _rModName, makeAny( _rModuleCode ) );
    }


    bool ScriptDocument::updateModule( const OUString& _rLibName, const OUString& _rModName, const OUString& _rModuleCode ) const
    {
        return m_pImpl->updateModule( _rLibName, _rModName, _rModuleCode );
    }


    bool ScriptDocument::removeDialog( const OUString& _rLibName, const OUString& _rDialogName ) const
    {
        return m_pImpl->removeModuleOrDialog( E_DIALOGS, _rLibName, _rDialogName );
    }


    bool ScriptDocument::hasDialog( const OUString& _rLibName, const OUString& _rDialogName ) const
    {
        return m_pImpl->hasModuleOrDialog( E_DIALOGS, _rLibName, _rDialogName );
    }


    bool ScriptDocument::getDialog( const OUString& _rLibName, const OUString& _rDialogName, Reference< XInputStreamProvider >& _out_rDialogProvider ) const
    {
        Any aCode;
        if ( !m_pImpl->getModuleOrDialog( E_DIALOGS, _rLibName, _rDialogName, aCode ) )
            return false;
        OSL_VERIFY( aCode >>= _out_rDialogProvider );
        return _out_rDialogProvider.is();
    }


    bool ScriptDocument::renameDialog( const OUString& _rLibName, const OUString& _rOldName, const OUString& _rNewName, const Reference< XNameContainer >& _rxExistingDialogModel ) const
    {
        return m_pImpl->renameModuleOrDialog( E_DIALOGS, _rLibName, _rOldName, _rNewName, _rxExistingDialogModel );
    }


    bool ScriptDocument::createDialog( const OUString& _rLibName, const OUString& _rDialogName, Reference< XInputStreamProvider >& _out_rDialogProvider ) const
    {
        if ( !m_pImpl->createDialog( _rLibName, _rDialogName, _out_rDialogProvider ) )
            return false;

        MarkDocumentModified( *const_cast< ScriptDocument* >( this ) );    // here?
        return true;
    }


    bool ScriptDocument::insertDialog( const OUString& _rLibName, const OUString& _rDialogName, const Reference< XInputStreamProvider >& _rxDialogProvider ) const
    {
        return m_pImpl->insertModuleOrDialog( E_DIALOGS, _rLibName, _rDialogName, makeAny( _rxDialogProvider ) );
    }


    void ScriptDocument::setDocumentModified() const
    {
        m_pImpl->setDocumentModified();
    }


    bool ScriptDocument::isDocumentModified() const
    {
        return m_pImpl->isDocumentModified();
    }


    bool ScriptDocument::saveDocument( const Reference< XStatusIndicator >& _rxStatusIndicator ) const
    {
        return m_pImpl->saveDocument( _rxStatusIndicator );
    }


    LibraryLocation ScriptDocument::getLibraryLocation( const OUString& _rLibName ) const
    {
        LibraryLocation eLocation = LIBRARY_LOCATION_UNKNOWN;
        if ( !_rLibName.isEmpty() )
        {
            if ( isDocument() )
            {
                eLocation = LIBRARY_LOCATION_DOCUMENT;
            }
            else
            {
                if  (  ( hasLibrary( E_SCRIPTS, _rLibName ) && !m_pImpl->isLibraryShared( _rLibName, E_SCRIPTS ) )
                    || ( hasLibrary( E_DIALOGS, _rLibName ) && !m_pImpl->isLibraryShared( _rLibName, E_DIALOGS ) )
                    )
                {
                    eLocation = LIBRARY_LOCATION_USER;
                }
                else
                {
                    eLocation = LIBRARY_LOCATION_SHARE;
                }
            }
        }

        return eLocation;
    }


    OUString ScriptDocument::getTitle( LibraryLocation _eLocation, LibraryType _eType ) const
    {
        OUString aTitle;

        switch ( _eLocation )
        {
            case LIBRARY_LOCATION_USER:
            {
                switch ( _eType )
                {
                case LIBRARY_TYPE_MODULE:   aTitle = IDE_RESSTR(RID_STR_USERMACROS); break;
                case LIBRARY_TYPE_DIALOG:   aTitle = IDE_RESSTR(RID_STR_USERDIALOGS); break;
                case LIBRARY_TYPE_ALL:      aTitle = IDE_RESSTR(RID_STR_USERMACROSDIALOGS); break;
                default:
                    break;
            }
            break;
            case LIBRARY_LOCATION_SHARE:
            {
                switch ( _eType )
                {
                case LIBRARY_TYPE_MODULE:   aTitle = IDE_RESSTR(RID_STR_SHAREMACROS); break;
                case LIBRARY_TYPE_DIALOG:   aTitle = IDE_RESSTR(RID_STR_SHAREDIALOGS); break;
                case LIBRARY_TYPE_ALL:      aTitle = IDE_RESSTR(RID_STR_SHAREMACROSDIALOGS); break;
                default:
                    break;
                }
            }
            break;
            case LIBRARY_LOCATION_DOCUMENT:
                aTitle = getTitle();
                break;
            default:
                break;
            }
        }

        return aTitle;
    }


    OUString ScriptDocument::getTitle() const
    {
        return m_pImpl->getTitle();
    }


    OUString ScriptDocument::getURL() const
    {
        return m_pImpl->getURL();
    }


    bool ScriptDocument::isActive() const
    {
        bool bIsActive( false );
        try
        {
            Reference< XFrame > xFrame;
            if ( m_pImpl->getCurrentFrame( xFrame ) )
                bIsActive = xFrame->isActive();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return bIsActive;
    }


    bool ScriptDocument::allowMacros() const
    {
        return m_pImpl->allowMacros();
    }

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
