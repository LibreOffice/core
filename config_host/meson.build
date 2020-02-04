cdata.set('SAL_TYPES_SIZEOFSHORT', cpp.sizeof('short'))
cdata.set('SAL_TYPES_SIZEOFINT', cpp.sizeof('int'))
cdata.set('SAL_TYPES_SIZEOFLONG', cpp.sizeof('long'))
cdata.set('SAL_TYPES_SIZEOFLONGLONG', cpp.sizeof('long long'))
cdata.set('SAL_TYPES_SIZEOFPOINTER', cpp.sizeof('void*'))

cdata.set10('HAVE_FEATURE_X11', true)
cdata.set10('HAVE_FEATURE_SCRIPTING', true)
cdata.set10('HAVE_FEATURE_MULTIUSER_ENVIRONMENT', true)
cdata.set10('HAVE_FEATURE_OPENGL', true)
cdata.set10('HAVE_FEATURE_UI', true)

cdata.set10('USE_SDT_PROBES', false)

cdata.set_quoted('LIBO_BIN_FOLDER', get_option('prefix') / get_option('bindir'))
cdata.set_quoted('LIBO_ETC_FOLDER', 'FIXME')
cdata.set_quoted('LIBO_LIB_FOLDER', get_option('prefix') / get_option('libdir'))
cdata.set_quoted('LIBO_SHARE_FOLDER', get_option('prefix') / get_option('datadir'))
cdata.set_quoted('LIBO_SHARE_HELP_FOLDER', 'FIXME')
cdata.set_quoted('LIBO_SHARE_JAVA_FOLDER', 'FIXME')
cdata.set_quoted('LIBO_SHARE_PRESETS_FOLDER', 'FIXME')
cdata.set_quoted('LIBO_SHARE_RESOURCE_FOLDER', 'FIXME')
cdata.set_quoted('LIBO_SHARE_SHELL_FOLDER', 'FIXME')

cdata.set_quoted('LIBO_URE_BIN_FOLDER', 'FIXME')
cdata.set_quoted('LIBO_URE_ETC_FOLDER', 'FIXME')
cdata.set_quoted('LIBO_URE_LIB_FOLDER', 'FIXME')
cdata.set_quoted('LIBO_URE_MISC_FOLDER', 'FIXME')
cdata.set_quoted('LIBO_URE_SHARE_JAVA_FOLDER', 'FIXME')
cdata.set_quoted('SRC_ROOT', meson.source_root())
cdata.set_quoted('BUILDDIR', meson.build_root())

cdata.set10('HAVE_GCC_BUILTIN_ATOMIC', true)
cdata.set10('HAVE_GCC_BUILTIN_FFS', true)
cdata.set10('HAVE_GCC_PRAGMA_OPERATOR', true)
cdata.set10('HAVE_GCC_DEPRECATED_MESSAGES', false)
cdata.set10('HAVE_SYSLOG_H', cpp.has_header('syslog.h'))

cdata.set10('STATIC_LINKING', false)
cdata.set10('ENABLE_RUNTIME_OPTIMIZATIONS', false)

cdata.set10('WITH_LOCALE_ALL', true)

configure_file(input: 'config_typesizes.h.meson',
  output: 'config_typesizes.h',
  configuration: cdata)

configure_file(input: 'config_features.h.meson',
  output: 'config_features.h',
  configuration: cdata)

configure_file(input: 'config_probes.h.meson',
  output: 'config_probes.h',
  configuration: cdata)

configure_file(input: 'config_folders.h.meson',
  output: 'config_folders.h',
  configuration: cdata)

configure_file(input: 'config_global.h.meson',
  output: 'config_global.h',
  configuration: cdata)

configure_file(input: 'config_options.h.meson',
  output: 'config_options.h',
  configuration: cdata)

configure_file(input: 'config_locales.h.meson',
  output: 'config_locales.h',
  configuration: cdata)
