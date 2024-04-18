# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import traceback
import officehelper

from com.sun.star.container import NoSuchElementException

# This example demonstrates how to access and use the PathSettings service

# List of pre-defined path variables supported by the path substitution service.
predefined_path_properties = [
    "$(home)", "$(inst)", "$(prog)", "$(temp)", "$(user)",
    "$(username)", "$(work)", "$(path)", "$(langid)", "$(vlang)"
]

def work_with_path_settings():
    try:
        # Get a reference to the remote office context
        # If necessary, a new office process is started
        context = officehelper.bootstrap()
        print("Connected to a running office ...")

        # Get a reference to the service manager and path substitution service
        srv_mgr = context.getServiceManager()
        path_subst_service = srv_mgr.createInstanceWithContext(
            "com.sun.star.comp.framework.PathSubstitution", context)

        work_with_path_variables(path_subst_service)

    except Exception as e:
        print(str(e))
        traceback.print_exc()

def work_with_path_variables(path_subst_service):
    if path_subst_service is None:
        return

    # Retrieve values for path properties from path settings service
    for prop in predefined_path_properties:
        try:
            value = path_subst_service.getSubstituteVariableValue(prop)
            print(prop, '=', value)
        except NoSuchElementException as e:
            print("NoSuchElementException thrown when accessing", prop)
            print(str(e))
            traceback.print_exc()

    try:
        # Now try the resubstitution function
        path = path_subst_service.getSubstituteVariableValue(predefined_path_properties[0])
        path += "/test" # extend the path
        print("Path =", path)
        resubst_path = path_subst_service.reSubstituteVariables(path)
        print("Resubstituted path =", resubst_path)
    except NoSuchElementException as e:
        print("NoSuchElementException thrown when accessing", predefined_path_properties[0])
        print(str(e))
        traceback.print_exc()

if __name__ == "__main__":
    work_with_path_settings()

# vim: set shiftwidth=4 softtabstop=4 expandtab: