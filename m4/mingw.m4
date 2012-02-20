# libo_FIND_MINGW_EXTERNAL_DLLS([library-names],[variable],[?exclude])
# uses: CC, SED, WITH_MINGW
# --------------------------------------------------------------------
AC_DEFUN([libo_FIND_MINGW_EXTERNAL_DLLS],
[if test "$WITH_MINGW" = yes -a -n "$CC"; then
    _libo_mingw_libdir=`$CC -print-sysroot`/mingw/lib
    for _libo_mingw_lib in $1; do
        _libo_mingw_lib="$_libo_mingw_libdir/$_libo_mingw_lib.la"
        _libo_mingw_find_dll([$_libo_mingw_lib],[$3],[_libo_mingw_new_dll])
        if test -n "$_libo_mingw_new_dll"; then
            _libo_mingw_new_dlls="$_libo_mingw_new_dll"
        fi

        _libo_mingw_get_libtool_var([dependency_libs],[$_libo_mingw_lib],[_libo_mingw_dep_libs])
        for _libo_mingw_dep_lib in $_libo_mingw_dep_libs; do
            if test "${_libo_mingw_dep_lib%.la}" != "$_libo_mingw_dep_lib"; then
                _libo_mingw_new_dll=''
                _libo_mingw_find_dll([$_libo_mingw_dep_lib],[$3],[_libo_mingw_new_dll])
                if test -n "$_libo_mingw_new_dll"; then
                    _libo_mingw_new_dlls="$_libo_mingw_new_dlls $_libo_mingw_new_dll"
                fi
            fi
        done
        $2="$_libo_mingw_new_dlls"
    done
fi[]dnl
]) # libo_FIND_MINGW_EXTERNAL_DLLS

# libo_ADD_MINGW_EXTERNAL_DLLS([library-names],[variable])
# uses: CC, SED, WITH_MINGW
# --------------------------------------------------------
AC_DEFUN([libo_ADD_MINGW_EXTERNAL_DLLS],
[libo_FIND_MINGW_EXTERNAL_DLLS([$1],[_libo_mingw_found_dlls],[$$2])
if test -n "$_libo_mingw_found_dlls"; then
    $2="$$2 $_libo_mingw_found_dlls"
fi[]dnl
]) # libo_ADD_MINGW_EXTERNAL_DLLS

# _libo_mingw_get_libtool_var([key],[lib],[out-var])
m4_define([_libo_mingw_get_libtool_var],
[$3=`$SED -n -e '/^$1=/{' -e "s/.*='//" -e "s/'//" -e p -e '}' $2`[]dnl
]) # _libo_mingw_get_libtool_var

# _libo_mingw_find_dll([library],[dlls],[out-var])
m4_define([_libo_mingw_find_dll],
[if test -f "$1"; then
    _libo_mingw_get_libtool_var([dlname],[$1],[_libo_mingw_dlname])
    _libo_mingw_dlname=`basename $_libo_mingw_dlname`
    _libo_mingw_dll_present=
    for _libo_mingw_dll in $2; do
        if test "$_libo_mingw_dlname" = "$_libo_mingw_dll"; then
            _libo_mingw_dll_present=yes
            break
        fi
    done
    if test -z "$_libo_mingw_dll_present"; then
        $3="$_libo_mingw_dlname"
    fi
fi[]dnl
]) # _libo_mingw_find_dll
