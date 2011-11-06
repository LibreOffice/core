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



#ifndef INCLUDED_FILE_IMPL_HXX
#define INCLUDED_FILE_IMPL_HXX

#include "osl/file.h"
#include <stddef.h>

struct DirectoryItem_Impl
{
    sal_Int32     m_RefCount;

    rtl_uString * m_ustrFilePath;       /* holds native file name */
    unsigned char m_DType;

    explicit DirectoryItem_Impl(
        rtl_uString * ustrFilePath, unsigned char DType = 0);
    ~DirectoryItem_Impl();

    static void * operator new(size_t n);
    static void operator delete (void * p, size_t);

    void acquire(); /* @see osl_acquireDirectoryItem() */
    void release(); /* @see osl_releaseDirectoryItem() */

    oslFileType getFileType() const;
};

#endif /* INCLUDED_FILE_IMPL_HXX */
