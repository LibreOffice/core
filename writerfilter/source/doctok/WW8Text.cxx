/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8Text.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:51:22 $
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
