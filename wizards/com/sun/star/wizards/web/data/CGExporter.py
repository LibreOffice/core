from common.ConfigSet import ConfigSet
from CGArgument import CGArgument
from common.ConfigGroup import ConfigGroup

class CGExporter(ConfigGroup):
    cp_Index = -1
    cp_Name = str()
    cp_ExporterClass = str()
    cp_OwnDirectory = bool()
    cp_SupportsFilename = bool()
    cp_DefaultFilename = str()
    cp_Extension = str()
    cp_SupportedMimeTypes = str()
    cp_Icon = str()
    cp_TargetType = str()
    cp_Binary = bool()
    cp_PageType = int()
    targetTypeName = ""
    cp_Arguments = ConfigSet(CGArgument())

    def supports(self, mime):
        return cp_SupportedMimeTypes == "" or \
            cp_SupportedMimeTypes.index(mime) > -1
