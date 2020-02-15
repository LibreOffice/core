project('libreoffice', 'c', 'cpp',
  version: '1.2.3',
  default_options: ['cpp_std=c++17'])

varr = meson.project_version().split('.')
major_version = varr[0]
minor_version = varr[1]
micro_version = varr[2]
patch_version = '0'
this_year = '2020'

cpp = meson.get_compiler('cpp')

add_global_arguments('-DLINUX', language: 'cpp')
add_global_arguments('-DLINUX', language: 'c')
add_global_arguments('-DUNIX', language: 'cpp')
add_global_arguments('-DUNIX', language: 'c')
add_global_arguments('-DUNX', language: 'cpp')
add_global_arguments('-DUNX', language: 'c')
add_global_arguments('-DX86_64', language: 'cpp')
add_global_arguments('-DX86_64', language: 'c')
add_global_arguments('-DCPPU_ENV=gcc3', language: 'cpp')

lex_bin = find_program('flex')
bison_bin = find_program('bison')
xslt_bin = find_program('xsltproc')

thread_dep = dependency('threads')
dl_dep = cpp.find_library('dl', required: false)
cppunit_dep = dependency('cppunit')
main_inc = include_directories('include', 'config_host')
langtag_dep = dependency('liblangtag')
icuuc_dep = dependency('icu-uc')
icu18_dep = dependency('icu-i18n')
openssl_dep = dependency('openssl')
zlib_dep = dependency('zlib')
libxml_dep = dependency('libxml-2.0')
boost_locale_dep = dependency('boost', modules: 'locale')
dl_dep = cpp.find_library('dl', required: false)
harfbuzz_dep = dependency('harfbuzz')
harfbuzz_icu_dep = dependency('harfbuzz-icu')
glm_dep = dependency('glm')
cairo_dep = dependency('cairo')
eot_dep = dependency('libeot')
png_dep = dependency('libpng')
jpg_dep = dependency('libjpeg')
graphite_dep = dependency('graphite2')
lcms_dep = dependency('lcms2')
epoxy_dep = dependency('epoxy')
gtk3_dep = dependency('gtk+-3.0')
dbus_dep = dependency('dbus-1')
gio_dep = dependency('gio-2.0')
xext_dep = dependency('xext')
x11_dep = dependency('x11')
fc_dep = dependency('fontconfig')

# The version in Ubuntu eoan is too old.
#mdds_dep = dependency('mdds-1.5')
mdds_dep = declare_dependency(include_directories: 'mdds')

cdata = configuration_data()

# FIXME. Java does not provide .pc files.
# Presumably this data can be obtained via other methods.
jni_dep = declare_dependency(compile_args: ['-I/usr/lib/jvm/java-13-openjdk-amd64/include',
                                            '-I/usr/lib/jvm/java-13-openjdk-amd64/include/linux'])

subdir('config_host')
subdir('sal')
subdir('salhelper')
subdir('store')
subdir('registry')
subdir('idlc')
subdir('unoidl')
subdir('codemaker')
subdir('udkapi')
subdir('offapi')
subdir('i18nlangtag')
subdir('cppu')
subdir('xmlreader')
subdir('cppuhelper')
subdir('basegfx')
subdir('ucbhelper')
subdir('comphelper')
subdir('animations')
subdir('eventattacher')
subdir('tools')
subdir('remotebridges')
subdir('binaryurp')
subdir('officecfg')
subdir('unotools')
subdir('io')
subdir('jvmfwk')
subdir('i18nutil')
subdir('sot')
subdir('svl')
subdir('external')
subdir('opencl')
subdir('vcl')
