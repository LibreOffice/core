# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# Norbert Thiebaud <nthiebaud@gmail.com> (C) 2010, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,scfilt))

$(eval $(call gb_Library_add_precompiled_header,sc,$(SRCDIR)/sc/source/filter/inc/filt_pch))

$(eval $(call gb_Library_set_include,scfilt,\
        -I$(SRCDIR)/sc/source/core/inc \
        -I$(SRCDIR)/sc/source/filter/inc \
        -I$(SRCDIR)/sc/source/ui/inc \
        -I$(SRCDIR)/sc/inc \
        -I$(SRCDIR)/sc/inc/pch \
        -I$(WORKDIR)/Misc/sc/ \
        $$(INCLUDE) \
        -I$(OUTDIR)/inc/offuh \
        -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_defs,scfilt,\
        $$(DEFS) \
))

$(eval $(call gb_Library_add_linked_libs,scfilt,\
        basegfx \
        comphelper \
        cppu \
        cppuhelper \
        drawinglayer \
	editeng \
	for \
	msfilter \
	oox \
	sal \
	sax \
	sb \
	sc \
	sfx \
	sot \
	svl \
	svt \
	svx \
	svxcore \
	tk \
	tl \
	utl \
	vcl \
))

$(eval $(call gb_Library_add_exception_objects,scfilt,\
        sc/source/filter/dif/difexp \
        sc/source/filter/dif/difimp \
        sc/source/filter/excel/colrowst \
        sc/source/filter/excel/excdoc \
        sc/source/filter/excel/excel \
        sc/source/filter/excel/excform \
        sc/source/filter/excel/excform8 \
        sc/source/filter/excel/excimp8 \
        sc/source/filter/excel/excrecds \
        sc/source/filter/excel/exctools \
        sc/source/filter/excel/expop2 \
        sc/source/filter/excel/fontbuff \
        sc/source/filter/excel/frmbase \
        sc/source/filter/excel/impop \
        sc/source/filter/excel/namebuff \
        sc/source/filter/excel/read \
        sc/source/filter/excel/tokstack \
        sc/source/filter/excel/xechart \
        sc/source/filter/excel/xecontent \
        sc/source/filter/excel/xeescher \
        sc/source/filter/excel/xeformula \
        sc/source/filter/excel/xehelper \
        sc/source/filter/excel/xelink \
        sc/source/filter/excel/xename \
        sc/source/filter/excel/xepage \
        sc/source/filter/excel/xepivot \
        sc/source/filter/excel/xerecord \
        sc/source/filter/excel/xeroot \
        sc/source/filter/excel/xestream \
        sc/source/filter/excel/xestring \
        sc/source/filter/excel/xestyle \
        sc/source/filter/excel/xetable \
        sc/source/filter/excel/xeview \
        sc/source/filter/excel/xichart \
        sc/source/filter/excel/xicontent \
        sc/source/filter/excel/xiescher \
        sc/source/filter/excel/xiformula \
        sc/source/filter/excel/xihelper \
        sc/source/filter/excel/xilink \
        sc/source/filter/excel/xiname \
        sc/source/filter/excel/xipage \
        sc/source/filter/excel/xipivot \
        sc/source/filter/excel/xiroot \
        sc/source/filter/excel/xistream \
        sc/source/filter/excel/xistring \
        sc/source/filter/excel/xistyle \
        sc/source/filter/excel/xiview \
        sc/source/filter/excel/xladdress \
        sc/source/filter/excel/xlchart \
        sc/source/filter/excel/xlescher \
        sc/source/filter/excel/xlformula \
        sc/source/filter/excel/xlpage \
        sc/source/filter/excel/xlpivot \
        sc/source/filter/excel/xlroot \
        sc/source/filter/excel/xlstyle \
        sc/source/filter/excel/xltoolbar \
        sc/source/filter/excel/xltools \
        sc/source/filter/excel/xltracer \
        sc/source/filter/excel/xlview \
        sc/source/filter/ftools/fapihelper \
        sc/source/filter/ftools/fprogressbar \
        sc/source/filter/ftools/ftools \
        sc/source/filter/html/htmlexp \
        sc/source/filter/html/htmlexp2 \
        sc/source/filter/html/htmlimp \
        sc/source/filter/html/htmlpars \
        sc/source/filter/lotus/expop \
        sc/source/filter/lotus/export \
        sc/source/filter/lotus/filter \
        sc/source/filter/lotus/lotattr \
        sc/source/filter/lotus/lotform \
        sc/source/filter/lotus/lotimpop \
        sc/source/filter/lotus/lotread \
        sc/source/filter/lotus/lotus \
        sc/source/filter/lotus/memory \
        sc/source/filter/lotus/op \
        sc/source/filter/lotus/tool \
        sc/source/filter/qpro/biff \
        sc/source/filter/qpro/qpro \
        sc/source/filter/qpro/qproform \
        sc/source/filter/qpro/qprostyle \
        sc/source/filter/rtf/eeimpars \
        sc/source/filter/rtf/expbase \
        sc/source/filter/rtf/rtfexp \
        sc/source/filter/rtf/rtfimp \
        sc/source/filter/rtf/rtfparse \
        sc/source/filter/starcalc/scflt \
        sc/source/filter/starcalc/scfobj \
        sc/source/filter/xcl97/XclExpChangeTrack \
        sc/source/filter/xcl97/XclImpChangeTrack \
        sc/source/filter/xcl97/xcl97esc \
        sc/source/filter/xcl97/xcl97rec \
))

$(eval $(call gb_Library_add_cxxobjects,scfilt,\
       sc/source/filter/lotus/optab \
))

