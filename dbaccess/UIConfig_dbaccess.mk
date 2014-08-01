# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,dbaccess))

$(eval $(call gb_UIConfig_add_uifiles,dbaccess, \
    dbaccess/uiconfig/ui/admindialog \
    dbaccess/uiconfig/ui/advancedsettingsdialog \
    dbaccess/uiconfig/ui/applycolpage \
    dbaccess/uiconfig/ui/backuppage \
    dbaccess/uiconfig/ui/choosedatasourcedialog \
    dbaccess/uiconfig/ui/collectionviewdialog \
    dbaccess/uiconfig/ui/colwidthdialog \
    dbaccess/uiconfig/ui/connectionpage \
    dbaccess/uiconfig/ui/copytablepage \
    dbaccess/uiconfig/ui/dbaseindexdialog \
    dbaccess/uiconfig/ui/dbwizmysqlintropage \
    dbaccess/uiconfig/ui/dbwizspreadsheetpage \
    dbaccess/uiconfig/ui/dbwiztextpage \
    dbaccess/uiconfig/ui/directsqldialog  \
    dbaccess/uiconfig/ui/emptypage \
    dbaccess/uiconfig/ui/fielddialog \
    dbaccess/uiconfig/ui/finalpagewizard \
    dbaccess/uiconfig/ui/generalpagedialog \
    dbaccess/uiconfig/ui/generalpagewizard \
    dbaccess/uiconfig/ui/generatedvaluespage \
    dbaccess/uiconfig/ui/indexdesigndialog \
    dbaccess/uiconfig/ui/joindialog \
    dbaccess/uiconfig/ui/migratepage \
    dbaccess/uiconfig/ui/password \
    dbaccess/uiconfig/ui/preparepage \
    dbaccess/uiconfig/ui/queryfilterdialog \
    dbaccess/uiconfig/ui/querypropertiesdialog  \
    dbaccess/uiconfig/ui/relationdialog \
    dbaccess/uiconfig/ui/rowheightdialog \
    dbaccess/uiconfig/ui/savedialog \
    dbaccess/uiconfig/ui/specialsettingspage \
    dbaccess/uiconfig/ui/sortdialog \
    dbaccess/uiconfig/ui/sqlexception \
    dbaccess/uiconfig/ui/summarypage \
    dbaccess/uiconfig/ui/tablesfilterdialog \
    dbaccess/uiconfig/ui/tablesfilterpage \
    dbaccess/uiconfig/ui/tablesjoindialog \
    dbaccess/uiconfig/ui/textconnectionsettings \
    dbaccess/uiconfig/ui/textpage \
    dbaccess/uiconfig/ui/useradmindialog \
))

# vim: set noet sw=4 ts=4:
