# Registries, Reflection, Introspection Implementation for UNO

The UNO types and services bootstrapping code is very old, and concepts
are tightly knit together. Whenever you want to change something you risk
backwards incompatibility.  The code causes mental pain, and whenever
you need to touch it you want to run away screaming.  One typically ends
up doing minimally invasive changes.  That way, you have a chance of
surviving the process.  But you also pile up guilt.

At the heart of the matter there is the old binary "store" file structure
and the `XRegistry` interface on top of it.  At runtime, both all the UNO
type information (scattered across a number of binary `.rdb` files) and
all the UNO service information (scattered across a number of `.rdb` files
that used to be binary but have been mostly changed to XML now) are
represented by a single `XRegistry` instance each.

The way the respective information is represented in the `XRegistry`
interface simply corresponds to the way the information is stored in the
binary `.rdb` files.  Those files are designed for storage of hierarchically
nested small blobs of information.  Hence, for example information about
a UNO interface type `com.sun.star.foo.XBar` is stored in a nested "folder"
with path `com - sun - star - foo - XBar`, containing little blobs of
information about the type's ancestors, its methods, etc.  Similarly
for information about instantiable services like `com.sun.star.baz.Boz`.

As there are typically multiple `.rdb` files containing types resp.
services (URE specific, LO specific, from extensions, ...), but they need
to be represented by a single `XRegistry` instance, so "nested registries"
were invented.  They effectively form a linear list of chaining `XRegistry`
instances together.  Whenever a path needs to be looked up in the top-level
registry, it effectively searches through the linear list of nested
registries.  All with the cumbersome UNO `XRegistry` interface between
the individual parts.  Horror.

When the XML service `.rdb`s were introduced, we chickened out (see above
for rationale) and put them behind an `XRegistry` facade, so that they
would seamlessly integrate with the existing mess. We postponed
systematic clean-up to the pie-in-the-sky days of LibreOffice 4 (or, "once we'll
become incompatible with OpenOffice.org," as the phrase used to be back then)
