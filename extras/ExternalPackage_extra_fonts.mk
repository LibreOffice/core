# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_extra_fonts,fonts-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_extra_fonts,$(INSTDIR)))

$(eval $(call gb_ExternalPackage_use_unpacked,extras_extra_fonts,fonts-pack))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_extra_fonts,$(LIBO_SHARE_FOLDER)/fonts/truetype,\
	decorative/BETSY2.ttf \
	decorative/Berylibi.ttf \
	decorative/Beryliub.ttf \
	decorative/Beryliui.ttf \
	decorative/Berylium.ttf \
	decorative/CHOPS___.TTF \
	decorative/DTPDingbats.ttf \
	decorative/GenAI102.TTF \
	decorative/GenAR102.TTF \
	decorative/GenI102.TTF \
	decorative/GenR102.TTF \
	decorative/Justus-Bold.ttf \
	decorative/Justus-Italic.ttf \
	decorative/Justus-ItalicOldstyle.ttf \
	decorative/Justus-Oldstyle.ttf \
	decorative/Justus-Roman.ttf \
	decorative/Justus-Versalitas.ttf \
	decorative/MgOpenCanonicaBold.ttf \
	decorative/MgOpenCanonicaBoldItalic.ttf \
	decorative/MgOpenCanonicaItalic.ttf \
	decorative/MgOpenCanonicaRegular.ttf \
	decorative/NEUROPOL.ttf \
	decorative/SAPPM___.ttf \
	decorative/Tuffy.ttf \
	decorative/Tuffy_Bold.ttf \
	decorative/Tuffy_Bold_Italic.ttf \
	decorative/Tuffy_Italic.ttf \
	decorative/Verina_Sans_Bold.otf \
	decorative/Verina_Sans_Bold_Oblique.otf \
	decorative/Verina_Sans_Mono.otf \
	decorative/Verina_Sans_Mono_Bold.otf \
	decorative/Verina_Sans_Mono_Bold_Oblique.otf \
	decorative/Verina_Sans_Mono_Oblique.otf \
	decorative/Verina_Sans_Oblique.otf \
	decorative/Verina_Sans_Roman.otf \
	decorative/Verina_Serif.otf \
	decorative/Verina_Serif_Bold.otf \
	decorative/a010013l.ttf \
	decorative/a010015l.ttf \
	decorative/a010033l.ttf \
	decorative/a010035l.ttf \
	decorative/airmole.ttf \
	decorative/airmolea.ttf \
	decorative/airmoleq.ttf \
	decorative/airmoles.ttf \
	decorative/astronbi.ttf \
	decorative/astronbo.ttf \
	decorative/astronbv.ttf \
	decorative/astronbw.ttf \
	decorative/b018012l.ttf \
	decorative/b018015l.ttf \
	decorative/b018032l.ttf \
	decorative/b018035l.ttf \
	decorative/baveuse.ttf \
	decorative/bluebold.ttf \
	decorative/bluecond.ttf \
	decorative/bluehigh.ttf \
	decorative/bluehigl.ttf \
	decorative/bullpen3.ttf \
	decorative/bullpen_.ttf \
	decorative/bullpeni.ttf \
	decorative/c059013l.ttf \
	decorative/c059016l.ttf \
	decorative/c059033l.ttf \
	decorative/c059036l.ttf \
	decorative/colourba.ttf \
	decorative/colourbb.ttf \
	decorative/counters.ttf \
	decorative/creditva.ttf \
	decorative/creditvb.ttf \
	decorative/creditvi.ttf \
	decorative/creditvz.ttf \
	decorative/d050000l.ttf \
	decorative/edmundis.ttf \
	decorative/edmunds.ttf \
	decorative/engeboit.ttf \
	decorative/engebold.ttf \
	decorative/engeexbi.ttf \
	decorative/engeexbo.ttf \
	decorative/engeexit.ttf \
	decorative/engeexpa.ttf \
	decorative/engeital.ttf \
	decorative/engeregu.ttf \
	decorative/goodfisb.ttf \
	decorative/goodfisc.ttf \
	decorative/goodfish.ttf \
	decorative/goodfisi.ttf \
	decorative/guanine_.ttf \
	decorative/gunplay.ttf \
	decorative/kirsty__.ttf \
	decorative/kirsty_b.ttf \
	decorative/kirsty_i.ttf \
	decorative/kirstybi.ttf \
	decorative/kirstyin.ttf \
	decorative/letstracebasic.ttf \
	decorative/letstraceruled.ttf \
	decorative/luximb.ttf \
	decorative/luximbi.ttf \
	decorative/luximr.ttf \
	decorative/luximri.ttf \
	decorative/luxirb.ttf \
	decorative/luxirbi.ttf \
	decorative/luxirr.ttf \
	decorative/luxirri.ttf \
	decorative/luxisb.ttf \
	decorative/luxisbi.ttf \
	decorative/luxisr.ttf \
	decorative/luxisri.ttf \
	decorative/mufferaw.ttf \
	decorative/n019003l.ttf \
	decorative/n019004l.ttf \
	decorative/n019023l.ttf \
	decorative/n019024l.ttf \
	decorative/n019043l.ttf \
	decorative/n019044l.ttf \
	decorative/n019063l.ttf \
	decorative/n019064l.ttf \
	decorative/n021003l.ttf \
	decorative/n021004l.ttf \
	decorative/n021023l.ttf \
	decorative/n021024l.ttf \
	decorative/n022003l.ttf \
	decorative/n022004l.ttf \
	decorative/n022023l.ttf \
	decorative/n022024l.ttf \
	decorative/nasaliza.ttf \
	decorative/numberpi.ttf \
	decorative/p052003l.ttf \
	decorative/p052004l.ttf \
	decorative/p052023l.ttf \
	decorative/p052024l.ttf \
	decorative/pricedow.ttf \
	decorative/pupcat__.ttf \
	decorative/rina.ttf \
	decorative/s050000l.ttf \
	decorative/sandoval.ttf \
	decorative/screenge.ttf \
	decorative/steelfib.ttf \
	decorative/steelfis.ttf \
	decorative/steelout.ttf \
	decorative/strenu3d.ttf \
	decorative/strenuou.ttf \
	decorative/subpear_.ttf \
	decorative/teen____.ttf \
	decorative/teenbdit.ttf \
	decorative/teenbold.ttf \
	decorative/teenital.ttf \
	decorative/teenlita.ttf \
	decorative/teenlite.ttf \
	decorative/typoderm.ttf \
	decorative/vahika_.ttf \
	decorative/vahikab.ttf \
	decorative/vahikac.ttf \
	decorative/vahikai.ttf \
	decorative/vectroid.ttf \
	decorative/velvenda.ttf \
	decorative/velvendc.ttf \
	decorative/vibroceb.ttf \
	decorative/vibrocei.ttf \
	decorative/vibrocen.ttf \
	decorative/vibrocex.ttf \
	decorative/wintermu.ttf \
	decorative/youregoi.ttf \
	decorative/youregon.ttf \
	decorative/z003034l.ttf \
	decorative/zekton__.ttf \
	decorative/zektonbi.ttf \
	decorative/zektonbo.ttf \
	decorative/zektonit.ttf \
	decorative/zorque.ttf \
))

# vim: set noet sw=4 ts=4:
