from common.ConfigGroup import ConfigGroup

class CGDesign(ConfigGroup):

    cp_Layout = str()
    cp_Style = str()
    cp_BackgroundImage = str()
    cp_IconSet = str()
    cp_DisplayTitle = bool()
    cp_DisplayDescription = bool()
    cp_DisplayAuthor = bool()
    cp_DisplayCreateDate = bool()
    cp_DisplayUpdateDate = bool()
    cp_DisplayFilename = bool()
    cp_DisplayFileFormat = bool()
    cp_DisplayFormatIcon = bool()
    cp_DisplayPages = bool()
    cp_DisplaySize = bool()
    cp_OptimizeDisplaySize = int()

    def createDOM(self, parent):
        return XMLHelper.addElement(parent, "design", (0,), (0,))

    def getStyle(self):
        style = root.cp_Styles.getElement(self.cp_Style)
        return [root.cp_Styles.getIndexOf(style)]

    def setStyle(self, newStyle):
        o = root.cp_Styles.getElementAt(newStyle[0])
        self.cp_Style = root.cp_Styles.getKey(o)

    def getLayout(self):
        layout = root.cp_Layouts.getElement(self.cp_Layout)
        return layout.cp_Index

    def setLayout(self, layoutIndex):
        layout = root.cp_Layouts.getElementAt(layoutIndex)
        self.cp_Layout = root.cp_Layouts.getKey(layout)
