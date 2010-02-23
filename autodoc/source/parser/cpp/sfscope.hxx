/*************************************************************************
 *
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

#ifndef ADC_CPP_SFSCOPE_HXX
#define ADC_CPP_SFSCOPE_HXX

// USED SERVICES
#include "cxt2ary.hxx"




namespace cpp
{


/** Implementation struct for cpp::ContextForAry.
*/
struct ContextForAry::S_FileScopeInfo
{
    ary::loc::File *    pCurFile;
    uintt               nLineCount;
    Dyn<StringVector>   pCurTemplateParameters;

                        S_FileScopeInfo();
};




// IMPLEMENTATION
inline
ContextForAry::
S_FileScopeInfo::S_FileScopeInfo()
    :   pCurFile(0),
        nLineCount(0),
        pCurTemplateParameters(0)
{
}




}   // namespace cpp
#endif
