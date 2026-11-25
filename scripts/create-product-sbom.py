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
# The SBOM of a Collabora Office desktop product (Windows, macOS, flatpak,
# snap, FreeBSD): one CycloneDX 1.6 document, the one to scan, and the same
# content as one SPDX 3.0.1 document, both describing exactly the files of
# the product as it ships.
#
# Three kinds of components:
# - built: what we build from source and ship, described by the engine's
#   per-package SPDX documents and the app's document from create-sbom.py
# - bundled-binary: prebuilt third-party binaries the product ships (the
#   snap's Ubuntu packages, the flatpak's QtWebEngine, NuGet packages)
# - platform: what the product needs at run time but does not ship (the
#   flatpak runtime, the snap's base and content snaps, FreeBSD packages,
#   the operating system), with the version found where it was built
#
# It runs in two steps, because some packaging tools change files after the
# build: flatpak-builder strips the binaries after a module is installed,
# the Mac app is signed when it is exported.
#
#   stage     runs where the build trees are: reads the engine's documents,
#             the app's build and the platform, and writes what it found
#             to a staging directory
#   finalize  runs on the product as it ships: hashes every file, fails on a
#             file nothing describes, writes the two documents
#   all       both, for products whose files no longer change after the
#             build (Windows, the snap, the FreeBSD port)
#   snapshot  lists the files present before the product is installed, for
#             a tree that already contains other things (flatpak's /app)

import argparse
import fnmatch
import glob
import hashlib
import importlib.util
import json
import os
import re
import shutil
import subprocess
import sys
import uuid
import urllib.parse
from datetime import datetime, timezone

# stage runs in the build, also as root in a FreeBSD port: no __pycache__
# in the source tree from the scripts it imports
sys.dont_write_bytecode = True

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

PRODUCT_SUPPLIER = "Collabora Productivity Ltd."
PRODUCT_URL = "https://www.collaboraoffice.com/"
SBOM_BASENAME = "collabora-office"
# Where the VEX is published: the SBOM points at it rather than carrying
# it, so the assessment can be updated after the release
DEFAULT_VEX_URL = ("https://raw.githubusercontent.com/CollaboraOnline/"
                   "online.mirror/main/docker/from-packages/vex/cool.vex.json")

timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")

verbose = False

def log(message):
    if verbose:
        print(message, file=sys.stderr)


def load_module(name, *candidates):
    """Import one of our hyphenated scripts by path."""
    for path in candidates:
        if os.path.isfile(path):
            spec = importlib.util.spec_from_file_location(name, path)
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            return module
    raise Exception(f"cannot find {name}: {', '.join(candidates)}")


def image_sbom_module():
    """generate-image-sbom.py, for reading SPDX identifiers the way the
    container image SBOM does. Next to this script in a staging directory,
    under docker/ in the source tree."""
    return load_module(
        "generate_image_sbom",
        os.path.join(SCRIPT_DIR, "generate-image-sbom.py"),
        os.path.join(SCRIPT_DIR, os.pardir, "docker", "from-packages",
                     "scripts", "generate-image-sbom.py"))


def sha512_file(path):
    digest = hashlib.sha512()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def run(command):
    """Output of a command, None if it cannot be run or fails."""
    try:
        return subprocess.run(command, check=True, capture_output=True,
                              text=True).stdout
    except (OSError, subprocess.CalledProcessError):
        return None


def parse_make_variables(path, names=None):
    """NAME=value assignments of a configured makefile."""
    values = {}
    pattern = re.compile(r"^(?:export\s+)?([A-Za-z0-9_]+)\s*:?=\s*(.*?)\s*$")
    with open(path, encoding="utf-8") as f:
        for line in f:
            match = pattern.match(line)
            if match and (names is None or match.group(1) in names):
                values[match.group(1)] = match.group(2)
    return values


# ---------------------------------------------------------------------------
# The product's files
# ---------------------------------------------------------------------------

def matches(path, patterns):
    return any(fnmatch.fnmatch(path, pattern) for pattern in patterns)


def walk_tree(tree, excludes=(), skip=()):
    """Every regular file of the tree, relative to it. Symlinks are not
    followed: the file they point to is listed where it is."""
    result = []
    skip = [os.path.realpath(path) for path in skip]
    for root, dirs, names in os.walk(tree):
        dirs.sort()
        dirs[:] = [d for d in dirs
                   if os.path.realpath(os.path.join(root, d)) not in skip]
        for name in sorted(names):
            abspath = os.path.join(root, name)
            if os.path.islink(abspath) or not os.path.isfile(abspath):
                continue
            path = os.path.relpath(abspath, tree).replace(os.sep, "/")
            if not matches(path, excludes):
                result.append(path)
    return result


def tree_path(tree, path):
    """Where a file the documents name really is in the tree: packaging
    moves some files and leaves a symlink (flatpak's per-language files)."""
    abspath = os.path.join(tree, path)
    real = os.path.realpath(abspath)
    tree_real = os.path.realpath(tree)
    if os.path.commonpath([real, tree_real]) == tree_real:
        return os.path.relpath(real, tree_real).replace(os.sep, "/")
    return path


# ---------------------------------------------------------------------------
# The engine's documents
# ---------------------------------------------------------------------------

def engine_documents(engine_builddir):
    pattern = os.path.join(engine_builddir, "workdir", "CustomTarget",
                           "instsetoo_native", "sbom", "*-sbom.spdx.json")
    paths = sorted(glob.glob(pattern))
    if not paths:
        raise Exception(f"no engine SBOM documents ({pattern}): configure "
                        f"the engine with --enable-sbom")
    return paths


def map_engine_path(name, engine_root):
    """The path in the product of a file an engine document names, which is
    relative to the engine's instdir."""
    (old, new) = engine_root
    if not name.startswith(old):
        raise Exception(f"engine file {name} is not under {old}")
    return new + name[len(old):]


def drop_elements(graph, dropped):
    """Remove elements and every reference to them from relationships."""
    result = []
    for element in graph:
        if element.get("spdxId") in dropped:
            continue
        if element.get("type") == "Relationship":
            if element.get("from") in dropped:
                continue
            to = [target for target in element.get("to", [])
                  if target not in dropped]
            if not to:
                continue
            element["to"] = to
        result.append(element)
    return result


