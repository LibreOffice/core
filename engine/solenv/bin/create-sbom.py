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
import hashlib
import zipfile

SRCDIR = os.environ.get('SRC_ROOT')
sys.path.insert(0, SRCDIR + "/external/pip-wheels/altgraph-0.17.5-py2.py3-none-any.whl")
sys.path.insert(0, SRCDIR + "/external/pip-wheels/dnfile-0.18.0-py3-none-any.whl")
sys.path.insert(0, SRCDIR + "/external/pip-wheels/macholib-1.16.4-py2.py3-none-any.whl")
sys.path.insert(0, SRCDIR + "/external/pip-wheels/pefile-2024.8.26-py3-none-any.whl")
sys.path.insert(0, SRCDIR + "/external/pip-wheels/pyelftools-0.33-py3-none-any.whl")
import elftools.elf.elffile
import dnfile
import pefile


sbom_data = {}
sbom_externals = {}
filelistdirs = []
filelistdirs_sdk = []
timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
productname = os.environ.get("PRODUCTNAME_WITHOUT_SPACES")
# suffix is hard-coded in makefile :(
productname_sdk = os.environ.get("PRODUCTNAME_WITHOUT_SPACES") + "_SDK"
root_version = (
    os.environ.get("LIBO_VERSION_MAJOR") + "." +
    os.environ.get("LIBO_VERSION_MINOR") + "." +
    os.environ.get("LIBO_VERSION_MICRO") + "." +
    os.environ.get("LIBO_VERSION_PATCH")
)
root_license = "MPL-2.0" if os.environ.get("MPL_SUBSET") == "TRUE" else "LGPL-3.0"


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

def get_sha512(abspath):
    digest = hashlib.sha512()
    with open(abspath, "rb") as f:
        while True:
            chunk = f.read(1<<20)
            if not chunk:
                break
            digest.update(chunk)
    return digest.hexdigest()

def parse_filelist(filelist, filelistdirs):
    for listdir in filelistdirs:
        path = os.path.join(listdir, filelist)
        if os.path.exists(path):
            with open(path, "r") as f:
                result = []
                for line in f.readlines():
                    line = line.strip()
                    if len(line) != 0:
                        result += line.split(" ")
# at least 3 conditionally empty ones
#                if len(result) == 0:
#                    raise Exception(f"filelist does not contain files: {path}")
                return result
    raise Exception(f"cannot find filelist: {filelist}")

SCP2TYPES = {"Directory", "File", "Profile", "Module", "MergeModule"}

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
        item['gid'] = gid
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

def parse_ziplist(filename):
    """Parse a ziplist file from instsetoo_native."""

    with open(filename) as f:
        lines = f.read().splitlines()

    # substitute everything outside of "include"
    def subst_vars(line):
        return line.replace("{buildid}", os.environ.get("LIBO_VERSION_PATCH"))
#unused            .replace("{os}", os.environ.get("OS"))
#unused            .replace("{productversion}", )
#unused            .replace("{languages}", )

    def parse_group(lines, i):
        result = {}
        ingroup = False
        n = len(lines)
        while i < n:
            line = lines[i].strip()
            i += 1

            # skip comments
            if len(line) == 0 or line[0] == "#":
                continue
            elif len(line) == 1 and line[0] == "{":
                if ingroup:
                    raise Exception(f"unexpected start of group in line {i}: {line}")
                ingroup = True
            elif len(line) == 1 and line[0] == "}":
                if not ingroup:
                    raise Exception(f"unexpected end of group in line {i}: {line}")
                ingroup = False
                return (i, result)
            else:
                parts = line.split(None, 1)
                if len(parts) == 1:
                    if parts[0].lower() in ["settings", "variables"]:
                        (i_next, group) = parse_group(lines, i)
                        i = i_next
                        result[parts[0].lower()] = group
                    else:
                        result[parts[0]] = None
                else:
                    result[parts[0]] = subst_vars(parts[1])

    result = {}
    i = 0
    n = len(lines)
    while i < n:
        line = lines[i].strip()
        i += 1

        # skip comments
        if len(line) == 0 or line[0] == "#":
            continue

        if len(line.split(None)) > 1:
            raise Exception(f"unexpected more than 1 token in line {i}: {line}")

        (i_next, group) = parse_group(lines, i)
        i = i_next
        result[line] = group

    return result

def resolve_ziplist_inheritance(ziplist):
    """Resolve Globals in ziplist file.  (The format supports general
    inheritance but only Globals is used in practice)"""

    def resolve(product, global_):
        for key in global_:
            if key in ["settings", "variables"]:
                resolve(product[key], global_[key])
            elif key not in product:
                product[key] = global_[key]

    def parse_include(variables, filename):
        with open(filename) as f:
            INC = re.compile(r"^\s*(\S+)\s*=\s*(.*?)\s*$")
            for line in f.read().splitlines():
                match = INC.match(line)
                if match:
                    variables[match.group(1)] = match.group(2)

    globals_ = ziplist.pop("Globals")
    for product in ziplist:
        resolve(ziplist[product], globals_)
    for product in ziplist:
        variables = ziplist[product]["settings"]["variables"]
        if "ADD_INCLUDE_FILES" in variables:
            includes = variables["ADD_INCLUDE_FILES"].split(",")
            for inc in includes:
                parse_include(variables, os.path.join(SRCDIR, inc))

def init_filelistdirs(ziplist, product, filelistdirs):
    for path in ziplist[product]["settings"]["include"].split(","):
        if path.startswith("{filelistpath}"):
            filelistdirs.append(path.replace("{filelistpath}", os.environ.get("WORKDIR")))

package_cache = {}

def package_id(package):
    if package not in package_cache:
        package_cache[package] = f"urn:uuid:{uuid.uuid4()}"
    return package_cache[package]


spdx_id_cache = {}

def make_spdx_id(fragment):
    """Create a URN UUID for an SPDX element; important to use the same in all packages."""
    key = fragment
    if key not in spdx_id_cache:
        spdx_id_cache[key] = f"urn:uuid:{uuid.uuid4()}"
    return spdx_id_cache[key]


def next_rel_id():
    """Generate a unique relationship URN UUID."""
    return f"urn:uuid:{uuid.uuid4()}"


license_cache = {}

def add_license_relationship(graph, from_id, type, license_expr):
    """Add a license expression element and hasDeclaredLicense relationship."""

    assert type in ("hasDeclaredLicense", "hasConcludedLicense")
    key = license_expr
    if key not in license_cache:
        license_id = make_spdx_id(f"License-{license_expr}")
        license_cache[key] = [{
            "type": "simplelicensing_LicenseExpression",
            "spdxId": license_id,
            "creationInfo": "_:creationinfo",
            "simplelicensing_licenseExpression": license_expr
        }]
    else:
        license_id = license_cache[key][0]["spdxId"]

    graph.append({
        "type": "Relationship",
        "spdxId": next_rel_id(),
        "creationInfo": "_:creationinfo",
        "from": from_id,
        "relationshipType": type,
        "to": [license_id]
    })


fetch_url_cache = None

def get_fetch_url(variable):
    """The URL a tarball variable is downloaded from, per Makefile.fetch.

    Most tarballs are on the LibreOffice mirror, but some are not: the CJK
    Noto fonts come from the code-assets release, OpenDyslexic and POCO from
    their upstream releases, and the /extern bucket has its own path."""
    global fetch_url_cache
    if fetch_url_cache is None:
        with open(os.environ.get('SRC_ROOT') + '/Makefile.fetch') as f:
            content = f.read()
        buckets = {}
        for bucket, body in re.findall(
                r'fetch_(\w+)_TARBALLS\s*:?=((?:[^\n]*\\\n)*[^\n]*)', content):
            variables = re.findall(r'fetch_Optional,\w+,(\w+)', body)
            variables += re.findall(r'(?:^|\s)([A-Z][A-Z0-9_]+)(?=\s|\\|$)',
                                    body)
            buckets[bucket] = variables
        fetch_url_cache = {}
        for bucket, url in re.findall(
                r'foreach item,\$\(fetch_(\w+)_TARBALLS\),'
                r'\$\(call fetch_Download_item\w*,([^,]+),', content):
            for var in buckets.get(bucket, []):
                fetch_url_cache[var] = url.rstrip('/')
    if variable not in fetch_url_cache:
        raise Exception(f"no Makefile.fetch bucket contains {variable}")
    return fetch_url_cache[variable]


def extract_spdx_info(line):
    """
    Extract relevant SPDX information from a line.
    The line format is assumed to be like:
    <!-- Name: box2d, Vendor:, Source: BOX2D_TARBALL, URL: https://github.com/erincatto/box2d, Declared: MIT, Concluded: MPL-2.0 -->
    Source may be either a tarball variable, or a path.
    Name and Vendor are used to create CPE2.3 identifier.
    """
    match = re.search(r"<!-- *Name:", line)
    if not(match):
        return

    CPE23_name = r"(?:[A-Za-z0-9._-]|\\[!-/:-@\[-`{-~])*"
    pattern = rf"<!-- Name:\s*(?P<name>{CPE23_name}),\s*Vendor:\s*(?P<vendor>{CPE23_name}),\s*Source:\s*(?P<source>[\w/.]+),\s*URL:\s*(?P<url>[^\s,]*),\s*Declared:\s*(?P<declared>[\w\s.+-]+),\s*Concluded:\s*(?P<concluded>[\w\s.+-]+) -->"
    match = re.search(pattern, line)

    if not(match):
        raise Exception(f"Malformed comment in license file: {line}")
    else:
        name = match.group("name").strip()
        if len(name) == 0:
            raise Exception(f"No name in license file comment: {line}")
        vendor = match.group("vendor").strip()
        source = match.group("source").strip()
        url = match.group("url").strip()
        if source.isupper():
            tarball = os.environ.get(source)
#            sha512 = get_sha512(os.environ.get("TARFILE_LOCATION") + "/" + tarball)
            sha256 = os.environ.get(source.replace("_TARBALL", "_SHA256SUM"))
            if sha256 is None:
                raise Exception(f"No SHA256SUM for {source}")
            version = extract_version_from_filename(tarball)
            locator = get_fetch_url(source) + "/" + tarball
        elif name.startswith("dict"):
            # unclear how to hash these, it's an entire dir tree...
            sha256 = None
            version = extract_version_for_dictionary(source)
            locator = "https://cgit.collaboraoffice.com/c/dictionaries/tree/" + source
        else:
            sha256 = None
            version = None
            locator = None if len(source) == 0 else "https://cgit.collaboraoffice.com/c/core/tree/" + source
        declared = match.group("declared").strip()
        if len(declared) == 0:
            raise Exception(f"No declared in license file comment: {line}")
        concluded = match.group("concluded").strip()
        if len(concluded) == 0:
            raise Exception(f"No concluded in license file comment: {line}")

        spdx_info = {
            "fragment": f"SPDXRef-{name}",
            "name": name,
            "vendor": vendor,
            "sha256" : sha256,
            "version": version,
            "locator": locator,
            "url": url,
            "declared": declared,
            "concluded": concluded,
        }
        return spdx_info
    return None


def process_license_file(file_path):
    """
    Process the file and append SPDX information for matching lines.
    """
    with open(file_path, 'r', encoding='utf-8') as file:
        for line in file:
            spdx_info = extract_spdx_info(line)
            if spdx_info:
                name = spdx_info["name"]
                if name in sbom_externals:
                    raise Exception(f"duplicate Name in license file: {name}")

                pkg_spdx_id = make_spdx_id(spdx_info["fragment"])

                def cpe23_any(string):
                    return "*" if string is None or len(string) == 0 else string

                def cpe23(spdx_info):
                    vendor = cpe23_any(spdx_info["vendor"])
                    version = cpe23_any(spdx_info["version"])
                    return f"cpe:2.3:a:{vendor}:{name}:{version}:*:*:*:*:*:*:*"

                # Add the package element
                pkg_element = {
                    "type": "software_Package",
                    "spdxId": pkg_spdx_id,
                    "originatedBy": ["https://collaboraoffice.com"],
                    "creationInfo": "_:creationinfo",
                    "name": spdx_info["name"],
                    "externalIdentifiers": [{
                        "externalIdentifierType": "cpe23",
                        "identifier": cpe23(spdx_info),
                    }]
                }
                if len(spdx_info["url"]) != 0:
                    pkg_element["software_homePage"] = spdx_info["url"]

                    # This was based on mistaken assumption that purl can be
                    # generated from a homepage; it looks like it needs to be a
                    # git or other VCS repository, or an actual upstream
                    # release tarball
#                    pkg_element["externalIdentifers"].append({
#                            "externalIdentifierType": "packageURL",
#                            "identifier": purl(spdx_info)}
#                         })

                if spdx_info["version"]:
                    pkg_element["software_packageVersion"] = spdx_info["version"]
                source_spdx_id = make_spdx_id(f'{spdx_info["fragment"]}-source')
                source_element = {
                    "type": "software_SoftwareArtifact",
                    "spdxId": source_spdx_id,
                    "software_primaryPurpose": "source",
                    "externalRef": [{
                        "type": "ExternalRef",
                        "externalRefType": "SourceArtifact",
                        "locator": spdx_info["locator"]
                    }],
                }

                if spdx_info["sha256"] is not None:
                    source_element["verifiedUsing"] = [{
                        "type": "Hash",
                        "algorithm": "sha256",
                        "hashValue": spdx_info["sha256"]
                    }]

                graph = [pkg_element, source_element,
                    {
                        "type": "Relationship",
                        "from": source_spdx_id,
                        "relationshipType": "generates",
                        "to": [pkg_spdx_id],
                        "completeness": "complete"
                    }]

                add_license_relationship(graph, pkg_spdx_id,
                     "hasDeclaredLicense", spdx_info["declared"])
                add_license_relationship(graph, pkg_spdx_id,
                     "hasConcludedLicense", spdx_info["concluded"])

                sbom_externals[name] = (graph, spdx_info["declared"], spdx_info["concluded"])


def sbom_skeleton(package, gid, languages):
    """Create root SPDX elements of package."""

    assert isinstance(languages, set)
    root_spdx_id = make_spdx_id(f"SPDXRef-{package}")
    package_spdx_id = package_id(package)
    tool_spdx_id = make_spdx_id("SPDXRef-Tool-CustomScript")

    sbom_data[package] = (gid, languages, root_spdx_id, {
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
                "spdxId": next_rel_id(),
                "creationInfo": "_:creationinfo",
                "from": package_spdx_id,
                "relationshipType": "describes",
                "to": [root_spdx_id]
            }
        ]
    })

    # Add license for root package
    graph = sbom_data[package][3]["@graph"]
    add_license_relationship(graph, root_spdx_id, "hasConcludedLicense", root_license)


def gen_packages(packinfos, ziplist, languages, product):
    """Generate one (empty) SBOM per RPM/DEB package."""

    root_gids = set()
    variables = ziplist[product]["settings"]["variables"]
    pattern = re.compile(r"%([A-Za-z0-9_]+)")

    def replace(match):
        var = match.group(1)
        if var == "LANGUAGESTRING":
            return "%" + var # will be replaced later
        if var in ["UNIXPACKAGENAME", "UNIXPRODUCTNAME"]:
            return product.lower() # this is hardcoded in installer
        if not var in variables:
            raise Exception(f"variable used in packinfos not defined in ziplist: {var}")
        return variables[var]

    def gen_package(name, gid, languages):
        if sbom_data.get(name):
            raise Exception(f"duplicate package in packinfos: {name}")
        root_gids.add(gid)
        sbom_skeleton(name, gid, languages)

    for package in packinfos:
        gid = package["module"]
        # only Linux gets fully split packages
        if sys.platform == "win32":
            if gid not in ("gid_Module_Root", "gid_Module_Helppack_Help", "gid_Module_Root_SDK"):
                continue
        if sys.platform == "darwin":
            if gid not in ("gid_Module_Root", "gid_Module_Langpack_Basis", "gid_Module_Langpack_Brand", "gid_Module_Root_SDK"):
                continue
        name_pi = package["packagename"]
        name = pattern.sub(replace, name_pi)
        if "%LANGUAGESTRING" in name:
            for lang in languages:
                gen_package(name.replace("%LANGUAGESTRING", lang), gid + "_" + lang.replace("-", "_"), {lang})
        else:
            if sys.platform == "win32" and gid == "gid_Module_Root":
                gen_package(name, gid, languages)
            else:
                gen_package(name, gid, {"en-US"})

    return root_gids


def install_script_value_to_array(value):
    if len(value) > 1 and value[0] == "(" and value[-1] == ")":
        value = value[1:-1]
    return [] if value == "" else value.split(",")

def process_install_script(install_script, root_gids):
    """
    Find all the files in install script, their parent directories, and
    group the files by the root package to get a dictionary of lists,
    each list being a file, its parent dir, etc.
    """

    modules = install_script["Module"]

    def get_lang(module):
        if "LANGUAGEMODULE" in install_script_value_to_array(module.get("Styles", "")):
            if not "Language" in module:
                raise Exception(f"LANGUAGEMODULE without Language: {module['gid']}")
            return module["Language"]
        return None # every language

    def get_root(module):
        parent = module["ParentID"]
        if parent in root_gids:
            return parent
        return get_root(modules[parent])

    def get_files(module, lang):
        if "Assigns" in module:
            templategid = module["Assigns"]
            template = modules[templategid]
            if not "TEMPLATEMODULE" in install_script_value_to_array(template["Styles"]):
                raise Exception(f"Assigns not TEMPLATEMODULE: {templategid}")
            return get_files(template, lang)
        else:
            result = []
            if not "Files" in module:
                return result
            files = install_script_value_to_array(module["Files"])
            for filegid in files:
                file = install_script["File"][filegid]
                if lang is not None:
                    # copy it so lang can be assigned to each copy
                    file = file.copy()
                    file["modulelang"] = lang
                file_with_parents = [file]
                parent = file["Dir"]
                while parent != "PREDEFINED_PROGDIR":
                    dir_ = install_script["Directory"][parent]
                    file_with_parents.append(dir_)
                    parent = dir_["ParentID"]
                result.append(file_with_parents)
            return result

    result = {}

    # package roots declared in the packinfos whose scp2 module is only
    # included in some configurations
    known_optional_root_gids = (
        "gid_Module_Optional_Gnome",
        "gid_Module_Optional_Pyuno_LibreLogo",
        "gid_Module_Pdfimport")

    for gid in root_gids:
        if not(gid in modules) and gid in known_optional_root_gids:
            continue # skip known makefile-disabled gids
        if gid.startswith("gid_Module_Helppack_Help_") \
            and "HIDDEN_ROOT" in install_script_value_to_array(modules["gid_Module_Helppack_Helproot"]["Styles"]):
                continue # skip if help is disabled or "online"
        module = modules[gid]
        lang = get_lang(module)
        files = get_files(module, lang)
        if len(files) == 0:
            raise Exception(f"unexpected root module with no files: {gid}")
        result[gid] = files

    assigned_templates = set(module["Assigns"] for module in modules.values() if "Assigns" in module)

    for gid in modules:
        if not(gid in root_gids):
            module = modules[gid]
            if "TEMPLATEMODULE" in install_script_value_to_array(module.get("Styles", "")):
                if len(get_files(module, None)) != 0 and not(gid in assigned_templates):
                    raise Exception(f"template module with files is never assigned: {gid}")
                continue
            rootgid = get_root(module)
            lang = get_lang(module)
            files = get_files(module, lang)
            if len(files) != 0: # some are empty
                result[rootgid] += files

    return result


