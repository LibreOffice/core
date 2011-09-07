from common.ConfigGroup import ConfigGroup
from common.ConfigSet import ConfigSet
from CGDocument import CGDocument

class CGContent(ConfigGroup):

    cp_Index = -1
    dirName = str()
    cp_Name = str()
    cp_Description = str()
    #COMMENTED
    #cp_Contents = ConfigSet(CGContent)
    cp_Documents = ConfigSet(CGDocument())

    def createDOM(self, parent):
        myElement = XMLHelper.addElement(
            parent, "content",
            ["name", "directory-name", "description", "directory"],
            [self.cp_Name, self.dirName, self.cp_Description, self.dirName])
        self.cp_Documents.createDOM(myElement)
        return myElement
