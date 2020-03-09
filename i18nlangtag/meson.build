i18nlangtag_lib = shared_library('i18nlangtaglo',
  'source/isolang/insys.cxx',
  'source/isolang/isolang.cxx',
  'source/isolang/mslangid.cxx',
  'source/languagetag/languagetag.cxx',
  'source/languagetag/languagetagicu.cxx',
  udk_fake_h, off_fake_h,
  include_directories: [main_inc, udkh_inc, offapi_inc],
  dependencies: [icuuc_dep, langtag_dep],
  cpp_args: ['-DLIBO_INTERNAL_ONLY',
             '-DI18NLANGTAG_DLLIMPLEMENTATION'],
  link_with: [sal_lib],
  gnu_symbol_visibility: 'hidden',
  install: true,
)

test_langtag = shared_library('test_languagetag',
  udk_fake_h, off_fake_h,
  'qa/cppunit/test_languagetag.cxx',
  cpp_args: ['-DLIBO_INTERNAL_ONLY'],
  link_with: [i18nlangtag_lib, sal_lib],
  include_directories: [main_inc, udkh_inc, offapi_inc],
  dependencies: [cppunit_dep],
)

test('languagetag', unittester,
   args: [test_langtag])
