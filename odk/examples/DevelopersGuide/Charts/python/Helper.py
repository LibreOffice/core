import sys
import traceback

import officehelper


class Helper(object):
    def __init__(self):
        # connect to a running office and get the ServiceManager
        try:
            # get the remote office component context
            self._context = officehelper.bootstrap()
            print("Connected to a running office ...")
            # get the remote office service manager
            self._factory = self._context.getServiceManager()

        except Exception as err:
            print(f"Couldn't get ServiceManager: {err}")
            traceback.print_exc()
            sys.exit(1)

    def create_spreadsheet_document(self):
        return self._create_document("scalc")

    def create_drawing_document(self):
        return self._create_document("sdraw")

    def create_text_document(self):
        return self._create_document("swriter")

    def _create_document(self, doc_type):
        result = None
        try:
            desktop = self._factory.createInstanceWithContext("com.sun.star.frame.Desktop", self._context)
            result = desktop.loadComponentFromURL(f"private:factory/{doc_type}", "_blank", 0, tuple())
        except Exception as err:
            print(f"Couldn't create Document of type {doc_type}: {err}")
            traceback.print_exc()
            return

        return result

    def get_componentContext(self):
        return self._context
