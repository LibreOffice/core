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
import uuid

sbom_data = {}
timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
productname = os.environ.get("PRODUCTNAME_WITHOUT_SPACES").lower()
root_version = (
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


SCP2TYPES = {"Directory", "File", "Profile", "Module", "WindowsCustomAction", "MergeModule"}

def parse_install_script(filename):
    """Parse the install script that is produced in scp2."""
    with open(filename) as f:
        lines = f.read().splitlines()

    result = {t: {} for t in SCP2TYPES}
    gids = set()
    i = 0
    n = len(lines)

    while i < n:
        line = lines[i]
        m = re.match(r'^\s*(\S+)\s+(\S+)\s*$', line)
        if not m:
            i += 1
            continue

        item_type = m.group(1)
        gid = m.group(2)
        if gid in gids:
            raise Exception(f"line {i} duplicate gid {gid}")
        gids.add(gid)

        if item_type not in SCP2TYPES:
            # Skip to End
            i += 1
            while i < n and not re.match(r'^\s*End\s*$', lines[i]):
                i += 1
            if i == n:
                raise Exception("expected End before EOF")
            i += 1
            continue

        item = {}
        ismultilang = False
        i += 1

        while i < n and not re.match(r'^\s*End\s*$', lines[i]):
            line = lines[i]

            # Single-line key = value;
            m2 = re.match(r'^\s*(.+?)=\s*(.+?);\s*$', line)
            if m2:
                key = m2.group(1).rstrip()
                value = m2.group(2).rstrip()
                # Remove surrounding quotes
                qm = re.match(r'^"(.*)"$', value)
                if qm:
                    value = qm.group(1)
                item[key] = value
                if re.match(r'^\S+\s+\(\S+\)$', key):
                    ismultilang = True
                i += 1
                continue

            # Multi-line value (Module only): key = (... spread across lines ...);
            if item_type == 'Module':
                m3 = re.match(r'^\s*(.+?)\s*=\s*\((.+?)\s*$', line)
                if m3 and not line.rstrip().endswith(');'):
                    key = m3.group(1).strip()
                    value = '(' + m3.group(2)
                    i += 1
                    while i < n and not lines[i].rstrip().endswith(');'):
                        value += lines[i].strip()
                        i += 1
                    if i == n:
                        raise Exception("expected ) before EOF")
                    value += lines[i].strip()
                    i += 1
                    if i == n:
                        raise Exception("expected End before EOF")
                    value = re.sub(r';\s*$', '', value)
                    item[key] = value
                    if re.match(r'^\S+\s+\(\S+\)$', key):
                        ismultilang = True
                    continue
            else:
                raise Exception(f"unexpected line {i}: {line}")

            i += 1

        if i == n:
            raise Exception("expected End before EOF")

        i += 1  # skip End line
        item['ismultilingual'] = 1 if ismultilang else 0
        result[item_type][gid] = item

    return result

def parse_packinfo(filename):
    """Parse a packinfo file from setup_native."""
    with open(filename) as f:
        lines = f.read().splitlines()
    result = []
    i = 0
    n = len(lines)
    while i < n:
        line = lines[i].strip()
        if not line or line.startswith("#"):
            i += 1
            continue
        if line == "Start":
            i += 1
            item = {}
            while i < n and not re.match(r'^\s*End\s*$', lines[i]):
                line = lines[i].strip()
                m = re.match(r'^(\w+)\s*=\s*"?(.*?)"?\s*$', line)
                if not m:
                    raise Exception(f"unexpected line {i}: {line}")
                item[m.group(1)] = m.group(2)
                i += 1
            if i == n:
                raise Exception("expected End before EOF")
            result.append(item)
            i += 1
            continue
        raise Exception(f"unexpected line {i}: {line}")
    return result

package_cache = {}

def package_id(package):
    if package not in package_cache:
        package_cache[package] = f"urn:uuid:{uuid.uuid4()}"
    return package_cache[package]


spdx_id_cache = {}

def make_spdx_id(package, fragment):
    """Create a URN UUID for an SPDX element"""
    key = (package, fragment)
    if key not in spdx_id_cache:
        spdx_id_cache[key] = f"urn:uuid:{uuid.uuid4()}"
    return spdx_id_cache[key]


def next_rel_id(package):
    """Generate a unique relationship URN UUID."""
    return f"urn:uuid:{uuid.uuid4()}"


license_cache = {}

def add_license_relationship(package, from_id, license_expr):
    """Add a license expression element and hasDeclaredLicense relationship."""

    graph = sbom_data[package]["@graph"]

    key = (package, license_expr)
    if key not in license_cache:
        license_id = make_spdx_id(package, f"License-{license_expr}")
        graph.append({
            "type": "simplelicensing_LicenseExpression",
            "spdxId": license_id,
            "creationInfo": "_:creationinfo",
            "simplelicensing_licenseExpression": license_expr
        })
        license_cache[key] = license_id
    else:
        license_id = license_cache[key]

    graph.append({
        "type": "Relationship",
        "spdxId": next_rel_id(package),
        "creationInfo": "_:creationinfo",
        "from": from_id,
        "relationshipType": "hasDeclaredLicense",
        "to": [license_id]
    })


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
            "fragment": f"SPDXRef-{name}",
            "name": name,
            "version": version,
            "license": license
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
                root_spdx_id = make_spdx_id(package, f"SPDXRef-{productname}-{package}")
                if not sbom_data.get(package):
                    sbom_skeleton(package, root_spdx_id)

                graph = sbom_data[package]["@graph"]
                pkg_spdx_id = make_spdx_id(package, spdx_info["fragment"])

                # Add the package element
                pkg_element = {
                    "type": "software_Package",
                    "spdxId": pkg_spdx_id,
                    "originatedBy": ["https://collaboraoffice.com"],
                    "creationInfo": "_:creationinfo",
                    "name": spdx_info["name"],
                }
                if spdx_info["version"]:
                    pkg_element["software_packageVersion"] = spdx_info["version"]
                graph.append(pkg_element)

                # Add CONTAINS relationship
                graph.append({
                    "type": "Relationship",
                    "spdxId": next_rel_id(package),
                    "creationInfo": "_:creationinfo",
                    "from": root_spdx_id,
                    "relationshipType": "contains",
                    "to": [pkg_spdx_id]
                })

                add_license_relationship(
                    package, pkg_spdx_id, spdx_info["license"])


def sbom_skeleton(package, root_spdx_id):
    package_spdx_id = package_id(package)
    tool_spdx_id = make_spdx_id(package, "SPDXRef-Tool-CustomScript")

    sbom_data[package] = {
        "@context": "https://spdx.org/rdf/3.0.1/spdx-context.jsonld",
        "@graph": [
            {
                "type": "Organization",
                "spdxId": "https://collaboraoffice.com",
                "creationInfo": "_:creationinfo",
                "externalIdentifers": [{
                    "type": "ExternalIdentifier",
                    "externalIdentifierType": "email",
                    "identifier": "hello@collaboraoffice.com"
                }]
            },
            {
                "type": "CreationInfo",
                "@id": "_:creationinfo",
                "specVersion": "3.0.1",
                "created": timestamp,
                "createdBy": ["https://collaboraoffice.com"],
                "createdUsing": [tool_spdx_id]
            },
            {
                "type": "Tool",
                "spdxId": tool_spdx_id,
                "creationInfo": "_:creationinfo",
                "name": "Custom Script"
            },
            {
                "type": "SpdxDocument",
                "spdxId": package_spdx_id,
                "creationInfo": "_:creationinfo",
                "name": f"{productname}-{package}",
                "rootElement": [root_spdx_id],
                "profileConformance": ["core", "software", "simpleLicensing"]
            },
            {
                "type": "software_Package",
                "spdxId": root_spdx_id,
                "creationInfo": "_:creationinfo",
                "name": f"{productname}-{package}",
                "software_packageVersion": root_version
            },
            {
                "type": "Relationship",
                "spdxId": next_rel_id(package),
                "creationInfo": "_:creationinfo",
                "from": package_spdx_id,
                "relationshipType": "describes",
                "to": [root_spdx_id]
            }
        ]
    }

    # Add license for root package
    add_license_relationship(package, root_spdx_id, "MPL-2.0")


if __name__ == "__main__":
    if len(sys.argv) < 8:
        print("Usage: python create-sbom.py <path of output SPDX JSON files> <path of LICENSE.html> <4 packinfo> <path of install script>")
    else:
        sbom_path = sys.argv[1]
        license_path = sys.argv[2]
        packinfos = []
        packinfos += parse_packinfo(sys.argv[3])
        packinfos += parse_packinfo(sys.argv[4])
        packinfos += parse_packinfo(sys.argv[5])
        packinfos += parse_packinfo(sys.argv[6])
        install_script = parse_install_script(sys.argv[7])
        process_file(license_path)
        for package, data in sbom_data.items():
            filename = f"{package}-sbom.spdx.json"
            filepath = os.path.join(sbom_path, filename)
            with open(filepath, "w", encoding="utf-8") as file:
                json.dump(data, file, indent=2)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
