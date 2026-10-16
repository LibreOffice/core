#!/usr/bin/env python3
"""Census the constructs this round turned on, over a directory of `.rtf`.

    python3 census.py /home/user/corpus-odf/words

  shpwr      how many of each `\\shpwr` value, and in how many documents
  shptbl     how many `{\\shptxt}` groups hold a table, and in how many documents
  posrel     how many of each `posrelh` value
"""
import sys, re, glob, os, collections


def groups_containing(text, needle, inside):
    """Count the groups opened by `needle` whose body contains one of `inside`."""
    found = 0
    for m in re.finditer(re.escape(needle), text):
        start = text.rfind("{", 0, m.start())
        if start < 0:
            continue
        depth, i = 0, start
        while i < len(text):
            c = text[i]
            if c == "\\":
                i += 2
                continue
            if c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    break
            i += 1
        body = text[start:i + 1]
        if any(w in body for w in inside):
            found += 1
    return found


def main():
    root = sys.argv[1]
    what = sys.argv[2] if len(sys.argv) > 2 else "all"
    files = sorted(glob.glob(os.path.join(root, "**", "*.rtf"), recursive=True))
    wr = collections.Counter()
    wr_docs = collections.defaultdict(set)
    pr = collections.Counter()
    tbl_groups = 0
    tbl_docs = set()

    for path in files:
        text = open(path, encoding="latin-1", errors="replace").read()
        for m in re.finditer(r"\\shpwr(-?\d+)", text):
            wr[m.group(1)] += 1
            wr_docs[m.group(1)].add(path)
        for m in re.finditer(r"\{\\sn posrelh\}\{\\sv (-?\d+)\}", text):
            pr[m.group(1)] += 1
        n = groups_containing(text, "\\shptxt", ("\\trowd", "\\intbl"))
        if n:
            tbl_groups += n
            tbl_docs.add(path)

    print(f"{len(files)} documents under {root}")
    if what in ("all", "shpwr"):
        for k in sorted(wr, key=lambda k: -wr[k]):
            print(f"  shpwr{k}: {wr[k]} occurrences in {len(wr_docs[k])} documents")
    if what in ("all", "posrel"):
        for k in sorted(pr, key=lambda k: -pr[k]):
            print(f"  posrelh {k}: {pr[k]} occurrences")
    if what in ("all", "shptbl"):
        print(f"  a table inside {{\\shptxt}}: {tbl_groups} groups in {len(tbl_docs)} documents")
        for p in sorted(tbl_docs):
            print("     " + os.path.basename(p))


if __name__ == "__main__":
    main()
