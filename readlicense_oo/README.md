# LibreOffice Licensing Blurb

Contains the stock libreoffice licensing blurb, as distributed in the install
directory, and also potentially at run-time.

## Generating Licence Files

License files are generated from a single source file (`license/license.xml`).
Output file formats are plain text and html.

- The plain text and the html format is generated with XSLT. There are two
  separate XSL files for plain text and html.

## Conditional Text

The contents of the license file depends on the build configuration. Several
externals may or may not be shipped with LibreOffice. Therefore, we need to pass
information about build configuration to the XSLT processor.

Variables used for conditional text:

- `BUILD_TYPE`: A space separated list of libraries/externals. If an external is
  present in that list, then the related license text should be included.

- `MPL_SUBSET`: If the variable is defined, then GPL and LGPL license text will not
  be included, because none of the built-in code need it.

- `OS`: The target platform. E.g. MSVC Runtime is packaged and used only on Windows.

- `WITH_THEMES`: A space separated list of icon sets that are used in the build.

Conditional text are surrounded by and extra `<div>` tag. The class attribute of
that `<div>` tag decides which parameter values are taken into consideration.

