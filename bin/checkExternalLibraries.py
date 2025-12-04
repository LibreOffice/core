#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Use this script to check the latest version of the external libraries

import requests
import sys
import re
from packaging.version import Version, parse
import subprocess
import argparse

python_branch = ""
openssl_branch = ""
postgres_branch = ""
mariadb_branch = ""
rhino_branch = ""

libraryIds = {
    "openssl": 2,
    "curl": 1,
    "libpng": 4,
    "freetype": 2,
    "lcms2": 1,
    "cairo": 1,
    "bzip2": 1,
    "zlib": 1,
    "libwebp": 1,
    "libffi": 1,
    "xz": 1,
    "zstd": 1
}

libraryNames = {
    "boost": "boost",
    "java-websocket": "java-websocket",
    "phc-winner-argon2": "argon2",
    "libe-book": "libe-book",
    "zxcvbn-c": "zxcvbn-c",
    "libjpeg-turbo": "libjpeg-turbo",
    "poppler-data": "poppler-data",
    "libgpg-error": "libgpg-error",
    "mariadb-connector-c": "mariadb-connector-c",
    "tiff": "libtiff",
    "zxing": "zxing-cpp",
    "liborcus": "orcus",
    "ltm": "libtommath",
    "clucene-core": "clucene-core",
    "lp_solve": "lp_solve",
    "hsqldb": "hsqldb",
    "y-crdt": "y-crdt",
    "sqlite": "sqlite",
    "reemkufi": "aliftype-reem-kufi-fonts",
}

def get_current_version(libName):
    if "sqlite" in libName:
        # 3XXYYZZ -> 3.X.Y.Z
        s = re.search("(\d{7})", libName )
        if s:
            num = str(s.group(1))
            return parse("{}.{}.{}.{}".format(num[0], num[1:3], num[3:5], num[6:7]))
    else:
        libraryName = libName.replace("_", ".")
        libraryName = re.sub("[0-9a-f]{5,40}", "", libraryName) #SHA1
        s = re.search("\d[\d\.]+\d", libraryName )
        if s:
            return parse(s.group())

    return Version("0.0.0")

def get_library_list(fileName):
    inputProces = "include " + fileName + "\nall:\n\t$(foreach v,$(sort $(filter %TARBALL,$(.VARIABLES))),$(info export $(v)=$($(v))))"
    process = subprocess.run(["make", "-q", "-f", "-"], input=bytes(inputProces, 'UTF-8'), stdout=subprocess.PIPE)

    libraryList = []
    for lib in process.stdout.splitlines():
        decodedName = lib.decode("utf-8")
        variableName = decodedName.split("=")[0]

        if "JFREEREPORT_" in variableName:
            continue

        # FIXME
        if "FONT_" in variableName and "REEM" not in variableName:
            continue

        libraryName = decodedName.split("=")[1]
        if libraryName.startswith("Python"):
            global python_branch
            python_branch = ''.join(re.findall("\d{1,2}\.\d{1,2}", libraryName)[0])
            print("Python is on branch: " + str(python_branch))
        elif libraryName.startswith("postgres"):
            global postgres_branch
            postgres_branch = ''.join(re.findall("\d{1,2}", libraryName)[0])
            print("Postgres is on branch: " + str(postgres_branch))
        elif libraryName.startswith("openssl"):
            global openssl_branch
            openssl_branch = ''.join(re.findall("\d{1,2}\.\d{1,2}", libraryName)[0])
            print("Openssl is on branch: " + str(openssl_branch))
        elif libraryName.startswith("mariadb"):
            global mariadb_branch
            mariadb_branch = ''.join(re.findall("\d{1,2}\.\d{1,2}", libraryName)[0])
            print("MariaDB is on branch: " + str(mariadb_branch))
        elif libraryName.startswith("rhino"):
            global rhino_branch
            rhino_branch = ''.join(re.findall("\d{1,2}\.\d{1,2}", libraryName)[0])
            print("Rhino is on branch: " + str(rhino_branch))
        libraryList.append(libraryName.lower())
    return libraryList

def get_latest_version(libName):
    bFound = False
    for k,v in libraryNames.items():
        if k in libName:
            libName = v
            bFound = True
            break

    if not bFound:
        if libName.startswith("postgresql"):
            libName = "postgresql%20" + str(postgres_branch) + ".x"
        elif re.match("[0-9a-f]{5,40}", libName.split("-")[0]): # SHA1
            libName = libName.split("-")[1]
        else:
            libName = libName.split("-")[0]

    item = 0
    latest_version = 0

    if libName in libraryIds:
        item = libraryIds[libName]

    urlApi = "https://release-monitoring.org/api/v2/projects/?name=" + libName
    res = requests.get(urlApi)
    json = res.json()
    if not json['items']:
        return Version("0.0.0"), ""

    if libName == "openssl":
        for idx, ver in enumerate(json['items'][item]['stable_versions']):
            if ver.startswith(openssl_branch):
                latest_version = idx
                break

    elif libName == "python":
        for idx, ver in enumerate(json['items'][item]['stable_versions']):
            if ver.startswith(python_branch):
                latest_version = idx
                break

    elif libName == "mariadb-connector-c":
        for idx, ver in enumerate(json['items'][item]['stable_versions']):
            if ver.startswith(mariadb_branch):
                latest_version = idx
                break

    elif libName == "rhino":
        for idx, ver in enumerate(json['items'][item]['stable_versions']):
            if ver.startswith(rhino_branch):
                latest_version = idx
                break

    return Version(json['items'][item]['stable_versions'][latest_version].replace("_", ".").replace("-", ".")), json['items'][item]['homepage']

if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('--file', action="store", dest="file", required=True)

    args = parser.parse_args()

    libraryList = get_library_list(args.file)
    if not libraryList:
        sys.exit(1)

    for lib in libraryList:
        if lib.startswith("language-subtag-registry"):
            print("CHECK https://www.iana.org/assignments/language-subtag-registry/language-subtag-registry")
            continue
        latestVersion, website = get_latest_version(lib)
        currentVersion = get_current_version(lib)
        if latestVersion == Version("0.0.0"):
            print("FAIL: " + lib + " not found in https://release-monitoring.org")
        elif currentVersion == Version("0.0.0"):
            print("FAIL: " + lib + " version not detected")
        elif currentVersion != latestVersion:
            print(lib + " is not updated: " + website)
            print(" ----> The current version is " + str(currentVersion))
            print(" ----> The latest version is " + str(latestVersion))
