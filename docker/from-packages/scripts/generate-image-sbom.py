#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Generate the image-level SBOMs of the Collabora Online container, in the
# 'sbom' stage of the from-packages Dockerfile. Standard library only.
#
# Two documents are produced:
# - a CycloneDX 1.6 SBOM (the format the ZenDiS / openCode / DevGuard
#   ecosystem publishes and consumes): the base image's own SBOM merged with
#   one pkg:deb component per shipped package
# - an SPDX 3.0.1 aggregate in the same shape as the engine's and online's
#   per-package SBOMs, for consumers of that ecosystem
#
# Both reference - rather than inline - the rich per-package SPDX 3.0.1
# documents that ship inside the image (/opt/collaboraoffice*/<package>-
# sbom.spdx.json from the engine, /usr/share/coolwsd/collabora-online-
# sbom.spdx.json from online), each pinned by its sha256. Those documents
# carry the per-file hashes, static-link relationships and upstream component
# identities (CPEs, purls) of the Collabora-published packages, which no
# distro security feed covers - scanners should be pointed at them.
#
# The shipped-package set is the union of /tmp/pkgs.added (the dpkg diff
# computed by assemble-rootfs.sh) and the owners of /tmp/rootfs.files: the
# assembler's library-closure loop ships files of packages (e.g. libstdc++6)
# without appending them to pkgs.added.

import argparse
import glob
import hashlib
import json
import os
import re
import subprocess
import sys
import urllib.parse
import uuid
from datetime import datetime, timezone

# Packages published by Collabora, not Debian: purl namespace pkg:deb/collabora
# and deliberately not matchable against distro security feeds - their
# vulnerability surface is described by the embedded SPDX documents instead.
COLLABORA_PACKAGE_RE = re.compile(
    r"^(cool|collaboraoffice|collabora-online-|code-brand)")

timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def log(message):
    print(f"generate-image-sbom: {message}", file=sys.stderr)


def sha256_file(path):
    digest = hashlib.sha256()
    with open(path, "rb") as f:
        while True:
            chunk = f.read(1 << 20)
            if not chunk:
                break
            digest.update(chunk)
    return digest.hexdigest()


def dpkg_query(args):
    return subprocess.run(["dpkg-query"] + args, check=True,
                          stdout=subprocess.PIPE, text=True).stdout


def shipped_packages(pkgs_added, rootfs_files):
    """Union of the recorded package diff and the owners of the shipped
    files (the closure loop ships files without updating pkgs.added)."""
    with open(pkgs_added, encoding="utf-8") as f:
        packages = {line.strip() for line in f if line.strip()}

    with open(rootfs_files, encoding="utf-8") as f:
        # regular files and symlinks only: a directory entry would match every
        # package that lists that directory
        files = [line.strip() for line in f
                 if line.strip().startswith(("/usr/lib/", "/lib/"))
                 and (os.path.islink(line.strip())
                      or os.path.isfile(line.strip()))]
    for i in range(0, len(files), 500):
        result = subprocess.run(["dpkg-query", "-S"] + files[i:i + 500],
                                stdout=subprocess.PIPE,
                                stderr=subprocess.DEVNULL, text=True)
        for line in result.stdout.splitlines():
            if line.startswith("diversion "):
                continue
            owners, separator, _ = line.partition(": ")
            if separator:
                for owner in owners.split(", "):
                    packages.add(owner.split(":")[0])
    return sorted(packages)


def copyright_license(package):
    """First License: value of the machine-readable copyright file."""
    path = f"/usr/share/doc/{package}/copyright"
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            content = f.read()
    except FileNotFoundError:
        return None
    if "Format:" not in content.split("\n", 1)[0]:
        return None  # not machine-readable
    match = re.search(r"^License:\s*(\S[^\n]*)$", content, re.MULTILINE)
    return match.group(1).strip() if match else None


