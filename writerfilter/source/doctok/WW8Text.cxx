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

#include <resources.hxx>

namespace writerfilter {
namespace doctok {
sal_uInt32 WW8sprmPChgTabsPapx::calcSize()
{
    return getS8(0x2) + 3;
}

sal_uInt32 WW8sprmPChgTabsPapx::get_dxaDel_count()
{
    return getS8(0x3);
}

sal_Int16  WW8sprmPChgTabsPapx::get_dxaDel(sal_uInt32 pos)
{
    return getS16(0x4 + pos * 2);
}

sal_uInt32 WW8sprmPChgTabsPapx::get_dxaAdd_count()
{
    return getS8(0x4 + get_dxaDel_count() * 2);
}

sal_Int16 WW8sprmPChgTabsPapx::get_dxaAdd(sal_uInt32 pos)
{
    return getS16(0x4 + get_dxaDel_count() * 2 + 1 + pos * 2);
}

sal_uInt32 WW8sprmPChgTabsPapx::get_tbdAdd_count()
{
    return get_dxaAdd_count();
}

writerfilter::Reference<Properties>::Pointer_t
WW8sprmPChgTabsPapx::get_tbdAdd(sal_uInt32 pos)
{
    //wntmsci compiler cannot handle 'too many inlines' ;-)
    writerfilter::Reference<Properties>::Pointer_t pRet( new WW8TBD(this, 
                    0x4 + get_dxaDel_count() * 2 + 1 + get_dxaAdd_count() * 2 
                    + pos, 1));
    return pRet;
    
/*    return writerfilter::Reference<Properties>::Pointer_t
        (new WW8TBD(this, 
                    0x4 + get_dxaDel_count() * 2 + 1 + get_dxaAdd_count() * 2 
                    + pos, 1));*/
}
}}
