#!/usr/bin/env python3
"""Order a document corpus by rendering complexity and cut it into batches of ten.

    make-batches.py <corpus-root> <complexity.tsv> [--apply]

Reads the TSV produced by pdf-complexity.py over LibreOffice's renders of the corpus,
sorts every document by what its PDF demands of a renderer, and prints the plan that
reorganises the repository into <family>/batch-NNN/<ext>/. With --apply it performs the
`git mv`s, deletes the duplicates and unrenderable files, and writes the two manifests.

Ten per batch is the unit of work: small enough that one session can take a batch,
fix what it finds and prove the fix, without the batch itself becoming a project.

Run it on a clean working tree — --apply rewrites the entire layout, and a failed run is
far easier to undo with `git checkout .` than by hand.
"""
import collections
import csv
import hashlib
import subprocess
import sys
from pathlib import Path

PER_BATCH = 10

# Which library owns each extension. Splitting the corpus this way is what lets three
# agents work in parallel without ever touching the same file.
FAMILY = {
    "doc": "words", "docx": "words",
    "xls": "sheets", "xlsx": "sheets",
    "ppt": "slides", "pptx": "slides",
}


def load_scores(tsv: Path) -> dict[str, float]:
    """Map the mangled PDF stem back to a score."""
    out = {}
    with tsv.open() as f:
        for row in csv.DictReader(f, delimiter="\t"):
            out[Path(row["path"]).stem] = float(row["score"])
    return out


def mangle(rel: Path) -> str:
    """The id render-corpus.sh gave this document's PDF."""
    ext = rel.suffix.lstrip(".")
    return str(rel.with_suffix("")).replace("/", "_").replace(" ", "_") + "__" + ext


def write_manifest(final, path: Path) -> None:
    """Record the score each document was placed by.

    Without this the ordering is unreproducible: the scores live in a scratch directory
    that does not survive the session, and a batch boundary with no recorded reason
    invites someone to 'tidy' the layout and silently destroy the ordering the whole
    workflow depends on.
    """
    with path.open("w", newline="") as f:
        w = csv.writer(f, delimiter="\t", lineterminator="\n")
        w.writerow(["family", "batch", "path", "ext", "score", "source"])
        for rel, dest, score in final:
            w.writerow([dest.parts[0], dest.parts[1], str(dest),
                        dest.suffix.lower().lstrip("."),
                        "" if score is None else f"{score:.1f}", str(rel)])


def write_duplicates(dupes, final, path: Path) -> None:
    """Record what was removed and where its surviving copy now lives.

    Pointing at the copy's *new* path rather than its old one is the difference between
    an auditable deletion and a list of names that no longer resolve to anything.
    """
    moved = {rel: dest for rel, dest, _ in final}
    with path.open("w", newline="") as f:
        w = csv.writer(f, delimiter="\t", lineterminator="\n")
        w.writerow(["removed", "identical_to"])
        for rel, kept in sorted(dupes, key=lambda d: str(d[0])):
            w.writerow([str(rel), str(moved.get(kept, kept))])


