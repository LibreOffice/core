#!/usr/bin/env python3
"""Regroup a batched corpus by what is *wrong* with each document, not by how hard it is.

    regroup-batches.py <corpus-root> <status.tsv> [--apply]

`make-batches.py` orders the corpus by rendering complexity, which is the right ordering
when nothing passes yet: it puts the cheap documents first so a track can be driven from
the easy end. Once most of the corpus passes, that ordering stops earning its keep — the
remaining failures are scattered across twenty batches, and a session that takes "the next
batch" gets nine documents it cannot learn anything from and one it can.

This regroups the same corpus by **status and defect kind**:

    <family>/done-NNN/<ext>/…          every document that currently passes the gate
    <family>/<kind>-NNN/<ext>/…        the rest, grouped by what is wrong with them

so a session can take `sheets/ceiling-*` and know every document in it is a measurement
artefact, or take `words/pagination-*` and work one mechanism across ten documents that
share it.

WHAT THIS COSTS, AND WHY THE MANIFEST MATTERS
─────────────────────────────────────────────
Batch membership in this corpus is the **directory layout** — `batch-check.sh` globs
directories and finds the files beneath them — so regrouping means moving files, and every
stored figure that names a path stops resolving. `dotnet/probes/` is full of archival
scripts that name batch paths, and `CLAUDE.md` says plainly that those are deliberately
left as the record of what a given round actually ran.

So the manifest is not bookkeeping here, it is the undo. It keeps `source` (the path the
document had before any batching at all, which `make-batches.py` recorded) and gains
`previous_batch`, `status` and `kind`. Between them, any path in any stored result can be
resolved forward to where the document lives now.

THE ORDERING INSIDE A GROUP IS STILL THE COMPLEXITY ORDERING
────────────────────────────────────────────────────────────
Documents keep their score and are placed in score order within their group, so the
easy-first property survives inside each kind. A `pagination-001` is still the cheapest ten
pagination failures. Losing that would make each new group a random pile.

INPUT
─────
A TSV with a header and at least:

    path      the document's CURRENT path, family/batch-NNN/ext/name
    status    `done` for passing, anything else for failing
    kind      for a failing document, the group to file it under; ignored when done

Unknown documents — present in the corpus and absent from the TSV — are an error rather
than a default, because silently filing a document as `done` because a sweep row was
missing is exactly the failure this whole workflow exists to avoid.
"""
from __future__ import annotations

import argparse
import collections
import csv
import subprocess
import sys
from pathlib import Path

PER_BATCH = 10

# A kind may only contain these, so that a directory name stays globbable and a group
# cannot be created by a typo in a classification.
ALLOWED = set("abcdefghijklmnopqrstuvwxyz-")


def read_status(path: Path) -> dict[str, tuple[str, str]]:
    """Map a corpus-relative path to its (status, kind)."""
    out: dict[str, tuple[str, str]] = {}
    with path.open() as handle:
        for row in csv.DictReader(handle, delimiter="\t"):
            if not row.get("path"):
                continue
            status = (row.get("status") or "").strip().lower()
            kind = (row.get("kind") or "").strip().lower()

            if status != "done":
                if not kind:
                    sys.exit(f"{row['path']}: not done and no kind — nothing to group it by")
                if set(kind) - ALLOWED:
                    sys.exit(f"{row['path']}: kind {kind!r} is not a usable directory name")

            out[row["path"].strip()] = (status, kind)
    return out


def read_manifest(root: Path) -> dict[str, dict[str, str]]:
    """The existing manifest, by current path. It carries the score and the true source."""
    manifest = root / "MANIFEST.tsv"
    if not manifest.exists():
        sys.exit(f"{manifest}: missing — regrouping without it would lose the original paths")

    with manifest.open() as handle:
        return {row["path"]: row for row in csv.DictReader(handle, delimiter="\t")}


def plan(root: Path, status: dict[str, tuple[str, str]], manifest: dict[str, dict[str, str]]):
    """Where every document should end up, in score order within its group."""
    documents = [
        p.relative_to(root) for p in sorted(root.rglob("*"))
        if p.is_file() and ".git" not in p.parts and len(p.relative_to(root).parts) == 4
    ]

    missing = [str(d) for d in documents if str(d) not in status]
    if missing:
        sys.exit("not in the status TSV, so their group is unknown:\n  "
                 + "\n  ".join(missing[:20])
                 + (f"\n  … and {len(missing) - 20} more" if len(missing) > 20 else ""))

    # family -> group -> [(score, relative path)]
    grouped: dict[tuple[str, str], list[tuple[float, Path]]] = collections.defaultdict(list)
    for rel in documents:
        state, kind = status[str(rel)]
        family = rel.parts[0]
        group = "done" if state == "done" else kind

        row = manifest.get(str(rel), {})
        try:
            score = float(row.get("score") or 0.0)
        except ValueError:
            score = 0.0

        grouped[(family, group)].append((score, rel))

    final = []
    for (family, group), members in sorted(grouped.items()):
        members.sort(key=lambda item: (item[0], str(item[1])))
        for index, (score, rel) in enumerate(members):
            batch = f"{group}-{index // PER_BATCH + 1:03d}"
            ext = rel.suffix.lower().lstrip(".")
            final.append((rel, Path(family) / batch / ext / rel.name, score, group))

    return final


def write_manifest(root: Path, final, manifest: dict[str, dict[str, str]],
                   status: dict[str, tuple[str, str]]) -> None:
    """Rewrite the manifest so the previous layout stays resolvable.

    `source` is the path the document had before it was ever batched and is carried
    through untouched; `previous_batch` is where this run found it. A stored figure naming
    either can be followed forward.
    """
    with (root / "MANIFEST.tsv").open("w", newline="") as handle:
        writer = csv.writer(handle, delimiter="\t", lineterminator="\n")
        writer.writerow(["family", "batch", "path", "ext", "score", "source",
                         "previous_batch", "status", "kind"])
        for rel, dest, score, group in final:
            was = manifest.get(str(rel), {})
            state, kind = status[str(rel)]
            writer.writerow([
                dest.parts[0], dest.parts[1], str(dest),
                dest.suffix.lower().lstrip("."),
                "" if score is None else f"{score:.1f}",
                was.get("source", str(rel)),
                rel.parts[1],
                state,
                "" if state == "done" else kind,
            ])


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("root", type=Path)
    ap.add_argument("status", type=Path)
    ap.add_argument("--apply", action="store_true",
                    help="perform the git mv's; without it, print the plan and change nothing")
    args = ap.parse_args()

    root = args.root.resolve()
    status = read_status(args.status)
    manifest = read_manifest(root)
    final = plan(root, status, manifest)

    moves = [(rel, dest) for rel, dest, _, _ in final if rel != dest]

    counts = collections.Counter((dest.parts[0], dest.parts[1].rsplit("-", 1)[0])
                                 for _, dest, _, _ in final)
    for (family, group), n in sorted(counts.items()):
        print(f"{family:8} {group:16} {n:4} documents")
    print(f"\n{len(moves)} of {len(final)} documents move")

    if not args.apply:
        print("\n(dry run — pass --apply to perform the git mv's)")
        return 0

    dirty = subprocess.run(["git", "-C", str(root), "status", "--porcelain"],
                           capture_output=True, text=True, check=True).stdout.strip()
    if dirty:
        sys.exit("the corpus working tree is not clean — a failed run is only easy to undo "
                 "with `git checkout .` when there is nothing else to lose")

    for rel, dest in moves:
        (root / dest).parent.mkdir(parents=True, exist_ok=True)
        subprocess.run(["git", "-C", str(root), "mv", "--", str(rel), str(dest)], check=True)

    # `git mv` leaves the emptied directories behind, and a corpus full of empty batch
    # directories makes every later glob match things that hold nothing.
    for path in sorted(root.rglob("*"), key=lambda p: len(p.parts), reverse=True):
        if path.is_dir() and ".git" not in path.parts and not any(path.iterdir()):
            path.rmdir()

    write_manifest(root, final, manifest, status)
    print(f"\napplied. MANIFEST.tsv rewritten with previous_batch, status and kind.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
