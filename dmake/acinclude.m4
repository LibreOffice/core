##   -*- autoconf -*-

dnl set the dmake root
AC_DEFUN(AC_SET_DMAKEROOT,
[
if test "${prefix}" != NONE; then
	DMAKEROOT=${prefix}/share/startup/startup.mk
	export DMAKEROOT
else
	DMAKEROOT=startup.mk
	export DMAKEROOT
fi
])

