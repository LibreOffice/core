from common.ConfigGroup import ConfigGroup
from common.ConfigSet import ConfigSet
from CGContent import CGContent
from CGDesign import CGDesign
from CGGeneralInfo import CGGeneralInfo
from CGPublish import CGPublish

class CGSession(ConfigGroup):

    cp_Index = -1
    cp_InDirectory = str()
    cp_OutDirectory = str()
    cp_Name = str()
    cp_Content = CGContent()
    cp_Design = CGDesign()
    cp_GeneralInfo = CGGeneralInfo()
    cp_Publishing = ConfigSet(CGPublish())
    valid = False

    def createDOM(self, parent):
        root = XMLHelper.addElement(
            parent, "session", ["name", "screen-size"],
            [self.cp_Name, getScreenSize()])
        self.cp_GeneralInfo.createDOM(root)
        self.cp_Content.createDOM(root)
        return root

    def getScreenSize(self):
        tmp_switch_var1 = self.cp_Design.cp_OptimizeDisplaySize
        if tmp_switch_var1 == 0:
            return "640"
        elif tmp_switch_var1 == 1:
            return "800"
        elif tmp_switch_var1 == 2:
            return "1024"
        else:
            return "800"

    def getLayout(self):
        return self. root.cp_Layouts.getElement(self.cp_Design.cp_Layout)

    def getStyle(self):
        return self.root.cp_Styles.getElement(self.cp_Design.cp_Style)

    def createDOM(self):
        factory = DocumentBuilderFactory.newInstance()
        doc = factory.newDocumentBuilder().newDocument()
        createDOM(doc)
        return doc
