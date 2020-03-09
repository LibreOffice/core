subdir('unotypes')

temp_hack = static_library('cppuuno',
  'source/exc_thrower.cxx',
  udk_fake_h, off_fake_h, cppuhelper_fake_h,
  include_directories: [main_inc, udkh_inc, offapi_inc, cpphelper_uno_inc],
)

cppuhelper_lib = shared_library('cppuhelperlo',
  'source/access_control.cxx',
  'source/bootstrap.cxx',
  'source/compat.cxx',
  'source/component_context.cxx',
  'source/component.cxx',
  'source/defaultbootstrap.cxx',
  'source/factory.cxx',
  'source/implbase.cxx',
  'source/implbase_ex.cxx',
  'source/implementationentry.cxx',
  'source/interfacecontainer.cxx',
  'source/macro_expander.cxx',
  'source/paths.cxx',
  'source/propertysetmixin.cxx',
  'source/propshlp.cxx',
  'source/servicemanager.cxx',
  'source/shlib.cxx',
  'source/supportsservice.cxx',
  'source/tdmgr.cxx',
  'source/typemanager.cxx',
  'source/typeprovider.cxx',
  'source/unourl.cxx',
  'source/weak.cxx',
  'source/findsofficepath.c',
  udk_fake_h, off_fake_h, cppuhelper_fake_h,
  include_directories: [main_inc, udkh_inc, offapi_inc, cpphelper_uno_inc],
  cpp_args: ['-DCPPUHELPER_DLLIMPLEMENTATION', '-DLIBO_INTERNAL_ONLY'],
  link_with: [temp_hack, cppu_lib, reg_lib, salhelper_lib, sal_lib, unoidl_lib, xmlreader_lib],
  gnu_symbol_visibility: 'hidden',
  install: true,
)

test('ifcontainer', unittester,
  args: [shared_library('test_ifcontainer',
      'qa/ifcontainer/cppu_ifcontainer.cxx',
      udk_fake_h,
      include_directories: [main_inc, udkh_inc],
      cpp_args: ['-DLIBO_INTERNAL_ONLY'],
      link_with: [cppu_lib, cppuhelper_lib, sal_lib],
      dependencies: [cppunit_dep],
    )
  ]
)

test('unourl', unittester,
  args: [shared_library('test_unourl',
      'qa/unourl/cppu_unourl.cxx',
      udk_fake_h,
      include_directories: [main_inc, udkh_inc],
      cpp_args: ['-DLIBO_INTERNAL_ONLY'],
      link_with: [cppu_lib, cppuhelper_lib, sal_lib],
      dependencies: [cppunit_dep],
    )
  ]
)

test('cppumisc', unittester,
  args: [shared_library('test_misc',
      'qa/misc/test_misc.cxx',
      udk_fake_h,
      include_directories: [main_inc, udkh_inc],
      cpp_args: ['-DLIBO_INTERNAL_ONLY'],
      link_with: [cppu_lib, cppuhelper_lib, sal_lib],
      install: true,
      dependencies: [cppunit_dep],
    )
  ]
)

test('cppuweak', unittester,
  args: [shared_library('test_cppuweak',
      'qa/weak/test_weak.cxx',
      udk_fake_h,
      include_directories: [main_inc, udkh_inc],
      cpp_args: ['-DLIBO_INTERNAL_ONLY'],
      link_with: [cppu_lib, cppuhelper_lib, sal_lib],
      dependencies: [cppunit_dep],
    )
  ]
)
