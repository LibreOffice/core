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
    dbaccess/uiconfig/ui/appborderwindow \
    dbaccess/uiconfig/ui/appdetailwindow \
    dbaccess/uiconfig/ui/applycolpage \
    dbaccess/uiconfig/ui/appswapwindow \
    dbaccess/uiconfig/ui/authentificationpage \
    dbaccess/uiconfig/ui/autocharsetpage \
    dbaccess/uiconfig/ui/choosedatasourcedialog \
    dbaccess/uiconfig/ui/colcontrolbox \
    dbaccess/uiconfig/ui/collectionviewdialog \
    dbaccess/uiconfig/ui/colwidthdialog \
    dbaccess/uiconfig/ui/connectionpage \
    dbaccess/uiconfig/ui/copytablepage \
    dbaccess/uiconfig/ui/dbaseindexdialog \
    dbaccess/uiconfig/ui/dbasepage \
    dbaccess/uiconfig/ui/dbtreelist \
    dbaccess/uiconfig/ui/dbwizconnectionpage \
    dbaccess/uiconfig/ui/dbwizmysqlintropage \
    dbaccess/uiconfig/ui/dbwizmysqlnativepage \
    dbaccess/uiconfig/ui/dbwizspreadsheetpage \
    dbaccess/uiconfig/ui/dbwiztextpage \
    dbaccess/uiconfig/ui/deleteallrowsdialog \
    dbaccess/uiconfig/ui/designsavemodifieddialog \
    dbaccess/uiconfig/ui/detailwindow \
    dbaccess/uiconfig/ui/directsqldialog  \
    dbaccess/uiconfig/ui/emptypage \
    dbaccess/uiconfig/ui/fielddialog \
    dbaccess/uiconfig/ui/fielddescpage \
    dbaccess/uiconfig/ui/fielddescpanel \
    dbaccess/uiconfig/ui/finalpagewizard \
    dbaccess/uiconfig/ui/generalpagedialog \
    dbaccess/uiconfig/ui/generalpagewizard \
    dbaccess/uiconfig/ui/generalspecialjdbcdetailspage \
    dbaccess/uiconfig/ui/generatedvaluespage \
    dbaccess/uiconfig/ui/indexdesigndialog \
    dbaccess/uiconfig/ui/jdbcconnectionpage \
    dbaccess/uiconfig/ui/joindialog \
    dbaccess/uiconfig/ui/jointablemenu \
    dbaccess/uiconfig/ui/joinviewmenu \
    dbaccess/uiconfig/ui/keymenu \
    dbaccess/uiconfig/ui/ldapconnectionpage \
    dbaccess/uiconfig/ui/ldappage \
    dbaccess/uiconfig/ui/limitbox \
    dbaccess/uiconfig/ui/mysqlnativepage \
    dbaccess/uiconfig/ui/mysqlnativesettings \
    dbaccess/uiconfig/ui/namematchingpage \
    dbaccess/uiconfig/ui/odbcpage \
    dbaccess/uiconfig/ui/parametersdialog \
    dbaccess/uiconfig/ui/password \
    dbaccess/uiconfig/ui/querycolmenu \
    dbaccess/uiconfig/ui/queryfilterdialog \
    dbaccess/uiconfig/ui/queryfuncmenu \
    dbaccess/uiconfig/ui/querypropertiesdialog  \
    dbaccess/uiconfig/ui/queryview \
    dbaccess/uiconfig/ui/relationdialog \
    dbaccess/uiconfig/ui/rowheightdialog \
    dbaccess/uiconfig/ui/saveindexdialog \
    dbaccess/uiconfig/ui/savedialog \
    dbaccess/uiconfig/ui/savemodifieddialog \
    dbaccess/uiconfig/ui/specialjdbcconnectionpage \
    dbaccess/uiconfig/ui/specialsettingspage \
    dbaccess/uiconfig/ui/sortdialog \
    dbaccess/uiconfig/ui/sqlexception \
    dbaccess/uiconfig/ui/tableborderwindow \
    dbaccess/uiconfig/ui/tabledesignrowmenu \
    dbaccess/uiconfig/ui/tabledesignsavemodifieddialog \
    dbaccess/uiconfig/ui/tablelistbox \
    dbaccess/uiconfig/ui/tablesfilterdialog \
    dbaccess/uiconfig/ui/tablesfilterpage \
    dbaccess/uiconfig/ui/tablesjoindialog \
    dbaccess/uiconfig/ui/taskwindow \
    dbaccess/uiconfig/ui/textconnectionsettings \
    dbaccess/uiconfig/ui/textpage \
    dbaccess/uiconfig/ui/titlewindow \
    dbaccess/uiconfig/ui/typeselectpage \
    dbaccess/uiconfig/ui/useradmindialog \
    dbaccess/uiconfig/ui/useradminpage \
    dbaccess/uiconfig/ui/userdetailspage \
    dbaccess/uiconfig/ui/migrwarndlg \
))

# vim: set noet sw=4 ts=4:
