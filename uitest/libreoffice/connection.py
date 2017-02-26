# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
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
        argv = [soffice, "--accept=" + socket + ";urp",
                "-env:UserInstallation=" + userdir,
                "--quickstart=no", "--nofirststartwizard",
                "--norestore", "--nologo"]
        if "--valgrind" in self.args:
            argv.append("--valgrind")

        if "--gdb" in self.args:
            argv.insert(0, "gdb")
            argv.insert(1, "-ex")
            argv.insert(2, "run")
            argv.insert(3, "--args")
            argv[4] = argv[4].replace("soffice", "soffice.bin")

        self.pro = subprocess.Popen(argv)
        return self.pro

    def connect(self, socket):
        xLocalContext = uno.getComponentContext()
        xUnoResolver = xLocalContext.ServiceManager.createInstanceWithContext(
                "com.sun.star.bridge.UnoUrlResolver", xLocalContext)
        url = "uno:" + socket + ";urp;StarOffice.ComponentContext"
        print("OfficeConnection: connecting to: " + url)
        while True:
            if self.soffice.poll() is not None:
                raise Exception("soffice has stopped.")

            try:
                xContext = xUnoResolver.resolve(url)
                return xContext
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
                except pyuno.getClass("com.sun.star.beans.UnknownPropertyException"):
                    print("caught UnknownPropertyException while TearDown")
                    pass # ignore, also means disposed
                except pyuno.getClass("com.sun.star.lang.DisposedException"):
                    print("caught DisposedException while TearDown")
                    pass # ignore
            else:
                self.soffice.terminate()

            DEFAULT_SLEEP = 0.1
            time_ = 0
            while time_ < 5:
                time_ += DEFAULT_SLEEP
                ret_attr = self.soffice.poll()
                if ret_attr is not None:
                    break
                time.sleep(DEFAULT_SLEEP)

            ret = 0
            if ret_attr is None:
                ret = 1
                self.soffice.terminate()

            # ret = self.soffice.wait()
            self.xContext = None
            self.socket = None
            self.soffice = None
            if ret != 0:
                raise Exception("Exit status indicates failure: " + str(ret))

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

    def tearDown(self):
        if self.connection:
            try:
                self.connection.tearDown()
            finally:
                self.connection = None

    def kill(self):
        if self.connection and self.connection.soffice:
            self.connection.soffice.kill()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
