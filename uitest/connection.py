# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os
import subprocess
import sys
import time
import uuid
import datetime

try:
    import pyuno
    import uno
    import unohelper
except ImportError:
    print("pyuno not found: try to set PYTHONPATH and URE_BOOTSTRAP variables")
    print("PYTHONPATH=/installation/opt/program")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

class OfficeConnection:
    def __init__(self, args):
        self.args = args
        self.soffice = None
        self.socket = None
        self.xContext = None
        self.pro = None

    def setUp(self):
        (method, sep, rest) = self.args["--soffice"].partition(":")
        if sep != ":":
            raise Exception("soffice parameter does not specify method")
        if method == "path":
                socket = "pipe,name=pytest" + str(uuid.uuid1())
                try:
                    userdir = self.args["--userdir"]
                except KeyError:
                    raise Exception("'path' method requires --userdir")
                if not(userdir.startswith("file://")):
                    raise Exception("--userdir must be file URL")
                self.soffice = self.bootstrap(rest, userdir, socket)
        elif method == "connect":
                socket = rest
        else:
            raise Exception("unsupported connection method: " + method)
        self.xContext = self.connect(socket)

    def bootstrap(self, soffice, userdir, socket):
        argv = [ soffice, "--accept=" + socket + ";urp",
                "-env:UserInstallation=" + userdir,
                "--quickstart=no", "--nofirststartwizard",
                "--norestore", "--nologo" ]
        if "--valgrind" in self.args:
            argv.append("--valgrind")
        self.pro = subprocess.Popen(argv)
        return self.pro

    def connect(self, socket):
        xLocalContext = uno.getComponentContext()
        xUnoResolver = xLocalContext.ServiceManager.createInstanceWithContext(
                "com.sun.star.bridge.UnoUrlResolver", xLocalContext)
        url = "uno:" + socket + ";urp;StarOffice.ComponentContext"
        print("OfficeConnection: connecting to: " + url)
        while True:
            try:
                xContext = xUnoResolver.resolve(url)
                return xContext
#            except com.sun.star.connection.NoConnectException
            except pyuno.getClass("com.sun.star.connection.NoConnectException"):
                print("NoConnectException: sleeping...")
                time.sleep(1)

    def tearDown(self):
        if self.soffice:
            if self.xContext:
                try:
                    print("tearDown: calling terminate()...")
                    xMgr = self.xContext.ServiceManager
                    xDesktop = xMgr.createInstanceWithContext(
                            "com.sun.star.frame.Desktop", self.xContext)
                    xDesktop.terminate()
                    print("...done")
#                except com.sun.star.lang.DisposedException:
                except pyuno.getClass("com.sun.star.beans.UnknownPropertyException"):
                    print("caught UnknownPropertyException while TearDown")
                    pass # ignore, also means disposed
                except pyuno.getClass("com.sun.star.lang.DisposedException"):
                    print("caught DisposedException while TearDown")
                    pass # ignore
            else:
                self.soffice.terminate()
            ret = self.soffice.wait()
            self.xContext = None
            self.socket = None
            self.soffice = None
            if ret != 0:
                raise Exception("Exit status indicates failure: " + str(ret))

    def kill(self):
        command = "kill " + str(self.pro.pid)
        print(command)
        os.system(command)

    @classmethod
    def getHelpText(cls):
        message = """
 --soffice=method:location
                   specify soffice instance to connect to
                   supported methods: 'path', 'connect'
 --userdir=URL     specify user installation directory for 'path' method
 --valgrind        pass --valgrind to soffice for 'path' method

 'location' is a pathname, not a URL. 'userdir' is a URL.
 """
        return message


class PersistentConnection:
    def __init__(self, args):
        self.args = args
        self.connection = None
    def getContext(self):
        return self.connection.xContext
    def setUp(self):
        assert(not self.connection)
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
    def kill(self):
        if self.connection:
            self.connection.kill()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
