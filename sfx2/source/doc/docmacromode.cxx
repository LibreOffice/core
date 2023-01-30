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

#include <config_features.h>

#include <sfx2/docmacromode.hxx>
#include <sfx2/signaturestate.hxx>
#include <sfx2/docfile.hxx>

#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/task/ErrorCodeRequest.hpp>
#include <com/sun/star/task/DocumentMacroConfirmationRequest.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>

#include <comphelper/processfactory.hxx>
#include <framework/interaction.hxx>
#include <osl/file.hxx>
#include <unotools/securityoptions.hxx>
#include <svtools/sfxecode.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/urlobj.hxx>

#if defined(_WIN32)
#include <o3tl/char16_t2wchar_t.hxx>
#include <officecfg/Office/Common.hxx>
#include <systools/win32/comtools.hxx>
#include <urlmon.h>
#endif

namespace sfx2
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::task::XInteractionHandler;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::task::DocumentMacroConfirmationRequest;
    using ::com::sun::star::task::ErrorCodeRequest;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::security::DocumentDigitalSignatures;
    using ::com::sun::star::security::XDocumentDigitalSignatures;
    using ::com::sun::star::embed::XStorage;
    using ::com::sun::star::document::XEmbeddedScripts;
    using ::com::sun::star::script::XLibraryContainer;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::uno::UNO_QUERY_THROW;

    namespace MacroExecMode = ::com::sun::star::document::MacroExecMode;


    //= DocumentMacroMode_Data

    struct DocumentMacroMode_Data
    {
        IMacroDocumentAccess&       m_rDocumentAccess;
        bool                    m_bMacroDisabledMessageShown;
        bool                    m_bDocMacroDisabledMessageShown;
        bool m_bHasUnsignedContentError;

        explicit DocumentMacroMode_Data( IMacroDocumentAccess& rDocumentAccess )
            :m_rDocumentAccess( rDocumentAccess )
            ,m_bMacroDisabledMessageShown( false )
            ,m_bDocMacroDisabledMessageShown( false )
            ,m_bHasUnsignedContentError( false )
        {
        }
    };

    namespace
    {
        bool lcl_showMacroWarning( const Reference< XInteractionHandler >& rxHandler,
            const OUString& rDocumentLocation )
        {
            DocumentMacroConfirmationRequest aRequest;
            aRequest.DocumentURL = rDocumentLocation;
            return SfxMedium::CallApproveHandler( rxHandler, Any( aRequest ), true );
        }
    }

    //= DocumentMacroMode
    DocumentMacroMode::DocumentMacroMode( IMacroDocumentAccess& rDocumentAccess )
        :m_xData( std::make_shared<DocumentMacroMode_Data>( rDocumentAccess ) )
    {
    }

    bool DocumentMacroMode::allowMacroExecution()
    {
        m_xData->m_rDocumentAccess.setCurrentMacroExecMode( MacroExecMode::ALWAYS_EXECUTE_NO_WARN );
        return true;
    }

    bool DocumentMacroMode::disallowMacroExecution()
    {
        m_xData->m_rDocumentAccess.setCurrentMacroExecMode( MacroExecMode::NEVER_EXECUTE );
        return false;
    }

    bool DocumentMacroMode::adjustMacroMode( const Reference< XInteractionHandler >& rxInteraction, bool bHasValidContentSignature )
    {
        sal_uInt16 nMacroExecutionMode = m_xData->m_rDocumentAccess.getCurrentMacroExecMode();

        if ( SvtSecurityOptions::IsMacroDisabled() )
        {
            // no macro should be executed at all
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
            // check confirm first, as nMacroExecutionMode is always overwritten by the GetMacroSecurityLevel() switch
            if (nMacroExecutionMode == MacroExecMode::USE_CONFIG_REJECT_CONFIRMATION)
                eAutoConfirm = eAutoConfirmReject;
            else if (nMacroExecutionMode == MacroExecMode::USE_CONFIG_APPROVE_CONFIRMATION)
                eAutoConfirm = eAutoConfirmApprove;

            switch ( SvtSecurityOptions::GetMacroSecurityLevel() )
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
                    OSL_FAIL( "DocumentMacroMode::adjustMacroMode: unexpected macro security level!" );
                    nMacroExecutionMode = MacroExecMode::NEVER_EXECUTE;
            }
        }

        if ( nMacroExecutionMode == MacroExecMode::NEVER_EXECUTE )
            return false;

        if ( nMacroExecutionMode == MacroExecMode::ALWAYS_EXECUTE_NO_WARN )
            return true;

        const OUString sURL(m_xData->m_rDocumentAccess.getDocumentLocation());
        try
        {
            // get document location from medium name and check whether it is a trusted one
            // the service is created without document version, since it is not of interest here
            Reference< XDocumentDigitalSignatures > xSignatures(DocumentDigitalSignatures::createDefault(::comphelper::getProcessComponentContext()));
            INetURLObject aURLReferer(sURL);

            OUString aLocation;
            if ( aURLReferer.removeSegment() )
                aLocation = aURLReferer.GetMainURL( INetURLObject::DecodeMechanism::NONE );

            if ( !aLocation.isEmpty() && xSignatures->isLocationTrusted( aLocation ) )
            {
                return allowMacroExecution();
            }

            // at this point it is clear that the document is not in the secure location
            if ( nMacroExecutionMode == MacroExecMode::FROM_LIST_NO_WARN )
            {
                return disallowMacroExecution();
            }

            // check whether the document is signed with trusted certificate
            if ( nMacroExecutionMode != MacroExecMode::FROM_LIST )
            {
                // the trusted macro check will also retrieve the signature state ( small optimization )
                const bool bAllowUIToAddAuthor = nMacroExecutionMode != MacroExecMode::FROM_LIST_AND_SIGNED_NO_WARN
                                                 && (nMacroExecutionMode == MacroExecMode::ALWAYS_EXECUTE
                                                     || !SvtSecurityOptions::IsReadOnly(SvtSecurityOptions::EOption::MacroTrustedAuthors));
                const bool bHasTrustedMacroSignature = m_xData->m_rDocumentAccess.hasTrustedScriptingSignature(bAllowUIToAddAuthor);

                SignatureState nSignatureState = m_xData->m_rDocumentAccess.getScriptingSignatureState();
                if ( nSignatureState == SignatureState::BROKEN )
                {
                    return disallowMacroExecution();
                }
                else if ( m_xData->m_rDocumentAccess.macroCallsSeenWhileLoading() &&
                          bHasTrustedMacroSignature &&
                          !bHasValidContentSignature)
                {
                    // When macros are signed, and the document has events which call macros, the document content needs to be signed too.
                    m_xData->m_bHasUnsignedContentError = true;
                    return disallowMacroExecution();
                }
                else if ( bHasTrustedMacroSignature )
                {
                    // there is trusted macro signature, allow macro execution
                    return allowMacroExecution();
                }
                else if ( nSignatureState == SignatureState::OK
                       || nSignatureState == SignatureState::NOTVALIDATED )
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
                return disallowMacroExecution();
            }
        }
        catch ( const Exception& )
        {
            if  (   ( nMacroExecutionMode == MacroExecMode::FROM_LIST_NO_WARN )
                ||  ( nMacroExecutionMode == MacroExecMode::FROM_LIST_AND_SIGNED_WARN )
                ||  ( nMacroExecutionMode == MacroExecMode::FROM_LIST_AND_SIGNED_NO_WARN )
                )
            {
                return disallowMacroExecution();
            }
        }

