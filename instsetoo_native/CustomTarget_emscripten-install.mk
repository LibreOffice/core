# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,instsetoo_native/emscripten-install))

$(if $(or $(gb_not $(filter emscripten,$(PKGFORMAT))),$(filter-out emscripten,$(PKGFORMAT))), \
    $(error Emscripten only supports --with-package-format=emscripten))

emscripten_install_files := \
    favicon.ico \
    soffice.data \
    soffice.data.js.metadata \
    soffice.js \
    soffice.wasm \
    $(if $(EMSCRIPTEN_WORKERJS),soffice.worker.js) \
    $(if $(ENABLE_SYMBOLS_FOR),soffice.wasm.dwp) \
    $(if $(DISABLE_GUI),, \
        qt_soffice.html \
        qtloader.js \
        qtlogo.svg) \

.PHONY: $(call gb_CustomTarget_get_target,instsetoo_native/emscripten-install)
$(call gb_CustomTarget_get_target,instsetoo_native/emscripten-install): \
    | $(call gb_Postprocess_get_target,AllModulesButInstsetNative)
	mkdir -p $(WORKDIR)/installation/LibreOffice/emscripten
	for i in $(emscripten_install_files); do \
        cp $(INSTDIR)/program/$$i $(WORKDIR)/installation/LibreOffice/emscripten/ || exit 1; \
    done

# vim: set noet sw=4 ts=4:
