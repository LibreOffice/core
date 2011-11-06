/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