def stage_engine(engine_builddir, engine_root, tree, product_files, staging):
    """Copy the engine's documents with the files renamed to where they are
    in the product. A file the product does not ship (packaging deletes
    soffice.exe on Windows, leaves the Spotlight plugin out on macOS) is
    dropped.

    The documents list the executables and archives only; installed-files
    .json, which the engine writes next to them, lists every file it
    installs. Returns the package of each of them that the product ships."""
    with open(os.path.join(os.path.dirname(engine_documents(engine_builddir)[0]),
                           "installed-files.json"), encoding="utf-8") as f:
        installed = json.load(f)
    claimed = {}
    for name, package in installed.items():
        location = map_engine_path(name, engine_root)
        if os.path.isfile(os.path.join(tree, location)):
            location = tree_path(tree, location)
            if location in product_files:
                claimed[location] = f"{package}-sbom.spdx.json"

    missing = 0
    os.makedirs(os.path.join(staging, "engine"), exist_ok=True)
    # the documents refer to each other's files, so what is dropped from
    # one is dropped from the references of all
    documents = []
    dropped = set()
    for path in engine_documents(engine_builddir):
        with open(path, encoding="utf-8") as f:
            document = json.load(f)
        documents.append((path, document))
        for element in document["@graph"]:
            if element.get("type") != "software_File":
                continue
            location = map_engine_path(element["name"], engine_root)
            if not os.path.isfile(os.path.join(tree, location)):
                dropped.add(element["spdxId"])
                missing += 1
                continue
            location = tree_path(tree, location)
            if location not in product_files:
                raise Exception(f"engine file {location} is not part of the "
                                f"product (excluded or present before it?)")
            element["name"] = location
    for (path, document) in documents:
        document["@graph"] = drop_elements(document["@graph"], dropped)
        with open(os.path.join(staging, "engine", os.path.basename(path)),
                  "w", encoding="utf-8") as f:
            json.dump(document, f, indent=1)
    log(f"engine: {len(claimed)} files, {missing} listed ones not shipped")
    return claimed, missing


# ---------------------------------------------------------------------------
# Platforms
# ---------------------------------------------------------------------------

def license_annotations(engine_srcdir):
    """div class -> (Name, Vendor, URL) of the engine's externals, from the
    annotations in license.xml: what NVD calls them."""
    path = os.path.join(engine_srcdir, "readlicense_oo", "license",
                        "license.xml")
    result = {}
    current = None
    with open(path, encoding="utf-8") as f:
        for line in f:
            match = re.search(r'<div class="([A-Z0-9_]+)"', line)
            if match:
                current = match.group(1)
            match = re.search(r"<!--\s*Name:(.*?)-->", line)
            if match and current:
                fields = {}
                for part in ("Name:" + match.group(1)).split(","):
                    if ":" in part:
                        key, value = part.split(":", 1)
                        fields[key.strip()] = value.strip()
                result.setdefault(current, fields)
    return result


# SYSTEM_X that are no library the product loads: tools and headers of the
# build, the tests' cppunit, data
NOT_LOADED_SYSTEM_EXTERNALS = {
    "CPPUNIT", "DICTS", "GENBRK", "GENCCODE", "GENCMN", "LIBEXTTEXTCAT_DATA",
    "LIBFIXMATH", "LIBNUMBERTEXT_DATA", "LIBXML_FOR_BUILD",
    "LIBXSLT_FOR_BUILD", "ODBC_HEADERS",
}

# the license.xml section of SYSTEM_X, where its name differs
SYSTEM_LICENSE_CLASSES = {
    "HYPH": "HYPHEN",
    "LIBJPEG": "LIBJPEG_TURBO",
    "LIBORCUS": "ORCUS",
    "LIBXML": "LIBXML2",
}

# Name and vendor of the externals the engine has no license.xml section for,
# because it never bundles them, as NVD names them
UNANNOTATED_SYSTEM_EXTERNALS = {
    "ABSEIL": {"Name": "abseil-cpp", "Vendor": "abseil"},
    "OPENJPEG2": {"Name": "openjpeg", "Vendor": "uclouvain"},
}

def system_externals(engine_builddir):
    """The engine's externals this build took from the system: SYSTEM_X."""
    values = parse_make_variables(os.path.join(engine_builddir,
                                               "config_host.mk"))
    return sorted(name[len("SYSTEM_"):] for name, value in values.items()
                  if name.startswith("SYSTEM_") and value == "TRUE"
                  and name[len("SYSTEM_"):] not in NOT_LOADED_SYSTEM_EXTERNALS)


# pkg-config modules of the externals whose name does not say it
PKGCONFIG_NAMES = {
    "ABSEIL": "absl_base",
    "CURL": "libcurl",
    "GRAPHITE": "graphite2",
    "HYPH": "hyphen",
    "ICU": "icu-uc",
    "LIBJPEG": "libjpeg",
    "LIBTIFF": "libtiff-4",
    "LIBXML": "libxml-2.0",
    "OPENJPEG2": "libopenjp2",
    "ZSTD": "libzstd",
}

# Externals whose pkg-config version is not the release version (freetype2
# reports its libtool version, 26.x for FreeType 2.14): the release is
# read from the header
HEADER_VERSIONS = {
    "FREETYPE": ("freetype2", "freetype/freetype.h",
                 ("FREETYPE_MAJOR", "FREETYPE_MINOR", "FREETYPE_PATCH")),
    "GRAPHITE": ("graphite2", "graphite2/Font.h",
                 ("GR2_VERSION_MAJOR", "GR2_VERSION_MINOR",
                  "GR2_VERSION_BUGFIX")),
}


def header_version(module, header, macros):
    flags = run(["pkg-config", "--cflags-only-I", module]) or ""
    includedir = (run(["pkg-config", "--variable=includedir", module])
                  or "").strip()
    for directory in [flag[2:] for flag in flags.split()] + [includedir]:
        path = os.path.join(directory, header)
        if directory and os.path.isfile(path):
            with open(path, encoding="utf-8", errors="replace") as f:
                content = f.read()
            parts = [re.search(r"#\s*define\s+" + macro + r"\s+(\d+)", content)
                     for macro in macros]
            if all(parts):
                return ".".join(part.group(1) for part in parts)
    return None


