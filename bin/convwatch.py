# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Conversion watch, initially intended to detect if document layout changed since the last time it was run.
#
# Print a set of docs, compare the pdf against the old run and highlight the differences
#

import getopt
import os
import subprocess
import sys
import time
import uuid
import datetime
import traceback
import threading
try:
    from urllib.parse import quote
except ImportError:
    from urllib import quote

try:
    import pyuno
    import uno
    import unohelper
except ImportError:
    print("pyuno not found: try to set PYTHONPATH and URE_BOOTSTRAP variables")
    print("PYTHONPATH=/installation/opt/program")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

try:
    from com.sun.star.document import XDocumentEventListener
except ImportError:
    print("UNO API class not found: try to set URE_BOOTSTRAP variable")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

### utilities ###

def log(*args):
    print(*args, flush=True)

def partition(list, pred):
    left = []
    right = []
    for e in list:
        if pred(e):
            left.append(e)
        else:
            right.append(e)
    return (left, right)

def filelist(dir, suffix):
    if len(dir) == 0:
        raise Exception("filelist: empty directory")
    if not(dir[-1] == "/"):
        dir += "/"
    files = [dir + f for f in os.listdir(dir)]
#    log(files)
    return [f for f in files
                    if os.path.isfile(f) and os.path.splitext(f)[1] == suffix]

def getFiles(dirs, suffix):
    files = []
    for dir in dirs:
        files += filelist(dir, suffix)
    return files

### UNO utilities ###

class OfficeConnection:
    def __init__(self, args):
        self.args = args
        self.soffice = None
        self.socket = None
        self.xContext = None
    def setUp(self):
        (method, sep, rest) = self.args["--soffice"].partition(":")
        if sep != ":":
            raise Exception("soffice parameter does not specify method")
        if method == "path":
                self.socket = "pipe,name=pytest" + str(uuid.uuid1())
                try:
                    userdir = self.args["--userdir"]
                except KeyError:
                    raise Exception("'path' method requires --userdir")
                if not(userdir.startswith("file://")):
                    raise Exception("--userdir must be file URL")
                self.soffice = self.bootstrap(rest, userdir, self.socket)
        elif method == "connect":
                self.socket = rest
        else:
            raise Exception("unsupported connection method: " + method)
        self.xContext = self.connect(self.socket)

    def bootstrap(self, soffice, userdir, socket):
        argv = [ soffice, "--accept=" + socket + ";urp",
                "-env:UserInstallation=" + userdir,
                "--quickstart=no",
                "--norestore", "--nologo", "--headless" ]
        if "--valgrind" in self.args:
            argv.append("--valgrind")
        return subprocess.Popen(argv)

    def connect(self, socket):
        xLocalContext = uno.getComponentContext()
        xUnoResolver = xLocalContext.ServiceManager.createInstanceWithContext(
                "com.sun.star.bridge.UnoUrlResolver", xLocalContext)
        url = "uno:" + socket + ";urp;StarOffice.ComponentContext"
        log("OfficeConnection: connecting to: " + url)
        while True:
            try:
                xContext = xUnoResolver.resolve(url)
                return xContext
#            except com.sun.star.connection.NoConnectException
            except pyuno.getClass("com.sun.star.connection.NoConnectException"):
                log("NoConnectException: sleeping...")
                time.sleep(1)

    def tearDown(self):
        if self.soffice:
            if self.xContext:
                try:
                    log("tearDown: calling terminate()...")
                    xMgr = self.xContext.ServiceManager
                    xDesktop = xMgr.createInstanceWithContext(
                            "com.sun.star.frame.Desktop", self.xContext)
                    xDesktop.terminate()
                    log("...done")
#                except com.sun.star.lang.DisposedException:
                except pyuno.getClass("com.sun.star.beans.UnknownPropertyException"):
                    log("caught UnknownPropertyException")
                    pass # ignore, also means disposed
                except pyuno.getClass("com.sun.star.lang.DisposedException"):
                    log("caught DisposedException")
                    pass # ignore
            else:
                self.soffice.terminate()
            ret = self.soffice.wait()
            self.xContext = None
            self.socket = None
            self.soffice = None
            if ret != 0:
                raise Exception("Exit status indicates failure: " + str(ret))
#            return ret

class WatchDog(threading.Thread):
    def __init__(self, connection):
        threading.Thread.__init__(self, name="WatchDog " + connection.socket)
        self.connection = connection
    def run(self):
        try:
            if self.connection.soffice: # not possible for "connect"
                self.connection.soffice.wait(timeout=120) # 2 minutes?
        except subprocess.TimeoutExpired:
            log("WatchDog: TIMEOUT -> killing soffice")
            self.connection.soffice.terminate() # actually killing oosplash...
            self.connection.xContext = None
            log("WatchDog: killed soffice")

