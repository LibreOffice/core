#!/usr/bin/env python3
"""Decide what a document's divergence *means*, against both reference binaries at once.

`compare-images.py` already answers "how do these two page images differ" better than any
single number can -- it has `ink_delta` for content that is missing, `shifted_tiles` and
`row_profile_shift` for content that merely moved, and `max_tile_error` for one small region
badly wrong. This script does not re-do any of that. It imports it, and adds the four things
that were missing above it, each of which cost this project a round to learn the hard way.

**1. Comparability comes before scoring.** The 26.2.4.2 tarball ships its own
`NotoSans-Regular.ttf` and `NotoSerif-Regular.ttf`, so for any family the system lacks it
resolves to Noto where the distro binary and X-Ray.Office both resolve to DejaVu. An ink figure
taken across that gap measures the font, not the renderer -- two agents independently spent
hours on `2017-04-27-Lease-Transition-Records-Checklist` before one of them read the faces out
of the PDFs and found the comparison was void. So the faces are compared *first*, and a
document whose two sides resolve different families is `unscoreable` against that reference
rather than "wrong".

**2. Two references, one verdict.** `soffice` here is 24.2.7.2 and the tree is calibrated to
26.2.4.2. Rescoring the worst thirty word-processing documents put eleven of them in the
version gap rather than in the tree. Scoring against one binary and reporting the number is
how that happened; this scores against both and makes the disagreement the verdict.

**3. Words per page, not per document.** `033_Event_planning_tracker` was ten words short on
page 1 and eighteen long on page 3, and its document total looked nearly right. Fixing page 1
"broke" the gate. A per-page count would have shown the compensating pair from the start.

**4. A page can ask for eyes.** Where the signals disagree -- the same quantity of ink in the
wrong place, or a page that scores well overall with one tile badly wrong -- no scalar settles
it and the honest output is a request for a reading, not a number. Those pages are listed with
the command that pairs them.

Usage:
    verdict.py <document> [--dpi 150] [--out DIR] [--pages 1-5]
"""
from __future__ import annotations

import argparse, importlib.util, json, os, re, subprocess, sys, tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
_spec = importlib.util.spec_from_file_location("cmpimg", HERE / "compare-images.py")
CMP = importlib.util.module_from_spec(_spec); _spec.loader.exec_module(CMP)

LO24 = os.environ.get("LO24", "soffice")
LO26 = os.environ.get("LO26", "/opt/libreoffice26.2/program/soffice")
CLI = os.environ.get("XRAY_OFFICE_CLI", "")

# A subset-embedded face is written `ABCDEF+Liberation Serif`; the tag is per-file and says
# nothing about which family was resolved, so it has to come off before the sets are compared.
SUBSET = re.compile(r"^[A-Z]{6}\+")


def run(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, timeout=kw.pop("timeout", 1800), **kw)


def render_lo(binary: str, src: Path, out: Path) -> Path | None:
    out.mkdir(parents=True, exist_ok=True)
    pdf = out / (src.stem + ".pdf")
    if not pdf.exists():
        run([binary, f"-env:UserInstallation=file://{out / 'prof'}", "--headless",
             "--convert-to", "pdf", "--outdir", str(out), str(src)])
    return pdf if pdf.exists() else None


def render_ours(src: Path, out: Path) -> Path | None:
    if not CLI:
        sys.exit("set XRAY_OFFICE_CLI to the tree you mean to measure")
    out.mkdir(parents=True, exist_ok=True)
    pdf = out / (src.stem + ".pdf")
    if not pdf.exists():
        run([CLI, "render", str(src), "--format", "pdf", "--outdir", str(out)])
    return pdf if pdf.exists() else None


def faces(pdf: Path) -> set[str]:
    """The families a PDF actually draws with, subset tags removed."""
    r = run(["pdffonts", str(pdf)])
    out = set()
    for line in r.stdout.splitlines()[2:]:
        if not line.strip():
            continue
        name = line.split()[0]
        out.add(SUBSET.sub("", name))
    return out


