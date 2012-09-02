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

#include "docsignature.hxx"
#include "scriptdocument.hxx"

#include <sfx2/objsh.hxx>
#include <sfx2/signaturestate.hxx>

//........................................................................
namespace basctl
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::frame::XModel;
    /** === end UNO using === **/

    //====================================================================
    //= DocumentSignature::Impl
    //====================================================================
    struct DocumentSignature::Impl
    {
        SfxObjectShell* pShell;

        Impl () : pShell(0) { }
    };

    //====================================================================
    //= DocumentSignature
    //====================================================================
    //--------------------------------------------------------------------
    DocumentSignature::DocumentSignature (ScriptDocument const& rDocument) :
        m_pImpl(new Impl)
    {
        if (rDocument.isDocument())
        {
            Reference<XModel> xDocument(rDocument.getDocument());
            // find object shell for document
            SfxObjectShell* pShell = SfxObjectShell::GetFirst();
            while ( pShell )
            {
                if ( pShell->GetModel() == xDocument )
                    break;
                pShell = SfxObjectShell::GetNext( *pShell );
            }
            m_pImpl->pShell = pShell;
        }
    }

    //--------------------------------------------------------------------
    DocumentSignature::~DocumentSignature()
    {
    }

    //--------------------------------------------------------------------
    bool DocumentSignature::supportsSignatures() const
    {
        return ( m_pImpl->pShell != NULL );
    }

    //--------------------------------------------------------------------
    void DocumentSignature::signScriptingContent() const
    {
        OSL_PRECOND( supportsSignatures(), "DocumentSignature::signScriptingContent: signatures not supported by this document!" );
        if ( m_pImpl->pShell )
            m_pImpl->pShell->SignScriptingContent();
    }

    //--------------------------------------------------------------------
    sal_uInt16 DocumentSignature::getScriptingSignatureState() const
    {
        if ( m_pImpl->pShell )
            return m_pImpl->pShell->GetScriptingSignatureState();
        return SIGNATURESTATE_NOSIGNATURES;
    }

//........................................................................
} // namespace basctl
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
