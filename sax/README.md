# UNO Services for SAX
 
UNO services for SAX parsing and C++ functions for XMLSchema-2 data types.

* `source/expwrap`:
    string-based SAX parser UNO service wrapping expat
* `source/fastparser`:
    multi-threaded token-based SAX parser UNO service wrapping libxml2
* `source/tools`:
    + C++ wrapper for fast SAX parser
    + C++ XMLSchema-2 data type conversion helpers

Multi-threading in FastParser can be disabled for debugging purposes with:

    SAX_DISABLE_THREADS=1 SAL_LOG="+INFO.sax.fastparser+WARN"
