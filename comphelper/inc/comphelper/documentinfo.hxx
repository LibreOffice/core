/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: documentinfo.hxx,v $
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

#ifndef COMPHELPER_DOCUMENTINFO_HXX
#define COMPHELPER_DOCUMENTINFO_HXX

#include "comphelper/comphelperdllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/frame/XModel.hpp>
/** === end UNO includes === **/

//........................................................................
namespace comphelper {
//........................................................................

    //====================================================================
    //= DocumentInfo
    //====================================================================
    class COMPHELPER_DLLPUBLIC DocumentInfo
    {
    public:
        /** retrieves the UI title of the given document
        */
        static ::rtl::OUString  getDocumentTitle( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxDocument );

    private:
        DocumentInfo(); // never implemented
    };

//........................................................................
}   // namespace comphelper
//........................................................................

#endif // COMPHELPER_DOCUMENTINFO_HXX
