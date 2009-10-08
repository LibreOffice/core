dnl @synopsis AX_FUNC_WHICH_GETSPNAM_R
dnl
dnl Determines which historical variant of the getspnam_r() call
dnl (taking four or five arguments) is available on the system
dnl and sets NEW_SHADOW_API=YES if there are five arguments.
dnl
dnl Originally named "AX_FUNC_WHICH_GETHOSTBYNAME_R". Rewritten
dnl for AX_FUNC_WHICH_GETSPNAM_R
dnl
dnl @author Caolan McNamara <caolan@skynet.ie>
dnl @author Daniel Richard G. <skunk@iskunk.org>
dnl @version 2006-05-01
dnl @license LGPL

AC_DEFUN([AX_FUNC_WHICH_GETSPNAM_R], [

    AC_LANG_PUSH(C)
    AC_MSG_CHECKING([how many arguments getspnam_r() takes])

    AC_CACHE_VAL(ac_cv_func_which_getspnam_r, [

################################################################

ac_cv_func_which_getspnam_r=unknown

#
# ONE ARGUMENT (sanity check)
#

# This should fail, as there is no variant of getspnam_r() that takes
# a single argument. If it actually compiles, then we can assume that
# netdb.h is not declaring the function, and the compiler is thereby
# assuming an implicit prototype. In which case, we're out of luck.
#
AC_COMPILE_IFELSE(
    AC_LANG_PROGRAM(
	[[
	    #include <sys/types.h>
	    #include <shadow.h>
	]],
	[[
	    const char *name = "myname";
	    getspnam_r(name) /* ; */
	]]),
    ac_cv_func_which_getspnam_r=no)

#
# FIVE ARGUMENTS
#

if test "$ac_cv_func_which_getspnam_r" = "unknown"; then

AC_COMPILE_IFELSE(
    AC_LANG_PROGRAM(
	[[
	    #include <sys/types.h>
	    #include <shadow.h>
	]],
	[[
	    char buffer[[]] = { '\0' };
	    struct spwd spwdStruct;
	    const char *name = "myname";
	    getspnam_r(name, &spwdStruct, buffer, sizeof buffer, 0) /* ; */
	]]),
    ac_cv_func_which_getspnam_r=five)

fi

#
# FOUR ARGUMENTS
#

if test "$ac_cv_func_which_getspnam_r" = "unknown"; then

AC_COMPILE_IFELSE(
    AC_LANG_PROGRAM(
	[[
	    #include <sys/types.h>
	    #include <shadow.h>
	]],
	[[
	    char buffer[[]] = { '\0' };
	    struct spwd spwdStruct;
	    const char *name = "myname";
	    getspnam_r(name, &spwdStruct, buffer, sizeof buffer) /* ; */
	]]),
    ac_cv_func_which_getspnam_r=four)

fi

################################################################

]) dnl end AC_CACHE_VAL

case "$ac_cv_func_which_getspnam_r" in
    five)
    AC_MSG_RESULT([five])
    NEW_SHADOW_API=YES
    ;;

    four)
    AC_MSG_RESULT([four])
    ;;

    no)
    AC_MSG_RESULT([cannot find function declaration in shadow.h])
    ;;

    unknown)
    AC_MSG_RESULT([can't tell])
    ;;

    *)
    AC_MSG_ERROR([internal error])
    ;;
esac

AC_LANG_POP(C)

]) dnl end AC_DEFUN

dnl PKG_CHECK_MODULES_MACHACK is like PKG_CHECK_MODULES but sneaks in an extra
dnl argument between the first and second, denoting a program to call instead of
dnl pkg-config on Mac OS X (aka Darwin):
AC_DEFUN([PKG_CHECK_MODULES_MACHACK],
[if test "$_os" = "Darwin"; then
   AC_MSG_CHECKING($1_CFLAGS)
   $1_CFLAGS=`$2 --cflags`
   AC_MSG_RESULT($$1_CFLAGS)
   AC_MSG_CHECKING($1_LIBS)
   $1_LIBS=`$2 --libs`
   AC_MSG_RESULT($$1_LIBS)
   AC_SUBST($1_CFLAGS)
   AC_SUBST($1_LIBS)
 else
   PKG_CHECK_MODULES($1, $3, $4, $5, $6)
 fi
])
