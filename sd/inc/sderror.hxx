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



#ifndef _SDSWERROR_H
#define _SDSWERROR_H

#include <tools/errcode.hxx>

#define ERROR_SD_BASE           (ERRCODE_AREA_SD)
#define ERROR_SD_READ_BASE      (ERROR_SD_BASE | ERRCODE_CLASS_READ)
#define ERROR_SD_WRITE_BASE     (ERROR_SD_BASE | ERRCODE_CLASS_WRITE)

#define WARN_SD_BASE            (ERRCODE_AREA_SD | ERRCODE_WARNING_MASK)
#define WARN_SD_READ_BASE       (WARN_SD_BASE | ERRCODE_CLASS_READ )
#define WARN_SD_WRITE_BASE      (WARN_SD_BASE | ERRCODE_CLASS_WRITE )


// Import errors
#define ERR_FORMAT_ROWCOL               (ERROR_SD_READ_BASE | 1)
#define ERR_FORMAT_FILE_ROWCOL          (ERROR_SD_READ_BASE | 2)

// ----- Warnings ---------------------------

#define WARN_FORMAT_FILE_ROWCOL         (WARN_SD_READ_BASE | 100)

#ifndef __RSC

inline bool IsWarning( sal_uLong nErr )
{
    return 0 != ( nErr & ERRCODE_WARNING_MASK & nErr );
}

inline bool IsError( sal_uLong nErr )
{
    return nErr && 0 == ( ERRCODE_WARNING_MASK & nErr );
}

#endif


#endif
