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

#ifndef INCLUDED_SFX2_DOCMACROMODE_HXX
#define INCLUDED_SFX2_DOCMACROMODE_HXX

#include <sfx2/dllapi.h>
#include <sfx2/signaturestate.hxx>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>

#include <memory>


namespace sfx2
{



    //= IMacroDocumentAccess

    /** provides access to several settings of a document, which are needed by ->DocumentMacroMode
        to properly determine the current macro execution mode of this document
    */
    class SAL_NO_VTABLE IMacroDocumentAccess
    {
    public:
        /** retrieves the current MacroExecutionMode.

            Usually, this is initialized from the media descriptor used to load the document,
            respectively the one passed into the document's XModel::attachResource call.

            If no such mode was passed there, document implementations should return
            MacroExecMode::NEVER_EXECUTE.

            @see css::document::MediaDescriptor::MacroExecutionMode
            @see css::frame::XComponentLoader::loadComponentFromURL
            @see css::frame::XModel::attachResource

            @see setCurrentMacroExecMode

            @todo
                Effectively, this is the MacroExecutionMode of the MediaDescriptor of
                the document. Thus, this setting could be obtained from the XModel
                directly. We should introduce a getDocumentModel method here, which
                can be used for this and other purposes.
        */
        virtual sal_Int16
                    getCurrentMacroExecMode() const = 0;

        /** sets the MacroExecutionMode of the document, as calculated by the DocumentMacroMode
            class.

            Effectively, the existence of this method means that the responsibility
            to store the current macro execution mode is not with the DocumentMacroMode
            instance, but with the document instance itself.

            Usually, a document implementation will simply put the macro execution mode
            into its media descriptor, as returned by XModel::getArgs.

            @see css::document::MediaDescriptor::MacroExecutionMode
            @see css::frame::XComponentLoader::loadComponentFromURL
            @see css::frame::XModel::attachResource

            see getCurrentMacroExecMode
        */
        virtual bool
                    setCurrentMacroExecMode( sal_uInt16 ) = 0;

        /** returns the origin of the document

            This usually is the document's location, or, if the document has been
            newly created from a template, then the location of the template. Location
            here means the complete path of the document, including the file name.

            @todo
                This probably can also be obtained from the XModel, by calling getURL
                or getLocation. If both are empty, then we need an UNO way to obtain
                the URL of the underlying template document - if any. If we have this,
                we could replace this method with a newly introduced method
                getDocumentModel and some internal code.
        */
        virtual OUString
                    getDocumentLocation() const = 0;

        /** checks whether the document's storage contains sub storages with macros or scripts

            A default implementation of this method will simply cann DocumentMacroMode::storageHasMacros
            with the document's root storage. However, there might be document types where this
            isn't sufficient (e.g. database documents which contain sub documents which can also
            contain macro/script storages).
        */
        virtual bool
                    documentStorageHasMacros() const = 0;

        /** provides access to the XEmbeddedScripts interface of the document

            Implementations are allowed to return <NULL/> here if and only if they
            do not (yet) support embedding scripts.

            @todo
                can also be replaced with a call to the (to be introduced) getDocumentModel
                method, and a queryInterface.
        */
        virtual css::uno::Reference< css::document::XEmbeddedScripts >
                    getEmbeddedDocumentScripts() const = 0;

        /** returns the state of the signatures for the scripts embedded in the document

            Note: On the medium run, the signature handling of a document should be outsourced
            into a dedicated class, instead of being hard-wired into the SfxObjectShell. This
            class could then be used outside the SfxObjectShell (e.g. in Base documents), too.
            When this happens, this method here becomes should be replaced by a method at this
            new class.

            @seealso <sfx2/signaturestate.hxx>
        */
        virtual SignatureState
                    getScriptingSignatureState() = 0;

        /** allows to detect whether there is a trusted scripting signature

            Note: On the medium run, the signature handling of a document should be outsourced
            into a dedicated class, instead of being hard-wired into the SfxObjectShell. This
            class could then be used outside the SfxObjectShell (e.g. in Base documents), too.
            When this happens, this method here should be replaced by a method at this
            new class.

            @seealso <sfx2/signaturestate.hxx>
        */
        virtual bool
                    hasTrustedScriptingSignature( bool bAllowUIToAddAuthor ) = 0;

        /** shows a warning that the document's signature is broken

            Here, a similar note applies as to getScriptingSignatureState: This method doesn't
            really belong here. It's just there because SfxObjectShell_Impl::bSignatureErrorIsShown
            is not accessible where the method is called.
            So, once the signature handling has been oursourced from SfxObjectShell/_Impl, so it
            is re-usable in non-SFX contexts as well, this method here is also unneeded, probably.

            @param _rxInteraction
                the interaction handler to use for showing the warning. It is exactly the same
                as passed to DocumentMacroMode::adjustMacroMode, so it is <NULL/> if and
                only if the instance passed to that method was <NULL/>.
        */
        virtual void
                    showBrokenSignatureWarning(
                        const css::uno::Reference< css::task::XInteractionHandler >& _rxInteraction
                    ) const = 0;

