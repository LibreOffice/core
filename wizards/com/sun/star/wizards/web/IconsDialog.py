from ImageListDialog import *
from WWHID import HID_IS
from common.FileAccess import FileAccess

'''
@author rpiterman
The dialog class for choosing an icon set.
This class simulates a model, though it does not functions really as one,
since it does not cast events.
It also implements the ImageList.ImageRenderer interface, to handle
its own objects.
'''

class IconsDialog(ImageListDialog):

    def __init__(self, xmsf, set_, resources):
        super(IconsDialog, self).__init__(xmsf, HID_IS,
            (resources.resIconsDialog,
                resources.resIconsDialogCaption,
                resources.resOK,
                resources.resCancel,
                resources.resHelp,
                resources.resDeselect,
                resources.resOther,
                resources.resCounter))
        self.htmlexpDirectory = FileAccess.getOfficePath2(
            xmsf, "Gallery", "share", "")
        self.icons = \
            ["firs", "prev", "next", "last", "nav", "text", "up", "down"]
        self.set = set_
        self.objects = (self.set.getSize() * len(self.icons),)

        self.il.listModel = self
        self.il.renderer = self
        self.il.rows = 4
        self.il.cols = 8
        self.il.ImageSize = Size (20, 20)
        self.il.showButtons = False
        self.il.rowSelect = True
        self.il.scaleImages = False
        self.showDeselectButton = True
        self.showOtherButton = False
        self.build()

    def getIconset(self):
        if getSelected() == None:
            return None
        else:
            return self.set.getKey((getSelected()) / len(self.icons))

    def setIconset(self, iconset):
        #COMMENTED
        icon = 0 #self.set.getIndexOf(self.set.getElement(iconset)) * len(self.icons)
        aux = None
        if icon >=0:
            aux = self.objects[icon]
        self.setSelected(aux)

    def getSize(self):
        return self.set.getSize() * len(self.icons)

    def getElementAt(self, arg0):
        return self.objects[arg0]

    def getImageUrls(self, listItem):
        i = (listItem).intValue()
        iset = getIconsetNum(i)
        icon = getIconNum(i)
        sRetUrls = range(2)
        sRetUrls[0] = self.htmlexpDirectory + "/htmlexpo/" \
            + getIconsetPref(iset) + self.icons[icon] + getIconsetPostfix(iset)
        sRetUrls[1] = sRetUrls[0]
        return sRetUrls

    def render(self, object):
        if object == None:
            return ""

        i = (object).intValue()
        iset = getIconsetNum(i)
        return getIconset(iset).cp_Name

    def getIconsetNum(self, i):
        return i / self.icons.length

    def getIconNum(self, i):
        return i % self.icons.length

    def getIconsetPref(self, iconset):
        return getIconset(iconset).cp_FNPrefix

    def getIconsetPostfix(self, iconset):
        return getIconset(iconset).cp_FNPostfix

    def getIconset(self, i):
        return self.set.getElementAt(i)
