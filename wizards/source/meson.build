idirs = ['access2base',
  'depot',
  'euro',
  'formwizard',
  'gimmicks',
  'importwizard',
  'template',
  'tools',
  'tutorials',
]

basic_dir = get_option('datadir') / 'basic'

foreach d: idirs
  install_subdir(d, install_dir: basic_dir)
endforeach