def pkgconfig_version(external):
    if external in HEADER_VERSIONS:
        return header_version(*HEADER_VERSIONS[external])
    candidates = [PKGCONFIG_NAMES.get(external), external, external.lower(),
                  "lib" + external.lower()]
    for module in candidates:
        if module:
            version = run(["pkg-config", "--modversion", module])
            if version:
                return version.strip()
    # a module named with its API version (liborcus-0.20)
    modules = run(["pkg-config", "--list-all"]) or ""
    for line in modules.splitlines():
        module = line.split(" ", 1)[0]
        if re.fullmatch("(lib)?" + re.escape(external.lower()) + r"-[0-9.]+",
                        module):
            version = run(["pkg-config", "--modversion", module])
            if version:
                return version.strip()
    return None


def system_library_components(engine_builddir, provided_by):
    """The engine's system externals as platform components, named the way
    the engine names them when it bundles them, so the same CVEs match."""
    values = parse_make_variables(os.path.join(engine_builddir,
                                               "config_host.mk"), ["SRCDIR"])
    annotations = license_annotations(values["SRCDIR"])
    components = []
    for external in system_externals(engine_builddir):
        fields = annotations.get(SYSTEM_LICENSE_CLASSES.get(external, external)) \
            or UNANNOTATED_SYSTEM_EXTERNALS.get(external, {})
        name = fields.get("Name") or external.lower()
        version = pkgconfig_version(external)
        component = {
            "name": name,
            "version": version,
            "vendor": fields.get("Vendor") or None,
            "url": fields.get("URL"),
            "type": "library",
            "layer": "platform",
            "comment": f"Supplied by {provided_by}; the version is the one "
                       f"the product was built against.",
        }
        components.append(component)
    return components


def flatpak_platform(args, product_files):
    with open(args.flatpak_manifest, encoding="utf-8") as f:
        manifest = json.load(f)
    runtime = f"{manifest['runtime']}//{manifest['runtime-version']}"
    components = [{
        "name": manifest["runtime"],
        "version": manifest["runtime-version"],
        "type": "platform",
        "layer": "platform",
        "url": "https://flathub.org/",
        "purl": f"pkg:generic/{manifest['runtime']}@{manifest['runtime-version']}",
        "comment": "The flatpak runtime the product runs on, with Qt and the "
                   "freedesktop libraries; updated by its publisher.",
    }]
    if "base" in manifest:
        qtwebengine = pkgconfig_version("Qt6WebEngineCore")
        components.append({
            "name": manifest["base"],
            "version": manifest["base-version"],
            "type": "framework",
            "layer": "bundled-binary",
            "purl": f"pkg:generic/{manifest['base']}@{manifest['base-version']}",
            "url": "https://github.com/flathub/io.qt.qtwebengine.BaseApp",
            "comment": "The flatpak base app the product is built on: Qt "
                       "WebEngine and the Chromium it embeds, copied into "
                       "the product.",
        })
        if qtwebengine:
            components.append({
                "name": "qtwebengine",
                "version": qtwebengine,
                "vendor": "qt",
                "type": "library",
                "layer": "bundled-binary",
                "url": "https://www.qt.io/",
                "comment": f"From {manifest['base']}.",
            })
    components.extend(system_library_components(
        args.engine_builddir, f"the flatpak runtime {runtime}"))
    return components, {}


def deb_fields(deb):
    """Package, version, architecture, homepage, source package and source
    version of a .deb; the homepage may be empty."""
    output = run(["dpkg-deb", "--show", "--showformat",
                  "${Package}\\t${Version}\\t${Architecture}\\t${Homepage}"
                  "\\t${source:Package}\\t${source:Version}\\n", deb])
    if not output:
        raise Exception(f"cannot read {deb}")
    fields = output.rstrip("\n").split("\t")
    return fields + [""] * (6 - len(fields))


def deb_files(deb):
    listing = run(["dpkg-deb", "--contents", deb])
    if listing is None:
        raise Exception(f"cannot read {deb}")
    files = []
    for line in listing.splitlines():
        parts = line.split(None, 5)
        if len(parts) == 6 and parts[0].startswith("-"):
            path = parts[5]
            if path.startswith("./"):
                path = path[2:]
            files.append(path)
    return files


def snap_platform(args, product_files):
    """Ubuntu packages the snap stages (they ship inside it) and the snaps
    it runs on."""
    components = []
    claims = {}
    debs = sorted(deb for directory in args.snap_stage_packages
                  for deb in glob.glob(os.path.join(directory, "*.deb")))
    if not debs:
        raise Exception("no staged packages in "
                        + ", ".join(args.snap_stage_packages))
    release = args.ubuntu_release
    for deb in debs:
        (package, version, arch, homepage, source, source_version) = \
            deb_fields(deb)
        files = [path for path in deb_files(deb) if path in product_files]
        if not files:
            continue  # filtered out by the snap's stage/prime rules
        source = source or package
        source_version = source_version or version
        # Ubuntu's security data is about source packages: grype finds it
        # by the upstream qualifier, trivy by its SrcName and SrcVersion
        # properties; without them neither reports anything
        upstream = source if source_version == version \
            else f"{source}@{source_version}"
        quote = lambda value: urllib.parse.quote(value, safe=".+~-")
        components.append({
            "name": package,
            "version": version,
            "type": "library",
            "layer": "bundled-binary",
            "url": homepage or None,
            "purl": f"pkg:deb/ubuntu/{package}@{quote(version)}?arch={arch}"
                    f"&distro=ubuntu-{release}&upstream={quote(upstream)}",
            "properties": [
                {"name": "aquasecurity:trivy:SrcName", "value": source},
                {"name": "aquasecurity:trivy:SrcVersion",
                 "value": source_version},
            ],
            "comment": f"Ubuntu {release} package, shipped inside the snap.",
        })
        for path in files:
            claims[path] = package
    # the launchers the kde-neon-6 extension puts into the snap
    launchers = [path for path in product_files
                 if path.startswith("snap/command-chain/")]
    if launchers:
        components.append({
            "name": "snapcraft-desktop-launchers",
            "version": None,
            "type": "application",
            "layer": "bundled-binary",
            "url": "https://snapcraft.io/docs/supported-extensions",
            "comment": "The command-chain scripts of the snap's desktop "
                       "extension.",
        })
        for path in launchers:
            claims[path] = "snapcraft-desktop-launchers"
    for spec in args.snap_platform:
        # NAME, or NAME=BUILDSNAP when the build used another snap with the
        # same content (kf6-core24 at run time, kde-qt6-core24-sdk to build)
        (snap, _, source) = spec.partition("=")
        version = None
        meta = os.path.join("/snap", source or snap, "current", "meta",
                            "snap.yaml")
        if os.path.isfile(meta):
            with open(meta, encoding="utf-8") as f:
                match = re.search(r"^version:\s*['\"]?([^'\"\n]+)", f.read(),
                                  re.MULTILINE)
                if match:
                    version = match.group(1)
        components.append({
            "name": snap,
            "version": version,
            "type": "platform",
            "layer": "platform",
            "purl": f"pkg:generic/{snap}" + (f"@{version}" if version else ""),
            "url": f"https://snapcraft.io/{snap}",
            "comment": "A snap the product runs on, updated by its publisher; "
                       "the version is the one it was built against.",
        })
    components.append({
        "name": "ubuntu",
        "version": release,
        "type": "operating-system",
        "layer": "platform",
        "comment": "The base the snap's packages come from.",
    })
    return components, claims


