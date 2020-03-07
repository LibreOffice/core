lgen = custom_target('unoidl_lex',
  output : 'sourceprovider-scanner.yy.cxx',
  input: 'source/sourceprovider-scanner.l',
  command : [lex_bin, '-o', '@OUTPUT@', '@INPUT@'])

pgen = custom_target('unoidl_bison',
  output : ['sourceprovider-parser.cxx', 'sourceprovider-parser.hxx'],
  input: 'source/sourceprovider-parser.y',
  command : [bison_bin, '@INPUT@', '--defines=@OUTPUT1@', '--output=@OUTPUT0@'])

unoidl_lib = shared_library('unoidllo',
  'source/legacyprovider.cxx',
  'source/sourcefileprovider.cxx',
  'source/sourcetreeprovider.cxx',
  'source/unoidl.cxx',
  'source/unoidlprovider.cxx',
  lgen, pgen,
  cpp_args: ['-DLO_DLLIMPLEMENTATION_UNOIDL',
             '-DLIBO_INTERNAL_ONLY'],
  include_directories: [main_inc, 'source'],
  link_with: [reg_lib, salhelper_lib, sal_lib],
  gnu_symbol_visibility: 'hidden',
  install: true,
)

executable('unoidl-check',
  'source/unoidl-check.cxx',
  include_directories: [main_inc],
  cpp_args: ['-DLIBO_INTERNAL_ONLY'],
  link_with: [unoidl_lib, salhelper_lib, sal_lib],
  install: true,
)

executable('unoidl-read',
  'source/unoidl-read.cxx',
  include_directories: [main_inc],
  cpp_args: ['-DLIBO_INTERNAL_ONLY'],
  link_with: [unoidl_lib, salhelper_lib, sal_lib],
  install: true,
)

unoidl_write_exe = executable('unoidl-write',
  'source/unoidl-write.cxx',
  include_directories: [main_inc],
  cpp_args: ['-DLIBO_INTERNAL_ONLY'],
  link_with: [unoidl_lib, salhelper_lib, sal_lib],
  install: true,
)
