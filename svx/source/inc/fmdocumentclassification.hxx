/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmdocumentclassification.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-04-16 16:22:13 $
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

#ifndef SVX_SOURCE_INC_FMDOCUMENTCLASSIFICATION_HXX
#define SVX_SOURCE_INC_FMDOCUMENTCLASSIFICATION_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
/** === end UNO includes === **/

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= DocumentType
    //====================================================================
    enum DocumentType
    {
        eTextDocument,
        eWebDocument,
        eSpreadsheetDocument,
        eDrawingDocument,
        ePresentationDocument,
        eEnhancedForm,
        eDatabaseForm,
        eDatabaseReport,

        eUnknownDocumentType
    };

    //====================================================================
    //= DocumentClassification
    //====================================================================
    class DocumentClassification
    {
    public:
        /** classifies a document model
        */
        static DocumentType classifyDocument(
                                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxDocumentModel
                            ) SAL_THROW(());

        static DocumentType classifyHostDocument(
                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxFormComponent
                            ) SAL_THROW(());

        static  DocumentType getDocumentTypeForModuleIdentifier(
                                const ::rtl::OUString& _rModuleIdentifier
                            );

        static  ::rtl::OUString getModuleIdentifierForDocumentType(
                                DocumentType _eType
                            );
    };

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_SOURCE_INC_FMDOCUMENTCLASSIFICATION_HXX

