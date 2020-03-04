smoketest_lib = shared_library('smoketest',
  'smoketest.cxx',
  udk_fake_h, off_fake_h,
  cpp_args: ['-DLIBO_INTERNAL_ONLY',
             '-DCPPUNIT_PLUGIN_EXPORT=extern "C" SAL_DLLPUBLIC_EXPORT'],
  include_directories: [main_inc, udkh_inc, offapi_inc],
  link_with: [cppu_lib, cppuhelper_lib, sal_lib, unotest_lib],
  dependencies: [cppunit_dep],
  install: true,
)

libtest_bin = executable('libtest',
  'libtest.cxx',
  udk_fake_h, off_fake_h,
  include_directories: [main_inc, udkh_inc, offapi_inc],
  dependencies: [dl_dep],
  install: true,
)
