Description.

The StarOffice ActiveX control shows an example of access to UNO through COM technology.
It requires a properly installed StarOffice version 6.0/6.1 or OpenOffice 1.0.
This is a Lite ActiveX control so it can be used only in containers that
allows to use such controls.

Pressing to any link to staroffice document should activate the control.
So the document will be opened in ReadOnly mode.

Also it can be activated with an <OBJECT> tag from a html-page.
Without any parameters for an object tag a new writer document will be
opened for editing. Possible parameters are
    src      - full URL to the file that should be edited/viewed;
               it can contain "private:factory/..." URLs to open new documents
			   for edit, for example "private:factory/swriter"
    readonly - the default value is "true", in case it is set to any other
               value the document is opened for editing

As any ActiveX control this one should be registered.
To let MSIE register it itself the "CODEBASE" parameter
for the "OBJECT" tag should be specified
with an URL to the library "so_activex.dll".
The example of registration with "OBJECT" tag is in example.html.

Also it can be done using regsvr32 application.
To do it please write
<Path to Windows installation>\System32\regsvr32 so_activex.dll

To unregister the control please use /u option:
<Path to Windows installation>\system32\regsvr32 so_activex.dll /u


