schemas that can be used for validating ODF files

subdirs:
- mathml2: W3C MathML 2.0 XML Schema (needed for Math embedded objects)
- odf1.0, odf1.1, odf1.2: official OASIS RelaxNG schemas
- odf1.3: current OASIS draft ODF 1.3 RelaxNG schema
- libreoffice: draft ODF schema, with additional LO extensions

The extension schema in "libreoffice/" is used by all unit tests if
--with-export-validation is given, which is the default.

Notably this means that if you add a new feature to the ODF filters and you add
the required unit test for the new feature, then most likely the test will fail
with a complaint from the validator; in this case the schema needs to be
updated to contain the new elements and attributes.

The extension schema uses the RelaxNG "include" feature to refer to the ODF
schema; this means that it only contains those parts of the schema that
actually need to be changed - this works well in many cases because the ODF
schema is quite well structured with many named patterns, but unfortunately
there are a few places where that isn't the case and large chunks needed to be
copied to override them.

In the easy case, to add an attribute you just want to search for the
corresponding element, which will have a "foo-attlist" named pattern, and then
add another attribute like this:

  <rng:define name="draw-enhanced-geometry-attlist" combine="interleave">
    <rng:optional>
      <rng:attribute name="drawooo:sub-view-size">
        <rng:ref name="string"/>
      </rng:attribute>
    </rng:optional>
  </rng:define>

Currently only the features that are actually exported in the unit tests have
been added to the schema; there is still some work to do here to add
everything; the crashtesting script also does ODF validation of all files and
now also uses the custom schema.

Unfortunately it turned out that there are a lot of extensions already for
which no proposal exists [1], and in many cases not even an entry on the Wiki
[2], so clearly something like this extension schema is needed.

[1] git grep TODO schema/libreoffice
[2] https://wiki.documentfoundation.org/Development/ODF_Implementer_Notes/List_of_LibreOffice_ODF_Extensions

