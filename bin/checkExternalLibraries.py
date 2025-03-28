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
libxml2_branch = ""

def get_current_version(libName):
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
        if "FONT_" in variableName:
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
        elif libraryName.startswith("libxml2"):
            global libxml2_branch
            libxml2_branch = ''.join(re.findall("\d{1,2}\.\d{1,2}", libraryName)[0])
            print("Libxml2 is on branch: " + str(libxml2_branch))
        libraryList.append(libraryName.lower())
    return libraryList

def get_latest_version(libName):

    if libName.startswith("boost"):
        libName = "boost"
    elif libName.startswith("java-websocket"):
        libName = "java-websocket"
    elif libName.startswith("phc-winner-argon2"):
        libName = "argon2"
    elif libName.startswith("libe-book"):
        libName = "libe-book"
    elif libName.startswith("zxcvbn-c"):
        libName = "zxcvbn-c"
    elif libName.startswith("libjpeg-turbo"):
        libName = "libjpeg-turbo"
    elif libName.startswith("poppler-data"):
        libName = "poppler-data"
    elif libName.startswith("libgpg-error"):
        libName = "libgpg-error"
    elif libName.startswith("mariadb-connector-c"):
        libName = "mariadb-connector-c"
    elif libName.startswith("postgresql"):
        libName = "postgresql%20" + str(postgres_branch) + ".x"
    elif libName.startswith("tiff"):
        libName = "libtiff"
    elif libName.startswith("zxing"):
        libName = "zxing-cpp"
    elif libName.startswith("liborcus"):
        libName = "orcus"
    elif libName.startswith("ltm"):
        libName = "libtommath"
    elif "clucene-core" in libName:
        libName = "clucene-core"
    elif "lp_solve" in libName:
        libName = "lp_solve"
    elif "hsqldb" in libName:
        libName = "hsqldb"
    elif re.match("[0-9a-f]{5,40}", libName.split("-")[0]): # SHA1
        libName = libName.split("-")[1]
    else:
        libName = libName.split("-")[0]

    item = 0
    itemId = 0
    latest_version = 0
    if libName == "openssl":
        item = 2
        itemId = 2566
    elif libName == "curl":
        item = 1
        itemId = 381
    elif libName == "libpng":
        item = 4
        itemId = 1705
    elif libName == "freetype":
        item = 2
        itemId = 854
    elif libName == "lcms2":
        item = 1
        itemId = 9815
    elif libName == "cairo":
        item = 1
        itemId = 247
    elif libName == "bzip2":
        item = 1
        itemId = 237
    elif libName == "zlib":
        item = 1
        itemId = 5303
    elif libName == "libwebp":
        item = 1
        itemId = 1761
    elif libName == "libffi":
        item = 1
        itemId = 1611

    urlApi = "https://release-monitoring.org/api/v2/projects/?name=" + libName
    res = requests.get(urlApi)
    json = res.json()
    if not json['items']:
        return Version("0.0.0"), ""

    if item != 0:
        assert json['items'][item]['id'] == itemId, str(json['items'][item]['id']) + " != " + str(itemId)

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

    elif libName == "libxml2":
        for idx, ver in enumerate(json['items'][item]['stable_versions']):
            if ver.startswith(libxml2_branch):
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
