# DocumentLoader
This example is somehow similar to DocumentLoader in C++ and Java.
to get started, first start LibreOffice listening on port 2083

    soffice "--accept=socket,port=2083;urp;"

The synatx for executing this example is:

    $Office/program/python DocumentLoader.py <path>

You should use 'setsdkenv_unix' for Unix/Linux and setsdkenv_windows.bat for Windows. In this way,
the LibreOffice internal Python interpretor will be used.
