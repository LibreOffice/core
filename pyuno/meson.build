pyuno_lib = shared_library('pyuno',
  'source/module/pyuno_runtime.cxx',
  'source/module/pyuno.cxx',
  'source/module/pyuno_struct.cxx',
  'source/module/pyuno_callable.cxx',
  'source/module/pyuno_module.cxx',
  'source/module/pyuno_type.cxx',
  'source/module/pyuno_util.cxx',
  'source/module/pyuno_except.cxx',
  'source/module/pyuno_adapter.cxx',
  'source/module/pyuno_gc.cxx',
  'source/module/pyuno_iterator.cxx',
  udk_fake_h,
  cpp_args: ['-DLO_DLLIMPLEMENTATION_PYUNO',
             '-DLIBO_INTERNAL_ONLY'],
  include_directories: [main_inc, udkh_inc, 'inc'],
  link_with: [cppu_lib, cppuhelper_lib, sal_lib, salhelper_lib],
  dependencies: [python3_dep],
  install: true,
)

pyuno_wrapper_lib = shared_library('pyuno_wrapper',
  'source/module/pyuno_dlopenwrapper.c',
  include_directories: [main_inc],
  dependencies: [python3_dep.partial_dependency(compile_args: true), dl_dep],
  install: true,
)

pythonloader_lib = shared_library('pythonloader',
  'source/loader/pyuno_loader.cxx',
  udk_fake_h,
  cpp_args: ['-DLIBO_INTERNAL_ONLY'],
  include_directories: [main_inc, udkh_inc, 'inc'],
  link_with: [cppu_lib, cppuhelper_lib, pyuno_lib, sal_lib],
  dependencies: [python3_dep],
  install: true,
)

if host_machine.system() == 'windows'
  python_bin = executable('python',
    'zipcore/python.cxx',
    cpp_args: ['-DLIBO_INTERNAL_ONLY'],
    include_directories: [main_inc, 'inc'],
    link_with: [oopathutils_lib],
  install: true,
  )
endif
