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
# Audit the cpe23 identifiers our SBOMs assert against a vulnerability
# database, because a CPE that matches nothing silently hides every CVE of
# that component.
#
# The identifiers come from hand-written metadata - the Vendor field of the
# license.xml annotations in the engine, scripts/sbom-externals.json here -
# and a scanner only reports what its database can match. A wrong vendor is
# worse than no vendor at all: an empty Vendor becomes a '*' wildcard, which
# matches every vendor of that product, while a misspelled one matches only
# itself. That is how 'Vendor: curl_project' hid 33 curl CVEs (including 6
# rated Critical) from the scan of an SBOM in which curl was present, named
# and correctly versioned.
#
# Two problems are reported:
#
#   WRONG-VENDOR   another vendor of the same product carries more CVEs than
#                  the vendor we assert - our identifier is under-matching
#   UNKNOWN-PRODUCT  no CPE in the database has this product name at all, so
#                  the wildcard cannot help either; either the name is
#                  misspelled or the component is not in the CPE dictionary
#                  (common for small libraries, hence a warning only)
#
# Usage:
#   scripts/audit-sbom-cpes.py [--db <grype vulnerability.db>] <sbom.spdx.json>...
#
# The database is grype's (~/.cache/grype/db/*/vulnerability.db, or wherever
# GRYPE_DB_CACHE_DIR points); run 'grype db update' first. Exits non-zero if
# any WRONG-VENDOR problem is found.

import argparse
import glob
import json
import os
import re
import sqlite3
import sys


def find_db():
    roots = [os.environ.get("GRYPE_DB_CACHE_DIR"),
             os.path.expanduser("~/.cache/grype/db")]
    for root in roots:
        if not root:
            continue
        found = sorted(glob.glob(os.path.join(root, "*", "vulnerability.db")))
        if found:
            return found[-1]
    return None


def collect_cpes(paths):
    """name -> set of (vendor, product) asserted for it, from SPDX 3.0.1.

    Components that also carry a packageURL are skipped: scanners match those
    by purl against the ecosystem advisory databases, so their CPE - which
    npm packages generally do not have at all - does not gate detection."""
    result = {}
    for path in paths:
        with open(path, encoding="utf-8") as f:
            document = json.load(f)
        for element in document.get("@graph", []):
            if element.get("type") != "software_Package":
                continue
            identifiers = element.get("externalIdentifiers", [])
            types = {i.get("externalIdentifierType") for i in identifiers}
            if "packageURL" in types:
                continue
            for identifier in identifiers:
                if identifier.get("externalIdentifierType") != "cpe23":
                    continue
                match = re.match(r"cpe:2\.3:a:([^:]*):([^:]*):",
                                 identifier["identifier"])
                if match:
                    result.setdefault(element.get("name"), set()).add(
                        (match.group(1), match.group(2)))
    return result


def main():
    parser = argparse.ArgumentParser(
        description="Audit SBOM cpe23 identifiers against a vulnerability DB")
    parser.add_argument("sboms", nargs="+")
    parser.add_argument("--db", default=None)
    parser.add_argument("--show-unknown", action="store_true",
                        help="list the UNKNOWN-PRODUCT components too; most "
                             "are small libraries the CPE dictionary simply "
                             "does not cover, so they are only counted by "
                             "default")
    args = parser.parse_args()

    database = args.db or find_db()
    if not database or not os.path.exists(database):
        print("audit-sbom-cpes: no grype vulnerability.db found; pass --db "
              "or run 'grype db update'", file=sys.stderr)
        sys.exit(2)
    db = sqlite3.connect(f"file:{database}?mode=ro", uri=True)

    def vendors_of(product):
        """vendor -> number of distinct CVEs, for one CPE product name."""
        rows = db.execute(
            "select c.vendor, count(distinct a.vulnerability_id) "
            "from affected_cpe_handles a join cpes c on c.id = a.cpe_id "
            "where c.product = ? group by c.vendor", (product,))
        return dict(rows)

    cpes = collect_cpes(args.sboms)
    wrong, unknown = [], []
    for name, pairs in sorted(cpes.items()):
        for vendor, product in sorted(pairs):
            if product in ("*", ""):
                continue
            vendors = vendors_of(product)
            if not vendors:
                unknown.append((name, vendor, product))
                continue
            ours = vendors.get(vendor, 0)
            if vendor == "*":
                continue  # wildcard matches every vendor of the product
            best, best_count = max(vendors.items(), key=lambda kv: kv[1])
            if best_count > ours:
                wrong.append((name, vendor, product, ours, best, best_count))

    for name, vendor, product, ours, best, best_count in wrong:
        print(f"WRONG-VENDOR   {name}: we assert {vendor}:{product} "
              f"({ours} CVEs), but {best}:{product} has {best_count}. "
              f"Use '{best}' or leave the Vendor empty.")
    if args.show_unknown:
        for name, vendor, product in unknown:
            print(f"UNKNOWN-PRODUCT {name}: no CPE has product '{product}' "
                  f"(vendor '{vendor}'); check the spelling against the CPE "
                  f"dictionary.")
    print(f"\n{len(cpes)} components, {len(wrong)} wrong vendor(s), "
          f"{len(unknown)} unknown product(s)")
    sys.exit(1 if wrong else 0)


if __name__ == "__main__":
    main()
