Modify the OpenSymbol.sfd and generate opens___.ttf from it with FontForge. The
.sfd is the canonical source for the .ttf. See .sfd format documentation at
https://fontforge.org/docs/techref/sfdformat.html.
Use TTX/FontTools, which can dump the content of .ttf as an XML file, in order
to make sure that the resulting .ttf differs only deliberately from the original
one.

Every new version of the font should increase the font version. This is
important so that when two versions of the font are installed, e.g.  one
bundled inside the office installation, and one already installed on the system
that the most recent font can be detected and favoured.

Bumping the font version is easiest done manually by bumping the place in
the .sfd where the version is mentioned, e.g.

-Version: 102.3
+Version: 102.4

The glyph names in the font should follow Adobe Glyph list for new fonts
(https://github.com/adobe-type-tools/agl-aglfn) or just the uniXXXX scheme
(see https://github.com/adobe-type-tools/agl-specification for the rationale).

Fragments of OpenSymbol are mapped internally to a number of other
fonts for interoperability purposes. See
unotools/source/misc/fontcvt.cxx pwrt, the StarSymbol conversion
tables.

Generate the ttf from updated sfd:
fontforge -lang=ff -c 'Open($1); Generate($2)' extras/source/truetype/symbol/OpenSymbol.sfd opens___.ttf

Update opens___.ttf on TDF server:
scp opens___.ttf libreoffice@dev-www.libreoffice.org:/srv/www/dev-www.libreoffice.org/extern/$(sha256sum opens___.ttf | awk '{ print $1 }')-opens___.ttf

Update sha256sums in source files like in this example commit:
https://gerrit.libreoffice.org/#/c/75577/
