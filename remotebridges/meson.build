uuresolver_lib = shared_library('uuresolverlo',
  'source/unourl_resolver/unourl_resolver.cxx',
  udk_fake_h, off_fake_h,
  cpp_args: ['-DLIBO_INTERNAL_ONLY'],
  include_directories: [main_inc, udkh_inc, offapi_inc],
  link_with: [cppuhelper_lib, cppu_lib, sal_lib],
  install: true,
)
