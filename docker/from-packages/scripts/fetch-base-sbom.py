#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Obtain the CycloneDX SBOM of the runtime base image, for merging into the
# image-level SBOM by generate-image-sbom.py. Runs in the 'sbom' stage of the
# from-packages Dockerfile, standard library only.
#
# ZenDiS / openCode publish the base image SBOM as a cosign attestation: an
# OCI manifest tagged sha256-<platform-manifest-digest>.att whose layers are
# DSSE envelopes, one of them with the https://cyclonedx.org/bom predicate.
# The base filesystem itself carries no package metadata (it is nix-built),
# so the SBOM must come from the registry - or, for other bases, from their
# dpkg database (--mode probe), or from a file supplied in the build context
# (--mode file, for air-gapped builds).
#
# The DSSE signature is not verified here (that would need the publisher's
# key and a signature implementation); integrity rests on TLS plus the
# digests recorded in the output. Full verification belongs in the consumer's
# 'cosign verify-attestation' flow, documented in docker/README.

import argparse
import base64
import glob
import json
import re
import subprocess
import sys
import urllib.error
import urllib.parse
import urllib.request

PREDICATE_CYCLONEDX = "https://cyclonedx.org/bom"

MANIFEST_TYPES = ", ".join([
    "application/vnd.oci.image.index.v1+json",
    "application/vnd.docker.distribution.manifest.list.v2+json",
    "application/vnd.oci.image.manifest.v1+json",
    "application/vnd.docker.distribution.manifest.v2+json",
])


def log(message):
    print(f"fetch-base-sbom: {message}", file=sys.stderr)


