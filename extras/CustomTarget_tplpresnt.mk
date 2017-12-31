# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/source/templates/presnt))

extras_TEMPLATES_PRESENTATIONS := \
	Alizarin \
	Beehive \
	Blue_Curve \
	Blueprint_Plans \
	BrightBlue \
	Classy_Red \
	DNA \
	Focus \
	Forestbird \
	Impress \
	Inspiration \
	Lights \
	LushGreen \
	Metropolis \
	Midnightblue \
	Nature_Illustration \
	Pencil \
	Piano \
	Portfolio \
	Progress \
	Sunset \
	Vintage \
	Vivid \


extras_PRESENTATIONS_XMLFILES := \
	Alizarin/content.xml \
	Alizarin/META-INF/manifest.xml \
	Alizarin/meta.xml \
	Alizarin/styles.xml \
	Alizarin/Thumbnails/thumbnail.png \
	Beehive/content.xml \
	Beehive/META-INF/manifest.xml \
	Beehive/meta.xml \
	Beehive/Pictures/10000201000004220000009801B6D855787A5E1C.png \
	Beehive/Pictures/1000020100000422000000A2433FA0A2295FE575.png \
	Beehive/Pictures/1000020100000422000000B279DFBB4971C54D33.png \
	Beehive/styles.xml \
	Beehive/Thumbnails/thumbnail.png \
	Blue_Curve/content.xml \
	Blue_Curve/META-INF/manifest.xml \
	Blue_Curve/meta.xml \
	Blue_Curve/Pictures/1000000000000421000000B8A3E59788A01B5109.jpg \
	Blue_Curve/styles.xml \
	Blue_Curve/Thumbnails/thumbnail.png \
	Blueprint_Plans/content.xml \
	Blueprint_Plans/META-INF/manifest.xml \
	Blueprint_Plans/meta.xml \
	Blueprint_Plans/Pictures/10000000000000580000001F99D2A38C014B254B.png \
	Blueprint_Plans/Pictures/100000000000033C0000026D847ACE982C6E20B6.png \
	Blueprint_Plans/Pictures/10000000000004220000031A5C2E04AB7C51FB60.png \
	Blueprint_Plans/styles.xml \
	Blueprint_Plans/Thumbnails/thumbnail.png \
	BrightBlue/content.xml \
	BrightBlue/META-INF/manifest.xml \
	BrightBlue/meta.xml \
	BrightBlue/Pictures/1000020100000520000000CCF41AC5B79CBB20DE.png \
	BrightBlue/Pictures/100010580000879200001508D21A14F0A9096EB5.svg \
	BrightBlue/styles.xml \
	BrightBlue/Thumbnails/thumbnail.png \
	Classy_Red/content.xml \
	Classy_Red/META-INF/manifest.xml \
	Classy_Red/meta.xml \
	Classy_Red/Object1/content.xml \
	Classy_Red/Object1/meta.xml \
	Classy_Red/Object1/styles.xml \
	Classy_Red/Pictures/10000000000002F000000022299043CB8313294B.png \
	Classy_Red/Pictures/10000000000002F00000003DEB79E9C260D95118.png \
	Classy_Red/Pictures/TablePreview1.svm \
	Classy_Red/styles.xml \
	Classy_Red/Thumbnails/thumbnail.png \
	DNA/content.xml \
	DNA/META-INF/manifest.xml \
	DNA/meta.xml \
	DNA/Pictures/100000000000055600000300C32362E53F69C13D.png \
	DNA/styles.xml \
	DNA/Thumbnails/thumbnail.png \
	Focus/content.xml \
	Focus/META-INF/manifest.xml \
	Focus/meta.xml \
	Focus/Pictures/2000000C0000634100004A4C3651558E9C43717A.svm \
	Focus/Pictures/2000000E0000634100004A4C2A08177FA3DF9654.svm \
	Focus/Pictures/2000000E0000634100004A4C8B8C0B6EC20B2683.svm \
	Focus/Pictures/2000000E0000634100004A4CCD2247D4D39A5527.svm \
	Focus/styles.xml \
	Focus/Thumbnails/thumbnail.png \
	Forestbird/content.xml \
	Forestbird/META-INF/manifest.xml \
	Forestbird/meta.xml \
	Forestbird/Pictures/10000000000003FB000002FE917E6CB003A83232.png \
	Forestbird/Pictures/10000000000003FD000002FE4C4CDF0F4A7ED9F9.png \
	Forestbird/Pictures/10000000000003FD000002FE92464623A3FB3F6D.png \
	Forestbird/Pictures/10000201000003FC000002FE87524FDDC56B0E9E.png \
	Forestbird/styles.xml \
	Forestbird/Thumbnails/thumbnail.png \
	Impress/content.xml \
	Impress/META-INF/manifest.xml \
	Impress/meta.xml \
	Impress/styles.xml \
	Impress/Thumbnails/thumbnail.png \
	Inspiration/content.xml \
	Inspiration/META-INF/manifest.xml \
	Inspiration/meta.xml \
	Inspiration/Pictures/1000000000000CEC000009B16D3849F3A43CBBBC.png \
	Inspiration/styles.xml \
	Inspiration/Thumbnails/thumbnail.png \
	Lights/content.xml \
	Lights/META-INF/manifest.xml \
	Lights/meta.xml \
	Lights/Pictures/100000000000042100000319935403CE7B0BE43B.jpg \
	Lights/Pictures/1000000000000423000000634B59AC09AB41D853.jpg \
	Lights/styles.xml \
	Lights/Thumbnails/thumbnail.png \
	LushGreen/content.xml \
	LushGreen/META-INF/manifest.xml \
	LushGreen/meta.xml \
	LushGreen/Pictures/1000020100000640000004AD17A4D9F0A443E0A2.png \
	LushGreen/Pictures/1000020100000640000004AD5A95F4B7B86B2ED1.png \
	LushGreen/Pictures/1000020100000640000004AD88FC52DBCACCDD82.png \
	LushGreen/Pictures/1000020100000640000004ADF67714D670705C1E.png \
	LushGreen/styles.xml \
	LushGreen/Thumbnails/thumbnail.png \
	Metropolis/content.xml \
	Metropolis/META-INF/manifest.xml \
	Metropolis/meta.xml \
	Metropolis/Pictures/10000201000003AF000002126A94207C4902B60B.png \
	Metropolis/Pictures/100021B200008D2E00004F60B312A649B9EEC188.svg \
	Metropolis/styles.xml \
	Metropolis/Thumbnails/thumbnail.png \
	Midnightblue/content.xml \
	Midnightblue/META-INF/manifest.xml \
	Midnightblue/meta.xml \
	Midnightblue/Pictures/1000000000000020000000204B249CA79A42C6D7.png \
	Midnightblue/styles.xml \
	Midnightblue/Thumbnails/thumbnail.png \
	Nature_Illustration/content.xml \
	Nature_Illustration/META-INF/manifest.xml \
	Nature_Illustration/meta.xml \
	Nature_Illustration/Pictures/10000000000004210000031931EA92AB7FB2FD6C.jpg \
	Nature_Illustration/Pictures/100000000000042100000319892187DC121460D8.jpg \
	Nature_Illustration/styles.xml \
	Nature_Illustration/Thumbnails/thumbnail.png \
	Pencil/content.xml \
	Pencil/META-INF/manifest.xml \
	Pencil/meta.xml \
	Pencil/Pictures/1000020100000088000000870A26C51C993940C3.png \
	Pencil/Pictures/100002010000037D0000001C2F14EFE8274AD9FC.png \
	Pencil/Pictures/1000020100000402000000A2F7F509AE74DE6C69.png \
	Pencil/styles.xml \
	Pencil/Thumbnails/thumbnail.png \
	Piano/content.xml \
	Piano/META-INF/manifest.xml \
	Piano/meta.xml \
	Piano/Pictures/100000000000035400000120266623530F1969D0.png \
	Piano/Pictures/100002010000033A0000006EDD62437E1CC42053.png \
	Piano/styles.xml \
	Piano/Thumbnails/thumbnail.png \
	Portfolio/content.xml \
	Portfolio/META-INF/manifest.xml \
	Portfolio/meta.xml \
	Portfolio/Pictures/1000020100000CEB000009B00D8BDC51D4D18334.png \
	Portfolio/Pictures/1000020100000CEB000009B0698EB1ACA470D143.png \
	Portfolio/Pictures/1000020100000CEB000009B0C3D535B3CBDAE49B.png \
	Portfolio/styles.xml \
	Portfolio/Thumbnails/thumbnail.png \
	Progress/content.xml \
	Progress/META-INF/manifest.xml \
	Progress/meta.xml \
	Progress/styles.xml \
	Progress/Thumbnails/thumbnail.png \
	Sunset/content.xml \
	Sunset/META-INF/manifest.xml \
	Sunset/meta.xml \
	Sunset/Pictures/10000201000003AF000002126EB1F3F571D17DD0.png \
	Sunset/Pictures/1000416600008D2E00004F6045D45030E4D3EC44.svg \
	Sunset/styles.xml \
	Sunset/Thumbnails/thumbnail.png \
	Vintage/content.xml \
	Vintage/META-INF/manifest.xml \
	Vintage/meta.xml \
	Vintage/Pictures/10000000000000580000001F99D2A38C014B254B.png \
	Vintage/Pictures/10000201000004B000000337490AE22881331C9B.png \
	Vintage/Pictures/10000201000004B00000033776CA70A4BFE2196B.png \
	Vintage/Pictures/10000201000004B0000003379D1127938561C230.png \
	Vintage/Pictures/10000201000004B000000337D99A2303560BD59C.png \
	Vintage/styles.xml \
	Vintage/Thumbnails/thumbnail.png \
	Vivid/content.xml \
	Vivid/META-INF/manifest.xml \
	Vivid/meta.xml \
	Vivid/Pictures/1000000000000020000000204B249CA79A42C6D7.png \
	Vivid/Pictures/10000000000004F900000322783ADE15FCCE3CBA.png \
	Vivid/Pictures/10000000000004F900000322B6E66A5FF68F74A7.png \
	Vivid/Pictures/10000000000005550000035C66CD1AA3E88406D5.png \
	Vivid/styles.xml \
	Vivid/Thumbnails/thumbnail.png \


