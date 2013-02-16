AC_DEFUN([libo_PUBLISH_MODULE],
[
    true
    AC_SUBST([SYSTEM_$1])
    AC_SUBST([$1_CFLAGS])
    AC_SUBST([$1_LIBS])
])

AC_DEFUN([libo_CHECK_SYSTEM_MODULE], [
AC_MSG_CHECKING([which $1 to use])
if test "$with_system_$1" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_$2=YES
    PKG_CHECK_MODULES([$2], [$3])
else
    AC_MSG_RESULT([internal])
    SYSTEM_$2=NO
    BUILD_TYPE="$BUILD_TYPE $2"
fi
    libo_PUBLISH_MODULE([$2])
])

