/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docsignature.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:25:25 $
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
#ifndef BASCTL_DOCSIGNATURE_HXX
#define BASCTL_DOCSIGNATURE_HXX

/** === begin UNO includes === **/
#include <com/sun/star/frame/XModel.hpp>
/** === end UNO includes === **/

#include <memory>

//........................................................................
namespace basctl
{
//........................................................................

    class ScriptDocument;
    //====================================================================
    //= DocumentSignature
    //====================================================================
    struct DocumentSignature_Data;
    /// encapsulates (actions on) the signature/state of a document
    class DocumentSignature
    {
    public:
        /** creates a DocumentSignature instance for the given document

            If the given ScriptDocument instance refers to the application, or to a document
            which does not support being signed, the DocumentSignature instance is invalid afterwards.
        */
        DocumentSignature( const ScriptDocument& _rDocument );
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
        sal_uInt16  getScriptingSignatureState() const;

    private:
        DocumentSignature();        // not implemented

    private:
        ::std::auto_ptr< DocumentSignature_Data >   m_pData;
    };

//........................................................................
} // namespace basctl
//........................................................................

#endif // BASCTL_DOCSIGNATURE_HXX
