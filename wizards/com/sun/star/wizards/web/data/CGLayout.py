from common.ConfigGroup import ConfigGroup
from ui.UIConsts import RID_IMG_WEB

class CGLayout(ConfigGroup):

    cp_Index = -1
    cp_Name = str()
    cp_FSName = str()

    def createTemplates(self, xmsf):
        self.templates = {}
        tf = TransformerFactory.newInstance()
        workPath = getSettings().workPath
        fa = FileAccess(xmsf)
        stylesheetPath = fa.getURL(
            getSettings().workPath, "layouts/" + self.cp_FSName)
        files = fa.listFiles(stylesheetPath, False)
        i = 0
        while i < files.length:
            if FileAccess.getExtension(files[i]).equals("xsl"):
                self.templates.put(
                    FileAccess.getFilename(files[i]),
                    tf.newTemplates(StreamSource (files[i])))
            i += 1

    def getImageUrls(self):
        sRetUrls = range(1)
        ResId = RID_IMG_WEB + (self.cp_Index * 2)
        return [ResId, ResId + 1]

    def getTemplates(self, xmsf):
        self.createTemplates(xmsf)
        return self.templates
