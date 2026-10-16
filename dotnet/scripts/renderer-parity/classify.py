#!/usr/bin/env python3
"""Put every one of the 192 catalogue documents in exactly one bucket.

The question this answers is "is anything still open, and if so why", so the
buckets are ordered by how much they let us off the hook and each document
takes the FIRST that applies -- a document that is both a version divergence
and a reflow is a version divergence, because the reflow is not ours to fix
against this reference.

Nothing here is a claim about coverage. `fixed` means the rendering measurably
moved toward the reference between the sweep's binary and this one; it does not
mean a patch was written for that document, and a document a patch was written
for that did not move is not counted as fixed.
"""
import json, pathlib, sys

sys.path.insert(0, "/data/bench/scripts")
import zorder

BENCH = pathlib.Path("/data/bench")
cases = {c["rank"]: c for c in json.loads((BENCH / "pl-cases.json").read_text())}
rescore = {r["rank"]: r for r in json.loads((BENCH / "rescore-final.json").read_text())}

# open causes, in the order they are checked; each is a real seat, not a symptom
CAUSE = [
    ("chart",            "charts -- data labels, legend keys, axis wrapping, series z order"),
    ("shape-fallback",   "preset shape geometry: a sub-path's own fill/stroke, chevrons and ellipses"),
    ("missing-graphics", "a drawing we do not draw: watermarks, anchored artwork, picture bullets"),
    ("list-markers",     "list markers -- custom bullets and numbering the reader does not reach"),
    ("field-values",     "field values: a field read but not recalculated, or resolved differently"),
    ("table-rules",      "table rules and shading"),
    ("overlap-clip",     "overlap and clipping"),
    ("text-style",       "character styling"),
    ("content-missing",  "content we do not draw"),
    ("pagination",       "page count only"),
]

def hidden(where, c):
    """Text blocks this build draws and then paints over, on the compared page."""
    return len(zorder.scan(BENCH / where / c["id"] / "out.pdf", c["page"]) or [])


def bucket(c):
    r = rescore.get(c["rank"], {})
    before, after = r.get("ssim_before"), r.get("ssim_after")
    moved = (before is not None and after is not None and after - before > 0.01)

    # SSIM alone UNDER-counts fixes, for the same reason `aggregate.py` says it
    # misranks defects. Drawing content that was previously invisible adds ink
    # that does not land pixel-perfect, so the score falls while the page gets
    # better: `Hazard Analysis Template.xls` went 0.791 -> 0.780 as its header
    # went from 0.120 pt to 7.920 against the reference's 7.887. So a
    # defect-specific measurement counts as evidence of a fix wherever one
    # exists, and here that is the paint-order census.
    was, now = hidden("pl", c), hidden("pl-final", c)
    if now < was:
        return "fixed", (f"text drawn and then painted over: {was} blocks -> {now}"
                         + (f"; SSIM {before:.3f} -> {after:.3f}" if before and after else ""))

    if c.get("version_divergence") or "version-divergence" in c["tags"]:
        return "version mismatch", c.get("version_divergence") or "reference is 24.2.7.2; the tree targets 26.2.4.2"
    if "lo-broken" in c["tags"]:
        return "LibreOffice bug", c.get("retag") or "the reference is the one at fault; our output is correct"
    if moved:
        return "fixed", f"SSIM {before:.3f} -> {after:.3f} against the same reference"
    for tag, why in CAUSE:
        if tag in c["tags"]:
            return "open", why
    return "open", "reflow: the advance-width divergence (architectural -- see dotnet/CLAUDE.md)"

out = []
for c in cases.values():
    kind, why = bucket(c)
    out.append({"rank": c["rank"], "name": c["name"], "family": c["family"],
                "tags": c["tags"], "bucket": kind, "why": why})

(BENCH / "classification.json").write_text(json.dumps(out, indent=1))

from collections import Counter
tally = Counter(o["bucket"] for o in out)
print(f"{len(out)} documents\n")
for k in ("fixed", "version mismatch", "LibreOffice bug", "open"):
    print(f"  {k:18s} {tally[k]:3d}")
print("\nopen, by cause:")
for why, n in Counter(o["why"] for o in out if o["bucket"] == "open").most_common():
    print(f"  {n:3d}  {why}")
