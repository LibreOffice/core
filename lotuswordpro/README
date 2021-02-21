Import filter for file format of Lotus Word Pro.

== Description ==

The import is not direct, but via an intermediate format: StarOffice
XML, the predecessor of ODF (yes, the code is old). The entry point to
the filter is class LotusWordProImportFilter (refer to Source code
section), but that just hooks up the necessary machinery for processing
StarOffice XML produced by the filter. The real fun starts in function
ReadWordproFile() (source/filter/lwpfilter.cxx); this function
initializes the parser (class Lwp9Reader) and the SAX XML handler that
produces the output (class XFSaxStream). The Lwp9Reader class then does
the actual parsing.

If the module is built with debug level greater than 0, it is possible
to examine the intermediate XML: set environment variable
DBG_LWPIMPORT_DIR= to an existing directory and, on opening an lwp
document, a file named lwpimport.xml will be created in that directory.

== Source code ==

=== Module contents ===
* inc: module-global headers (can be included by any file in source)
* qa: cppunit tests
* source: the filter itself
* util: UNO passive registration config

=== Source contents ===
* filter: lwp document format parser
* filter/LotusWordProImportFilter.cxx: the entry point to the filter
* filter/xfilter: export to StarOffice XML
