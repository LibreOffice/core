clew_lib = shared_library('clewlo',
  'source/clew.c',
  include_directories: ['source/include'],
  dependencies: [dl_dep],
)

clew_dep = declare_dependency(link_with: clew_lib,
  include_directories: 'source/include')
