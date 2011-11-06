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


#ifndef _ZIP_ENTRY_HXX_
#define _ZIP_ENTRY_HXX_

#include <rtl/ustring.hxx>

struct ZipEntry
{
    sal_Int16 nVersion;
    sal_Int16 nFlag;
    sal_Int16 nMethod;
    sal_Int32 nTime;
    sal_Int32 nCrc;
    sal_Int32 nCompressedSize;
    sal_Int32 nSize;
    sal_Int32 nOffset;
    sal_Int16 nPathLen;
    sal_Int16 nExtraLen;
    ::rtl::OUString sPath;
};
#endif
