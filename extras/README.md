# Extra Modules

Contains templates, clipart galleries, palettes, symbol font, autocorrections, autotexts etc.

+ How-to add a new gallery:
	+ create a directory `extras/source/gallery/foo/`
	+ create a `.str` file `extras/source/gallery/foo/foo.str`
	+ add a `foo` section to `extras/source/gallery/share/gallery_names.ulf`
	+ add a `Gallery_foo.mk` at the top-level (and mention in `Module_extra.mk`)
	+ add a new `GALLERY_FILELIST` statement in `scp2/`

+ How-to add a new autotext category
	+ create a directory `extras/source/autotext/lang/xx/foo/` where `xx` is your lang code. `xx` must exactly fit with an UI lang code.
	+ unzip your `foo.bau` autotext file in this directory (including an empty mimetype file)
	+ add `xx/foo.bau` in `extras/AllLangPackage_autotextshare.mk`
	+ in `extras/CustomTarget_autotextshare.mk`:
		+ add `xx/foo` in `extras_AUTOTEXTSHARE_AUTOTEXTS`
		+ add all files contained in `foo.bau` (except mimetype) in `extras_AUTOTEXTSHARE_XMLFILES`
		+ if `foo.bau` contains files with other extension than `.xml`, `.rdf`, `.svm` and `.png`
			+ add a `CPY` call at the end of the file

+ How-to add a new autotext to an existing category
	+ create a directory `extras/source/autotext/lang/xx/standard/FOO/` to add it in category standard of lang `xx`
	+ add files of the autotext (at least `FOO.xml` for an unformatted autotext)
	+ add autotext name in `extras/source/autotext/lang/xx/standard/BlockList.xml`
	+ add all files of autotext in `extras/source/autotext/lang/xx/standard/META-INF/manifest.xml`
	+ in `extras/CustomTarget_autotextshare.mk`:
		+ add all files of autotext in `extras_AUTOTEXTSHARE_XMLFILES`
		+ if some files have different extension from `.xml`, `.rdf`, `.svm` and `.png`
			+ add a `CPY` call at the end of the file

+ How-to add a new Impress template
	+ clean-up template file as indicated on wiki <https://wiki.documentfoundation.org/Documentation/HowTo/Impress/Make_template_language_independent>
	+ add `<dc:title>Foo</dc:title>` in `meta.xml` to make presentation name translatable
	+ unzip `Foo.otp` file in `extras/source/templates/presnt/Foo` (no space allowed in any file names)
	+ add `Foo.otp` in `Package_tplpresnt.mk`
	+ in `CustomTarget_tplpresnt.mk`:
		+ add `Foo /` in `extras_TEMPLATES_PRESENTATIONS`
		+ add files names contained in `Foo.otp` (except mimetype) in `extras_PRESENTATIONS_XMLFILES`
		+ if `Foo.otp` contains files with other extension than `.xml`, `.svm`, `.svg`, `.png` and `.jpg`
			+ add a `CPY` call at the end of file

+ How-to add a new Writer template
	+ clean-up template file as much as possible, and choose a template category `<Category>`
	+ unzip `Foo.ott` in `extras/source/templates/<Category>/Foo` (no space allowed in any file names)
	+ add `Foo.ott` in `Package_<tplCategory>.mk`
	+ in `CustomTarget_<tplCategory>.mk`:
		+ add `Foo /` in `extras_TEMPLATES_<CATEGORY>`
		+ add files names contained in `Foo.otp` (except mimetype) in `extras_<CATEGORY>_XMLFILES`
		+ if `Foo.ott` contains files with other extension than `.xml`, `.rdf`, `.svm`, `.svg`, `.png` and `.jpg`
			+ add a `CPY` call at the end of file

+ How-to add a new template category
	+ create a directory `extras/source/templates/foo/`
	+ unzip your foo0.ott template files in `extras/source/templates/foo/foo0`
	+ add `Package_tplfoo` and `CustomTarget_tplfoo` in `Module_extras.mk`
	+ use other category `Package_tplcategory.mk` to create `Package_tplfoo.mk`
	+ use other category `CustomTarget_tplcategory.mk` to create `CustomTarget_tplfoo.mk`
		+ replace all category by foo and `CATEGORY` by `FOO`
		+ add all files contained in `foo0.ott` (except mimetype) in `extras_FOO_XMLFILES`
		+ if `foo0.ott` contains files with other extension than `.xml`, `.rdf`, `.svm`, `.svg`, `.png` and `.jpg`
			+ add a `CPY` call at the end of the file
		+ optionally, replace extension ott (4 places)