def pages(pdf: Path) -> int:
    r = run(["pdfinfo", str(pdf)])
    return next((int(l.split()[1]) for l in r.stdout.splitlines() if l.startswith("Pages:")), 0)


def words(pdf: Path, page: int) -> int:
    """Tokens carrying at least one letter or digit -- `batch-check.sh`'s own rule, so the
    figures here and the corpus gate's are the same quantity."""
    r = run(["pdftotext", "-f", str(page), "-l", str(page), str(pdf), "-"])
    return len([w for w in r.stdout.split() if re.search(r"[^\W_]", w, re.UNICODE)])


def raster(pdf: Path, page: int, dpi: int, into: Path) -> Path | None:
    into.mkdir(parents=True, exist_ok=True)
    stem = into / f"p{page:04d}"
    hit = sorted(into.glob(f"p{page:04d}*.png"))
    if hit:
        return hit[0]
    run(["pdftoppm", "-r", str(dpi), "-f", str(page), "-l", str(page), "-gray", "-png",
         str(pdf), str(stem)])
    hit = sorted(into.glob(f"p{page:04d}*.png"))
    return hit[0] if hit else None



def profile_shift(a_png: Path, b_png: Path, axis: int, span: int = 60) -> tuple[int, float]:
    """The offset best aligning two ink profiles along one axis, and how much it helps.

    `compare-images.py` has `row_profile_shift` and a `shifted_tiles` detector, and **both
    look only for vertical displacement**. A purely horizontal shift therefore scores as
    `LOCALISED DIFFERENCE` -- high worst-tile, low mean error, zero shifted tiles -- which
    reads as "one small region is badly wrong" and sends the reader looking for a colour or
    a border. It cost this exercise a wrong call: page 7 of the WordArt catalogue was
    published as an example of *agreement* on a 0.38 mean-error reading, and is in fact the
    whole page 23 px to the left, which at 150 dpi is 11.04 pt -- the corpus-wide
    `wp:effectExtent` horizontal defect, arrived at from the other end.
    """
    import zlib  # noqa: F401  (read_png needs it via CMP)
    A = CMP.read_png(a_png); B = CMP.read_png(b_png)
    wa, ha = A.width, A.height
    wb, hb = B.width, B.height
    w, h = min(wa, wb), min(ha, hb)

    def prof(img, W, H):
        out = [0] * (W if axis == 0 else H)
        for y in range(H):
            for x in range(W):
                v = 255 - img.at(x, y)
                out[x if axis == 0 else y] += v
        return out

    pa, pb = prof(A, w, h), prof(B, w, h)
    n = min(len(pa), len(pb))
    base = sum(abs(pa[i] - pb[i]) for i in range(n)) / n
    best, at = base, 0
    for s in range(-span, span + 1):
        if s == 0:
            continue
        acc = cnt = 0
        for i in range(n):
            j = i + s
            if 0 <= j < n:
                acc += abs(pa[j] - pb[i]); cnt += 1
        if cnt:
            e = acc / cnt
            if e < best:
                best, at = e, s
    return at, (0.0 if base == 0 else 1.0 - best / base)


def page_verdict(m: dict, dw: int, hshift: int = 0) -> str:
    """One page, one reference. Deliberately coarser than `diagnose()` and about a different
    question: not "how do these images differ" but "is this ours to fix, and can a number
    settle it".
    """
    ink = abs(m.get("ink_delta", 0.0))
    shifted = m.get("shifted_tiles", 0)
    worst = m.get("max_tile_error", 0.0)
    mae = m.get("mean_abs_error", 0.0)

    if dw != 0 and ink > 0.002:
        return "content-differs"          # words and ink agree that something is absent or extra
    if dw != 0:
        return "text-layer-differs"       # words move but ink does not: an extraction-level defect
    if shifted > 0 and ink <= 0.002:
        return "displaced-vertical"       # the same ink lower or higher -- a layout cascade
    if hshift != 0 and ink <= 0.002:
        return "displaced-horizontal"     # the same ink left or right; no existing metric sees this
    if worst >= 0.15 and mae < 0.01:
        return "localised"                # one small region badly wrong; an aggregate hides it
    if mae < 0.004 and ink <= 0.002:
        return "match"
    return "needs-eyes"                   # signals do not agree; no scalar settles it


