/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: linedescriptor.hxx,v $
 * $Revision: 1.11 $
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
#ifndef _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_
#define _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_

#include <vcl/image.hxx>
/** === begin UNO includes === **/
#include <com/sun/star/inspection/LineDescriptor.hpp>
/** === end UNO includes === **/

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= OLineDescriptor
    //========================================================================
    struct OLineDescriptor : public ::com::sun::star::inspection::LineDescriptor
    {
        ::rtl::OUString             sName;              // the name of the property
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >
                                    xPropertyHandler;   // the handler for this property
        ::com::sun::star::uno::Any  aValue;             // the current value of the property

        bool                        bUnknownValue    : 1;   // is the property value currently "unknown"? (PropertyState_AMBIGUOUS)
        bool                        bReadOnly        : 1;

        OLineDescriptor()
            :bUnknownValue( false )
            ,bReadOnly( false )
        {
        }

        void assignFrom( const ::com::sun::star::inspection::LineDescriptor& _rhs )
        {
            ::com::sun::star::inspection::LineDescriptor::operator=( _rhs );
        }
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_