def normalize_cpe(cpe):
    """A full CPE 2.3 string from the ports framework's CPE_STR, which
    leaves fields empty and ends with the FreeBSD release and architecture
    (cpe:2.3:a:haxx:curl:8.17.0:::::freebsd15:x64): the vulnerability data
    has no such target, so only part, vendor, product and version are kept."""
    parts = cpe.split(":")[:6]
    parts += ["*"] * (13 - len(parts))
    return ":".join(part or "*" for part in parts)


def freebsd_platform(args, product_files):
    """The FreeBSD packages providing the libraries the product loads,
    found with pkg which, with the CPE the port declares."""
    packages = {}
    unresolved = []
    for name in sorted(args.loaded_libraries):
        path = None
        for directory in ("/usr/local/lib", "/usr/local/lib/qt6",
                          "/usr/local/lib/nss", "/lib", "/usr/lib"):
            candidate = os.path.join(directory, name)
            if os.path.exists(candidate):
                path = candidate
                break
        if path is None:
            unresolved.append(name)
            continue
        owner = run(["pkg", "which", "-q", path])
        if owner:  # otherwise it is the base system's
            packages[owner.strip()] = None
    components = []
    for package in sorted(packages):
        query = run(["pkg", "query", "%n\t%v\t%o\t%w", package])
        if not query:
            continue
        (name, version, origin, www) = query.strip().split("\t")
        cpe = None
        annotations = run(["pkg", "annotate", "--show", name, "cpe"])
        if annotations:
            match = re.search(r"(cpe:2\.3:\S+)", annotations)
            if match:
                cpe = normalize_cpe(match.group(1))
        components.append({
            "name": name,
            "version": version,
            "type": "library",
            "layer": "platform",
            "url": www or None,
            "cpe": cpe,
            "purl": f"pkg:generic/{name}@{version}?origin={origin}",
            "comment": "FreeBSD package the product depends on; the version "
                       "is the one it was built against.",
        })
    release = run(["freebsd-version", "-u"]) or ""
    components.append({
        "name": "freebsd",
        "version": release.strip() or None,
        "type": "operating-system",
        "layer": "platform",
        "cpe": "cpe:2.3:o:freebsd:freebsd:"
               + (re.sub(r"-.*", "", release.strip()) or "*")
               + ":*:*:*:*:*:*:*",
    })
    if unresolved:
        log(f"freebsd: not found: {', '.join(unresolved)}")
    return components, {}


def windows_platform(args, product_files):
    """What the MSIX package declares it needs, and the WebView2 runtime."""
    manifest = os.path.join(args.srcdir, "windows", "coda",
                            "AppxManifest.xml.in")
    with open(manifest, encoding="utf-8") as f:
        content = f.read()
    components = []
    match = re.search(r'<TargetDeviceFamily[^>]*MinVersion="([^"]+)"', content)
    components.append({
        "name": "windows",
        "version": match.group(1) if match else None,
        "type": "operating-system",
        "layer": "platform",
        "comment": "The minimum version the package declares.",
    })
    for (name, version) in re.findall(
            r'<PackageDependency\s+Name="([^"]+)"[^>]*MinVersion="([^"]+)"',
            content):
        components.append({
            "name": name,
            "version": version,
            "type": "framework",
            "layer": "platform",
            "comment": "A framework package the MSIX package depends on, "
                       "serviced by Windows; the minimum version it declares.",
        })
    components.append({
        "name": "Microsoft Edge WebView2 Runtime",
        "version": None,
        "type": "framework",
        "layer": "platform",
        "url": "https://developer.microsoft.com/microsoft-edge/webview2/",
        "comment": "The Evergreen runtime installed on the machine, which "
                   "Microsoft updates.",
    })
    # the .NET runtimes a framework-dependent app (PrintPDFAndDelete) needs
    for path in sorted(product_files):
        if not path.endswith(".runtimeconfig.json"):
            continue
        with open(os.path.join(args.tree, path), encoding="utf-8") as f:
            options = json.load(f).get("runtimeOptions", {})
        frameworks = options.get("frameworks") \
            or [options.get("framework") or {}]
        for framework in frameworks:
            if framework.get("name"):
                components.append({
                    "name": framework["name"],
                    "version": framework.get("version"),
                    "type": "framework",
                    "layer": "platform",
                    "url": "https://dotnet.microsoft.com/",
                    "comment": f"Needed by {os.path.basename(path)[:-20]}, "
                               f"which is framework-dependent; the minimum "
                               f"version.",
                })
    return components, {}


def macos_platform(args, product_files):
    target = None
    project = os.path.join(args.srcdir, "macos", "coda", "coda.xcodeproj",
                           "project.pbxproj")
    with open(project, encoding="utf-8") as f:
        match = re.search(r"MACOSX_DEPLOYMENT_TARGET = ([0-9.]+);", f.read())
        if match:
            target = match.group(1)
    return [{
        "name": "macos",
        "version": target,
        "type": "operating-system",
        "layer": "platform",
        "cpe": "cpe:2.3:o:apple:macos:*:*:*:*:*:*:*:*",
        "comment": "The minimum version the app supports; WebKit and the "
                   "system frameworks come with it.",
    }], {}


# where in the product the SBOM goes, relative to --tree
OUTPUT_DIRS = {
    "flatpak": "share/sbom",                    # /app/share/sbom
    "snap": "usr/share/sbom",                   # $SNAP/usr/share/sbom
    "freebsd": "share/collabora-office/sbom",   # ${PREFIX}/share/...
    "windows": "sbom",                          # next to program\
    "macos": "Contents/Resources/sbom",
}

