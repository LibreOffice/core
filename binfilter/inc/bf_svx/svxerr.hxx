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
#ifndef _SVXERR_HXX
#define _SVXERR_HXX

// include ---------------------------------------------------------------


// define ----------------------------------------------------------------




#define ERRCODE_SVX_GRAPHIC_WRONG_FILEFORMAT   (8UL | ERRCODE_AREA_SVX | \
                                                ERRCODE_CLASS_READ )
#define ERRCODE_SVX_FORMS_NOIOSERVICES		   (10UL | ERRCODE_AREA_SVX )
#define ERRCODE_SVX_FORMS_READWRITEFAILED	   (11UL | ERRCODE_AREA_SVX )

#define ERRCODE_SVX_BULLETITEM_NOBULLET        (12UL | ERRCODE_AREA_SVX )


#define ERRCODE_SVX_VBASIC_STORAGE_EXIST       (14UL | ERRCODE_AREA_SVX     \
                                                     | ERRCODE_WARNING_MASK \
                                                     | ERRCODE_CLASS_WRITE )

// both codes will be used twice : with ERRCODE_CLASS_READ- and ERRCODE_CLASS_WRITE-bits

// class SvxErrorHandler -------------------------------------------------

#ifndef __RSC

#ifndef _EHDL_HXX //autogen
#include <bf_svtools/ehdl.hxx>
#endif
namespace binfilter {
class SvxErrorHandler : private SfxErrorHandler
{
  public:
    SvxErrorHandler();
};

}//end of namespace binfilter
#endif

#endif

