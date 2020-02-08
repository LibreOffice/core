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
add_global_arguments('-DUNX', language: 'cpp')
add_global_arguments('-DUNX', language: 'c')
add_global_arguments('-DCPPU_ENV=gcc', language: 'cpp')

lex_bin = find_program('flex')
bison_bin = find_program('bison')

thread_dep = dependency('threads')
dl_dep = cpp.find_library('dl', required: false)
cppunit_dep = dependency('cppunit')
main_inc = include_directories('include', 'config_host')
langtag_dep = dependency('liblangtag')
icuuc_dep = dependency('icu-uc')

cdata = configuration_data()

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
