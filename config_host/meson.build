cdata.set('SAL_TYPES_SIZEOFSHORT', cpp.sizeof('short'))
cdata.set('SAL_TYPES_SIZEOFINT', cpp.sizeof('int'))
cdata.set('SAL_TYPES_SIZEOFLONG', cpp.sizeof('long'))
cdata.set('SAL_TYPES_SIZEOFLONGLONG', cpp.sizeof('long long'))
cdata.set('SAL_TYPES_SIZEOFPOINTER', cpp.sizeof('void*'))
# FIXME: don't know where these should be measured from.
cdata.set('SAL_TYPES_ALIGNMENT8', 8)

cdata.set10('HAVE_FEATURE_X11', true)
cdata.set10('HAVE_FEATURE_SCRIPTING', true)
cdata.set10('HAVE_FEATURE_MULTIUSER_ENVIRONMENT', true)
cdata.set10('HAVE_FEATURE_OPENGL', true)
cdata.set10('HAVE_FEATURE_UI', true)

cdata.set10('USE_SDT_PROBES', false)

# FIXME, these are probably wrong for distro installs.
cdata.set_quoted('LIBO_BIN_FOLDER', get_option('bindir'))
cdata.set_quoted('LIBO_ETC_FOLDER', get_option('bindir'))
cdata.set_quoted('LIBO_LIB_FOLDER', get_option('libdir'))
cdata.set_quoted('LIBO_LIBEXEC_FOLDER', get_option('libdir'))
cdata.set_quoted('LIBO_SHARE_FOLDER', get_option('datadir'))
cdata.set_quoted('LIBO_SHARE_HELP_FOLDER', 'help')
cdata.set_quoted('LIBO_SHARE_JAVA_FOLDER', get_option('bindir') / 'classes')
cdata.set_quoted('LIBO_SHARE_PRESETS_FOLDER', 'presets')
cdata.set_quoted('LIBO_SHARE_RESOURCE_FOLDER', get_option('bindir') / 'resource')
cdata.set_quoted('LIBO_SHARE_SHELL_FOLDER', get_option('bindir') / 'shell')

cdata.set_quoted('LIBO_URE_BIN_FOLDER', get_option('bindir'))
cdata.set_quoted('LIBO_URE_ETC_FOLDER', get_option('bindir'))
cdata.set_quoted('LIBO_URE_LIB_FOLDER', get_option('bindir'))
cdata.set_quoted('LIBO_URE_MISC_FOLDER', get_option('bindir'))
cdata.set_quoted('LIBO_URE_SHARE_JAVA_FOLDER', get_option('bindir') / 'classes')
cdata.set_quoted('SRC_ROOT', meson.source_root())
cdata.set_quoted('BUILDDIR', meson.build_root())

cdata.set10('HAVE_GCC_BUILTIN_ATOMIC', cpp.get_id() == 'gcc')
cdata.set10('HAVE_GCC_BUILTIN_FFS', cpp.get_id() == 'gcc')
cdata.set10('HAVE_GCC_PRAGMA_OPERATOR', cpp.get_id() == 'gcc')
cdata.set10('HAVE_GCC_DEPRECATED_MESSAGES', false)
cdata.set10('HAVE_SYSLOG_H', cpp.has_header('syslog.h'))

cdata.set10('STATIC_LINKING', false)
cdata.set10('ENABLE_RUNTIME_OPTIMIZATIONS', false)

cdata.set10('WITH_LOCALE_ALL', true)

cdata.set('LIBO_VERSION_MAJOR', major_version)
cdata.set('LIBO_VERSION_MINOR', minor_version)
cdata.set('LIBO_VERSION_MICRO', micro_version)
cdata.set('LIBO_VERSION_PATCH', patch_version)
cdata.set('LIBO_THIS_YEAR', this_year)

cdata.set10('HAVE_FEATURE_GPGME', false)

cdata.set10('LIBLANGTAG_INLINE_FIX', false)

cdata.set10('USE_TLS_OPENSSL', openssl_dep.found())
cdata.set10('USE_TLS_NSS', nss_dep.found())

cdata.set10('HAVE_LIBCXX', false)

cdata.set10('VCL_FLOAT_DEVICE_PIXEL', false) # ????

cdata.set10('ENABLE_CAIRO_CANVAS', cairo_dep.found())

cdata.set10('HAVE_FEATURE_DESKTOP', true)

cdata.set10('ENABLE_EOT', eot_dep.found())

cdata.set10('MPL_HAVE_SUBSET', false) # ?????

cdata.set10('HAVE_FEATURE_OPENCL', false)

cdata.set10('ENABLE_GTK3', gtk3_dep.found())

cdata.set10('ENABLE_DBUS', dbus_dep.found())
cdata.set10('DBUS_HAVE_GLIB', true) # ???

cdata.set10('ENABLE_GIO', gio_dep.found())

