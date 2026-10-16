#!/usr/bin/env python3
"""Render every authored variant already rendered by 26.2.4.2 with *our* CLI and compare
the page-shape sequences side by side. The DOCX files are the same bytes on both sides."""
from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import sweep as S  # noqa: E402

CLI = os.environ["XRAY_OFFICE_CLI"]
ENV = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")


def ours(docx: Path, out: Path) -> str:
    # `xray-office render` names its output <stem>.pdf in --outdir, which is exactly the name
    # soffice used, so ours must go to a directory of its own or one silently overwrites the
    # other and the comparison reads as perfect agreement.
    d = out / "ours"
    d.mkdir(exist_ok=True)
    pdf = d / (docx.stem + ".pdf")
    pdf.unlink(missing_ok=True)
    r = subprocess.run([CLI, "render", str(docx), "--outdir", str(d), "--quiet"],
                       env=ENV, capture_output=True, text=True, timeout=300)
    if not pdf.exists():
        return f"RENDER-FAILED({r.returncode}:{(r.stderr or r.stdout).strip()[:60]})"
    return S.shapes(pdf)


if __name__ == "__main__":
    bad = 0
    for d in sorted(Path(p) for arg in sys.argv[1:] for p in Path(arg).glob("*.docx")):
        ref = d.with_suffix(".pdf")
        r = S.shapes(ref) if ref.exists() else "MISSING"
        o = ours(d, d.parent)
        flag = "" if r == o else "   <<< DIFFERS"
        if r != o:
            bad += 1
        print(f"{d.parent.name}/{d.stem}\tref={r}\tours={o}{flag}")
    print(f"# differing: {bad}")
