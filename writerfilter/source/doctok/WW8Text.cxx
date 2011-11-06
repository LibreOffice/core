/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
