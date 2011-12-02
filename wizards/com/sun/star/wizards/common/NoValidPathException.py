class NoValidPathException(Exception):

    def __init__(self, xMSF, _sText):
        super(NoValidPathException,self).__init__(_sText)
        # TODO: NEVER open a dialog in an exception
        from SystemDialog import SystemDialog
        if xMSF:
            SystemDialog.showErrorBox(xMSF,
                "dbwizres", "dbw", 521) #OfficePathnotavailable

