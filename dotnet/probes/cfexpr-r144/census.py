#!/usr/bin/env python3
"""Census of OOXML `<cfRule type="expression">` formulas in the corpus.

Walks every OPC spreadsheet under the corpus root (by extension, case-insensitively,
and by sniffing `PK` for files whose extension lies), collects every expression rule
from both the SpreadsheetML `conditionalFormatting` blocks and the `x14` rules in a
worksheet's `extLst`, classifies each formula against a faithful port of
`SheetConditions.Comparison.Parse`, and reports the token inventory of the refused
ones, with and without defined-name expansion.

Outputs (written next to this script):
  rules.tsv              one row per expression rule
  functions.tsv          function-name inventory, un-expanded and expanded
  names.tsv              defined names referenced, with their expansion
  operators.tsv          operator inventory
  reference-forms.tsv    reference-form inventory
  by-cells.tsv           refused rules ranked by covered cell count
  by-document.tsv        per-document totals
"""

from __future__ import annotations

import csv
import io
import os
import re
import sys
import zipfile
from collections import Counter, defaultdict

CORPUS = "/home/user/sample-files"
OUT = os.path.dirname(os.path.abspath(__file__))

import xml.etree.ElementTree as ET

NS_MAIN = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
NS_REL = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
NS_PKGREL = "http://schemas.openxmlformats.org/package/2006/relationships"
NS_X14 = "http://schemas.microsoft.com/office/spreadsheetml/2009/9/main"
NS_XM = "http://schemas.microsoft.com/office/excel/2006/main"

M = lambda t: f"{{{NS_MAIN}}}{t}"


# --------------------------------------------------------------------------
# Port of SheetConditions.cs (read 2026-09-17):
#   Operand.Parse            lines 119-143
#   Operand.Reference        lines 170-203
#   Comparison.Parse         lines 211-229
#   Comparison.Operators     line  243
#   Comparison.IndexOfOperator lines 254-276
# and the `expression` arm of XlsxConditionalStyles.ConditionOf, lines 293-294
# (`formulas.Count == 1 ? Comparison.Parse(formulas[0]) : null`).
# --------------------------------------------------------------------------

OPERATORS = ["<>", "<=", ">=", "=", "<", ">"]

_CS_NUMBER = re.compile(r"^[+-]?(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?$")


def cs_try_parse_double(text: str) -> bool:
    """`double.TryParse(one, NumberStyles.Float, InvariantCulture, out _)`."""
    t = text.strip()
    if not t:
        return False
    if t.lstrip("+-").lower() in ("infinity", "nan"):
        return True
    return bool(_CS_NUMBER.match(t))


def parse_reference(text: str):
    """`SheetConditions.Operand.Reference` — returns a form tag or None."""
    if "!" in text:
        return None
    at = 0
    absolute_column = False
    if text[at] == "$":
        absolute_column = True
        at += 1
    letters = at
    while letters < len(text) and (("a" <= text[letters] <= "z") or ("A" <= text[letters] <= "Z")):
        letters += 1
    if letters == at or letters - at > 3:
        return None
    digits_start = letters
    absolute_row = False
    if digits_start < len(text) and text[digits_start] == "$":
        absolute_row = True
        digits_start += 1
    if digits_start >= len(text):
        return None
    for i in range(digits_start, len(text)):
        if not text[i].isdigit() or not text[i].isascii():
            return None
    try:
        row = int(text[digits_start:])
    except ValueError:
        return None
    if row < 1:
        return None
    return (absolute_row, absolute_column)


def parse_operand(text):
    """`SheetConditions.Operand.Parse` — True when it yields an operand."""
    if text is None or not text.strip():
        return False
    one = text.strip()
    if one[0] == '"':
        if len(one) < 2 or one[-1] != '"':
            return False
        body = one[1:-1].replace('""', '"')
        if '"' in body:
            return False
        return True
    if cs_try_parse_double(one):
        return True
    if one.upper() in ("TRUE", "FALSE"):
        return True
    return parse_reference(one) is not None


def index_of_operator(text: str, op: str) -> int:
    depth = 0
    quoted = False
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c == '"':
            quoted = not quoted
            i += 1
            continue
        if quoted:
            i += 1
            continue
        if c == "(":
            depth += 1
            i += 1
            continue
        if c == ")":
            depth -= 1
            i += 1
            continue
        if depth != 0:
            i += 1
            continue
        if i + len(op) <= n and text[i:i + len(op)] == op:
            return i
        i += 1
    return -1


def comparison_parse(formula):
    """`SheetConditions.Comparison.Parse` — True when the tree evaluates the rule."""
    if formula is None or not formula.strip():
        return False
    text = formula.strip()
    if text.startswith("="):
        text = text[1:]
    for op in OPERATORS:
        at = index_of_operator(text, op)
        if at < 0:
            continue
        if not parse_operand(text[:at]):
            return False
        if not parse_operand(text[at + len(op):]):
            return False
        return True
    return False


def accepted(formulas) -> bool:
    """The `expression` arm of `ConditionOf`."""
    return len(formulas) == 1 and comparison_parse(formulas[0])


# --------------------------------------------------------------------------
# A1 helpers
# --------------------------------------------------------------------------

CELL_RE = re.compile(r"^\$?([A-Za-z]{1,3})\$?(\d+)$")


def col_to_num(letters: str) -> int:
    n = 0
    for ch in letters.upper():
        n = n * 26 + (ord(ch) - 64)
    return n


def parse_a1(one: str):
    one = one.replace("$", "")
    m = re.match(r"^([A-Za-z]{1,3})(\d+)$", one)
    if m:
        return col_to_num(m.group(1)), int(m.group(2))
    return None


def parse_range(part: str):
    """Returns (c1, r1, c2, r2) 1-based inclusive, or None."""
    part = part.strip().replace("$", "")
    if "!" in part:
        part = part.split("!")[-1]
    if ":" in part:
        a, b = part.split(":", 1)
        pa, pb = parse_a1(a), parse_a1(b)
        if pa and pb:
            return (min(pa[0], pb[0]), min(pa[1], pb[1]), max(pa[0], pb[0]), max(pa[1], pb[1]))
        # whole-column A:D or whole-row 3:9
        if re.match(r"^[A-Za-z]{1,3}$", a) and re.match(r"^[A-Za-z]{1,3}$", b):
            return (col_to_num(a), 1, col_to_num(b), 1048576)
        if a.isdigit() and b.isdigit():
            return (1, int(a), 16384, int(b))
        return None
    p = parse_a1(part)
    if p:
        return (p[0], p[1], p[0], p[1])
    return None


def parse_sqref(sqref):
    out = []
    if not sqref:
        return out
    for part in sqref.split():
        r = parse_range(part)
        if r:
            out.append(r)
    return out


def area(ranges):
    return sum((c2 - c1 + 1) * (r2 - r1 + 1) for c1, r1, c2, r2 in ranges)


def intersects(ranges, box):
    if box is None:
        return False
    bc1, br1, bc2, br2 = box
    for c1, r1, c2, r2 in ranges:
        if c1 <= bc2 and c2 >= bc1 and r1 <= br2 and r2 >= br1:
            return True
    return False


# --------------------------------------------------------------------------
# Formula tokenising
# --------------------------------------------------------------------------

STRING_RE = re.compile(r'"(?:[^"]|"")*"')
SHEETREF_RE = re.compile(
    r"(?:'[^']+'|[A-Za-z_][A-Za-z0-9_.]*)!\$?[A-Za-z]{1,3}\$?\d+(?::\$?[A-Za-z]{1,3}\$?\d+)?")
RANGE_RE = re.compile(r"\$?[A-Za-z]{1,3}\$?\d+:\$?[A-Za-z]{1,3}\$?\d+")
COLRANGE_RE = re.compile(r"\$?[A-Za-z]{1,3}:\$?[A-Za-z]{1,3}(?![A-Za-z0-9$])")
CELL_TOKEN_RE = re.compile(r"\$?[A-Za-z]{1,3}\$?\d+(?![A-Za-z0-9_.(])")
FUNC_RE = re.compile(r"([A-Za-z_][A-Za-z0-9_.]*)\s*\(")
NAME_RE = re.compile(r"(?<![A-Za-z0-9_.$!'#])([A-Za-z_\\][A-Za-z0-9_.\\]*)(?![A-Za-z0-9_.(])")
OP_RE = re.compile(r"<>|<=|>=|=|<|>|\+|-|\*|/|\^|&|%")

BUILTIN_WORDS = {"TRUE", "FALSE"}


def tokenise(formula: str):
    """Returns (functions, names, operators, ref_forms, has_error_literal, has_structured)."""
    text = formula.strip()
    if text.startswith("="):
        text = text[1:]

    funcs = Counter()
    for m in FUNC_RE.finditer(text):
        funcs[m.group(1).upper()] += 1

    # blank out strings before anything else
    masked = STRING_RE.sub(lambda m: " " * len(m.group(0)), text)

    ref_forms = Counter()
    errs = set(re.findall(r"#(?:REF|DIV/0|VALUE|NAME\?|N/A|NULL|NUM)!?", masked))
    structured = bool(re.search(r"\[[^\]]*\]", masked))

    work = masked
    for m in SHEETREF_RE.finditer(work):
        ref_forms["Sheet!A1"] += 1
    work = SHEETREF_RE.sub(lambda m: " " * len(m.group(0)), work)

    for m in RANGE_RE.finditer(work):
        ref_forms["range A1:B2"] += 1
    work = RANGE_RE.sub(lambda m: " " * len(m.group(0)), work)

    for m in COLRANGE_RE.finditer(work):
        ref_forms["whole column A:B"] += 1
    work = COLRANGE_RE.sub(lambda m: " " * len(m.group(0)), work)

    for m in CELL_TOKEN_RE.finditer(work):
        tok = m.group(0)
        dollar_col = tok.startswith("$")
        body = tok[1:] if dollar_col else tok
        dollar_row = "$" in body
        if dollar_col and dollar_row:
            ref_forms["$A$1"] += 1
        elif dollar_col:
            ref_forms["$A1"] += 1
        elif dollar_row:
            ref_forms["A$1"] += 1
        else:
            ref_forms["A1"] += 1
    work = CELL_TOKEN_RE.sub(lambda m: " " * len(m.group(0)), work)

    # remove function names so they are not also counted as defined names
    work2 = FUNC_RE.sub(lambda m: " " * len(m.group(0)), work)
    names = Counter()
    for m in NAME_RE.finditer(work2):
        tok = m.group(1)
        if tok.upper() in BUILTIN_WORDS:
            continue
        names[tok] += 1

    # operators: also blank single-quoted sheet names, whose text can contain `&`
    op_source = re.sub(r"'[^']*'", lambda m: " " * len(m.group(0)), masked)
    ops = Counter()
    for m in OP_RE.finditer(op_source):
        ops[m.group(0)] += 1

    return funcs, names, ops, ref_forms, errs, structured


# --------------------------------------------------------------------------
# Workbook reading
# --------------------------------------------------------------------------

def is_opc_spreadsheet(path):
    try:
        with open(path, "rb") as fh:
            if fh.read(2) != b"PK":
                return False
    except OSError:
        return False
    try:
        with zipfile.ZipFile(path) as z:
            names = set(z.namelist())
        return "xl/workbook.xml" in names
    except Exception:
        return False


def candidates():
    hits = []
    for root, _dirs, files in os.walk(CORPUS):
        for f in files:
            p = os.path.join(root, f)
            ext = os.path.splitext(f)[1].lower()
            if ext in (".xlsx", ".xlsm", ".xltx", ".xltm", ".xlam"):
                if is_opc_spreadsheet(p):
                    hits.append((p, "extension"))
            elif ext in (".xls", ".xlsb", ".zip", ""):
                if is_opc_spreadsheet(p):
                    hits.append((p, "sniffed"))
    return sorted(hits)


def rels_for(z, part):
    d, base = os.path.split(part)
    relpath = f"{d}/_rels/{base}.rels" if d else f"_rels/{base}.rels"
    out = {}
    try:
        data = z.read(relpath)
    except KeyError:
        return out
    try:
        root = ET.fromstring(data)
    except ET.ParseError:
        return out
    for rel in root:
        out[rel.get("Id")] = (rel.get("Target"), rel.get("TargetMode"))
    return out


def resolve(base_part, target):
    if target.startswith("/"):
        return target.lstrip("/")
    d = os.path.dirname(base_part)
    return os.path.normpath(os.path.join(d, target)).replace("\\", "/")


def dxf_paints(dxf):
    """Mirrors ReadDifferences/ReadFill: a text change or a resolved fill colour."""
    if dxf is None:
        return False
    font = dxf.find(M("font"))
    if font is not None and len(font) > 0:
        return True
    fill = dxf.find(M("fill"))
    if fill is None:
        return False
    pattern = fill.find(M("patternFill"))
    if pattern is None:
        return False
    ptype = pattern.get("patternType")
    if ptype == "none":
        return False
    bg = pattern.find(M("bgColor"))
    fg = pattern.find(M("fgColor"))
    if bg is not None and ptype in (None, "solid"):
        return True
    if ptype == "solid" and fg is None:
        return False
    return fg is not None or bg is not None


def used_range_of(ws_root):
    """Two extents: the cells that state a value, and the extent the reader walks.

    The second is `XlsxConditionalStyles.ReadSheet`'s own: every stated `<row>` and `<c>`,
    whether or not it holds a value, plus every `<col>` run's `max`. A conditional format
    paints an empty cell as readily as a full one, so the walked extent is the one that
    decides whether a rule can reach anything.
    """
    minc = minr = maxc = maxr = None
    smaxc = smaxr = None
    for run in ws_root.findall(M("cols")):
        for c in run.findall(M("col")):
            try:
                mx = int(c.get("max"))
            except (TypeError, ValueError):
                continue
            if mx <= 16384:
                smaxc = mx if smaxc is None else max(smaxc, mx)
    sheet_data = ws_root.find(M("sheetData"))
    if sheet_data is not None:
        for row in sheet_data:
            try:
                rr0 = int(row.get("r"))
            except (TypeError, ValueError):
                rr0 = None
            if rr0 is not None:
                smaxr = rr0 if smaxr is None else max(smaxr, rr0)
            for c in row:
                ref = c.get("r")
                if not ref:
                    continue
                p = parse_a1(ref)
                if not p:
                    continue
                cc, rr = p
                smaxc = cc if smaxc is None else max(smaxc, cc)
                smaxr = rr if smaxr is None else max(smaxr, rr)
                has = (c.find(M("v")) is not None or c.find(M("f")) is not None
                       or c.find(M("is")) is not None)
                if not has:
                    continue
                minc = cc if minc is None else min(minc, cc)
                maxc = cc if maxc is None else max(maxc, cc)
                minr = rr if minr is None else min(minr, rr)
                maxr = rr if maxr is None else max(maxr, rr)
    values = None if minc is None else (minc, minr, maxc, maxr)
    walked = None if (smaxc is None or smaxr is None) else (1, 1, smaxc, smaxr)
    return values, walked


def expand_name(name, defined, sheet_key, seen=None, depth=0):
    """Transitively expands a defined name's formula. Returns (text, chain, unresolved)."""
    if seen is None:
        seen = set()
    key = (sheet_key, name.upper())
    body = defined.get(key)
    if body is None:
        body = defined.get((None, name.upper()))
    if body is None:
        return None, [], {name}
    if name.upper() in seen or depth > 12:
        return body, [], set()
    seen = seen | {name.upper()}
    _f, names, _o, _r, _e, _s = tokenise(body)
    chain = []
    unresolved = set()
    out = body
    for inner in names:
        sub, subchain, subun = expand_name(inner, defined, sheet_key, seen, depth + 1)
        if sub is None:
            unresolved |= subun
            continue
        chain.append(inner)
        chain.extend(subchain)
        out = re.sub(r"(?<![A-Za-z0-9_.$!'])" + re.escape(inner) + r"(?![A-Za-z0-9_.(])",
                     "(" + sub + ")", out)
    return out, chain, unresolved


def main():
    rows = []
    x14_blocks = [0]
    x14_types = Counter()
    docs_scanned = 0
    sniffed = []
    total_sheets = 0

    for path, how in candidates():
        docs_scanned += 1
        if how == "sniffed":
            sniffed.append(path)
        doc = os.path.relpath(path, CORPUS)
        try:
            z = zipfile.ZipFile(path)
        except Exception:
            continue
        names = set(z.namelist())
        try:
            wb = ET.fromstring(z.read("xl/workbook.xml"))
        except Exception:
            continue

        wbrels = rels_for(z, "xl/workbook.xml")

        # defined names: key (localSheetId or None, UPPERNAME) -> text
        defined = {}
        defined_raw = []
        dn = wb.find(M("definedNames"))
        if dn is not None:
            for d in dn.findall(M("definedName")):
                nm = d.get("name") or ""
                lsi = d.get("localSheetId")
                lsi = int(lsi) if lsi is not None else None
                body = (d.text or "").strip()
                defined[(lsi, nm.upper())] = body
                defined_raw.append((nm, lsi, body))

        # sheets
        sheets = []
        sh_parent = wb.find(M("sheets"))
        if sh_parent is not None:
            for i, s in enumerate(sh_parent.findall(M("sheet"))):
                rid = s.get(f"{{{NS_REL}}}id")
                tgt = wbrels.get(rid, (None, None))[0]
                part = resolve("xl/workbook.xml", tgt) if tgt else None
                sheets.append({
                    "index": i,
                    "name": s.get("name") or "",
                    "state": s.get("state") or "visible",
                    "part": part,
                })

        # dxfs
        dxfs = []
        stylepart = None
        for rid, (tgt, _m) in wbrels.items():
            if tgt and tgt.endswith("styles.xml"):
                stylepart = resolve("xl/workbook.xml", tgt)
        if stylepart and stylepart in names:
            try:
                st = ET.fromstring(z.read(stylepart))
                dl = st.find(M("dxfs"))
                if dl is not None:
                    dxfs = list(dl.findall(M("dxf")))
            except Exception:
                dxfs = []

        for sh in sheets:
            part = sh["part"]
            if not part or part not in names:
                continue
            total_sheets += 1
            try:
                ws = ET.fromstring(z.read(part))
            except Exception:
                continue

            used, walked = used_range_of(ws)

            # print area for this sheet
            pa = defined.get((sh["index"], "_XLNM.PRINT_AREA"))
            print_box = None
            if pa:
                pr = []
                for p in pa.split(","):
                    r = parse_range(p)
                    if r:
                        pr.append(r)
                if pr:
                    print_box = (min(x[0] for x in pr), min(x[1] for x in pr),
                                 max(x[2] for x in pr), max(x[3] for x in pr))

            found = []  # (origin, sqref, formulas, dxfId, priority)

            for block in ws.findall(M("conditionalFormatting")):
                sq = block.get("sqref")
                for rule in block.findall(M("cfRule")):
                    if rule.get("type") != "expression":
                        continue
                    fs = [(f.text or "") for f in rule.findall(M("formula"))]
                    found.append(("main", sq, fs, rule.get("dxfId"), rule.get("priority")))

            # x14 rules in extLst
            x14_blocks[0] += len(list(ws.iter(f"{{{NS_X14}}}conditionalFormatting")))
            for x14rule in ws.iter(f"{{{NS_X14}}}cfRule"):
                x14_types[x14rule.get("type") or "(none)"] += 1
            for x14cf in ws.iter(f"{{{NS_X14}}}conditionalFormatting"):
                sq = None
                sqe = x14cf.find(f"{{{NS_XM}}}sqref")
                if sqe is not None:
                    sq = (sqe.text or "").strip()
                for rule in x14cf.findall(f"{{{NS_X14}}}cfRule"):
                    if rule.get("type") != "expression":
                        continue
                    fs = [(f.text or "") for f in rule.findall(f"{{{NS_XM}}}f")]
                    found.append(("x14", sq, fs, rule.get("dxfId"), rule.get("priority")))

            for origin, sq, fs, dxfid, prio in found:
                ranges = parse_sqref(sq)
                ok = accepted(fs)
                did = None
                try:
                    did = int(dxfid) if dxfid is not None else None
                except ValueError:
                    did = None
                paints_style = (origin == "main" and did is not None
                                and 0 <= did < len(dxfs) and dxf_paints(dxfs[did]))
                rows.append({
                    "document": doc,
                    "sheet": sh["name"],
                    "sheet_index": sh["index"],
                    "sheet_state": sh["state"],
                    "origin": origin,
                    "sqref": sq or "",
                    "formula_count": len(fs),
                    "formula": fs[0] if fs else "",
                    "all_formulas": " || ".join(fs),
                    "dxfId": "" if did is None else did,
                    "priority": prio or "",
                    "accepted": "yes" if ok else "no",
                    "cells": area(ranges),
                    "ranges": ranges,
                    "used_range": used,
                    "walked_range": walked,
                    "print_area": pa or "",
                    "hits_used": "yes" if intersects(ranges, used) else "no",
                    "hits_walked": "yes" if intersects(ranges, walked) else "no",
                    "has_print_area": "yes" if print_box is not None else "no",
                    "hits_print": ("n/a" if print_box is None
                                   else ("yes" if intersects(ranges, print_box) else "no")),
                    "dxf_paints": "yes" if paints_style else "no",
                    "_defined": defined,
                    "_sheet_index": sh["index"],
                })
        z.close()

    print(f"x14 conditionalFormatting blocks seen: {x14_blocks[0]}; "
          f"x14:cfRule types: {dict(x14_types)}")
    write_outputs(rows, docs_scanned, sniffed, total_sheets)


def fmt_box(b):
    if b is None:
        return ""
    c1, r1, c2, r2 = b
    def col(n):
        s = ""
        while n:
            n, r = divmod(n - 1, 26)
            s = chr(65 + r) + s
        return s
    return f"{col(c1)}{r1}:{col(c2)}{r2}"


def write_outputs(rows, docs_scanned, sniffed, total_sheets):
    def w(name, header, data):
        with open(os.path.join(OUT, name), "w", newline="") as fh:
            wr = csv.writer(fh, delimiter="\t", lineterminator="\n")
            wr.writerow(header)
            wr.writerows(data)

    # rules.tsv
    w("rules.tsv",
      ["document", "sheet", "sheet_state", "origin", "sqref", "cells", "dxfId", "priority",
       "accepted", "formula_count", "formula", "hits_value_range", "hits_walked_range",
       "hits_print_area", "dxf_paints", "value_range", "walked_range"],
      [[r["document"], r["sheet"], r["sheet_state"], r["origin"], r["sqref"], r["cells"],
        r["dxfId"], r["priority"], r["accepted"], r["formula_count"],
        r["all_formulas"].replace("\t", " "), r["hits_used"], r["hits_walked"],
        r["hits_print"], r["dxf_paints"], fmt_box(r["used_range"]),
        fmt_box(r["walked_range"])] for r in rows])

    refused = [r for r in rows if r["accepted"] == "no"]

    # --- token inventory, un-expanded ---
    fn_occ = Counter(); fn_docs = defaultdict(set); fn_cells = Counter()
    nm_occ = Counter(); nm_docs = defaultdict(set); nm_cells = Counter()
    op_occ = Counter(); rf_occ = Counter(); rf_docs = defaultdict(set)
    err_docs = defaultdict(set); err_occ = Counter()

    # --- expanded ---
    fn_occ_x = Counter(); fn_docs_x = defaultdict(set); fn_cells_x = Counter()
    op_occ_x = Counter(); rf_occ_x = Counter(); rf_docs_x = defaultdict(set)
    fn_paint_rules = Counter(); fn_paint_cells = Counter()
    name_rows = []
    unresolved_names = Counter()

    for r in refused:
        f = r["all_formulas"]
        funcs, names, ops, rfs, errs, _st = tokenise(f)
        for k, v in funcs.items():
            fn_occ[k] += v; fn_docs[k].add(r["document"]); fn_cells[k] += r["cells"]
        for k, v in names.items():
            nm_occ[k] += v; nm_docs[k].add(r["document"]); nm_cells[k] += r["cells"]
        for k, v in ops.items():
            op_occ[k] += v
        for k, v in rfs.items():
            rf_occ[k] += v; rf_docs[k].add(r["document"])
        for e in errs:
            err_occ[e] += 1; err_docs[e].add(r["document"])

        # expansion
        expanded = f
        chain_all = []
        for nm in names:
            sub, chain, un = expand_name(nm, r["_defined"], r["_sheet_index"])
            if sub is None:
                unresolved_names[nm] += 1
                continue
            chain_all.append(nm)
            chain_all.extend(chain)
            expanded = re.sub(r"(?<![A-Za-z0-9_.$!'])" + re.escape(nm) + r"(?![A-Za-z0-9_.(])",
                              "(" + sub + ")", expanded)
            name_rows.append([r["document"], r["sheet"], nm,
                              r["_defined"].get((r["_sheet_index"], nm.upper()),
                                                r["_defined"].get((None, nm.upper()), "")),
                              sub, ";".join(chain), r["cells"]])
        xf, xn, xo, xr, xe, _ = tokenise(expanded)
        if can_paint(r):
            for k in set(xf):
                fn_paint_rules[k] += 1
                fn_paint_cells[k] += r["cells"]
        for k, v in xf.items():
            fn_occ_x[k] += v; fn_docs_x[k].add(r["document"]); fn_cells_x[k] += r["cells"]
        for k, v in xo.items():
            op_occ_x[k] += v
        for k, v in xr.items():
            rf_occ_x[k] += v; rf_docs_x[k].add(r["document"])

    w("functions.tsv",
      ["function", "occurrences", "documents", "cells_covered",
       "occurrences_expanded", "documents_expanded", "cells_covered_expanded",
       "can_paint_rules", "can_paint_cells", "doc_list"],
      [[k, fn_occ.get(k, 0), len(fn_docs.get(k, set())), fn_cells.get(k, 0),
        fn_occ_x.get(k, 0), len(fn_docs_x.get(k, set())), fn_cells_x.get(k, 0),
        fn_paint_rules.get(k, 0), fn_paint_cells.get(k, 0),
        "; ".join(sorted(set(fn_docs.get(k, set())) | set(fn_docs_x.get(k, set()))))]
       for k in sorted(set(fn_occ) | set(fn_occ_x),
                       key=lambda k: (-fn_cells_x.get(k, fn_cells.get(k, 0)), k))])

    w("names.tsv",
      ["document", "sheet", "name", "definition", "expanded", "transitive_chain", "cells"],
      sorted(name_rows))

    w("operators.tsv", ["operator", "occurrences", "occurrences_expanded"],
      [[k, op_occ.get(k, 0), op_occ_x.get(k, 0)]
       for k in sorted(set(op_occ) | set(op_occ_x),
                       key=lambda k: (-max(op_occ.get(k, 0), op_occ_x.get(k, 0)), k))])

    w("reference-forms.tsv",
      ["form", "occurrences", "documents", "occurrences_expanded", "documents_expanded",
       "doc_list"],
      [[k, rf_occ.get(k, 0), len(rf_docs.get(k, set())),
        rf_occ_x.get(k, 0), len(rf_docs_x.get(k, set())),
        "; ".join(sorted(set(rf_docs.get(k, set())) | set(rf_docs_x.get(k, set()))))]
       for k in sorted(set(rf_occ) | set(rf_occ_x),
                       key=lambda k: (-max(rf_occ.get(k, 0), rf_occ_x.get(k, 0)), k))])

    w("by-cells.tsv",
      ["cells", "document", "sheet", "sheet_state", "origin", "sqref", "formula",
       "hits_value_range", "hits_walked_range", "hits_print_area", "dxf_paints", "can_paint"],
      [[r["cells"], r["document"], r["sheet"], r["sheet_state"], r["origin"], r["sqref"],
        r["all_formulas"].replace("\t", " "), r["hits_used"], r["hits_walked"],
        r["hits_print"], r["dxf_paints"], "yes" if can_paint(r) else "no"]
       for r in sorted(refused, key=lambda r: (-r["cells"], r["document"], r["sheet"], r["sqref"]))])

    shape_occ = Counter(); shape_cells = Counter(); shape_docs = defaultdict(set)
    for r in refused:
        _f, nms, _o, _rf, _e, _s = tokenise(r["all_formulas"])
        sh = shape_of(r["all_formulas"], nms)
        shape_occ[sh] += 1
        shape_cells[sh] += r["cells"]
        shape_docs[sh].add(r["document"])
    shape_paint = Counter(); shape_paint_cells = Counter()
    for r in refused:
        if not can_paint(r):
            continue
        _f, nms, _o, _rf, _e, _s = tokenise(r["all_formulas"])
        sh = shape_of(r["all_formulas"], nms)
        shape_paint[sh] += 1
        shape_paint_cells[sh] += r["cells"]
    w("shapes.tsv",
      ["shape", "rules", "documents", "cells_covered", "rules_can_paint", "cells_can_paint"],
      [[k, shape_occ[k], len(shape_docs[k]), shape_cells[k],
        shape_paint.get(k, 0), shape_paint_cells.get(k, 0)]
       for k in sorted(shape_occ, key=lambda k: (-shape_cells[k], k))])

    bydoc = defaultdict(lambda: [0, 0, 0, 0])
    for r in rows:
        e = bydoc[r["document"]]
        e[0] += 1
        if r["accepted"] == "no":
            e[1] += 1
            e[2] += r["cells"]
            if can_paint(r):
                e[3] += 1
    w("by-document.tsv",
      ["document", "expression_rules", "refused", "refused_cells", "refused_can_paint"],
      [[k] + v for k, v in sorted(bydoc.items(), key=lambda kv: (-kv[1][1], kv[0]))])

    # summary to stdout
    acc = sum(1 for r in rows if r["accepted"] == "yes")
    ref = len(refused)
    x14 = sum(1 for r in rows if r["origin"] == "x14")
    x14ref = sum(1 for r in refused if r["origin"] == "x14")
    paint = [r for r in refused if can_paint(r)]
    print(f"documents scanned (OPC spreadsheets): {docs_scanned}  (sniffed, lying extension: {len(sniffed)})")
    for s in sniffed:
        print(f"  sniffed: {s}")
    print(f"worksheets parsed: {total_sheets}")
    print(f"expression rules: {len(rows)}  accepted: {acc}  refused: {ref}")
    print(f"  from x14 extLst: {x14} (refused {x14ref})")
    print(f"documents with expression rules: {len({r['document'] for r in rows})}")
    print(f"documents with refused rules: {len({r['document'] for r in refused})}")
    print(f"refused rules that can actually paint: {len(paint)}")
    print(f"  in documents: {sorted({r['document'] for r in paint})}")
    refs = [r for r in refused if "#REF!" in r["all_formulas"]]
    paint_no_ref = [r for r in paint if "#REF!" not in r["all_formulas"]]
    print(f"  of those, whose formula contains #REF! (the reference cannot evaluate it either): "
          f"{len(paint) - len(paint_no_ref)}")
    print(f"  can-paint and free of #REF!: {len(paint_no_ref)} "
          f"covering {sum(r['cells'] for r in paint_no_ref)} cells "
          f"in {len({r['document'] for r in paint_no_ref})} documents")
    print(f"refused rules containing #REF!: {len(refs)} "
          f"in {len({r['document'] for r in refs})} documents, {sum(r['cells'] for r in refs)} cells")
    print(f"refused cells covered (sum of sqref areas): {sum(r['cells'] for r in refused)}")
    print(f"refused cells covered, can-paint rules only: {sum(r['cells'] for r in paint)}")
    print(f"hidden/veryHidden sheet refused rules: {sum(1 for r in refused if r['sheet_state'] != 'visible')}")
    print(f"refused rules missing/invalid dxf paint: {sum(1 for r in refused if r['dxf_paints']=='no')}")
    print(f"refused rules whose sqref misses the value-bearing range: {sum(1 for r in refused if r['hits_used']=='no')}")
    print(f"refused rules whose sqref misses the walked extent: {sum(1 for r in refused if r['hits_walked']=='no')}")
    print(f"refused rules on a sheet that states a print area: {sum(1 for r in refused if r['has_print_area']=='yes')}")
    print(f"refused rules whose sqref misses a stated print area: {sum(1 for r in refused if r['hits_print']=='no')}")
    print(f"refused rules with formula count != 1: {sum(1 for r in refused if r['formula_count'] != 1)}")
    print(f"refused rules with no usable dxfId: {sum(1 for r in refused if r['dxfId']=='')}")
    print()
    print("function set, un-expanded:", ", ".join(sorted(fn_occ)))
    print("function set, after defined-name expansion:", ", ".join(sorted(set(fn_occ) | set(fn_occ_x))))
    transitive = set()
    for row in name_rows:
        for c in (row[5] or "").split(";"):
            if c:
                transitive.add(c)
    print("defined names referenced directly by a rule:", ", ".join(sorted(nm_occ)))
    print("further names reached transitively:", ", ".join(sorted(transitive - set(nm_occ))))
    print("unresolved names:", ", ".join(sorted(unresolved_names)))


SHAPE_RE_AND = re.compile(r"^\s*=?\s*AND\s*\(", re.I)

def shape_of(f, names_seen):
    """A coarse top-level shape for the refused formula, for scoping."""
    t = f.strip()
    if t.startswith("="):
        t = t[1:].strip()
    if "#REF!" in t:
        return "contains #REF!"
    if SHAPE_RE_AND.match(t):
        return "AND(...) of 2-3 comparisons"
    if re.match(r"^[A-Za-z_][A-Za-z0-9_.]*$", t):
        return "bare defined name / bare TRUE"
    if index_of_operator(t, "=") < 0 and index_of_operator(t, "<") < 0 \
       and index_of_operator(t, ">") < 0:
        if "*" in t or "+" in t:
            return "boolean arithmetic ((...)*(...))"
        return "bare function call, no comparison"
    if FUNC_RE.search(t):
        return "comparison with a function on one side"
    if names_seen:
        return "comparison against a defined name"
    return "comparison, other"


def can_paint(r):
    return (r["accepted"] == "no"
            and r["sheet_state"] == "visible"
            and r["dxf_paints"] == "yes"
            and r["hits_walked"] == "yes"
            and r["hits_print"] in ("yes", "n/a"))


if __name__ == "__main__":
    main()
