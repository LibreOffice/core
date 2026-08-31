#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Online contributors.
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
    # poco-1.15.3-all: the "-all" flavor suffix is not part of the version
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


def read_engine_tarballs(engine_builddir, prefixes):
    """Find version and sha256 of engine-built externals via download.lst."""
    config = parse_make_variables(
        os.path.join(engine_builddir, "config_host.mk"), ["SRCDIR"])
    if "SRCDIR" not in config:
        raise Exception(f"no SRCDIR in {engine_builddir}/config_host.mk")
    engine_srcdir = config["SRCDIR"]
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
    with open(os.path.join(browser_srcdir, "npm-shrinkwrap.json"),
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
    add("etc/coolwsd/coolkitconfig.xcu", os.path.join(builddir, "coolkitconfig.xcu"))

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


def file_external(instpath, npm_names, vendored_by_dist_path):
    """Which third-party component an installed file belongs to, if any."""
    dist_prefix = "usr/share/coolwsd/browser/dist/"
    if instpath.startswith(dist_prefix):
        rel = instpath[len(dist_prefix):]
        if rel.startswith("node_modules/"):
            parts = rel.split("/")
            name = "/".join(parts[1:3]) if parts[1].startswith("@") else parts[1]
            if name in npm_names:
                return ("npm", name)
        if rel in vendored_by_dist_path:
            return ("vendored", vendored_by_dist_path[rel])
    return None


# ---------------------------------------------------------------------------
# Document assembly
# ---------------------------------------------------------------------------

def sbom_skeleton(version, root_license):
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
                "externalIdentifers": [{
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
                "name": "CollaboraOnline-coolwsd",
                "rootElement": [root_spdx_id],
                "profileConformance": ["core", "software", "simpleLicensing"],
            },
            {
                "type": "software_Package",
                "spdxId": root_spdx_id,
                "creationInfo": "_:creationinfo",
                "name": "CollaboraOnline-coolwsd",
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


def make_component(graph, fragment, name, vendor, version, homepage, locator,
                   sha256, declared, concluded, sha512=None, purl=None):
    """Emit a software_Package + source artifact, engine-shape."""
    pkg_spdx_id = make_spdx_id(fragment)
    pkg_element = {
        "type": "software_Package",
        "spdxId": pkg_spdx_id,
        "originatedBy": ["https://collaboraoffice.com"],
        "creationInfo": "_:creationinfo",
        "name": name,
        "externalIdentifiers": [{
            "externalIdentifierType": "cpe23",
            "identifier": cpe23(vendor, name, version),
        }],
    }
    if purl:
        pkg_element["externalIdentifiers"].append({
            "externalIdentifierType": "packageURL",
            "identifier": purl,
        })
    if homepage:
        pkg_element["software_homePage"] = homepage
    if version:
        pkg_element["software_packageVersion"] = version
    graph.append(pkg_element)

    if locator or sha256 or sha512:
        source_spdx_id = make_spdx_id(f"{fragment}-source")
        source_element = {
            "type": "software_SoftwareArtifact",
            "spdxId": source_spdx_id,
            "software_primaryPurpose": "source",
        }
        if locator:
            source_element["externalRef"] = [{
                "type": "ExternalRef",
                "externalRefType": "SourceArtifact",
                "locator": locator,
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
            "from": source_spdx_id,
            "relationshipType": "generates",
            "to": [pkg_spdx_id],
            "completeness": "complete",
        })

    add_license_relationship(graph, pkg_spdx_id, "hasDeclaredLicense", declared)
    add_license_relationship(graph, pkg_spdx_id, "hasConcludedLicense", concluded)
    return pkg_spdx_id


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

    for entry in annotations["vendored"]:
        source = os.path.join(args.srcdir, entry["source"])
        if not os.path.exists(source):
            errors.append(f"vendored source of {entry['name']} is gone: "
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


def main():
    parser = argparse.ArgumentParser(
        description="Generate the Collabora Online SPDX 3.0.1 SBOM")
    parser.add_argument("--srcdir", default=os.path.dirname(
        os.path.dirname(os.path.abspath(__file__))))
    parser.add_argument("--builddir", required=True)
    parser.add_argument("--engine-builddir", required=True)
    parser.add_argument("--version",
                        help="package version (default: from config.h)")
    parser.add_argument("--shipped-js",
                        help="manifest written by browser/Makefile.am "
                             "(default: BUILDDIR/browser/sbom-shipped-js.txt)")
    parser.add_argument("--mobileapp", action="store_true",
                        help="dist excludes of an ENABLE_MOBILEAPP build")
    parser.add_argument("--output",
                        help="default: BUILDDIR/collabora-online-sbom.spdx.json")
    parser.add_argument("--check", action="store_true",
                        help="only run the consistency checks, write nothing")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    global verbose
    verbose = args.verbose

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

    document, root_spdx_id = sbom_skeleton(version, "MPL-2.0")
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
    for name, entry in sorted(annotations["engine_externals"].items()):
        tarball, sha256 = tarballs[entry["tarball_var"]]
        version = extract_version_from_filename(tarball)
        # most tarballs come from the LibreOffice mirror; the exceptions
        # (poco, see engine/Makefile.fetch) carry their own source_url
        locator = entry.get(
            "source_url", "https://dev-www.libreoffice.org/src/{tarball}"
        ).format(tarball=tarball, version=version)
        external_ids[name] = make_component(
            graph, f"SPDXRef-{name}", name, entry["vendor"],
            version, entry["url"], locator, sha256,
            entry["declared"], entry["concluded"])
        emit_licenses(entry["declared"], entry["concluded"])
        add_contains(graph, root_spdx_id, external_ids[name])

    # Sources bundled inside POCO
    poco_unpacked = os.path.join(args.engine_builddir, "workdir",
                                 "UnpackedTarball", "poco")
    for entry in annotations["bundled"]:
        name = entry["name"]
        if name not in poco_bundled:
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
                "from": npm_ids[name],
                "relationshipType": "dependsOn",
                "to": deps,
                "completeness": "complete",
            })

    # Installed files
    files = collect_files(args.srcdir, args.builddir, args.mobileapp)
    seen_instpaths = set()
    for file in files:
        if file["instpath"] in seen_instpaths:
            raise Exception(f"duplicate installed file: {file['instpath']}")
        seen_instpaths.add(file["instpath"])

    binaries_needed = {}
    for exe in INSTALLED_EXECUTABLES:
        binaries_needed[exe] = elf_needed(os.path.join(args.builddir, exe))

    npm_names = set(npm_runtime.keys())
    sysdep_ids = {}
    for file in files:
        instpath = file["instpath"]
        file_spdx_id = make_spdx_id(f"File-{instpath}")
        owner = file_external(instpath, npm_names, vendored_by_dist_path)
        if owner is None:
            parent = root_spdx_id
        elif owner[0] == "npm":
            parent = npm_ids[owner[1]]
        else:
            parent = vendored_ids[owner[1]]

        flags = ["executable"] if is_executable_path(instpath) else []
        hash_element = {
            "type": "Hash",
            "algorithm": "sha512",
            "hashValue": get_sha512(file["abspath"]),
        }
        if flags:
            hash_element["software_additionalPurpose"] = flags
            hash_element["comment"] = ("software_additionalPurpose field is "
                "used to indicate the properties of BSI TR-03183-2")
        graph.append({
            "type": "software_File",
            "spdxId": file_spdx_id,
            "name": instpath,
            "verifiedUsing": [hash_element],
        })
        graph.append({
            "type": "Relationship",
            "from": parent,
            "relationshipType": "hasDistributionArtifact",
            "to": [file_spdx_id],
            "completeness": "incomplete",
        })

        exe = instpath[len("usr/bin/"):] if instpath.startswith("usr/bin/") else None
        if exe in gbuild_externals:
            # Statically linked externals: what gbuild links minus what this
            # build actually loads dynamically (verified via DT_NEEDED).
            dynamic = {soname_external(soname)
                       for soname in binaries_needed[exe]} - {None}
            static = sorted(gbuild_externals[exe] - dynamic)
            static_ids = [external_ids[external] for external in static]
            if "poco" in gbuild_externals[exe]:
                static_ids.extend(vendored_ids[name]
                                  for name in vendored_compiled)
            if static_ids:
                graph.append({
                    "type": "Relationship",
                    "from": file_spdx_id,
                    "relationshipType": "contains",
                    "software_softwareLinkage": "static",
                    "to": static_ids,
                    "completeness": "noAssertion",
                })
            sysdeps = []
            for soname in binaries_needed[exe]:
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
                    "from": file_spdx_id,
                    "relationshipType": "dependsOn",
                    "to": sysdeps,
                    "completeness": "noAssertion",
                })

    output = args.output or os.path.join(args.builddir,
                                         "collabora-online-sbom.spdx.json")
    with open(output, "w", encoding="utf-8") as f:
        json.dump(document, f, indent=2)
        f.write("\n")
    log(f"wrote {output}: {len(graph)} elements, {len(files)} files, "
        f"{len(npm_runtime)} npm packages")


if __name__ == "__main__":
    main()
