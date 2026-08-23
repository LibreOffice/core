# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import convert_property_values_to_dict, mkPropertyValues
from contextlib import contextmanager
import org.libreoffice.unotest
import pathlib

def get_state_as_dict(ui_object):
    return convert_property_values_to_dict(ui_object.getState())

def type_text(ui_object, text):
    ui_object.executeAction("TYPE", mkPropertyValues({"TEXT": text}))

def select_pos(ui_object, pos):
    assert isinstance(pos, str), "select_pos: POS must be of type str"
    ui_object.executeAction("SELECT", mkPropertyValues({"POS": pos}))

def select_by_text(ui_object, text):
    ui_object.executeAction("SELECT", mkPropertyValues({"TEXT": text}))

def select_text(ui_object, from_pos, to):
    ui_object.executeAction("SELECT", mkPropertyValues({"FROM": from_pos, "TO": to}))

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

# FieldUnit, see include/tools/fldunit.hxx
MEASUREMENT_UNITS = {
    'Millimeter': 1,
    'Centimeter': 2,
    'Point': 6,
    'Pica': 7,
    'Inch': 8,
}

# Writer keeps a single value; the others keep a metric and a non metric one and
# choose between them by locale, so both are set. The configuration items are
# notified of the change, so the running module picks it up.
MEASUREMENT_UNIT_PATHS = (
    '/org.openoffice.Office.Writer/Layout/Other/MeasureUnit',
    '/org.openoffice.Office.WriterWeb/Layout/Other/MeasureUnit',
    '/org.openoffice.Office.Calc/Layout/Other/MeasureUnit/Metric',
    '/org.openoffice.Office.Calc/Layout/Other/MeasureUnit/NonMetric',
    '/org.openoffice.Office.Impress/Layout/Other/MeasureUnit/Metric',
    '/org.openoffice.Office.Impress/Layout/Other/MeasureUnit/NonMetric',
    '/org.openoffice.Office.Draw/Layout/Other/MeasureUnit/Metric',
    '/org.openoffice.Office.Draw/Layout/Other/MeasureUnit/NonMetric',
)

# Impress and Draw do not follow the configuration, the unit reaches their
# module item pool only through a dispatch, see SdModule::Execute. The value to
# dispatch is the one of the Impress item.
IMPRESS_MEASUREMENT_UNIT_PATH = '/org.openoffice.Office.Impress/Layout/Other/MeasureUnit/Metric'

def set_measurement_unit(UITestCase, values):
    # Dispatch first: the applications that do follow the configuration are
    # notified afterwards and so have the last word on their own item.
    UITestCase.xUITest.executeCommandWithParameters(
        ".uno:MetricUnit",
        mkPropertyValues({"MetricUnit": values[IMPRESS_MEASUREMENT_UNIT_PATH]}))
    for path, value in values.items():
        UITestCase.ui_test.set_config_value(path, value)

# Sets the measurement unit and puts the old values back afterwards.
@contextmanager
def change_measurement_unit(UITestCase, unit):
    old_values = {}
    for path in MEASUREMENT_UNIT_PATHS:
        value = UITestCase.ui_test.get_config(path)
        # Writer keeps no default, and an item without a value leaves the last
        # one in place, so go back to the inch SfxModule::GetFieldUnit falls
        # back to.
        old_values[path] = MEASUREMENT_UNITS['Inch'] if value is None else value

    set_measurement_unit(
        UITestCase, dict.fromkeys(MEASUREMENT_UNIT_PATHS, MEASUREMENT_UNITS[unit]))
    try:
        yield
    finally:
        set_measurement_unit(UITestCase, old_values)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
