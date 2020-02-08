cnbuilder = find_program('classnamebuilder.py')

# FIXME, this only generates headers. There is no
# proper dependency tracking or dealing of the rdb file.
udk_fake_h = custom_target('udkapi_h',
  output: 'udk_fake.h',
  command: [cnbuilder,
            unoidl_write_exe,
            cppumaker_exe,
            '@OUTDIR@',
            '@PRIVATE_DIR@',
            '@OUTPUT@',
            meson.current_source_dir()]
)
udkh_inc = include_directories('.')
