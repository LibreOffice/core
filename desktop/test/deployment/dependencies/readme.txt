no-description.oxt, no-dependencies.oxt, empty-dependencies.oxt effectively have
no dependencies and should thus install successfully.

broken-dependencies.oxt contains a malformed description.xml and should thus
display an error and not install.

double-dependencies.oxt contains a description.xml with two dependencies
elements.  This is not allowed by the spec but behaviour is unspecified.  In the
current implementation, it combines the two elements, and thus finds two
unsatisfied dependencies, displays the Unsatisfied Dependencies dialog and does
not install.

version21.oxt contains a dependency on OOo 2.1 (and should thus only install in
OOo 2.1 or later); version21ns.oxt is the same, but with a different way of
using XML namespaces; version21other.oxt additionally contains an unsatisfied
dependency (and should thus not install in any OOo version).  version22.oxt
contains a dependency on OOo 2.2 (and should thus only install in OOo 2.2 or
later).  version10000.oxt contains a dependency on the hypothetical OOo version
10000 (and should thus not install in any OOo version).  versionempty.oxt
contains an empty value attribute and versionnone.oxt lacks the value attribute;
neither is allowed by the spec, but the current implementation treats both as
pre OOo 2.1 versions (and the extensions should thus install in OOo 2.1 or
later).

All of the following testcases should result in the Unsatisfied Dependencies
dialog being displayed and the extension not being installed:

unknown-dependency.oxt contains a dependency without a name attribute, and
should thus display "Unknown" (localized).

funny-dependency.oxt, many-dependencies.oxt contain somewhat extreme input.

license-dependency.oxt contains both a license to be accepted by the user and
dependencies.  What is important here is that the Unsatisfied Dependencies
dialog is displayed, but not the license (as installation aborts as soon as
unsatisfied dependencies are found).