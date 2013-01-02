dnl Version: MPL 1.1 / GPLv3+ / LGPLv3+
dnl
dnl The contents of this file are subject to the Mozilla Public License Version
dnl 1.1 (the "License"); you may not use this file except in compliance with
dnl the License or as specified alternatively below. You may obtain a copy of
dnl the License at http://www.mozilla.org/MPL/
dnl
dnl Software distributed under the License is distributed on an "AS IS" basis,
dnl WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
dnl for the specific language governing rights and limitations under the
dnl License.
dnl
dnl Major Contributor(s):
dnl Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
dnl  (initial developer)
dnl
dnl All Rights Reserved.
dnl
dnl For minor contributions see the git repository.
dnl
dnl Alternatively, the contents of this file may be used under the terms of
dnl either the GNU General Public License Version 3 or later (the "GPLv3+"), or
dnl the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
dnl in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
dnl instead of those above.

# libo_MINGW_CHECK_DLL(variable-infix,dll-name-stem,[action-if-found],[action-if-not-found])
#
# Checks for presence of dll dll-name-stem . Sets variable
# MINGW_variable-infix_DLL if found, issues an error otherwise.
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
# libo_MINGW_CHECK_DLL([EXPAT], [libexpat])
# might result in MINGW_EXPAT_DLL=libexpat-1.dll being set.
#
# uses CC, WITH_MINGW
# ------------------------------------------------------------------------------------------
AC_DEFUN([libo_MINGW_CHECK_DLL],
[AC_ARG_VAR([MINGW_][$1][_DLL],[output variable containing the found dll name])dnl

if test -n "$WITH_MINGW"; then
    _libo_mingw_dlldir="[$MINGW_SYSROOT]"/bin
    _libo_mingw_dllname=
    AC_MSG_CHECKING([for $2 dll])

    dnl try one- or two-numbered version
    _libo_mingw_try_dll([$2][-?.dll])
    if test "$_libo_mingw_dllname" = ""; then
        _libo_mingw_try_dll([$2][-??.dll])
    fi
    dnl maybe the version contains a dot (e.g., libdb)
    if test "$_libo_mingw_dllname" = ""; then
        _libo_mingw_try_dll([$2][-?.?.dll])
    fi
    dnl maybe the version contains a dash (e.g., libpixman)
    if test "$_libo_mingw_dllname" = ""; then
        _libo_mingw_try_dll([$2][-?-?.dll])
    fi
    dnl maybe it is not versioned
    if test "$_libo_mingw_dllname" = ""; then
        _libo_mingw_try_dll([$2][.dll])
    fi

    if test "$_libo_mingw_dllname" = ""; then
        AC_MSG_RESULT([no])
        m4_default([$4],[AC_MSG_ERROR([no dll found for $2])])
    else
        AC_MSG_RESULT([$_libo_mingw_dllname])
        [MINGW_][$1][_DLL]="$_libo_mingw_dllname"
        m4_default([$3],[])
    fi
fi[]dnl
]) # libo_MINGW_CHECK_DLL

# libo_MINGW_TRY_DLL(variable-infix,dll-name-stem)
#
# Checks for presence of dll dll-name-stem . Sets variable
# MINGW_variable-infix_DLL if found, does nothing otherwise.
#
# See libo_MINGW_CHECK_DLL for further info.
#
# uses CC, WITH_MINGW
# ------------------------------------------------
AC_DEFUN([libo_MINGW_TRY_DLL],
[dnl shortcut: do not test for already found dlls
if test -z "$[MINGW_][$1][_DLL]"; then
    libo_MINGW_CHECK_DLL([$1],[$2],[[]],[[]])
fi[]dnl
]) # libo_MINGW_TRY_DLL

# _libo_mingw_try_dll(dll-name,dll-dir)
m4_define([_libo_mingw_try_dll],
[_libo_mingw_trying_dll=`ls "[$_libo_mingw_dlldir]"/[$1] 2>/dev/null`
if test -f "$_libo_mingw_trying_dll"; then
    _libo_mingw_dllname=`basename "$_libo_mingw_trying_dll"`
fi[]dnl
]) # _libo_mingw_try_dll

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
