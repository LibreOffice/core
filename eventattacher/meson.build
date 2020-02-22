evtatt_lib = shared_library('evtattlo',
  'source/eventattacher.cxx',
  udk_fake_h, off_fake_h,
  include_directories: [main_inc],
  cpp_args: ['-DLIBO_INTERNAL_ONLY' ],
  link_with: [comphelper_lib, cppuhelper_lib, cppu_lib, sal_lib],
  install: true,
)
