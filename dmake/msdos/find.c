/*
    Directory Access Library

           FIND.C taken from DIRLIB.C by M. J. Weinstein
         Released to public domain 1-Jan-89

    The author may be contacted at:
    matt@cs.ucla.edu -or- POB 84524, L.A., CA  90073

    Modified by dvadura@watdragon.edu to work with dmake.
    (nuked the DOS version 2 code, since dmake needs version
    3.0 or greater to function).
 */


/*
 * revision history:
 *
 *  VER MM/DD/YY    COMMENTS
 *  ----    --------    --------
 *  0.99    02/24/86    Beta release to INTERNET
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <alloc.h>
#include <dos.h>
#include "dosdta.h"

#ifndef MK_FP
#define MK_FP(seg,ofs)  ((void far *) \
               (((unsigned long)(seg) << 16) | (unsigned)(ofs)))
#endif
#ifndef FP_SEG
#define FP_SEG(fp)  ((unsigned)((unsigned long)(fp) >> 16))
#endif
#ifndef FP_OFF
#define FP_OFF(fp)  ((unsigned)(fp))
#endif

int find_err;

/*
 * get/set dta address
 */

static DTA far *
_getsetdta(newdta)
DTA far *newdta;
{
    DTA far *olddta;
    union REGS r;
    struct SREGS s;

    /* get old dta */
    r.h.ah = 0x2f;
    intdos(&r, &r);
    segread(&s);
    olddta = (DTA far *) MK_FP(s.es, r.x.bx);

    /* conditionally set new dta */
    if (newdta) {
        r.h.ah = 0x1a;
        s.ds    = FP_SEG(newdta);
        r.x.dx  = FP_OFF(newdta);
        intdosx(&r, &r, &s);
    }

    return olddta;
}

/*
 * dos findfirst
 */

DTA *
findfirst(name, dta)
char *name;
DTA  *dta;
{
    union REGS r;
    struct SREGS s;
    DTA far *dtasave;
    char far *nmp = (char far *)name;

    dtasave = _getsetdta((DTA far *)dta);

    /* do directory lookup */
    segread(&s);
    r.h.ah  = 0x4e;
    r.x.cx  = 0x10;
    r.x.dx  = FP_OFF(nmp);
    s.ds    = FP_SEG(nmp);
    intdosx(&r, &r, &s);
    /* restore dta */
    _getsetdta(dtasave);
    find_err = r.x.ax;
    if (r.x.cflag)
        return(NULL);

    return dta;
}

/*
 * dos findnext
 */

DTA *
findnext(dta)
DTA *dta;
{
    union REGS r;
    DTA far *dtasave;

    dtasave = _getsetdta((DTA far *)dta);

    /* do directory lookup */
    r.h.ah = 0x4f;
    intdos(&r, &r);
    /* restore old dta */
    _getsetdta(dtasave);
    find_err = r.x.ax;
    if (r.x.cflag)
        return(NULL);

    return dta;
}