PLATFORMS = {
    "flatpak": ("qt", flatpak_platform),
    "snap": ("qt", snap_platform),
    "freebsd": ("qt", freebsd_platform),
    "windows": ("windows", windows_platform),
    "macos": ("macos", macos_platform),
}


def loaded_libraries(staging):
    """The names of the system libraries the product's binaries load, as the
    documents record them."""
    names = set()
    for path in glob.glob(os.path.join(staging, "engine", "*.json")) \
            + [os.path.join(staging, "app.spdx.json")]:
        with open(path, encoding="utf-8") as f:
            for element in json.load(f)["@graph"]:
                if element.get("type") == "software_Package" \
                        and element.get("comment") == \
                        "Supplied by the operating system.":
                    names.add(element["name"])
    return names


# ---------------------------------------------------------------------------
# stage
# ---------------------------------------------------------------------------

def product_version(builddir):
    with open(os.path.join(builddir, "config_version.h"),
              encoding="utf-8") as f:
        match = re.search(r'#define COOLWSD_VERSION "([^"]+)"', f.read())
    if not match:
        raise Exception("no COOLWSD_VERSION in config_version.h")
    return match.group(1)


def stage(args):
    tree = args.tree
    staging = args.staging
    if os.path.exists(staging):
        shutil.rmtree(staging)
    os.makedirs(staging)
    (app, probe) = PLATFORMS[args.platform]
    create_sbom = load_module("create_sbom",
                              os.path.join(SCRIPT_DIR, "create-sbom.py"))
    args.engine_builddir = create_sbom.local_path(args.engine_builddir)

    excludes = list(args.exclude)
    preexisting = set()
    if args.preexisting:
        with open(args.preexisting, encoding="utf-8") as f:
            preexisting = {line.rstrip("\n") for line in f if line.strip()}
    output_dir = os.path.join(tree, args.output_dir)
    files = [path for path in walk_tree(tree, excludes,
                                        skip=[staging, output_dir])
             if path not in preexisting]
    product_files = set(files)
    log(f"{len(files)} files in the product")

    engine_claimed, engine_missing = stage_engine(
        args.engine_builddir, args.engine_root, tree, product_files, staging)

    # the platform's claims first: a file of a third-party package is never
    # the app's
    args.loaded_libraries = set()
    platform_components, platform_claims = [], {}
    if args.platform != "freebsd":
        platform_components, platform_claims = probe(args, product_files)

    # The rest is the app's, if the app's build put it there. A file in
    # the engine's part of the product that its install list does not have
    # was delivered into instdir by the engine's build and copied along
    # (Windows and macOS copy all of instdir): it belongs to the product,
    # but to none of the engine's packages.
    app_globs = create_sbom.app_layout(
        app, args.srcdir, args.builddir, args.engine_builddir)["files"]
    engine_prefix = args.engine_root[1]
    app_files, extra = [], []
    for path in files:
        if path in engine_claimed or path in platform_claims:
            continue
        if matches(path, app_globs) if app_globs is not None \
                else not path.startswith(engine_prefix):
            app_files.append({"instpath": path,
                              "abspath": os.path.join(tree, path)})
        else:
            extra.append(path)
    if extra:
        log(f"{len(extra)} files in the engine's part of the product that "
            f"the engine does not install, e.g. {extra[0]}")
    version = args.version or product_version(args.builddir)
    document = create_sbom.app_document(
        app, args.srcdir, args.builddir, args.engine_builddir, app_files,
        version, product_names={os.path.basename(path) for path in files})
    with open(os.path.join(staging, "app.spdx.json"), "w",
              encoding="utf-8") as f:
        json.dump(document, f, indent=1)

    if args.platform == "freebsd":
        args.loaded_libraries = loaded_libraries(staging)
        platform_components, platform_claims = probe(args, product_files)

    owners = {}
    extra = set(extra)
    for path in files:
        if path in engine_claimed:
            owners[path] = "engine:" + engine_claimed[path]
        elif path in platform_claims:
            owners[path] = "platform:" + platform_claims[path]
        elif path in extra:
            owners[path] = "extra"
        else:
            owners[path] = "app"
    product = {
        "platform": args.platform,
        "name": args.product_name,
        "version": version,
        "build": args.build,
        "hash_basis": args.hash_basis,
        "vex_url": args.vex_url,
        "output_dir": args.output_dir,
        "excludes": excludes,
        "preexisting": sorted(preexisting),
        "files": owners,
        "engine_files_not_shipped": engine_missing,
        "platform_components": platform_components,
    }
    with open(os.path.join(staging, "product.json"), "w",
              encoding="utf-8") as f:
        json.dump(product, f, indent=1)
    # what finalize needs, for when it runs without the source tree
    shutil.copy(os.path.abspath(__file__), staging)
    image_sbom = image_sbom_module()
    shutil.copy(image_sbom.__file__, os.path.join(staging,
                                                  "generate-image-sbom.py"))
    log(f"staged in {staging}")


# ---------------------------------------------------------------------------
# finalize
# ---------------------------------------------------------------------------

def spdx_id():
    return f"urn:uuid:{uuid.uuid4()}"