    protected:
        ~IMacroDocumentAccess() {}
    };


    //= DocumentMacroMode

    struct DocumentMacroMode_Data;

    /** encapsulates handling the macro mode of a document

        @see css::document::MacroExecMode
    */
    class SFX2_DLLPUBLIC DocumentMacroMode
    {
    public:
        /** creates an instance

        @param _rDocumentAccess
            access to the document which this instance works for. Must live as long as the
            DocumentMacroMode instance lives, at least
        */
        DocumentMacroMode( IMacroDocumentAccess& _rDocumentAccess );
        ~DocumentMacroMode();

        /** allows macro execution in the document

            Effectively, the macro mode is set to MacroExecMode::ALWAYS_EXECUTE_NO_WARN.

            @return
                <TRUE/>, always
        */
        bool    allowMacroExecution();

        /** disallows macro execution in the document

            Effectively, the macro mode is set to MacroExecMode::NEVER_EXECUTE.

            @return
                <TRUE/>, always
        */
        bool    disallowMacroExecution();

        /** checks whether the document allows executing contained macros.

            The method transforms the current macro execution mode into either
            ALWAYS_EXECUTE_NO_WARN or NEVER_EXECUTE, depending on the current value,
            possible configuration settings, and possible user interaction.

            @param _rxInteraction
                A handler for interactions which might become necessary.
                This includes
                <ul><li>Asking the user for confirmation for macro execution.</li>
                    <li>Telling the user that macro execution is disabled.</li>
                </ul>

                If the user needs to be asked for macro execution confirmation, and if
                this parameter is <NULL/>, the most defensive assumptions will be made,
                effectively disabling macro execution.

            @return
                <TRUE/> if and only if macro execution in this document is allowed.
        */
        bool    adjustMacroMode(
                    const css::uno::Reference< css::task::XInteractionHandler >& _rxInteraction
                );

        /** determines whether macro execution is disallowed

            There's a number of reasons why macro execution could be disallowed:
            <ul><li>Somebody called ->disallowMacroExecution</li>
                <li>Macro execution is disabled globally, via the security options</li>
                <li>Macro execution mode was not defined initially, and the user denied
                    executing macros for this particular document.</li>
            </ul>

            Note that if this method returns <FALSE/>, then subsequent calls of
            ->adjustMacroMode can still return <FALSE/>.
            That is, if the current macro execution mode for the document is not yet known
            (and inparticular <em>not</em> MacroExecMode::NEVER_EXECUTE), then ->isMacroExecutionDisallowed
            will return <FALSE/>.
            However, a subsequent call to ->adjustMacroMode can result in the user
            denying macro execution, in which ->adjustMacroMode will return <FALSE/>,
            and the next call to isMacroExecutionDisallowed will return <TRUE/>.
        */
        bool    isMacroExecutionDisallowed() const;

        /** determines whether the document actually has a macros library

            Effectively, this method checks the Basic library container (as returned by
            IMacroDocumentAccess::getEmbeddedDocumentScripts().getBasicLibraries) for
            content.
        */
        bool    hasMacroLibrary() const;

        /** determines whether the given document storage has sub storages containing scripts
            or macros.

            Effectively, the method checks for the presence of a sub-storage name "Scripts" (where
            BeanShell-/JavaScript-/Python-Scripts are stored, and a sub storage named "Basic" (where
            Basic scripts are stored).
        */
        static bool storageHasMacros( const css::uno::Reference< css::embed::XStorage >& _rxStorage );

        static bool containerHasBasicMacros( const css::uno::Reference< css::script::XLibraryContainer >& xContainter );
        /** checks the macro execution mode while loading the document.

            This must be called when the loading is effectively finished, but before any macro action
            happened.

            The method will disallow macro execution for this document if it is disabled
            globally (SvtSecurityOptions::IsMacroDisabled). Otherwise, it will check whether
            the document contains a macro storage or macro libraries. If so, it will
            properly calculate the MacroExecutionMode by calling adjustMacroMode.

            If the document doesn't contain macros, yet, then the macro execution for this
            document will be allowed (again: unless disabled globally), since in this case
            macros which later are newly created by the user should be allowed, of course.

            @return
                <TRUE/> if and only if macro execution is allowed in the document

            @see isMacroExecutionDisallowed
            @see IMacroDocumentAccess::documentStorageHasMacros
            @see hasMacroLibrary
            @see IMacroDocumentAccess::checkForBrokenScriptingSignatures
        */
        bool
                checkMacrosOnLoading(
                    const css::uno::Reference< css::task::XInteractionHandler >& _rxInteraction
                );

    private:
        std::shared_ptr< DocumentMacroMode_Data >   m_xData;
    };


} // namespace sfx2


#endif // INCLUDED_SFX2_DOCMACROMODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
