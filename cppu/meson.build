cppu_lib = shared_library('cppulo',
  'source/cppu/compat.cxx',
  'source/cppu/cppu_opt.cxx',
  'source/threadpool/current.cxx',
  'source/threadpool/jobqueue.cxx',
  'source/threadpool/thread.cxx',
  'source/threadpool/threadident.cxx',
  'source/threadpool/threadpool.cxx',
  'source/typelib/static_types.cxx',
  'source/typelib/typelib.cxx',
  'source/uno/any.cxx',
  'source/uno/cascade_mapping.cxx',
  'source/uno/check.cxx',
  'source/uno/data.cxx',
  'source/uno/EnvDcp.cxx',
  'source/uno/EnvStack.cxx',
  'source/uno/IdentityMapping.cxx',
  'source/uno/lbenv.cxx',
  'source/uno/lbmap.cxx',
  'source/uno/loadmodule.cxx',
  'source/uno/sequence.cxx',
  udk_fake_h,
  include_directories: [main_inc, udkh_inc],
  cpp_args: ['-DLIBO_INTERNAL_ONLY',
             '-DCPPU_DLLIMPLEMENTATION'],
  link_with: [salhelper_lib, sal_lib],
  gnu_symbol_visibility: 'hidden',
  install: true,
)

test_unotype = shared_library('test_unotype',
  'qa/test_unotype.cxx',
  udk_fake_h,
  cpp_args: ['-DLIBO_INTERNAL_ONLY'],
  include_directories: [main_inc, udkh_inc],
  link_with: [cppu_lib, sal_lib],
  dependencies: [cppunit_dep],
)

test('unotype', unittester,
  args: [test_unotype])

test_anyexternal = shared_library('test_anyexternal',
  'qa/any-external.cxx',
  udk_fake_h,
  include_directories: [main_inc, udkh_inc],
  link_with: [cppu_lib, sal_lib],
  dependencies: [cppunit_dep],
)

test('unotype', unittester,
  args: [test_anyexternal])

#test_cppumaker = shared_library('test_cppumaker',
#  'qa/cppumaker/test_cppumaker.cxx',
#  udk_fake_h,
#  include_directories: [main_inc, udkh_inc],
#  link_with: [cppu_lib, sal_lib],
#  dependencies: [cppunit_dep],
#)

#test('cppumaker', unittester,
#  args: [test_cppumaker])
