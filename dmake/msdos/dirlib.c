/*
            DIRLIB for MS-DOS
            -----------------

Enclosed is an implementation of the `dirlib' package for MS-DOS.
The implementation is targeted for MS-C, although any reasonably
competent C compiler should manage.  The package consists of:

    dir.h       the header file
    dir.c       the functions
    testdir.c   a q&d test program

The package tries to view directory naming in a Un*x light; in particular,
directories such as '/.' and '/..' (as well as `.' and `..' if your
current directory is root) are understood.   Indefinite paths like
`/../.././../..' will correctly refer to the root (of the particular disk).
Names such as `a:////./../' are okay too.

I've tried to be as sensible about DTA's as possible, since you never
know who will be using one; they are set before use, and reset afterwards.

There is some cruft in the package, namely the way `seekdir' and
`telldir' are done.  The code was derived from a little experimentation,
and may not work after a certain point (although I believe the 2.x version
to be solid).  Caveat utilitor.

Documentation for the package is available in the public domain; the
package's functionality was derived from this documentation.

Bug reports and comments are welcome.  Enjoy!

                - Matt

-------
UUCP:   {ucbvax,ihnp4,randvax,trwrb!trwspp,ism780}!ucla-cs!matt
ARPA:   matt@LOCUS.UCLA.EDU
Ph: (213) 825-2756

--------
Modified for use in dmake by Dennis Vadura.  Mostly just clean up and an
effort to make correctly typed objects are passed to functions in find.c.
Also deleted all dos version 2.0 specific code.  It is not required any
more.
*/

/*
 * revision history:
 *
 *  VER MM/DD/YY    COMMENTS
 *  ----    --------    --------
 *  0.99    02/24/86    Beta release to INTERNET
 */

#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <dos.h>

#include "dirent.h"

extern int find_err;


static char *
getdcwd(drive)
int drive;
{
   union REGS r;
   struct SREGS s;
   static char xcwd[64];
   char far *cwd = xcwd;

   r.h.ah = 0x47;
   r.h.dl = drive;
   r.x.si = FP_OFF(cwd);
   s.ds = FP_SEG(cwd);
   intdosx(&r, &r, &s);
   find_err = r.x.ax;
   if (r.x.cflag)
      return (char *) 0;
   return xcwd;
}



/*
 * opendir
 */

#define SUFFIX  "\\*.*"
#define SLASH   "\\"
#define streq(a,b)  (strcmp(a,b)==0)

DIR *
opendir(name)
char *name;
{
   register DIR *nd;
   char *cwd;
   char drive[3];
   int atroot = 0;
   int rooted = 0;

   /*
   * hack off drive designator if present
   */

   if (name[1] == ':') {
      cwd = getdcwd(toupper(name[0]) - 'A' + 1);
      drive[0] = name[0]; drive[1] = ':'; drive[2] = '\0';
      name += 2;
   }
   else {
      cwd = getdcwd(0);
      drive[0] = '\0';
   }

   /* is the name 'rooted'? */
   if ((*name == '/') || (*name == '\\')) ++rooted;

   /* see if we are at the root directory for this device */
   if (!*cwd) ++atroot;

   /*
   * MSDOS '/' doesn't have a '.' or '..'
   * also, double '/' sequences don't make sense.
   * many ported programs expect them to work, so we fix it up...
   */

   /* chop off leading . and .. if at root */
   if (atroot && (*name == '.')) {
      switch (*++name) {
     case '\0':
     case '/':
     case '\\':
        break;

     case '.':
        switch (*++name) {
           case '\0':
           case '/':
           case '\\':
            break;
           default:
            --name;
            --name;
        }
        break;

     default:
        --name;
      }
   }

   /* chop off leading /'s, /.'s and /..'s to make naming sensible */
   while (*name && ((*name == '/') || (*name == '\\'))) {
      if (*++name == '.') {
     switch (*++name) {
        case '\0':
        case '/':
        case '\\':
           break;

        case '.':
           switch (*++name) {
          case '\0':
          case '/':
          case '\\':
              break;

         default:
              --name;
              --name;
           }
           break;

        default:
           --name;
     }
      }
   }


   /*
   * name should now look like: path/path/path
   * we must now construct name based on whether or not it
   * was 'rooted' (started with a /)
   */

   if (rooted) cwd = "";

   /* construct DIR */
   if (!(nd = (DIR *)malloc(
      sizeof(DIR)+strlen(drive)+strlen(cwd)+strlen(SLASH)+
      strlen(name)+strlen(SUFFIX))))
      return (DIR *) 0;

   /* create long name */
   strcpy(nd->dd_name, drive);
   if (*cwd) {
      strcat(nd->dd_name, SLASH);
      strcat(nd->dd_name, cwd);
   }
   if (*name) {
      strcat(nd->dd_name, SLASH);
      strcat(nd->dd_name, name);
   }
   strcat(nd->dd_name, SUFFIX);

   /* search */
   if (!findfirst(&nd->dd_name[0], &nd->dd_dta)) {
      free((char *)nd);
      errno = ENOENT;
      return (DIR *) 0;
   }
   nd->dd_stat = 0;
   return nd;
}


struct dirent *
readdir(dirp)
DIR *dirp;
{
   static struct dirent dir;

   if (dirp->dd_stat)
      return (struct dirent *) 0;

   /* format structure */
   dir.d_ino = 0; /* not valid for DOS */
   dir.d_reclen = 0;
   strcpy(dir.d_name, dirp->dd_dta.name);
   dir.d_namlen = strlen(dir.d_name);
   strlwr(dir.d_name); /* DOSism */

   /* read ahead */
   if (findnext(&dirp->dd_dta) != NULL)
      dirp->dd_stat = 0;
   else
      dirp->dd_stat = find_err;

   return &dir;
}


void
closedir(dirp)
DIR *dirp;
{
   free((char *)dirp);
}


void
seekdir(dirp, pos)
DIR *dirp;
long pos;
{
   /*
   * check against DOS limits
   */

   if ((pos < 0) || (pos > 4095)) {
      dirp->dd_stat = 1;
      return;
   }

   *(short *)&dirp->dd_dta.fcb[13] = pos + 1;

   /* read ahead */
   if (findnext(&dirp->dd_dta))
      dirp->dd_stat = 0;
   else
      dirp->dd_stat = find_err;
}


long
telldir(dirp)
DIR *dirp;
{
   return (long) (*(short *)&dirp->dd_dta.fcb[13] - 2);
}