def main() -> int:
    argv = [a for a in sys.argv[1:] if not a.startswith("--")]
    if len(argv) != 2:
        print(__doc__, file=sys.stderr)
        return 2
    SRC, scores = Path(argv[0]), load_scores(Path(argv[1]))

    docs, seen_bytes, dupes, unrenderable = [], {}, [], []
    for p in sorted(SRC.rglob("*")):
        if not p.is_file() or ".git" in p.parts:
            continue
        if p.suffix.lower().lstrip(".") not in FAMILY:
            continue
        rel = p.relative_to(SRC)

        # Byte-identical copies are 39% of this corpus, mostly the same document collected
        # from two sites. Rendering one twice proves nothing and costs a batch slot that a
        # document with different content should have had, so only the first is kept — by
        # content, not by name, since the copies rarely share one.
        digest = hashlib.sha256(p.read_bytes()).hexdigest()
        if digest in seen_bytes:
            dupes.append((rel, seen_bytes[digest]))
            continue
        seen_bytes[digest] = rel

        # A document LibreOffice cannot convert has no score and, more to the point, no
        # reference rendering — there is nothing to compare a result against, so keeping it
        # would put a file in a batch that can never be marked passing. Each was retried
        # once with a ten-minute ceiling before being dropped.
        score = scores.get(mangle(rel))
        if score is None:
            unrenderable.append(rel)
            continue
        docs.append((score, rel))

    ordered = sorted(docs, key=lambda d: (d[0], str(d[1])))

    # Batches are cut per family rather than across the whole corpus. One global ordering
    # would front-load the easy end with word processing and leave the spreadsheet and
    # presentation work with nothing to do until batch forty, which defeats the point of
    # working the three families in parallel. Within a family the ordering is still
    # strictly ascending, so each track is worked easiest-first exactly as before.
    plan, batches = [], collections.defaultdict(list)
    per_family = collections.Counter()
    for score, rel in ordered:
        ext = rel.suffix.lower().lstrip(".")
        fam = FAMILY[ext]
        n = per_family[fam] // PER_BATCH + 1
        per_family[fam] += 1
        dest = Path(fam) / f"batch-{n:03d}" / ext / rel.name
        plan.append((rel, dest, score))
        batches[(fam, n)].append((rel, dest, score))

    # A name can repeat across the source's extension folders; make it unique per target.
    seen, final = collections.Counter(), []
    for rel, dest, score in plan:
        seen[dest] += 1
        if seen[dest] > 1:
            dest = dest.with_name(f"{dest.stem}-{seen[dest]}{dest.suffix}")
        final.append((rel, dest, score))

    if "--apply" in sys.argv:
        for rel in [r for r, _ in dupes] + unrenderable:
            subprocess.run(["git", "-C", str(SRC), "rm", "-q", "--", str(rel)], check=True)
        for rel, dest, _ in final:
            target = SRC / dest
            target.parent.mkdir(parents=True, exist_ok=True)
            subprocess.run(["git", "-C", str(SRC), "mv", "--", str(rel), str(dest)], check=True)
        # Drop whatever directories the move emptied. `git mv` leaves them behind, and an
        # empty `docx/` beside the new tracks reads as "some files were missed".
        for p in sorted(SRC.rglob("*"), key=lambda q: -len(q.parts)):
            if p.is_dir() and ".git" not in p.parts and not any(p.iterdir()):
                p.rmdir()
        write_manifest(final, SRC / "MANIFEST.tsv")
        write_duplicates(dupes, final, SRC / "DUPLICATES.tsv")
        print(f"removed {len(dupes)} byte-identical copies and "
              f"{len(unrenderable)} documents LibreOffice cannot convert; "
              f"moved {len(final)} documents into {len(batches)} batches")
        for rel in unrenderable:
            print(f"  dropped (unrenderable): {rel}")
    else:
        by_track = collections.defaultdict(list)
        for rel, dest, score in final:
            by_track[(dest.parts[0], dest.parts[1])].append((rel, dest, score))
        for (fam, batch) in sorted(by_track):
            rows = by_track[(fam, batch)]
            got = [s for *_, s in rows if s is not None]
            kinds = collections.Counter(d.parts[2] for _, d, _ in rows)
            span = f"{min(got):.0f}-{max(got):.0f}" if got else "unrendered"
            print(f"{fam:6s} {batch}  {len(rows):2d} files  score {span:>12}  " +
                  " ".join(f"{k}:{v}" for k, v in sorted(kinds.items())))
        print(f"\n{len(final)} documents, {len(batches)} batches of up to {PER_BATCH}"
              f"  ({len(dupes)} byte-identical copies and "
              f"{len(unrenderable)} unrenderable documents dropped)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
