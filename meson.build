project('libreoffice', 'c', 'cpp',
  version: '1.2.3',
  default_options: ['cpp_std=c++17'])

varr = meson.project_version().split('.')
major_version = varr[0]
minor_version = varr[1]
micro_version = varr[2]
patch_version = '0'
this_year = '2020'
main_inc = include_directories('include', 'config_host')
cpp = meson.get_compiler('cpp')
thread_dep = dependency('threads')
dl_dep = cpp.find_library('dl', required: false)
cdata = configuration_data()
osx_frameworks_dep = []

if host_machine.system() == 'linux'
  add_global_arguments('-DLINUX', language: 'cpp')
  add_global_arguments('-DLINUX', language: 'c')
  add_global_arguments('-DUNIX', language: 'cpp')
  add_global_arguments('-DUNIX', language: 'c')
  add_global_arguments('-DUNX', language: 'cpp')
  add_global_arguments('-DUNX', language: 'c')
  add_global_arguments('-DX86_64', language: 'cpp')
  add_global_arguments('-DX86_64', language: 'c')
  add_global_arguments('-DCPPU_ENV=gcc3', language: 'cpp')
  rtl_os = 'linux'
elif host_machine.system() == 'windows'
  # HACK: just random values thrown in to make
  # sal compile.
  add_global_arguments('-DWNT', language: 'cpp')
  add_global_arguments('-DWNT', language: 'c')
  add_global_arguments('-DX86_64', language: 'cpp')
  add_global_arguments('-DX86_64', language: 'c')
  add_global_arguments('-DNOMINMAX', language: 'cpp')
  #add_global_arguments('-DCPPU_ENV=gcc3', language: 'cpp')
  rtl_os = 'nt'
  cppunit_dep = dependency('', required: false)
  openssl_dep = cppunit_dep
  nss_dep = cppunit_dep
  cairo_dep = cppunit_dep
  eot_dep = cppunit_dep
  gtk3_dep = cppunit_dep
  dbus_dep = cppunit_dep
  gio_dep = cppunit_dep
  subdir('config_host')
  subdir('sal')
  subdir_done()
elif host_machine.system() == 'darwin'
  add_languages('objcpp')
  osx_frameworks_dep = [dependency('Carbon', method: 'extraframework'),
                        dependency('CoreFoundation', method: 'extraframework'),
                        dependency('Foundation', method: 'extraframework')]

  # HACK: just random values thrown in to make
  # sal compile. OSX only.
  add_global_arguments('-DMACOSX', language: 'objcpp')
  add_global_arguments('-DMACOSX', language: 'cpp')
  add_global_arguments('-DMACOSX', language: 'c')
  add_global_arguments('-DUNIX', language: 'objcpp')
  add_global_arguments('-DUNIX', language: 'cpp')
  add_global_arguments('-DUNIX', language: 'c')
  add_global_arguments('-DUNX', language: 'objccpp')
  add_global_arguments('-DUNX', language: 'cpp')
  add_global_arguments('-DUNX', language: 'c')
  add_global_arguments('-DX86_64', language: 'objcpp')
  add_global_arguments('-DX86_64', language: 'cpp')
  add_global_arguments('-DX86_64', language: 'c')
  #add_global_arguments('-DCPPU_ENV=gcc3', language: 'cpp')
  rtl_os = 'macosx'
  cppunit_dep = dependency('', required: false)
  openssl_dep = cppunit_dep
  nss_dep = cppunit_dep
  cairo_dep = cppunit_dep
  eot_dep = cppunit_dep
  gtk3_dep = cppunit_dep
  dbus_dep = cppunit_dep
  gio_dep = cppunit_dep
  subdir('config_host')
  subdir('sal')
  subdir_done()
else
  error('Unsupported OS.')
endif

lex_bin = find_program('flex')
bison_bin = find_program('bison')
xslt_bin = find_program('xsltproc')
gentokens_bin = find_program('solenv/bin/generate-tokens.py')

cppunit_dep = dependency('cppunit')
langtag_dep = dependency('liblangtag')
icuuc_dep = dependency('icu-uc')
icu18_dep = dependency('icu-i18n')
openssl_dep = dependency('', required: false) #dependency('openssl')
nss_dep = dependency('nss')
zlib_dep = dependency('zlib')
libxml_dep = dependency('libxml-2.0')
expat_dep = dependency('expat')
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
gtkprint_dep = dependency('gtk+-unix-print-3.0')
dbus_dep = dependency('dbus-1')
gio_dep = dependency('gio-2.0')
xext_dep = dependency('xext')
x11_dep = dependency('x11')
fc_dep = dependency('fontconfig')
cairo_dep = dependency('cairo')
orcus_dep = dependency('liborcus-0.14')
clucene_dep = dependency('libclucene-core')
libxslt_dep = dependency('libxslt')
qrcodegen_dep = declare_dependency(compile_args: ['-I/usr/include/qrcodegen'],
  dependencies: cpp.find_library('qrcodegencpp'))
revenge_dep = dependency('librevenge-0.0')
odfgen_dep = dependency('libodfgen-0.1')
epubgen_dep = dependency('libepubgen-0.1')
staroffice_dep = dependency('libstaroffice-0.0')
abw_dep = dependency('libabw-0.1')
mwaw_dep = dependency('libmwaw-0.3')
etonyek_dep = dependency('libetonyek-0.1')
wps_dep = dependency('libwps-0.4')
wpd_dep = dependency('libwpd-0.10')
wpg_dep = dependency('libwpg-0.3')
libebook_dep = dependency('libe-book-0.1')

# The version in Ubuntu eoan is too old.
#mdds_dep = dependency('mdds-1.5')
mdds_dep = declare_dependency(include_directories: 'mdds')
broken_mdds_dep = declare_dependency(include_directories: '.')

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
subdir('oovbaapi')
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
subdir('idl')
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
subdir('xmlscript')
subdir('toolkit')
subdir('svtools')
subdir('basic')
subdir('canvas')
subdir('sax')
subdir('cppcanvas')
subdir('framework')
subdir('drawinglayer')
subdir('sfx2')
subdir('xmloff')
subdir('linguistic')
subdir('editeng')
subdir('avmedia')
subdir('connectivity')
subdir('svx')
subdir('package')
subdir('filter')
subdir('oox')
subdir('vbahelper')
subdir('sw')
subdir('helpcompiler')
subdir('desktop')
subdir('starmath')
subdir('cui')
subdir('sd')
subdir('formula')
subdir('sc')
subdir('UnoControls')
subdir('embeddedobj')
subdir('fpicker')
subdir('accessibility')
subdir('lotuswordpro')
subdir('uui')
subdir('basctl')
subdir('chart2')
subdir('forms')
subdir('writerperfect')
subdir('emfio')
