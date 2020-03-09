salhelper_lib = shared_library('salhelperlo',
  'source/condition.cxx',
  'source/dynload.cxx',
  'source/simplereferenceobject.cxx',
  'source/thread.cxx',
  'source/timer.cxx',
  include_directories: [main_inc],
  cpp_args: ['-DLIBO_INTERNAL_ONLY',
             '-DSALHELPER_DLLIMPLEMENTATION'],
  link_with: sal_lib,
  gnu_symbol_visibility: 'hidden',
  install: true,
)

sh_test = shared_library('test_salhelper',
  'qa/test_api.cxx',
  include_directories: [main_inc],
  link_with: [salhelper_lib, sal_lib],
  dependencies: [cppunit_dep],
)

test('salhelper', unittester,
    args: [sh_test])
