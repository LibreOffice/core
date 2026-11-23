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
# Scan the engine and online SBOMs for known vulnerabilities and read the
# result against the VEX, so that what is left is what needs a decision.
#
# The scanners cannot read SPDX 3.0.1, so the component inventory is first
# lifted into a CycloneDX document with the same code that builds the
# container image's SBOM (docker/from-packages/scripts/generate-image-sbom.py).
# A scan here therefore sees the components the published image declares,
# minus the distribution packages, which only exist in the image.
#
# Both scanners run, as in publish.sh: grype matches the CPEs of the
# statically linked C libraries and the npm purls, trivy the npm purls from
# its own advisory data. Their findings are merged per vulnerability, and
# each one is classified against the VEX:
#
#   NEW         not in the VEX at all - nobody has looked at it yet
#   OPEN        in the VEX, but still in_triage (or exploitable)
#   SUPPRESSED  in the VEX as resolved, resolved_with_pedigree, not_affected
#               or false_positive
#   UNVERSIONED a finding on a component the SBOM gives no version: every CVE
#               of the product matches it, so the finding says nothing until
#               the SBOM is fixed - an SBOM problem, not a vulnerability
#   STALE-VEX   a VEX statement no finding refers to any more (informational:
#               expected after a bump, and a candidate for clean-up)
#   PLATFORM    a finding on a component the product does not ship but runs
#               on (a desktop product's runtime, base snap, FreeBSD package,
#               the operating system): fixed by updating the platform, or by
#               requiring a newer version of it
#
# One VEX covers all products. A statement whose reasoning holds for some of
# them only (a patch to a bundled library that another product takes from
# the system) names them in a collabora:platforms property: server for
# Collabora Online, windows, macos, flatpak, snap, freebsd for Collabora
# Office. A statement with "affects" applies only where one of its refs is
# the bom-ref or purl of the product or of a component of the scanned SBOM.
#
# Usage:
#   scripts/scan-sbom.py --vex <cool.vex.json> --out-dir <dir>
#       [--product-version V] [--require-trivy] <sbom.spdx.json>...
#   scripts/scan-sbom.py --vex <cool.vex.json> --out-dir <dir>
#       [--probe] --cdx <collabora-office.cdx.json>
#   scripts/scan-sbom.py --vex <cool.vex.json> --out-dir <dir>
#       [--probe] --installed
#
# --cdx scans the SBOM of a desktop product (Windows, macOS, flatpak, snap,
# FreeBSD) as it is, --installed finds that of the Collabora Office installed
# on this machine. --probe first replaces the versions of the platform
# components, which the SBOM records as the product was built, with those
# installed here, where they can be found out.
#
# grype uses its database as it is ('grype db update' first, as for
# 'make sbom-audit'); trivy refreshes its own. Exits 1 if any finding is NEW
# or OPEN, 2 if the scan itself could not be done; UNVERSIONED findings
# alone do not change the exit code, they are reported for the SBOM to be
# fixed.

import argparse
import importlib.util
import json
import os
import re
import shutil
import subprocess
import sys
import uuid
from datetime import datetime, timezone

SUPPRESSING_STATES = ("resolved", "resolved_with_pedigree", "not_affected",
                      "false_positive")
SEVERITIES = ("critical", "high", "medium", "low", "negligible", "unknown")


class ToolError(Exception):
    pass


def load_image_sbom_module():
    """generate-image-sbom.py, for its SPDX -> CycloneDX component lift."""
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir,
                        "docker", "from-packages", "scripts",
                        "generate-image-sbom.py")
    spec = importlib.util.spec_from_file_location("generate_image_sbom", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def embedded_documents(paths):
    """The input SBOMs in the form generate-image-sbom.py expects, with the
    owning package named the way find_embedded_sboms() names it."""
    documents = []
    for path in paths:
        basename = os.path.basename(path)
        owner = None
        if basename == "collabora-online-sbom.spdx.json":
            owner = "coolwsd"
        else:
            match = re.match(r"(.+)-sbom\.spdx\.json$", basename)
            if match:
                owner = match.group(1)
        documents.append({"path": path, "abspath": path, "owner": owner})
    return documents


def cyclonedx(components, version):
    return {
        "bomFormat": "CycloneDX",
        "specVersion": "1.6",
        "serialNumber": "urn:uuid:{}".format(uuid.uuid4()),
        "version": 1,
        "metadata": {
            "timestamp": datetime.now(timezone.utc).strftime(
                "%Y-%m-%dT%H:%M:%SZ"),
            "tools": {"components": [{
                "type": "application",
                "name": "scan-sbom.py",
                "supplier": {"name": "Collabora Productivity Ltd."},
            }]},
            "component": {
                "bom-ref": "collabora-online",
                "type": "application",
                "name": "collabora-online",
                "version": version or "unknown",
            },
        },
        "components": components,
    }


def run(command, env=None):
    try:
        result = subprocess.run(command, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE,
                                universal_newlines=True, env=env)
    except OSError as error:
        raise ToolError("cannot run {}: {}".format(command[0], error))
    if result.returncode != 0:
        raise ToolError("{} exited with {}:\n{}".format(
            " ".join(command), result.returncode, result.stderr.strip()))
    return result.stdout


def component(name, version):
    return "{}@{}".format(name, version) if version else \
        "{} (no version)".format(name)


def severity(value):
    value = (value or "unknown").lower()
    return value if value in SEVERITIES else "unknown"


def grype_findings(report):
    findings = []
    for match in report.get("matches", []):
        artifact = match.get("artifact", {})
        vulnerability = match.get("vulnerability", {})
        related = [r.get("id") for r in match.get("relatedVulnerabilities", [])
                   if r.get("id")]
        description = vulnerability.get("description") or ""
        for r in match.get("relatedVulnerabilities", []):
            if not description:
                description = r.get("description") or ""
        findings.append({
            "ids": [vulnerability.get("id")] + related,
            "severity": severity(vulnerability.get("severity")),
            "component": component(artifact.get("name"),
                                   artifact.get("version")),
            "unversioned": not artifact.get("version"),
            "fixed": (vulnerability.get("fix") or {}).get("versions") or [],
            "description": description,
            "scanner": "grype",
        })
    return findings


def trivy_findings(report):
    findings = []
    for result in report.get("Results", []) or []:
        for vulnerability in result.get("Vulnerabilities", []) or []:
            fixed = vulnerability.get("FixedVersion") or ""
            findings.append({
                "ids": [vulnerability.get("VulnerabilityID")]
                       + (vulnerability.get("VendorIDs") or []),
                "severity": severity(vulnerability.get("Severity")),
                "component": component(vulnerability.get("PkgName"),
                                       vulnerability.get("InstalledVersion")),
                "unversioned": not vulnerability.get("InstalledVersion"),
                "fixed": [v.strip() for v in fixed.split(",") if v.strip()],
                "description": vulnerability.get("Title")
                               or vulnerability.get("Description") or "",
                "scanner": "trivy",
            })
    return findings


def merge(findings):
    """One entry per vulnerability: a scanner may report a GHSA with the CVE
    as related while the other reports the CVE itself, so findings sharing
    any id are merged."""
    merged = []
    by_id = {}
    for finding in findings:
        ids = [i for i in finding["ids"] if i]
        if not ids:
            continue
        entry = None
        for i in ids:
            if i in by_id:
                entry = by_id[i]
                break
        if entry is None:
            entry = {"ids": [], "severity": "unknown", "components": [],
                     "fixed": [], "description": "", "scanners": [],
                     "unversioned": True}
            merged.append(entry)
        for i in ids:
            if i not in entry["ids"]:
                entry["ids"].append(i)
            by_id[i] = entry
        if SEVERITIES.index(finding["severity"]) < \
                SEVERITIES.index(entry["severity"]):
            entry["severity"] = finding["severity"]
        for key, value in (("components", [finding["component"]]),
                           ("fixed", finding["fixed"]),
                           ("scanners", [finding["scanner"]])):
            for item in value:
                if item not in entry[key]:
                    entry[key].append(item)
        if not entry["description"]:
            entry["description"] = finding["description"]
        # one versioned match is enough for the finding to mean something
        entry["unversioned"] = entry["unversioned"] and finding["unversioned"]
    for entry in merged:
        # name the vulnerability by its CVE when there is one
        cves = sorted(i for i in entry["ids"] if i.startswith("CVE-"))
        entry["id"] = cves[0] if cves else entry["ids"][0]
    return merged


def applies(statement, platform, scope):
    """Whether a VEX statement is about the product scanned."""
    for prop in statement.get("properties", []):
        if prop.get("name") == "collabora:platforms" and platform not in \
                [value.strip() for value in prop.get("value", "").split(",")]:
            return False
    affects = statement.get("affects")
    return not affects \
        or any(target.get("ref") in scope for target in affects)


def classify(merged, vex, platform, scope, platform_components):
    statements = {}
    for statement in vex.get("vulnerabilities", []):
        if statement.get("id") and applies(statement, platform, scope):
            statements.setdefault(statement["id"], statement)
    referenced = set()
    for entry in merged:
        matching = [statements[i] for i in entry["ids"] if i in statements]
        referenced.update(s["id"] for s in matching)
        if not matching:
            if entry["components"] and all(
                    name in platform_components
                    for name in entry["components"]):
                entry["class"] = "PLATFORM"
            else:
                entry["class"] = ("UNVERSIONED" if entry["unversioned"]
                                  else "NEW")
            entry["vex"] = None
            continue
        # if the CVE and its GHSA disagree, the open statement wins
        states = [(s.get("analysis") or {}).get("state") for s in matching]
        statement = matching[0]
        for s, state in zip(matching, states):
            if state not in SUPPRESSING_STATES:
                statement = s
                break
        analysis = statement.get("analysis") or {}
        entry["vex"] = {"id": statement["id"], "state": analysis.get("state"),
                        "detail": analysis.get("detail", "")}
        entry["class"] = ("SUPPRESSED"
                          if analysis.get("state") in SUPPRESSING_STATES
                          else "OPEN")
    stale = [{"id": s["id"],
              "state": (s.get("analysis") or {}).get("state")}
             for i, s in sorted(statements.items()) if i not in referenced]
    return stale


def one_line(text, limit):
    text = " ".join((text or "").split())
    return text if len(text) <= limit else text[:limit - 3] + "..."


def write_report(out_dir, merged, stale, header, components):
    lines = list(header)
    lines.append("")
    attention = [e for e in merged if e["class"] in ("NEW", "OPEN")]
    attention.sort(key=lambda e: (e["class"] != "NEW",
                                  SEVERITIES.index(e["severity"]), e["id"]))
    for entry in attention:
        if entry["class"] == "NEW":
            fix = ("fixed in " + ", ".join(entry["fixed"])
                   if entry["fixed"] else "no fixed version known")
            note = one_line(entry["description"], 120)
        else:
            fix = "VEX " + (entry["vex"]["state"] or "no state")
            note = one_line(entry["vex"]["detail"], 120)
        lines.append("{:<10} {:<20} {:<10} {} - {} [{}]".format(
            entry["class"], entry["id"], entry["severity"].capitalize(),
            ", ".join(entry["components"]), fix,
            "+".join(entry["scanners"])))
        if note:
            lines.append("{:<10} {}".format("", note))
    platform = [e for e in merged if e["class"] == "PLATFORM"]
    platform.sort(key=lambda e: (SEVERITIES.index(e["severity"]), e["id"]))
    if platform:
        lines.append("")
        lines.append("Vulnerabilities in what the product runs on but does not "
                     "ship; fixed by updating")
        lines.append("the platform, or by requiring a version of it that has "
                     "the fix:")
        for entry in platform:
            fix = ("fixed in " + ", ".join(entry["fixed"])
                   if entry["fixed"] else "no fixed version known")
            lines.append("{:<10} {:<20} {:<10} {} - {}".format(
                "PLATFORM", entry["id"], entry["severity"].capitalize(),
                ", ".join(entry["components"]), fix))
    unversioned = {}
    for entry in merged:
        if entry["class"] == "UNVERSIONED":
            for name in entry["components"]:
                unversioned[name] = unversioned.get(name, 0) + 1
    if unversioned:
        lines.append("")
        lines.append("Components without a version: every CVE of the product "
                     "matches them, so these")
        lines.append("findings are not assessed. Give the component a version "
                     "in the SBOM.")
        for name, count in sorted(unversioned.items()):
            lines.append("UNVERSIONED {} - {} finding(s)".format(name, count))
    if stale:
        lines.append("")
        lines.append("VEX statements no finding refers to any more:")
        for statement in stale:
            lines.append("STALE-VEX  {:<20} {}".format(statement["id"],
                                                       statement["state"]))
    suppressed = {}
    for entry in merged:
        if entry["class"] == "SUPPRESSED":
            state = entry["vex"]["state"]
            suppressed[state] = suppressed.get(state, 0) + 1
    new = sum(1 for e in merged if e["class"] == "NEW")
    open_ = sum(1 for e in merged if e["class"] == "OPEN")
    lines.append("")
    lines.append("{} components scanned, {} vulnerabilities: {} new, {} open, "
                 "{} in the platform, "
                 "{} on {} unversioned component(s), "
                 "{} suppressed by the VEX ({}), {} stale VEX statement(s)"
                 .format(components, len(merged), new, open_, len(platform),
                         sum(unversioned.values()), len(unversioned),
                         sum(suppressed.values()),
                         ", ".join("{} {}".format(n, s) for s, n
                                   in sorted(suppressed.items())) or "none",
                         len(stale)))
    text = "\n".join(lines) + "\n"
    with open(os.path.join(out_dir, "report.txt"), "w",
              encoding="utf-8") as f:
        f.write(text)
    with open(os.path.join(out_dir, "report.json"), "w",
              encoding="utf-8") as f:
        json.dump({"header": header, "findings": merged, "stale_vex": stale},
                  f, indent=2)
        f.write("\n")
    sys.stdout.write(text)
    return new + open_


# ---------------------------------------------------------------------------
# A desktop product installed on this machine
# ---------------------------------------------------------------------------

SBOM_NAME = "collabora-office.cdx.json"


def output(command):
    try:
        return subprocess.run(command, stdout=subprocess.PIPE,
                              stderr=subprocess.DEVNULL,
                              universal_newlines=True, check=True).stdout
    except (OSError, subprocess.CalledProcessError):
        return None


def installed_sbom():
    """Where the SBOM of the installed Collabora Office is: inside the
    product, where its packaging put it."""
    candidates = []
    location = output(["flatpak", "info", "--show-location",
                       "com.collaboraoffice.Office"])
    if location:
        candidates.append(os.path.join(location.strip(), "files", "share",
                                       "sbom", SBOM_NAME))
    candidates.append(os.path.join("/snap", "collabora-office", "current",
                                   "usr", "share", "sbom", SBOM_NAME))
    candidates.append(os.path.join("/usr", "local", "share",
                                   "collabora-office", "sbom", SBOM_NAME))
    for app in ("/Applications/Collabora Office.app",
                os.path.expanduser("~/Applications/Collabora Office.app")):
        candidates.append(os.path.join(app, "Contents", "Resources", "sbom",
                                       SBOM_NAME))
    if sys.platform == "win32":
        location = output(["powershell", "-NoProfile", "-Command",
                           "(Get-AppxPackage -Name "
                           "CollaboraProductivityLtd.CollaboraOffice*)"
                           ".InstallLocation"])
        for line in (location or "").splitlines():
            if line.strip():
                candidates.append(os.path.join(line.strip(), "sbom",
                                               SBOM_NAME))
    for path in candidates:
        if os.path.isfile(path):
            return path
    return None


def platform_version(component, platform):
    """The version of a platform component installed here, None if it
    cannot be found out."""
    name = component["name"]
    if platform == "freebsd":
        if component.get("type") == "operating-system":
            return (output(["freebsd-version", "-u"]) or "").strip() or None
        return (output(["pkg", "query", "%v", name]) or "").strip() or None
    if platform == "snap":
        for line in (output(["snap", "list", name]) or "").splitlines()[1:]:
            fields = line.split()
            if fields and fields[0] == name:
                return fields[1]
        return None
    if platform == "flatpak" and component.get("type") == "platform":
        info = output(["flatpak", "info", name + "//" + component["version"]])
        match = re.search(r"^\s*Version:\s*(\S+)", info or "", re.M)
        return match.group(1) if match else None
    if platform == "macos" and component.get("type") == "operating-system":
        return (output(["sw_vers", "-productVersion"]) or "").strip() or None
    if platform == "windows":
        if component.get("type") == "operating-system":
            return (output(["powershell", "-NoProfile", "-Command",
                            "[Environment]::OSVersion.Version.ToString()"])
                    or "").strip() or None
        if name == "Microsoft Edge WebView2 Runtime":
            key = ("HKLM:\\SOFTWARE\\WOW6432Node\\Microsoft\\EdgeUpdate\\"
                   "Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}")
            return (output(["powershell", "-NoProfile", "-Command",
                            "(Get-ItemProperty '" + key + "').pv"])
                    or "").strip() or None
        return (output(["powershell", "-NoProfile", "-Command",
                        "(Get-AppxPackage -Name " + name + " | Sort-Object "
                        "Version | Select-Object -Last 1).Version"])
                or "").strip() or None
    return None


def probe_platform(document):
    """Replace the build-time versions of the platform components with the
    ones installed here; a CPE carries the version too."""
    product = document["metadata"]["component"]
    platform = next((p["value"] for p in product.get("properties", [])
                     if p["name"] == "collabora:platform"), None)
    for c in document.get("components", []):
        if not any(p["name"] == "collabora:layer" and p["value"] == "platform"
                   for p in c.get("properties", [])):
            continue
        version = platform_version(c, platform)
        if not version or version == c.get("version"):
            continue
        print("{}: {} here, {} at build time".format(
            c["name"], version, c.get("version")))
        if c.get("cpe") and c.get("version"):
            parts = c["cpe"].split(":")
            if len(parts) > 5 and parts[5] == c["version"]:
                parts[5] = version
                c["cpe"] = ":".join(parts)
        c["version"] = version
        for p in c["properties"]:
            if p["name"] == "collabora:version-basis":
                p["value"] = "installed"



def scan(args):
    os.makedirs(args.out_dir, exist_ok=True)
    for name in ("sbom.cdx.json", "grype.json", "trivy.json", "report.txt",
                 "report.json"):
        path = os.path.join(args.out_dir, name)
        if os.path.exists(path):
            os.remove(path)

    try:
        with open(args.vex, encoding="utf-8") as f:
            vex = json.load(f)
        if args.cdx:
            # a desktop product's SBOM, scanned as it is
            with open(args.cdx, encoding="utf-8") as f:
                document = json.load(f)
            if document.get("bomFormat") != "CycloneDX":
                raise ToolError("not a CycloneDX document: " + args.cdx)
            if args.probe:
                probe_platform(document)
            sboms = [args.cdx]
        else:
            for path in args.sboms:
                if not os.path.isfile(path):
                    raise ToolError("no such SBOM: {}".format(path))
            lift = load_image_sbom_module().lift_embedded_components
            document = cyclonedx(lift(embedded_documents(args.sboms)),
                                 args.product_version)
            sboms = args.sboms
    except (OSError, ValueError) as error:
        raise ToolError(str(error))
    components = document.get("components", [])
    if not components:
        raise ToolError("no components with a CPE or purl in the input SBOMs")
    cdx_path = os.path.join(args.out_dir, "sbom.cdx.json")
    with open(cdx_path, "w", encoding="utf-8") as f:
        json.dump(document, f, indent=2)
        f.write("\n")

    # what VEX statements with "affects" are matched against
    product = document["metadata"]["component"]
    platform = "server"
    for prop in product.get("properties", []):
        if prop["name"] == "collabora:platform":
            platform = prop["value"]
    scope = {product.get("bom-ref"), product.get("purl")}
    platform_components = set()
    for c in components:
        scope.update((c.get("bom-ref"), c.get("purl")))
        if any(p["name"] == "collabora:layer" and p["value"] == "platform"
               for p in c.get("properties", [])):
            platform_components.add(component(c.get("name"), c.get("version")))
    scope.discard(None)

    names = [os.path.basename(p) for p in sboms]
    header = ["SBOM scan of {} ({} components: {} with a CPE, {} with a purl)"
              .format(", ".join(names) if len(names) <= 3
                      else "{} SBOM documents".format(len(names)),
                      len(components),
                      sum(1 for c in components if "cpe" in c),
                      sum(1 for c in components if "purl" in c))]

    grype = shutil.which(args.grype)
    if not grype:
        raise ToolError("grype not found; install it and run "
                        "'grype db update'")
    env = dict(os.environ, GRYPE_DB_AUTO_UPDATE="false",
               GRYPE_CHECK_FOR_APP_UPDATE="false")
    version = re.search(r"^Version:\s*(\S+)", run([grype, "version"], env),
                        re.M)
    status = run([grype, "db", "status"], env)
    built = re.search(r"^Built:\s*(.+)$", status, re.M)
    header.append("grype {}, database built {}".format(
        version.group(1) if version else "(unknown version)",
        built.group(1).strip() if built else "(unknown)"))
    report = run([grype, "-q", "sbom:" + cdx_path, "-o", "json"], env)
    with open(os.path.join(args.out_dir, "grype.json"), "w",
              encoding="utf-8") as f:
        f.write(report)
    findings = grype_findings(json.loads(report))

    trivy = shutil.which(args.trivy)
    if trivy:
        trivy_path = os.path.join(args.out_dir, "trivy.json")
        run([trivy, "sbom", "--quiet", "--format", "json", "--output",
             trivy_path, cdx_path])
        with open(trivy_path, encoding="utf-8") as f:
            trivy_report = json.load(f)
        # asked after the scan, which may have refreshed the database
        status = run([trivy, "--version"])
        version = re.search(r"^Version:\s*(\S+)", status, re.M)
        updated = re.search(r"^\s+UpdatedAt:\s*(.+)$", status, re.M)
        header.append("trivy {}, database updated {}".format(
            version.group(1) if version else "(unknown version)",
            updated.group(1).strip() if updated else "(unknown)"))
        findings += trivy_findings(trivy_report)
    elif args.require_trivy:
        raise ToolError("trivy not found, and --require-trivy is given")
    else:
        header.append("trivy: skipped, not installed (grype-only scan)")

    merged = merge(findings)
    stale = classify(merged, vex, platform, scope, platform_components)
    return write_report(args.out_dir, merged, stale, header, len(components))


def main():
    parser = argparse.ArgumentParser(
        description="Scan the SBOMs and read the findings against the VEX")
    parser.add_argument("sboms", nargs="*")
    parser.add_argument("--cdx",
                        help="scan this CycloneDX SBOM of a desktop product")
    parser.add_argument("--installed", action="store_true",
                        help="scan the SBOM of the Collabora Office installed "
                             "on this machine")
    parser.add_argument("--probe", action="store_true",
                        help="with --cdx or --installed: use the versions of "
                             "the platform components installed here")
    parser.add_argument("--vex", required=True,
                        help="CycloneDX VEX document "
                             "(docker/from-packages/vex/cool.vex.json)")
    parser.add_argument("--out-dir", required=True,
                        help="where sbom.cdx.json, the scanner reports and "
                             "report.txt / report.json go")
    parser.add_argument("--product-version", default=None)
    parser.add_argument("--grype", default="grype")
    parser.add_argument("--trivy", default="trivy")
    parser.add_argument("--require-trivy", action="store_true",
                        help="fail instead of scanning with grype alone "
                             "when trivy is not installed")
    args = parser.parse_args()
    if args.installed:
        args.cdx = installed_sbom()
        if args.cdx is None:
            print("scan-sbom: no installed Collabora Office with an SBOM found",
                  file=sys.stderr)
            sys.exit(2)
        print("scanning " + args.cdx)
    if bool(args.cdx) == bool(args.sboms):
        parser.error("give either SPDX documents, or --cdx, or --installed")

    try:
        attention = scan(args)
    except ToolError as error:
        print("scan-sbom: {}".format(error), file=sys.stderr)
        sys.exit(2)
    sys.exit(1 if attention else 0)


if __name__ == "__main__":
    main()