def package_info(packages):
    fields = dpkg_query(["-W", "-f",
                         "${Package}\\t${Version}\\t${Architecture}\\t"
                         "${Homepage}\\n"] + packages)
    result = []
    for line in fields.splitlines():
        name, version, arch, homepage = (line.split("\t") + [""])[:4]
        namespace = ("collabora" if COLLABORA_PACKAGE_RE.match(name)
                     else "debian")
        purl = (f"pkg:deb/{namespace}/{name}@{urllib.parse.quote(version)}"
                f"?arch={arch}")
        if namespace == "debian":
            purl += "&distro=debian-13"
        result.append({
            "name": name,
            "version": version,
            "arch": arch,
            "homepage": homepage,
            "namespace": namespace,
            "purl": purl,
            "license": copyright_license(name),
        })
    return result


def find_embedded_sboms(rootfs):
    """The per-package SPDX documents that ship inside the image."""
    embedded = []
    for pattern in ("opt/collaboraoffice*/*sbom*.json",
                    "usr/share/coolwsd/*sbom*.json",
                    "usr/share/doc/*/*sbom*.json"):
        for path in sorted(glob.glob(os.path.join(rootfs, pattern))):
            image_path = "/" + os.path.relpath(path, rootfs)
            basename = os.path.basename(path)
            owner = None
            if basename == "collabora-online-sbom.spdx.json":
                owner = "coolwsd"
            else:
                match = re.match(r"(.+)-sbom\.spdx\.json$", basename)
                if match:
                    owner = match.group(1)
            embedded.append({
                "path": image_path,
                "abspath": path,
                "sha256": sha256_file(path),
                "owner": owner,
            })
    return embedded


def lift_embedded_components(embedded):
    """Lift the component inventory (name, version, purl, CPE) out of the
    embedded SPDX 3.0.1 documents.

    The Collabora-published packages have no distro security feed, so their
    vulnerability surface is exactly these inner components: the statically
    linked C++ externals (CPEs) and the shipped npm packages (purls). Most
    scanners cannot read SPDX 3.x yet, so replicating the inventory as
    CycloneDX components makes one scan of the image-level SBOM cover the
    contents of COOL itself, not just the deb layer."""
    components = {}
    for doc in embedded:
        try:
            with open(doc["abspath"], encoding="utf-8") as f:
                graph = json.load(f).get("@graph", [])
        except (OSError, ValueError) as error:
            log(f"warning: cannot read {doc['path']}: {error}")
            continue
        for element in graph:
            if element.get("type") != "software_Package":
                continue
            identifiers = {i.get("externalIdentifierType"): i.get("identifier")
                           for i in element.get("externalIdentifiers", [])}
            if not identifiers:
                continue  # root packages and OS-supplied stubs
            name = element.get("name")
            version = element.get("software_packageVersion")
            key = (name, version)
            if key in components:
                continue
            component = {
                "bom-ref": identifiers.get("packageURL")
                           or f"component:{name}@{version}",
                "type": "library",
                "name": name,
                "properties": [
                    {"name": "collabora:layer", "value": "components"},
                    {"name": "collabora:distribution-package",
                     "value": doc["owner"] or "unknown"},
                ],
            }
            if version:
                component["version"] = version
            if "packageURL" in identifiers:
                component["purl"] = identifiers["packageURL"]
            if "cpe23" in identifiers:
                component["cpe"] = identifiers["cpe23"]
            components[key] = component
    return list(components.values())


# ---------------------------------------------------------------------------
# CycloneDX
# ---------------------------------------------------------------------------

