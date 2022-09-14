# Document Handling Examples

These examples are somehow similar to DocumentLoader in C++ and Java.
To get started, first start LibreOffice listening on port 2083

    $OFFICE_PROGRAM_PATH/soffice "--accept=socket,port=2083;urp;"

Each example script accepts `-h` to show the usage, e.g.

    $OFFICE_PROGRAM_PATH/python DocumentLoader.py -h

You should use `setsdkenv_unix` for Unix/Linux and `setsdkenv_windows.bat` for
Windows. In this way, the LibreOffice internal Python interpreter will be used.

## Document Converter

```bash
$OFFICE_PROGRAM_PATH/python DocumentConverter.py ./tests/ "MS Word 97" doc /tmp/
```

This command searches LibreOffice documents recursively under directory
`./tests` and convert every found document to format Microsoft Word 97. The
converted document can be found under directory `/tmp`.

## Document Loader

```bash
$OFFICE_PROGRAM_PATH/python DocumentLoader.py ./tests/hello.odt
```

This command opens document `./tests/hello.odt` in a separate window.

This script also accepts option to open an empty document. For example, to open
an empty spreadsheet document:

```bash
$OFFICE_PROGRAM_PATH/python DocumentLoader.py --calc
```

## Document Printer

```bash
$OFFICE_PROGRAM_PATH/python DocumentPrinter.py myprinter ./tests/hello.odt "1-3"
```

This command prints the first three pages of document `./tests/hello.odt` on
printer `myprinter `. You can also specify the page range in various format,
e.g. `1,3-5`.

## Document Saver

```bash
$OFFICE_PROGRAM_PATH/python DocumentSaver.py path/to/hello.doc path/to/hello.odt
```

This command saves a Word document `hello.doc` as an OpenDocument file `hello.odt`.
