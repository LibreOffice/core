# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import pathlib
import subprocess
import time
import uuid
import argparse
import os
import shutil
import urllib.parse
import urllib.request

try:
    import pyuno
    import uno
except ImportError:
    print("pyuno not found: try to set PYTHONPATH and URE_BOOTSTRAP variables")
    print("to something like:")
    print("  PYTHONPATH=/installation/opt/program")
    print("  URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

try:
    from com.sun.star.document import XDocumentEventListener
except ImportError:
    print("UNO API class not found: try to set URE_BOOTSTRAP variable")
    print("to something like:")
    print("  URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

try:
    from urllib.parse import quote
except ImportError:
    from urllib import quote

### utilities ###

def mkPropertyValue(name, value):
    return uno.createUnoStruct("com.sun.star.beans.PropertyValue", name, 0, value, 0)

def mkPropertyValues(**kwargs):
    '''mkPropertyValues(Name=Value, Name=Value,...) -> (PropertyValue, PropertyValue,...)
    ex. : mkPropertyValues(Hidden=True, ReadOnly=False)'''
    from com.sun.star.beans import PropertyValue
    return tuple(PropertyValue(k,0,kwargs[k],0) for k in kwargs)

def fileUrlToSystemPath(url):
    return pyuno.fileUrlToSystemPath(url)

def systemPathToFileUrl(systemPath):
    return pyuno.systemPathToFileUrl(systemPath)

### UNO utilities ###

class OfficeConnection(object):

    def __init__(self, args):
        self.args = args
        self.soffice = None
        self.xContext = None
        self.channel = None

    def setUp(self):
        try:
            self.verbose = self.args["verbose"]
        except KeyError:
            self.verbose = False
        try:
            prog = self.args["program"]
        except KeyError:
            prog = os.getenv("SOFFICE_BIN")
        if not (prog):
            raise Exception("SOFFICE_BIN must be set")
        channel = "pipe,name=pytest" + str(uuid.uuid1())
        try:
            userdir = self.args["userdir"]
        except KeyError:
            userdir = "file:///tmp"
        if not(userdir.startswith("file://")):
            raise Exception("--userdir must be file URL")
        self.soffice = self.bootstrap(prog, userdir, channel)
        return self.connect(channel)

    def bootstrap(self, soffice, userdir, channel):
        argv = [ soffice, "--accept=" + channel + ";urp",
                "-env:UserInstallation=" + userdir,
                "--quickstart=no",
                "--norestore", "--nologo", "--headless"]
        if "--valgrind" in self.args:
            argv.append("--valgrind")
        if self.verbose:
            print ("starting LibreOffice with channel: ", channel)
        return subprocess.Popen(argv)

    def connect(self, channel):
        xLocalContext = uno.getComponentContext()
        xUnoResolver = xLocalContext.ServiceManager.createInstanceWithContext(
                "com.sun.star.bridge.UnoUrlResolver", xLocalContext)
        url = ("uno:%s;urp;StarOffice.ComponentContext" % channel)
        if self.verbose:
            print("Connecting to: ", url)
        while True:
            try:
                self.xContext = xUnoResolver.resolve(url)
                return self.xContext
#            except com.sun.star.connection.NoConnectException
            except pyuno.getClass("com.sun.star.connection.NoConnectException"):
                print("WARN: NoConnectException: sleeping...")
                time.sleep(1)

    def tearDown(self):
        if self.soffice:
            if self.xContext:
                try:
                    if self.verbose:
                        print("tearDown: calling terminate()...")
                    xMgr = self.xContext.ServiceManager
                    xDesktop = xMgr.createInstanceWithContext(
                            "com.sun.star.frame.Desktop", self.xContext)
                    xDesktop.terminate()
                    if self.verbose:
                        print("...done")
#                except com.sun.star.lang.DisposedException:
                except pyuno.getClass("com.sun.star.beans.UnknownPropertyException"):
                    print("caught UnknownPropertyException")
                    pass # ignore, also means disposed
                except pyuno.getClass("com.sun.star.lang.DisposedException"):
                    print("caught DisposedException")
                    pass # ignore
            else:
                self.soffice.terminate()
            ret = self.soffice.wait()
            self.xContext = None
            self.socket = None
            self.soffice = None
            if ret != 0:
                raise Exception("Exit status indicates failure: " + str(ret))

    def getContext(self):
        return self.xContext

class UnoRemoteConnection:
    def __init__(self, args):
        self.args = args
        self.connection = None
    def getContext(self):
        return self.connection.xContext
    def getDoc(self):
        return self.xDoc
    def setUp(self):
        conn = OfficeConnection(self.args)
        conn.setUp()
        self.connection = conn
    def openEmptyWriterDoc(self):
        assert(self.connection)
        smgr = self.getContext().ServiceManager
        desktop = smgr.createInstanceWithContext("com.sun.star.frame.Desktop", self.getContext())
        props = [("Hidden", True), ("ReadOnly", False)]
        loadProps = tuple([mkPropertyValue(name, value) for (name, value) in props])
        self.xDoc = desktop.loadComponentFromURL("private:factory/swriter", "_blank", 0, loadProps)
        return self.xDoc

    def checkProperties(self, obj, dict, test):
        for k,v in dict.items():
            obj.setPropertyValue(k, v)
            value = obj.getPropertyValue(k)
            test.assertEqual(value, v)

    def postTest(self):
        assert(self.connection)
    def tearDown(self):
        if self.connection:
            try:
                self.connection.tearDown()
            finally:
                self.connection = None

havePonies = False

class UnoInProcess:
    def getContext(self):
        return self.xContext
    def getDoc(self):
        return self.xDoc
    def setUp(self):
        self.xContext = pyuno.getComponentContext()
        global havePonies
        if not(havePonies):
            pyuno.private_initTestEnvironment()
            havePonies = True

    def openEmptyWriterDoc(self):
        return self.openEmptyDoc("private:factory/swriter")

    def openEmptyCalcDoc(self):
        return self.openEmptyDoc("private:factory/scalc")

    def openEmptyDoc(self, url, bHidden = True, bReadOnly = False):
        props = [("Hidden", bHidden), ("ReadOnly", bReadOnly)]
        return self.__openDocFromURL(url, props)

    def openTemplateFromTDOC(self, file):
        return self.openDocFromTDOC(file, True)

    def openDocFromTDOC(self, file, asTemplate = False):
        path = makeCopyFromTDOC(file)
        return self.openDocFromAbsolutePath(path, asTemplate)

    def openDocFromAbsolutePath(self, file, asTemplate = False):
        return self.openDocFromURL(pathlib.Path(file).as_uri(), asTemplate)

    def openDocFromURL(self, url, asTemplate = False):
        props = [("Hidden", True), ("ReadOnly", False), ("AsTemplate", asTemplate)]
        return self.__openDocFromURL(url, props)

    def __openDocFromURL(self, url, props):
        assert(self.xContext)
        smgr = self.getContext().ServiceManager
        desktop = smgr.createInstanceWithContext("com.sun.star.frame.Desktop", self.getContext())
        loadProps = tuple([mkPropertyValue(name, value) for (name, value) in props])
        self.xDoc = desktop.loadComponentFromURL(url, "_blank", 0, loadProps)
        assert(self.xDoc)
        return self.xDoc

    def checkProperties(self, obj, dict, test):
        for k,v in dict.items():
            obj.setPropertyValue(k, v)
            value = obj.getPropertyValue(k)
            test.assertEqual(value, v)

    def setProperties(self, obj, dict):
        for k,v in dict.items():
            obj.setPropertyValue(k, v)

    def postTest(self):
        assert(self.xContext)
    def tearDown(self):
        if hasattr(self, 'xDoc'):
            if self.xDoc:
                self.xDoc.close(True)
                # HACK in case self.xDoc holds a UNO proxy to an SwXTextDocument (whose dtor calls
                # Application::GetSolarMutex via sw::UnoImplPtrDeleter), which would potentially only be
                # garbage-collected after VCL has already been deinitialized:
                self.xDoc = None

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
            print("retryInvoke: caught exception")
    raise Exception("FAILED retryInvoke")

def runConnectionTests(connection, invoker, tests):
    try:
        connection.setUp()
        for test in tests:
            invoker(connection, test)
    finally:
        connection.tearDown()

def makeCopyFromTDOC(file):
    src = os.getenv("TDOC")
    assert(src is not None)
    src = os.path.join(src, file)
    dst = os.getenv("TestUserDir")
    assert(dst is not None)
    uri = urllib.parse.urlparse(dst)
    assert(uri.scheme.casefold() == "file")
    assert(uri.netloc == "" or uri.netloc.casefold() == "localhost")
    assert(uri.params == "")
    assert(uri.query == "")
    assert(uri.fragment == "")
    dst = urllib.request.url2pathname(uri.path)
    dst = os.path.join(dst, "tmp", file)
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    try:
        os.remove(dst)
    except FileNotFoundError:
        pass
    shutil.copyfile(src, dst)
    return dst

### tests ###

if __name__ == "__main__":
    parser = argparse.ArgumentParser("Help utilities for testing LibreOffice")
    group = parser.add_mutually_exclusive_group()
    group.add_argument("-v", "--verbose", help="increase output verbosity", action="store_true")
    #parser.add_argument("p", type=str, help="program name")
    args = parser.parse_args()
    if args.verbose:
        verbose = True
    con = PersistentConnection({"verbose" : args.verbose})
    print("starting soffice ... ", end="")
    con.setUp()
    print("done")
    con.get
    print ("shutting down ... ", end="")
    con.tearDown()
    print("done")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
