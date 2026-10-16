#!/usr/bin/env python3
"""Which icon-theme asset 26.2.4.2 actually paints, established rather than assumed.

`drawIconSets` (`sc/source/ui/view/output.cxx`:960-989, this tree) draws a `Bitmap` loaded by
name from whichever icon theme the application is running, so the *artwork* is nowhere in the
document and the reference's own renderings are the only place it can be read from. This is that
reading, in three steps:

1. every image at most 32 x 32 in the banked 26.2.4.2 PDFs of the corpus documents that state an
   `iconSet` rule is extracted **with its soft mask applied** -- the base image is RGB with the
   alpha in a separate `/SMask`, so compositing is what makes it comparable at all, and skipping
   that step reads a transparent background as black and matches nothing;
2. the composited pixels are hashed and counted per document, which is the corpus's own glyph
   census;
3. each distinct image is compared against every `icon-set-*` entry of all twenty `images_*.zip`
   the reference installs, by mean absolute channel difference over the composited RGBA.

Usage: reference-glyphs.py [bank-directory]
"""
import collections
import glob
import hashlib
import io
import os
import sys

import numpy as np
import pymupdf
from PIL import Image

BANK = sys.argv[1] if len(sys.argv) > 1 else "/home/user/gate-orig-r83/ref"
THEMES = "/opt/libreoffice26.2/share/config/images_*.zip"

# The ten corpus documents that state an `iconSet` rule, from census-iconset.py.
DOCUMENTS = [
    "041_Business_budget_ef942467",
    "042_Business_monthly_budget_4e4d092f",
    "066_Agile_Gantt_chart_08f9de45",
    "069_Blue_modern_balance_sheet_Use_this_template_6b4d70d3",
    "075_Idea_planner_tasks_f44ebd73",
    "076_Inventory_list_accessibility_guide_Use_this_template_f43dab19",
    "077_Inventory_list_with_highlighting_Use_this_template_36d8d57a",
    "078_Modern_inventory_list_Use_this_template_41abd5bf",
    "088_To-do_list_with_progress_tracker_Use_this_template_8c438912",
    "sistem-rekod-markah-srm-_-rekod-master",
]


def composited(doc, xref):
    """The image as it is drawn: its own pixels under its soft mask."""
    pix = pymupdf.Pixmap(doc, xref)
    smask = doc.extract_image(xref).get("smask", 0)
    if smask:
        pix = pymupdf.Pixmap(pix, pymupdf.Pixmap(doc, smask))
    return pix.tobytes("png")


def rgba(data, size=None):
    image = Image.open(io.BytesIO(data)).convert("RGBA")
    if size and image.size != size:
        image = image.resize(size)
    return np.asarray(image, dtype=float)


def main():
    found = collections.defaultdict(collections.Counter)
    pixels = {}

    for name in DOCUMENTS:
        path = os.path.join(BANK, f"{name}__xlsx.pdf")
        if not os.path.exists(path):
            print(f"{name}: MISSING")
            continue
        doc = pymupdf.open(path)
        for page in doc:
            for info in page.get_image_info(xrefs=True):
                if info["width"] > 32 or info["height"] > 32:
                    continue
                data = composited(doc, info["xref"])
                digest = hashlib.sha1(data).hexdigest()[:12]
                found[name][digest] += 1
                pixels[digest] = data

    print(f"# 26.2.4.2's own renderings, {BANK}")
    print(f"# {len(DOCUMENTS)} corpus documents state an iconSet rule; "
          f"{sum(1 for d in DOCUMENTS if found[d])} of them draw an icon.")
    print()
    total = 0
    for name in DOCUMENTS:
        counts = found[name]
        total += sum(counts.values())
        listed = ", ".join(f"{k} x{v}" for k, v in sorted(counts.items())) or "(none)"
        print(f"{name[:56]:56} {sum(counts.values()):3d}  {listed}")
    print(f"\ntotal icons {total}, distinct assets {len(pixels)}")

    print("\n# nearest icon-theme asset, by mean absolute channel difference out of 255,")
    print("# with the best any OTHER theme manages on the same image beside it")
    import zipfile
    for digest, data in sorted(pixels.items()):
        target = rgba(data)
        per_theme = {}
        for archive in sorted(glob.glob(THEMES)):
            theme = os.path.basename(archive)
            zf = zipfile.ZipFile(archive)
            for entry in zf.namelist():
                if "icon-set" not in entry or not entry.endswith(".png"):
                    continue
                try:
                    candidate = rgba(zf.read(entry), target.shape[1::-1])
                except Exception:
                    continue
                distance = float(np.abs(candidate - target).mean())
                if theme not in per_theme or distance < per_theme[theme][0]:
                    per_theme[theme] = (distance, entry)
        ranked = sorted((v[0], t, v[1]) for t, v in per_theme.items())
        best = ranked[0]
        other = next(r for r in ranked[1:] if r[1] != best[1])
        print(f"{digest}  {best[0]:6.3f}  {best[1]}  {best[2]}"
              f"   next theme {other[0]:6.3f}  {other[1]}")


if __name__ == "__main__":
    main()
