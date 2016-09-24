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
import traceback
import importlib

from .WebWizardConst import *
from ..common.UCB import UCB
from ..common.FileAccess import FileAccess
from ..ui.event.Task import Task
from ..ui.event.CommonListener import StreamListenerProcAdapter
from .ProcessErrors import ProcessErrors
from .ExtensionVerifier import ExtensionVerifier
from .ErrorHandler import ErrorHandler
from .data.CGDocument import CGDocument

from com.sun.star.io import IOException
from com.sun.star.uno import SecurityException
from com.sun.star.beans import NamedValue
from com.sun.star.beans import StringPair

# This class is used to process a CGSession object
# and generate a site. </br>
# it does the following: <br/>
# 1. create a temporary directory.<br/>
# 2. export documents to the temporary directory.<br/>
# 3. generate the TOC page, includes copying images from the
# web wizard work directory and other layout files.<br/>
# 4. publish, or copy, from the temporary directory to
# different destinations.<br/>
# 5. delete the temporary directory.<br/>
# <br/>
# to follow up the status/errors it uses a TaskListener object,
# and an ErrorHandler. <br/>
# in practice, the TaskListener is the status dialog,
# and the Errorhandler does the interaction with the user,
# if something goes wrong.<br/>
# Note that this class takes it in count that
# the given session object is prepared for it -
# all preparations are done in WWD_Events.finishWizard methods.
# <br/>
# <br/>
#
# note on error handling: <br/>
# on "catch" clauses I tries to decide whether the
# exception is fatal or not. For fatal exception an error message
# is displayed (or rather: the errorHandler is being called...)
# and a false is returned.
# In less-fatal errors, the errorHandler "should decide" which means,
# the user is given the option to "OK" or to "Cancel" and depending
# on that interaction I cary on.
class Process(ProcessErrors):

    TASKS_PER_DOC = 5
    TASKS_PER_XSL = 2
    TASKS_PER_PUBLISH = 2
    TASKS_IN_PREPARE = 1
    TASKS_IN_EXPORT = 2
    TASKS_IN_GENERATE = 2
    TASKS_IN_PUBLISH = 2
    TASKS_IN_FINISHUP = 1
    settings = None
    xmsf = None
    errorHandler = None
    tempDir = None
    fileAccess = None
    ucb = None
    myTask = None
    #This is a cache for exporters, so I do not need to
    #instanciate the same exporter more than once.
    exporters = {}
    result = None

    def __init__(self, settings, xmsf, er):
        self.xmsf = xmsf
        self.settings = settings
        self.fileAccess = FileAccess(xmsf)
        self.errorHandler = er

        self.ucb = UCB(xmsf)

        self.taskSteps = self.getTaskSteps()
        self.myTask = Task(TASK, TASK_PREPARE, self.taskSteps)

    # @return to how many destinations should the
    # generated site be published.
    def countPublish(self):
        count = 0
        publishers = self.settings.cp_DefaultSession.cp_Publishing
        for e in publishers.childrenList:
            if e.cp_Publish:
                count += 1
        return count

    # @return the number of task steps that this
    # session should have
    def getTaskSteps(self):
        docs = self.settings.cp_DefaultSession.cp_Content.cp_Documents.getSize()
        xsl = 0
        try:
            layout = self.settings.cp_DefaultSession.getLayout()
            xsl = len(layout.getTemplates(self.xmsf))
        except Exception:
            traceback.print_exc()

        publish = self.countPublish()
        return \
                self.TASKS_IN_PREPARE + \
                self.TASKS_IN_EXPORT + docs * self.TASKS_PER_DOC + \
                self.TASKS_IN_GENERATE + xsl * self.TASKS_PER_XSL + \
                self.TASKS_IN_PUBLISH + publish * self.TASKS_PER_PUBLISH + \
                self.TASKS_IN_FINISHUP

    # does the job
    def runProcess(self):
        self.myTask.start()
        try:
            try:
                # I use here '&&' so if one of the
                # methods returns false, the next
                # will not be called.
                self.result = self.createTempDir(self.myTask) and self.export(self.myTask) and self.generate(self.tempDir, self.myTask) and self.publish(self.tempDir, self.myTask)
            finally:
                # cleanup must be called.
                self.result = self.result and self.cleanup(self.myTask)
        except Exception:
            traceback.print_exc()
            self.result = False

        if not self.result:
            # this is a bug protection.
            self.myTask.fail()

        while (self.myTask.getStatus() < self.myTask.getMax()):
            self.myTask.advance(True)

    # creates a temporary directory.
    # @param task
    # @return true should continue
    def createTempDir(self, task):
        try:
            self.tempDir = self.fileAccess.createNewDir(self.getSOTempDir(self.xmsf), "/wwiztemp")
        except Exception:
            traceback.print_exc()
        if self.tempDir is None:
            self.error(None, None, ProcessErrors.ERROR_MKDIR, ErrorHandler.ERROR_PROCESS_FATAL)
            return False
        else:
            task.advance(True)
            return True

    # @param xmsf
    # @return the staroffice /openoffice temporary directory
    def getSOTempDir(self, xmsf):
        try:
            return FileAccess.getOfficePath(self.xmsf, "Temp", "")
        except Exception:
            traceback.print_exc()
        return None

    # CLEANUP

    # delete the temporary directory
    # @return true should continue
    def cleanup(self, task):
        task.setSubtaskName(TASK_FINISH)
        b = self.fileAccess.delete(self.tempDir)
        if not b:
            self.error(None, None, ProcessErrors.ERROR_CLEANUP, ErrorHandler.ERROR_WARNING)
        task.advance(b)
        return b

    # This method is used to copy style files to a target
    # Directory: css and background.
    # Note that this method is static since it is
    # also used when displaying a "preview"
    def copyMedia(self, copy, settings, targetDir, task):
        # 1. .css
        sourceDir = FileAccess.connectURLs(settings.workPath, "styles")
        filename = settings.cp_DefaultSession.getStyle().cp_CssHref
        copy.copy2(sourceDir, filename, targetDir, "style.css")

        task.advance(True)

        # 2. background image
        background = settings.cp_DefaultSession.cp_Design.cp_BackgroundImage
        if (background is not None and background is not ""):
            sourceDir = FileAccess.getParentDir(background)
            filename = background[len(sourceDir):]
            copy.copy2(sourceDir, filename, targetDir + "/images", "background.gif")

        task.advance(True)

    # Copy "static" files (which are always the same,
    # thus not user-input-dependent) to a target directory.
    # Note that this method is static since it is
    # also used when displaying a "preview"
    # @param copy
    # @param settings
    # @param targetDir
    # @throws Exception
    @classmethod
    def copyStaticImages(self, copy, settings, targetDir):
        source = FileAccess.connectURLs(settings.workPath, "images")
        target = targetDir + "/images"
        copy.copy(source, target)

    # publish the given directory.
    # @param dir the source directory to publish from
    # @param task task tracking.
    # @return true if should continue
    def publish(self, folder, task):
        task.setSubtaskName(TASK_PUBLISH_PREPARE)
        configSet = self.settings.cp_DefaultSession.cp_Publishing
        try:
            self.copyMedia(self.ucb, self.settings, folder, task)
            self.copyStaticImages(self.ucb, self.settings, folder)
            task.advance(True)
        except Exception as ex:
            # error in copying media
            traceback.print_exc()
            self.error(ex, "", ProcessErrors.ERROR_PUBLISH_MEDIA, ErrorHandler.ERROR_PROCESS_FATAL)
            return False
        for p in configSet.childrenList:
            if p.cp_Publish:
                key = configSet.getKey(p)
                task.setSubtaskName(key)
                if key is ZIP_PUBLISHER:
                    self.fileAccess.delete(p.cp_URL)
                if (not self.publish1(folder, p, self.ucb, task)):
                    return False
        return True

    # publish the given directory to the
    # given target CGPublish.
    # @param dir the dir to copy from
    # @param publish the object that specifies the target
    # @param copy ucb encapsulation
    # @param task task tracking
    # @return true if should continue
    def publish1(self, folder, publish, copy, task):
        try:
            task.advance(True)
            url = publish.url
            copy.copy(folder, url)
            task.advance(True)
            return True
        except Exception as e:
            task.advance(False)
            traceback.print_exc()
            return self.error(e, publish, ProcessErrors.ERROR_PUBLISH, ErrorHandler.ERROR_NORMAL_IGNORE)

    # Generates the TOC pages for the current session.
    # @param targetDir generating to this directory.
    def generate(self, targetDir, task):
        result = False
        task.setSubtaskName(TASK_GENERATE_PREPARE)


        layout = self.settings.cp_DefaultSession.getLayout()

        try:
            # here I create the DOM of the TOC to pass to the XSL
            doc = self.settings.cp_DefaultSession.createDOM1()
            self.generate1(self.xmsf, layout, doc, self.fileAccess, targetDir, task)
        except Exception as ex:
            traceback.print_exc()
            self.error(ex, "", ProcessErrors.ERROR_GENERATE_XSLT, ErrorHandler.ERROR_PROCESS_FATAL)
            return False

        # copy files which are not xsl from layout directory to
        # website root.
        try:
            task.setSubtaskName(TASK_GENERATE_COPY)

            self.copyLayoutFiles(self.ucb, self.fileAccess, self.settings, layout, targetDir)

            task.advance(True)

            result = True
        except Exception as ex:
            task.advance(False)
            traceback.print_exc()
            return self.error(ex, None, ProcessErrors.ERROR_GENERATE_COPY, ErrorHandler.ERROR_NORMAL_ABORT)
        return result

    # copies layout files which are not .xsl files
    # to the target directory.
    # @param ucb UCB encapsulation object
    # @param fileAccess filaAccess encapsulation object
    # @param settings web wizard settings
    # @param layout the layout object
    # @param targetDir the target directory to copy to
    # @throws Exception
    @classmethod
    def copyLayoutFiles(self, ucb, fileAccess, settings, layout, targetDir):
        filesPath = fileAccess.getURL(FileAccess.connectURLs(settings.workPath, "layouts/"), layout.cp_FSName)
        ucb.copy1(filesPath, targetDir, ExtensionVerifier("xsl"))

    # generates the TOC page for the given layout.
    # This method might generate more than one file, depending
    # on how many .xsl files are in the
    # directory specifies by the given layout object.
    # @param xmsf
    # @param layout specifies the layout to use.
    # @param doc the DOM representation of the web wizard session
    # @param fileAccess encapsulation of FileAccess
    # @param targetPath target directory
    # @param task
    # @throws Exception
    @classmethod
    def generate1(self, xmsf, layout, doc, fileAccess, targetPath, task):
        # a map that contains xsl templates. the keys are the xsl file names.
        templates = layout.getTemplates(xmsf)
        self.node = doc

        task.advance1(True, TASK_GENERATE_XSL)

        # each template generates a page.
        for key in templates:
            temp = templates[key]

            # The target file name is like the xsl template filename
            # without the .xsl extension.
            fn = fileAccess.getPath(targetPath, key[:len(key) - 4])

            args = list(range(1))
            nv = NamedValue()
            nv.Name = "StylesheetURL"
            nv.Value = temp
            args[0] = nv
            arguments = list(range(1))
            arguments[0] = tuple(args)

            self.tf = Process.createTransformer(xmsf, arguments)

            self.node.normalize()
            task.advance(True)

            # we want to be notfied when the processing is done...
            self.tf.addListener(StreamListenerProcAdapter(self,
                                                          self.streamTerminatedHandler,
                                                          self.streamStartedHandler,
                                                          self.streamClosedHandler,
                                                          self.streamErrorHandler))

            # create pipe
            pipeout = xmsf.createInstance("com.sun.star.io.Pipe")
            pipein = pipeout

            # connect sax writer to pipe
            self.xSaxWriter = xmsf.createInstance( "com.sun.star.xml.sax.Writer" )
            self.xSaxWriter.setOutputStream(pipeout)

            # connect pipe to transformer
            self.tf.setInputStream(pipein)

            # connect transformer to output
            xOutputStream = fileAccess.xInterface.openFileWrite(fn)
            self.tf.setOutputStream(xOutputStream)

            self.tf.start()
            while (not self.tfCompleted):
                pass
            self.tf.terminate()
            task.advance(True)


    @classmethod
    def createTransformer(self, xmsf, args):
        tf = xmsf.createInstanceWithArguments("com.sun.star.xml.xslt.XSLT2Transformer",
                                              tuple(args))
        if (tf is None):
            # TODO: put a dialog telling about the need to install
            # xslt2-transformer extension here
            tf = xmsf.createInstanceWithArguments("com.sun.star.xml.xslt.XSLTTransformer",
                                                  tuple(args))
        return tf

    def streamTerminatedHandler(self):
        parent.isTerminated = True

    def streamStartedHandler(self, parent):
        parent.tfCompleted = False
        parent.node.serialize(parent.xSaxWriter, tuple([StringPair()]))

    def streamErrorHandler(self, aException):
        print ("DEBUG !!! Stream 'error' event handler")

    def streamClosedHandler(self, parent):
        parent.tfCompleted = True

    # I broke the export method to two methods
    # in a time where a tree with more than one contents was planned.
    # I left it that way, because it may be used in the future.
    # @param task
    # @return
    def export(self, task):
        return self.export1(self.settings.cp_DefaultSession.cp_Content, self.tempDir, task)

    # This method could actually, with light modification, use recursion.
    # In the present situation, where we only use a "flat" list of
    # documents, instead of the original plan to use a tree,
    # the recursion is not implemented.
    # @param content the content ( directory-like, contains documents) 
    # @param dir (target directory for exporting this content.
    # @param task
    # @return true if should continue
    def export1(self, content, folder, task):
        toPerform = 1
        contentDir = None

        try:
            task.setSubtaskName(TASK_EXPORT_PREPARE)

            # 1. create a content directory.
            # each content (at the moment there is only one :-( )
            # is created in its own directory.
            # faileure here is fatal.
            contentDir = self.fileAccess.createNewDir(folder, content.cp_Name);
            if (contentDir is None or contentDir is ""):
                raise IOException("Directory " + folder + " could not be created.")

            content.dirName = FileAccess.getFilename(contentDir)

            task.advance1(True, TASK_EXPORT_DOCUMENTS)
            toPerform -= 1

            # 2. export all documents and sub contents.
            # (at the moment, only documents, no subcontents)
            for item in content.cp_Documents.childrenList:
                try:
                    #
                    # In present this is always the case.
                    # may be in the future, when
                    # a tree is used, it will be a bit different.
                    if (isinstance (item, CGDocument)):
                        if (not self.export2(item, contentDir, task)):
                            return False
                    elif (not self.export2(item, contentDir, task)):
                    # we never get here since we
                    # did not implement sub-contents.
                        return False
                except SecurityException as sx:
                    # nonfatal
                    traceback.print_exc()
                    if (not self.error(sx, item, ProcessErrors.ERROR_EXPORT_SECURITY, ErrorHandler.ERROR_NORMAL_IGNORE)):
                        return False
                    self.result = False
        except IOException as iox:
            # nonfatal
            traceback.print_exc()
            return self.error(iox, content, ProcessErrors.ERROR_EXPORT_IO, ErrorHandler.ERROR_NORMAL_IGNORE)
        except SecurityException as se:
            # nonfatal
            traceback.print_exc()
            return self.error(se, content, ProcessErrors.ERROR_EXPORT_SECURITY, ErrorHandler.ERROR_NORMAL_IGNORE)

        self.failTask(task, toPerform)
        return True

    # exports a single document
    # @param doc the document to export
    # @param dir the target directory
    # @param task task tracking
    # @return true if should continue
    def export2(self, doc, folder, task):
        # first I check if the document was already validated...
        if (not doc.valid):
            try:
                doc.validate(self.xmsf, task)
            except Exception as ex:
                # fatal
                traceback.print_exc()
                self.error(ex, doc, ProcessErrors.ERROR_DOC_VALIDATE, ErrorHandler.ERROR_PROCESS_FATAL)
                return False
        # get the exporter specified for this document
        exp = doc.cp_Exporter
        exporter = self.settings.cp_Exporters.getElement(exp)

        try:
             # here I calculate the destination filename.
             # I take the original filename (docFilename), subtract the extension, (docExt) -> (fn)
             # and find an available filename which starts with
             # this filename, but with the new extension. (destExt)
            docFilename = FileAccess.getFilename(doc.cp_URL)

            docExt = FileAccess.getExtension(docFilename)
            # filename without extension
            #fn = doc.localFilename.substring(0, doc.localFilename.length() - docExt.length() - 1)
            fn = doc.localFilename[:len(doc.localFilename) - len(docExt) - 1]

            # the copyExporter does not change
            # the extension of the target...
            destExt = FileAccess.getExtension(docFilename) \
                if (exporter.cp_Extension is "") \
                else exporter.cp_Extension

            # if this filter needs to export to its own directory...
            # this is the case in, for example, impress html export
            if (exporter.cp_OwnDirectory):
                # +++
                folder = self.fileAccess.createNewDir(folder, fn)
                doc.dirName = FileAccess.getFilename(folder)

            # if two files with the same name
            # need to be exported ? So here
            # I get a new filename, so I do not
            # overwrite files...
            f = self.fileAccess.getNewFile(folder, fn, destExt)


            # set filename with extension.
            # this will be used by the exporter,
            # and to generate the TOC.
            doc.urlFilename = FileAccess.getFilename(f)

            task.advance(True)

            try:
                # export
                self.getExporter(exporter).export(doc, f, self.xmsf, task)
                task.advance(True)
             # getExporter(..) throws
             # IllegalAccessException, InstantiationException, ClassNotFoundException
             # export() throws Exception
            except Exception as ex:
                # nonfatal
                traceback.print_exc()
                if (not self.error(ex, doc, ProcessErrors.ERROR_EXPORT, ErrorHandler.ERROR_NORMAL_IGNORE)):
                    return False
        except Exception as ex:
            # nonfatal
            traceback.print_exc()
            if (not self.error(ex, doc, ProcessErrors.ERROR_EXPORT_MKDIR, ErrorHandler.ERROR_NORMAL_ABORT)):
                return False

        return True

    # submit an error.
    # @param ex the exception
    # @param arg1 error argument
    # @param arg2 error argument 2
    # @param errType error type
    # @return the interaction result
    def error(self, ex, arg1, arg2, errType):
        self.result = False
        return self.errorHandler.error(ex, arg1, arg2, errType)


    # advances the given task in the given count of steps,
    # marked as failed.
    # @param task the task to advance
    # @param count the number of steps to advance
    def failTask(self, task, count):
        while (count > 0):
            task.advance(False)
            count -= 1

    # creates an instance of the exporter class
    # as specified by the
    # exporter object.
    # @param export specifies the exporter to be created
    # @return the Exporter instance
    # @throws ClassNotFoundException
    # @throws IllegalAccessException
    # @throws InstantiationException
    def createExporter(self, export):
        pkgname = ".".join(export.cp_ExporterClass.split(".")[3:])
        className = export.cp_ExporterClass.split(".")[-1]
        mod = importlib.import_module(pkgname)
        return getattr(mod, className)(export)

    # searches the an exporter for the given CGExporter object
    # in the cache.
    # If its not there, creates it, stores it in the cache and
    # returns it.
    # @param export specifies the needed exporter.
    # @return an Exporter instance
    # @throws ClassNotFoundException thrown when using Class.forName(string)
    # @throws IllegalAccessException thrown when using Class.forName(string)
    # @throws InstantiationException thrown when using Class.forName(string)
    def getExporter(self, export):
        exp = self.exporters.get(export.cp_Name)
        if (exp is None):
            exp = self.createExporter(export)
            self.exporters[export.cp_Name] = exp
        return exp

    # @return true if everything went smooth, false
    # if error(s) accured.
    def getResult(self):
        return (self.myTask.getFailed() == 0) and self.result
