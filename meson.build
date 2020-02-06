project('libreoffice', 'c', 'cpp',
  default_options: ['cpp_std=c++17'])

cpp = meson.get_compiler('cpp')

add_global_arguments('-DLINUX', language: 'cpp')
add_global_arguments('-DLINUX', language: 'c')
add_global_arguments('-DUNX', language: 'cpp')
add_global_arguments('-DUNX', language: 'c')

thread_dep = dependency('threads')
dl_dep = cpp.find_library('dl', required: false)
cppunit_dep = dependency('cppunit')
main_inc = include_directories('include', 'config_host')

cdata = configuration_data()

subdir('config_host')
subdir('sal')
subdir('salhelper')
subdir('store')
subdir('registry')
subdir('idlc')
