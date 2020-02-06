salhelper_lib = shared_library('salhelper',
  'source/condition.cxx',
  'source/dynload.cxx',
  'source/simplereferenceobject.cxx',
  'source/thread.cxx',
  'source/timer.cxx',
  include_directories: [main_inc],
  cpp_args: ['-DLIBO_INTERNAL_ONLY'],
  link_with: sal_lib,
  )

