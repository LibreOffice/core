These are configuration files for various features as detected by configure.

Include only those files you need (in order to reduce rebuilds when a setting changes).

Settings here are only C/C++ #define directives, so they apply only to C/C++ source,
not to Makefiles.



Adding a new setting:
=====================

- do AC_DEFINE(HAVE_FOO) in configure.ac when a setting should be set
- choose the proper config_host/config_XXX.h file to use
    - if it is a global setting (such as availability of a compiler feature),
        use config_host/config_global.h
    - otherwise check if there is a matching config_host/config_XXX.h file
    - if none matches, add a new one:
        - add config_host/config_XXX.h.in here, with just #ifndef include guard
        - add AC_CONFIG_HEADERS([config_host/config_XXX.h]) next to the others
            in configure.ac
- add #define HAVE_FOO 0 to the config_host/config_XXX.h , possibly with a comment
  (do not use #undef HAVE_FOO, unless the setting has more values than on/off)
- add #include <config_XXX.h> before any #if HAVE_FOO in a source file
- make sure you use #if HAVE_FOO for on/off settings, do not use #ifdef