def compare_pair(ours: Path, ref: Path, dpi: int, work: Path, want: list[int]) -> dict:
    """Per-page metrics and verdicts for one (ours, reference) pair."""
    n = min(pages(ours), pages(ref))
    out = {"pages_ours": pages(ours), "pages_ref": pages(ref), "pages": {}}
    for p in want or range(1, n + 1):
        if p > n:
            continue
        a = raster(ours, p, dpi, work / "ours")
        b = raster(ref, p, dpi, work / ref.parent.name)
        if a is None or b is None:
            continue
        m = CMP.compare(CMP.read_png(a), CMP.read_png(b))
        dw = words(ours, p) - words(ref, p)
        # Only ask the horizontal question when the vertical detector has already said no:
        # it is the case that would otherwise be misread, and the profile is not free.
        hshift, hgain = (0, 0.0)
        if m["shifted_tiles"] == 0 and m["max_tile_error"] >= 0.05:
            hshift, hgain = profile_shift(a, b, axis=0)
            if hgain < 0.15:
                hshift = 0
        out["pages"][p] = {
            "words_ours": words(ours, p), "words_ref": words(ref, p), "words_delta": dw,
            "ink_delta": round(m["ink_delta"], 4), "shifted_tiles": m["shifted_tiles"],
            "row_shift_px": m["row_profile_shift"], "max_tile_error": round(m["max_tile_error"], 4),
            "mean_abs_error": round(m["mean_abs_error"], 4),
            "col_shift_px": hshift,
            "verdict": page_verdict(m, dw, hshift), "diagnosis": CMP.diagnose(m).split(" - ")[0],
        }
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("document", type=Path)
    ap.add_argument("--dpi", type=int, default=150)
    ap.add_argument("--out", type=Path, default=Path("/tmp/verdict"))
    ap.add_argument("--pages", default="", help="e.g. 1-5 or 3,7,18; default every page")
    ap.add_argument("--json", type=Path)
    args = ap.parse_args()

    want: list[int] = []
    for part in filter(None, args.pages.split(",")):
        if "-" in part:
            lo, hi = part.split("-"); want += list(range(int(lo), int(hi) + 1))
        else:
            want.append(int(part))

    src = args.document.resolve()
    work = args.out / src.stem
    ours = render_ours(src, work / "ours-pdf")
    if ours is None:
        print(f"{src.name}: OUR RENDER FAILED"); return 2

    report: dict = {"document": src.name, "dpi": args.dpi, "references": {}}
    for tag, binary in (("24.2", LO24), ("26.2", LO26)):
        ref = render_lo(binary, src, work / f"ref{tag}")
        if ref is None:
            report["references"][tag] = {"status": "reference-cannot-render"}
            continue

        # Comparability first. A face set that differs is a void comparison, not a defect.
        fo, fr = faces(ours), faces(ref)
        only_ref, only_ours = sorted(fr - fo), sorted(fo - fr)

        # A *swap* -- each side drawing with a family the other never uses -- is a void
        # comparison: the ink measures the face. An extra family on one side only is a
        # fallback for some glyph the other side had covered, which is worth saying and is
        # not a reason to refuse to score. The distinction was added because the blunt rule
        # called `033_Event_planning_tracker` unscoreable against 24.2 on one extra fallback
        # face, which is a note, not a void.
        if only_ref and only_ours:
            report["references"][tag] = {
                "status": "unscoreable-font",
                "reference_only": only_ref, "ours_only": only_ours,
            }
            continue
        scored = {"status": "scored", **compare_pair(ours, ref, args.dpi, work, want)}
        if only_ref or only_ours:
            scored["font_note"] = {"reference_only": only_ref, "ours_only": only_ours}
        report["references"][tag] = scored

    # The document-level reading is a function of both references, never one.
    r24, r26 = report["references"].get("24.2", {}), report["references"].get("26.2", {})
    p24, p26 = r24.get("pages_ref"), r26.get("pages_ref")
    po = r24.get("pages_ours") or r26.get("pages_ours")
    if r24.get("status") == "unscoreable-font" or r26.get("status") == "unscoreable-font":
        overall = "unscoreable-font"
    elif p24 is not None and p26 is not None and p24 != p26:
        overall = "references-disagree"
    elif p26 is not None and po == p26 and p24 is not None and po != p24:
        overall = "matches-26.2-only"
    else:
        verdicts = {v["verdict"] for r in (r24, r26) if r.get("status") == "scored"
                    for v in r["pages"].values()}
        for rank in ("content-differs", "text-layer-differs", "displaced-vertical",
                     "displaced-horizontal", "needs-eyes", "localised", "match"):
            if rank in verdicts:
                overall = rank; break
        else:
            overall = "no-pages-scored"
    report["verdict"] = overall

    # Print the card.
    print(f"\n{src.name}   ->  {overall.upper()}")
    for tag in ("24.2", "26.2"):
        r = report["references"].get(tag, {})
        st = r.get("status")
        if st == "unscoreable-font":
            print(f"  vs {tag}: UNSCOREABLE - the two sides resolve different faces")
            if r["reference_only"]: print(f"      reference only: {', '.join(r['reference_only'])}")
            if r["ours_only"]:      print(f"      ours only:      {', '.join(r['ours_only'])}")
            continue
        if st != "scored":
            print(f"  vs {tag}: {st}"); continue
        print(f"  vs {tag}: pages {r['pages_ours']}/{r['pages_ref']}")
        if "font_note" in r:
            fn = r["font_note"]
            side = f"ours falls back to {', '.join(fn['ours_only'])}" if fn["ours_only"] \
                else f"the reference falls back to {', '.join(fn['reference_only'])}"
            print(f"      note: {side} -- one side only, so scored, but the ink carries it")
        print(f"      {'pg':>4} {'verdict':<21} {'dwords':>7} {'ink':>8} {'shifted':>8} {'dy':>6} {'dx':>6} {'worst':>7}")
        for p, v in sorted(r["pages"].items()):
            if v["verdict"] == "match":
                continue
            print(f"      {p:>4} {v['verdict']:<21} {v['words_delta']:>7} {v['ink_delta']:>8} "
                  f"{v['shifted_tiles']:>8} {v['row_shift_px']:>4}px {v.get('col_shift_px', 0):>4}px "
                  f"{v['max_tile_error']:>7}")
        clean = sum(1 for v in r["pages"].values() if v["verdict"] == "match")
        print(f"      ({clean} of {len(r['pages'])} pages match)")

    eyes = sorted({p for r in report["references"].values() if r.get("status") == "scored"
                   for p, v in r["pages"].items() if v["verdict"] in ("needs-eyes", "localised")})
    if eyes:
        print(f"\n  ASK FOR EYES on page(s) {', '.join(map(str, eyes[:12]))}"
              f"{' ...' if len(eyes) > 12 else ''} - no scalar settles these:")
        print(f"      .claude/skills/page-vision/scripts/pair.sh \"{src.stem}__{src.suffix.lstrip('.')}\" "
              f"--page {eyes[0]} --outdir /abs/pairs")

    if args.json:
        args.json.write_text(json.dumps(report, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
