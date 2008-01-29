/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docmacromode.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:28:54 $
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
#include "precompiled_sfx2.hxx"

#include "sfx2/docmacromode.hxx"
#include "sfx2/signaturestate.hxx"

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
#include <svtools/securityoptions.hxx>
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
        IMacroDocumentAccess&   rDocumentAccess;
        sal_Bool                    bMacroDisabledMessageShown;
        sal_Bool                    bDocMacroDisabledMessageShown;

        DocumentMacroMode_Data( IMacroDocumentAccess& _rDocumentAccess )
            :rDocumentAccess( _rDocumentAccess )
            ,bMacroDisabledMessageShown( sal_False )
            ,bDocMacroDisabledMessageShown( sal_False )
        {
        }
    };

    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        //................................................................
        /** calls the given interaction handler with the given interaction request, offering
            the two continuations "Approve" and "Abort"

            @return
                <TRUE/> if and only if the given handler handled the the request, and the "Approve"
                continuation was selected.
        */
        sal_Bool lcl_callInterActionHandler( const Reference< XInteractionHandler >& _rxHandler, const Any& _rRequest )
        {
            if ( !_rxHandler.is() )
                return sal_False;

            try
            {
                Sequence< Reference< XInteractionContinuation > > aContinuations(2);

                ::rtl::Reference< ::framework::ContinuationAbort > pAbort( new ::framework::ContinuationAbort() );
                aContinuations[ 0 ] = pAbort.get();

                ::rtl::Reference< ::framework::ContinuationApprove > pApprove( new ::framework::ContinuationApprove() );
                aContinuations[ 1 ] = pApprove.get();

                Reference< XInteractionRequest > xRequest( new ::framework::InteractionRequest( _rRequest, aContinuations ) );
                _rxHandler->handle( xRequest );

                if ( pApprove->isSelected() )
                    return sal_True;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return sal_False;
        }

        //................................................................
        void lcl_showGeneralSfxErrorOnce( const Reference< XInteractionHandler >& _rxHandler, const sal_Int32 _nSfxErrorCode, sal_Bool& _rbAlreadyShown )
        {
            if ( _rbAlreadyShown )
                return;

            ErrorCodeRequest aErrorCodeRequest;
            aErrorCodeRequest.ErrCode = _nSfxErrorCode;

            _rbAlreadyShown = lcl_callInterActionHandler( _rxHandler, makeAny( aErrorCodeRequest ) );
        }

        //................................................................
        void lcl_showMacrosDisabledError( const Reference< XInteractionHandler >& _rxHandler, sal_Bool& _rbAlreadyShown )
        {
            lcl_showGeneralSfxErrorOnce( _rxHandler, ERRCODE_SFX_MACROS_SUPPORT_DISABLED, _rbAlreadyShown );
        }

        //................................................................
        void lcl_showDocumentMacrosDisabledError( const Reference< XInteractionHandler >& _rxHandler, sal_Bool& _rbAlreadyShown )
        {
            lcl_showGeneralSfxErrorOnce( _rxHandler, ERRCODE_SFX_DOCUMENT_MACRO_DISABLED, _rbAlreadyShown );
        }

        //................................................................
        sal_Bool lcl_showMacroWarning( const Reference< XInteractionHandler >& _rxHandler,
            const ::rtl::OUString& _rDocumentLocation )
        {
            DocumentMacroConfirmationRequest aRequest;
            aRequest.DocumentURL = _rDocumentLocation;
            return lcl_callInterActionHandler( _rxHandler, makeAny( aRequest ) );
        }

        //................................................................
        sal_Bool lcl_showMacroWarning( const Reference< XInteractionHandler >& _rxHandler,
            const ::rtl::OUString& _rDocumentLocation, const Reference< XStorage >& _rxDocStor,
            const Sequence< DocumentSignatureInformation >& _rDocSigInfo )
        {
            DocumentMacroConfirmationRequest aRequest;
            aRequest.DocumentURL = _rDocumentLocation;
            aRequest.DocumentStorage = _rxDocStor;
            aRequest.DocumentSignatureInformation = _rDocSigInfo;
            aRequest.Classification = InteractionClassification_QUERY;
            return lcl_callInterActionHandler( _rxHandler, makeAny( aRequest ) );
        }
    }

    //====================================================================
    //= DocumentMacroMode
    //====================================================================
    //--------------------------------------------------------------------
    DocumentMacroMode::DocumentMacroMode( IMacroDocumentAccess& _rDocumentAccess )
        :m_pData( new DocumentMacroMode_Data( _rDocumentAccess ) )
    {
    }

    //--------------------------------------------------------------------
    DocumentMacroMode::~DocumentMacroMode()
    {
    }

    //--------------------------------------------------------------------
    sal_Bool DocumentMacroMode::allowMacroExecution()
    {
        m_pData->rDocumentAccess.setImposedMacroExecMode( MacroExecMode::ALWAYS_EXECUTE_NO_WARN );
        return sal_True;
    }

    //--------------------------------------------------------------------
    sal_Bool DocumentMacroMode::disallowMacroExecution()
    {
        m_pData->rDocumentAccess.setImposedMacroExecMode( MacroExecMode::NEVER_EXECUTE );
        return sal_False;
    }

    //--------------------------------------------------------------------
    sal_Bool DocumentMacroMode::adjustMacroMode( const Reference< XInteractionHandler >& _rxInteraction )
    {
        sal_uInt16 nMacroExecutionMode = m_pData->rDocumentAccess.getImposedMacroExecMode();

        if ( SvtSecurityOptions().IsMacroDisabled() )
        {
            // no macro should be executed at all
            lcl_showMacrosDisabledError( _rxInteraction, m_pData->bMacroDisabledMessageShown );
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
            ::rtl::OUString sReferrer( m_pData->rDocumentAccess.getDocumentLocation() );

            // get document location from medium name and check whether it is a trusted one
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
                lcl_showDocumentMacrosDisabledError( _rxInteraction, m_pData->bDocMacroDisabledMessageShown );
                return disallowMacroExecution();
            }

            // check whether the document is signed with trusted certificate
            if ( xSignatures.is() && nMacroExecutionMode != MacroExecMode::FROM_LIST )
            {
                Sequence< DocumentSignatureInformation > aScriptingSignatureInformations;
                Reference < XStorage > xStore( m_pData->rDocumentAccess.getLastCommitDocumentStorage() );

                sal_uInt16 nSignatureState = m_pData->rDocumentAccess.getScriptingSignatureState();
                if ( nSignatureState == SIGNATURESTATE_SIGNATURES_BROKEN )
                {
                    if ( nMacroExecutionMode != MacroExecMode::FROM_LIST_AND_SIGNED_NO_WARN )
                    {
                        m_pData->rDocumentAccess.showBrokenSignatureWarning( _rxInteraction );
                        return disallowMacroExecution();
                    }
                }
                else
                {
                    if  (   (   ( nSignatureState == SIGNATURESTATE_SIGNATURES_OK )
                            ||  ( nSignatureState == SIGNATURESTATE_SIGNATURES_NOTVALIDATED )
                            )
                        &&  xStore.is()
                        )
                    {
                        aScriptingSignatureInformations =
                            xSignatures->verifyScriptingContentSignatures( xStore, NULL );
                    }
                }

                sal_Int32 nNumOfInfos = aScriptingSignatureInformations.getLength();

                // from now on sReferrer is the system file path
                // sReferrer = INetURLObject::decode( sReferrer, '%', INetURLObject::DECODE_WITH_CHARSET );
                ::rtl::OUString aSystemFileURL;
                if ( osl::FileBase::getSystemPathFromFileURL( sReferrer, aSystemFileURL ) == osl::FileBase::E_None )
                    sReferrer = aSystemFileURL;

                if ( nNumOfInfos )
                {
                    for ( sal_Int32 i = 0; i < nNumOfInfos; i++ )
                        if ( xSignatures->isAuthorTrusted( aScriptingSignatureInformations[i].Signer ) )
                        {
                            // there's at least one author we trust which signed the doc
                            return allowMacroExecution();
                        }

                    if ( nMacroExecutionMode != MacroExecMode::FROM_LIST_AND_SIGNED_NO_WARN )
                    {
                        sal_Bool bApproved = lcl_showMacroWarning( _rxInteraction,
                            sReferrer, xStore, aScriptingSignatureInformations );
                        return ( bApproved ? allowMacroExecution() : disallowMacroExecution() );
                    }
                }
            }

            // at this point it is clear that the document is neither in secure location nor signed with trusted certificate
            if  (   ( nMacroExecutionMode == MacroExecMode::FROM_LIST_AND_SIGNED_NO_WARN )
                ||  ( nMacroExecutionMode == MacroExecMode::FROM_LIST_AND_SIGNED_WARN )
                )
            {
                if  ( nMacroExecutionMode == MacroExecMode::FROM_LIST_AND_SIGNED_WARN )
                {
                    lcl_showDocumentMacrosDisabledError( _rxInteraction, m_pData->bDocMacroDisabledMessageShown );
                }
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
            ::rtl::OUString sReferrer( m_pData->rDocumentAccess.getDocumentLocation() );

            ::rtl::OUString aSystemFileURL;
            if ( osl::FileBase::getSystemPathFromFileURL( sReferrer, aSystemFileURL ) == osl::FileBase::E_None )
                sReferrer = aSystemFileURL;

            bSecure = lcl_showMacroWarning( _rxInteraction, sReferrer );
        }
        else
            bSecure = ( eAutoConfirm == eAutoConfirmApprove );

        return ( bSecure ? allowMacroExecution() : disallowMacroExecution() );
    }

    //--------------------------------------------------------------------
    sal_Bool DocumentMacroMode::isMacroExecutionDisallowed() const
    {
        return m_pData->rDocumentAccess.getImposedMacroExecMode() == MacroExecMode::NEVER_EXECUTE;
    }

    //--------------------------------------------------------------------
    sal_Bool DocumentMacroMode::hasMacroLibrary() const
    {
        sal_Bool bHasMacroLib = sal_False;
        try
        {
            Reference< XEmbeddedScripts > xScripts( m_pData->rDocumentAccess.getEmbeddedDocumentScripts() );
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
    sal_Bool DocumentMacroMode::storageHasMacros( const Reference< XStorage >& _rxStorage )
    {
        sal_Bool bHasMacros = sal_False;
        if ( _rxStorage.is() )
        {
            try
            {
                static const ::rtl::OUString s_sBasicStorageName( ::rtl::OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "Basic" ) ) );
                static const ::rtl::OUString s_sScriptsStorageName( ::rtl::OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "Scripts" ) ) );

                bHasMacros =(   (   _rxStorage->hasByName( s_sBasicStorageName )
                                &&  _rxStorage->isStorageElement( s_sBasicStorageName )
                                )
                            ||  (   _rxStorage->hasByName( s_sScriptsStorageName )
                                &&  _rxStorage->isStorageElement( s_sScriptsStorageName )
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
    void DocumentMacroMode::checkMacrosOnLoading( const Reference< XInteractionHandler >& _rxInteraction )
    {
        if ( SvtSecurityOptions().IsMacroDisabled() )
        {
            // no macro should be executed at all
            disallowMacroExecution();
        }
        else
        {
            if ( m_pData->rDocumentAccess.documentStorageHasMacros() || hasMacroLibrary() )
            {
                adjustMacroMode( _rxInteraction );
            }
            else if ( !isMacroExecutionDisallowed() )
            {
                // if macros will be added by the user later, the security check is obsolete
                allowMacroExecution();
            }
        }
    }

//........................................................................
} // namespace sfx2
//........................................................................
