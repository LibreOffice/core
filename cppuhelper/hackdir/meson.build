# This is in its own dir because the output file has the same
# name as the other services file.
services2_rdb = custom_target('services_rdb',
  input: '../../solenv/bin/packcomponents.xslt',
  output: 'services.rdb',
  command: [find_program('genservices.py'),
            '@OUTPUT@',
            '@INPUT@',
            ],
  install: true,
  install_dir: get_option('bindir'),
)
