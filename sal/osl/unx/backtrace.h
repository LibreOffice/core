/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: backtrace.h,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2006-02-09 17:05:59 $
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


#if defined (SOLARIS) || (FREEBSD)

#ifdef __cplusplus
extern "C" {
#endif

/* backtrace function with same behaviour as defined in GNU libc */

int backtrace( void **buffer, int max_frames );

void backtrace_symbols_fd( void **buffer, int size, int fd );

/* no frame.h on FreeBSD */
#if defined FREEBSD
struct frame {
    long    arg0[8];
    long    arg1[6];
    struct frame *fr_savfp;
    long    fr_savpc;
};
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* defined SOLARIS || FREEBSD */

#if defined (LINUX) && defined (SPARC)
#ifdef __cplusplus
extern "C" {
#endif

/* backtrace function with same behaviour as defined in GNU libc */

int backtrace( void **buffer, int max_frames );

void backtrace_symbols_fd( void **buffer, int size, int fd );

/* no frame.h on linux sparc */
struct frame {
    long    arg0[8];
    long    arg1[6];
    struct frame *fr_savfp;
    long    fr_savpc;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* defined LINUX && SPARC */
