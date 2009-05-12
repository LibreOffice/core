/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: documentenumeration.hxx,v $
 * $Revision: 1.3 $
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
