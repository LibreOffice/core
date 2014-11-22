#!/usr/bin/python3
# Copyright 2014 Jonathan Riddell <jr@jriddell.org>
# May be copied under the MPLv2
# Map breeze freedesktop named icons to libreoffice icon names

import os
import subprocess
import re

if os.path.isdir("plasma-next-icons"):
    os.chdir("plasma-next-icons")
    subprocess.check_call(["git", "pull", "-r"])
    os.chdir("..")
else:
    subprocess.check_call(["git", "clone", "https://github.com/NitruxSA/plasma-next-icons.git"])

mapping = {}

mappingFile = open('mapping', 'r')
for line in mappingFile:
    if line.startswith("#"):
        continue
    if len(line.rsplit(" ")) != 2:
        continue
    freeDesktopFile = "plasma-next-icons/" + line.rsplit(" ")[0]
    libreOfficeFile = line.rsplit(" ")[1].rstrip()
    size = "16"
    if "lc_" in libreOfficeFile:
        size = "24"
    print(freeDesktopFile + " → " + libreOfficeFile + " @ " + size+"x"+size)
    subprocess.check_call(["ksvgtopng5", size, size, freeDesktopFile, libreOfficeFile], env=os.environ)
    if "lc_" in libreOfficeFile:
        smallLibreOfficeFile = re.sub("lc_", "sc_", libreOfficeFile)
        size = "16"
        print(freeDesktopFile + " → " + smallLibreOfficeFile + " @ " + size+"x"+size)
        subprocess.check_call(["ksvgtopng5", size, size, freeDesktopFile, smallLibreOfficeFile], env=os.environ)
