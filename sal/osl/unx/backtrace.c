/*************************************************************************
 *
 *  $RCSfile: backtrace.c,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-16 17:20:28 $
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


#ifdef SOLARIS

#include <dlfcn.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdio.h>
#include <sys/frame.h>
#include "backtrace.h"

#if defined(SPARC)

#define FRAME_PTR_OFFSET 1
#define FRAME_OFFSET 0

#elif defined( INTEL )

#define FRAME_PTR_OFFSET 3
#define FRAME_OFFSET 1

#else

#error Unknown Solaris target platform.

#endif /* defined SPARC or INTEL */


int backtrace( void **buffer, int max_frames )
{
    struct frame *fp;
    jmp_buf ctx;
    int i;

    /* flush register windows */
#ifdef SPARC
    asm("ta 3");
#endif
    /* get stack- and framepointer */
    setjmp(ctx);
    fp = (struct frame*)(((size_t*)(ctx))[FRAME_PTR_OFFSET]);
    for ( i=0; (i<FRAME_OFFSET) && (fp!=0); i++)
        fp = fp->fr_savfp;

    /* iterate through backtrace */
    for (i=0; fp && fp->fr_savpc && i<max_frames; i++)
    {
        /* store frame */
        *(buffer++) = (void *)fp->fr_savpc;
        /* next frame */
        fp=fp->fr_savfp;
    }
    return i;
}

void backtrace_symbols_fd( void **buffer, int size, int fd )
{
    FILE    *fp = fdopen( fd, "w" );

    if ( fp )
    {
        void **pFramePtr;

        for ( pFramePtr = buffer; size > 0 && pFramePtr && *pFramePtr; pFramePtr++, size-- )
        {
            Dl_info     dli;
            ptrdiff_t   offset;

            if ( 0 != dladdr( *pFramePtr, &dli ) )
            {
                if ( dli.dli_fname && dli.dli_fbase )
                {
                    offset = (ptrdiff_t)*pFramePtr - (ptrdiff_t)dli.dli_fbase;
                    fprintf( fp, "%s+0x%x", dli.dli_fname, offset );
                }
                if ( dli.dli_sname && dli.dli_saddr )
                {
                    offset = (ptrdiff_t)*pFramePtr - (ptrdiff_t)dli.dli_saddr;
                    fprintf( fp, "(%s+0x%x)", dli.dli_sname, offset );
                }
            }
            fprintf( fp, "[0x%x]\n", *pFramePtr );
        }

        fflush( fp );
        fclose( fp );
    }
}

#endif /* defined SOLARIS */


#ifdef FREEBSD
#include <dlfcn.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdio.h>
#include "backtrace.h"

#define FRAME_PTR_OFFSET 1
#define FRAME_OFFSET 0

int backtrace( void **buffer, int max_frames )
{
    return 1;
}

void backtrace_symbols_fd( void **buffer, int size, int fd )
{

}
#endif /* defined FREEBSD */

#if defined(IRIX)
#include <stdio.h>
#include <rld_interface.h>
#include <exception.h>
#include <sys/signal.h>
#include <unistd.h>

/* Need extra libs -lexc -ldwarf -lelf */

int backtrace( void **buffer, int max_frames )
{
    struct sigcontext context;
    int i = 0;

    memset(&context, 0, sizeof(struct sigcontext));

    exc_setjmp(&context);
    while(context.sc_pc != 1 && i < max_frames) {
        exc_unwind(&context, 0);
        if(context.sc_pc != 1) {
            *(buffer++) = (void *)context.sc_pc;
            i++;
        }
    }
    return(i);
}

void backtrace_symbols_fd( void **buffer, int size, int fd )
{
    FILE    *fp = fdopen( fd, "w" );
    struct sigcontext context;
    char *name;

    if ( fp ) {
        while(context.sc_pc!=1) {
            if(context.sc_pc != 1) {
                exc_unwind_name(&context, 0, &name);
                fprintf(fp, " 0x%012lx %.100s\n", context.sc_pc, name ? name : "<unknown function>");
                free(name);
            }
        }

        fflush( fp );
        fclose( fp );
    }
}
#endif /* defined IRIX */

