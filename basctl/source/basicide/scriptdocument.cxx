/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scriptdocument.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-15 15:59:30 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basctl.hxx"

#include "scriptdocument.hxx"
#include "basobj.hxx"
#include "basidesh.hrc"
#include "iderid.hxx"
#include "dlgeddef.hxx"
#include "localizationmgr.hxx"

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINER2_HPP_
#include <com/sun/star/script/XLibraryContainer2.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_URI_XURIREFERENCEFACTORY_HPP_
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMACROEXPANDER_HPP_
#include <com/sun/star/util/XMacroExpander.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_MACROEXECMODE_HPP_
#include <com/sun/star/document/MacroExecMode.hpp>
#endif
/** === end UNO includes === **/

#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/docfile.hxx>

#include <xmlscript/xmldlg_imexp.hxx>

#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>

#include <rtl/uri.hxx>

#include <osl/process.h>
#include <osl/file.hxx>

#include <algorithm>
#include <set>

//........................................................................
namespace basctl
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::script::XLibraryContainer;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
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
    /** === end UNO using === **/
    namespace MacroExecMode = ::com::sun::star::document::MacroExecMode;

    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        //----------------------------------------------------------------
        static const ::rtl::OUString&   lcl_getScriptLibrariesPropertyName()
        {
            static ::rtl::OUString s_sBasicLibraries( RTL_CONSTASCII_USTRINGPARAM( "BasicLibraries" ) );
            return s_sBasicLibraries;
        }

        //----------------------------------------------------------------
        static const ::rtl::OUString&   lcl_getDialogLibrariesPropertyName()
        {
            static ::rtl::OUString s_sBasicLibraries( RTL_CONSTASCII_USTRINGPARAM( "DialogLibraries" ) );
            return s_sBasicLibraries;
        }

        //----------------------------------------------------------------
        static bool StringCompareLessThan( const String& lhs, const String& rhs )
        {
            return ( lhs.CompareIgnoreCaseToAscii( rhs ) == COMPARE_LESS );
        }
    }

    //====================================================================
    //= ScriptDocument_Impl
    //====================================================================
    class ScriptDocument_Impl
    {
    private:
        SfxObjectShell*             m_pShell;
        bool                        m_bIsApplication;
        bool                        m_bValid;
        Reference< XModel >         m_aDocument;
        Reference< XPropertySet >   m_xDocProperties;

    public:
        ScriptDocument_Impl( );
        ScriptDocument_Impl( SfxObjectShell& _rShell );
        ScriptDocument_Impl( const Reference< XModel >& _rxDocument );

        /** determines whether the instance refers to a valid "document" with script and
            dialog libraries
        */
        inline  bool    isValid()       const   { return m_bValid; }
        /// determines whether the "document" refers to the application in real
        inline  bool    isApplication() const   { return m_bValid && m_bIsApplication; }
        /// determines whether the document refers to a real document (instead of the application)
        inline  bool    isDocument()    const   { return m_bValid && !m_bIsApplication; }

        /// returns the SfxObjectShell the document is based on
        SfxObjectShell* getObjectShell() const { return m_pShell; }

        /// returns a library container belonging to the document
        Reference< XLibraryContainer >
                    getLibraryContainer( LibraryContainerType _eType ) const;

        /// determines whether a given library is part of the shared installation
        bool        isLibraryShared( const ::rtl::OUString& _rLibName, LibraryContainerType _eType );

        // versions with the same signature/semantics as in ScriptDocument itself
        bool        isReadOnly() const;
        BasicManager*
                    getBasicManager() const;
        Reference< XModel >
                    getDocument() const;
        void        setDocumentModified() const;
        bool        isDocumentModified() const;
        bool        saveDocument( const Reference< XStatusIndicator >& _rxStatusIndicator ) const;

        Reference< XNameContainer >
                    getLibrary( LibraryContainerType _eType, const ::rtl::OUString& _rLibName, bool _bLoadLibrary ) const
                        SAL_THROW((NoSuchElementException));
        bool        hasLibrary( LibraryContainerType _eType, const ::rtl::OUString& _rLibName ) const;
        Reference< XNameContainer >
                    getOrCreateLibrary( LibraryContainerType _eType, const ::rtl::OUString& _rLibName ) const;

        void        loadLibraryIfExists( LibraryContainerType _eType, const ::rtl::OUString& _rLibrary );

        bool        removeModuleOrDialog( LibraryContainerType _eType, const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rModuleName );
        bool        hasModuleOrDialog( LibraryContainerType _eType, const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rModName ) const;
        bool        getModuleOrDialog( LibraryContainerType _eType, const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rObjectName, Any& _out_rModuleOrDialog );
        bool        renameModuleOrDialog( LibraryContainerType _eType, const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rOldName, const ::rtl::OUString& _rNewName, const Reference< XNameContainer >& _rxExistingDialogModel );
        bool        createModule( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rModName, bool _bCreateMain, ::rtl::OUString& _out_rNewModuleCode ) const;
        bool        insertModuleOrDialog( LibraryContainerType _eType, const ::rtl::OUString& _rObjectName, const ::rtl::OUString& _rModName, const Any& _rElement ) const;
        bool        updateModule( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rModName, const ::rtl::OUString& _rModuleCode ) const;
        bool        createDialog( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rDialogName, Reference< XInputStreamProvider >& _out_rDialogProvider ) const;

    private:
        void    impl_initDocument_nothrow( SfxObjectShell& _rShell );
    };

    //--------------------------------------------------------------------
    ScriptDocument_Impl::ScriptDocument_Impl()
        :m_pShell( NULL )
        ,m_bIsApplication( true )
        ,m_bValid( true )
    {
    }

    //--------------------------------------------------------------------
    ScriptDocument_Impl::ScriptDocument_Impl( SfxObjectShell& _rShell )
        :m_pShell( NULL )
        ,m_bIsApplication( false )
        ,m_bValid( false )
    {
        impl_initDocument_nothrow( _rShell );
    }

    //--------------------------------------------------------------------
    ScriptDocument_Impl::ScriptDocument_Impl( const Reference< XModel >& _rxDocument )
        :m_pShell( NULL )
        ,m_bIsApplication( false )
        ,m_bValid( false )
    {
        if ( _rxDocument.is() )
        {
            SfxObjectShell* pShell = SfxObjectShell::GetFirst();
            while ( pShell )
            {
                if ( pShell->GetModel() == _rxDocument )
                    break;
                pShell = SfxObjectShell::GetNext( *pShell );
            }

            OSL_ENSURE( pShell, "ScriptDocument_Impl::ScriptDocument_Impl: did not find the shell for the given model!" );
            if ( pShell )
                impl_initDocument_nothrow( *pShell );
        }
    }

    //--------------------------------------------------------------------
    void ScriptDocument_Impl::impl_initDocument_nothrow( SfxObjectShell& _rShell )
    {
        m_pShell = &_rShell;

        try
        {
            m_aDocument.set     ( m_pShell->GetModel(), UNO_QUERY_THROW );
            m_xDocProperties.set( m_pShell->GetModel(), UNO_QUERY );
            if ( m_xDocProperties.is() )
            {

                Reference< XPropertySetInfo >   xPSI( m_xDocProperties->getPropertySetInfo(), UNO_QUERY_THROW );
                m_bValid =
                        xPSI->hasPropertyByName( lcl_getScriptLibrariesPropertyName() )
                    &&  xPSI->hasPropertyByName( lcl_getDialogLibrariesPropertyName() );
            }
            else
                m_bValid = false;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            m_bValid = false;
        }

        if ( !m_bValid )
        {
            m_aDocument.clear();
            m_xDocProperties.clear();
        }
    }
    //--------------------------------------------------------------------
    Reference< XLibraryContainer > ScriptDocument_Impl::getLibraryContainer( LibraryContainerType _eType ) const
    {
        OSL_ENSURE( isValid(), "ScriptDocument_Impl::getLibraryContainer: invalid!" );

        Reference< XLibraryContainer > xContainer;
        if ( !isValid() )
            return xContainer;

        try
        {
            if ( isApplication() )
                xContainer.set( _eType == E_SCRIPTS ? SFX_APP()->GetBasicContainer() : SFX_APP()->GetDialogContainer(), UNO_QUERY_THROW );
            else
            {
                xContainer.set( m_xDocProperties->getPropertyValue(
                    _eType == E_SCRIPTS ? lcl_getScriptLibrariesPropertyName() : lcl_getDialogLibrariesPropertyName() ),
                    UNO_QUERY_THROW );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return xContainer;
    }

    //--------------------------------------------------------------------
    bool ScriptDocument_Impl::isReadOnly() const
    {
        OSL_ENSURE( isValid(), "ScriptDocument_Impl::isReadOnly: invalid state!" );
        OSL_ENSURE( !isApplication(), "ScriptDocument_Impl::isReadOnly: not allowed to be called for the application!" );
        if ( !isValid() || isApplication() )
            return true;
        return m_pShell->IsReadOnly();
    }

    //--------------------------------------------------------------------
    BasicManager* ScriptDocument_Impl::getBasicManager() const
    {
        OSL_ENSURE( isValid(), "ScriptDocument_Impl::getBasicManager: invalid state!" );
        if ( !isValid() )
            return NULL;

        if ( isApplication() )
            return SFX_APP()->GetBasicManager();

        return m_pShell->GetBasicManager();
    }

    //--------------------------------------------------------------------
    Reference< XModel > ScriptDocument_Impl::getDocument() const
    {
        OSL_ENSURE( isValid(), "ScriptDocument_Impl::getDocument: invalid state!" );
        OSL_ENSURE( isDocument(), "ScriptDocument_Impl::getDocument: for documents only!" );
        if ( !isValid() || !isDocument() )
            return NULL;

        return m_aDocument;
    }

    //--------------------------------------------------------------------
    Reference< XNameContainer > ScriptDocument_Impl::getLibrary( LibraryContainerType _eType, const ::rtl::OUString& _rLibName, bool _bLoadLibrary ) const
        SAL_THROW((NoSuchElementException))
    {
        OSL_ENSURE( isValid(), "ScriptDocument_Impl::getLibrary: invalid state!" );

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

    //--------------------------------------------------------------------
    bool ScriptDocument_Impl::hasLibrary( LibraryContainerType _eType, const ::rtl::OUString& _rLibName ) const
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

    //--------------------------------------------------------------------
    Reference< XNameContainer > ScriptDocument_Impl::getOrCreateLibrary( LibraryContainerType _eType, const ::rtl::OUString& _rLibName ) const
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

    //--------------------------------------------------------------------
    void ScriptDocument_Impl::loadLibraryIfExists( LibraryContainerType _eType, const ::rtl::OUString& _rLibrary )
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

    //--------------------------------------------------------------------
    bool ScriptDocument_Impl::removeModuleOrDialog( LibraryContainerType _eType, const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rModuleName )
    {
        OSL_ENSURE( isValid(), "ScriptDocument_Impl::removeModuleOrDialog: invalid!" );
        if ( isValid() )
        {
            try
            {
                Reference< XNameContainer > xLib( getLibrary( _eType, _rLibName, TRUE ) );
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

    //--------------------------------------------------------------------
    bool ScriptDocument_Impl::hasModuleOrDialog( LibraryContainerType _eType, const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rModName ) const
    {
        OSL_ENSURE( isValid(), "ScriptDocument_Impl::hasModuleOrDialog: invalid!" );
        if ( !isValid() )
            return false;

        try
        {
            Reference< XNameContainer > xLib( getLibrary( _eType, _rLibName, TRUE ) );
            if ( xLib.is() )
                return xLib->hasByName( _rModName );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }

    //--------------------------------------------------------------------
    bool ScriptDocument_Impl::getModuleOrDialog( LibraryContainerType _eType, const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rObjectName, Any& _out_rModuleOrDialog )
    {
        OSL_ENSURE( isValid(), "ScriptDocument_Impl::getModuleOrDialog: invalid!" );
        if ( !isValid() )
            return false;

        _out_rModuleOrDialog.clear();
        try
        {
            Reference< XNameContainer > xLib( getLibrary( _eType, _rLibName, TRUE ), UNO_QUERY_THROW );
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

    //--------------------------------------------------------------------
    bool ScriptDocument_Impl::renameModuleOrDialog( LibraryContainerType _eType, const ::rtl::OUString& _rLibName,
        const ::rtl::OUString& _rOldName, const ::rtl::OUString& _rNewName, const Reference< XNameContainer >& _rxExistingDialogModel )
    {
        OSL_ENSURE( isValid(), "ScriptDocument_Impl::renameModuleOrDialog: invalid!" );
        if ( !isValid() )
            return false;

        try
        {
            Reference< XNameContainer > xLib( getLibrary( _eType, _rLibName, TRUE ), UNO_QUERY_THROW );

            // get element
            Any aElement( xLib->getByName( _rOldName ) );

            // remove element from container
            xLib->removeByName( _rOldName );

            // if it's a dialog, import and export, to reflect the new name
            if ( _eType == E_DIALOGS )
            {
                // create dialog model
                ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
                Reference< XNameContainer > xDialogModel;
                if ( _rxExistingDialogModel.is() )
                    xDialogModel = _rxExistingDialogModel;
                else
                    if ( !aContext.createComponent( "com.sun.star.awt.UnoControlDialogModel", xDialogModel ) )
                        return false;

                // import dialog model
                Reference< XInputStreamProvider > xISP( aElement, UNO_QUERY_THROW );
                if ( !_rxExistingDialogModel.is() )
                {
                    Reference< XInputStream > xInput( xISP->createInputStream(), UNO_QUERY_THROW );
                    ::xmlscript::importDialogModel( xInput, xDialogModel, aContext.getUNOContext() );
                }

                // set new name as property
                Reference< XPropertySet > xDlgPSet( xDialogModel, UNO_QUERY_THROW );
                xDlgPSet->setPropertyValue( DLGED_PROP_NAME, makeAny( _rNewName ) );

                // export dialog model
                xISP = ::xmlscript::exportDialogModel( xDialogModel, aContext.getUNOContext() );
                aElement <<= xISP;
            }

            // insert element by new name in container
            xLib->insertByName( _rNewName, aElement );
            return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }

    //--------------------------------------------------------------------
    bool ScriptDocument_Impl::createModule( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rModName, bool _bCreateMain, ::rtl::OUString& _out_rNewModuleCode ) const
    {
        _out_rNewModuleCode = ::rtl::OUString();
        try
        {
            Reference< XNameContainer > xLib( getLibrary( E_SCRIPTS, _rLibName, TRUE ) );
            if ( !xLib.is() || xLib->hasByName( _rModName ) )
                return false;

            // create new module
            _out_rNewModuleCode = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "REM  *****  BASIC  *****\n\n" ) );
            if ( _bCreateMain )
                _out_rNewModuleCode += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Sub Main\n\nEnd Sub\n" ) );

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

    //--------------------------------------------------------------------
    bool ScriptDocument_Impl::insertModuleOrDialog( LibraryContainerType _eType, const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rObjectName, const Any& _rElement ) const
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

    //--------------------------------------------------------------------
    bool ScriptDocument_Impl::updateModule( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rModName, const ::rtl::OUString& _rModuleCode ) const
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

    //--------------------------------------------------------------------
    bool ScriptDocument_Impl::createDialog( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rDialogName, Reference< XInputStreamProvider >& _out_rDialogProvider ) const
    {
        try
        {
            Reference< XNameContainer > xLib( getLibrary( E_DIALOGS, _rLibName, TRUE ), UNO_QUERY_THROW );

            // create dialog
            _out_rDialogProvider.clear();
            if ( xLib->hasByName( _rDialogName ) )
                return false;

            // create new dialog model
            ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
            Reference< XNameContainer > xDialogModel;
            if ( !aContext.createComponent( "com.sun.star.awt.UnoControlDialogModel", xDialogModel ) )
                return false;

            // set name property
            Reference< XPropertySet > xDlgPSet( xDialogModel, UNO_QUERY_THROW );
            xDlgPSet->setPropertyValue( DLGED_PROP_NAME, makeAny( _rDialogName ) );

            // export dialog model
            _out_rDialogProvider = ::xmlscript::exportDialogModel( xDialogModel, aContext.getUNOContext() );

            // insert dialog into library
            xLib->insertByName( _rDialogName, makeAny( _out_rDialogProvider ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return _out_rDialogProvider.is();
    }

    //--------------------------------------------------------------------
    void ScriptDocument_Impl::setDocumentModified() const
    {
        OSL_ENSURE( isValid() && isDocument(), "ScriptDocument_Impl::setDocumentModified: only to be called for real documents!" );
        if ( isValid() && isDocument() )
            getObjectShell()->SetModified();
    }

    //--------------------------------------------------------------------
    bool ScriptDocument_Impl::isDocumentModified() const
    {
        OSL_ENSURE( isValid() && isDocument(), "ScriptDocument_Impl::isDocumentModified: only to be called for real documents!" );
        if ( isValid() && isDocument() )
            return getObjectShell()->IsModified();
        return false;
    }

    //--------------------------------------------------------------------
    bool ScriptDocument_Impl::saveDocument( const Reference< XStatusIndicator >& _rxStatusIndicator ) const
    {
        OSL_ENSURE( isValid() && isDocument(), "ScriptDocument_Impl::saveDocument: only to be called for real documents!" );
        if ( !isValid() || !isDocument() )
            return false;

        const SfxPoolItem* aArgs[2];
        aArgs[0] = aArgs[1] = NULL;

        ::std::auto_ptr< SfxUnoAnyItem > pStatusIndicatorItem;
        OSL_ENSURE( _rxStatusIndicator.is(), "ScriptDocument_Impl::saveDocument: no status indicator!" );
        if ( _rxStatusIndicator.is() )
            pStatusIndicatorItem.reset( new SfxUnoAnyItem( SID_PROGRESS_STATUSBAR_CONTROL, makeAny( _rxStatusIndicator ) ) );
        aArgs[0] = pStatusIndicatorItem.get();

        SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst( getObjectShell() );
        if ( !pViewFrame )
            return false;

        pViewFrame->GetBindings().Execute( SID_SAVEDOC, aArgs, 0, SFX_CALLMODE_SYNCHRON );
        return true;
    }

    //--------------------------------------------------------------------
    bool ScriptDocument_Impl::isLibraryShared( const ::rtl::OUString& _rLibName, LibraryContainerType _eType )
    {
        bool bIsShared = false;
        try
        {
            Reference< XLibraryContainer2 > xLibContainer( getLibraryContainer( _eType ), UNO_QUERY_THROW );

            if ( !xLibContainer->hasByName( _rLibName ) || !xLibContainer->isLibraryLink( _rLibName ) )
                return false;
            ::rtl::OUString aFileURL;
            Reference< XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );
            Reference< XUriReferenceFactory > xUriFac;
            if ( xMSF.is() )
            {
                xUriFac.set(
                    xMSF->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.uri.UriReferenceFactory" ) ) ),
                    UNO_QUERY_THROW );
            }

            ::rtl::OUString aLinkURL( xLibContainer->getLibraryLinkURL( _rLibName ) );
            Reference< XUriReference > xUriRef( xUriFac->parse( aLinkURL ), UNO_QUERY_THROW );

            ::rtl::OUString aScheme = xUriRef->getScheme();
            if ( aScheme.equalsIgnoreAsciiCaseAscii( "file" ) )
            {
                aFileURL = aLinkURL;
            }
            else if ( aScheme.equalsIgnoreAsciiCaseAscii( "vnd.sun.star.pkg" ) )
            {
                ::rtl::OUString aAuthority = xUriRef->getAuthority();
                if ( aAuthority.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "vnd.sun.star.expand:" ) ) )
                {
                    ::rtl::OUString aDecodedURL( aAuthority.copy( sizeof ( "vnd.sun.star.expand:" ) - 1 ) );
                    aDecodedURL = ::rtl::Uri::decode( aDecodedURL, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
                    Reference< XComponentContext > xContext;
                    Reference< XPropertySet > xProps( xMSF, UNO_QUERY_THROW );
                    xContext.set( xProps->getPropertyValue( ::rtl::OUString::createFromAscii( "DefaultContext" ) ), UNO_QUERY_THROW );
                    Reference< XMacroExpander > xMacroExpander(
                        xContext->getValueByName(
                        ::rtl::OUString::createFromAscii( "/singletons/com.sun.star.util.theMacroExpander" ) ),
                        UNO_QUERY_THROW );
                    aFileURL = xMacroExpander->expandMacros( aDecodedURL );
                }
            }

            if ( aFileURL.getLength() )
            {
                ::osl::DirectoryItem aFileItem;
                ::osl::FileStatus aFileStatus( FileStatusMask_FileURL );
                OSL_VERIFY( ::osl::DirectoryItem::get( aFileURL, aFileItem ) == ::osl::FileBase::E_None );
                OSL_VERIFY( aFileItem.getFileStatus( aFileStatus ) == ::osl::FileBase::E_None );
                ::rtl::OUString aCanonicalFileURL( aFileStatus.getFileURL() );

                ::rtl::OUString aShareURL;
                OSL_VERIFY( osl_getExecutableFile( &aShareURL.pData ) == osl_Process_E_None );
                sal_Int32 nIndex = aShareURL.lastIndexOf( '/' );
                if ( nIndex >= 0 )
                {
                    nIndex = aShareURL.lastIndexOf( '/', nIndex );
                    if ( nIndex >= 0 )
                    {
                        aShareURL = aShareURL.copy( 0, nIndex + 1 );
                        aShareURL += ::rtl::OUString::createFromAscii( "share" );
                    }
                }

                ::osl::DirectoryItem aShareItem;
                ::osl::FileStatus aShareStatus( FileStatusMask_FileURL );
                OSL_VERIFY( ::osl::DirectoryItem::get( aShareURL, aShareItem ) == ::osl::FileBase::E_None );
                OSL_VERIFY( aShareItem.getFileStatus( aShareStatus ) == ::osl::FileBase::E_None );
                ::rtl::OUString aCanonicalShareURL( aShareStatus.getFileURL() );

                if ( aCanonicalFileURL.match( aCanonicalShareURL ) )
                    bIsShared = true;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return bIsShared;
    }

    //====================================================================
    //= ScriptDocument
    //====================================================================
    //--------------------------------------------------------------------
    ScriptDocument::ScriptDocument()
        :m_pImpl( new ScriptDocument_Impl() )
    {
    }

    //--------------------------------------------------------------------
    ScriptDocument::ScriptDocument( ScriptDocument::SpecialDocument _eType )
        :m_pImpl( new ScriptDocument_Impl( Reference< XModel >() ) )
    {
        OSL_ENSURE( _eType == NoDocument, "ScriptDocument::ScriptDocument: unknown SpecialDocument type!" );
        (void)_eType;
    }

    //--------------------------------------------------------------------
    ScriptDocument::ScriptDocument( SfxObjectShell& _rShell )
        :m_pImpl( new ScriptDocument_Impl( _rShell ) )
    {
    }

    //--------------------------------------------------------------------
    ScriptDocument::ScriptDocument( const Reference< XModel >& _rxDocument )
        :m_pImpl( new ScriptDocument_Impl( _rxDocument ) )
    {
        OSL_ENSURE( _rxDocument.is(), "ScriptDocument_Impl::ScriptDocument: document must not be NULL!" );
            // a NULL document results in an uninitialized instance, and for this
            // purpose, there is a dedicated constructor
    }

    //--------------------------------------------------------------------
    ScriptDocument::ScriptDocument( const ScriptDocument& _rSource )
        :m_pImpl( _rSource.m_pImpl )
    {
    }

    //--------------------------------------------------------------------
    ScriptDocument::~ScriptDocument()
    {
    }

    //--------------------------------------------------------------------
    const ScriptDocument& ScriptDocument::getApplicationScriptDocument()
    {
        static ScriptDocument s_aApplicationScripts;
        return s_aApplicationScripts;
    }

    //--------------------------------------------------------------------
    ScriptDocument ScriptDocument::getDocumentForBasicManager( const BasicManager* _pManager )
    {
        if ( _pManager == SFX_APP()->GetBasicManager() )
            return getApplicationScriptDocument();

        SfxObjectShell* pDocShell = SfxObjectShell::GetFirst();
        while ( pDocShell )
        {
            if  (   ( pDocShell->GetBasicManager() != SFX_APP()->GetBasicManager() )
                &&  ( pDocShell->GetBasicManager() == _pManager )
                )
            {
                return ScriptDocument( *pDocShell );
            }
            pDocShell = SfxObjectShell::GetNext( *pDocShell );
        }

        OSL_ENSURE( false, "ScriptDocument::getDocumentForBasicManager: did not find a document for this manager!" );
        return ScriptDocument( NoDocument );
    }

    //--------------------------------------------------------------------
    ScriptDocument ScriptDocument::getDocumentWithCaption( const ::rtl::OUString& _rCaption )
    {
        ScriptDocument aDocument( getApplicationScriptDocument() );
        if ( _rCaption.getLength() != 0 )
        {
            SfxViewFrame* pView = SfxViewFrame::GetFirst();
            while ( pView )
            {
                SfxObjectShell* pObjShell = pView->GetObjectShell();
                if ( pObjShell )
                {
                    SfxMedium* pMedium = pObjShell->GetMedium();
                    if  (   (   pMedium
                            &&  _rCaption == pMedium->GetURLObject().GetMainURL( INetURLObject::NO_DECODE )
                            )
                        ||  ( _rCaption == ::rtl::OUString( pObjShell->GetTitle( SFX_TITLE_CAPTION ) ) )
                        )
                    {
                        aDocument = ScriptDocument( *pObjShell );
                        break;
                    }
                }
                pView = SfxViewFrame::GetNext( *pView );
            }
        }
        return aDocument;
    }

    //--------------------------------------------------------------------
    ScriptDocuments ScriptDocument::getAllScriptDocuments( bool _bIncludingApplication )
    {
        ScriptDocuments aDocuments;
        if ( _bIncludingApplication )
            aDocuments.push_back( getApplicationScriptDocument() );

        SfxObjectShell* pDocShell = SfxObjectShell::GetFirst();
        while ( pDocShell )
        {
            // exclude invisible docs, and docs without own scripts/dialogs
            if  (   SfxViewFrame::GetFirst( pDocShell )
                &&  pDocShell->GetBasicManager() != SFX_APP()->GetBasicManager()
                )
            {
                ScriptDocument aDoc( *pDocShell );
                OSL_ENSURE( aDoc.isValid(),
                    "ScriptDocument::getAllScriptDocuments: an SfxObjectShell with own BasicManager, but without library containers?" );
                if ( aDoc.isValid() )
                    aDocuments.push_back( aDoc );
            }

            pDocShell = SfxObjectShell::GetNext( *pDocShell );
        }

        return aDocuments;
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::operator==( const ScriptDocument& _rhs ) const
    {
        return m_pImpl->getObjectShell() == _rhs.m_pImpl->getObjectShell();
    }

    //--------------------------------------------------------------------
    sal_Int32 ScriptDocument::hashCode() const
    {
        return reinterpret_cast< sal_Int32 >( m_pImpl->getObjectShell() );
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::isValid() const
    {
        return m_pImpl->isValid();
    }

    //--------------------------------------------------------------------
    Reference< XLibraryContainer > ScriptDocument::getLibraryContainer( LibraryContainerType _eType ) const
    {
        return m_pImpl->getLibraryContainer( _eType );
    }

    //--------------------------------------------------------------------
    Reference< XNameContainer > ScriptDocument::getLibrary( LibraryContainerType _eType, const ::rtl::OUString& _rLibName, bool _bLoadLibrary ) const
        SAL_THROW((NoSuchElementException))
    {
        return m_pImpl->getLibrary( _eType, _rLibName, _bLoadLibrary );
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::hasLibrary( LibraryContainerType _eType, const ::rtl::OUString& _rLibName ) const
    {
        return m_pImpl->hasLibrary( _eType, _rLibName );
    }

    //--------------------------------------------------------------------
    Reference< XNameContainer > ScriptDocument::getOrCreateLibrary( LibraryContainerType _eType, const ::rtl::OUString& _rLibName ) const
    {
        return m_pImpl->getOrCreateLibrary( _eType, _rLibName );
    }

    //--------------------------------------------------------------------
    void ScriptDocument::loadLibraryIfExists( LibraryContainerType _eType, const ::rtl::OUString& _rLibrary )
    {
        m_pImpl->loadLibraryIfExists( _eType, _rLibrary );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > ScriptDocument::getObjectNames( LibraryContainerType _eType, const ::rtl::OUString& _rLibName ) const
    {
        Sequence< ::rtl::OUString > aModuleNames;

        try
        {
            Reference< XNameContainer > xLib( getLibrary( _eType, _rLibName, false ) );
            if ( xLib.is() )
                aModuleNames = xLib->getElementNames();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        // sort
        ::std::sort( aModuleNames.getArray() , aModuleNames.getArray() + aModuleNames.getLength() , StringCompareLessThan );

        return aModuleNames;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString ScriptDocument::createObjectName( LibraryContainerType _eType, const ::rtl::OUString& _rLibName ) const
    {
        ::rtl::OUString aObjectName;

        ::rtl::OUString aBaseName = _eType == E_SCRIPTS
            ?   ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Module" ) )
            :   ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Dialog" ) );

        Sequence< ::rtl::OUString > aUsedNames( getObjectNames( _eType, _rLibName ) );
        ::std::set< ::rtl::OUString > aUsedNamesCheck;
        ::std::copy( aUsedNames.getConstArray(), aUsedNames.getConstArray() + aUsedNames.getLength(),
            ::std::insert_iterator< ::std::set< ::rtl::OUString > >( aUsedNamesCheck, aUsedNamesCheck.begin() ) );

        bool bValid = false;
        USHORT i = 1;
        while ( !bValid )
        {
            aObjectName = aBaseName;
            aObjectName += String::CreateFromInt32( i );

            if ( aUsedNamesCheck.find( aObjectName ) == aUsedNamesCheck.end() )
                bValid = TRUE;

            ++i;
        }

        return aObjectName;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > ScriptDocument::getLibraryNames() const
    {
        return BasicIDE::GetMergedLibraryNames( getLibraryContainer( E_SCRIPTS ), getLibraryContainer( E_DIALOGS ) );
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::isReadOnly() const
    {
        return m_pImpl->isReadOnly();
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::isApplication() const
    {
        return m_pImpl->isApplication();
    }

    //--------------------------------------------------------------------
    BasicManager* ScriptDocument::getBasicManager() const
    {
        return m_pImpl->getBasicManager();
    }

    //--------------------------------------------------------------------
    Reference< XModel > ScriptDocument::getDocument() const
    {
        return m_pImpl->getDocument();
    }

    //--------------------------------------------------------------------
    Reference< XModel > ScriptDocument::getDocumentOrNull() const
    {
        if ( isDocument() )
            return m_pImpl->getDocument();
        return NULL;
    }

    //--------------------------------------------------------------------
    void ScriptDocument::LEGACY_startDocumentListening( SfxListener& _rListener ) const
    {
        OSL_ENSURE( isValid(), "ScriptDocument::LEGACY_startDocumentListening: invalid, this will crash!" );
        OSL_ENSURE( !isApplication(), "ScriptDocument::LEGACY_startDocumentListening: not allowed for the application!" );
        if ( isValid() && !isApplication() )
            _rListener.StartListening( *m_pImpl->getObjectShell(), TRUE );
    }

    //--------------------------------------------------------------------
    void ScriptDocument::LEGACY_resetWorkingDocument()
    {
        SfxObjectShell::SetWorkingDocument( NULL );
    }

    //--------------------------------------------------------------------
    void ScriptDocument::LEGACY_setWorkingDocument( const ScriptDocument& _rDocument )
    {
        if ( _rDocument.isApplication() )
            LEGACY_resetWorkingDocument();
        else
            SfxObjectShell::SetWorkingDocument( _rDocument.m_pImpl->getObjectShell() );
    }

    //--------------------------------------------------------------------
    ScriptDocument ScriptDocument::LEGACY_getWorkingDocument()
    {
        SfxObjectShell* pWorkingDocument( SfxObjectShell::GetWorkingDocument() );
        if ( pWorkingDocument )
            return ScriptDocument( *pWorkingDocument );
        return getApplicationScriptDocument();
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::removeModule( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rModuleName ) const
    {
        return m_pImpl->removeModuleOrDialog( E_SCRIPTS, _rLibName, _rModuleName );
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::hasModule( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rModuleName ) const
    {
        return m_pImpl->hasModuleOrDialog( E_SCRIPTS, _rLibName, _rModuleName );
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::getModule( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rModName, ::rtl::OUString& _out_rModuleSource ) const
    {
        Any aCode;
        if ( !m_pImpl->getModuleOrDialog( E_SCRIPTS, _rLibName, _rModName, aCode ) )
            return false;
        OSL_VERIFY( aCode >>= _out_rModuleSource );
        return true;
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::renameModule( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rOldName, const ::rtl::OUString& _rNewName ) const
    {
        return m_pImpl->renameModuleOrDialog( E_SCRIPTS, _rLibName, _rOldName, _rNewName, NULL );
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::createModule( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rModName, bool _bCreateMain, ::rtl::OUString& _out_rNewModuleCode ) const
    {
        if ( !m_pImpl->createModule( _rLibName, _rModName, _bCreateMain, _out_rNewModuleCode ) )
            return false;

        // doc shell modified
        BasicIDE::MarkDocumentModified( *const_cast< ScriptDocument* >( this ) );    // here?
        return true;
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::insertModule( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rModName, const ::rtl::OUString& _rModuleCode ) const
    {
        return m_pImpl->insertModuleOrDialog( E_SCRIPTS, _rLibName, _rModName, makeAny( _rModuleCode ) );
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::updateModule( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rModName, const ::rtl::OUString& _rModuleCode ) const
    {
        return m_pImpl->updateModule( _rLibName, _rModName, _rModuleCode );
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::removeDialog( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rDialogName ) const
    {
        return m_pImpl->removeModuleOrDialog( E_DIALOGS, _rLibName, _rDialogName );
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::hasDialog( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rDialogName ) const
    {
        return m_pImpl->hasModuleOrDialog( E_DIALOGS, _rLibName, _rDialogName );
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::getDialog( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rDialogName, Reference< XInputStreamProvider >& _out_rDialogProvider ) const
    {
        Any aCode;
        if ( !m_pImpl->getModuleOrDialog( E_DIALOGS, _rLibName, _rDialogName, aCode ) )
            return false;
        OSL_VERIFY( aCode >>= _out_rDialogProvider );
        return _out_rDialogProvider.is();
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::renameDialog( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rOldName, const ::rtl::OUString& _rNewName, const Reference< XNameContainer >& _rxExistingDialogModel ) const
    {
        return m_pImpl->renameModuleOrDialog( E_DIALOGS, _rLibName, _rOldName, _rNewName, _rxExistingDialogModel );
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::createDialog( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rDialogName, Reference< XInputStreamProvider >& _out_rDialogProvider ) const
    {
        if ( !m_pImpl->createDialog( _rLibName, _rDialogName, _out_rDialogProvider ) )
            return false;

        BasicIDE::MarkDocumentModified( *const_cast< ScriptDocument* >( this ) );    // here?
        return true;
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::insertDialog( const ::rtl::OUString& _rLibName, const ::rtl::OUString& _rDialogName, const Reference< XInputStreamProvider >& _rxDialogProvider ) const
    {
        return m_pImpl->insertModuleOrDialog( E_DIALOGS, _rLibName, _rDialogName, makeAny( _rxDialogProvider ) );
    }

    //--------------------------------------------------------------------
    void ScriptDocument::setDocumentModified() const
    {
        m_pImpl->setDocumentModified();
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::isDocumentModified() const
    {
        return m_pImpl->isDocumentModified();
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::saveDocument( const Reference< XStatusIndicator >& _rxStatusIndicator ) const
    {
        return m_pImpl->saveDocument( _rxStatusIndicator );
    }

    //--------------------------------------------------------------------
    LibraryLocation ScriptDocument::getLibraryLocation( const ::rtl::OUString& _rLibName ) const
    {
        LibraryLocation eLocation = LIBRARY_LOCATION_UNKNOWN;
        if ( _rLibName.getLength() )
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

    //--------------------------------------------------------------------
    ::rtl::OUString ScriptDocument::getTitle( LibraryLocation _eLocation, LibraryType _eType ) const
    {
        ::rtl::OUString aTitle;

        switch ( _eLocation )
        {
            case LIBRARY_LOCATION_USER:
            {
                switch ( _eType )
                {
                case LIBRARY_TYPE_MODULE:   aTitle = String( IDEResId( RID_STR_USERMACROS ) ); break;
                case LIBRARY_TYPE_DIALOG:   aTitle = String( IDEResId( RID_STR_USERDIALOGS ) ); break;
                case LIBRARY_TYPE_ALL:      aTitle = String( IDEResId( RID_STR_USERMACROSDIALOGS ) ); break;
                default:
                    break;
            }
            break;
            case LIBRARY_LOCATION_SHARE:
            {
                switch ( _eType )
                {
                case LIBRARY_TYPE_MODULE:   aTitle = String( IDEResId( RID_STR_SHAREMACROS ) ); break;
                case LIBRARY_TYPE_DIALOG:   aTitle = String( IDEResId( RID_STR_SHAREDIALOGS ) ); break;
                case LIBRARY_TYPE_ALL:      aTitle = String( IDEResId( RID_STR_SHAREMACROSDIALOGS ) ); break;
                default:
                    break;
                }
            }
            break;
            case LIBRARY_LOCATION_DOCUMENT:
            {
                if ( isDocument() )
                    aTitle = m_pImpl->getObjectShell()->GetTitle( SFX_TITLE_CAPTION );
            }
            break;
            default:
                break;
            }
        }

        return aTitle;
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::isClosing() const
    {
        OSL_ENSURE( isDocument(), "ScriptDocument::isClosing: for documents only!" );
        if ( isDocument() )
            return m_pImpl->getObjectShell()->IsInPrepareClose();
        return false;
    }

    //--------------------------------------------------------------------
    bool ScriptDocument::isActive() const
    {
        OSL_ENSURE( isDocument(), "ScriptDocument::isActive: for documents only!" );
        if ( isDocument() )
            return m_pImpl->getObjectShell() == SfxObjectShell::Current();
        return false;
    }

    //--------------------------------------------------------------------
    void ScriptDocument::signScriptingContent() const
    {
        OSL_ENSURE( isDocument(), "ScriptDocument::signScriptingContent: for documents only!" );
        if ( isDocument() )
            m_pImpl->getObjectShell()->SignScriptingContent();
    }

    //--------------------------------------------------------------------
    sal_uInt16 ScriptDocument::getScriptingSignatureState() const
    {
        OSL_ENSURE( isDocument(), "ScriptDocument::getScriptingSignatureState: for documents only!" );
        if ( isDocument() )
            return m_pImpl->getObjectShell()->GetScriptingSignatureState();
        return SIGNATURESTATE_NOSIGNATURES;
    }

    //--------------------------------------------------------------------
    void ScriptDocument::adjustMacroMode( const ::rtl::OUString& _rScriptType ) const
    {
        OSL_ENSURE( isDocument(), "ScriptDocument::adjustMacroMode: for documents only!" );
        if ( isDocument() )
            m_pImpl->getObjectShell()->AdjustMacroMode( _rScriptType );
    }

    //--------------------------------------------------------------------
    sal_Int16 ScriptDocument::getMacroMode() const
    {
        OSL_ENSURE( isDocument(), "ScriptDocument::getMacroMode: for documents only!" );
        if ( isDocument() )
            return m_pImpl->getObjectShell()->GetMacroMode();
        return MacroExecMode::NEVER_EXECUTE;
    }

//........................................................................
} // namespace basctl
//........................................................................
