#!/usr/bin/env python3
"""Index every stored probe figure by the environment that produced it.

Why this exists
---------------

Stored evidence on this project decays silently while continuing to look
authoritative. Three separate cases surfaced in one day: a `words-after.tsv`
carrying numbers from a sweep that overlapped a rebuild; a "39/39 exact" CJK fit
measured on a face whose line gap is zero, so it could not have discriminated;
and a "exact on all 96" advance probe that is 16 of 96 against today's binary.

The census that motivated this file is the sharp version of the same point:

    prose write-ups naming a LibreOffice version   122 of 154
    stored TSVs recording any environment at all     3 of 256

The write-up says what it was measured against. The *data* does not — and the
data is what a later round greps, pastes into a brief and acts on. A TSV is a
grid of numbers with no way to tell a reader that its reference bank no longer
exists.

What it does NOT do
-------------------

It does not rewrite the probe files. `dotnet/CLAUDE.md` is explicit that the
archival scripts and their output are *the record of what a given round actually
ran*, and rewriting them would falsify it. A `#` header prepended to a TSV would
also break every consumer that reads line 1 as the column names.

So this writes a **sidecar**: one index, beside the data, that nothing parses and
anything can read.

Eras
----

One boundary matters, and it is sharp. On 2026-08-13 the project moved
containers in commit 4cbaeb41c3b ("Adapt the tree to the new container, and bank
the reference half of the gate"), and a5d453fae3f the same day established that
`fonts-dejavu-core` had been missing ("The gate's inputs include the font set,
and nothing declared it"). Both inputs to the gate changed together:

    before 2026-08-13   LibreOffice 24.2.7.2, font set unverified
    2026-08-13 onward   LibreOffice 26.2.4.2, fonts-dejavu-core present

Neither half of that is cosmetic. Re-rendering the reference across the version
change moved 63 of 534 page counts and put 210 of 534 word counts outside the
gate's own band; holding the version constant and varying only the font set
moved 53 of 534 page counts on its own. A figure from the earlier era is not
stale in the sense of "slightly old" — it was measured against a reference bank
this container cannot reproduce.

Usage
-----

    python3 dotnet/probes/provenance-index.py            # write PROVENANCE.tsv
    python3 dotnet/probes/provenance-index.py --check    # exit 1 if out of date
    python3 dotnet/probes/provenance-index.py --force    # write even if rows are lost

Re-run it after adding probe output, or the index is itself an instance of the
problem it documents.

Two safety rules, both added after this script destroyed data
-------------------------------------------------------------

**Any unrecognised argument used to trigger a full rewrite.** `main` tested
`"--check" in sys.argv` and otherwise wrote, so `--help`, `-h` and every typo
silently regenerated the file. One round ran it by accident that way. Arguments
are now parsed explicitly and anything unknown writes nothing.

**And a rewrite loses rows, in the main checkout, today.** `build()` reproduces
1232 rows against the 2155 the file holds — rounds have appended rows by hand
that the walk does not regenerate, so the "just re-run it" contract has been
broken for some time and regenerating silently discards 923 figures. A write
that would drop any path now refuses and names the count; `--force` overrides it
for whoever genuinely means to prune. Fixing `build()` to reproduce the whole
file is a separate job and is not attempted here.
"""

import os
import subprocess
import sys

BOUNDARY = "2026-08-13"
ROOTS = ["dotnet/probes", "dotnet/research/probes"]
OUT = "dotnet/probes/PROVENANCE.tsv"

ERAS = {
    "pre-container": ("24.2.7.2", "unverified"),
    "current": ("26.2.4.2", "present"),
}


def repo_root() -> str:
    return subprocess.run(
        ["git", "rev-parse", "--show-toplevel"],
        capture_output=True, text=True, check=True).stdout.strip()


def added_dates(root: str) -> dict[str, str]:
    """The date each tracked file under ROOTS first entered git.

    `--diff-filter=A` and `--name-only` in one pass rather than a `git log` per
    file: the surrounding LibreOffice tree is large enough that 256 invocations
    take minutes and one takes seconds.
    """
    out = subprocess.run(
        ["git", "log", "--diff-filter=A", "--format=C %ad", "--date=short",
         "--name-only", "--"] + ROOTS,
        capture_output=True, text=True, cwd=root, check=True).stdout

    dates: dict[str, str] = {}
    date = None
    for line in out.splitlines():
        if line.startswith("C "):
            date = line[2:].strip()
        elif line.strip() and date:
            # setdefault: a file added, deleted and re-added keeps its first date,
            # which is the one that says which reference bank produced it.
            dates.setdefault(line.strip(), date)
    return dates


