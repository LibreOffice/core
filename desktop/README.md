# LibreOffice Binary

Code for the LibreOffice main binary (`soffice`) resides here. The `soffice_main`
function for the `soffice` binary can be found here.


## Stable Interface

Some of the artifacts built here are part of a LibreOffice installation set's
stable interface, which (programmatic) clients can depend on.  Among them are:

### soffice

In the `program` directory (`program/` on Linux and Windows, `Contents/MacOS/`
on macOS).

### unoinfo

In the `program` directory (`program/` on Linux and Windows, `Contents/MacOS/`
on macOS).

When called with a sole argument of `c++`, it prints to stdout an absolute
pathname denoting the directory where the public URE libraries are found.

When called with a sole argument of `java`, it prints to stdout a marker
character (either an ASCII '0' or '1') followed by a sequence of zero or more
absolute pathnames denoting jars or directories that need to be included in a
class loader's search locations.

If the marker character is '0' (on Linux and macOS), the pathnames are
encoded as bytes, and any two pathnames in the sequence are separated from each
other by NUL bytes.

If the marker character is '1' (on Windows), the pathnames are encoded as
UTF-16-LE two-byte code units, and any two pathnames in the sequence are
separated from each other by two-byte `NUL` code units.

## Other Binaries

### oosplash
Splash screen for the LibreOffice `soffice` binary.


## Extensions

The directory `test/deployment` contains some extensions to be used for testing:

* `test/deployment/crashextension`: C++ extension to make LibreOffice crash. Useful for testing Crashreporter.
  * Build with `Extension_test-crashextension`.
  * Extension can be found in `workdir/Extension/test-crashextension.oxt`
* `test/deployment/passive`: C++, Java and Python extension samples with passive registration.
  * Build with `make Extension_test-passive`.
  * Extension can be found in `workdir/Extension/test-passive.oxt`
* `test/deployment/active`: C++, Java and Python extension samples with active registration.
  * Build with `make Extension_test-active`.
  * Extension can be found in `workdir/Extension/test-active.oxt`
