#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from os import path, walk

if __name__ == '__main__':

    ignoredPackages = ["dbm", "curses", "test", "tkinter", "turtledemo", "sqlite3", "idlelib", "venv"]

    coreDir = path.dirname(path.dirname(path.dirname(path.abspath(__file__))))
    pythonDir = path.join(coreDir, "workdir/UnpackedTarball/python3")
    libDir = path.join(pythonDir, "Lib")
    subDirDict = {}
    for subdir, dirs, files in walk(libDir):
        filesList = []
        relPythonDir = path.relpath(subdir, pythonDir)
        relLibDir = path.relpath(subdir, libDir)

        isShipped = True
        for package in ignoredPackages:
            if relLibDir.startswith(package):
                isShipped = False
                continue

        if not isShipped:
            continue

        if "/test" in relLibDir:
            continue

        for fileName in sorted(files):
            if not fileName.endswith(".py") and not fileName.endswith(".pem"):
                continue
            filesList.append(path.join(relPythonDir, fileName))
        if filesList:
            subDirDict[relPythonDir] = filesList

    for k,v in sorted(subDirDict.items()):
        print()
        if k == "Lib/msilib":
            print("ifeq (WNT,$(OS))")
        print("$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/{},\\".format(k.lower()))
        for fileName in sorted(v):
            print("\t{} \\".format(fileName))
        print("))")
        if k == "Lib/msilib":
            print("endif")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
