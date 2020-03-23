urbcompiler = find_program('urbcompiler.py')
headerbuilder = find_program('headerbuilder.py')

udkapi_rdb = custom_target('udkapi_rdb',
  output: 'udkapi.rdb',
  command: [urbcompiler,
            unoidl_write_exe,
            meson.current_source_dir(),
            '@PRIVATE_DIR@',
            '@OUTPUT@'])

# udkapi.rdb seems to be installed as types.rdb.
# Dunno why. The simplest thing here is to just gen it again.
custom_target('types_rdb',
  output: 'types.rdb',
  command: [urbcompiler,
            unoidl_write_exe,
            meson.current_source_dir(),
            '@PRIVATE_DIR@',
            '@OUTPUT@'],
  install: true,
  install_dir: get_option('bindir')
)


# FIXME, this only generates headers. There is no
# proper dependency tracking or dealing of the rdb file.
udk_fake_h = custom_target('udkapi_h',
  output: 'udk_fake.h',
  command: [headerbuilder,
            cppumaker_exe,
            udkapi_rdb,
            '@OUTDIR@',
            '@PRIVATE_DIR@',
            '@OUTPUT@',
            meson.current_source_dir()]
)
udkh_inc = include_directories('.')
