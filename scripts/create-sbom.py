#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Generate the SPDX 3.0.1 SBOM for Collabora Online, following the same
# conventions as the engine's solenv/bin/create-sbom.py so the two sets of
# documents can be consumed uniformly.
#
# Data sources:
# - engine build tree (config_host.mk -> SRCDIR -> download.lst) for the
#   versions and source tarball hashes of the statically linked C++ externals
# - gbuild/Executable_*.mk for which executable links which external
# - the engine's external/poco/StaticLibrary_Poco*.mk for the third-party
#   sources bundled inside POCO, with versions probed from the unpacked poco
#   tarball in the engine workdir
# - DT_NEEDED of the built binaries, to verify what is actually linked
#   statically in this build and what comes from the operating system
# - browser/npm-shrinkwrap.json + the shipped-JS manifest written by
#   browser/Makefile.am for the runtime npm dependency closure
# - scripts/sbom-externals.json for everything that cannot be discovered
#   mechanically (vendored sources, vendors, licenses, homepages)
# - browser/branding, when the build applied a theme (--branding-theme): its
#   files in dist belong to the Apache-2.0 branding component, not the root
#
# With --brand-package it instead describes one of the brand packages that
# browser/branding/packaging/make-brand-packages.sh stages: the Apache-2.0
# theme is the root, and the Nextcloud community theme that the package may
# carry under dist/nextcloud is a component of its own. No build tree or
# engine is needed for that.

import argparse
import base64
import hashlib
import json
import os
import re
import struct
import sys
import uuid
from datetime import datetime, timezone

INSTALLED_EXECUTABLES = [
    "coolconfig",
    "coolconvert",
    "coolforkit-caps",
    "coolforkit-ns",
    "coolmount",
    "coolstress",
    "coolwsd",
]

# rsync EXCLUDES of browser/Makefile.am install-data-hook (release mode); the
# two dist/src files copied explicitly afterwards are re-added below.
DIST_EXCLUDES = (
    "src",
    "debug.html",
    "documents.html",
    "framed.doc.html",
    "framed.html",
    "load.doc.html",
    "multidocs.html",
    "tsconfig.tsbuildinfo",
    "wasm.html",
)
DIST_EXPLICIT = (
    "src/layer/tile/CanvasTileUtils.js",
    "src/app/TaskWorker.js",
)

timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")

verbose = False

def log(message):
    if verbose:
        print(message, file=sys.stderr)


# ---------------------------------------------------------------------------
# SPDX emission helpers, following engine/solenv/bin/create-sbom.py
# ---------------------------------------------------------------------------

spdx_id_cache = {}

def make_spdx_id(fragment):
    """Create a URN UUID for an SPDX element; stable per fragment."""
    if fragment not in spdx_id_cache:
        spdx_id_cache[fragment] = f"urn:uuid:{uuid.uuid4()}"
    return spdx_id_cache[fragment]


def next_rel_id():
    """Generate a unique relationship URN UUID."""
    return f"urn:uuid:{uuid.uuid4()}"


license_cache = {}
custom_licenses = {}

def add_license_relationship(graph, from_id, type, license_expr):
    """Add a license expression element and a license relationship."""

    assert type in ("hasDeclaredLicense", "hasConcludedLicense")
    if license_expr not in license_cache:
        license_id = make_spdx_id(f"License-{license_expr}")
        if license_expr in custom_licenses:
            custom = custom_licenses[license_expr]
            license_cache[license_expr] = [{
                "type": "expandedlicensing_CustomLicense",
                "spdxId": license_id,
                "creationInfo": "_:creationinfo",
                "name": custom["name"],
                "simplelicensing_licenseText": custom["text"],
            }]
        else:
            license_cache[license_expr] = [{
                "type": "simplelicensing_LicenseExpression",
                "spdxId": license_id,
                "creationInfo": "_:creationinfo",
                "simplelicensing_licenseExpression": license_expr,
            }]
    else:
        license_id = license_cache[license_expr][0]["spdxId"]

    graph.append({
        "type": "Relationship",
        "spdxId": next_rel_id(),
        "creationInfo": "_:creationinfo",
        "from": from_id,
        "relationshipType": type,
        "to": [license_id],
    })


def get_sha512(abspath):
    digest = hashlib.sha512()
    with open(abspath, "rb") as f:
        while True:
            chunk = f.read(1 << 20)
            if not chunk:
                break
            digest.update(chunk)
    return digest.hexdigest()


def extract_version_from_filename(filename):
    filename = re.sub(r"\.(tar\.gz|tar\.xz|tar\.bz2|zip)$", "", filename)
    # poco-1.15.4-all: the "-all" flavor suffix is not part of the version
    filename = re.sub(r"-all$", "", filename)
    match = re.search(r"\d+(?:\.\d+)+(?:-[\w\.]+)*", filename)
    if match:
        return match.group()
    return None


def cpe23(vendor, name, version):
    def any_(string):
        return "*" if string is None or len(string) == 0 else string
    return f"cpe:2.3:a:{any_(vendor)}:{name}:{any_(version)}:*:*:*:*:*:*:*"


# ---------------------------------------------------------------------------
# Input parsing
# ---------------------------------------------------------------------------

def parse_make_variables(path, names):
    """Read simple 'NAME := value' / 'export NAME=value' assignments."""
    values = {}
    pattern = re.compile(
        r"^(?:export\s+)?(" + "|".join(re.escape(n) for n in names)
        + r")\s*:?=\s*(.*?)\s*$")
    with open(path, encoding="utf-8") as f:
        for line in f:
            match = pattern.match(line)
            if match:
                values[match.group(1)] = match.group(2)
    return values


def local_path(path):
    """A path as the engine's build recorded it, usable here: on Windows the
    engine records C:/..., whether its build used MSYS2 or WSL for the Unix
    helper tools, and this script runs in either of them: MSYS2 calls it
    /c/..., WSL /mnt/c/..."""
    if os.path.exists(path):
        return path
    match = re.match(r"^([A-Za-z]):[/\\](.*)$", path)
    if match:
        drive = match.group(1).lower()
        rest = match.group(2).replace("\\", "/")
        for candidate in (f"/mnt/{drive}/{rest}", f"/cygdrive/{drive}/{rest}",
                          f"/{drive}/{rest}"):
            if os.path.exists(candidate):
                return candidate
    return path


def read_engine_tarballs(engine_builddir, prefixes):
    """Find version and sha256 of engine-built externals via download.lst."""
    config = parse_make_variables(
        os.path.join(engine_builddir, "config_host.mk"), ["SRCDIR"])
    if "SRCDIR" not in config:
        raise Exception(f"no SRCDIR in {engine_builddir}/config_host.mk")
    engine_srcdir = local_path(config["SRCDIR"])
    names = []
    for prefix in prefixes:
        names.extend([f"{prefix}_TARBALL", f"{prefix}_SHA256SUM"])
    tarballs = parse_make_variables(
        os.path.join(engine_srcdir, "download.lst"), names)
    result = {}
    for prefix in prefixes:
        tarball = tarballs.get(f"{prefix}_TARBALL")
        sha256 = tarballs.get(f"{prefix}_SHA256SUM")
        if tarball is None or sha256 is None:
            raise Exception(f"no {prefix}_TARBALL/_SHA256SUM in download.lst")
        result[prefix] = (tarball, sha256)
    return result, engine_srcdir


def parse_gbuild_executables(srcdir):
    """Read gbuild/Executable_*.mk: externals and POCO usage per binary."""
    result = {}
    for exe in INSTALLED_EXECUTABLES:
        path = os.path.join(srcdir, "gbuild", f"Executable_{exe}.mk")
        with open(path, encoding="utf-8") as f:
            content = f.read()
        externals = set()
        match = re.search(
            r"gb_Executable_use_externals," + re.escape(exe) + r"\b,?\s*\\\n(.*?)\)\)",
            content, re.DOTALL)
        if match:
            for token in match.group(1).replace("\\", " ").split():
                # openssl_headers is the include-only variant of openssl
                externals.add("openssl" if token == "openssl_headers" else token)
        if "online_poco_whole_libs" in content \
                or re.search(r"\bPocoFoundation\b", content):
            externals.add("poco")
        result[exe] = externals
    return result


def poco_bundled_names(engine_srcdir):
    """Third-party sources compiled into the POCO static libraries."""
    names = set()
    poco_dir = os.path.join(engine_srcdir, "external", "poco")
    for entry in sorted(os.listdir(poco_dir)):
        if not (entry.startswith("StaticLibrary_Poco") and entry.endswith(".mk")):
            continue
        with open(os.path.join(poco_dir, entry), encoding="utf-8") as f:
            names.update(re.findall(r"dependencies/([A-Za-z0-9_]+)/", f.read()))
    return names


def probe_poco_bundled_version(name, poco_unpacked):
    """Best-effort version of a source bundled in the poco tarball."""
    dep_dir = os.path.join(poco_unpacked, "dependencies", name)
    if name == "pcre2":
        with open(os.path.join(dep_dir, "src", "pcre2.h"), encoding="utf-8") as f:
            content = f.read()
        major = re.search(r"#define PCRE2_MAJOR\s+(\d+)", content)
        minor = re.search(r"#define PCRE2_MINOR\s+(\d+)", content)
        if major and minor:
            return f"{major.group(1)}.{minor.group(1)}"
    elif name == "v8_double_conversion":
        with open(os.path.join(dep_dir, "CMakeLists.txt"), encoding="utf-8") as f:
            match = re.search(r"releases/tag/v([\d.]+)", f.read())
        if match:
            return match.group(1)
    elif name == "utf8proc":
        with open(os.path.join(dep_dir, "src", "utf8proc.h"), encoding="utf-8") as f:
            content = f.read()
        parts = re.findall(r"UTF8PROC_VERSION_(?:MAJOR|MINOR|PATCH)\s+(\d+)", content)
        if len(parts) == 3:
            return ".".join(parts)
    return None


# Map DT_NEEDED sonames to the externals they satisfy: if a binary loads the
# library dynamically, the engine-built static external is not inside it.
SONAME_EXTERNALS = {
    "libz.so": "zlib",
    "libpng16.so": "libpng",
    "libssl.so": "openssl",
    "libcrypto.so": "openssl",
    "libzstd.so": "zstd",
    "libexpat.so": "expat",
}

def soname_external(soname):
    for prefix, external in SONAME_EXTERNALS.items():
        if soname == prefix or soname.startswith(prefix + "."):
            return external
    return None


def elf_needed(path):
    """Return the DT_NEEDED sonames of an ELF binary (no external tools)."""
    with open(path, "rb") as f:
        ident = f.read(16)
        if ident[:4] != b"\x7fELF":
            return []
        is64 = ident[4] == 2
        endian = "<" if ident[5] == 1 else ">"

        def read_at(offset, fmt):
            f.seek(offset)
            return struct.unpack(endian + fmt, f.read(struct.calcsize(endian + fmt)))

        if is64:
            e_phoff, = read_at(0x20, "Q")
            e_phentsize, e_phnum = read_at(0x36, "HH")
            ph_fmt, dyn_fmt = "IIQQQQQQ", "qQ"
        else:
            e_phoff, = read_at(0x1c, "I")
            e_phentsize, e_phnum = read_at(0x2a, "HH")
            ph_fmt, dyn_fmt = "IIIIIIII", "iI"

        loads = []
        dynamic = None
        for i in range(e_phnum):
            fields = read_at(e_phoff + i * e_phentsize, ph_fmt)
            if is64:
                p_type, _, p_offset, p_vaddr, _, p_filesz = fields[:6]
            else:
                p_type, p_offset, p_vaddr, _, p_filesz = fields[:5]
            if p_type == 1:  # PT_LOAD
                loads.append((p_vaddr, p_offset, p_filesz))
            elif p_type == 2:  # PT_DYNAMIC
                dynamic = (p_offset, p_filesz)
        if dynamic is None:
            return []

        def vaddr_to_offset(vaddr):
            for v, o, size in loads:
                if v <= vaddr < v + size:
                    return o + (vaddr - v)
            return None

        needed_offsets = []
        strtab_vaddr = None
        entry_size = struct.calcsize(endian + dyn_fmt)
        f.seek(dynamic[0])
        data = f.read(dynamic[1])
        for i in range(0, len(data) - entry_size + 1, entry_size):
            d_tag, d_val = struct.unpack_from(endian + dyn_fmt, data, i)
            if d_tag == 0:  # DT_NULL
                break
            if d_tag == 1:  # DT_NEEDED
                needed_offsets.append(d_val)
            elif d_tag == 5:  # DT_STRTAB
                strtab_vaddr = d_val
        if strtab_vaddr is None:
            return []
        strtab_offset = vaddr_to_offset(strtab_vaddr)
        if strtab_offset is None:
            return []

        sonames = []
        for value in needed_offsets:
            f.seek(strtab_offset + value)
            raw = b""
            while True:
                chunk = f.read(256)
                if not chunk:
                    break
                end = chunk.find(b"\0")
                if end != -1:
                    raw += chunk[:end]
                    break
                raw += chunk
            sonames.append(raw.decode("utf-8"))
        return sonames


def pe_needed(path, engine_srcdir):
    """The DLLs a PE binary imports, also delay-loaded, with the engine's
    copy of pefile."""
    sys.path.insert(0, os.path.join(engine_srcdir, "external", "pip-wheels",
                                    "pefile-2024.8.26-py3-none-any.whl"))
    import pefile
    pe = pefile.PE(path, fast_load=True)
    pe.parse_data_directories(directories=[
        pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_IMPORT"],
        pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT"],
    ])
    result = []
    for entry in getattr(pe, "DIRECTORY_ENTRY_IMPORT", []) \
            + getattr(pe, "DIRECTORY_ENTRY_DELAY_IMPORT", []):
        name = entry.dll.decode("ascii", "replace")
        if name not in result:
            result.append(name)
    pe.close()
    return result


def mach_o_needed(path, engine_srcdir):
    """The dylibs and frameworks a Mach-O binary loads, with the engine's
    copy of macholib. Names relative to the bundle are reduced to the file
    name, system ones are kept whole, as the engine's generator does."""
    wheels = os.path.join(engine_srcdir, "external", "pip-wheels")
    sys.path.insert(0, os.path.join(wheels, "altgraph-0.17.5-py2.py3-none-any.whl"))
    sys.path.insert(0, os.path.join(wheels, "macholib-1.16.4-py2.py3-none-any.whl"))
    import macholib.MachO
    result = []
    for header in macholib.MachO.MachO(path).headers:
        for (_, _, name) in header.walkRelocatables():
            if name.startswith("@"):
                name = name.rsplit("/", 1)[-1]
            if name not in result:
                result.append(name)
    return result


def binary_needed(path, engine_srcdir):
    """The libraries an ELF, PE or Mach-O binary loads, None if the file is
    none of these."""
    with open(path, "rb") as f:
        magic = f.read(4)
    if magic == b"\x7fELF":
        return elf_needed(path)
    if magic[:2] == b"MZ":
        return pe_needed(path, engine_srcdir)
    if magic in (b"\xcf\xfa\xed\xfe", b"\xce\xfa\xed\xfe",
                 b"\xfe\xed\xfa\xcf", b"\xfe\xed\xfa\xce",
                 b"\xca\xfe\xba\xbe"):
        return mach_o_needed(path, engine_srcdir)
    return None


def integrity_sha512_hex(integrity):
    """Last sha512 token of an npm integrity value: shrinkpack appends the
    hash of its vendored tarball, which is the artifact the build actually
    consumes and the one our source locator points at."""
    result = None
    for token in integrity.split():
        if token.startswith("sha512-"):
            result = base64.b64decode(token[len("sha512-"):]).hex()
    return result


def npm_purl(name, version):
    return "pkg:npm/" + name.replace("@", "%40") + f"@{version}"


def read_npm_runtime(browser_srcdir, browser_builddir, shipped_js_path):
    """Compute the runtime npm dependency closure from npm-shrinkwrap.json.

    Seeds are the packages owning files in the shipped-JS manifest written by
    browser/Makefile.am plus the bare require()s browserified into
    admin-bundle.js; the closure follows the shrinkwrap 'dependencies' edges.
    """
    with open(os.path.join(browser_builddir, "npm-shrinkwrap.json"),
              encoding="utf-8") as f:
        packages = json.load(f)["packages"]

    def package_key(name, parent_key=None):
        if parent_key:
            nested = f"{parent_key}/node_modules/{name}"
            if nested in packages:
                return nested
        key = f"node_modules/{name}"
        return key if key in packages else None

    seeds = set()
    with open(shipped_js_path, encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line.startswith("node_modules/"):
                continue
            parts = line.split("/")
            name = "/".join(parts[1:3]) if parts[1].startswith("@") else parts[1]
            seeds.add(name)

    with open(os.path.join(browser_srcdir, "admin", "main-admin.js"),
              encoding="utf-8") as f:
        for name in re.findall(r"require\(['\"]([^'\"]+)['\"]\)", f.read()):
            if not name.startswith(".") and package_key(name):
                seeds.add(name)

    result = {}
    def add(name, parent_key):
        key = package_key(name, parent_key)
        if key is None:
            raise Exception(f"npm package not in shrinkwrap: {name}")
        if name in result:
            return
        entry = packages[key]
        result[name] = {
            "version": entry["version"],
            "license": entry.get("license"),
            "sha512": integrity_sha512_hex(entry.get("integrity", "")),
            "resolved": entry.get("resolved", ""),
            "dependencies": sorted(entry.get("dependencies", {}).keys()),
        }
        for dep in result[name]["dependencies"]:
            add(dep, key)

    for seed in sorted(seeds):
        add(seed, None)
    return seeds, result


# ---------------------------------------------------------------------------
# Installed file discovery
# ---------------------------------------------------------------------------

def is_executable_path(instpath):
    return instpath.startswith("usr/bin/") \
        or os.path.splitext(instpath)[1] in (".js", ".py", ".sh")


def collect_files(srcdir, builddir, enable_mobileapp=False):
    """Enumerate the files 'make install' ships, as (instpath, abspath)."""
    files = []

    def add(instpath, abspath):
        if not os.path.isfile(abspath):
            raise Exception(f"installed file missing from build: {abspath}")
        files.append({"instpath": instpath, "abspath": abspath})

    for exe in INSTALLED_EXECUTABLES:
        add(f"usr/bin/{exe}", os.path.join(builddir, exe))
    add("usr/bin/coolwsd-systemplate-setup",
        os.path.join(srcdir, "coolwsd-systemplate-setup"))

    add("usr/share/coolwsd/discovery.xml", os.path.join(srcdir, "discovery.xml"))
    add("usr/share/coolwsd/favicon.ico", os.path.join(srcdir, "favicon.ico"))
    add("etc/coolwsd/coolwsd.xml", os.path.join(builddir, "coolwsd.xml"))

    dist = os.path.join(builddir, "browser", "dist")
    for root, dirs, names in os.walk(dist):
        rel_root = os.path.relpath(root, dist)
        if rel_root == ".":
            dirs[:] = [d for d in dirs if d != "src"]
        for name in sorted(names):
            rel = name if rel_root == "." else f"{rel_root}/{name}"
            if rel_root == "." and name in DIST_EXCLUDES:
                continue
            if enable_mobileapp and rel == "l10n-all.js":
                continue
            add(f"usr/share/coolwsd/browser/dist/{rel}", os.path.join(root, name))
    for rel in DIST_EXPLICIT:
        add(f"usr/share/coolwsd/browser/dist/{rel}", os.path.join(dist, rel))

    templates = os.path.join(builddir, "browser", "admin-templates")
    if not os.path.isdir(templates):
        templates = os.path.join(srcdir, "browser", "admin", "admintemplates")
    for root, dirs, names in os.walk(templates):
        rel_root = os.path.relpath(root, templates)
        for name in sorted(names):
            rel = name if rel_root == "." else f"{rel_root}/{name}"
            add(f"usr/share/coolwsd/browser/admin-templates/{rel}",
                os.path.join(root, name))

    return files


SERVER_DIST_PREFIX = "usr/share/coolwsd/browser/dist/"

def file_external(instpath, npm_names, vendored_by_dist_path,
                  firstparty_by_dist_path, dist_prefix=SERVER_DIST_PREFIX):
    """Which component other than the root an installed file belongs to."""
    if instpath.startswith(dist_prefix):
        rel = instpath[len(dist_prefix):]
        if rel.startswith("node_modules/"):
            parts = rel.split("/")
            name = "/".join(parts[1:3]) if parts[1].startswith("@") else parts[1]
            if name in npm_names:
                return ("npm", name)
        if rel in vendored_by_dist_path:
            return ("vendored", vendored_by_dist_path[rel])
        if rel in firstparty_by_dist_path:
            return ("firstparty", firstparty_by_dist_path[rel])
    return None


def branding_dist_files(srcdir, builddir):
    """The files of the theme from browser/branding that are in browser/dist.

    brand.sh writes the stylesheets and scripts to the top of dist and copies
    the images the stylesheets reference next to the tree's own images, so
    those are told apart by name against the theme's image directory."""
    dist = os.path.join(builddir, "browser", "dist")
    files = [name for name in os.listdir(dist)
             if name.startswith("branding") and name.endswith((".css", ".js"))]
    theme_images = os.path.join(srcdir, "browser", "branding", "themes",
                                "collabora-online-brand", "images")
    for name in os.listdir(theme_images):
        if os.path.isfile(os.path.join(dist, "images", name)):
            files.append(f"images/{name}")
    return sorted(files)


# ---------------------------------------------------------------------------
# Document assembly
# ---------------------------------------------------------------------------

def sbom_skeleton(version, root_license, name="CollaboraOnline-coolwsd"):
    root_spdx_id = make_spdx_id("SPDXRef-coolwsd")
    document_spdx_id = f"urn:uuid:{uuid.uuid4()}"
    tool_spdx_id = make_spdx_id("SPDXRef-Tool-CustomScript")

    document = {
        "@context": "https://spdx.org/rdf/3.0.1/spdx-context.jsonld",
        "@graph": [
            {
                "type": "Organization",
                "spdxId": "https://collaboraoffice.com",
                "creationInfo": "_:creationinfo",
                "externalIdentifier": [{
                    "type": "ExternalIdentifier",
                    "externalIdentifierType": "email",
                    "identifier": "hello@collaboraoffice.com",
                }],
            },
            {
                "type": "CreationInfo",
                "@id": "_:creationinfo",
                "specVersion": "3.0.1",
                "created": timestamp,
                "createdBy": ["https://collaboraoffice.com"],
                "createdUsing": [tool_spdx_id],
            },
            {
                "type": "Tool",
                "spdxId": tool_spdx_id,
                "creationInfo": "_:creationinfo",
                "name": "Custom Script",
            },
            {
                "type": "SpdxDocument",
                "spdxId": document_spdx_id,
                "creationInfo": "_:creationinfo",
                "name": name,
                "rootElement": [root_spdx_id],
                "profileConformance": ["core", "software", "simpleLicensing"],
            },
            {
                "type": "software_Package",
                "spdxId": root_spdx_id,
                "creationInfo": "_:creationinfo",
                "name": name,
                "software_packageVersion": version,
            },
            {
                "type": "Relationship",
                "spdxId": next_rel_id(),
                "creationInfo": "_:creationinfo",
                "from": document_spdx_id,
                "relationshipType": "describes",
                "to": [root_spdx_id],
            },
        ],
    }
    graph = document["@graph"]
    add_license_relationship(graph, root_spdx_id, "hasConcludedLicense",
                             root_license)
    return document, root_spdx_id


def add_source_artifact(graph, fragment, pkg_spdx_id, locator, sha256=None,
                        sha512=None):
    """Emit the source artifact a package is generated from."""
    source_spdx_id = make_spdx_id(f"{fragment}-source")
    source_element = {
        "type": "software_Package",
        "spdxId": source_spdx_id,
        "creationInfo": "_:creationinfo",
        "software_primaryPurpose": "source",
    }
    if locator:
        source_element["externalRef"] = [{
            "type": "ExternalRef",
            "externalRefType": "sourceArtifact",
            "locator": [locator],
        }]
    if sha256 or sha512:
        source_element["verifiedUsing"] = [{
            "type": "Hash",
            "algorithm": "sha256" if sha256 else "sha512",
            "hashValue": sha256 if sha256 else sha512,
        }]
    graph.append(source_element)
    graph.append({
        "type": "Relationship",
        "spdxId": next_rel_id(),
        "creationInfo": "_:creationinfo",
        "from": source_spdx_id,
        "relationshipType": "generates",
        "to": [pkg_spdx_id],
        "completeness": "complete",
    })


def make_component(graph, fragment, name, vendor, version, homepage, locator,
                   sha256, declared, concluded, sha512=None, purl=None,
                   cpe_name=None, with_cpe=True, comment=None):
    """Emit a software_Package + source artifact, engine-shape.

    cpe_name overrides the CPE product name where it differs from the name we
    use (NVD calls expat 'libexpat'); a product name no CPE uses matches no
    CVE, and unlike a wrong vendor it cannot be saved by the '*' wildcard.
    with_cpe=False is for our own components, which have no CPE."""
    pkg_spdx_id = make_spdx_id(fragment)
    pkg_element = {
        "type": "software_Package",
        "spdxId": pkg_spdx_id,
        "originatedBy": ["https://collaboraoffice.com"],
        "creationInfo": "_:creationinfo",
        "name": name,
        "externalIdentifier": [],
    }
    if with_cpe:
        pkg_element["externalIdentifier"].append({
            "type": "ExternalIdentifier",
            "externalIdentifierType": "cpe23",
            "identifier": cpe23(vendor, cpe_name or name, version),
        })
    if purl:
        pkg_element["externalIdentifier"].append({
            "type": "ExternalIdentifier",
            "externalIdentifierType": "packageUrl",
            "identifier": purl,
        })
    if not pkg_element["externalIdentifier"]:
        del pkg_element["externalIdentifier"]
    if homepage:
        pkg_element["software_homePage"] = homepage
    if version:
        pkg_element["software_packageVersion"] = version
    if comment:
        pkg_element["comment"] = comment
    graph.append(pkg_element)

    if locator or sha256 or sha512:
        add_source_artifact(graph, fragment, pkg_spdx_id, locator, sha256,
                            sha512)

    add_license_relationship(graph, pkg_spdx_id, "hasDeclaredLicense", declared)
    add_license_relationship(graph, pkg_spdx_id, "hasConcludedLicense", concluded)
    return pkg_spdx_id


def emit_file(graph, parent_id, instpath, abspath):
    """Emit an installed file with its hash as an artifact of parent_id."""
    file_spdx_id = make_spdx_id(f"File-{instpath}")
    flags = ["executable"] if is_executable_path(instpath) else []
    hash_element = {
        "type": "Hash",
        "algorithm": "sha512",
        "hashValue": get_sha512(abspath),
    }
    file_element = {
        "type": "software_File",
        "spdxId": file_spdx_id,
        "creationInfo": "_:creationinfo",
        "name": instpath,
        "verifiedUsing": [hash_element],
    }
    # where the engine's generator puts them
    if flags:
        file_element["software_additionalPurpose"] = flags
        file_element["comment"] = ("software_additionalPurpose field is "
            "used to indicate the properties of BSI TR-03183-2")
    graph.append(file_element)
    graph.append({
        "type": "Relationship",
        "spdxId": next_rel_id(),
        "creationInfo": "_:creationinfo",
        "from": parent_id,
        "relationshipType": "hasDistributionArtifact",
        "to": [file_spdx_id],
        "completeness": "incomplete",
    })
    return file_spdx_id


def add_contains(graph, parent_id, child_id, static=False):
    relationship = {
        "type": "Relationship",
        "spdxId": next_rel_id(),
        "creationInfo": "_:creationinfo",
        "from": parent_id,
        "relationshipType": "contains",
        "to": [child_id],
    }
    if static:
        relationship["software_softwareLinkage"] = "static"
        relationship["completeness"] = "noAssertion"
    graph.append(relationship)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def read_annotations(srcdir):
    with open(os.path.join(srcdir, "scripts", "sbom-externals.json"),
              encoding="utf-8") as f:
        return json.load(f)


def repo_url(source):
    return f"https://github.com/CollaboraOnline/online/tree/master/{source}"


def run_checks(args, annotations, gbuild_externals, poco_bundled, seeds,
               npm_runtime):
    """Consistency checks, in the spirit of the engine's check_externals."""
    errors = []

    for exe, externals in gbuild_externals.items():
        for external in externals:
            if external not in annotations["engine_externals"]:
                errors.append(f"{exe} links external with no entry in "
                              f"sbom-externals.json: {external}")
    used = set().union(*gbuild_externals.values())
    for external in annotations["engine_externals"]:
        if external not in used:
            errors.append(f"unused engine_externals entry: {external}")
    for exe in ("coolwsd", "coolforkit-ns", "coolforkit-caps"):
        if "poco" not in gbuild_externals[exe]:
            errors.append(f"{exe} does not link POCO? gbuild parse is broken")

    annotated_bundled = {entry["name"]: entry for entry in annotations["bundled"]}
    for name in poco_bundled:
        if name not in annotated_bundled:
            errors.append(f"POCO bundles a source with no entry in "
                          f"sbom-externals.json: {name}")
    for name, entry in annotated_bundled.items():
        # entries restricted to other platforms are kept for documentation
        if name not in poco_bundled and "platforms" not in entry:
            errors.append(f"stale bundled entry in sbom-externals.json: {name}")

    for section in ("vendored", "firstparty"):
        for entry in annotations[section]:
            source = os.path.join(args.srcdir, entry["source"])
            if not os.path.exists(source):
                errors.append(f"{section} source of {entry['name']} is gone: "
                              f"{entry['source']}")

    # Policy: direct dependencies are listed in THIRDPARTYLICENSES.
    with open(os.path.join(args.srcdir, "THIRDPARTYLICENSES"),
              encoding="utf-8") as f:
        thirdparty = f.read()
    for section in ("engine_externals", "vendored"):
        entries = annotations[section]
        entries = entries.values() if isinstance(entries, dict) else entries
        for entry in entries:
            if entry["thirdparty"] not in thirdparty:
                errors.append(f"not found in THIRDPARTYLICENSES: "
                              f"{entry['thirdparty']}")
    for name, heading in annotations["npm_thirdparty"].items():
        if heading not in thirdparty:
            errors.append(f"not found in THIRDPARTYLICENSES: {heading}")
    for seed in seeds:
        if seed not in annotations["npm_thirdparty"]:
            errors.append(f"directly shipped npm package missing from "
                          f"npm_thirdparty in sbom-externals.json: {seed}")
    for name in annotations["npm_thirdparty"]:
        if name not in npm_runtime:
            errors.append(f"stale npm_thirdparty entry (not in the runtime "
                          f"closure): {name}")

    return errors


NEXTCLOUD_THEME_PREFIX = "usr/share/coolwsd/browser/dist/nextcloud/"


def write_brand_package_sbom(args):
    """The SBOM of a brand package staged by make-brand-packages.sh.

    The root is the theme from browser/branding. The Nextcloud community
    theme, when the package carries it, comes from the online-branding
    repository under its own terms, so its files hang off a component of
    their own."""
    for option in ("package_root", "version", "output"):
        if not getattr(args, option):
            raise Exception(f"--brand-package needs --{option.replace('_', '-')}")

    annotations = read_annotations(args.srcdir)
    custom_licenses.update(annotations.get("custom_licenses", {}))
    entry = next(entry for entry in annotations["firstparty"]
                 if entry["source"] == "browser/branding")

    document, root_spdx_id = sbom_skeleton(
        args.version, entry["concluded"],
        name=f"CollaboraOnline-{args.brand_package}")
    graph = document["@graph"]
    add_license_relationship(graph, root_spdx_id, "hasDeclaredLicense",
                             entry["declared"])
    for element in graph:
        if element.get("spdxId") == root_spdx_id:
            element["software_homePage"] = entry["url"]
            element["originatedBy"] = ["https://collaboraoffice.com"]
    add_source_artifact(graph, "SPDXRef-coolwsd", root_spdx_id,
                        repo_url(entry["source"]))
    emitted_licenses = set()

    def emit_licenses(*license_exprs):
        for expr in license_exprs:
            if expr not in emitted_licenses:
                graph.extend(license_cache[expr])
                emitted_licenses.add(expr)

    emit_licenses(entry["declared"], entry["concluded"])

    nextcloud_id = None
    output = os.path.abspath(args.output)
    count = 0
    for root, dirs, names in os.walk(args.package_root):
        dirs.sort()
        for name in sorted(names):
            abspath = os.path.join(root, name)
            if os.path.abspath(abspath) == output:
                continue
            instpath = os.path.relpath(abspath, args.package_root)
            parent = root_spdx_id
            if instpath.startswith(NEXTCLOUD_THEME_PREFIX):
                if nextcloud_id is None:
                    nextcloud_id = make_component(
                        graph, "SPDXRef-NC-theme-community",
                        "NC-theme-community", None, args.version,
                        "https://www.collaboraonline.com/", None, None,
                        "NOASSERTION", "NOASSERTION", with_cpe=False,
                        comment="The Nextcloud community theme, generated "
                                "from the online-branding repository of "
                                "Collabora Productivity.")
                    emit_licenses("NOASSERTION")
                    add_contains(graph, root_spdx_id, nextcloud_id)
                parent = nextcloud_id
            emit_file(graph, parent, instpath, abspath)
            count += 1

    with open(output, "w", encoding="utf-8") as f:
        json.dump(document, f, indent=2)
        f.write("\n")
    log(f"wrote {output}: {len(graph)} elements, {count} files")


# ---------------------------------------------------------------------------
# Collabora Office desktop apps
# ---------------------------------------------------------------------------

# The engine's static libraries an app can name on its link line, and the
# external each belongs to
ENGINE_LIBRARY_EXTERNALS = {
    "libpng": "libpng",
    "liblibpng": "libpng",
    "zlib": "zlib",
    "zstd": "zstd",
    "libzstd": "zstd",
    "expat": "expat",
    "libexpat": "expat",
}

def engine_library_external(name, strict=True):
    """The external an engine library on a link line belongs to. strict:
    the link line names engine libraries only, so an unknown one needs an
    entry here; otherwise it is taken to be the operating system's."""
    if name.startswith("Poco") or name.startswith("libPoco"):
        return "poco"
    if name in ENGINE_LIBRARY_EXTERNALS:
        return ENGINE_LIBRARY_EXTERNALS[name]
    if strict:
        raise Exception(
            f"engine library with no external in create-sbom.py: {name}")
    return None


def qt_linked_externals(builddir, engine_builddir):
    """The engine externals coda-qt links statically: those of its link
    line that come from the engine's build tree. One that the configure of
    a flatpak, snap or FreeBSD build took from the system (-lz) is loaded
    dynamically instead."""
    names = ["POCOLIB", "PNG_LIBS", "ZSTD_LIBS", "ZLIB_LIBS", "OPENSSL_LIBS",
             "ENABLE_SSL"]
    values = parse_make_variables(os.path.join(builddir, "qt", "Makefile"),
                                  names)
    engine = os.path.realpath(engine_builddir) + os.sep

    def from_engine(value):
        for token in value.split():
            path = token[2:] if token.startswith("-L") else token
            if os.path.isabs(path) \
                    and os.path.realpath(path).startswith(engine):
                return True
        return False

    if not from_engine(values.get("POCOLIB", "")):
        raise Exception("coda-qt links a POCO from outside the engine: "
                        + values.get("POCOLIB", ""))
    result = {"poco"}
    for variable, external in (("PNG_LIBS", "libpng"), ("ZSTD_LIBS", "zstd"),
                               ("ZLIB_LIBS", "zlib")):
        if from_engine(values.get(variable, "")):
            result.add(external)
    if values.get("ENABLE_SSL") == "true" \
            and from_engine(values.get("OPENSSL_LIBS", "")):
        result.add("openssl")
    return result


def vcxproj_linked_externals(project):
    """The engine externals an MSBuild project links from $(EngineLibDir)."""
    with open(project, encoding="utf-8") as f:
        content = f.read()
    names = set(re.findall(r"\$\(EngineLibDir\)\\([A-Za-z0-9_]+)\.lib", content))
    return {engine_library_external(name) for name in names}


def nuget_references(project):
    """(name, version) of the PackageReferences of an MSBuild project."""
    with open(project, encoding="utf-8") as f:
        content = f.read()
    return re.findall(
        r'<PackageReference\s+Include="([^"]+)"\s+Version="([^"]+)"', content)


def nuget_runtime_packages(deps_json):
    """The NuGet packages a .NET app ships, its transitive ones too, from
    the .deps.json the build writes next to it: (name, version, the DLLs of
    each)."""
    with open(deps_json, encoding="utf-8") as f:
        deps = json.load(f)
    result = []
    for (key, library) in sorted(deps.get("libraries", {}).items()):
        if library.get("type") != "package":
            continue
        (name, version) = key.split("/", 1)
        dlls = set()
        for target in deps.get("targets", {}).values():
            entry = target.get(key, {})
            for section in ("runtime", "native"):
                dlls.update(os.path.basename(path)
                            for path in entry.get(section, {}))
            for runtime in entry.get("runtimeTargets", {}):
                dlls.add(os.path.basename(runtime))
        result.append((name, version, sorted(dlls)))
    return result


def macos_linked_externals(srcdir):
    """The engine externals the macOS app links, from OTHER_LDFLAGS of the
    Xcode project; the library directories are the engine's (configure
    takes POCO, libpng and zstd from its workdir). The system's libraries
    (-liconv, -lz) are named there too."""
    path = os.path.join(srcdir, "macos", "coda", "coda.xcodeproj",
                        "project.pbxproj")
    with open(path, encoding="utf-8") as f:
        content = f.read()
    result = set()
    for block in re.findall(r"OTHER_LDFLAGS = \((.*?)\);", content, re.DOTALL):
        for token in re.findall(r'"([^"]+)"', block):
            if token.startswith("-l"):
                result.add(engine_library_external(token[2:], strict=False))
            elif token.endswith("$(LIBPNG_STATIC_LIB)"):
                result.add("libpng")
            elif token.endswith(".a"):
                name = os.path.splitext(os.path.basename(token))[0]
                result.add(engine_library_external(name))
    return result - {None}


def nuget_component(name, version, provides=(), comment=None):
    """A NuGet package an app ships or compiles in."""
    return {
        "name": name,
        "version": version,
        "vendor": None,
        "url": f"https://www.nuget.org/packages/{name}/{version}",
        "purl": f"pkg:nuget/{name}@{version}",
        "with_cpe": False,
        "declared": "NOASSERTION",
        "concluded": "NOASSERTION",
        "provides": list(provides),
        "comment": comment,
    }


def app_layout(app, srcdir, builddir, engine_builddir):
    """Where an app's own files are and what its binaries link.

    Returns files, the globs of the app's files in the product (None:
    everything outside the engine's directory), dist_prefix, the location of
    browser/dist relative to the product root, binaries, a list of (match, linkinfo) where match is a
    function of the instpath, and components, the prebuilt third-party
    components the app ships with their files' match functions."""
    if app == "qt":
        static = qt_linked_externals(builddir, engine_builddir)
        return {
            # everything outside the engine's directory
            "files": None,
            "dist_prefix": "share/coolwsd/browser/dist/",
            "binaries": [(lambda path: path.endswith("bin/coda-qt"),
                          {"static": static})],
            "components": [],
        }
    if app == "windows":
        project_dir = os.path.join(srcdir, "windows", "coda")
        coda = os.path.join(project_dir, "CODA", "CODA.vcxproj")
        coconvert = os.path.join(project_dir, "COConvert", "COConvert.vcxproj")
        printpdf = os.path.join(project_dir, "PrintPDFAndDelete",
                                "PrintPDFAndDelete.csproj")
        components = []
        coda_components = []
        for (name, version) in nuget_references(coda):
            if name == "Microsoft.Web.WebView2":
                component = nuget_component(
                    name, version, provides=["WebView2Loader.dll"],
                    comment="The WebView2 SDK: its loader DLL ships next to "
                            "the executable, the headers are compiled in.")
                components.append((
                    lambda path: path.rsplit("/", 1)[-1] == "WebView2Loader.dll",
                    component))
            else:
                component = nuget_component(
                    name, version, comment="Header-only, compiled into the "
                                           "executable.")
                components.append((lambda path: False, component))
            coda_components.append(name)
        printpdf_components = []
        for (name, version) in nuget_references(printpdf):
            dll = name + ".dll"
            component = nuget_component(name, version, provides=[dll])
            components.append((
                lambda path, dll=dll: path.rsplit("/", 1)[-1] == dll,
                component))
            printpdf_components.append(name)
        return {
            # CODA.vcxproj's post-build step and the binaries it builds
            "files": ["cool/*", "templates/*", "program/translations/*",
                      "program/*.exe", "program/WebView2Loader.dll",
                      "program/runtimes/*",
                      "PrintPDFAndDelete*", "program/PrintPDFAndDelete*",
                      "*.runtimeconfig.json", "*.deps.json"]
                     + [f"*{name}.dll" for (name, _) in
                        nuget_references(printpdf)],
            "dist_prefix": "cool/",
            "binaries": [
                # the app's executable, named after the product
                (lambda path: path.startswith("program/")
                    and path.count("/") == 1 and path.endswith(".exe")
                    and path not in ("program/COConvert.exe",
                                     "program/PrintPDFAndDelete.exe"),
                 {"static": vcxproj_linked_externals(coda),
                  "components": coda_components}),
                (lambda path: path.endswith("COConvert.exe"),
                 {"static": vcxproj_linked_externals(coconvert)}),
                (lambda path: path.rsplit("/", 1)[-1] == "PrintPDFAndDelete.exe"
                    or path.rsplit("/", 1)[-1] == "PrintPDFAndDelete.dll",
                 {"static": set(), "components": printpdf_components}),
            ],
            "components": components,
        }
    if app == "macos":
        dist = os.path.join(builddir, "browser", "dist")
        return {
            # the Xcode project's own: the app binary, Info.plist, the
            # signature, browser/dist and the app's resources at the top of
            # Resources; the engine's are in directories of its own there
            "files": ["Contents/MacOS/*", "Contents/Info.plist",
                      "Contents/PkgInfo", "Contents/_CodeSignature/*",
                      "Contents/embedded.provisionprofile",
                      "Contents/Resources/*.lproj/*",
                      "Contents/Resources/*.car", "Contents/Resources/*.icns",
                      "Contents/Resources/coolwsd.xml",
                      "Contents/Resources/discovery.xml",
                      "Contents/Resources/hello.odt",
                      "Contents/Resources/TaskWorker.js",
                      "Contents/Resources/branding*"]
                     + [f"Contents/Resources/{name}" for name in os.listdir(dist)]
                     + [f"Contents/Resources/{name}/*" for name in os.listdir(dist)
                        if os.path.isdir(os.path.join(dist, name))],
            "dist_prefix": "Contents/Resources/",
            "binaries": [(lambda path: path.startswith("Contents/MacOS/"),
                          {"static": macos_linked_externals(srcdir)})],
            "components": [],
        }
    raise Exception(f"unknown app: {app}")


def app_document(app, srcdir, builddir, engine_builddir, files, version,
                 product_names=(), branding_theme=None):
    """The SPDX document of the online part of a desktop app.

    files are the app's own files in the product, [{"instpath", "abspath"}],
    with instpaths relative to the product root; the engine's files are
    described by the engine's own documents and are not among them."""
    annotations = read_annotations(srcdir)
    custom_licenses.update(annotations.get("custom_licenses", {}))
    prefixes = {entry["tarball_var"]
                for entry in annotations["engine_externals"].values()}
    tarballs, engine_srcdir = read_engine_tarballs(engine_builddir, prefixes)
    poco_bundled = poco_bundled_names(engine_srcdir)
    seeds, npm_runtime = read_npm_runtime(
        os.path.join(srcdir, "browser"), os.path.join(builddir, "browser"),
        os.path.join(builddir, "browser", "sbom-shipped-js.txt"))
    if branding_theme is None:
        branding_theme = parse_make_variables(
            os.path.join(builddir, "Makefile"),
            ["BRANDING_THEME"]).get("BRANDING_THEME", "")

    layout = app_layout(app, srcdir, builddir, engine_builddir)
    # under the install prefix, which is usr/ in the snap
    dist_prefix = layout["dist_prefix"]
    for file in files:
        index = file["instpath"].find("/" + dist_prefix)
        if index >= 0 and not file["instpath"].startswith(dist_prefix):
            dist_prefix = file["instpath"][:index + 1] + dist_prefix
            break
    files = [dict(file) for file in files]
    if app == "windows":
        # the NuGet packages a .NET app ships, with the versions the build
        # resolved, in place of the PackageReferences they come from
        for file in files:
            if not file["instpath"].endswith(".deps.json"):
                continue
            for (name, version, dlls) in nuget_runtime_packages(
                    file["abspath"]):
                layout["components"] = [
                    entry for entry in layout["components"]
                    if entry[1]["name"] != name]
                layout["components"].append((
                    lambda path, dlls=frozenset(dlls):
                        path.rsplit("/", 1)[-1] in dlls,
                    nuget_component(name, version, provides=dlls)))
    for (match, component) in layout["components"]:
        for file in files:
            if match(file["instpath"]):
                file["owner"] = ("component", component["name"])

    executables = {}
    for file in files:
        for (match, linkinfo) in layout["binaries"]:
            if match(file["instpath"]):
                needed = binary_needed(file["abspath"], engine_srcdir)
                if needed is None:
                    break  # a script or data file next to the binaries
                # what the link line names minus what is loaded dynamically
                dynamic = {soname_external(name) for name in needed} - {None}
                executables[file["instpath"]] = {
                    "static": set(linkinfo["static"]) - dynamic,
                    "needed": needed,
                    "components": linkinfo.get("components", ()),
                }
                break
    if not executables:
        raise Exception(f"no binary of the {app} app among its files")

    # a component is in the product if it owns a file of it, or is linked
    # into a binary of it
    present = {file["owner"][1] for file in files if "owner" in file}
    for linked in executables.values():
        present.update(linked["components"])
    extra_components = [component for (_, component) in layout["components"]
                        if component["name"] in present]

    target_os = {"qt": "LINUX", "windows": "WNT", "macos": "MACOSX"}[app]
    return build_document(
        srcdir, builddir, version, branding_theme, files, executables,
        annotations, tarballs, poco_bundled, npm_runtime, engine_builddir,
        root_name="CollaboraOffice-app", dist_prefix=dist_prefix,
        target_os=target_os, extra_components=extra_components,
        only_linked_externals=True, product_names=product_names)



def build_document(srcdir, builddir, version, branding_theme, files,
                   executables, annotations, tarballs, poco_bundled,
                   npm_runtime, engine_builddir, root_name="CollaboraOnline-coolwsd",
                   dist_prefix=SERVER_DIST_PREFIX, target_os="LINUX",
                   extra_components=(), only_linked_externals=False,
                   product_names=()):
    """The SPDX document of the online part of a product.

    files are the installed files, [{"instpath", "abspath"}], optionally with
    an "owner" ("component", name) for a file of an extra component.
    executables maps the instpath of each binary we build to "static", the
    engine externals it links statically, "needed", the libraries it loads,
    and optionally "components", the extra components linked into it.
    extra_components are prebuilt third-party components the product ships;
    "provides" lists the library names of theirs that binaries load.
    product_names are the file names of the whole product, also those other
    documents describe: a binary that loads one of them loads no system
    library.
    """
    document, root_spdx_id = sbom_skeleton(version, "MPL-2.0", name=root_name)
    package_version = version
    graph = document["@graph"]
    graph.extend(license_cache["MPL-2.0"])
    emitted_licenses = {"MPL-2.0"}

    def emit_licenses(*license_exprs):
        for expr in license_exprs:
            if expr not in emitted_licenses:
                graph.extend(license_cache[expr])
                emitted_licenses.add(expr)

    # Engine-built statically linked externals
    external_ids = {}
    # The server lists every annotated external; an app only those its
    # binaries link
    linked_externals = set().union(
        *[linked["static"] for linked in executables.values()])
    for name, entry in sorted(annotations["engine_externals"].items()):
        if only_linked_externals and name not in linked_externals:
            continue
        tarball, sha256 = tarballs[entry["tarball_var"]]
        version = extract_version_from_filename(tarball)
        # the build fetches every external from our own mirror, so that is
        # the download location; an entry can still override it with its
        # own source_url
        locator = entry.get(
            "source_url", "https://gerrit.collaboraoffice.com/src/{tarball}"
        ).format(tarball=tarball, version=version)
        external_ids[name] = make_component(
            graph, f"SPDXRef-{name}", name, entry["vendor"],
            version, entry["url"], locator, sha256,
            entry["declared"], entry["concluded"],
            cpe_name=entry.get("cpe_name"))
        emit_licenses(entry["declared"], entry["concluded"])
        add_contains(graph, root_spdx_id, external_ids[name])

    # Sources bundled inside POCO
    poco_unpacked = os.path.join(engine_builddir, "workdir",
                                 "UnpackedTarball", "poco")
    for entry in annotations["bundled"]:
        name = entry["name"]
        if name not in poco_bundled:
            continue
        if "platforms" in entry and target_os not in entry["platforms"]:
            continue
        display = entry.get("display_name", name)
        bundled_id = make_component(
            graph, f"SPDXRef-{display}", display, entry["vendor"],
            probe_poco_bundled_version(name, poco_unpacked), entry["url"],
            None, None, entry["declared"], entry["concluded"])
        emit_licenses(entry["declared"], entry["concluded"])
        add_contains(graph, external_ids[entry["parent"]], bundled_id)

    # Vendored / checked-in third-party sources
    vendored_ids = {}
    vendored_compiled = []
    vendored_by_dist_path = {}
    for entry in annotations["vendored"]:
        name = entry["name"]
        vendored_ids[name] = make_component(
            graph, f"SPDXRef-{name}", name, entry["vendor"],
            entry.get("version"), entry["url"], repo_url(entry["source"]),
            None, entry["declared"], entry["concluded"])
        emit_licenses(entry["declared"], entry["concluded"])
        add_contains(graph, root_spdx_id, vendored_ids[name])
        if entry.get("compiled"):
            vendored_compiled.append(name)
        basename = os.path.basename(entry["source"])
        if entry["source"].startswith("browser/js/"):
            vendored_by_dist_path[basename] = name
        elif entry["source"].startswith("browser/admin/"):
            subdir = entry["source"].split("/")[2]
            vendored_by_dist_path[f"admin/{subdir}/{basename}"] = name

    # Our own components under a license other than the tree's. The theme is
    # in dist only when the build applied one; a theme that --with-app-branding
    # copied in is not ours to declare and stays with the root.
    firstparty_ids = {}
    firstparty_by_dist_path = {}
    for entry in annotations["firstparty"]:
        if entry["source"] != "browser/branding" or not branding_theme:
            continue
        name = entry["name"]
        firstparty_ids[name] = make_component(
            graph, f"SPDXRef-{name}", name, None, package_version,
            entry["url"], repo_url(entry["source"]), None, entry["declared"],
            entry["concluded"], with_cpe=False)
        emit_licenses(entry["declared"], entry["concluded"])
        add_contains(graph, root_spdx_id, firstparty_ids[name])
        for rel in branding_dist_files(srcdir, builddir):
            firstparty_by_dist_path[rel] = name
        log(f"theme {branding_theme}: {len(firstparty_by_dist_path)} "
            f"files of {name}")

    # Prebuilt third-party components a product ships, which no build of
    # ours produces: their files are assigned by the caller
    extra_ids = {}
    bundled_names = set()
    for component in extra_components:
        name = component["name"]
        extra_ids[name] = make_component(
            graph, f"SPDXRef-{name}", name, component.get("vendor"),
            component.get("version"), component.get("url"), None, None,
            component.get("declared", "NOASSERTION"),
            component.get("concluded", "NOASSERTION"),
            purl=component.get("purl"), cpe_name=component.get("cpe_name"),
            with_cpe=component.get("with_cpe", True),
            comment=component.get("comment"))
        emit_licenses(component.get("declared", "NOASSERTION"),
                      component.get("concluded", "NOASSERTION"))
        add_contains(graph, root_spdx_id, extra_ids[name])
        bundled_names.update(component.get("provides", ()))

    # Runtime npm packages
    npm_ids = {}
    for name, info in sorted(npm_runtime.items()):
        if info["resolved"].startswith("file:"):
            locator = repo_url("browser/" + info["resolved"][len("file:"):])
        else:
            basename = name.split("/")[-1]
            locator = (f"https://registry.npmjs.org/{name}/-/"
                       f"{basename}-{info['version']}.tgz")
        license = info["license"] or "NOASSERTION"
        npm_ids[name] = make_component(
            graph, f"SPDXRef-npm-{name}", name, None, info["version"],
            f"https://www.npmjs.com/package/{name}", locator, None,
            license, license, sha512=info["sha512"],
            purl=npm_purl(name, info["version"]))
        emit_licenses(license)
        add_contains(graph, root_spdx_id, npm_ids[name])
    # inter-package dependencies
    for name, info in sorted(npm_runtime.items()):
        deps = [npm_ids[dep] for dep in info["dependencies"]]
        if deps:
            graph.append({
                "type": "Relationship",
                "spdxId": next_rel_id(),
                "creationInfo": "_:creationinfo",
                "from": npm_ids[name],
                "relationshipType": "dependsOn",
                "to": deps,
                "completeness": "complete",
            })

    # Installed files
    seen_instpaths = set()
    for file in files:
        if file["instpath"] in seen_instpaths:
            raise Exception(f"duplicate installed file: {file['instpath']}")
        seen_instpaths.add(file["instpath"])

    npm_names = set(npm_runtime.keys())
    sysdep_ids = {}
    for file in files:
        instpath = file["instpath"]
        owner = file.get("owner") or file_external(
            instpath, npm_names, vendored_by_dist_path,
            firstparty_by_dist_path, dist_prefix)
        if owner is None:
            parent = root_spdx_id
        elif owner[0] == "npm":
            parent = npm_ids[owner[1]]
        elif owner[0] == "firstparty":
            parent = firstparty_ids[owner[1]]
        elif owner[0] == "component":
            parent = extra_ids[owner[1]]
        else:
            parent = vendored_ids[owner[1]]

        file_spdx_id = emit_file(graph, parent, instpath, file["abspath"])

        if instpath in executables:
            linked = executables[instpath]
            static = sorted(linked["static"])
            static_ids = [external_ids[external] for external in static]
            if "poco" in linked["static"]:
                static_ids.extend(vendored_ids[name]
                                  for name in vendored_compiled)
            static_ids.extend(extra_ids[name]
                              for name in linked.get("components", ()))
            if static_ids:
                graph.append({
                    "type": "Relationship",
                    "spdxId": next_rel_id(),
                    "creationInfo": "_:creationinfo",
                    "from": file_spdx_id,
                    "relationshipType": "hasStaticLink",
                    "to": static_ids,
                    "completeness": "noAssertion",
                })
            sysdeps = []
            for soname in linked["needed"]:
                if soname in product_names or soname in bundled_names:
                    continue  # loads a file of the product
                if soname not in sysdep_ids:
                    sysdep_ids[soname] = make_spdx_id(f"Sysdep-{soname}")
                    graph.append({
                        "type": "software_Package",
                        "spdxId": sysdep_ids[soname],
                        "creationInfo": "_:creationinfo",
                        "name": soname,
                        "comment": "Supplied by the operating system.",
                    })
                sysdeps.append(sysdep_ids[soname])
            if sysdeps:
                graph.append({
                    "type": "Relationship",
                    "spdxId": next_rel_id(),
                    "creationInfo": "_:creationinfo",
                    "from": file_spdx_id,
                    "relationshipType": "dependsOn",
                    "to": sysdeps,
                    "completeness": "noAssertion",
                })

    log(f"{len(graph)} elements, {len(files)} files, "
        f"{len(npm_runtime)} npm packages")
    return document


def main():
    parser = argparse.ArgumentParser(
        description="Generate the Collabora Online SPDX 3.0.1 SBOM")
    parser.add_argument("--srcdir", default=os.path.dirname(
        os.path.dirname(os.path.abspath(__file__))))
    parser.add_argument("--builddir",
                        help="required unless --brand-package is given")
    parser.add_argument("--engine-builddir",
                        help="required unless --brand-package is given")
    parser.add_argument("--brand-package",
                        help="describe this brand package (code-brand or "
                             "collabora-online-brand) staged under "
                             "--package-root instead of the build")
    parser.add_argument("--package-root",
                        help="the staged tree of --brand-package")
    parser.add_argument("--version",
                        help="package version (default: from config.h)")
    parser.add_argument("--shipped-js",
                        help="manifest written by browser/Makefile.am "
                             "(default: BUILDDIR/browser/sbom-shipped-js.txt)")
    parser.add_argument("--mobileapp", action="store_true",
                        help="dist excludes of an ENABLE_MOBILEAPP build")
    parser.add_argument("--branding-theme", default="",
                        help="the theme from browser/branding that the build "
                             "applied to dist (configure's BRANDING_THEME), "
                             "empty for an unbranded build")
    parser.add_argument("--output",
                        help="default: BUILDDIR/collabora-online-sbom.spdx.json")
    parser.add_argument("--check", action="store_true",
                        help="only run the consistency checks, write nothing")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    global verbose
    verbose = args.verbose

    if args.brand_package:
        write_brand_package_sbom(args)
        return
    if not args.builddir or not args.engine_builddir:
        parser.error("--builddir and --engine-builddir are required")

    version = args.version
    if version is None:
        with open(os.path.join(args.builddir, "config_version.h"),
                  encoding="utf-8") as f:
            match = re.search(r'#define COOLWSD_VERSION "([^"]+)"', f.read())
        if not match:
            raise Exception(
                "no --version and no COOLWSD_VERSION in config_version.h")
        version = match.group(1)

    annotations = read_annotations(args.srcdir)
    custom_licenses.update(annotations.get("custom_licenses", {}))

    prefixes = {entry["tarball_var"]
                for entry in annotations["engine_externals"].values()}
    tarballs, engine_srcdir = read_engine_tarballs(args.engine_builddir, prefixes)

    gbuild_externals = parse_gbuild_executables(args.srcdir)
    poco_bundled = {name for name in poco_bundled_names(engine_srcdir)
                    if name != "wepoll"}  # wepoll is only compiled on Windows

    shipped_js = args.shipped_js or os.path.join(
        args.builddir, "browser", "sbom-shipped-js.txt")
    seeds, npm_runtime = read_npm_runtime(
        os.path.join(args.srcdir, "browser"),
        os.path.join(args.builddir, "browser"), shipped_js)

    if args.check:
        errors = run_checks(args, annotations, gbuild_externals, poco_bundled,
                            seeds, npm_runtime)
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        sys.exit(1 if errors else 0)

    # Installed files
    files = collect_files(args.srcdir, args.builddir, args.mobileapp)
    executables = {}
    for exe in INSTALLED_EXECUTABLES:
        needed = elf_needed(os.path.join(args.builddir, exe))
        # Statically linked externals: what gbuild links minus what this
        # build actually loads dynamically (verified via DT_NEEDED).
        dynamic = {soname_external(soname) for soname in needed} - {None}
        executables[f"usr/bin/{exe}"] = {
            "static": gbuild_externals[exe] - dynamic,
            "needed": needed,
        }

    document = build_document(
        args.srcdir, args.builddir, version, args.branding_theme, files,
        executables, annotations, tarballs, poco_bundled, npm_runtime,
        engine_builddir=args.engine_builddir)

    output = args.output or os.path.join(args.builddir,
                                         "collabora-online-sbom.spdx.json")
    with open(output, "w", encoding="utf-8") as f:
        json.dump(document, f, indent=2)
        f.write("\n")
    log(f"wrote {output}")


if __name__ == "__main__":
    main()
