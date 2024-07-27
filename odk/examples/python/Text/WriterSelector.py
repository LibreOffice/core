# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import sys
import traceback
import officehelper


def main():
    try:
        remote_context = officehelper.bootstrap()
        print("Connected to a running office ...")
        srv_mgr = remote_context.getServiceManager()
        desktop = srv_mgr.createInstanceWithContext("com.sun.star.frame.Desktop", remote_context)

        print("Opening an empty Writer document")
        doc_url = "private:factory/swriter"
        doc = desktop.loadComponentFromURL(doc_url, "_blank", 0, tuple())

        text = doc.getText()
        text.setString("Please select something in this text and press then \"return\" in the shell "
                       "where you have started the example.\n")

        # Returned object supports service com.sun.star.text.TextDocumentView and com.sun.star.view.OfficeDocumentView
        # Both of them implements interface com::sun::star::view::XViewSettingsSupplier
        obj = doc.getCurrentController()
        obj.getViewSettings().setPropertyValue("ZoomType", 0)

        print()
        input("Please select something in the test document and press "
              "then \"return\" to continues the example ... ")

        frame = desktop.getCurrentFrame()
        selection = frame.getController().getSelection()

        if selection.supportsService("com.sun.star.text.TextRanges"):
            for selected in selection:
                print("You have selected a text range:", f'"{selected.getString()}".')

        if selection.supportsService("com.sun.star.text.TextGraphicObject"):
            print("You have selected a graphics.")

        if selection.supportsService("com.sun.star.text.TexttableCursor"):
            print("You have selected a text table.")

        # When object returned from loadComponentFromURL does not support a service
        # that implements XCloseable interface, fallback to call
        # XComponent.dispose.
        try:
            doc.close(False)
        except Exception:
            doc.dispose()
    except Exception:
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
