/*************************************************************************
 *
 *  $RCSfile: svxerr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVXERR_HXX
#define _SVXERR_HXX

// include ---------------------------------------------------------------

#ifndef _ERRCODE_HXX
#include <tools/errcode.hxx>
#endif

// define ----------------------------------------------------------------

#define ERRCODE_SVX_LINGU_THESAURUSNOTEXISTS   (1UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_NOTEXISTS)
#define ERRCODE_SVX_LINGU_LANGUAGENOTEXISTS    (2UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_NOTEXISTS)
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
#define ERRCODE_SVX_GRAPHIC_WRONG_FILEFORMAT   (8UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_READ )
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

// both codes will be used twice : with ERRCODE_CLASS_READ- and ERRCODE_CLASS_WRITE-bits
#define ERRCTX_SVX_LINGU_THESAURUS              1
#define ERRCTX_SVX_LINGU_SPELLING               2
#define ERRCTX_SVX_LINGU_HYPHENATION            3
#define ERRCTX_SVX_LINGU_DICTIONARY             4
#define ERRCTX_SVX_BACKGROUND                   5
#define ERRCTX_SVX_IMPORT_GRAPHIC               6

// class SvxErrorHandler -------------------------------------------------

#ifndef __RSC

#ifndef _EHDL_HXX //autogen
#include <svtools/ehdl.hxx>
#endif

class SvxErrorHandler : private SfxErrorHandler
{
  public:
    SvxErrorHandler();
};

#endif


#endif

