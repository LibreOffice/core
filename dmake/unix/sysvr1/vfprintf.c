/* From:
 * John Limpert            johnl@gronk.UUCP        uunet!n3dmc!gronk!johnl
 */

#include <stdio.h>
#include <varargs.h>

#ifndef BUFSIZ
#include <stdio.h>
#endif

#ifndef va_dcl
#include <varargs.h>
#endif

int
vsprintf(str, fmt, ap)
    char *str, *fmt;
    va_list ap;
{
    FILE f;
    int len;

    f._flag = _IOWRT+_IOMYBUF;
    f._ptr = (char *)str;   /* My copy of BSD stdio.h has this as (char *)
                 * with a comment that it should be
                 * (unsigned char *).  Since this code is
                 * intended for use on a vanilla BSD system,
                 * we'll stick with (char *) for now.
                 */
    f._cnt = 32767;
    len = _doprnt(fmt, ap, &f);
    *f._ptr = 0;
    return (len);
}

int
vfprintf(iop, fmt, ap)
    FILE *iop;
    char *fmt;
    va_list ap;
{
    int len;

    len = _doprnt(fmt, ap, iop);
    return (ferror(iop) ? EOF : len);
}

int
vprintf(fmt, ap)
    char *fmt;
    va_list ap;
{
    int len;

    len = _doprnt(fmt, ap, stdout);
    return (ferror(stdout) ? EOF : len);
}