cdata.set_quoted('OOO_VENDOR', 'Vogon high comman')

cdata.set10('HAVE_FEATURE_EXTENSIONS', 1)
cdata.set10('ENABLE_EXTENSION_UPDATE', 1)

cdata.set10('HAVE_FEATURE_JAVA', false)

cdata.set_quoted('EXTRA_BUILDID', 'buildid')

cdata.set10('ENABLE_FORMULA_LOGGER', true)

cdata.set_quoted('PYTHON_VERSION_STRING', '3.2.1')

cdata.set10('ENABLE_FIREBIRD_SDBC', false)
cdata.set10('HAVE_FIREBIRD_30', false)

cdata.set10('HAVE_CXXABI_H_BASE_CLASS_TYPE_INFO', true)
cdata.set10('HAVE_CXXABI_H_CLASS_TYPE_INFO', true)
cdata.set10('HAVE_CXXABI_H_CXA_ALLOCATE_EXCEPTION', true)
cdata.set10('HAVE_CXXABI_H_CXA_EH_GLOBALS', false)
cdata.set10('HAVE_CXXABI_H_CXA_EXCEPTION', false)
cdata.set10('HAVE_CXXABI_H_CXA_GET_GLOBALS', true)
cdata.set10('HAVE_CXXABI_H_CXA_CURRENT_EXCEPTION_TYPE', true)
cdata.set10('HAVE_CXXABI_H_CXA_THROW', true)
cdata.set10('HAVE_CXXABI_H_SI_CLASS_TYPE_INFO', true)
cdata.set10('HAVE_CXXABI_H_VMI_CLASS_TYPE_INFO', true)

cdata.set10('HAVE_DCONF', false)

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

configure_file(input: 'config_version.h.meson',
  output: 'config_version.h',
  configuration: cdata)

configure_file(input: 'config_liblangtag.h.meson',
  output: 'config_liblangtag.h',
  configuration: cdata)

configure_file(input: 'config_gpgme.h.meson',
  output: 'config_gpgme.h',
  configuration: cdata)

configure_file(input: 'config_oox.h.meson',
  output: 'config_oox.h',
  configuration: cdata)

configure_file(input: 'config_libcxx.h.meson',
  output: 'config_libcxx.h',
  configuration: cdata)

configure_file(input: 'config_vcl.h.meson',
  output: 'config_vcl.h',
  configuration: cdata)

configure_file(input: 'config_cairo_canvas.h.meson',
  output: 'config_cairo_canvas.h',
  configuration: cdata)

configure_file(input: 'config_feature_desktop.h.meson',
  output: 'config_feature_desktop.h',
  configuration: cdata)

configure_file(input: 'config_eot.h.meson',
  output: 'config_eot.h',
  configuration: cdata)

configure_file(input: 'config_mpl.h.meson',
  output: 'config_mpl.h',
  configuration: cdata)

configure_file(input: 'config_feature_opencl.h.meson',
  output: 'config_feature_opencl.h',
  configuration: cdata)

configure_file(input: 'config_vclplug.h.meson',
  output: 'config_vclplug.h',
  configuration: cdata)

configure_file(input: 'config_dbus.h.meson',
  output: 'config_dbus.h',
  configuration: cdata)

configure_file(input: 'config_gio.h.meson',
  output: 'config_gio.h',
  configuration: cdata)

configure_file(input: 'config_oauth2.h.meson',
  output: 'config_oauth2.h',
  configuration: cdata)

configure_file(input: 'config_langs.h.meson',
  output: 'config_langs.h',
  configuration: cdata)

configure_file(input: 'config_vendor.h.meson',
  output: 'config_vendor.h',
  configuration: cdata)

configure_file(input: 'config_extensions.h.meson',
  output: 'config_extensions.h',
  configuration: cdata)

configure_file(input: 'config_java.h.meson',
  output: 'config_java.h',
  configuration: cdata)

configure_file(input: 'config_buildid.h.meson',
  output: 'config_buildid.h',
  configuration: cdata)

configure_file(input: 'config_fuzzers.h.meson',
  output: 'config_fuzzers.h',
  configuration: cdata)

configure_file(input: 'config_options_calc.h.meson',
  output: 'config_options_calc.h',
  configuration: cdata)

configure_file(input: 'config_lgpl.h.meson',
  output: 'config_lgpl.h',
  configuration: cdata)

configure_file(input: 'config_python.h.meson',
  output: 'config_python.h',
  configuration: cdata)

configure_file(input: 'config_firebird.h.meson',
  output: 'config_firebird.h',
  configuration: cdata)

configure_file(input: 'config_cxxabi.h.meson',
  output: 'config_cxxabi.h',
  configuration: cdata)

configure_file(input: 'config_dconf.h.meson',
  output: 'config_dconf.h',
  configuration: cdata)

configure_file(input: 'config_skia.h.in',
  output: 'config_skia.h',
  configuration: cdata)