def read_externals_file(path):
    """Read a temp file."""
    entries = []
    with open(path, "r") as f:
        for line in f.readlines():
            if line != "\n":
                entries += line.strip().split(" ")
        if len(entries) == 0: # practically impossible to build with none
            raise Exception(f"externals file empty: {path}")
    return entries

def read_externals(path):
    """Read the gb_Externals value from a temp file."""
    entries = read_externals_file(path)
    result = {}
    for entry in entries:
        e = entry.split(";")
        result[(e[1], e[2])] = e[0]
    return result

def read_external_staticlink(path):
    """Read the gb_External_StaticLink value from a temp file."""
    # note: this can only contain content in a *top-level* make invocation
    entries = read_externals_file(path)
    result = {}
    for entry in entries:
        e = entry.split(";")
        result[e[0]] = result.get(e[0], set()).union({e[1]})
    return result

def assign_externals(files_by_package, externals):
    """Find all files that come from externals, and set "external" property on them."""

    def simple_get_dir(dirs):
        if len(dirs) == 1:
            assert dirs[0]["ParentID"] == "PREDEFINED_PROGDIR"
            return "/"
        return simple_get_dir(dirs[1:]) + dirs[0]["HostName"] + "/"

    def simple_get_path(pathlist):
        file = pathlist[0]
        return os.environ.get("INSTDIR") + simple_get_dir(pathlist[1:]) + file["Name"]

    for package in files_by_package:
        for pathlist in files_by_package[package]:
            file = pathlist[0]
            if "Name" in file: # externals are never locale specific
                if "FILELIST" in install_script_value_to_array(file["Styles"]):
                    (packagename, ext) = os.path.splitext(file["Name"])
                    if ext != ".filelist":
                        raise Exception(f"unexpected filelist {packagename}{ext}")
                    if ("pkg", packagename) in externals:
                        external = externals[("pkg", packagename)]
                        file["external"] = external
                else:
                    filename = simple_get_path(pathlist)
                    if os.path.splitext(filename)[1] == ".jar":
                        if ("jar", filename) in externals:
                            file["external"] = externals[("jar", filename)]
                    else:
                        if ("native", filename) in externals:
                            file["external"] = externals[("native", filename)]


from enum import Flag, auto
class FileFlags(Flag):
    EXECUTABLE = auto()
    ARCHIVE = auto()
    STRUCTURED = auto()

def locate_files(files_by_package, ziplist, product, filelistdirs):
    """
    Find actual paths of the files, which depends on language and variables,
    and determine flags.
    """

    archives = [
            ".odb", ".odc", ".odf", ".odg", ".odm", ".odp", ".ods", ".odt",
            ".otc", ".otf", ".otg", ".oth", ".otm", ".otp", ".ots", ".ott",
            ".stw", ".otr", ".bau", ".dat", ".sob", ".sop", ".zip" ]
    executables = [
            ".so", ".pyd", ".dll", ".dylib", ".jnilib", ".exe", ".com", ".bin",
            ".jar", ".class", ".java", ".bsh", ".js",
            ".py", ".pyi",
            ".xba", ".xdl",
            ".xsl",
            ".pl",
            ".glsl",
            ".PS" ]

    def get_flags(file, abspath, instpath):
        (basename, ext) = os.path.splitext(instpath)
        if ext == ".pyc":
            raise Exception(f"Eeeek! a .pyc file to be installed: {instpath}")
        styles = install_script_value_to_array(file["Styles"]) if "Styles" in file else []
        perms = file.get("UnixRights", "644")
        if not(perms in ("644", "755")):
            raise Exception(f"Unexpected UnixRights: {file}")
        # there are both shell scripts and ELF executables without suffix
        elif ext in executables or perms == "755" \
            or ("FILELIST" in styles and "USE_INTERNAL_RIGHTS" in styles \
                and sys.platform != "win32" and os.access(abspath, os.X_OK)) \
            or os.path.splitext(basename)[1] in (".so", ".dylib"): # .so.N
                return FileFlags.EXECUTABLE
        elif ext in archives:
            if ext != ".dat" or "autocorr" in basename:
                # for now assume no structured files other than archives
                return FileFlags.ARCHIVE | FileFlags.STRUCTURED

    def check_file(abspath):
        instpath = os.path.relpath(abspath, os.environ.get("INSTDIR")).replace("\\", "/")
        if not(os.path.exists(abspath)):
            raise Exception(f"file not found: '{abspath}'")
        return (abspath, instpath)

    def find_file(sourcepath, instpath):
        abspath = os.environ.get("INSTDIR") + "/" + sourcepath
        if not(os.path.exists(abspath)):
            raise Exception(f"file not found: {sourcepath} expected '{abspath}'")
        return (abspath, instpath)

    ZIPLIST_VAR = re.compile(r"\$\{(\w+)\}")
    variables = ziplist[product]["settings"]["variables"]

    def subst_ziplist_vars(value):
        return ZIPLIST_VAR.sub(lambda match: variables.get(match.group(1), match.group(0)), value)

    def get_dir(dirs, lang):
        if len(dirs) == 0: # only in SDK
            return ""
        if len(dirs) == 1:
            assert dirs[0]["ParentID"] == "PREDEFINED_PROGDIR"
            name = dirs[0]["HostName"]
            # gid_Dir_Sdkoo_Root gid_Dir_Brand_Root have path that doesn't exist
            # in instdir on WNT and unclear if it should be included in output
            gid = dirs[0]["gid"]
            return "" if gid in ("gid_Dir_Brand_Root", "gid_Dir_Sdkoo_Root") else name + "/"
        if dirs[0]["ismultilingual"] == 1 and f"HostName ({lang})" in dirs[0]:
            name = dirs[0][f"HostName ({lang})"]
        else:
            name = dirs[0]["HostName"]
        return get_dir(dirs[1:], lang) + subst_ziplist_vars(name) + "/"

    def get_files(pathlist, languages):
        result = set() # set because multiple languages may resolve to same path
        file = pathlist[0]
        for lang in languages:
            parent = get_dir(pathlist[1:], lang)
            styles = install_script_value_to_array(file["Styles"]) if "Styles" in file else []
            if file["ismultilingual"] == 1:
                if not(f"Name ({lang})" in file):
                    continue # skip it!
                name = subst_ziplist_vars(file[f"Name ({lang})"])
                if "MAKE_LANG_SPECIFIC" in styles:
                    (basename, ext) = os.path.splitext(name)
                    instname = basename + "_" + lang + ext
                else:
                    instname = name
            else:
                name = subst_ziplist_vars(file["Name"])
                instname = name
            if "FILELIST" in styles:
                if parent != "":
                    raise Exception(f"unexpected dir {parent} on filelist: {name}")
                # Package can have empty directory now which has x bit
                listfiles = [lf for lf in parse_filelist(name, filelistdirs) if not os.path.isdir(lf)]
                result = result.union([check_file(lf) for lf in listfiles])
            else:
                result.add(find_file(parent + name, parent + instname))
        return result

    result = {}

    for package in files_by_package:
        package_files = []
        package_langs = next(sbom_data[key][1] for key in sbom_data if sbom_data[key][0] == package)
        for pathlist in files_by_package[package]:
            file = pathlist[0]
            if "modulelang" in file:
                languages = package_langs.intersection({file["modulelang"]})
            else:
                languages = package_langs
            files = get_files(pathlist, languages)
            for (abspath, instpath) in files:
                flags = get_flags(file, abspath, instpath)
                package_files.append({"flags": flags, "instpath": instpath,
                    "abspath": abspath, "external": file.get("external")})
        result[package] = package_files

    return result

def filter_files(files_by_package):
    """Remove files that are not required to be in SBOM."""
    externals_by_package = {}
    for package in files_by_package:
        externals_by_package[package] = set([file["external"] for file in files_by_package[package]])
        files_by_package[package] = [file for file in files_by_package[package] if bool(file["flags"])]
    return externals_by_package

def add_dependencies(files_by_package, files_by_package_extra_deps):
    """Add required checksum and dependencies to files."""

    def find_dep(files_by_package, dep, instpath=None):
        found = None
        for package in files_by_package:
            for file in files_by_package[package]:
                if os.path.basename(file["instpath"]) == dep if sys.platform == "win32" \
                        else os.path.basename(file["instpath"]).lower() == dep.lower():
                    if found is None:
                        found = file
                    else:
                        # special case for Python `_ssl` on Windows...
                        if dep.lower() in ("libcrypto-3.dll", "libssl-3.dll"):
                            if instpath and os.path.dirname(instpath).lower() == os.path.dirname(found["instpath"]).lower():
                                continue
                            elif instpath and os.path.dirname(instpath).lower() == os.path.dirname(file["instpath"]).lower():
                                found = file
                                continue
                        raise Exception(f"ambiguous dependency {dep}")
        return found

    def get_jar_deps(abspath):
        with open(abspath, "rb") as f:
            if not zipfile.is_zipfile(f):
                raise Exception(f"cannot parse jar: {abspath}")
            with zipfile.ZipFile(f) as archive:
                try:
                    text = archive.read("META-INF/MANIFEST.MF").decode("utf-8", "replace")
                except KeyError: # some .jars are help content ...
                    return (None, set())

                headers = {}
                name = None
                for line in text.splitlines():
                    if not line.strip():
                        break
                    if line.startswith(" ") and name is not None:
                        headers[name] += line[1:]
                    elif ":" in line:
                        name, _, value = line.partition(":")
                        name = name.strip()
                        headers[name] = value.strip()
                deps = [item for item in headers.get("Class-Path", "").split()
                        if item != "../" and item != ".."]
                for dep in deps:
                    if not(find_dep(files_by_package, dep)): # expect all jars to exist
                        raise Exception(f"cannot find jar dependency: {dep}")
                return ("JVM", set(deps))

    def get_elf_deps(abspath):
        with open(abspath, "rb") as f:
            elf = elftools.elf.elffile.ELFFile(f)
            needed = set()
            dynamic = next(elf.iter_segments(type="PT_DYNAMIC"), None)
            if dynamic is not None:
                for tag in dynamic.iter_tags():
                    if tag.entry.d_tag == "DT_NEEDED":
                        needed.add(tag.needed)
            return needed

    def get_pe_deps(abspath):
        result = set()
        pe = pefile.PE(abspath, fast_load=True)
        pe.parse_data_directories(
            directories = [
                pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_IMPORT"],
                pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT"],
            ])
        for entry in getattr(pe, "DIRECTORY_ENTRY_IMPORT", []):
            result.add(entry.dll.decode("ascii", "replace"))
        for entry in getattr(pe, "DIRECTORY_ENTRY_DELAY_IMPORT", []):
            result.add(entry.dll.decode("ascii", "replace"))
        pe.close()

        # Managed metadata only appears once data directories have been parsed
        managed = dnfile.dnPE(abspath)
        net = getattr(managed, "net", None)
        if net is not None:
            tables = net.mdtables
            if tables.AssemblyRef:
                for row in tables.AssemblyRef.rows:
                    name = str(row.Name)
                    if "." in name:
                        raise Exception(f"AssemblyRef not expected to have suffix: {name} in {abspath}")
                    result.add(name + ".dll")
            # Managed code may call unmanaged code: these are delayed imports
            if tables.ModuleRef:
                for row in tables.ModuleRef.rows:
                    name = str(row.Name)
                    if name and name not in result:
                        result.add(name)
        managed.close()
        return result

    MACH_O_MAGIC = (
        b"\xfe\xed\xfa\xce",
        b"\xce\xfa\xed\xfe",
        b"\xfe\xed\xfa\xcf",
        b"\xcf\xfa\xed\xfe",
    )

    def get_deps(abspath):
        (basename, ext) = os.path.splitext(abspath)
        # first, check scripts with known interpreters
        if ext == ".jar":
            return get_jar_deps(abspath)
        elif ext in (".class", ".java", ".bsh", ".js"):
            return ("JVM", set())
        elif ext in (".py", ".pyi"):
            if sys.platform == "win32":
                return ("program/python.exe", set())
            elif sys.platform == "linux":
                return ("program/python.bin", set())
            elif sys.platform == "darwin":
                return ("Contents/Resources/python", set())
            else:
                raise Exception("unexpected Python file on mobile platform")
        elif ext in (".xba", ".xdl"):
            if sys.platform == "win32" or sys.platform == "linux":
                return ("program/soffice.bin", set())
            elif sys.platform == "darwin":
                return ("Contents/MacOS/soffice", set())
            else:
                raise Exception("unexpected Basic file on mobile platform")
        elif ext == ".xsl":
            if sys.platform == "win32" or sys.platform == "linux":
                return ("program/soffice.bin", set())
            elif sys.platform == "darwin":
                return ("Contents/MacOS/soffice", set())
            else:
                raise Exception("TODO mobile platform")
        elif ext == ".pl":
            return ("Perl", set())
        elif ext == ".glsl":
            return ("OpenGL", set())
        elif ext == ".PS":
            return ("Printer", set())
        # just try everything; executables do not have extensions, Python
        # libraries are named ".so" on macOS...
        else:
            with open(abspath, "rb") as f:
                header = f.read(4096)
                if header.startswith(b"\x7fELF"):
                    return (None, get_elf_deps(abspath))
                # no Universal Binaries are used so only check simple ones
                elif header in MACH_O_MAGIC:
                    return (None, get_mach_o_deps(abspath))
                elif header.startswith(b"MZ") and len(header) > 0x40 \
                    and f.seek(int.from_bytes(header[0x3C:0x40], "little")) \
                    and f.read(4) == b"PE\0\0":
                        return (None, get_pe_deps(abspath))
                else: # all kinds of script files
                    if header.startswith(b"#!"):
                        return (header[2:].split()[0].decode("ascii", "replace"), set())
                    # at the moment there are only /bin/sh scripts
                    raise Exception(f"Unknown interpreter: {abspath}")

    SYSDEPS = set() # just for debugging

    for package in files_by_package:
        for file in files_by_package[package]:
            abspath = file["abspath"]
            file["sha512"] = get_sha512(abspath)
            if bool(file["flags"] & FileFlags.EXECUTABLE):
                deps = []
                sysdeps = []
                temp = get_deps(abspath)
                assert isinstance(temp, tuple)
                (interpreter, alldeps) = temp
                if interpreter is not None:
                    if interpreter.find("/") == -1 or interpreter[0] == '/':
                        sysdeps.append(interpreter)
                    else: # bundled interpreters have relative paths
                        deps.append(interpreter)
                for dep in alldeps:
                    depfile = find_dep(files_by_package, dep, file["instpath"])
                    if depfile is None:
                        if files_by_package_extra_deps is not None:
                            depfile = find_dep(files_by_package_extra_deps, dep, file["instpath"])
                    if depfile is None:
                        sysdeps.append(dep)
                    else:
                        deps.append(depfile["instpath"])
                file["deps"] = deps
                file["sysdeps"] = sysdeps
                SYSDEPS = SYSDEPS.union(sysdeps)

#    print(f"SYSDEPS: {SYSDEPS}")


def add_static_dependencies(files_by_package, externalstaticlink, with_path):
    """Add static link dependencies on externals to files."""

    for package in files_by_package:
        for file in files_by_package[package]:
            instpath = file["instpath"]
            filename = instpath if with_path else os.path.basename(instpath)
            if filename in externalstaticlink:
                # special case for CPython modules static linking, the annotations
                # in the build system affect everything in the Package
                if instpath.find("python-core") != -1 and with_path:
                    if instpath.endswith(".py"):
                        continue
                    if len(externalstaticlink[filename].difference({"bzip2", "libexpat", "libffi", "openssl", "zlib"})) != 0:
                        raise Exception(f"Unexpected static linking in python module, please adapt: {externalstaticlink[filename]}")
                    if instpath.find("_bz2.") != -1 and "bzip2" in externalstaticlink[filename]:
                        file["externaldeps"] = {"bzip2"}
                    elif instpath.find("_ctypes.") != -1 and "libffi" in externalstaticlink[filename]:
                        file["externaldeps"] = {"libffi"}
                    elif (instpath.find("_ssl.") != -1 or instpath.find("_hashlib.") != -1) \
                            and "openssl" in externalstaticlink[filename]:
                        file["externaldeps"] = {"openssl"}
                    elif instpath.find("pyexpat.") != -1 and "libexpat" in externalstaticlink[filename]:
                        file["externaldeps"] = {"libexpat"}
                elif instpath.find("python3") + 2 == instpath.find(".dll") and with_path:
                    if "zlib" in externalstaticlink[filename]:
                        file["externaldeps"] = {"zlib"}
                else:
                    file["externaldeps"] = externalstaticlink[filename]


