class ViewHandler(object):
    '''Creates a new instance of View '''

    def __init__ (self, xMSF, xTextDocument):
        self.xMSFDoc = xMSF
        self.xTextDocument = xTextDocument
        self.xTextViewCursorSupplier = self.xTextDocument.CurrentController

    def selectFirstPage(self, oTextTableHandler):
        try:
            xPageCursor = self.xTextViewCursorSupplier.ViewCursor
            xPageCursor.jumpToFirstPage()
            xPageCursor.jumpToStartOfPage()
            Helper.setUnoPropertyValue(
                xPageCursor, "PageDescName", "First Page")
            oPageStyles = self.xTextDocument.StyleFamilies.getByName(
                "PageStyles")
            oPageStyle = oPageStyles.getByName("First Page")
            xAllTextTables = oTextTableHandler.xTextTablesSupplier.TextTables
            xTextTable = xAllTextTables.getByIndex(0)
            xRange = xTextTable.getAnchor().getText()
            xPageCursor.gotoRange(xRange, False)
            if not com.sun.star.uno.AnyConverter.isVoid(XTextRange):
                xViewTextCursor.gotoRange(xHeaderRange, False)
                xViewTextCursor.collapseToStart()
            else:
                print "No Headertext available"

        except com.sun.star.uno.Exception, exception:
            exception.printStackTrace(System.out)

    def setViewSetting(self, Setting, Value):
        setattr(self.xTextViewCursorSupplier.ViewSettings, Setting, Value)

    def collapseViewCursorToStart(self):
        xTextViewCursor = self.xTextViewCursorSupplier.ViewCursor
        xTextViewCursor.collapseToStart()

