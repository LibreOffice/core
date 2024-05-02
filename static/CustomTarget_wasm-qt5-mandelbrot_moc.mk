# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,static/qt5-mandelbrot))

$(call gb_CustomTarget_get_target,static/qt5-mandelbrot) : \
	$(gb_CustomTarget_workdir)/static/qt5-mandelbrot/renderthread.moc \
	$(gb_CustomTarget_workdir)/static/qt5-mandelbrot/mandelbrotwidget.moc \

qt5_mandelbrot_MOCDEFS_H := $(gb_CustomTarget_workdir)/static/qt5-mandelbrot/moc_predefs.h
qt5_mandelbrot_MOCDEFS_CXX := $(gb_CustomTarget_workdir)/static/qt5-mandelbrot/moc_dummy.cxx
qt5_mandelbrot_WORKDIR :=  $(gb_CustomTarget_workdir)/static/qt5-mandelbrot/.dir

$(qt5_mandelbrot_MOCDEFS_CXX): | $(qt5_mandelbrot_WORKDIR)
	touch $@

$(qt5_mandelbrot_MOCDEFS_H): $(qt5_mandelbrot_MOCDEFS_CXX) | $(qt5_mandelbrot_WORKDIR)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CXX,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),CXX)
	$(CXX) -pipe -O2 -std=gnu++11 -fno-exceptions $(gb_EMSCRIPTEN_CPPFLAGS) -dM -E -o $@ $<
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),CXX)

$(gb_CustomTarget_workdir)/static/qt5-mandelbrot/%.moc : \
		$(SRCDIR)/static/source/qt5-mandelbrot/%.h \
		$(qt5_mandelbrot_MOCDEFS_H) | $(qt5_mandelbrot_WORKDIR)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),MOC,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),MOC)
	$(MOC5) --include $(qt5_mandelbrot_MOCDEFS_H) $(gb_EMSCRIPTEN_QTDEFS) $< -o $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),MOC)

# vim: set noet sw=4:
