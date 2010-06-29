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

/* $Id: hstyle.h,v 1.3 2008-04-10 12:07:29 rt Exp $ */

#ifndef _HWPSTYLE_H_
#define _HWPSTYLE_H_

#include <stdlib.h>
#include <string.h>

#include <hwplib.h>
#include <hinfo.h>
/**
 * @short Using for global style object like "Standard"
 */
class DLLEXPORT HWPStyle
{
    short nstyles;
    void *style;
    public:
        HWPStyle( void );
        ~HWPStyle( void );

        int Num( void ) const;
        char *GetName( int n ) const;
        CharShape *GetCharShape( int n ) const;
        ParaShape *GetParaShape( int n ) const;

        void SetName( int n, char *name );
        void SetCharShape( int n, CharShape *cshapep );
        void SetParaShape( int n, ParaShape *pshapep );

        bool Read( HWPFile &hwpf );
};
#endif
/* _HWPSTYLE+H_ */