def cyclonedx_document(image_purl, image_name, image_version, base_sbom,
                       base_ref, packages, embedded, completeness):
    # an operating-system component keys scanners (e.g. trivy) to match the
    # pkg:deb components against the Debian security feeds
    components = [{
        "bom-ref": "os:debian-13",
        "type": "operating-system",
        "name": "debian",
        "version": "13",
    }]
    for component in base_sbom.get("components", []):
        component = dict(component)
        component.setdefault("properties", []).append(
            {"name": "collabora:layer", "value": "base"})
        components.append(component)
    if not base_sbom.get("components"):
        components.append({
            "bom-ref": f"pkg:oci/{base_ref}",
            "type": "container",
            "name": base_ref,
            "properties": [
                {"name": "collabora:layer", "value": "base"},
                {"name": "collabora:base-sbom", "value": "unavailable"},
            ],
        })

    embedded_by_owner = {}
    for doc in embedded:
        embedded_by_owner.setdefault(doc["owner"], []).append(doc)

    seen_purls = {c.get("purl") for c in components}
    for package in packages:
        if package["purl"] in seen_purls:
            continue
        component = {
            "bom-ref": package["purl"],
            "type": "library",
            "name": package["name"],
            "version": package["version"],
            "purl": package["purl"],
            "properties": [{"name": "collabora:layer", "value": "packages"}],
        }
        if package["license"]:
            component["licenses"] = [
                {"license": {"name": package["license"]}}]
        if package["homepage"]:
            component["externalReferences"] = [
                {"type": "website", "url": package["homepage"]}]
        for doc in embedded_by_owner.get(package["name"], []):
            component.setdefault("externalReferences", []).append({
                "type": "bom",
                "url": f"file://{doc['path']}",
                "hashes": [{"alg": "SHA-256", "content": doc["sha256"]}],
                "comment": "SPDX 3.0.1 SBOM of this package, inside the image",
            })
        components.append(component)

    components.extend(lift_embedded_components(embedded))

    return {
        "bomFormat": "CycloneDX",
        "specVersion": "1.6",
        "serialNumber": f"urn:uuid:{uuid.uuid4()}",
        "version": 1,
        "metadata": {
            "timestamp": timestamp,
            "tools": {"components": [{
                "type": "application",
                "name": "generate-image-sbom.py",
                "supplier": {"name": "Collabora Productivity Ltd."},
            }]},
            "component": {
                "bom-ref": image_purl,
                "type": "container",
                "name": image_name,
                "version": image_version,
                "purl": image_purl,
            },
        },
        "components": components,
        "properties": [
            {"name": "collabora:sbom-completeness", "value": completeness},
            {"name": "collabora:base-image", "value": base_ref},
        ],
    }


# ---------------------------------------------------------------------------
# SPDX 3.0.1 aggregate, in the engine's element shapes
# ---------------------------------------------------------------------------

def spdx_document(image_purl, image_name, image_version, base_sbom, base_ref,
                  packages, embedded, completeness):
    def new_id():
        return f"urn:uuid:{uuid.uuid4()}"

    root_spdx_id = new_id()
    document_spdx_id = new_id()
    tool_spdx_id = new_id()
    graph = [
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
            "name": "generate-image-sbom.py",
        },
        {
            "type": "SpdxDocument",
            "spdxId": document_spdx_id,
            "creationInfo": "_:creationinfo",
            "name": image_name,
            "rootElement": [root_spdx_id],
            "profileConformance": ["core", "software"],
            "comment": f"completeness: {completeness}",
        },
        {
            "type": "software_Package",
            "spdxId": root_spdx_id,
            "creationInfo": "_:creationinfo",
            "name": image_name,
            "software_packageVersion": image_version,
            "software_primaryPurpose": "container",
            "externalIdentifiers": [{
                "externalIdentifierType": "packageURL",
                "identifier": image_purl,
            }],
        },
        {
            "type": "Relationship",
            "spdxId": new_id(),
            "creationInfo": "_:creationinfo",
            "from": document_spdx_id,
            "relationshipType": "describes",
            "to": [root_spdx_id],
        },
    ]

    def add_package(name, version, purl, comment=None, external_refs=None):
        pkg_spdx_id = new_id()
        element = {
            "type": "software_Package",
            "spdxId": pkg_spdx_id,
            "creationInfo": "_:creationinfo",
            "name": name,
        }
        if version:
            element["software_packageVersion"] = version
        if purl:
            element["externalIdentifiers"] = [{
                "externalIdentifierType": "packageURL",
                "identifier": purl,
            }]
        if comment:
            element["comment"] = comment
        if external_refs:
            element["externalRef"] = external_refs
        graph.append(element)
        graph.append({
            "type": "Relationship",
            "spdxId": new_id(),
            "creationInfo": "_:creationinfo",
            "from": root_spdx_id,
            "relationshipType": "contains",
            "to": [pkg_spdx_id],
        })
        return pkg_spdx_id

    for component in base_sbom.get("components", []):
        add_package(component.get("name"), component.get("version"),
                    component.get("purl"),
                    comment=f"From the base image {base_ref}.")

    embedded_by_owner = {}
    for doc in embedded:
        embedded_by_owner.setdefault(doc["owner"], []).append(doc)
    for package in packages:
        external_refs = [{
            "type": "ExternalRef",
            "externalRefType": "other",
            "locator": f"file://{doc['path']}",
            "comment": ("SPDX 3.0.1 SBOM of this package inside the image, "
                        f"sha256:{doc['sha256']}"),
        } for doc in embedded_by_owner.get(package["name"], [])]
        add_package(package["name"], package["version"], package["purl"],
                    external_refs=external_refs or None)

    return {
        "@context": "https://spdx.org/rdf/3.0.1/spdx-context.jsonld",
        "@graph": graph,
    }


