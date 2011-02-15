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


#ifndef APPLICAT_HXX
#define APPLICAT_HXX

class SvxErrorHandler;

#include <sfx2/app.hxx>

/**************************************************************************/
/*
**
**  MACRO DEFINITION
**
**/

#define SMDLL   1

#define APPLICATIONNAME      "smath3"

/**************************************************************************/
/*
**
**  CLASS DEFINITION
**
**/

#ifdef PM2
#define RELEASE     "PM304"
#endif

#ifdef WNT
#define RELEASE     "WNT304"
#endif

#ifdef UNX
#define RELEASE     "UNX304"
#endif

#ifndef SMDLL
class SmResId : public ResId
{
public:
    SmResId(sal_uInt16 nId) :
        ResId(nId)
    {
    }

};

#endif

#endif

