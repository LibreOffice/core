instdata = configuration_data()

# Add configuration data entries here and in the template.

configure_file(input: 'sofficerc.in',
  output: 'sofficerc',
  configuration: cdata,
  install: true,
  install_dir: get_option('bindir')
)

configure_file(input: 'fundamentalrc.in',
  output: 'fundamentalrc',
  configuration: cdata,
  install: true,
  install_dir: get_option('bindir')
)

configure_file(input: 'bootstraprc.in',
  output: 'bootstraprc',
  configuration: cdata,
  install: true,
  install_dir: get_option('bindir')
)
