# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,desktop/soffice_bin-emscripten-exports))

$(eval $(call gb_CustomTarget_register_targets,desktop/soffice_bin-emscripten-exports, \
    exports \
))

$(gb_CustomTarget_workdir)/desktop/soffice_bin-emscripten-exports/exports: \
        $(SRCDIR)/desktop/util/Executable_soffice_bin-emscripten-exports \
        $(gb_CustomTarget_workdir)/bridges/gcc3_wasm/exports
	# Prior to <https://github.com/emscripten-core/emscripten/commit/
	# 7baa690ed4bf801a411130c7137ce830b93120f3> "Allow comments in response files. (#21330)" towards
	# emsdk 3.1.54, comment lines were not supported, so filter them out here for now:
	grep -v '^#' $(SRCDIR)/desktop/util/Executable_soffice_bin-emscripten-exports >$@
	cat $(gb_CustomTarget_workdir)/bridges/gcc3_wasm/exports >>$@
ifeq ($(ENABLE_QT6)-$(ENABLE_EMSCRIPTEN_JSPI)-$(ENABLE_EMSCRIPTEN_PROXY_TO_PTHREAD),TRUE-TRUE-)
	printf '__ZN10emscripten8internal13MethodInvokerINS0_3rvp11default_tagEMN7qstdweb13EventListenerEFvNS_3valEEvPS5_JS6_EE6invokeERKS8_S9_PNS_7_EM_VALE\n' >>$@
endif

# vim: set noet sw=4 ts=4:
