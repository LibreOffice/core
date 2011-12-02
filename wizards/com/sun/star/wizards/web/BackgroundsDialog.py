from ImageListDialog import *
from WWHID import HID_BG
from common.SystemDialog import SystemDialog
from common.FileAccess import FileAccess

class BackgroundsDialog(ImageListDialog):

    def __init__(self, xmsf, set_, resources):
        super(BackgroundsDialog, self).__init__(xmsf, HID_BG,
            (resources.resBackgroundsDialog,
                resources.resBackgroundsDialogCaption,
                resources.resOK,
                resources.resCancel,
                resources.resHelp,
                resources.resDeselect,
                resources.resOther,
                resources.resCounter))
        self.sd = SystemDialog.createOpenDialog(xmsf)
        self.sd.addFilter(
            resources.resImages, "*.jpg;*.jpeg;*.jpe;*.gif", True)
        self.sd.addFilter(resources.resAllFiles, "*.*", False)
        self.settings = set_.root
        self.fileAccess = FileAccess(xmsf)
        #COMMENTED
        #self.il.setListModel(Model(set_))
        self.il.imageSize = Size (40, 40)
        #self.il.setRenderer(BGRenderer (0))
        self.build()

    '''
    trigered when the user clicks the "other" button.
    opens a "file open" dialog, adds the selected
    image to the list and to the web wizard configuration,
    and then jumps to the new image, selecting it in the list.
    @see add(String)
    '''

    def other(self):
        filename = self.sd.callOpenDialog(
            False, self.settings.cp_DefaultSession.cp_InDirectory)
        if filename != None and filename.length > 0 and filename[0] != None:
            self.settings.cp_DefaultSession.cp_InDirectory = \
                FileAccess.getParentDir(filename[0])
            i = add(filename[0])
            il.setSelected(i)
            il.display(i)

    '''
    adds the given image to the image list (to the model)
    and to the web wizard configuration.
    @param s
    @return
    '''

    def add(self, s):
        #first i check the item does not already exists in the list...
        i = 0
        while i < il.getListModel().getSize():
            if il.getListModel().getElementAt(i).equals(s):
                return i

            i += 1
        il.getListModel().addElement(s)
        try:
            configView = Configuration.getConfigurationRoot(
                self.xMSF, FileAccess.connectURLs(
                    WebWizardConst.CONFIG_PATH, "BackgroundImages"), True)
            i = Configuration.getChildrenNames(configView).length + 1
            o = Configuration.addConfigNode(configView, "" + i)
            Configuration.set(s, "Href", o)
            Configuration.commit(configView)
        except Exception, ex:
            ex.printStackTrace()

        return il.getListModel().getSize() - 1

    '''
    an ImageList Imagerenderer implemtation.
    The image URL is the object given from the list model.
    the image name, got from the "render" method is
    the filename portion of the url.
    @author rpiterman
    '''

    class BGRenderer(object):

        def __init__(self, cut_):
            ImageListDialog.ImageListDialog_body()
            self.cut = cut_

        def getImageUrls(self, listItem):
            if listItem != None:
                sRetUrls = range(1)
                sRetUrls[0] = listItem
                return sRetUrls

            return None

        def render(self, _object):
            if _object is None:
                return ""
            else:
                return FileAccess.getPathFilename(
                    self.fileAccess.getPath(_object, None))

    '''
    This is a list model for the image list of the
    backgrounds dialog.
    It takes the Backgrounds config set as an argument,
    and "parses" it to a list of files:
    It goes through each image in the set, and checks it:
    if it is a directory it lists all image files in this directory.
    if it is a file, it adds the file to the list.
    @author rpiterman
    '''

    class Model(object):
        '''
        constructor. </br>
        see class description for a description of
        the handling of the given model
        @param model the configuration set of the background images.
        '''

        def __init__(self, model):
            try:
                i = 0
                while i < model.getSize():
                    image = model.getElementAt(i)
                    path = self.sd.xStringSubstitution.substituteVariables(
                        image.cp_Href, False)
                    if self.fileAccess.exists(path, False):
                        addDir(path)
                    else:
                        remove(model.getKey(image))

                    i += 1
            except Exception, ex:
                ex.printStackTrace()

        '''
        when instanciating the model, it checks if each image
        exists. If it doesnot, it will be removed from
        the configuration.
        This is what this method does...
        @param imageName
        '''

        def remove(self, imageName):
            try:
                conf = Configuration.getConfigurationRoot(
                    self.xMSF, WebWizardConst.CONFIG_PATH + "/BackgroundImages",
                    True)
                Configuration.removeNode(conf, imageName)
            except Exception, ex:
                ex.printStackTrace()

        '''
        if the given url is a directory
        adds the images in the given directory,
        otherwise (if it is a file) adds the file to the list.
        @param url
        '''

        def addDir(self, url):
            if self.fileAccess.isDirectory(url):
                add(self.fileAccess.listFiles(url, False))
            else:
                add(url)

        '''
        adds the given filenames (urls) to
        the list
        @param filenames
        '''

        def add(self, filenames):
            i = 0
            while i < filenames.length:
                add(filenames[i])
                i += 1

        '''
        adds the given image url to the list.
        if and only if it ends with jpg, jpeg or gif
        (case insensitive)
        @param filename image url.
        '''

        def add(self, filename):
            lcase = filename.toLowerCase()
            if lcase.endsWith("jpg") or lcase.endsWith("jpeg") or \
                    lcase.endsWith("gif"):
                Model.this.addElement(filename)
