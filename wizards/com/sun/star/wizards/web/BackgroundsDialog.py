#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
import uno
import traceback

from .ImageListDialog import ImageListDialog
from .WWHID import HID_BG
from .WebWizardConst import *
from ..common.SystemDialog import SystemDialog
from ..common.FileAccess import FileAccess
from ..common.Configuration import Configuration

from com.sun.star.awt import Size

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
        if filename is not None and filename.length > 0 and filename[0] is not None:
            self.settings.cp_DefaultSession.cp_InDirectory = \
                FileAccess.getParentDir(filename[0])
            i = self.add(filename[0])
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
            if il.getListModel().getElementAt(i) == s:
                return i

            i += 1
        il.getListModel().addElement(s)
        try:
            configView = Configuration.getConfigurationRoot(
                self.xMSF, FileAccess.connectURLs(
                    CONFIG_PATH, "BackgroundImages"), True)
            i = Configuration.getChildrenNames(configView).length + 1
            o = Configuration.addConfigNode(configView, "" + i)
            Configuration.set(s, "Href", o)
            Configuration.commit(configView)
        except Exception:
            traceback.print_exc()

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
            sRetUrls = []
            if (listItem is not None):
                sRetUrls.append(listItem)
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
            except Exception:
                traceback.print_exc()

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
            except Exception:
                traceback.print_exc()

        '''
        if the given url is a directory
        adds the images in the given directory,
        otherwise (if it is a file) adds the file to the list.
        @param url
        '''

        def addDir(self, url):
            if self.fileAccess.isDirectory(url):
                self.add(self.fileAccess.listFiles(url, False))
            else:
                self.add(url)

        '''
        adds the given filenames (urls) to
        the list
        @param filenames
        '''

        def add(self, filenames):
            i = 0
            while i < len(filenames):
                self.add1(filenames[i])
                i += 1

        '''
        adds the given image url to the list.
        if and only if it ends with jpg, jpeg or gif
        (case insensitive)
        @param filename image url.
        '''

        def add1(self, filename):
            lcase = filename.toLowerCase()
            if lcase.endsWith("jpg") or lcase.endsWith("jpeg") or \
                    lcase.endsWith("gif"):
                Model.this.addElement(filename)
