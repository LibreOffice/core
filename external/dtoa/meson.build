dtoa_st_lib = static_library('dtoa_static',
  'source/dtoa.cxx',
  include_directories: [
    '../../include',
    'source/inc',
    'source',
  ],
  cpp_args: [
    '-DIEEE_8087',
    '-D_PTHREADS',
    '-D_REENTRANT',
  ],
  dependencies: [thread_dep],
)

dtoa_dep = declare_dependency(link_with: dtoa_st_lib,
  include_directories: 'source/inc')

