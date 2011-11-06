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


#ifndef _SVXERR_HXX
#define _SVXERR_HXX

// include ---------------------------------------------------------------

#include <tools/errcode.hxx>

// define ----------------------------------------------------------------

#define ERRCODE_SVX_LINGU_THESAURUSNOTEXISTS   (1UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_NOTEXISTS)

// moved to editeng lib
//#define ERRCODE_SVX_LINGU_LANGUAGENOTEXISTS    (2UL | ERRCODE_AREA_SVX)

#define ERRCODE_SVX_LINGU_LINGUNOTEXISTS       (3UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_SVX_LINGU_HYPHENNOTEXISTS      (4UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_SVX_LINGU_DICT_NOTREADABLE     (5UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_READ )
#define ERRCODE_SVX_LINGU_DICT_NOTWRITEABLE    (6UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_WRITE )

#define ERRCODE_SVX_GRAPHIC_NOTREADABLE        (7UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_READ )
// moved to editeng lib
//#define ERRCODE_SVX_GRAPHIC_WRONG_FILEFORMAT   (8UL | ERRCODE_AREA_SVX | ERRCODE_CLASS_READ )

#define ERRCODE_SVX_LINGU_NOLANGUAGE           (9UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_NOTEXISTS )
#define ERRCODE_SVX_FORMS_NOIOSERVICES         (10UL | ERRCODE_AREA_SVX )
#define ERRCODE_SVX_FORMS_READWRITEFAILED      (11UL | ERRCODE_AREA_SVX )

#define ERRCODE_SVX_BULLETITEM_NOBULLET        (12UL | ERRCODE_AREA_SVX )

#define ERRCODE_SVX_MODIFIED_VBASIC_STORAGE    (13UL | ERRCODE_AREA_SVX     \
                                                     | ERRCODE_WARNING_MASK \
                                                     | ERRCODE_CLASS_WRITE )

#define ERRCODE_SVX_VBASIC_STORAGE_EXIST       (14UL | ERRCODE_AREA_SVX     \
                                                     | ERRCODE_WARNING_MASK \
                                                     | ERRCODE_CLASS_WRITE )

/** Error message: "Wrong password." */
#define ERRCODE_SVX_WRONGPASS               (15UL | ERRCODE_AREA_SVX | ERRCODE_CLASS_NONE)

/** Error message: "Read error. Unsupported encryption method." */
#define ERRCODE_SVX_READ_FILTER_CRYPT       (16UL | ERRCODE_AREA_SVX | ERRCODE_CLASS_READ)

/** Error message: "Read error. Passwort encrypted Powerpoint documents..." */
#define ERRCODE_SVX_READ_FILTER_PPOINT      (17UL | ERRCODE_AREA_SVX | ERRCODE_CLASS_READ)

/** Error message: "Warning. Passwort protection is not suppported when ..." */
#define ERRCODE_SVX_EXPORT_FILTER_CRYPT      (18UL | ERRCODE_AREA_SVX | ERRCODE_CLASS_EXPORT | ERRCODE_WARNING_MASK)


// both codes will be used twice : with ERRCODE_CLASS_READ- and ERRCODE_CLASS_WRITE-bits
#define ERRCTX_SVX_LINGU_THESAURUS              1
#define ERRCTX_SVX_LINGU_SPELLING               2
#define ERRCTX_SVX_LINGU_HYPHENATION            3
#define ERRCTX_SVX_LINGU_DICTIONARY             4
#define ERRCTX_SVX_BACKGROUND                   5
#define ERRCTX_SVX_IMPORT_GRAPHIC               6

// class SvxErrorHandler -------------------------------------------------

#ifndef __RSC

#include <svtools/ehdl.hxx>
#include "svx/svxdllapi.h"

class SVX_DLLPUBLIC SvxErrorHandler : private SfxErrorHandler
{
  public:
    SvxErrorHandler();
    static void Get();
};

#endif


#endif