def merged_spdx(product, documents, hashes):
    """One SPDX 3.0.1 document of all the staged ones: the elements keep
    their (unique) ids; one creation info, one document, one root."""
    graph = []
    tool_id = spdx_id()
    document_id = spdx_id()
    root_id = spdx_id()
    seen = set()
    roots = []
    for document in documents:
        for element in document["@graph"]:
            kind = element.get("type")
            if kind in ("CreationInfo", "Tool", "SpdxDocument"):
                if kind == "SpdxDocument":
                    roots.extend(element.get("rootElement", []))
                continue
            if kind == "Relationship" \
                    and element.get("relationshipType") == "describes":
                continue
            ident = element.get("spdxId")
            if ident is not None:
                if ident in seen:
                    continue  # the shared Organization
                seen.add(ident)
            element = dict(element)
            if "creationInfo" in element or ident is not None:
                element["creationInfo"] = "_:creationinfo"
            if kind == "software_File":
                element["verifiedUsing"] = [dict(
                    (element.get("verifiedUsing") or [{}])[0],
                    type="Hash", algorithm="sha512",
                    hashValue=hashes[element["name"]])]
            graph.append(element)

    name = product["name"]
    header = [
        {
            "type": "CreationInfo",
            "@id": "_:creationinfo",
            "specVersion": "3.0.1",
            "created": timestamp,
            "createdBy": ["https://collaboraoffice.com"],
            "createdUsing": [tool_id],
        },
        {
            "type": "Tool",
            "spdxId": tool_id,
            "creationInfo": "_:creationinfo",
            "name": "create-product-sbom.py",
        },
        {
            "type": "SpdxDocument",
            "spdxId": document_id,
            "creationInfo": "_:creationinfo",
            "name": f"{name} for {product['platform']}",
            "rootElement": [root_id],
            "profileConformance": ["core", "software", "simpleLicensing"],
        },
        {
            "type": "software_Package",
            "spdxId": root_id,
            "creationInfo": "_:creationinfo",
            "name": name,
            "software_packageVersion": product["version"],
            "software_homePage": PRODUCT_URL,
            "originatedBy": ["https://collaboraoffice.com"],
            "software_primaryPurpose": "application",
            "comment": product_comment(product),
        },
        {
            "type": "Relationship",
            "spdxId": spdx_id(),
            "creationInfo": "_:creationinfo",
            "from": document_id,
            "relationshipType": "describes",
            "to": [root_id],
        },
    ]
    if roots:
        header.append({
            "type": "Relationship",
            "spdxId": spdx_id(),
            "creationInfo": "_:creationinfo",
            "from": root_id,
            "relationshipType": "contains",
            "to": roots,
        })
    if "https://collaboraoffice.com" not in seen:
        header.insert(0, {
            "type": "Organization",
            "spdxId": "https://collaboraoffice.com",
            "creationInfo": "_:creationinfo",
            "name": PRODUCT_SUPPLIER,
        })

    shipped, required = [], []
    for component in product["platform_components"]:
        element = {
            "type": "software_Package",
            "spdxId": spdx_id(),
            "creationInfo": "_:creationinfo",
            "name": component["name"],
            "comment": component.get("comment") or "",
        }
        if component.get("version"):
            element["software_packageVersion"] = component["version"]
        if component.get("url"):
            element["software_homePage"] = component["url"]
        identifiers = []
        # a CPE without a version matches every CVE of the product
        if component.get("cpe") or (component.get("vendor") is not None
                                    and component.get("version")):
            identifiers.append({
                "type": "ExternalIdentifier",
                "externalIdentifierType": "cpe23",
                "identifier": component_cpe(component),
            })
        if component.get("purl"):
            identifiers.append({
                "type": "ExternalIdentifier",
                "externalIdentifierType": "packageUrl",
                "identifier": component["purl"],
            })
        if identifiers:
            element["externalIdentifier"] = identifiers
        graph.append(element)
        component["spdxId"] = element["spdxId"]
        (shipped if component["layer"] == "bundled-binary"
         else required).append(element["spdxId"])
    if shipped:
        header.append({
            "type": "Relationship", "spdxId": spdx_id(),
            "creationInfo": "_:creationinfo", "from": root_id,
            "relationshipType": "contains", "to": shipped,
        })
    if required:
        header.append({
            "type": "Relationship", "spdxId": spdx_id(),
            "creationInfo": "_:creationinfo", "from": root_id,
            "relationshipType": "dependsOn", "to": required,
            "completeness": "noAssertion",
            "comment": "Supplied by the platform, not shipped.",
        })
    # the files of the bundled third-party packages, and those the engine
    # delivered into the product without installing them
    by_name = {component["name"]: component
               for component in product["platform_components"]}
    for path, owner in sorted(product["files"].items()):
        if owner == "extra":
            parent = root_id
        elif owner.startswith("platform:"):
            parent = by_name[owner[len("platform:"):]]["spdxId"]
        else:
            continue
        file_id = spdx_id()
        graph.append({
            "type": "software_File",
            "spdxId": file_id,
            "creationInfo": "_:creationinfo",
            "name": path,
            "verifiedUsing": [{"type": "Hash", "algorithm": "sha512",
                               "hashValue": hashes[path]}],
        })
        graph.append({
            "type": "Relationship", "spdxId": spdx_id(),
            "creationInfo": "_:creationinfo",
            "from": parent,
            "relationshipType": "hasDistributionArtifact", "to": [file_id],
            "completeness": "incomplete",
        })
    # A relationship to an element no document defines (the engine's
    # generator names interpreters the product leaves out) says nothing
    graph = header + graph
    defined = {element.get("spdxId") for element in graph} - {None}
    graph = drop_elements(graph, {
        target for element in graph if element.get("type") == "Relationship"
        for target in [element.get("from")] + element.get("to", [])
        if target not in defined and target.startswith("urn:")})
    return {
        "@context": "https://spdx.org/rdf/3.0.1/spdx-context.jsonld",
        "@graph": graph,
    }


def component_cpe(component):
    if component.get("cpe"):
        return component["cpe"]
    vendor = component.get("vendor") or "*"
    version = component.get("version") or "*"
    return f"cpe:2.3:a:{vendor}:{component['name']}:{version}:*:*:*:*:*:*:*"


def product_comment(product):
    comment = (f"Collabora Office for {product['platform']}, as shipped. "
               f"Components supplied by the platform are listed with the "
               f"version the product was built against.")
    if product.get("hash_basis") == "pre-signing":
        comment += (" The hashes of the Mach-O binaries are those before "
                    "code signing; the code signature verifies them.")
    return comment


def cyclonedx_license(expression, acknowledgement):
    """One license entry: an SPDX license id as a license, our own
    LicenseRef-s by name."""
    if expression.startswith("LicenseRef-"):
        return {"license": {"name": expression[len("LicenseRef-"):],
                            "acknowledgement": acknowledgement}}
    return {"license": {"id": expression, "acknowledgement": acknowledgement}}


def cyclonedx_licenses(declared, concluded):
    """CycloneDX takes either license entries, as many as needed, or exactly
    one expression: both the declared and the concluded license when they
    are single licenses, otherwise the concluded one as an expression."""
    pairs = [(expression, acknowledgement) for (expression, acknowledgement)
             in ((declared, "declared"), (concluded, "concluded"))
             if expression and expression != "NOASSERTION"]
    if all(re.fullmatch(r"[A-Za-z0-9.+-]+", expression)
           for (expression, _) in pairs):
        return [cyclonedx_license(expression, acknowledgement)
                for (expression, acknowledgement) in pairs]
    (expression, acknowledgement) = pairs[-1]
    return [{"expression": expression, "acknowledgement": acknowledgement}]


def cyclonedx(product, spdx, image_sbom):
    """The CycloneDX 1.6 document: the components of the merged SPDX
    document that a scanner can match, plus the platform's."""
    graph = spdx["@graph"]
    by_id = {element.get("spdxId"): element for element in graph
             if element.get("spdxId")}
    declared, concluded, homepages, sources = {}, {}, {}, {}
    for element in graph:
        if element.get("type") != "Relationship":
            continue
        kind = element.get("relationshipType")
        if kind in ("hasDeclaredLicense", "hasConcludedLicense"):
            target = by_id.get(element["to"][0], {})
            expression = target.get("simplelicensing_licenseExpression") \
                or target.get("name")
            (declared if kind == "hasDeclaredLicense"
             else concluded)[element["from"]] = expression
        elif kind == "generates":
            source = by_id.get(element["from"], {})
            for target in element["to"]:
                sources[target] = source

    platform_ids = {component.get("spdxId"): component
                    for component in product["platform_components"]}
    # packages that ship files of their own; a NuGet package that does is a
    # prebuilt binary, a header-only one is compiled into our executables
    file_ids = {element["spdxId"] for element in graph
                if element.get("type") == "software_File"}
    with_files = {element["from"] for element in graph
                  if element.get("type") == "Relationship"
                  and element.get("relationshipType")
                      in ("contains", "hasDistributionArtifact")
                  and any(target in file_ids for target in element["to"])}
    components = {}
    for element in graph:
        if element.get("type") != "software_Package":
            continue
        ident = element["spdxId"]
        platform = platform_ids.get(ident)
        identifiers = image_sbom.external_identifiers(element)
        if not identifiers and platform is None:
            continue  # our own packages, system library stubs
        name = element.get("name")
        version = element.get("software_packageVersion")
        key = (name, version)
        if key in components:
            continue
        if platform:
            layer = platform["layer"]
        elif identifiers.get("packageUrl", "").startswith("pkg:nuget/") \
                and ident in with_files:
            layer = "bundled-binary"
        else:
            layer = "built"
        component = {
            # the bom-ref convention of the container image's SBOM
            "bom-ref": identifiers.get("packageUrl")
                       or f"component:{name}@{version}",
            "type": (platform or {}).get("type", "library"),
            "name": name,
        }
        if version:
            component["version"] = version
        if layer == "built":
            # built from source and shipped by us
            component["supplier"] = {"name": PRODUCT_SUPPLIER}
        if "cpe23" in identifiers:
            component["cpe"] = identifiers["cpe23"]
        if "packageUrl" in identifiers:
            component["purl"] = identifiers["packageUrl"]
        licenses = cyclonedx_licenses(declared.get(ident),
                                      concluded.get(ident))
        if licenses:
            component["licenses"] = licenses
        references = []
        if element.get("software_homePage"):
            references.append({"type": "website",
                               "url": element["software_homePage"]})
        source = sources.get(ident)
        if source:
            for ref in source.get("externalRef", []):
                locator = ref["locator"]
                if isinstance(locator, list):  # as SPDX 3.0.1 has it
                    locator = locator[0]
                reference = {"type": "source-distribution", "url": locator}
                for hash_ in source.get("verifiedUsing", []):
                    reference["hashes"] = [{
                        "alg": {"sha256": "SHA-256", "sha512": "SHA-512"}[
                            hash_["algorithm"]],
                        "content": hash_["hashValue"]}]
                references.append(reference)
        if references:
            component["externalReferences"] = references
        properties = [{"name": "collabora:layer", "value": layer}]
        properties.extend((platform or {}).get("properties", []))
        if layer == "platform":
            properties.append({"name": "collabora:version-basis",
                               "value": "built-against"})
        if element.get("comment") and platform:
            component["description"] = element["comment"]
        component["properties"] = properties
        components[key] = component

    product_ref = (f"pkg:generic/collabora-office@{product['version']}"
                   f"?platform={product['platform']}")
    properties = [
        {"name": "collabora:platform", "value": product["platform"]},
        # what the store packaging adds (the MSIX manifest and assets, the
        # code signature, the flatpak's appstream data) is not described
        {"name": "collabora:sbom-completeness",
         "value": "all-files-the-build-installs"},
    ]
    if product.get("build"):
        properties.append({"name": "collabora:build",
                           "value": str(product["build"])})
    if product.get("hash_basis"):
        properties.append({"name": "collabora:hash-basis",
                           "value": product["hash_basis"]})
    component_list = sorted(components.values(),
                            key=lambda c: (c["name"].lower(),
                                           c.get("version") or ""))
    document = {
        "bomFormat": "CycloneDX",
        "specVersion": "1.6",
        "serialNumber": f"urn:uuid:{uuid.uuid4()}",
        "version": 1,
        "metadata": {
            "timestamp": timestamp,
            "lifecycles": [{"phase": "build"}],
            "tools": {"components": [{
                "type": "application",
                "name": "create-product-sbom.py",
                "supplier": {"name": PRODUCT_SUPPLIER},
            }]},
            "component": {
                "bom-ref": product_ref,
                "type": "application",
                "name": product["name"],
                "version": product["version"],
                "purl": product_ref,
                "supplier": {"name": PRODUCT_SUPPLIER,
                             "url": [PRODUCT_URL]},
                "licenses": [{"license": {"id": "MPL-2.0"}}],
                "properties": properties,
            },
            "supplier": {"name": PRODUCT_SUPPLIER, "url": [PRODUCT_URL]},
        },
        "components": component_list,
        "dependencies": [{
            "ref": product_ref,
            "dependsOn": [component["bom-ref"]
                          for component in component_list],
        }],
    }
    if product.get("vex_url"):
        document["externalReferences"] = [{
            "type": "exploitability-statement",
            "url": product["vex_url"],
            "comment": "Collabora's assessment of the vulnerabilities "
                       "scanners report for this product.",
        }]
    return document


def finalize(args):
    staging = args.staging
    tree = args.tree
    with open(os.path.join(staging, "product.json"), encoding="utf-8") as f:
        product = json.load(f)
    output_dir = os.path.join(tree, product["output_dir"])

    files = walk_tree(tree, product["excludes"], skip=[staging, output_dir])
    preexisting = set(product["preexisting"])
    expected = product["files"]
    unclaimed = [path for path in files
                 if path not in expected and path not in preexisting]
    gone = [path for path in expected if not os.path.isfile(
        os.path.join(tree, path))]
    if unclaimed or gone:
        for path in unclaimed[:50]:
            print(f"ERROR: not described by the SBOM: {path}", file=sys.stderr)
        for path in gone[:50]:
            print(f"ERROR: described by the SBOM but gone: {path}",
                  file=sys.stderr)
        raise Exception(f"{len(unclaimed)} files of the product are not in "
                        f"the SBOM, {len(gone)} in it are not in the product")

    hashes = {}
    for (count, path) in enumerate(expected, 1):
        hashes[path] = sha512_file(os.path.join(tree, path))
        if count % 2000 == 0:
            print(f"hashed {count} of {len(expected)} files", file=sys.stderr)

    documents = []
    for path in sorted(glob.glob(os.path.join(staging, "engine", "*.json"))):
        with open(path, encoding="utf-8") as f:
            documents.append(json.load(f))
    with open(os.path.join(staging, "app.spdx.json"), encoding="utf-8") as f:
        documents.append(json.load(f))

    spdx = merged_spdx(product, documents, hashes)
    image_sbom = load_module(
        "generate_image_sbom",
        os.path.join(staging, "generate-image-sbom.py"))
    cdx = cyclonedx(product, spdx, image_sbom)

    # the engine copies its per-package documents into instdir, so they can
    # be in the product's SBOM directory: this one replaces them
    os.makedirs(output_dir, exist_ok=True)
    for old in glob.glob(os.path.join(output_dir, "*-sbom.spdx.json")):
        os.remove(old)
    outputs = []
    for (suffix, document) in (("cdx.json", cdx), ("spdx.json", spdx)):
        path = os.path.join(output_dir, f"{SBOM_BASENAME}.{suffix}")
        with open(path, "w", encoding="utf-8") as f:
            json.dump(document, f, indent=1)
            f.write("\n")
        outputs.append(path)
    if args.sidecar:
        # next to the artifact: <artifact basename>.cdx.json, .spdx.json
        for (suffix, path) in zip(("cdx.json", "spdx.json"), outputs):
            shutil.copy(path, f"{args.sidecar}.{suffix}")
    if not args.keep_staging:
        shutil.rmtree(staging)
    print(f"{outputs[0]}: {len(cdx['components'])} components, "
          f"{len(expected)} files")


def snapshot(args):
    with open(args.output, "w", encoding="utf-8") as f:
        for path in walk_tree(args.tree):
            f.write(path + "\n")


def main():
    parser = argparse.ArgumentParser(
        description="The SBOM of a Collabora Office desktop product")
    subparsers = parser.add_subparsers(dest="step", required=True)

    def common(sub):
        sub.add_argument("--tree", required=True,
                         help="the product as installed: the root of the "
                              "flatpak's /app, the snap, the FreeBSD "
                              "package's PREFIX, the Windows app directory, "
                              "the Mac .app bundle")
        sub.add_argument("--staging", required=True,
                         help="directory for what stage found")
        sub.add_argument("--verbose", action="store_true")

    for name in ("stage", "all"):
        sub = subparsers.add_parser(name)
        common(sub)
        sub.add_argument("--platform", required=True, choices=PLATFORMS)
        sub.add_argument("--srcdir", default=os.path.dirname(SCRIPT_DIR))
        sub.add_argument("--builddir", required=True,
                         help="the online build tree")
        sub.add_argument("--engine-builddir", required=True)
        sub.add_argument("--engine-root", required=True,
                         help="where the engine's instdir is in the product, "
                              "OLD=NEW to replace a prefix of the engine's "
                              "paths (macOS: CollaboraOffice.app/=)")
        sub.add_argument("--product-name", default="Collabora Office")
        sub.add_argument("--version",
                         help="default: COOLWSD_VERSION of the build")
        sub.add_argument("--build", help="build number of the package")
        sub.add_argument("--output-dir",
                         help="where in the product the SBOM goes (default: "
                              "where the platform's packaging has it)")
        sub.add_argument("--exclude", action="append", default=[],
                         help="glob of product paths that are not shipped")
        sub.add_argument("--preexisting",
                         help="list of files that were in the tree before "
                              "the product (from the snapshot step)")
        sub.add_argument("--hash-basis",
                         help="pre-signing: binaries are signed afterwards")
        sub.add_argument("--vex-url", default=DEFAULT_VEX_URL)
        sub.add_argument("--flatpak-manifest", default=os.path.join(
            os.path.dirname(SCRIPT_DIR), "qt", "flatpak",
            "com.collaboraoffice.Office.json"))
        sub.add_argument("--snap-stage-packages", action="append",
                         default=[],
                         help="a directory of the .debs a snap part stages")
        sub.add_argument("--snap-platform", action="append", default=[],
                         help="a snap the product runs on (base, content), "
                              "NAME=BUILDSNAP to take its version from the "
                              "snap the build used in its place")
        sub.add_argument("--ubuntu-release", default="24.04")
        if name == "all":
            sub.add_argument("--sidecar",
                             help="also write the documents as SIDECAR."
                                  "cdx.json and SIDECAR.spdx.json")
            sub.add_argument("--keep-staging", action="store_true")
    sub = subparsers.add_parser("finalize")
    common(sub)
    sub.add_argument("--sidecar",
                     help="also write the documents as SIDECAR.cdx.json "
                          "and SIDECAR.spdx.json")
    sub.add_argument("--keep-staging", action="store_true")
    sub = subparsers.add_parser("snapshot")
    sub.add_argument("--tree", required=True)
    sub.add_argument("--output", required=True)
    args = parser.parse_args()

    global verbose
    verbose = getattr(args, "verbose", False)
    if args.step == "snapshot":
        snapshot(args)
        return
    if args.step in ("stage", "all"):
        (old, sep, new) = args.engine_root.partition("=")
        args.engine_root = (old, new) if sep else ("", old)
        args.output_dir = (args.output_dir
                           or OUTPUT_DIRS[args.platform]).strip("/")
        stage(args)
    if args.step in ("finalize", "all"):
        finalize(args)


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(f"create-product-sbom.py: {error}", file=sys.stderr)
        sys.exit(1)