extras_TPLPRESNT_MIMETYPEFILES := $(foreach atexts,$(extras_TEMPLATES_PRESENTATIONS),$(atexts)/mimetype)


ifneq ($(sort $(foreach file,$(extras_PRESENTATIONS_XMLFILES),$(wordlist 1, 1, $(subst /, ,$(file))))),$(sort $(extras_TEMPLATES_PRESENTATIONS)))
$(call gb_Output_error,defined template presentation do not match existing directories)
endif

$(call gb_CustomTarget_get_target,extras/source/templates/presnt) : \
	$(foreach atexts,$(extras_TEMPLATES_PRESENTATIONS),$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/$(atexts).otp)

$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/%/mimetype : $(SRCDIR)/extras/source/templates/presnt/%/mimetype
	$(call gb_Output_announce,$*/mimetype,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/%.jpg : $(SRCDIR)/extras/source/templates/presnt/%.jpg
	$(call gb_Output_announce,$*.jpg,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/%.svg : $(SRCDIR)/extras/source/templates/presnt/%.svg
	$(call gb_Output_announce,$*.svg,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/%.png : $(SRCDIR)/extras/source/templates/presnt/%.png
	$(call gb_Output_announce,$*.png,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/%.svm : $(SRCDIR)/extras/source/templates/presnt/%.svm
	$(call gb_Output_announce,$*.svm,$(true),CPY,1)
	cp $< $@

$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/%.xml : $(SRCDIR)/extras/source/templates/presnt/%.xml \
		| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$*.xml,$(true),XSL,1)
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $@ $(SRCDIR)/extras/util/compact.xsl $<

$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)/%.otp :
	$(call gb_Output_announce,$*.otp,$(true),ZIP,2)
	$(call gb_Helper_abbreviate_dirs,\
		cd $(EXTRAS_PRESENTATIONS_DIR) && \
		zip -qrX --filesync --must-match $@ $(EXTRAS_PRESENTATIONS_FILES) \
	)

define extras_Tplpresnt_make_file_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : $(SRCDIR)/$(1)/$(2) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

endef

define extras_Tplpresnt_make_zip_deps
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	$(addprefix $(call gb_CustomTarget_get_workdir,$(1))/,$(filter $(3)/%,$(extras_TPLPRESNT_MIMETYPEFILES) $(extras_PRESENTATIONS_XMLFILES))) \
	| $(dir $(call gb_CustomTarget_get_workdir,$(1))/$(2)).dir

$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_PRESENTATIONS_FILES := $(foreach file,$(filter $(3)/%,$(extras_TPLPRESNT_MIMETYPEFILES) $(extras_PRESENTATIONS_XMLFILES)),$(subst $(3)/,,$(file)))
$(call gb_CustomTarget_get_workdir,$(1))/$(2) : \
	EXTRAS_PRESENTATIONS_DIR := $(call gb_CustomTarget_get_workdir,$(1))/$(3)

endef

$(eval $(foreach file,$(extras_TPLPRESNT_MIMETYPEFILES) $(extras_PRESENTATIONS_XMLFILES),\
	$(call extras_Tplpresnt_make_zip_deps,extras/source/templates/presnt,$(file)) \
))

$(eval $(foreach atexts,$(extras_TEMPLATES_PRESENTATIONS),\
	$(call extras_Tplpresnt_make_zip_deps,extras/source/templates/presnt,$(atexts).otp,$(atexts)) \
))

# vim: set noet sw=4 ts=4:
