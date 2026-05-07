#!/usr/bin/env python3
"""Analyze officecfg .xcs schemas and .xcu data, rank config path stems.

Schemas (.xcs) define the universe of keys via <prop> nested in
<group>/<set>/<node-ref>; data files (.xcu/.xcu.tmpl) provide actual values
through <node oor:name=...>/<prop oor:name=...> hierarchies.

Outputs:
  <out_csv>           : path-stem, schema-keys, data-keys, data-xml-bytes
  <out_csv>.drop.txt  : top recommendations of subtrees to drop
"""

import argparse
import csv
import os
import sys
import xml.etree.ElementTree as ET
from collections import defaultdict

OOR = "http://openoffice.org/2001/registry"


def oor_attr(elem, name):
    return elem.get(f"{{{OOR}}}{name}") or elem.get(name)


def localtag(elem):
    t = elem.tag
    return t.split("}", 1)[1] if "}" in t else t


def collect_templates(xcs_files):
    templates = {}
    for path in xcs_files:
        try:
            tree = ET.parse(path)
        except ET.ParseError as e:
            print(f"warn: parse failed {path}: {e}", file=sys.stderr)
            continue
        root = tree.getroot()
        pkg = oor_attr(root, "package") or ""
        name = oor_attr(root, "name") or ""
        comp = f"{pkg}.{name}" if pkg else name
        defs = {}
        for tmpl_block in root.iter():
            if localtag(tmpl_block) != "templates":
                continue
            for child in tmpl_block:
                if localtag(child) in ("group", "set"):
                    tname = oor_attr(child, "name")
                    if tname:
                        defs[tname] = child
        templates[comp] = defs
    return templates


def lookup_template(elem, node_type, comp_full_name, templates):
    if not node_type:
        return None
    target_comp = oor_attr(elem, "component") or comp_full_name
    defs = templates.get(target_comp) or {}
    if node_type in defs:
        return defs[node_type]
    # Fallback: cross-component refs without an explicit oor:component.
    for comp_defs in templates.values():
        if node_type in comp_defs:
            return comp_defs[node_type]
    return None


def enumerate_props(component_root, comp_full_name, templates):
    base_path = comp_full_name.split(".") if comp_full_name else []
    visiting = set()  # cycle guard for recursive templates

    def walk(elem, path):
        for child in elem:
            lt = localtag(child)
            cname = oor_attr(child, "name")
            if lt == "prop":
                if cname:
                    yield path + [cname]
            elif lt == "group":
                if cname:
                    yield from walk(child, path + [cname])
            elif lt == "set":
                # "*" marks a set entry slot — one template instance per entry.
                new_path = path + ([cname] if cname else []) + ["*"]
                yield from walk(child, new_path)
                tmpl = lookup_template(child, oor_attr(child, "node-type"), comp_full_name, templates)
                if tmpl is not None and id(tmpl) not in visiting:
                    visiting.add(id(tmpl))
                    try:
                        yield from walk(tmpl, new_path)
                    finally:
                        visiting.discard(id(tmpl))
            elif lt == "node-ref":
                if cname:
                    new_path = path + [cname]
                    yield from walk(child, new_path)
                    tmpl = lookup_template(child, oor_attr(child, "node-type"), comp_full_name, templates)
                    if tmpl is not None and id(tmpl) not in visiting:
                        visiting.add(id(tmpl))
                        try:
                            yield from walk(tmpl, new_path)
                        finally:
                            visiting.discard(id(tmpl))

    yield from walk(component_root, base_path)


def enumerate_data_props(component_root, comp_full_name):
    # In .xcu data, hierarchy is <node oor:name=...> wrapping <prop oor:name=...>.
    base_path = comp_full_name.split(".") if comp_full_name else []

    def walk(elem, path):
        for child in elem:
            lt = localtag(child)
            cname = oor_attr(child, "name")
            if lt == "prop":
                if cname:
                    nbytes = len(ET.tostring(child, encoding="utf-8"))
                    yield path + [cname], nbytes
            elif lt == "node":
                if cname:
                    yield from walk(child, path + [cname])

    yield from walk(component_root, base_path)


def find_files(root, exts):
    out = []
    for dirpath, _dirnames, filenames in os.walk(root):
        for fn in filenames:
            if any(fn.endswith(e) for e in exts):
                out.append(os.path.join(dirpath, fn))
    out.sort()
    return out


def find_body(root, tag_names):
    for child in root:
        if localtag(child) in tag_names:
            return child
    return None


def parse_root(path):
    try:
        tree = ET.parse(path)
    except ET.ParseError:
        return None, "", ""
    root = tree.getroot()
    pkg = oor_attr(root, "package") or ""
    name = oor_attr(root, "name") or ""
    return root, pkg, name


def analyze(registry_root):
    schema_root = os.path.join(registry_root, "schema")
    data_root = os.path.join(registry_root, "data")
    xcs_files = find_files(schema_root, [".xcs"]) if os.path.isdir(schema_root) else []
    xcu_files = find_files(data_root, [".xcu", ".xcu.tmpl"]) if os.path.isdir(data_root) else []

    templates = collect_templates(xcs_files)

    schema_paths = []
    for path in xcs_files:
        root, pkg, name = parse_root(path)
        if root is None:
            continue
        comp = f"{pkg}.{name}" if pkg else name
        body = find_body(root, ("component",))
        if body is None:
            continue
        for full_path in enumerate_props(body, comp, templates):
            schema_paths.append(full_path)

    data_paths = []  # list of (full_path, nbytes)
    data_per_file = {}
    for path in xcu_files:
        root, pkg, name = parse_root(path)
        if root is None:
            continue
        comp = f"{pkg}.{name}" if pkg else name
        # .xcu root *is* the body (oor:component-data); .xcs has a <component> child.
        body = root if localtag(root) == "component-data" else find_body(root, ("component-data",))
        if body is None:
            continue
        n = 0
        for full_path, nbytes in enumerate_data_props(body, comp):
            data_paths.append((full_path, nbytes))
            n += 1
        data_per_file[path] = (comp, n, os.path.getsize(path))

    schema_stem = defaultdict(int)
    for full in schema_paths:
        for depth in range(1, len(full)):
            schema_stem[".".join(full[:depth])] += 1

    data_stem_keys = defaultdict(int)
    data_stem_bytes = defaultdict(int)
    for full, nbytes in data_paths:
        for depth in range(1, len(full)):
            stem = ".".join(full[:depth])
            data_stem_keys[stem] += 1
            data_stem_bytes[stem] += nbytes

    return {
        "schema_paths": schema_paths,
        "data_paths": data_paths,
        "schema_stem": schema_stem,
        "data_stem_keys": data_stem_keys,
        "data_stem_bytes": data_stem_bytes,
        "data_per_file": data_per_file,
        "n_xcs": len(xcs_files),
        "n_xcu": len(xcu_files),
    }


def write_csv(result, out_csv):
    stems = set(result["schema_stem"]) | set(result["data_stem_keys"])
    rows = []
    for s in stems:
        sk = result["schema_stem"].get(s, 0)
        dk = result["data_stem_keys"].get(s, 0)
        db = result["data_stem_bytes"].get(s, 0)
        rows.append((s, sk, dk, db))
    rows.sort(key=lambda r: (-r[1], -r[3], r[0]))
    with open(out_csv, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["path-stem", "schema-keys", "data-keys", "data-xml-bytes"])
        for r in rows:
            w.writerow(r)


def top_level_breakdown(result):
    # Component-level groupings live at depth 4: org.openoffice.<package>.<Component>.
    rows = []
    for stem, sk in result["schema_stem"].items():
        if stem.count(".") != 3 or not stem.startswith("org.openoffice."):
            continue
        dk = result["data_stem_keys"].get(stem, 0)
        db = result["data_stem_bytes"].get(stem, 0)
        rows.append((stem, sk, dk, db))
    # Also include data-only components (no schema entry at this depth).
    seen = {r[0] for r in rows}
    for stem, dk in result["data_stem_keys"].items():
        if stem in seen or stem.count(".") != 3 or not stem.startswith("org.openoffice."):
            continue
        rows.append((stem, 0, dk, result["data_stem_bytes"].get(stem, 0)))
    rows.sort(key=lambda r: (-r[1], -r[3], r[0]))
    return rows


def recommend_drops(result, top_n=20, min_keys=20, min_depth=5):
    """Pick deeper non-overlapping stems with >= min_keys; collapse "*" wildcards."""
    counts = dict(result["schema_stem"])
    candidates = []
    seen = set()
    for stem, n in counts.items():
        if n < min_keys or stem.count(".") + 1 < min_depth:
            continue
        segs = stem.split(".")
        while segs and segs[-1] == "*":
            segs.pop()
        if not segs:
            continue
        norm = ".".join(segs)
        if norm in seen:
            continue
        seen.add(norm)
        parent = norm.rsplit(".", 1)[0] if "." in norm else norm
        parent_n = counts.get(parent, n)
        dk = result["data_stem_keys"].get(norm, 0)
        db = result["data_stem_bytes"].get(norm, 0)
        candidates.append((norm, counts.get(norm, n), parent, parent_n, parent_n - n, dk, db))

    candidates.sort(key=lambda r: (-r[1], -r[6], -r[4], r[0]))

    chosen, chosen_paths = [], []
    for row in candidates:
        stem = row[0]
        if any(stem == c or stem.startswith(c + ".") or c.startswith(stem + ".") for c in chosen_paths):
            continue
        chosen.append(row)
        chosen_paths.append(stem)
        if len(chosen) >= top_n:
            break
    return chosen


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--root", default="engine/officecfg/registry",
                    help="Path to officecfg registry root (containing schema/ and data/)")
    ap.add_argument("--out", default="officecfg-stems.csv",
                    help="Output CSV path")
    ap.add_argument("--top", type=int, default=20,
                    help="How many drop recommendations to print")
    args = ap.parse_args()

    if not os.path.isdir(args.root):
        print(f"error: registry root not found: {args.root}", file=sys.stderr)
        sys.exit(2)

    result = analyze(args.root)
    total_schema = len(result["schema_paths"])
    total_data = len(result["data_paths"])
    total_data_bytes = sum(b for _p, b in result["data_paths"])
    total_xcu_filesize = sum(sz for _c, _n, sz in result["data_per_file"].values())

    write_csv(result, args.out)

    rec_path = args.out + ".drop.txt"
    with open(rec_path, "w") as f:
        f.write(f"Schema files (.xcs): {result['n_xcs']}; data files (.xcu/.xcu.tmpl): {result['n_xcu']}\n")
        f.write(f"Total schema leaf <prop> paths: {total_schema}\n")
        f.write(f"Total data leaf <prop> paths:   {total_data}\n")
        f.write(f"Total data <prop> XML bytes:    {total_data_bytes}\n")
        f.write(f"Total .xcu/.tmpl file bytes:    {total_xcu_filesize}\n\n")

        f.write("Component-level breakdown (org.openoffice.<package>.<Component>):\n\n")
        f.write(f"{'sch-keys':>9}  {'%sch':>5}  {'dat-keys':>9}  {'dat-bytes':>10}  path\n")
        f.write("-" * 78 + "\n")
        for stem, sk, dk, db in top_level_breakdown(result):
            pct = 100.0 * sk / total_schema if total_schema else 0.0
            f.write(f"{sk:>9}  {pct:>4.1f}%  {dk:>9}  {db:>10}  {stem}\n")
        f.write("\n")

        f.write("Top sub-feature drop candidates (depth >= 5, >= 20 schema keys, non-overlapping):\n\n")
        f.write(f"{'sch-keys':>9}  {'%sch':>5}  {'parent':>7}  {'dat-keys':>9}  {'dat-bytes':>10}  path\n")
        f.write("-" * 92 + "\n")
        for stem, n, _parent, parent_n, _sib, dk, db in recommend_drops(result, top_n=args.top):
            pct = 100.0 * n / total_schema if total_schema else 0.0
            f.write(f"{n:>9}  {pct:>4.1f}%  {parent_n:>7}  {dk:>9}  {db:>10}  {stem}\n")

    print(f"Scanned {result['n_xcs']} .xcs schemas and {result['n_xcu']} .xcu data files.")
    print(f"Schema leaf props: {total_schema}; data leaf props: {total_data}; data XML bytes: {total_data_bytes}.")
    print(f"Wrote per-stem CSV: {args.out}")
    print(f"Wrote drop recommendations: {rec_path}")


if __name__ == "__main__":
    main()
