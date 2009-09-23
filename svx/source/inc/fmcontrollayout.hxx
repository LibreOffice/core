/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fmcontrollayout.hxx,v $
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

#ifndef SVX_SOURCE_INC_FMCONTROLLAYOUT_HXX
#define SVX_SOURCE_INC_FMCONTROLLAYOUT_HXX

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/

#ifndef SVX_SOURCE_FORM_FMDOCUMENTCLASSIFICATION_HXX
#include "fmdocumentclassification.hxx"
#endif
#include <unotools/confignode.hxx>

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= ControlLayouter
    //====================================================================
    class ControlLayouter
    {
    public:
        /** initializes the layout of a newly created form control (model)
        */
        static void initializeControlLayout(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
                        DocumentType _eDocType
                    );

        /** determines whether for the given document type, dynamic control border coloring is enabled
        */
        static bool useDynamicBorderColor( DocumentType _eDocType );

        /** determines whether for the given document type, form controls should use the document's reference device
            for text rendering
        */
        static bool useDocumentReferenceDevice( DocumentType _eDocType );

    private:
        ControlLayouter();    // never implemented

    private:
        static ::utl::OConfigurationNode
            getLayoutSettings(
                DocumentType _eDocType
            );
    };

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_SOURCE_INC_FMCONTROLLAYOUT_HXX