def build_architecture():
    try:
        return subprocess.run(["dpkg", "--print-architecture"], check=True,
                              stdout=subprocess.PIPE, text=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        import platform
        machine = platform.machine()
        return {"x86_64": "amd64", "aarch64": "arm64",
                "ppc64le": "ppc64el"}.get(machine, machine)


def parse_reference(ref):
    """Split an image reference into (registry, repository, tag)."""
    if "@" in ref:
        raise Exception(f"digest references not supported: {ref}")
    first, _, rest = ref.partition("/")
    if "." not in first and ":" not in first and first != "localhost":
        # dockerhub-style short name
        first, rest = "registry-1.docker.io", ref
    repository, _, tag = rest.rpartition(":")
    if not repository or "/" in tag:
        repository, tag = rest, "latest"
    return first, repository, tag


class Registry:
    def __init__(self, registry, repository):
        self.base = f"https://{registry}/v2/{repository}"
        self.repository = repository
        self.token = None
        self._authenticate(registry)

    def _authenticate(self, registry):
        try:
            urllib.request.urlopen(f"https://{registry}/v2/", timeout=30)
            return  # no auth required
        except urllib.error.HTTPError as error:
            if error.code != 401:
                raise
            challenge = error.headers.get("WWW-Authenticate", "")
        params = dict(re.findall(r'(\w+)="([^"]*)"', challenge))
        if "realm" not in params:
            raise Exception(f"unsupported auth challenge: {challenge}")
        query = {"scope": f"repository:{self.repository}:pull"}
        if "service" in params:
            query["service"] = params["service"]
        url = params["realm"] + "?" + urllib.parse.urlencode(query)
        with urllib.request.urlopen(url, timeout=30) as response:
            self.token = json.load(response).get("token")

    def get(self, path, accept):
        request = urllib.request.Request(self.base + path)
        request.add_header("Accept", accept)
        if self.token:
            request.add_header("Authorization", f"Bearer {self.token}")
        with urllib.request.urlopen(request, timeout=60) as response:
            return response.read(), dict(response.headers)

    def manifest(self, reference):
        data, headers = self.get(f"/manifests/{reference}", MANIFEST_TYPES)
        return json.loads(data), headers.get("Docker-Content-Digest")

    def blob(self, digest):
        data, _ = self.get(f"/blobs/{digest}", "application/octet-stream")
        return data


def fetch_attested_sbom(ref):
    registry, repository, tag = parse_reference(ref)
    client = Registry(registry, repository)
    manifest, digest = client.manifest(tag)

    if "manifests" in manifest:  # multi-arch index: pick this build's platform
        arch = build_architecture()
        entries = [m for m in manifest["manifests"]
                   if m.get("platform", {}).get("architecture") == arch
                   and m.get("platform", {}).get("os") != "unknown"]
        if not entries:
            raise Exception(f"no {arch} manifest in index of {ref}")
        digest = entries[0]["digest"]
    if not digest:
        raise Exception(f"no content digest for {ref}")

    att_tag = "sha256-" + digest.split(":", 1)[1] + ".att"
    attestation, _ = client.manifest(att_tag)
    for layer in attestation.get("layers", []):
        if layer.get("mediaType") != "application/vnd.dsse.envelope.v1+json":
            continue
        envelope = json.loads(client.blob(layer["digest"]))
        statement = json.loads(base64.b64decode(envelope["payload"]))
        if statement.get("predicateType") == PREDICATE_CYCLONEDX:
            sbom = statement["predicate"]
            properties = sbom.setdefault("properties", [])
            properties.append({"name": "collabora:base-image-digest",
                               "value": digest})
            properties.append({"name": "collabora:base-attestation-layer",
                               "value": layer["digest"]})
            log(f"fetched CycloneDX attestation of {ref} ({digest})")
            return sbom
    raise Exception(f"no CycloneDX predicate in attestation {att_tag} of {ref}")


def parse_dpkg_status(text):
    packages = []
    for stanza in text.split("\n\n"):
        fields = {}
        for line in stanza.split("\n"):
            if line[:1] not in ("", " ", "\t") and ":" in line:
                key, _, value = line.partition(":")
                fields[key] = value.strip()
        if "Package" in fields and "Version" in fields:
            packages.append(fields)
    return packages


def probe_base_filesystem(hardened_root):
    """Synthesize a minimal SBOM from the base's dpkg metadata, for base
    images that have one (debian:stable-slim, gcr.io distroless)."""
    stanzas = []
    status = f"{hardened_root}/var/lib/dpkg/status"
    status_d = f"{hardened_root}/var/lib/dpkg/status.d"
    try:
        with open(status, encoding="utf-8") as f:
            stanzas = parse_dpkg_status(f.read())
    except FileNotFoundError:
        for path in sorted(glob.glob(f"{status_d}/*")):
            if path.endswith(".md5sums"):
                continue
            with open(path, encoding="utf-8") as f:
                stanzas.extend(parse_dpkg_status(f.read()))
    if not stanzas:
        raise Exception(f"no dpkg metadata under {hardened_root}")

    arch = build_architecture()
    components = []
    for fields in stanzas:
        name, version = fields["Package"], fields["Version"]
        purl = (f"pkg:deb/debian/{name}@{urllib.parse.quote(version)}"
                f"?arch={fields.get('Architecture', arch)}")
        components.append({
            "bom-ref": purl,
            "type": "library",
            "name": name,
            "version": version,
            "purl": purl,
        })
    log(f"probed {len(components)} packages from the base filesystem")
    return {
        "bomFormat": "CycloneDX",
        "specVersion": "1.6",
        "version": 1,
        "components": components,
        "properties": [{"name": "collabora:base-sbom-status",
                        "value": "probed from the base image dpkg metadata"}],
    }


def stub(reason):
    log(f"no base SBOM: {reason}")
    return {
        "bomFormat": "CycloneDX",
        "specVersion": "1.6",
        "version": 1,
        "components": [],
        "properties": [{"name": "collabora:base-sbom-status", "value": reason}],
    }


def main():
    parser = argparse.ArgumentParser(
        description="Obtain the CycloneDX SBOM of the runtime base image")
    parser.add_argument("--ref", required=True, help="base image reference")
    parser.add_argument("--mode", default="fetch",
                        choices=["fetch", "file", "probe", "none"])
    parser.add_argument("--context-dir", default="/tmp/sbom-ctx",
                        help="directory searched for base-sbom*.cdx.json")
    parser.add_argument("--hardened-root", default="/hardened")
    parser.add_argument("--strict", action="store_true",
                        help="fail instead of degrading to a stub")
    parser.add_argument("--out", required=True)
    args = parser.parse_args()

    sbom = None
    errors = []
    if args.mode == "file":
        supplied = sorted(glob.glob(f"{args.context_dir}/base-sbom*.cdx.json"))
        if supplied:
            with open(supplied[0], encoding="utf-8") as f:
                sbom = json.load(f)
            log(f"using context-supplied base SBOM {supplied[0]}")
        else:
            errors.append("no base-sbom*.cdx.json in the build context")
    elif args.mode == "fetch":
        try:
            sbom = fetch_attested_sbom(args.ref)
        except Exception as error:
            errors.append(f"fetch: {error}")
            try:
                sbom = probe_base_filesystem(args.hardened_root)
            except Exception as probe_error:
                errors.append(f"probe: {probe_error}")
    elif args.mode == "probe":
        try:
            sbom = probe_base_filesystem(args.hardened_root)
        except Exception as error:
            errors.append(f"probe: {error}")
    else:
        errors.append("base SBOM disabled (sbom_base=none)")

    if sbom is None:
        if args.strict and args.mode != "none":
            for error in errors:
                log(f"ERROR: {error}")
            sys.exit(1)
        sbom = stub("; ".join(errors))

    with open(args.out, "w", encoding="utf-8") as f:
        json.dump(sbom, f, indent=2)
        f.write("\n")


if __name__ == "__main__":
    main()
