oovbaapiurbcompiler = find_program('oovbaapiurbcompiler.py')
oovbabuilder = find_program('oovbaapihbuilder.py')

oovbaapi_rdb = custom_target('oovbaapi_urb',
  output: 'oovbaapi.urb',
  command: [oovbaapiurbcompiler,
            unoidl_write_exe,
            udkapi_rdb,
            offapi_rdb,
            meson.current_source_dir(),
            '@PRIVATE_DIR@',
            '@OUTPUT@'])


oovba_fake_h = custom_target('oovbaapi_h',
  output: 'oovba_fake.h',
  command: [oovbabuilder,
            cppumaker_exe,
            udkapi_rdb,
            offapi_rdb,
            oovbaapi_rdb,
            '@OUTDIR@',
            '@PRIVATE_DIR@',
            '@OUTPUT@',
            meson.current_source_dir()]
)

oovbaapi_inc = include_directories('.')
