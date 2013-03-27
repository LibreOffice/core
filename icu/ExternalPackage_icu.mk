# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,icu,icu))

$(eval $(call gb_ExternalPackage_add_unpacked_files,icu,inc/external/layout,\
	source/layout/LayoutEngine.h \
	source/layout/LEFontInstance.h \
	source/layout/LEGlyphFilter.h \
	source/layout/LEGlyphStorage.h \
	source/layout/LEInsertionList.h \
	source/layout/LELanguages.h \
	source/layout/LEScripts.h \
	source/layout/LESwaps.h \
	source/layout/LETypes.h \
	source/layout/loengine.h \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,icu,inc/external/unicode,\
	source/common/unicode/appendable.h \
	source/common/unicode/brkiter.h \
	source/common/unicode/bytestream.h \
	source/common/unicode/bytestrie.h \
	source/common/unicode/bytestriebuilder.h \
	source/common/unicode/caniter.h \
	source/common/unicode/chariter.h \
	source/common/unicode/dbbi.h \
	source/common/unicode/docmain.h \
	source/common/unicode/dtintrv.h \
	source/common/unicode/errorcode.h \
	source/common/unicode/icudataver.h \
	source/common/unicode/icuplug.h \
	source/common/unicode/idna.h \
	source/common/unicode/localpointer.h \
	source/common/unicode/locid.h \
	source/common/unicode/messagepattern.h \
	source/common/unicode/normalizer2.h \
	source/common/unicode/normlzr.h \
	source/common/unicode/parseerr.h \
	source/common/unicode/parsepos.h \
	source/common/unicode/platform.h \
	source/common/unicode/ptypes.h \
	source/common/unicode/putil.h \
	source/common/unicode/rbbi.h \
	source/common/unicode/rep.h \
	source/common/unicode/resbund.h \
	source/common/unicode/schriter.h \
	source/common/unicode/std_string.h \
	source/common/unicode/strenum.h \
	source/common/unicode/stringpiece.h \
	source/common/unicode/stringtriebuilder.h \
	source/common/unicode/symtable.h \
	source/common/unicode/ubidi.h \
	source/common/unicode/ubrk.h \
	source/common/unicode/ucasemap.h \
	source/common/unicode/ucat.h \
	source/common/unicode/uchar.h \
	source/common/unicode/ucharstrie.h \
	source/common/unicode/ucharstriebuilder.h \
	source/common/unicode/uchriter.h \
	source/common/unicode/uclean.h \
	source/common/unicode/ucnv.h \
	source/common/unicode/ucnv_cb.h \
	source/common/unicode/ucnv_err.h \
	source/common/unicode/ucnvsel.h \
	source/common/unicode/uconfig.h \
	source/common/unicode/udata.h \
	source/common/unicode/uenum.h \
	source/common/unicode/uidna.h \
	source/common/unicode/uiter.h \
	source/common/unicode/uloc.h \
	source/common/unicode/umachine.h \
	source/common/unicode/umisc.h \
	source/common/unicode/unifilt.h \
	source/common/unicode/unifunct.h \
	source/common/unicode/unimatch.h \
	source/common/unicode/uniset.h \
	source/common/unicode/unistr.h \
	source/common/unicode/unorm.h \
	source/common/unicode/unorm2.h \
	source/common/unicode/uobject.h \
	source/common/unicode/urename.h \
	source/common/unicode/urep.h \
	source/common/unicode/ures.h \
	source/common/unicode/uscript.h \
	source/common/unicode/uset.h \
	source/common/unicode/usetiter.h \
	source/common/unicode/ushape.h \
	source/common/unicode/usprep.h \
	source/common/unicode/ustring.h \
	source/common/unicode/ustringtrie.h \
	source/common/unicode/utext.h \
	source/common/unicode/utf.h \
	source/common/unicode/utf16.h \
	source/common/unicode/utf32.h \
	source/common/unicode/utf8.h \
	source/common/unicode/utf_old.h \
	source/common/unicode/utrace.h \
	source/common/unicode/utypes.h \
	source/common/unicode/uvernum.h \
	source/common/unicode/uversion.h \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,icu,inc/external/unicode,\
	source/i18n/unicode/alphaindex.h \
	source/i18n/unicode/basictz.h \
	source/i18n/unicode/bms.h \
	source/i18n/unicode/bmsearch.h \
	source/i18n/unicode/calendar.h \
	source/i18n/unicode/choicfmt.h \
	source/i18n/unicode/coleitr.h \
	source/i18n/unicode/coll.h \
	source/i18n/unicode/colldata.h \
	source/i18n/unicode/curramt.h \
	source/i18n/unicode/currpinf.h \
	source/i18n/unicode/currunit.h \
	source/i18n/unicode/datefmt.h \
	source/i18n/unicode/dcfmtsym.h \
	source/i18n/unicode/decimfmt.h \
	source/i18n/unicode/dtfmtsym.h \
	source/i18n/unicode/dtitvfmt.h \
	source/i18n/unicode/dtitvinf.h \
	source/i18n/unicode/dtptngen.h \
	source/i18n/unicode/dtrule.h \
	source/i18n/unicode/fieldpos.h \
	source/i18n/unicode/fmtable.h \
	source/i18n/unicode/format.h \
	source/i18n/unicode/fpositer.h \
	source/i18n/unicode/gregocal.h \
	source/i18n/unicode/locdspnm.h \
	source/i18n/unicode/measfmt.h \
	source/i18n/unicode/measunit.h \
	source/i18n/unicode/measure.h \
	source/i18n/unicode/msgfmt.h \
	source/i18n/unicode/numfmt.h \
	source/i18n/unicode/numsys.h \
	source/i18n/unicode/plurfmt.h \
	source/i18n/unicode/plurrule.h \
	source/i18n/unicode/rbnf.h \
	source/i18n/unicode/rbtz.h \
	source/i18n/unicode/regex.h \
	source/i18n/unicode/search.h \
	source/i18n/unicode/selfmt.h \
	source/i18n/unicode/simpletz.h \
	source/i18n/unicode/smpdtfmt.h \
	source/i18n/unicode/sortkey.h \
	source/i18n/unicode/stsearch.h \
	source/i18n/unicode/tblcoll.h \
	source/i18n/unicode/timezone.h \
	source/i18n/unicode/tmunit.h \
	source/i18n/unicode/tmutamt.h \
	source/i18n/unicode/tmutfmt.h \
	source/i18n/unicode/translit.h \
	source/i18n/unicode/tzfmt.h \
	source/i18n/unicode/tznames.h \
	source/i18n/unicode/tzrule.h \
	source/i18n/unicode/tztrans.h \
	source/i18n/unicode/ucal.h \
	source/i18n/unicode/ucol.h \
	source/i18n/unicode/ucoleitr.h \
	source/i18n/unicode/ucsdet.h \
	source/i18n/unicode/ucurr.h \
	source/i18n/unicode/udat.h \
	source/i18n/unicode/udateintervalformat.h \
	source/i18n/unicode/udatpg.h \
	source/i18n/unicode/uldnames.h \
	source/i18n/unicode/ulocdata.h \
	source/i18n/unicode/umsg.h \
	source/i18n/unicode/unirepl.h \
	source/i18n/unicode/unum.h \
	source/i18n/unicode/upluralrules.h \
	source/i18n/unicode/uregex.h \
	source/i18n/unicode/usearch.h \
	source/i18n/unicode/uspoof.h \
	source/i18n/unicode/utmscale.h \
	source/i18n/unicode/utrans.h \
	source/i18n/unicode/vtzone.h \
))

