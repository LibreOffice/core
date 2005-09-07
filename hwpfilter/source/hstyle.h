/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hstyle.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:38:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/* $Id: hstyle.h,v 1.2 2005-09-07 16:38:35 rt Exp $ */

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
