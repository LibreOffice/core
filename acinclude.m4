dnl @synopsis PKG_CHECK_MODULES_MACHACK

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
