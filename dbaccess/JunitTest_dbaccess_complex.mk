# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,dbaccess_ju_complex))

$(eval $(call gb_JunitTest_use_jars,dbaccess_ju_complex,\
    ConnectivityTools \
    OOoRunner \
    jurt \
    ridl \
    test \
    unoil \
))

$(eval $(call gb_JunitTest_set_defs,dbaccess_ju_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/dbaccess/qa/dbaccess \
))

$(eval $(call gb_JunitTest_add_classes,dbaccess_ju_complex,\
    complex.dbaccess.Beamer \
    complex.dbaccess.PropertyBag \
))

$(eval $(call gb_JunitTest_add_sourcefiles,dbaccess_ju_complex,\
    dbaccess/qa/complex/dbaccess/ApplicationController \
    dbaccess/qa/complex/dbaccess/Beamer \
    dbaccess/qa/complex/dbaccess/CRMBasedTestCase \
    dbaccess/qa/complex/dbaccess/CopyTableInterActionHandler \
    dbaccess/qa/complex/dbaccess/CopyTableWizard \
    dbaccess/qa/complex/dbaccess/DataSource \
    dbaccess/qa/complex/dbaccess/DatabaseApplication \
    dbaccess/qa/complex/dbaccess/DatabaseDocument \
    dbaccess/qa/complex/dbaccess/FileHelper \
    dbaccess/qa/complex/dbaccess/Parser \
    dbaccess/qa/complex/dbaccess/PropertyBag \
    dbaccess/qa/complex/dbaccess/Query \
    dbaccess/qa/complex/dbaccess/QueryInQuery \
    dbaccess/qa/complex/dbaccess/RowSet \
    dbaccess/qa/complex/dbaccess/RowSetEventListener \
    dbaccess/qa/complex/dbaccess/SingleSelectQueryComposer \
    dbaccess/qa/complex/dbaccess/TestCase \
    dbaccess/qa/complex/dbaccess/UISettings \
))

# vim: set noet sw=4 ts=4:
