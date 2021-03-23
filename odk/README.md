Office development kit - implements the first step on the way to the LibreOffice SDK tarball.

Part of the SDK; to build you need to add --enable-odk.


Testing the examples:
=====================

* Go to instdir/sdk (Don't try directly in odk/)

* See <https://api.libreoffice.org/docs/install.html> how to set up the SDK.

** When asked about it during configuration, tell the SDK to do automatic
   deployment of the example extensions that get built.

* In a shell set up for SDK development, build (calling "make") and test
  (following the instructions given at the end of each "make" invocation) each
  of the SDK's examples/ sub-directories.

** An example script to build (though not test) the various examples in batch
   mode is

     find examples \( -type d -name nativelib -prune \) -o \
      \( -name Makefile -a -print -a \( -execdir make \; -o -quit \) \)

   (Note that one of the example extensions asks you to accept an example
   license on stdin during deployment.)
