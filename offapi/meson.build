offapiurbcompiler = find_program('offapiurbcompiler.py')
offbuilder = find_program('offapihbuilder.py')

offapi_rdb = custom_target('offapi_urb',
  output: 'offapi.urb',
  command: [offapiurbcompiler,
            unoidl_write_exe,
            udkapi_rdb,
            meson.current_source_dir(),
            '@PRIVATE_DIR@',
            '@OUTPUT@'])


off_fake_h = custom_target('offapi_h',
  output: 'off_fake.h',
  command: [offbuilder,
            cppumaker_exe,
            offapi_rdb,
            udkapi_rdb,
            '@OUTDIR@',
            '@PRIVATE_DIR@',
            '@OUTPUT@',
            meson.current_source_dir()]
)

offapi_inc = include_directories('.')
