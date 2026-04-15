# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Configuration_Configuration,dict-pt-BR,nodeliver))

$(eval $(call gb_Configuration_add_schemas,dict-pt-BR,dictionaries/pt_BR/dialog/registry/schema, \
       org/openoffice/Lightproof_pt_BR.xcs \
))

$(eval $(call gb_Configuration_add_localized_datas,dict-pt-BR,dictionaries/pt_BR/dialog/registry/data, \
       org/openoffice/Office/OptionsDialog.xcu \
))

# vim: set noet sw=4 ts=4:
