#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import re
import sys
import os
import xml.etree.ElementTree as ET
import json
from datetime import datetime, timezone

sbom_data = {}
productname = os.environ.get("PRODUCTNAME_WITHOUT_SPACES").lower()
version = (
    os.environ.get("LIBO_VERSION_MAJOR") + "." +
    os.environ.get("LIBO_VERSION_MINOR") + "." +
    os.environ.get("LIBO_VERSION_MICRO") + "." +
    os.environ.get("LIBO_VERSION_PATCH")
)


def extract_version_from_filename(filename):
    filename = re.sub(r'\.(tar\.gz|tar\.xz|tar\.bz2|zip)$', '', filename)
    if '_' in filename:
        # e.g. boost_1_87_0 -> boost-1.8.7, twaindsm_2.4.1.orig -> twaindsm-2.4.1.orig
        name, version = filename.split('_', 1)
        version = version.replace('_', '.')
        filename =  f"{name}-{version}"
    match = re.search(r'\d+(?:\.\d+)+(?:-[\w\.]+)*', filename)
    # d+(?:\.\d+)+ – matches a version core like 0.910.12
    # (?:-[\w\.]+)* – matches optional trailing parts like -rc2, -2019.10.17, etc.
    # It stops at .zip or whatever follows, since it doesn’t include a literal dot unless it’s part of [\w\.]
    if match:
        return match.group()
    return None


def extract_version_for_dictionary(dict):
    filename = os.environ.get('SRC_ROOT') + '/dictionaries/' + dict + '/description.xml'
    tree = ET.parse(filename)
    root = tree.getroot()
    ns = {"d": "http://openoffice.org/extensions/description/2006"}
    version_element = root.find('.//d:version', ns)
    if version_element is not None and 'value' in version_element.attrib:
        return version_element.attrib['value']
    return None


def extract_spdx_info(line):
    """
    Extract relevant SPDX information from a line.
    The line format is assumed to be like:
    <!-- Name: Box2D, Source: BOX2D_TARBALL, Package: core, SPDX-License-Identifier: MIT -->
    """
    pattern = r"<!-- Name:\s*(?P<name>[\w\s-]+),\s*Source:\s*(?P<source>[\w/]+),\s*Package:\s*(?P<package>[\w-]+),\s*SPDX-License-Identifier:\s*(?P<license>[\w\s.+-]+) -->"
    match = re.search(pattern, line)

    if match:
        name = match.group("name").strip()
        source = match.group("source").strip()
        package = match.group("package").strip()
        if not source.isupper():
            version = None
        elif package.startswith("dict"):
            version = extract_version_for_dictionary(source)
        else:
            version = extract_version_from_filename(os.environ.get(source))
        license = match.group("license").strip()

        spdx_info = {
            "package": package,
            "SPDXID": f"SPDXRef-{name}",
            "name": name,
            "versionInfo": version,
            "filesAnalyzed": False,
            "downloadLocation": "NONE",
            "licenseConcluded": license
        }
        return spdx_info
    return None


def process_file(file_path):
    """
    Process the file and append SPDX information for matching lines.
    """
    with open(file_path, 'r', encoding='utf-8') as file:
        for line in file:
            spdx_info = extract_spdx_info(line)
            if spdx_info:
                package = spdx_info["package"]
                spdx_info.pop("package", None)
                if not sbom_data.get(package):
                    sbom_skeleton(package)
                sbom_data[package]["packages"].append(spdx_info)
                relationships_data = {
                    "spdxElementId": f"SPDXRef-{productname}-{package}",
                    "relationshipType": "CONTAINS",
                    "relatedSpdxElement": spdx_info["SPDXID"]
                }
                sbom_data[package]["relationships"].append(relationships_data)


def sbom_skeleton(package):
    timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    sbom_data[package] = {
        "SPDXID": "SPDXRef-DOCUMENT",
        "spdxVersion": "SPDX-2.3",
        "dataLicense": "CC0-1.0",
        "name": f"{productname}-{package}",
        "documentNamespace": f"http://spdx.org/spdxdocs/{productname}-{package}-{version}",
        "creationInfo": {
            "creators": ["Tool: Custom Script"],
            "created": timestamp
        },
        "packages": [
            {
                "SPDXID": f"SPDXRef-{productname}-{package}",
                "name": f"{productname}-{package}",
                "versionInfo": version,
                "filesAnalyzed": False,
                "downloadLocation": "NONE",
                "licenseConcluded": "MPL-2.0"
            }
        ],
        "relationships": [
            {
                "spdxElementId": "SPDXRef-DOCUMENT",
                "relationshipType": "DESCRIBES",
                "relatedSpdxElement": f"SPDXRef-{productname}-{package}"
            }
        ]
    }


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python create-sbom.py <path of LICENSE.html> <path of output SPDX JSON files>")
    else:
        license_path = sys.argv[1]
        sbom_path = sys.argv[2]
        process_file(license_path)
        for package, data in sbom_data.items():
            filename = f"{package}-sbom.spdx.json"
            filepath = os.path.join(sbom_path, filename)
            with open(filepath, "w", encoding="utf-8") as file:
                json.dump(data, file, indent=2)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
