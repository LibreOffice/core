from common.ConfigGroup import ConfigGroup

class CGStyle(ConfigGroup):
    cp_Index = -1
    cp_Name = str()
    cp_CssHref = str()
    cp_BackgroundImage = str()
    cp_IconSet = str()

    def getBackgroundUrl(self):
        if CGStyle.cp_BackgroundImage is None \
                or CGStyle.cp_BackgroundImage == "":
            return None
        else:
            return CGStyle.cp_BackgroundImage
