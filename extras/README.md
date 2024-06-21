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
	+ unzip your `foo.bau` autotext file in this directory (excluding an empty mimetype file)
	+ add `xx/foo.bau` in `extras/AllLangPackage_autotextshare.mk`
	+ in `extras/CustomTarget_autotextshare.mk`:
		+ add all files contained in `foo.bau` (except mimetype) in `extras_AUTOTEXTSHARE_XMLFILES`

+ How-to add a new autotext to an existing category
	+ create a directory `extras/source/autotext/lang/xx/standard/FOO/` to add it in category standard of lang `xx`
	+ add files of the autotext (at least `FOO.xml` for an unformatted autotext)
	+ add autotext name in `extras/source/autotext/lang/xx/standard/BlockList.xml`
	+ add all files of autotext in `extras/source/autotext/lang/xx/standard/META-INF/manifest.xml`
	+ in `extras/CustomTarget_autotextshare.mk`:
		+ add all files of autotext in `extras_AUTOTEXTSHARE_XMLFILES`

+ How-to add a new template category
	+ add long category name in TEMPLATE_LONG_NAMES_ARY in sfx2/inc/doctempl.hrc
	+ add short category name in TEMPLATE_SHORT_NAMES_ARY in sfx2/source/doc/doctemplates.cxx

+ How-to add a new template to an existing category
	+ clean-up template file as indicated on wiki <https://wiki.documentfoundation.org/Documentation/HowTo/Impress/Make_template_language_independent>
	+ recommendation for settings.xml are given on wiki <https://wiki.documentfoundation.org/Documentation/CompatibilityFlags#Settings_to_include_in_a_new_template>
	+ add `<dc:title>Foo</dc:title>` in `meta.xml` to make presentation name translatable
	+ choose a template category `<Category>`
	+ unzip `Foo.ot?` (? = p, t, s or g) in `extras/source/templates/<Category>/Foo` (no space allowed in any file names)
	+ add `<Category>/Foo.ot? /` in `Package_templates.mk` (or `Package_tplpresnt.mk` if it's a presentation)
	+ in `template_files.mk`:
		+ add e.g. `<Category>/Foo/settings.xml` (and all other files which are not automatically added) to `extra_files_COMMON` (or `extra_files_PRESENTATIONS` in case of a presentation template)
	+ for translation of template name
		+ define a new STR_TEMPLATE_NAMExx in include/sfx2/strings.hrc
		+ define a new STR_TEMPLATE_NAMExx_DEF in sfx2/inc/strings.hxx
		+ in sfx2/source/doc/doctempl.cxx
			+ add STR_TEMPLATE_NAMExx_DEF to aTemplateNames
			+ add STR_TEMPLATE_NAMExx to STR_TEMPLATE_NAME
