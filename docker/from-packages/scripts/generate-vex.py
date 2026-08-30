#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Merge new scanner findings into the checked-in CycloneDX VEX document.
#
# The COOL container packages (coolwsd, collaboraoffice*) are published by
# Collabora, not Debian, so no distro security feed covers them; CVE discovery
# happens by scanning the image-level SBOM, whose embedded per-package SPDX
# documents identify the bundled upstream components (CPEs, purls). Collabora,
# as the supplier, then triages each finding here - this is the supplier-VEX
# scenario, and it is also how false positives are corrected downstream: the
# engine backports security fixes to bundled libraries without version bumps,
# so version-based matching flags CVEs that are in fact already fixed.
#
# Usage: generate-vex.py --vex docker/from-packages/vex/cool.vex.json \
#            --scan trivy-scan.json [--product <image purl>]
#
# New findings are appended with analysis state 'in_triage'; existing
# statements (any state) are never touched. Update the states by hand as
# triage concludes: affected / exploitable, fixed / resolved, not_affected
# (with justification), etc. - see the CycloneDX VEX analysis schema.

import argparse
import json
import sys
from datetime import datetime, timezone


def scan_findings(scan):
    """Extract (id, purl, description) from a trivy JSON report."""
    findings = []
    for result in scan.get("Results", []):
        for vulnerability in result.get("Vulnerabilities", []) or []:
            findings.append({
                "id": vulnerability.get("VulnerabilityID"),
                "purl": vulnerability.get("PkgIdentifier", {}).get("PURL")
                        or result.get("Target", ""),
                "description": vulnerability.get("Title")
                        or vulnerability.get("Description", "")[:200],
            })
    return findings


def main():
    parser = argparse.ArgumentParser(
        description="Merge scanner findings into the CycloneDX VEX document")
    parser.add_argument("--vex", required=True,
                        help="checked-in CycloneDX VEX document to update")
    parser.add_argument("--scan", required=True,
                        help="trivy JSON report of the image-level SBOM")
    parser.add_argument("--product",
                        help="product purl for new statements' affects")
    args = parser.parse_args()

    with open(args.vex, encoding="utf-8") as f:
        vex = json.load(f)
    with open(args.scan, encoding="utf-8") as f:
        scan = json.load(f)

    vulnerabilities = vex.setdefault("vulnerabilities", [])
    known = {v.get("id") for v in vulnerabilities}
    timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")

    added = 0
    for finding in scan_findings(scan):
        if not finding["id"] or finding["id"] in known:
            continue
        known.add(finding["id"])
        statement = {
            "id": finding["id"],
            "description": finding["description"],
            "analysis": {
                "state": "in_triage",
                "detail": f"Found by SBOM scan on {timestamp}; "
                          f"component: {finding['purl']}",
            },
        }
        if args.product:
            statement["affects"] = [{"ref": args.product}]
        vulnerabilities.append(statement)
        added += 1

    with open(args.vex, "w", encoding="utf-8") as f:
        json.dump(vex, f, indent=2)
        f.write("\n")
    print(f"generate-vex: {added} new finding(s) added as in_triage, "
          f"{len(vulnerabilities)} statement(s) total", file=sys.stderr)


if __name__ == "__main__":
    main()
