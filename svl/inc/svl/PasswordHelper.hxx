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



#ifndef _SVTOOLS_PASSWORDHELPER_HXX
#define _SVTOOLS_PASSWORDHELPER_HXX

#include "svl/svldllapi.h"
#include "sal/types.h"
#include "com/sun/star/uno/Sequence.hxx"

class String;

class SvPasswordHelper
{
    static void     GetHashPassword(com::sun::star::uno::Sequence <sal_Int8>& rPassHash, const sal_Char* pPass, sal_uInt32 nLen);
    static void     GetHashPasswordLittleEndian(com::sun::star::uno::Sequence<sal_Int8>& rPassHash, const String& sPass);
    static void     GetHashPasswordBigEndian(com::sun::star::uno::Sequence<sal_Int8>& rPassHash, const String& sPass);

public:
    SVL_DLLPUBLIC static void     GetHashPassword(com::sun::star::uno::Sequence<sal_Int8>& rPassHash, const String& sPass);
    /**
    Use this method to compare a given string with another given Hash value.
    This is necessary, because in older versions exists different hashs of the same string. They were endian dependent.
    We need this to handle old files. This method will compare against big and little endian. See #101326#
    */
    SVL_DLLPUBLIC static bool     CompareHashPassword(const com::sun::star::uno::Sequence<sal_Int8>& rOldPassHash, const String& sNewPass);
};

#endif

