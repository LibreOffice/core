dnl aclocal.m4 generated automatically by aclocal 1.4

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.


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