#if defined(_WIN32)
        // Windows specific: try to decide macros loading depending on Windows Security Zones
        // (is the file local, or it was downloaded from internet, etc?)
        OUString sFilePath;
        osl::FileBase::getSystemPathFromFileURL(sURL, sFilePath);
        sal::systools::COMReference<IZoneIdentifier> pZoneId;
        pZoneId.CoCreateInstance(CLSID_PersistentZoneIdentifier);
        sal::systools::COMReference<IPersistFile> pPersist(pZoneId, sal::systools::COM_QUERY_THROW);
        DWORD dwZone;
        if (!SUCCEEDED(pPersist->Load(o3tl::toW(sFilePath.getStr()), STGM_READ)) ||
            !SUCCEEDED(pZoneId->GetId(&dwZone)))
        {
            // no Security Zone info found -> assume a local file, not
            // from the internet
            dwZone = URLZONE_LOCAL_MACHINE;
        }

        // determine action from zone and settings
        sal_Int32 nAction;
        switch (dwZone) {
            case URLZONE_LOCAL_MACHINE:
                nAction = officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneLocal::get();
                break;
            case URLZONE_INTRANET:
                nAction = officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneIntranet::get();
                break;
            case URLZONE_TRUSTED:
                nAction = officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneTrusted::get();
                break;
            case URLZONE_INTERNET:
                nAction = officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneInternet::get();
                break;
            case URLZONE_UNTRUSTED:
                nAction = officecfg::Office::Common::Security::Scripting::WindowsSecurityZone::ZoneUntrusted::get();
                break;
            default:
                // unknown zone, let's ask the user
                nAction = 0;
                break;
        }

        // act on result
        switch (nAction)
        {
            case 0: // Ask
                break;
            case 1: // Allow
                return allowMacroExecution();
            case 2: // Deny
                return disallowMacroExecution();
        }
