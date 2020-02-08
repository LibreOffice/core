cppuhelperc = find_program('cppuhelperurb.py')

cppuhelper_urb = custom_target('cppuhelper_urb',
  output: 'cpphelper.urb',
  command: [cppuhelperc,
            unoidl_write_exe,
            udkapi_urb,
            meson.current_source_dir(),
            '@PRIVATE_DIR@',
            '@OUTPUT@'])


cppuhelper_fake_h = custom_target('cpphelper_h',
  output: 'cpphelper_fake.h',
  command: [offbuilder,
            cppumaker_exe,
            cppuhelper_urb,
            udkapi_urb,
            '@OUTDIR@',
            '@PRIVATE_DIR@',
            '@OUTPUT@',
            meson.current_source_dir()]
)

cpphelper_uno_inc = include_directories('.')