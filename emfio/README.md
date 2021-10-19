# WMF/EMF/EMF+ Reader

## Introduction
The **emfio** module is used to read **WMF** (Windows Metafile), **EMF**
(Enhanced Metafiles) and also **EMF+** (enhanced EMF) files [1], which are
binary formats for vector images from Microsoft. These files can contain vector
graphics, bitmap components and text.

This folder contains `emfio/source/reader` which is used for reading the
WMF/EMF/EMF+ files, either directly, or inside a document. For embedding Windows
Metafiles inside a document, one can use "Insert -> Picture -> From File" to put
such a file into the document. It is possible to export the Windows Metafile
by using right click and choose "save".

Most of the records of WMF/EMF formats come from the Windows Graphics Device
Interface (GDI) API, and for EMF+ they comes from the newer Windows GDI+.

More information about rendering Windows Metafiles can be found in the
[Visual Class Library (VCL)](../vcl) and also in the
[GDIMetaFile](../vcl/README.GDIMetaFile) documentation.

An example demo that renders a metafile using `vcl` be seen by invoking:

    ./bin/run mtfdemo example.wmf

assuming that example.wmf is available in the current directory.
The demo code structure is described in [GDIMetaFile](../vcl/README.GDIMetaFile)
documentation.

[1] [Windows Meta File](https://en.wikipedia.org/wiki/Windows_Metafile),
Wikipedia

## EMF+ Specifics
Handling EMF+ is to some extent different from handling WMF/EMF. More
information can be found in the [VCL](../vcl)
documentation.

## Limitations
Not all the WMF/EMF/EMF+ records are supported by this module. Unsupported
records are marked as "not implemented". You can file a bug report for
implementing these records.
Due to the difference on the fonts available on various platforms, the outcome
of text rendering can be different on Linux, Windows, macOS and elsewhere.

## Known Bugs
Known remaing bugs for this module is gathered here:

* [Bug 103859 \[META\] EMF/WMF (Enhanced/Windows Metafile) bugs and
enhancements](https://bugs.documentfoundation.org/show_bug.cgi?id=103859)

## Dependencies
Direct dependencies for **emfio** are [**drawinglayer**](../drawinglayer) and
[**sax**](../sax).

## Tools
Several tools are available for inspecting WMF/EMF/EMF+ files, which are binary
formats. Some of them are:

* [mso-dumper](https://git.libreoffice.org/mso-dumper/): Reads and dumps various
  binary formats from Microsoft including WMF/EMF/EMF+. The output is in a
  custom XML format. emf-dump.py and wmf-dump.py are usable.
* [RE-lab (Formerly OLEToy)](https://github.com/renyxa/re-lab): Reads, dumps and
modifies several binary formats from Microsoft including WMF/EMF/EMF+, and also
other companies.
* [EMF+ diagnostics reporting tool](https://github.com/chrissherlock/emfplus-decoder)

## Related Software
* [libemf](http://libemf.sourceforge.net/)
* [libwmf](https://github.com/caolanm/libwmf)

## References
Documentation for WMF/EMF/EMF+ formats are available on Microsoft website:

* [\[MS-WMF\]: Windows Metafile Format](https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-wmf/4813e7fd-52d0-4f42-965f-228c8b7488d2)
* [\[MS-EMF\]: Enhanced Metafile Format](https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-emf/91c257d7-c39d-4a36-9b1f-63e3f73d30ca)
* [\[MS-EMFPLUS\]: Enhanced Metafile Format Plus Extensions](https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-emfplus/5f92c789-64f2-46b5-9ed4-15a9bb0946c6)
