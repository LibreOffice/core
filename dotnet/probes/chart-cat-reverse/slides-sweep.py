"""Re-sweep the slides track against the gate's own banked reference PDFs.

`batch-check.sh` renders both halves; the reference half of the whole-corpus gate at
`2f4709c08` is already on disk under `/home/user/gate-2f47/ref/`, so only our half needs
rendering and the reference cannot drift underneath the comparison. The verdict rule is
`batch-check.sh`:262-286 transcribed — page counts equal, glyph counts within max(2%, 15), no
unembedded font — and it is validated before use by scoring the gate's own banked `ours/` PDFs
and requiring that every verdict comes back equal to the stored one.

Usage:  slides-sweep.py <out-dir> [--validate-only]
"""
import os
import subprocess
import sys
from pathlib import Path

CORPUS = Path("/home/user/sample-files")
GATE = Path("/home/user/gate-2f47")
CLI = os.environ.get(
    "XRAY_OFFICE_CLI",
    "/home/user/wt-slidechart/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli")


def identity(path: Path) -> str:
    return f"{path.stem}__{path.suffix.lstrip('.').lower()}"


def counts(pdf: Path):
    """Pages, alphanumeric-bearing tokens, raw tokens, alphanumeric characters, unembedded faces."""
    if not pdf.exists():
        return None
    info = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True).stdout
    pages = next((int(l.split()[1]) for l in info.splitlines() if l.startswith("Pages:")), 0)
    text = subprocess.run(["pdftotext", str(pdf), "-"], capture_output=True).stdout.decode(
        "utf-8", "replace")
    tokens = text.split()
    words = sum(1 for t in tokens if any(c.isalnum() for c in t))
    glyphs = sum(1 for c in text if c.isalnum())
    fonts = subprocess.run(["pdffonts", str(pdf)], capture_output=True, text=True).stdout
    rows = [r for r in fonts.splitlines()[2:] if r.strip()]
    unembedded = sum(1 for r in rows if len(r.split()) >= 8 and r.split()[-5] == "no")
    return pages, words, len(tokens), glyphs, len(rows), unembedded


def verdict(ours, ref):
    if ours is None and ref is None:
        return "both-failed"
    if ref is None:
        return "ref-failed"
    if ours is None:
        return "ours-failed"

    op, _, _, og, _, un = ours
    rp, _, _, rg, _, _ = ref

    faults = []
    if op != rp:
        faults.append("pages")
    if rg > 0:
        if abs(og - rg) > rg * 0.02 and abs(og - rg) > 15:
            faults.append("words")
    elif og > 15:
        faults.append("words")
    if un:
        faults.append("unembedded")
    return ",".join(faults) if faults else "match"


def stored():
    rows = {}
    for line in (GATE / "parity.tsv").read_text().splitlines():
        if line.startswith("#") or line.startswith("path\t"):
            continue
        parts = line.split("\t")
        if parts[0].startswith("slides/"):
            rows[parts[0]] = parts[6]
    return rows


def main():
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    validate_only = "--validate-only" in sys.argv

    want = stored()
    paths = sorted(want)

    # Validate the transcription against the gate's own two halves before trusting it.
    disagreed = []
    for rel in paths:
        src = CORPUS / rel
        key = identity(src)
        got = verdict(counts(GATE / "ours" / f"{key}.pdf"), counts(GATE / "ref" / f"{key}.pdf"))
        if got != want[rel]:
            disagreed.append((rel, want[rel], got))
    print(f"# validation: {len(paths) - len(disagreed)} of {len(paths)} verdicts reproduced")
    for rel, was, now in disagreed:
        print(f"#   DISAGREES\t{rel}\t{was}\t{now}")
    if validate_only:
        return
    if disagreed:
        sys.exit("the verdict rule does not reproduce the gate; refusing to score against it")

    (out / "ours").mkdir(exist_ok=True)
    print("path\tpages\twords\tfonts\tunemb\tverdict\trawwords\tglyphs\twas")
    for rel in paths:
        src = CORPUS / rel
        key = identity(src)
        mine = out / "ours" / f"{key}.pdf"
        if not mine.exists():
            work = out / "t"
            subprocess.run(["rm", "-rf", str(work)])
            work.mkdir(parents=True, exist_ok=True)
            subprocess.run([CLI, "render", str(src), "--format", "pdf", "--outdir", str(work)],
                           capture_output=True, timeout=300)
            made = work / (src.stem + ".pdf")
            if made.exists():
                made.replace(mine)

        ours = counts(mine)
        ref = counts(GATE / "ref" / f"{key}.pdf")
        got = verdict(ours, ref)
        o = ours or ("-",) * 6
        r = ref or ("-",) * 6
        print(f"{rel}\t{o[0]}/{r[0]}\t{o[1]}/{r[1]}\t{o[4]}/{r[4]}\t{o[5]}\t{got}"
              f"\t{o[2]}/{r[2]}\t{o[3]}/{r[3]}\t{want[rel]}")
        sys.stdout.flush()


if __name__ == "__main__":
    main()
