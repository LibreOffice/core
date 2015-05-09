#!/usr/bin/python3
# Copyright 2014 Jonathan Riddell <jr@jriddell.org>
# May be copied under the MPLv2
# Map breeze freedesktop named icons to libreoffice icon names

import os
import subprocess

if os.path.isdir("plasma-next-icons"):
    os.chdir("plasma-next-icons")
    subprocess.check_call(["git", "pull", "-r"])
    os.chdir("..")
else:
    subprocess.check_call(["git", "clone", "https://github.com/NitruxSA/plasma-next-icons.git"])

mapping = {}

mappingFile = open('mapping', 'r')
for line in mappingFile:
    freeDesktopFile = "plasma-next-icons/" + line.rsplit(" ")[0]
    libreOfficeFile = line.rsplit(" ")[1].rstrip()
    size = line.rsplit(" ")[2].rstrip()
    print(freeDesktopFile + " → " + libreOfficeFile + " @ " + size+"x"+size)
    subprocess.check_call(["ksvgtopng", size, size, freeDesktopFile, libreOfficeFile])
