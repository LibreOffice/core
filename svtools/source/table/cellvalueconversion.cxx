/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "precompiled_svtools.hxx"

#include "cellvalueconversion.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//......................................................................................................................
namespace svt
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Any;
    /** === end UNO using === **/

    //==================================================================================================================
    //= CellValueConversion
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString CellValueConversion::convertToString( const Any& i_value )
    {
        ::rtl::OUString sConvertString;
        if ( !i_value.hasValue() )
            return sConvertString;


        // TODO: use css.script.XTypeConverter?

        sal_Int32 nInt = 0;
        sal_Bool bBool = false;
        double fDouble = 0;

        ::rtl::OUString sStringValue;
        if ( i_value >>= sConvertString )
            sStringValue = sConvertString;
        else if ( i_value >>= nInt )
            sStringValue = sConvertString.valueOf( nInt );
        else if ( i_value >>= bBool )
            sStringValue = sConvertString.valueOf( bBool );
        else if ( i_value >>= fDouble )
            sStringValue = sConvertString.valueOf( fDouble );
        else
            OSL_ENSURE( !i_value.hasValue(), "CellValueConversion::convertToString: cannot handle the given cell content type!" );

        return sStringValue;
    }

//......................................................................................................................
} // namespace svt
//......................................................................................................................
