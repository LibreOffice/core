# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from com.sun.star.awt import Size

# Bug 170190 - Form based filter reads wrong filter string
class tdf170190(UITestCase):
    def test_tdf170190(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            # a form on the Bibliography table, filtered by two conditions on the same column
            xForm = document.createInstance("com.sun.star.form.component.DataForm")
            xForm.DataSourceName = "Bibliography"
            xForm.Command = "biblio"
            xForm.CommandType = 0  # TABLE
            xForm.Filter = "\"Title\" LIKE 'A%' AND \"Title\" LIKE '%e'"
            xForm.ApplyFilter = True
            document.DrawPage.Forms.insertByName("Form", xForm)

            xTextField = document.createInstance("com.sun.star.form.component.TextField")
            xTextField.DataField = "Title"
            xForm.insertByName("Title", xTextField)
            xShape = document.createInstance("com.sun.star.drawing.ControlShape")
            xShape.Size = Size(5000, 1000)
            xShape.Control = xTextField
            document.DrawPage.add(xShape)

            xController = document.CurrentController
            xController.setFormDesignMode(False)
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.waitUntilAllIdlesDispatched()
            self.assertTrue(xForm.isLoaded())

            # start the form based filter, which shows the current filter in the controls
            xController.getControl(xTextField).setFocus()
            xToolkit.waitUntilAllIdlesDispatched()
            self.xUITest.executeCommand(".uno:FormFilter")
            xToolkit.waitUntilAllIdlesDispatched()

            aPredicates = xController.getFormController(xForm).getPredicateExpressions()
            self.xUITest.executeCommand(".uno:FormFilterExit")

            # Before the fix: AssertionError: "LIKE 'A%' AND LIKE '%e'" != "LIKE 'A%' AND '%e'"
            self.assertEqual(1, len(aPredicates))
            self.assertEqual(1, len(aPredicates[0]))
            self.assertEqual("LIKE 'A%' AND LIKE '%e'", " ".join(aPredicates[0][0].split()))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
