# Default Settings for LibreOffice

The schema and default settings for the LibreOffice configuration database.

If you change a file in this module, then a make postprocess is needed after make officecfg.

See also: `configmgr`

`AcceleratorKeyChecker.fodt` in the `util` folder is a tool written in Basic that check menus for
entries that use the same accelerator key. The tool goes through the menus using the accessibility
api and checks the accelerator keys. For information on how to use the tool open the fodt file
in LibreOffice.

For more details about the file format, see
<https://www.openoffice.org/projects/util/common/configuration/oor-document-format.html> (also
mirrored at <https://people.freedesktop.org/~vmiklos/2013/oor-document-format.html>).