def add_merge_module(files_by_package, externals_by_package, install_script):
    """Add any MSI merge modules as dummy files to root package."""

    license_spdx_id = None
    for msm in install_script["MergeModule"]:
        msm_file = install_script["MergeModule"][msm]["Name"]
        abspath = os.environ.get("MSM_PATH") + msm_file
        files_by_package["gid_Module_Root"].append({
            "flags": FileFlags.ARCHIVE | FileFlags.STRUCTURED,
            "instpath": msm_file, "abspath": abspath,
            "external": msm_file,
            "sha512": get_sha512(abspath), "deps": [], "sysdeps": []})

        # also add package component and license - this is a very special case
        vcver = os.environ.get("VCVER")
        if vcver.startswith("16."):
            vcyear = 2019
        elif vcver.startswith("17."):
            vcyear = 2022
        elif vcver.startswith("18."):
            vcyear = 2026
        else:
            raise Exception(f"Unknown Visual Studio version: {vcver}")

        pkg_spdx_id = make_spdx_id(f"SPDXRef-{msm_file}")
        pkg_element = {
            "type": "software_Package",
            "spdxId": pkg_spdx_id,
            "originatedBy": ["https://microsoft.com"],
            "creationInfo": "_:creationinfo",
            "name": msm_file,
            "software_homePage": "https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist",
            "externalIdentifiers": [{
                "externalIdentifierType": "cpe23",
                "identifier": f"cpe:2.3:a:microsoft:visual_c\\+\\+:{vcyear}:*:*:*:redistributable_package:*:*:*",
            }]
        }
        msm_elements = [pkg_element]

        # hack: all MSM end up in same MSI, so add license definition only to first one
        if license_spdx_id is None:
            custom_license_spdx_id = make_spdx_id("License-msvcrt-custom")
            license_spdx_id = make_spdx_id("License-msvcrt")
            license_cache["License-msvcrt"] = ([
                {
                    "type": "simplelicensing_LicenseExpression",
                    "spdxId": license_spdx_id,
                    "creationInfo": "_:creationInfo",
                    "simplelicensing_licenseExpression": f"LicenseRef-Microsoft-Visual-Studio-{vcyear}-Distributable-Code",
                    "simplelicensing_customIdToUri": [{
                        "type": "DictionaryEntry",
                        "key": f"LicenseRef-Microsoft-Visual-Studio-{vcyear}-Distributable-Code",
                        "value": custom_license_spdx_id
                    }]
                },
                {
                    "type": "expandedlicensing_CustomLicense",
                    "spdxId": custom_license_spdx_id,
                    "creationInfo": "_:creationInfo",
                    "name": f"Microsoft Visual Studio {vcyear} License Terms, Distributable Code",
                    "expandedlicensing_licenseText": "...verbatim terms...",
                    "expandedlicensing_isOsiApproved": False,
                    "expandedlicensing_isFsfLibre": False,
                    "expandedlicensing_seeAlso": ["https://visualstudio.microsoft.com/license-terms/"]
                }])

        msm_elements.append({
            "type": "Relationship",
            "spdxId": next_rel_id(),
            "creationInfo": "_:creationinfo",
            "from": pkg_spdx_id,
            "relationshipType": "hasDeclaredLicense",
            "to": [license_spdx_id]
        })

        sbom_externals[msm_file] = (msm_elements, "License-msvcrt", "License-msvcrt")
        externals_by_package["gid_Module_Root"].add(msm_file)


def check_files(*fileses):
    """Sanity check for duplicates."""
    allfiles = set()
    allexternaldeps = set()
    for files_by_package in fileses:
        for package in files_by_package:
            for file in files_by_package[package]:
                instpath = file["instpath"]
                if instpath in allfiles:
                    raise Exception(f"Unexpected duplicate instpath: {file}")
                allfiles.add(instpath)
                if "externaldeps" in file:
                    allexternaldeps = allexternaldeps.union(file["externaldeps"])
    return allexternaldeps

def check_externals(allexternaldeps, *externalses):
    """Check all externals in license files are used."""

    allexternals = allexternaldeps.copy()

    # fix known peculiarities
    for external in sbom_externals:
        if external.startswith("dict-"):
            # hack: assign to its dict package
            if sys.platform == "win32" or sys.platform == "darwin":
                externalses[0]["gid_Module_Root"].add(external)
            else:
                gid = "gid_Module_Root_Extension_Dictionary_" + external.replace("dict-","").replace("-spell","").replace("-hyph","").replace("-thes","").replace("-grammar","").replace("-","_").title().replace("Gug", "Gu")
                externalses[0][gid].add(external)
        # this is hidden in an extension
        if external == "xsltml":
            if sys.platform == "win32" or sys.platform == "darwin":
                externalses[0]["gid_Module_Root"].add(external)
            else:
                externalses[0]["gid_Module_Optional_Extensions_MEDIAWIKI"].add(external)
        if external == "nlpsolver":
            if sys.platform == "win32" or sys.platform == "darwin":
                externalses[0]["gid_Module_Root"].add(external)
            else:
                externalses[0]["gid_Module_Optional_Extensions_NLPSolver"].add(external)

    for externals_by_package in externalses:
        for externals in externals_by_package.values():
            for external in externals:
                allexternals.add(external)
    unused = set(sbom_externals.keys()).difference(allexternals)
    if len(unused) != 0:
        raise Exception(f"Unused externals: {unused}")
    # externals missing in license file will raise KeyError in get_external below


