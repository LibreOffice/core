How to use the demo programs
=================================


PERFORMANCE
-------------------------------

This program is for evaluating the performance of the XML Security framework.

You can use the sample files from tools/examples:

run "performance eval_export.txt eval_import.txt"

The export file list includes all files need to be signed, and the import file list include all files need to be verified.

JavaFlatFilter:
Used to test performance with Java.
Currently it doesn't work, seems to be because of JavaFramework doesn't work in stand alone program.

In case you want to try it:
- set CLASSPATH=e:\Solar\r\j2sdk1.4.1_03\jre\lib;d:\x\juh.jar;d:\x\jurt.jar;d:\x\ridl.jar;d:\x\unoil.jar;d:\x\java_uno.jar
- regcomp -register -r demo.rdb -c file://d/x/jflatfilter.jar -br demo.rdb