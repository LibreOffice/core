/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sderror.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:59:28 $
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

#ifndef _SDSWERROR_H
#define _SDSWERROR_H

#ifndef _ERRCODE_HXX
#include <tools/errcode.hxx>
#endif

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

inline FASTBOOL IsWarning( ULONG nErr )
{
    return 0 != ( nErr & ERRCODE_WARNING_MASK & nErr );
}

inline FASTBOOL IsError( ULONG nErr )
{
    return nErr && 0 == ( ERRCODE_WARNING_MASK & nErr );
}

#endif


#endif