def sbom_add_files(files_by_package, externals_by_package):
    """Add all files to the SBOM graphs."""

    for package in files_by_package:
        sbom = next(sbom_data[key] for key in sbom_data if sbom_data[key][0] == package)
        graph = sbom[3]["@graph"]
        root_spdx_id = sbom[2]
        package_sysdeps = set()
        package_externals = {}
        package_licenses = set()

        def add_license(license):
            if not(license in package_licenses):
                graph.extend(license_cache[license])
                package_licenses.add(license)

        add_license(root_license)

        def add_external(external):
            if external is None:
                parent = root_spdx_id
            else:
                if external in package_externals:
                    parent = package_externals[external]
                else:
                    parent = sbom_externals[external][0][0]["spdxId"]
                    graph.extend(sbom_externals[external][0])
                    graph.append({
                        "type": "Relationship",
                        "spdxId": next_rel_id(),
                        "creationInfo": "_:creationinfo",
                        "from": root_spdx_id,
                        "relationshipType": "contains",
                        "to": [parent]
                    })
                    package_externals[external] = parent

                    add_license(sbom_externals[external][1])
                    add_license(sbom_externals[external][2])
            return parent

        for external in externals_by_package[package]:
            add_external(external)

        for file in files_by_package[package]:
            file_spdx_id = make_spdx_id(f"File-{file["instpath"]}")
            parent = add_external(file["external"])

            def get_flags(flags):
                result = []
                if flags & FileFlags.EXECUTABLE:
                    result.append("executable")
                if flags & FileFlags.ARCHIVE:
                    result.append("archive")
                if flags & FileFlags.STRUCTURED:
                    result.append("container")
                return result
            flags = get_flags(file["flags"])

            graph.append({
                "type": "software_File",
                "spdxId": file_spdx_id,
                "name": file["instpath"],
                "verifiedUsing": [{
                    "type": "Hash",
                    "algorithm": "sha512",
                    "hashValue": file["sha512"],
                    "software_additionalPurpose": flags,
                    "comment": "software_additionalPurpose field is used to indicate the properties of BSI TR-03183-2"
                    }]
                })
            graph.append({
                    "type": "Relationship",
                    "from": parent,
                    "relationshipType": "hasDistributionArtifact",
                    "to": [file_spdx_id],
                    # "incomplete" because filter_files throws everything without a flag away
                    "completeness": "incomplete"
                })

            # for dependencies, hopefully it will work if it's in another package
            if "deps" in file:
                deps = [make_spdx_id(f"File-{dep}") for dep in file["deps"]]
                if len(deps) != 0:
                    graph.append({
                            "type": "Relationship",
                            "from": file_spdx_id,
                            "relationshipType": "dependsOn",
                            "to": deps,
                            "completeness": "noAssertion"
                        })

            if "externaldeps" in file:
                externaldeps = [add_external(dep) for dep in file["externaldeps"]]
                graph.append({
                        "type": "Relationship",
                        "from": file_spdx_id,
                        "relationshipType": "contains",
                        "software_softwareLinkage": "static",
                        "to": externaldeps,
                        "completeness": "noAssertion"
                    })

            if "sysdeps" in file:
                sysdeps = []
                for dep in file["sysdeps"]:
                    sysdep_spdx_id = make_spdx_id(f"Sysdep-{dep}")
                    if not(dep in package_sysdeps):
                        graph.append({
                                "type": "software_Package",
                                "spdxId": sysdep_spdx_id,
                                "creationInfo": "_:creationinfo",
                                "name": dep,
                                "comment": "Supplied by the operating system."
                            })
                        package_sysdeps.add(dep)
                    sysdeps.append(sysdep_spdx_id)
                if len(sysdeps) != 0:
                    graph.append({
                            "type": "Relationship",
                            "from": file_spdx_id,
                            "relationshipType": "dependsOn",
                            "to": sysdeps,
                            "completeness": "noAssertion"
                        })


def gen_product(ziplist, packinfos, install_script, languages, externalsfile,
        externalstaticfile, externalpackagestaticfile, product, filelistdirs,
        files_extra_deps = None):
    """Generate SBOM graph for a product."""

    init_filelistdirs(ziplist, product, filelistdirs)
    root_gids = gen_packages(packinfos, ziplist, languages, product)
    files_by_package = process_install_script(install_script, root_gids)
    externalfiles = read_externals(externalsfile)
    externalstaticlink = read_external_staticlink(externalstaticfile)
    externalpackagestaticlink = read_external_staticlink(externalpackagestaticfile)
    assign_externals(files_by_package, externalfiles)
    files = locate_files(files_by_package, ziplist, product, filelistdirs)
    externals_by_package = filter_files(files)
    add_dependencies(files, files_extra_deps)
    add_static_dependencies(files, externalstaticlink, False)
    add_static_dependencies(files, externalpackagestaticlink, True)
    add_merge_module(files, externals_by_package, install_script)
    return (files, externals_by_package)


if __name__ == "__main__":
    if len(sys.argv) < 16:
        print("Usage: python create-sbom.py <path of output SPDX JSON files> <path of LICENSE.html> <path of openoffice.lst> <5 packinfo> <path of install script> <packinfo> <path of install script> <languages> <externals> <externalstatic> <externalpackagestatic>")
    else:
        sbom_path = sys.argv[1]
        license_path = sys.argv[2]
        process_license_file(license_path)
        ziplist = parse_ziplist(sys.argv[3])
        resolve_ziplist_inheritance(ziplist)
        packinfos = []
        packinfos += parse_packinfo(sys.argv[4])
        packinfos += parse_packinfo(sys.argv[5])
        packinfos += parse_packinfo(sys.argv[6])
        packinfos += parse_packinfo(sys.argv[7])
        packinfos += parse_packinfo(sys.argv[8])
        install_script = parse_install_script(sys.argv[9])
        languages = set(sys.argv[12].split())
        externalsfile = sys.argv[13]
        externalstaticfile = sys.argv[14]
        externalpackagestaticfile = sys.argv[15]

        (files_product, externals_product) = gen_product(
            ziplist, packinfos, install_script, languages,
            externalsfile, externalstaticfile, externalpackagestaticfile,
            productname, filelistdirs)

        if "ODK" in os.environ.get("BUILD_TYPE").split(" "):
            packinfos_sdk = parse_packinfo(sys.argv[10])
            install_script_sdk = parse_install_script(sys.argv[11])
            (files_sdk, externals_sdk) = gen_product(
                ziplist, packinfos_sdk, install_script_sdk, languages,
                externalsfile, externalstaticfile, externalpackagestaticfile,
                productname_sdk, filelistdirs_sdk, files_product)

            allexternaldeps = check_files(files_product, files_sdk)
            check_externals(allexternaldeps, externals_product, externals_sdk)
            sbom_add_files(files_product, externals_product)
            sbom_add_files(files_sdk, externals_sdk)
        else:
            allexternaldeps = check_files(files_product)
            check_externals(allexternaldeps, externals_product)
            sbom_add_files(files_product, externals_product)

        for package, data in sbom_data.items():
            filename = f"{package}-sbom.spdx.json"
            filepath = os.path.join(sbom_path, filename)
            with open(filepath, "w", encoding="utf-8") as file:
                json.dump(data[3], file, indent=2)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
