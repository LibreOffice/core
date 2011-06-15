from com.sun.star.awt.VclWindowPeerAttribute import OK
from Configuration import Configuration
from SystemDialog import SystemDialog
import traceback

class Resource(object):
    '''
    Creates a new instance of Resource
    @param _xMSF
    @param _Unit
    @param _Module
    '''

    @classmethod
    def __init__(self, _xMSF, _Module):
        self.xMSF = _xMSF
        self.Module = _Module
        try:
            xResource = self.xMSF.createInstanceWithArguments(
                "org.libreoffice.resource.ResourceIndexAccess", (self.Module,))
            if xResource is None:
                raise Exception ("could not initialize ResourceIndexAccess")

            self.xStringIndexAccess = xResource.getByName("String")
            self.xStringListIndexAccess = xResource.getByName("StringList")

            if self.xStringListIndexAccess is None:
                raise Exception ("could not initialize xStringListIndexAccess")

            if self.xStringIndexAccess is None:
                raise Exception ("could not initialize xStringIndexAccess")

        except Exception, exception:
            traceback.print_exc()
            self.showCommonResourceError(self.xMSF)

    def getResText(self, nID):
        try:
            return self.xStringIndexAccess.getByIndex(nID)
        except Exception, exception:
            traceback.print_exc()
            raise ValueError("Resource with ID not " + str(nID) + " not found")

    def getStringList(self, nID):
        try:
            return self.xStringListIndexAccess.getByIndex(nID)
        except Exception, exception:
            traceback.print_exc()
            raise ValueError("Resource with ID not " + str(nID) + " not found")

    def getResArray(self, nID, iCount):
        try:
            ResArray = range(iCount)
            i = 0
            while i < iCount:
                ResArray[i] = getResText(nID + i)
                i += 1
            return ResArray
        except Exception, exception:
            traceback.print_exc()
            raise ValueError("Resource with ID not" + str(nID) + " not found")

    @classmethod
    def showCommonResourceError(self, xMSF):
        ProductName = Configuration.getProductName(xMSF)
        sError = "The files required could not be found.\n" + \
            "Please start the %PRODUCTNAME Setup and choose 'Repair'."
        sError = sError.replace("%PRODUCTNAME", ProductName)
        SystemDialog.showMessageBox(xMSF, "ErrorBox", OK, sError)