class PerTestConnection:
    def __init__(self, args):
        self.args = args
        self.connection = None
        self.watchdog = None
    def getContext(self):
        return self.connection.xContext
    def setUp(self):
        assert(not(self.connection))
    def preTest(self):
        conn = OfficeConnection(self.args)
        conn.setUp()
        self.connection = conn
        self.watchdog = WatchDog(self.connection)
        self.watchdog.start()
    def postTest(self):
        if self.connection:
            try:
                self.connection.tearDown()
            finally:
                self.connection = None
                self.watchdog.join()
    def tearDown(self):
        assert(not(self.connection))

class PersistentConnection:
    def __init__(self, args):
        self.args = args
        self.connection = None
    def getContext(self):
        return self.connection.xContext
    def setUp(self):
        conn = OfficeConnection(self.args)
        conn.setUp()
        self.connection = conn
    def preTest(self):
        assert(self.connection)
    def postTest(self):
        assert(self.connection)
    def tearDown(self):
        if self.connection:
            try:
                self.connection.tearDown()
            finally:
                self.connection = None

def simpleInvoke(connection, test):
    try:
        connection.preTest()
        test.run(connection.getContext())
    finally:
        connection.postTest()

def retryInvoke(connection, test):
    tries = 5
    while tries > 0:
        try:
            tries -= 1
            try:
                connection.preTest()
                test.run(connection.getContext())
                return
            finally:
                connection.postTest()
        except KeyboardInterrupt:
            raise # Ctrl+C should work
        except:
            log("retryInvoke: caught exception")
    raise Exception("FAILED retryInvoke")

def runConnectionTests(connection, invoker, tests):
    try:
        connection.setUp()
        failed = []
        for test in tests:
            try:
                invoker(connection, test)
            except KeyboardInterrupt:
                raise # Ctrl+C should work
            except:
                failed.append(test.file)
                estr = traceback.format_exc()
                log("... FAILED with exception:\n" + estr)
        return failed
    finally:
        connection.tearDown()

class EventListener(XDocumentEventListener,unohelper.Base):
    def __init__(self):
        self.layoutFinished = False
    def documentEventOccured(self, event):
#        log(str(event.EventName))
        if event.EventName == "OnLayoutFinished":
            self.layoutFinished = True
    def disposing(event):
        pass

def mkPropertyValue(name, value):
    return uno.createUnoStruct("com.sun.star.beans.PropertyValue",
            name, 0, value, 0)

### tests ###

def loadFromURL(xContext, url):
    xDesktop = xContext.ServiceManager.createInstanceWithContext(
            "com.sun.star.frame.Desktop", xContext)
    props = [("Hidden", True), ("ReadOnly", True)] # FilterName?
    loadProps = tuple([mkPropertyValue(name, value) for (name, value) in props])
    xListener = EventListener()
    xGEB = xContext.getValueByName(
        "/singletons/com.sun.star.frame.theGlobalEventBroadcaster")
    xGEB.addDocumentEventListener(xListener)
    xDoc = None
    try:
        xDoc = xDesktop.loadComponentFromURL(url, "_blank", 0, loadProps)
        log("...loadComponentFromURL done")
        if xDoc is None:
            raise Exception("No document loaded?")
        time_ = 0
        while time_ < 30:
            if xListener.layoutFinished:
                return xDoc
            log("delaying...")
            time_ += 1
            time.sleep(1)
        log("timeout: no OnLayoutFinished received")
        return xDoc
    except:
        if xDoc:
            log("CLOSING")
            xDoc.close(True)
        raise
    finally:
        if xListener:
            xGEB.removeDocumentEventListener(xListener)

def printDoc(xContext, xDoc, url):
    props = [ mkPropertyValue("FileName", url) ]
# xDoc.print(props)
    uno.invoke(xDoc, "print", (tuple(props),)) # damn, that's a keyword!
    busy = True
    while busy:
        log("printing...")
        time.sleep(1)
        prt = xDoc.getPrinter()
        for value in prt:
            if value.Name == "IsBusy":
                busy = value.Value
    log("...done printing")

class LoadPrintFileTest:
    def __init__(self, file, prtsuffix):
        self.file = file
        self.prtsuffix = prtsuffix
    def run(self, xContext):
        start = datetime.datetime.now()
        log("Time: " + str(start) + " Loading document: " + self.file)
        xDoc = None
        try:
            if os.name == 'nt' and self.file[1] == ':':
                url = "file:///" + self.file[0:2] + quote(self.file[2:])
            else:
                url = "file://" + quote(self.file)
            xDoc = loadFromURL(xContext, url)
            log("loadFromURL in: "  + str(datetime.datetime.now() - start))
            printDoc(xContext, xDoc, url + self.prtsuffix)
        finally:
            if xDoc:
                xDoc.close(True)
            end = datetime.datetime.now()
            log("...done with: " + self.file + " in: " + str(end - start))

