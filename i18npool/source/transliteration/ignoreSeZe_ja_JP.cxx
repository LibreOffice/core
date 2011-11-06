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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

// prevent internal compiler error with MSVC6SP3
#include <utility>

#define TRANSLITERATION_SeZe_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

static Mapping SeZe[] = {
        // SI + E --> SE
        { 0x30B7, 0x30A7, 0x30BB, sal_True },
        // SI + E --> SE
        { 0x3057, 0x3047, 0x305B, sal_True },
        // ZI + E --> ZE
        { 0x30B8, 0x30A7, 0x30BC, sal_True },
        // ZI + E --> ZE
        { 0x3058, 0x3047, 0x305C, sal_True },

        { 0, 0, 0, sal_True }
};

ignoreSeZe_ja_JP::ignoreSeZe_ja_JP()
{
        func = (TransFunc) 0;
        table = 0;
        map = SeZe;
        transliterationName = "ignoreSeZe_ja_JP";
        implementationName = "com.sun.star.i18n.Transliteration.ignoreSeZe_ja_JP";
}

} } } }
