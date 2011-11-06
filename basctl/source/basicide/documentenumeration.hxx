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



#ifndef BASCTL_DOCUMENTENUMERATION_HXX
#define BASCTL_DOCUMENTENUMERATION_HXX

/** === begin UNO includes === **/
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>

#include <memory>
#include <vector>

//........................................................................
namespace basctl { namespace docs {
//........................................................................

    typedef ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                         Model;
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > >   Controllers;

    struct DocumentDescriptor
    {
        Model       xModel;
        Controllers aControllers;
    };

    typedef ::std::vector< DocumentDescriptor > Documents;

    //====================================================================
    //= IDocumentDescriptorFilter
    //====================================================================
    /// allows pre-filtering when enumerating document descriptors
    class SAL_NO_VTABLE IDocumentDescriptorFilter
    {
    public:
        virtual bool    includeDocument( const DocumentDescriptor& _rDocument ) const = 0;
    };

    //====================================================================
    //= DocumentEnumeration
    //====================================================================
    struct DocumentEnumeration_Data;
    /** is a helper class for enumerating documents in OOo

        If you need a list of all open documents in OOo, this is little bit of
        a hassle: You need to iterate though all components at the desktop, which
        might or might not be documents.

        Additionally, you need to examine the existing documents' frames
        for sub frames, which might contain sub documents (e.g. embedded objects
        edited out-place).

        DocumentEnumeration relieves you from this hassle.
    */
    class DocumentEnumeration
    {
    public:
        DocumentEnumeration( const ::comphelper::ComponentContext& _rContext, const IDocumentDescriptorFilter* _pFilter = NULL );
        ~DocumentEnumeration();

        /** retrieves a list of all currently known documents in the application

            @param _out_rDocuments
                output parameter taking the collected document information
            @
        */
        void    getDocuments(
            Documents& _out_rDocuments
        ) const;

    private:
        ::std::auto_ptr< DocumentEnumeration_Data > m_pData;
    };

//........................................................................
} } // namespace basctl::docs
//........................................................................

#endif // BASCTL_DOCUMENTENUMERATION_HXX