def runLoadPrintFileTests(opts, dirs, suffix, reference):
    if reference:
        prtsuffix = ".pdf.reference"
    else:
        prtsuffix = ".pdf"
    files = getFiles(dirs, suffix)
    tests = (LoadPrintFileTest(file, prtsuffix) for file in files)
#    connection = PersistentConnection(opts)
    connection = PerTestConnection(opts)
    failed = runConnectionTests(connection, simpleInvoke, tests)
    print("all printed: FAILURES: " + str(len(failed)))
    for fail in failed:
        print(fail)
    return failed

def mkImages(file, resolution):
    argv = [ "gs", "-r" + resolution, "-sOutputFile=" + file + ".%04d.jpeg",
             "-dNOPROMPT", "-dNOPAUSE", "-dBATCH", "-sDEVICE=jpeg", file ]
    ret = subprocess.check_call(argv)

def mkAllImages(dirs, suffix, resolution, reference, failed):
    if reference:
        prtsuffix = ".pdf.reference"
    else:
        prtsuffix = ".pdf"
    for dir in dirs:
        files = filelist(dir, suffix)
        log(files)
        for f in files:
            if f in failed:
                log("Skipping failed: " + f)
            else:
                mkImages(f + prtsuffix, resolution)

def identify(imagefile):
    argv = ["identify", "-format", "%k", imagefile]
    process = subprocess.Popen(argv, stdout=subprocess.PIPE)
    result, _ = process.communicate()
    if process.wait() != 0:
        raise Exception("identify failed")
    if result.partition(b"\n")[0] != b"1":
        log("identify result: " + result.decode('utf-8'))
        log("DIFFERENCE in " + imagefile)

def compose(refimagefile, imagefile, diffimagefile):
    argv = [ "composite", "-compose", "difference",
            refimagefile, imagefile, diffimagefile ]
    subprocess.check_call(argv)

def compareImages(file):
    allimages = [f for f in filelist(os.path.dirname(file), ".jpeg")
                   if f.startswith(file)]
#    refimages = [f for f in filelist(os.path.dirname(file), ".jpeg")
#                   if f.startswith(file + ".reference")]
#    log("compareImages: allimages:" + str(allimages))
    (refimages, images) = partition(sorted(allimages),
            lambda f: f.startswith(file + ".pdf.reference"))
#    log("compareImages: images" + str(images))
    for (image, refimage) in zip(images, refimages):
        compose(image, refimage, image + ".diff")
        identify(image + ".diff")
    if (len(images) != len(refimages)):
        log("DIFFERENT NUMBER OF IMAGES FOR: " + file)

def compareAllImages(dirs, suffix):
    log("compareAllImages...")
    for dir in dirs:
        files = filelist(dir, suffix)
#        log("compareAllImages:" + str(files))
        for f in files:
            compareImages(f)
    log("...compareAllImages done")


def parseArgs(argv):
    (optlist,args) = getopt.getopt(argv[1:], "hr",
            ["help", "soffice=", "userdir=", "reference", "valgrind"])
#    print optlist
    return (dict(optlist), args)

def usage():
    message = """usage: {program} [option]... [directory]..."
 -h | --help:      print usage information
 -r | --reference: generate new reference files (otherwise: compare)
 --soffice=method:location
                   specify soffice instance to connect to
                   supported methods: 'path', 'connect'
 --userdir=URL     specify user installation directory for 'path' method
 --valgrind        pass --valgrind to soffice for 'path' method"""
    print(message.format(program = os.path.basename(sys.argv[0])))

def checkTools():
    try:
        subprocess.check_output(["gs", "--version"])
    except:
        print("Cannot execute 'gs'. Please install ghostscript.")
        sys.exit(1)
    try:
        subprocess.check_output(["composite", "-version"])
        subprocess.check_output(["identify", "-version"])
    except:
        print("Cannot execute 'composite' or 'identify'.")
        print("Please install ImageMagick.")
        sys.exit(1)

if __name__ == "__main__":
    checkTools()
    (opts,args) = parseArgs(sys.argv)
    if len(args) == 0:
        usage()
        sys.exit(1)
    if "-h" in opts or "--help" in opts:
        usage()
        sys.exit()
    elif "--soffice" in opts:
        reference = "-r" in opts or "--reference" in opts
        failed = runLoadPrintFileTests(opts, args, ".odt", reference)
        mkAllImages(args, ".odt", "200", reference, failed)
        if not(reference):
            compareAllImages(args, ".odt")
    else:
        usage()
        sys.exit(1)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