$(eval $(call gb_ExternalPackage_use_external_project,icu,icu))
ifeq ($(OS),WNT)
ifeq ($(COM),GCC)

$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicudata.dll.a,source/data/lib.lib))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicuuc.dll.a,source/lib/libicuuc.lib))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicui18n.dll.a,source/lib/libicuin.lib))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicule.dll.a,source/lib/libicule.lib))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicutu.dll.a,source/lib/libicutu.lib))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/icudt$(ICU_MAJOR).dll,source/lib/libicudt$(ICU_MAJOR).dll))
$(eval $(call gb_ExternalPackage_add_files,icu,bin,\
	source/lib/icuuc$(ICU_MAJOR).dll \
	source/lib/icuin$(ICU_MAJOR).dll \
	source/lib/icule$(ICU_MAJOR).dll \
	source/lib/icutu$(ICU_MAJOR).dll \
	source/bin/genccode.exe \
	source/bin/genbrk.exe \
	source/bin/gencmn.exe \
))

else
$(eval $(call gb_ExternalPackage_add_files,icu,lib,\
	source/lib/icudt.lib \
	source/lib/icuuc.lib \
	source/lib/icuin.lib \
	source/lib/icule.lib \
	source/lib/icutu.lib\
))
$(eval $(call gb_ExternalPackage_add_files,icu,bin,\
	source/lib/icudt$(ICU_MAJOR).dll \
	source/lib/icuuc$(ICU_MAJOR).dll \
	source/lib/icuin$(ICU_MAJOR).dll \
	source/lib/icule$(ICU_MAJOR).dll \
	source/lib/icutu$(ICU_MAJOR).dll \
	source/bin/genccode.exe \
	source/bin/genbrk.exe \
	source/bin/gencmn.exe \
))
endif
else # $(OS)
ifeq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_ExternalPackage_add_files,icu,lib,\
	source/lib/libicudata.a \
	source/lib/libicuuc.a \
	source/lib/libicui18n.a \
	source/lib/libicule.a \
	source/lib/libicutu.a \
))
else ifeq ($(OS),ANDROID)
$(eval $(call gb_ExternalPackage_add_files,icu,lib,\
	source/lib/libicudatalo.so \
	source/lib/libicuuclo.so \
	source/lib/libicui18nlo.so \
	source/lib/libiculelo.so \
	source/lib/libicutulo.so \
))
else
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicudata$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO),source/lib/libicudata$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicudata$(gb_Library_DLLEXT).$(ICU_MAJOR),source/lib/libicudata$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicudata$(gb_Library_DLLEXT),source/lib/libicudata$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicuuc$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO),source/lib/libicuuc$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicuuc$(gb_Library_DLLEXT).$(ICU_MAJOR),source/lib/libicuuc$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicuuc$(gb_Library_DLLEXT),source/lib/libicuuc$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicui18n$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO),source/lib/libicui18n$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicui18n$(gb_Library_DLLEXT).$(ICU_MAJOR),source/lib/libicui18n$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicui18n$(gb_Library_DLLEXT),source/lib/libicui18n$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicule$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO),source/lib/libicule$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicule$(gb_Library_DLLEXT).$(ICU_MAJOR),source/lib/libicule$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicule$(gb_Library_DLLEXT),source/lib/libicule$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicutu$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO),source/lib/libicutu$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicutu$(gb_Library_DLLEXT).$(ICU_MAJOR),source/lib/libicutu$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicutu$(gb_Library_DLLEXT),source/lib/libicutu$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO)))

$(eval $(call gb_ExternalPackage_add_files,icu,bin,\
	source/bin/genccode \
	source/bin/genbrk \
	source/bin/gencmn \
))
endif
endif
# vim: set noet sw=4 ts=4:
