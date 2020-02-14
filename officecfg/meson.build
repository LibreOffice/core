subdir('Office')

setup_hxx = custom_target('Setup.hxx',
  output: 'Setup.hxx',
  input: ['registry/cppheader.xsl',
          'registry/schema/org/openoffice/Setup.xcs'],
  command: [xslt_bin,
            '--nonet',
            '--stringparam',
            'ns1',
            'Setup',
            '-o',
            '@OUTPUT@',
            '@INPUT@'])
setup_inc = include_directories('..') # HACK