#endif
        // confirmation is required
        bool bSecure = false;

        if ( eAutoConfirm == eNoAutoConfirm )
        {
            OUString sReferrer(sURL);

            OUString aSystemFileURL;
            if ( osl::FileBase::getSystemPathFromFileURL( sReferrer, aSystemFileURL ) == osl::FileBase::E_None )
                sReferrer = aSystemFileURL;

            bSecure = lcl_showMacroWarning( rxInteraction, sReferrer );
        }
        else
            bSecure = ( eAutoConfirm == eAutoConfirmApprove );

        return ( bSecure ? allowMacroExecution() : disallowMacroExecution() );
    }


    bool DocumentMacroMode::isMacroExecutionDisallowed() const
    {
        return m_xData->m_rDocumentAccess.getCurrentMacroExecMode() == MacroExecMode::NEVER_EXECUTE;
    }


    bool DocumentMacroMode::containerHasBasicMacros( const Reference< XLibraryContainer >& xContainer )
    {
        bool bHasMacroLib = false;
        try
        {
            if ( xContainer.is() )
            {
                // a library container exists; check if it's empty

                // if there are libraries except the "Standard" library
                // we assume that they are not empty (because they have been created by the user)
                if ( !xContainer->hasElements() )
                    bHasMacroLib = false;
                else
                {
                    static const OUStringLiteral aStdLibName( u"Standard" );
                    static const OUStringLiteral aVBAProject( u"VBAProject" );
                    const Sequence< OUString > aElements = xContainer->getElementNames();
                    for( const OUString& aElement : aElements )
                    {
                        if( aElement == aStdLibName || aElement == aVBAProject )
                        {
                            Reference < XNameAccess > xLib;
                            Any aAny = xContainer->getByName( aElement );
                            aAny >>= xLib;
                            if ( xLib.is() && xLib->hasElements() )
                                return true;
                        }
                        else
                            return true;
                    }
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("sfx.doc");
        }
        return bHasMacroLib;
    }


    bool DocumentMacroMode::hasMacroLibrary() const
    {
        bool bHasMacroLib = false;
#if HAVE_FEATURE_SCRIPTING
        try
        {
            Reference< XEmbeddedScripts > xScripts( m_xData->m_rDocumentAccess.getEmbeddedDocumentScripts() );
            Reference< XLibraryContainer > xContainer;
            if ( xScripts.is() )
                xContainer.set( xScripts->getBasicLibraries(), UNO_QUERY_THROW );
            bHasMacroLib = containerHasBasicMacros( xContainer );

        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("sfx.doc");
        }
#endif
        return bHasMacroLib;
    }

    bool DocumentMacroMode::hasUnsignedContentError() const
    {
        return m_xData->m_bHasUnsignedContentError;
    }


    bool DocumentMacroMode::storageHasMacros( const Reference< XStorage >& rxStorage )
    {
        bool bHasMacros = false;
        if ( rxStorage.is() )
        {
            try
            {
                static constexpr OUStringLiteral s_sBasicStorageName( u"Basic" );
                static constexpr OUStringLiteral s_sScriptsStorageName( u"Scripts" );

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
                DBG_UNHANDLED_EXCEPTION("sfx.doc");
            }
        }
        return bHasMacros;
    }


    bool DocumentMacroMode::checkMacrosOnLoading( const Reference< XInteractionHandler >& rxInteraction, bool bHasValidContentSignature )
    {
        bool bAllow = false;
        if ( SvtSecurityOptions::IsMacroDisabled() )
        {
            // no macro should be executed at all
            bAllow = disallowMacroExecution();
        }
        else
        {
            if (m_xData->m_rDocumentAccess.documentStorageHasMacros() || hasMacroLibrary() || m_xData->m_rDocumentAccess.macroCallsSeenWhileLoading())
            {
                bAllow = adjustMacroMode( rxInteraction, bHasValidContentSignature );
            }
            else if ( !isMacroExecutionDisallowed() )
            {
                // if macros will be added by the user later, the security check is obsolete
                bAllow = allowMacroExecution();
            }
        }
        return bAllow;
    }


} // namespace sfx2


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
