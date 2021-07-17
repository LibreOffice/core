# DocumentLoader
This example is somehow similar to DocumentLoader in C++ and Java.
to get started, first start LibreOffice listening on port 2083

    $OFFICE_PROGRAM_PATH/soffice "--accept=socket,port=2083;urp;"

The syntax to run this example is:

    $OFFICE_PROGRAM_PATH/python DocumentLoader.py <path>

You should use 'setsdkenv_unix' for Unix/Linux and setsdkenv_windows.bat for Windows. In this way,
the LibreOffice internal Python interpreter will be used.