def main():
    parser = argparse.ArgumentParser(
        description="Generate the image-level SBOMs of the container")
    parser.add_argument("--rootfs", default="/rootfs")
    parser.add_argument("--pkgs-added", default="/tmp/pkgs.added")
    parser.add_argument("--rootfs-files", default="/tmp/rootfs.files")
    parser.add_argument("--base-sbom", required=True)
    parser.add_argument("--base-ref", required=True)
    parser.add_argument("--image-name", default="collabora-online")
    parser.add_argument("--image-version", required=True)
    parser.add_argument("--strict", action="store_true")
    parser.add_argument("--out-dir", default="/sbom")
    args = parser.parse_args()

    with open(args.base_sbom, encoding="utf-8") as f:
        base_sbom = json.load(f)

    packages = package_info(
        shipped_packages(args.pkgs_added, args.rootfs_files))
    embedded = find_embedded_sboms(args.rootfs)
    log(f"{len(packages)} shipped packages, "
        f"{len(base_sbom.get('components', []))} base components, "
        f"{len(embedded)} embedded SPDX documents")
    if not embedded:
        log("notice: no embedded per-package SPDX documents found - the "
            "installed packages predate SBOM-carrying releases")

    parts = ["debs"]
    if base_sbom.get("components"):
        parts.insert(0, "base")
    if any(doc["owner"] != "coolwsd" for doc in embedded):
        parts.append("engine-spdx")
    if any(doc["owner"] == "coolwsd" for doc in embedded):
        parts.append("online-spdx")
    completeness = "+".join(parts)

    arch = subprocess.run(["dpkg", "--print-architecture"], check=True,
                          stdout=subprocess.PIPE, text=True).stdout.strip()
    image_purl = (f"pkg:oci/{args.image_name}@{args.image_version}"
                  f"?arch={arch}")

    os.makedirs(args.out_dir, exist_ok=True)
    cdx = cyclonedx_document(image_purl, args.image_name, args.image_version,
                             base_sbom, args.base_ref, packages, embedded,
                             completeness)
    with open(os.path.join(args.out_dir, "collabora-online.cdx.json"), "w",
              encoding="utf-8") as f:
        json.dump(cdx, f, indent=2)
        f.write("\n")

    spdx = spdx_document(image_purl, args.image_name, args.image_version,
                         base_sbom, args.base_ref, packages, embedded,
                         completeness)
    with open(os.path.join(args.out_dir,
                           "collabora-online-image-sbom.spdx.json"), "w",
              encoding="utf-8") as f:
        json.dump(spdx, f, indent=2)
        f.write("\n")

    log(f"wrote {args.out_dir}/collabora-online.cdx.json and "
        f"collabora-online-image-sbom.spdx.json ({completeness})")


if __name__ == "__main__":
    main()
