cppuhelperc = find_program('cppuhelperurb.py')
cppuhbuilder = find_program('cppuhbuilder.py')

cppuhelper_rdb = custom_target('cppuhelper_urb',
  output: 'cpphelper.rdb',
  command: [cppuhelperc,
            unoidl_write_exe,
            udkapi_rdb,
            meson.current_source_dir(),
            '@PRIVATE_DIR@',
            '@OUTPUT@'])


cppuhelper_fake_h = custom_target('cpphelper_h',
  output: 'cpphelper_fake.h',
  command: [cppuhbuilder,
            cppumaker_exe,
            cppuhelper_rdb,
            udkapi_rdb,
            '@OUTDIR@',
            '@PRIVATE_DIR@',
            '@OUTPUT@',
            meson.current_source_dir()]
)

cpphelper_uno_inc = include_directories('.')