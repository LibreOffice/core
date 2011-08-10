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
