/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: formpdfexport.hxx,v $
 *
 * $Revision: 1.2 $
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

#ifndef _TOOLKIT_HELPER_FORM_FORMPDFEXPORT_HXX
#define _TOOLKIT_HELPER_FORM_FORMPDFEXPORT_HXX

#ifndef TOOLKIT_DLLAPI_H
#include <toolkit/dllapi.h>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
/** === end UNO includes === **/

#ifndef _VCL_PDFWRITER_HXX
#include <vcl/pdfwriter.hxx>
#endif

#include <memory>

//........................................................................
namespace toolkitform
{
//........................................................................

    /** creates a PDF compatible control descriptor for the given control
    */
    void TOOLKIT_DLLPUBLIC describePDFControl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl,
            ::std::auto_ptr< ::vcl::PDFWriter::AnyWidget >& _rpDescriptor
        ) SAL_THROW(());

//........................................................................
} // namespace toolkitform
//........................................................................

#endif // _TOOLKIT_HELPER_FORM_FORMPDFEXPORT_HXX
