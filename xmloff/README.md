# ODF Import and Export Filter Logic

The main library "xo" contains the basic ODF import/export filter
implementation for most applications.  The document is accessed
via its UNO API, which has the advantage that the same import/export
code can be used for text in all applications (from/to Writer/EditEngine).
The filter consumes/produces via SAX UNO API interface (implemented in
"sax").  Various bits of the ODF filters are also implemented in
applications, for example [git:sw/source/filter/xml].

There is a central list of all element or attribute names in
`git:include/xmloff/xmltoken.hxx`.  The main class of the import filter
is SvXMLImport, and of the export filter SvXMLExport.

The Import filter maintains a stack of contexts for each element being
read.  There are many classes specific to particular elements, derived
from SvXMLImportContext.

Note that for export several different versions of ODF are supported,
with the default being the latest ODF version with "extensions", which
means it may contain elements and attributes that are only in drafts of
the specification or are not yet submitted for specification.  Documents
produced in the other (non-extended) ODF modes are supposed to be
strictly conforming to the respective specification, i.e., only markup
defined by the ODF specification is allowed.

There is another library "xof" built from the source/transform directory,
which is the filter for the OpenOffice.org XML format.  This legacy format
is a predecessor of ODF and was the default in OpenOffice.org 1.x versions,
which did not support ODF.  This filter works as a SAX transformation
from/to ODF, i.e., when importing a document the transform library reads
the SAX events from the file and generates SAX events that are then
consumed by the ODF import filter.

[OpenOffice.org XML File Format](http://www.openoffice.org/xml/general.html)

There is some stuff in the "dtd" directory which is most likely related
to the OpenOffice.org XML format but is possibly outdated and obsolete.

## Add New XML Tokens

When adding a new XML token, you need to add its entry in the following three
files:

* `git:include/xmloff/xmltoken.hxx`
* `git:xmloff/source/core/xmltoken.cxx`
* `git:xmloff/source/token/tokens.txt`
