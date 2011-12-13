/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 * (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "sal/config.h"
#include "sal/precppunit.hxx"

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

// Just verify that all the generated headers can be included (i.e., that all
// the identifiers are valid and do not clash:
#include "officecfg/FirstStartWizard.hxx"
#include "officecfg/Inet.hxx"
#include "officecfg/Interaction.hxx"
#include "officecfg/LDAP.hxx"
#include "officecfg/Office/Accelerators.hxx"
#include "officecfg/Office/Addons.hxx"
#include "officecfg/Office/Calc.hxx"
#include "officecfg/Office/CalcAddIns.hxx"
#include "officecfg/Office/Canvas.hxx"
#include "officecfg/Office/Chart.hxx"
#include "officecfg/Office/Commands.hxx"
#include "officecfg/Office/Common.hxx"
#include "officecfg/Office/Compatibility.hxx"
#include "officecfg/Office/DataAccess.hxx"
#include "officecfg/Office/DataAccess/Drivers.hxx"
#include "officecfg/Office/Draw.hxx"
#include "officecfg/Office/Embedding.hxx"
#include "officecfg/Office/Events.hxx"
#include "officecfg/Office/ExtendedColorScheme.hxx"
#include "officecfg/Office/ExtensionManager.hxx"
#include "officecfg/Office/FormWizard.hxx"
#include "officecfg/Office/Histories.hxx"
#include "officecfg/Office/Impress.hxx"
#include "officecfg/Office/Java.hxx"
#include "officecfg/Office/Jobs.hxx"
#include "officecfg/Office/Labels.hxx"
#include "officecfg/Office/Linguistic.hxx"
#include "officecfg/Office/Logging.hxx"
#include "officecfg/Office/Math.hxx"
#include "officecfg/Office/OOoImprovement/Settings.hxx"
#include "officecfg/Office/OptionsDialog.hxx"
#include "officecfg/Office/Paths.hxx"
#include "officecfg/Office/ProtocolHandler.hxx"
#include "officecfg/Office/Recovery.hxx"
#include "officecfg/Office/SFX.hxx"
#include "officecfg/Office/Scripting.hxx"
#include "officecfg/Office/Security.hxx"
#include "officecfg/Office/Substitution.hxx"
#include "officecfg/Office/TabBrowse.hxx"
#include "officecfg/Office/TableWizard.hxx"
#include "officecfg/Office/TypeDetection.hxx"
#include "officecfg/Office/UI.hxx"
#include "officecfg/Office/UI/BaseWindowState.hxx"
#include "officecfg/Office/UI/BasicIDECommands.hxx"
#include "officecfg/Office/UI/BasicIDEWindowState.hxx"
#include "officecfg/Office/UI/BibliographyCommands.hxx"
#include "officecfg/Office/UI/BibliographyWindowState.hxx"
#include "officecfg/Office/UI/CalcCommands.hxx"
#include "officecfg/Office/UI/CalcWindowState.hxx"
#include "officecfg/Office/UI/Category.hxx"
#include "officecfg/Office/UI/ChartCommands.hxx"
#include "officecfg/Office/UI/ChartWindowState.hxx"
#include "officecfg/Office/UI/Commands.hxx"
#include "officecfg/Office/UI/Controller.hxx"
#include "officecfg/Office/UI/DbBrowserWindowState.hxx"
#include "officecfg/Office/UI/DbQueryWindowState.hxx"
#include "officecfg/Office/UI/DbRelationWindowState.hxx"
#include "officecfg/Office/UI/DbTableDataWindowState.hxx"
#include "officecfg/Office/UI/DbTableWindowState.hxx"
#include "officecfg/Office/UI/DbuCommands.hxx"
#include "officecfg/Office/UI/DrawImpressCommands.hxx"
#include "officecfg/Office/UI/DrawWindowState.hxx"
#include "officecfg/Office/UI/Effects.hxx"
#include "officecfg/Office/UI/Factories.hxx"
#include "officecfg/Office/UI/GenericCategories.hxx"
#include "officecfg/Office/UI/GenericCommands.hxx"
#include "officecfg/Office/UI/GlobalSettings.hxx"
#include "officecfg/Office/UI/ImpressWindowState.hxx"
#include "officecfg/Office/UI/MathCommands.hxx"
#include "officecfg/Office/UI/MathWindowState.hxx"
#include "officecfg/Office/UI/StartModuleCommands.hxx"
#include "officecfg/Office/UI/StartModuleWindowState.hxx"
#include "officecfg/Office/UI/WindowContentFactories.hxx"
#include "officecfg/Office/UI/WindowState.hxx"
#include "officecfg/Office/UI/WriterCommands.hxx"
#include "officecfg/Office/UI/WriterFormWindowState.hxx"
#include "officecfg/Office/UI/WriterGlobalWindowState.hxx"
#include "officecfg/Office/UI/WriterReportWindowState.hxx"
#include "officecfg/Office/UI/WriterWebWindowState.hxx"
#include "officecfg/Office/UI/WriterWindowState.hxx"
#include "officecfg/Office/UI/XFormsWindowState.hxx"
#include "officecfg/Office/Views.hxx"
#include "officecfg/Office/WebWizard.hxx"
#include "officecfg/Office/Writer.hxx"
#include "officecfg/Office/WriterWeb.hxx"
#include "officecfg/Setup.hxx"
#include "officecfg/System.hxx"
#include "officecfg/TypeDetection/Filter.hxx"
#include "officecfg/TypeDetection/GraphicFilter.hxx"
#include "officecfg/TypeDetection/Misc.hxx"
#include "officecfg/TypeDetection/Types.hxx"
#include "officecfg/TypeDetection/UISort.hxx"
#include "officecfg/UserProfile.hxx"
#include "officecfg/VCL.hxx"
#include "officecfg/ucb/Configuration.hxx"
#include "officecfg/ucb/Hierarchy.hxx"
#include "officecfg/ucb/InteractionHandler.hxx"
#include "officecfg/ucb/Store.hxx"

namespace {

class Test: public CppUnit::TestFixture {
    void test() {}

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
