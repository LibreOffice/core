# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
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

import getopt
import sys
import uno

from unohelper import Base,systemPathToFileUrl
from os import getcwd
from com.sun.star.beans import PropertyValue
from com.sun.star.uno import Exception as UnoException
from com.sun.star.io import IOException, XOutputStream

class OutputStream(Base, XOutputStream):
    def __init__(self):
        self.closed = 0

    def closeOutput(self):
        self.closed = 1

    def writeBytes(self, seq):
        sys.stdout.write(seq.value)

    def flush(self):
        pass

def main():
    retVal = 0
    doc = None

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hc:", ["help", "connection-string=", "html"])
        url = "uno:socket,host=localhost,port=2002;urp;StarOffice.ComponentContext"
        filterName = "Text (Encoded)"
        for o, a in opts:
            if o in ("-h", "--help"):
                usage()
                sys.exit()
            if o in ("-c", "--connection-string"):
                url = "uno:" + a + ";urp;StarOffice.ComponentContext"
            if o == "--html":
                filterName = "HTML (StarWriter)"

        print(filterName)
        if not len(args):
            usage()
            sys.exit()

        ctxLocal = uno.getComponentContext()
        smgrLocal = ctxLocal.ServiceManager

        resolver = smgrLocal.createInstanceWithContext(
                 "com.sun.star.bridge.UnoUrlResolver", ctxLocal)
        ctx = resolver.resolve(url)
        smgr = ctx.ServiceManager

        desktop = smgr.createInstanceWithContext("com.sun.star.frame.Desktop", ctx)

        cwd = systemPathToFileUrl(getcwd())
        outProps = (
            PropertyValue("FilterName" , 0, filterName, 0),
            PropertyValue("OutputStream", 0, OutputStream(), 0))
        inProps = PropertyValue("Hidden", 0 , True, 0),
        for path in args:
            try:
                fileUrl = uno.absolutize(cwd, systemPathToFileUrl(path))
                doc = desktop.loadComponentFromURL(fileUrl , "_blank", 0, inProps)

                if not doc:
                    raise UnoException("Could not open stream for unknown reason", None)

                doc.storeToURL("private:stream", outProps)
            except IOException as e:
                sys.stderr.write("Error during conversion: " + e.Message + "\n")
                retVal = 1
            except UnoException as e:
                sys.stderr.write("Error (" + repr(e.__class__) + ") during conversion: " + e.Message + "\n")
                retVal = 1
            if doc:
                doc.dispose()

    except UnoException as e:
        sys.stderr.write("Error (" + repr(e.__class__) + "): " + e.Message + "\n")
        retVal = 1
    except getopt.GetoptError as e:
        sys.stderr.write(str(e) + "\n")
        usage()
        retVal = 1

    sys.exit(retVal)

def usage():
    sys.stderr.write("usage: ooextract.py --help |\n"+
                  "       [-c <connection-string> | --connection-string=<connection-string>\n"+
                  "       file1 file2 ...\n"+
                  "\n" +
                  "Extracts plain text from documents and prints it to stdout.\n" +
                  "Requires an OpenOffice.org instance to be running. The script and the\n"+
                  "running OpenOffice.org instance must be able to access the file with\n"+
                  "by the same system path.\n"
                  "\n"+
                  "-c <connection-string> | --connection-string=<connection-string>\n" +
                  "        The connection-string part of a UNO URL to where the\n" +
                  "        the script should connect to in order to do the conversion.\n" +
                  "        The strings defaults to socket,host=localhost,port=2002\n"
                  "--html \n"
                  "        Instead of the text filter, the writer html filter is used\n"
                  )

main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
