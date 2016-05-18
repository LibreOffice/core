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
#ifndef INCLUDED_BASCTL_SOURCE_INC_DOCSIGNATURE_HXX
#define INCLUDED_BASCTL_SOURCE_INC_DOCSIGNATURE_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <sfx2/signaturestate.hxx>
#include <memory>


namespace basctl
{


    class ScriptDocument;

    /// encapsulates (actions on) the signature/state of a document
    class DocumentSignature
    {
    public:
        /** creates a DocumentSignature instance for the given document

            If the given ScriptDocument instance refers to the application, or to a document
            which does not support being signed, the DocumentSignature instance is invalid afterwards.
        */
        explicit DocumentSignature (ScriptDocument const&);
        ~DocumentSignature();

        /** determines whether the instance is valid

            An instance is valid if and only if it has been constructed with a document
            which supports signatures.
        */
        bool    supportsSignatures() const;

        /** signs the scripting content inside the document

            @precond
                isValid returns <TRUE/>
        */
        void        signScriptingContent() const;

        /** retrieves the state of the signature of the scripting content inside the document

            If the instance is not valid, then SIGNATURESTATE_NOSIGNATURES is returned.
        */
        SignatureState getScriptingSignatureState() const;

    private:
        DocumentSignature() = delete;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };


} // namespace basctl


#endif // INCLUDED_BASCTL_SOURCE_INC_DOCSIGNATURE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
