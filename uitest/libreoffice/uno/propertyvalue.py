# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

try:
    import pyuno
    import uno
    import unohelper
except ImportError:
    print("pyuno not found: try to set PYTHONPATH and URE_BOOTSTRAP variables")
    print("PYTHONPATH=/installation/opt/program")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

def mkPropertyValue(name, value):
    """ Create a UNO PropertyValue from two input values.
    """
    return uno.createUnoStruct("com.sun.star.beans.PropertyValue",
            name, 0, value, 0)

def mkPropertyValues(vals):
    """ Create UNO property values from a map.
    """
    return tuple([mkPropertyValue(name, value) for (name, value) in vals.items()])

def convert_property_values_to_dict(propMap):
    """ Create a dictionary from a sequence of property values
    """
    ret = {}
    for entry in propMap:
        name = entry.Name
        val = entry.Value
        ret[name] = val

    return ret

# vim: set shiftwidth=4 softtabstop=4 expandtab:
