prebuilt_jar(
  name = 'juh',
  binary_jar = 'instdir/program/classes/juh.jar',
  visibility = ['PUBLIC'],
)

prebuilt_jar(
  name = 'jurt',
  binary_jar = 'instdir/program/classes/jurt.jar',
  visibility = ['PUBLIC'],
)

prebuilt_jar(
  name = 'officebean',
  binary_jar = 'instdir/program/classes/officebean.jar',
  visibility = ['PUBLIC'],
)

prebuilt_jar(
  name = 'ridl',
  binary_jar = 'instdir/program/classes/ridl.jar',
  visibility = ['PUBLIC'],
)

prebuilt_jar(
  name = 'unoil',
  binary_jar = 'instdir/program/classes/unoil.jar',
  visibility = ['PUBLIC'],
)

prebuilt_jar(
  name = 'unoloader',
  binary_jar = 'instdir/program/classes/unoloader.jar',
  visibility = ['PUBLIC'],
)

zip_file(
  name = 'api',
  srcs = [
   ':juh',
   '//javaunohelper:juh-src',
   '//javaunohelper:juh-javadoc',
   ':jurt',
   '//jurt:jurt-src',
   '//jurt:jurt-javadoc',
   ':officebean',
   '//bean:officebean-src',
   '//bean:officebean-javadoc',
   ':ridl',
   '//ridljar:ridl-src',
   '//ridljar:ridl-javadoc',
   ':unoil',
   '//unoil:unoil-src',
   '//unoil:unoil-javadoc',
   ':unoloader',
   '//ridljar/source/unoloader:unoloader-src',
   '//ridljar/source/unoloader:unoloader-javadoc',
  ]
)
