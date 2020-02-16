subdir('Office')
subdir('TypeDetection')

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

system_hxx = custom_target('System.hxx',
  output: 'System.hxx',
  input: ['registry/cppheader.xsl',
          'registry/schema/org/openoffice/System.xcs'],
  command: [xslt_bin,
            '--nonet',
            '--stringparam',
            'ns1',
            'System',
            '-o',
            '@OUTPUT@',
            '@INPUT@'])

vcl_hxx = custom_target('VCL.hxx',
  output: 'VCL.hxx',
  input: ['registry/cppheader.xsl',
          'registry/schema/org/openoffice/VCL.xcs'],
  command: [xslt_bin,
            '--nonet',
            '--stringparam',
            'ns1',
            'VCL',
            '-o',
            '@OUTPUT@',
            '@INPUT@'])

inet_hxx = custom_target('Inet.hxx',
  output: 'Inet.hxx',
  input: ['registry/cppheader.xsl',
          'registry/schema/org/openoffice/Inet.xcs'],
  command: [xslt_bin,
            '--nonet',
            '--stringparam',
            'ns1',
            'Inet',
            '-o',
            '@OUTPUT@',
            '@INPUT@'])

setup_inc = include_directories('..') # HACK
