/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: docsignature.cxx,v $
 * $Revision: 1.4 $
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
#include "precompiled_basctl.hxx"

#include "docsignature.hxx"
#include "scriptdocument.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

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
    //= DocumentSignature_Data
    //====================================================================
    struct DocumentSignature_Data
    {
        SfxObjectShell*   pShell;

        DocumentSignature_Data() : pShell( NULL ) { }
    };

    //====================================================================
    //= DocumentSignature
    //====================================================================
    //--------------------------------------------------------------------
    DocumentSignature::DocumentSignature( const ScriptDocument& _rDocument )
        :m_pData( new DocumentSignature_Data )
    {
        if ( _rDocument.isDocument() )
        {
            Reference< XModel > xDocument( _rDocument.getDocument() );
            // find object shell for document
            SfxObjectShell* pShell = SfxObjectShell::GetFirst();
            while ( pShell )
            {
                if ( pShell->GetModel() == xDocument )
                    break;
                pShell = SfxObjectShell::GetNext( *pShell );
            }
            m_pData->pShell = pShell;
        }
    }

    //--------------------------------------------------------------------
    DocumentSignature::~DocumentSignature()
    {
    }

    //--------------------------------------------------------------------
    bool DocumentSignature::supportsSignatures() const
    {
        return ( m_pData->pShell != NULL );
    }

    //--------------------------------------------------------------------
    void DocumentSignature::signScriptingContent() const
    {
        OSL_PRECOND( supportsSignatures(), "DocumentSignature::signScriptingContent: signatures not supported by this document!" );
        if ( m_pData->pShell )
            m_pData->pShell->SignScriptingContent();
    }

    //--------------------------------------------------------------------
    sal_uInt16 DocumentSignature::getScriptingSignatureState() const
    {
        if ( m_pData->pShell )
            return m_pData->pShell->GetScriptingSignatureState();
        return SIGNATURESTATE_NOSIGNATURES;
    }

//........................................................................
} // namespace basctl
//........................................................................
