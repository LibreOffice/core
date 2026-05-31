# Extensions for the Impress and Draw Applications

`source/pdfimport/` - PDF type detection and hybrid PDF/ODF import

PDF rendering is done by Pdfium (see sd/source/filter/pdf); this
directory holds the type detector and the hybrid-PDF handler that
extracts an embedded original document.

`source/minimizer/` - Presentation Minimizer

Shrinks presentations by down-scaling images, and removing
extraneous eg. embedded OLE content.