def cite_key(path: str) -> str:
    """The fragment a document would use to name this figure: `<round>/<file>`.

    Matching on the basename alone does not work and the way it fails is worth
    recording, because it is the same class of error this whole index exists to
    catch: 84 of the probe directories contain a `README.md`, and every `TODO.md`
    in the tree uses the word "README.md" generically. A basename match therefore
    reported all 84 as cited by nine documents each — an instrument returning a
    confident answer to a question it was not measuring.

    The last two components are distinctive (`sheets-r22/README.md`) and are how
    these files are actually referenced in prose.
    """
    parts = path.split("/")
    return "/".join(parts[-2:]) if len(parts) >= 2 else path


def cited_by(root: str, names: set[str]) -> dict[str, list[str]]:
    """Which live guidance documents name each stored figure.

    "Live" means a document a later round reads as current instruction rather
    than as a record: CLAUDE.md, the TODO scoreboards, the skills. A stale figure
    nobody cites is archival and harmless. A stale figure cited by
    `TODO.raster-ceiling.md` — which `dotnet/CLAUDE.md` tells every agent to
    consult before working a word-count failure — is not.
    """
    live: list[str] = []
    for path in ("dotnet/CLAUDE.md", "CLAUDE.md", "MISSING_PACKAGES.md"):
        if os.path.isfile(os.path.join(root, path)):
            live.append(path)
    for base in (".claude/skills", "dotnet"):
        for dirpath, _, filenames in os.walk(os.path.join(root, base)):
            if "probes" in dirpath.split(os.sep):
                continue
            for name in filenames:
                if name.startswith("TODO") or name == "SKILL.md" or name == "results.md":
                    live.append(os.path.relpath(os.path.join(dirpath, name), root))

    hits: dict[str, list[str]] = {}
    for doc in sorted(set(live)):
        try:
            text = open(os.path.join(root, doc), errors="ignore").read()
        except OSError:
            continue
        for key in names:
            if key in text:
                hits.setdefault(key, []).append(doc)
    return hits


def build(root: str) -> list[tuple[str, ...]]:
    dates = added_dates(root)
    stored = {p: d for p, d in dates.items() if p.endswith((".tsv", ".md", ".json"))}
    hits = cited_by(root, {cite_key(p) for p in stored})

    rows = []
    for path in sorted(stored):
        date = stored[path]
        era = "current" if date >= BOUNDARY else "pre-container"
        binary, fonts = ERAS[era]
        citers = hits.get(cite_key(path), [])
        # A pre-boundary figure that live guidance still cites is the whole point
        # of the index; anything else is archival and flagged as such.
        risk = "CITED-STALE" if era == "pre-container" and citers else (
            "archival" if era == "pre-container" else "current")
        rows.append((path, date, era, binary, fonts, risk, ";".join(citers) or "-"))
    return rows


def main() -> int:
    root = repo_root()
    rows = build(root)
    header = ("path", "added", "era", "libreoffice", "dejavu", "risk", "cited_by")
    body = "\n".join("\t".join(r) for r in rows)
    text = "\t".join(header) + "\n" + body + "\n"

    target = os.path.join(root, OUT)

    args = [a for a in sys.argv[1:]]
    unknown = [a for a in args if a not in ("--check", "--force")]
    if unknown:
        # Anything unrecognised - `--help` and every typo included - must write
        # nothing. This branch did not exist, and `main` wrote unconditionally
        # unless `--check` was present, so asking the script for help rewrote the
        # index instead.
        print(__doc__.split("Usage")[1].split("Two safety rules")[0].strip(), file=sys.stderr)
        return 0 if unknown[0] in ("-h", "--help") else 2

    if "--check" in args:
        existing = open(target).read() if os.path.isfile(target) else ""
        if existing != text:
            print(f"{OUT} is out of date; re-run provenance-index.py", file=sys.stderr)
            return 1
        print(f"{OUT} is up to date ({len(rows)} figures)")
        return 0

    # A rewrite currently *loses* rows: rounds have appended entries by hand that
    # the walk does not regenerate. Refuse rather than discard them silently.
    lost = 0
    if os.path.isfile(target):
        have = {ln.split("\t", 1)[0] for ln in open(target).read().splitlines()[1:] if ln}
        lost = len(have - {r[0] for r in rows})
    if lost and "--force" not in args:
        print(f"{OUT}: refusing to write - {lost} of {len(have)} rows would be lost "
              f"({len(rows)} regenerated). Re-run with --force if that is intended.",
              file=sys.stderr)
        return 1

    open(target, "w").write(text)
    stale = sum(1 for r in rows if r[5] == "CITED-STALE")
    pre = sum(1 for r in rows if r[2] == "pre-container")
    print(f"{OUT}: {len(rows)} stored figures, "
          f"{pre} from before {BOUNDARY}, {stale} of those still cited by live guidance")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
