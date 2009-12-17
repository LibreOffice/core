/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: docmacromode.cxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "sfx2/docmacromode.hxx"
#include "sfx2/signaturestate.hxx"
#include "sfx2/docfile.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/task/ErrorCodeRequest.hpp>
#include <com/sun/star/task/DocumentMacroConfirmationRequest.hpp>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>
#include <framework/interaction.hxx>
#include <osl/file.hxx>
#include <rtl/ref.hxx>
#include <unotools/securityoptions.hxx>
#include <svtools/sfxecode.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>

//........................................................................
namespace sfx2
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::task::XInteractionHandler;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::task::XInteractionHandler;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::task::XInteractionContinuation;
    using ::com::sun::star::task::XInteractionRequest;
    using ::com::sun::star::task::DocumentMacroConfirmationRequest;
    using ::com::sun::star::task::ErrorCodeRequest;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::security::XDocumentDigitalSignatures;
    using ::com::sun::star::security::DocumentSignatureInformation;
    using ::com::sun::star::embed::XStorage;
    using ::com::sun::star::task::InteractionClassification_QUERY;
    using ::com::sun::star::document::XEmbeddedScripts;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::script::XLibraryContainer;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    /** === end UNO using === **/
    namespace MacroExecMode = ::com::sun::star::document::MacroExecMode;

    //====================================================================
    //= DocumentMacroMode_Data
    //====================================================================
    struct DocumentMacroMode_Data
    {
        IMacroDocumentAccess&       m_rDocumentAccess;
        sal_Bool                    m_bMacroDisabledMessageShown;
        sal_Bool                    m_bDocMacroDisabledMessageShown;

        DocumentMacroMode_Data( IMacroDocumentAccess& rDocumentAccess )
            :m_rDocumentAccess( rDocumentAccess )
            ,m_bMacroDisabledMessageShown( sal_False )
            ,m_bDocMacroDisabledMessageShown( sal_False )
        {
        }
    };

    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        //................................................................
        void lcl_showGeneralSfxErrorOnce( const Reference< XInteractionHandler >& rxHandler, const sal_Int32 nSfxErrorCode, sal_Bool& rbAlreadyShown )
        {
            if ( rbAlreadyShown )
                return;

            ErrorCodeRequest aErrorCodeRequest;
            aErrorCodeRequest.ErrCode = nSfxErrorCode;

            SfxMedium::CallApproveHandler( rxHandler, makeAny( aErrorCodeRequest ), sal_False );
            rbAlreadyShown = sal_True;
        }

        //................................................................
        void lcl_showMacrosDisabledError( const Reference< XInteractionHandler >& rxHandler, sal_Bool& rbAlreadyShown )
        {
            lcl_showGeneralSfxErrorOnce( rxHandler, ERRCODE_SFX_MACROS_SUPPORT_DISABLED, rbAlreadyShown );
        }

        //................................................................
        void lcl_showDocumentMacrosDisabledError( const Reference< XInteractionHandler >& rxHandler, sal_Bool& rbAlreadyShown )
        {
            lcl_showGeneralSfxErrorOnce( rxHandler, ERRCODE_SFX_DOCUMENT_MACRO_DISABLED, rbAlreadyShown );
        }

        //................................................................
        sal_Bool lcl_showMacroWarning( const Reference< XInteractionHandler >& rxHandler,
            const ::rtl::OUString& rDocumentLocation )
        {
            DocumentMacroConfirmationRequest aRequest;
            aRequest.DocumentURL = rDocumentLocation;
            return SfxMedium::CallApproveHandler( rxHandler, makeAny( aRequest ), sal_True );
        }
    }

    //====================================================================
    //= DocumentMacroMode
    //====================================================================
    //--------------------------------------------------------------------
    DocumentMacroMode::DocumentMacroMode( IMacroDocumentAccess& rDocumentAccess )
        :m_pData( new DocumentMacroMode_Data( rDocumentAccess ) )
    {
    }

    //--------------------------------------------------------------------
    DocumentMacroMode::~DocumentMacroMode()
    {
    }

    //--------------------------------------------------------------------
    sal_Bool DocumentMacroMode::allowMacroExecution()
    {
        m_pData->m_rDocumentAccess.setCurrentMacroExecMode( MacroExecMode::ALWAYS_EXECUTE_NO_WARN );
        return sal_True;
    }

    //--------------------------------------------------------------------
    sal_Bool DocumentMacroMode::disallowMacroExecution()
    {
        m_pData->m_rDocumentAccess.setCurrentMacroExecMode( MacroExecMode::NEVER_EXECUTE );
        return sal_False;
    }

    //--------------------------------------------------------------------
    sal_Bool DocumentMacroMode::adjustMacroMode( const Reference< XInteractionHandler >& rxInteraction )
    {
        sal_uInt16 nMacroExecutionMode = m_pData->m_rDocumentAccess.getCurrentMacroExecMode();

        if ( SvtSecurityOptions().IsMacroDisabled() )
        {
            // no macro should be executed at all
            lcl_showMacrosDisabledError( rxInteraction, m_pData->m_bMacroDisabledMessageShown );
            return disallowMacroExecution();
        }

        // get setting from configuration if required
        enum AutoConfirmation
        {
            eNoAutoConfirm,
            eAutoConfirmApprove,
            eAutoConfirmReject
        };
        AutoConfirmation eAutoConfirm( eNoAutoConfirm );

        if  (   ( nMacroExecutionMode == MacroExecMode::USE_CONFIG )
            ||  ( nMacroExecutionMode == MacroExecMode::USE_CONFIG_REJECT_CONFIRMATION )
            ||  ( nMacroExecutionMode == MacroExecMode::USE_CONFIG_APPROVE_CONFIRMATION )
            )
        {
            SvtSecurityOptions aOpt;
            switch ( aOpt.GetMacroSecurityLevel() )
            {
                case 3:
                    nMacroExecutionMode = MacroExecMode::FROM_LIST_NO_WARN;
                    break;
                case 2:
                    nMacroExecutionMode = MacroExecMode::FROM_LIST_AND_SIGNED_WARN;
                    break;
                case 1:
                    nMacroExecutionMode = MacroExecMode::ALWAYS_EXECUTE;
                    break;
                case 0:
                    nMacroExecutionMode = MacroExecMode::ALWAYS_EXECUTE_NO_WARN;
                    break;
                default:
                    OSL_ENSURE( sal_False, "DocumentMacroMode::adjustMacroMode: unexpected macro security level!" );
                    nMacroExecutionMode = MacroExecMode::NEVER_EXECUTE;
            }

            if ( nMacroExecutionMode == MacroExecMode::USE_CONFIG_REJECT_CONFIRMATION )
                eAutoConfirm = eAutoConfirmReject;
            else if ( nMacroExecutionMode == MacroExecMode::USE_CONFIG_APPROVE_CONFIRMATION )
                eAutoConfirm = eAutoConfirmApprove;
        }

        if ( nMacroExecutionMode == MacroExecMode::NEVER_EXECUTE )
            return sal_False;

        if ( nMacroExecutionMode == MacroExecMode::ALWAYS_EXECUTE_NO_WARN )
            return sal_True;

        try
        {
            ::rtl::OUString sReferrer( m_pData->m_rDocumentAccess.getDocumentLocation() );

            // get document location from medium name and check whether it is a trusted one
            // the service is created ohne document version, since it is not of interest here
            ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
            Reference< XDocumentDigitalSignatures > xSignatures;
            if ( aContext.createComponent( "com.sun.star.security.DocumentDigitalSignatures", xSignatures ) )
            {
                INetURLObject aURLReferer( sReferrer );

                ::rtl::OUString aLocation;
                if ( aURLReferer.removeSegment() )
                    aLocation = aURLReferer.GetMainURL( INetURLObject::NO_DECODE );

                if ( aLocation.getLength() && xSignatures->isLocationTrusted( aLocation ) )
                {
                    return allowMacroExecution();
                }
            }

            // at this point it is clear that the document is not in the secure location
            if ( nMacroExecutionMode == MacroExecMode::FROM_LIST_NO_WARN )
            {
                lcl_showDocumentMacrosDisabledError( rxInteraction, m_pData->m_bDocMacroDisabledMessageShown );
                return disallowMacroExecution();
            }

            // check whether the document is signed with trusted certificate
            if ( nMacroExecutionMode != MacroExecMode::FROM_LIST )
            {
                // the trusted macro check will also retrieve the signature state ( small optimization )
                sal_Bool bHasTrustedMacroSignature = m_pData->m_rDocumentAccess.hasTrustedScriptingSignature( nMacroExecutionMode != MacroExecMode::FROM_LIST_AND_SIGNED_NO_WARN );

                sal_uInt16 nSignatureState = m_pData->m_rDocumentAccess.getScriptingSignatureState();
                if ( nSignatureState == SIGNATURESTATE_SIGNATURES_BROKEN )
                {
                    // the signature is broken, no macro execution
                    if ( nMacroExecutionMode != MacroExecMode::FROM_LIST_AND_SIGNED_NO_WARN )
                        m_pData->m_rDocumentAccess.showBrokenSignatureWarning( rxInteraction );

                    return disallowMacroExecution();
                }
                else if ( bHasTrustedMacroSignature )
                {
                    // there is trusted macro signature, allow macro execution
                    return allowMacroExecution();
                }
                else if ( nSignatureState == SIGNATURESTATE_SIGNATURES_OK
                       || nSignatureState == SIGNATURESTATE_SIGNATURES_NOTVALIDATED )
                {
                    // there is valid signature, but it is not from the trusted author
                    return disallowMacroExecution();
                }
            }

            // at this point it is clear that the document is neither in secure location nor signed with trusted certificate
            if  (   ( nMacroExecutionMode == MacroExecMode::FROM_LIST_AND_SIGNED_NO_WARN )
                ||  ( nMacroExecutionMode == MacroExecMode::FROM_LIST_AND_SIGNED_WARN )
                )
            {
                if  ( nMacroExecutionMode == MacroExecMode::FROM_LIST_AND_SIGNED_WARN )
                    lcl_showDocumentMacrosDisabledError( rxInteraction, m_pData->m_bDocMacroDisabledMessageShown );

                return disallowMacroExecution();
            }
        }
        catch ( Exception& )
        {
            if  (   ( nMacroExecutionMode == MacroExecMode::FROM_LIST_NO_WARN )
                ||  ( nMacroExecutionMode == MacroExecMode::FROM_LIST_AND_SIGNED_WARN )
                ||  ( nMacroExecutionMode == MacroExecMode::FROM_LIST_AND_SIGNED_NO_WARN )
                )
            {
                return disallowMacroExecution();
            }
        }

        // conformation is required
        sal_Bool bSecure = sal_False;

        if ( eAutoConfirm == eNoAutoConfirm )
        {
            ::rtl::OUString sReferrer( m_pData->m_rDocumentAccess.getDocumentLocation() );

            ::rtl::OUString aSystemFileURL;
            if ( osl::FileBase::getSystemPathFromFileURL( sReferrer, aSystemFileURL ) == osl::FileBase::E_None )
                sReferrer = aSystemFileURL;

            bSecure = lcl_showMacroWarning( rxInteraction, sReferrer );
        }
        else
            bSecure = ( eAutoConfirm == eAutoConfirmApprove );

        return ( bSecure ? allowMacroExecution() : disallowMacroExecution() );
    }

    //--------------------------------------------------------------------
    sal_Bool DocumentMacroMode::isMacroExecutionDisallowed() const
    {
        return m_pData->m_rDocumentAccess.getCurrentMacroExecMode() == MacroExecMode::NEVER_EXECUTE;
    }

    //--------------------------------------------------------------------
    sal_Bool DocumentMacroMode::hasMacroLibrary() const
    {
        sal_Bool bHasMacroLib = sal_False;
        try
        {
            Reference< XEmbeddedScripts > xScripts( m_pData->m_rDocumentAccess.getEmbeddedDocumentScripts() );
            Reference< XLibraryContainer > xContainer;
            if ( xScripts.is() )
                xContainer.set( xScripts->getBasicLibraries(), UNO_QUERY_THROW );

            if ( xContainer.is() )
            {
                // a library container exists; check if it's empty

                // if there are libraries except the "Standard" library
                // we assume that they are not empty (because they have been created by the user)
                if ( !xContainer->hasElements() )
                    bHasMacroLib = sal_False;
                else
                {
                    ::rtl::OUString aStdLibName( RTL_CONSTASCII_USTRINGPARAM( "Standard" ) );
                    Sequence< ::rtl::OUString > aElements = xContainer->getElementNames();
                    if ( aElements.getLength() )
                    {
                        if ( aElements.getLength() > 1 || !aElements[0].equals( aStdLibName ) )
                            bHasMacroLib = sal_True;
                        else
                        {
                            // usually a "Standard" library is always present (design)
                            // for this reason we must check if it's empty
                            //
                            // Note: Since #i73229#, this is not true anymore. There's no default
                            // "Standard" lib anymore. Wouldn't it be time to get completely
                            // rid of the "Standard" thingie - this shouldn't be necessary
                            // anymore, should it?
                            // 2007-01-25 / frank.schoenheit@sun.com
                            Reference < XNameAccess > xLib;
                            Any aAny = xContainer->getByName( aStdLibName );
                            aAny >>= xLib;
                            if ( xLib.is() )
                                bHasMacroLib = xLib->hasElements();
                        }
                    }
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return bHasMacroLib;
    }

    //--------------------------------------------------------------------
    sal_Bool DocumentMacroMode::storageHasMacros( const Reference< XStorage >& rxStorage )
    {
        sal_Bool bHasMacros = sal_False;
        if ( rxStorage.is() )
        {
            try
            {
                static const ::rtl::OUString s_sBasicStorageName( ::rtl::OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "Basic" ) ) );
                static const ::rtl::OUString s_sScriptsStorageName( ::rtl::OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "Scripts" ) ) );

                bHasMacros =(   (   rxStorage->hasByName( s_sBasicStorageName )
                                &&  rxStorage->isStorageElement( s_sBasicStorageName )
                                )
                            ||  (   rxStorage->hasByName( s_sScriptsStorageName )
                                &&  rxStorage->isStorageElement( s_sScriptsStorageName )
                                )
                            );
            }
            catch ( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return bHasMacros;
    }

    //--------------------------------------------------------------------
    sal_Bool DocumentMacroMode::checkMacrosOnLoading( const Reference< XInteractionHandler >& rxInteraction )
    {
        sal_Bool bAllow = sal_False;
        if ( SvtSecurityOptions().IsMacroDisabled() )
        {
            // no macro should be executed at all
            bAllow = disallowMacroExecution();
        }
        else
        {
            if ( m_pData->m_rDocumentAccess.documentStorageHasMacros() || hasMacroLibrary() )
            {
                bAllow = adjustMacroMode( rxInteraction );
            }
            else if ( !isMacroExecutionDisallowed() )
            {
                // if macros will be added by the user later, the security check is obsolete
                bAllow = allowMacroExecution();
            }
        }
        return bAllow;
    }

//........................................................................
} // namespace sfx2
//........................................................................
