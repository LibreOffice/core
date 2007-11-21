/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docsignature.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 16:37:39 $
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
