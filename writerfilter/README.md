# Import Filters for LibreOffice Writer

The writerfilter module contains import filters for Writer, using its UNO API.

Import filter for DOCX and RTF.

* Module contents
    * `documentation`: RNG schema for the OOXML tokenizer, etc.
    * `inc`: module-global headers (can be included by any files under source)
    * `qa`: `cppunit` tests
    * `source`: the filters themselves
    * `util`: UNO passive registration config

* Source contents
    * `dmapper`: the domain mapper, hiding UNO from the tokenizers, used by DOCX and RTF import
        * The incoming traffic of `dmapper` can be dumped into an XML file in `/tmp` in
         `dbgutil` builds, start soffice with the `SW_DEBUG_WRITERFILTER=1`
         environment variable if you want that.
    * `filter`: the UNO filter service implementations, invoked by UNO and calling
   the dmapper + one of the tokenizers
    * `ooxml`: the docx tokenizer
    * `rtftok`: the rtf tokenizer
