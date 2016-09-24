* Compile.

Warning: Before the control can be built user has to add path to MS ATL headers
 into Makefile.

If you use the build environment of the SDK you have to check which MS compiler
do you use. It you use the MS Visual Studio .NET compiler everything should work
fine with the SDK. If you use an older MS compiler please edit the Makefile and
uncomment or remove the line 'CL_NEW_LIB=atls.lib'.

* Description.

The LibreOffice ActiveX control shows an example of access to UNO through COM technology.
It requires a properly installed LibreOffice version 4.0 or later
This is a Lite ActiveX control so it can be used only in containers that
allows to use such controls. It can be activated with an <OBJECT> tag from
a html-page to embed a document. Without any parameters a new writer document will be
opened for editing. Possible parameters are
    src      - full URL to the file that should be edited/viewed;
               it can contain "private:factory/..." URLs to open new documents
			   for edit, for example "private:factory/swriter"
    readonly - if it is set to "true" the document will be opened readonly,
	           otherwise the document will be opened for editing.

The control can be extended easily, for example it can be changed
to allow scripting to load different documents.

As any ActiveX control this one should be registered.
To let MSIE register it itself the "CODEBASE" parameter
for the "OBJECT" tag should be specified
with an URL to the library "so_activex.dll".

Also it can be done using regsvr32 application.
To do it please write
<Path to Windows installation>\System32\regsvr32 so_activex.dll

To unregister the control please use /u option:
<Path to Windows installation>\system32\regsvr32 so_activex.dll /u


