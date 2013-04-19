dnl
dnl This file is part of the LibreOffice project.
dnl
dnl This Source Code Form is subject to the terms of the Mozilla Public
dnl License, v. 2.0. If a copy of the MPL was not distributed with this
dnl file, You can obtain one at http://mozilla.org/MPL/2.0/.
dnl

# libo_MINGW_CHECK_DLL(dll-name-stem,[action-if-found],[action-if-not-found])
#
# Checks for presence of dll dll-name-stem . Appends the dll name to
# variable MINGW_DLLS if found, issues an error otherwise.
#
# It recognizes these dll patterns (x, y match any character, but they
# are supposed to be numerals):
# * name-x.dll
# * name-xy.dll
# * name-x.y.dll
# * name.dll
#
#
# Example:
# MINGW_DLLS=
# libo_MINGW_CHECK_DLL([libexpat])
# might result in MINGW_DLLS containing libexpat-1.dll .
#
# uses MINGW_SYSROOT, WITH_MINGW
# changes MINGW_DLLS
# ------------------------------------------------------------------------------------------
AC_DEFUN([libo_MINGW_CHECK_DLL],
[dnl

if test -n "$WITH_MINGW"; then
    _libo_mingw_dlldir="[$MINGW_SYSROOT]"/bin
    _libo_mingw_dllname=
    AC_MSG_CHECKING([for $1 dll])

    dnl try one- or two-numbered version
    _libo_mingw_try_dll([$1][-?.dll])
    if test "$_libo_mingw_dllname" = ""; then
        _libo_mingw_try_dll([$1][-??.dll])
    fi
    dnl maybe the version contains a dot (e.g., libdb)
    if test "$_libo_mingw_dllname" = ""; then
        _libo_mingw_try_dll([$1][-?.?.dll])
    fi
    dnl maybe the version contains a dash (e.g., libpixman)
    if test "$_libo_mingw_dllname" = ""; then
        _libo_mingw_try_dll([$1][-?-?.dll])
    fi
    dnl maybe it is not versioned
    if test "$_libo_mingw_dllname" = ""; then
        _libo_mingw_try_dll([$1][.dll])
    fi

    if test "$_libo_mingw_dllname" = ""; then
        AC_MSG_RESULT([no])
        m4_default([$3],[AC_MSG_ERROR([no dll found for $1])])
    else
        AC_MSG_RESULT([$_libo_mingw_dllname])
        MINGW_DLLS="$MINGW_DLLS $_libo_mingw_dllname"
        m4_default([$2],[])
    fi
fi[]dnl
]) # libo_MINGW_CHECK_DLL

# libo_MINGW_TRY_DLL(dll-name-stem)
#
# Checks for presence of dll dll-name-stem . Appends the dll name to
# variable MINGW_DLLS if found, does nothing otherwise.
#
# See libo_MINGW_CHECK_DLL for further info.
#
# uses MINGW_SYSROOT, WITH_MINGW
# changes MINGW_DLLS
# ------------------------------------------------
AC_DEFUN([libo_MINGW_TRY_DLL],
[dnl TODO: ignore already tested for dlls
libo_MINGW_CHECK_DLL([$1],[[]],[[]])dnl
]) # libo_MINGW_TRY_DLL

# _libo_mingw_try_dll(dll-name,dll-dir)
m4_define([_libo_mingw_try_dll],
[_libo_mingw_trying_dll=`ls "[$_libo_mingw_dlldir]"/[$1] 2>/dev/null`
if test -f "$_libo_mingw_trying_dll"; then
    _libo_mingw_dllname=`basename "$_libo_mingw_trying_dll"`
fi[]dnl
]) # _libo_mingw_try_dll

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
