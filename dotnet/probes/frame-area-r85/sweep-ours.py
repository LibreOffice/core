#!/usr/bin/env python3
"""Render one side of the corpus — ours — into `batch-check.sh`'s own file naming.

Two runs of this at two binaries byte-compare document for document, which answers *what
moved* far more precisely than a verdict can: a position defect adds no glyphs and no pages,
so the gate is blind to it and only the bytes say which renderings changed at all.

The naming is `batch-check.sh`'s `<stem>__<ext>.pdf`, so the output is interchangeable with a
gate sweep's `ours/` directory and can be scored against a banked reference half.

Each document gets its **own** temporary directory rather than one per worker slot: a thread
pool does not work consecutive indices, so two live renders land in one slot's directory and
one deletes the other's output. That is silent and reads as documents that failed to render.

Usage:  XRAY_OFFICE_CLI=<abs path> python3 sweep-ours.py <corpus-root> <glob> <outdir> [workers]
"""
import os, shutil, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

EXTENSIONS = {
    ".doc", ".docx", ".docm", ".dot", ".dotx", ".dotm", ".rtf", ".odt", ".ott", ".fodt", ".sxw",
    ".xls", ".xlsx", ".xlsm", ".xlsb", ".xlt", ".xltx", ".xltm", ".ods", ".ots", ".fods", ".csv",
    ".sxc", ".ppt", ".pptx", ".pptm", ".pot", ".potx", ".potm", ".ppsx", ".ppsm", ".pps", ".odp",
    ".otp", ".fodp", ".sxi",
}


def main():
    root, glob, out = Path(sys.argv[1]), sys.argv[2], Path(sys.argv[3])
    workers = int(sys.argv[4]) if len(sys.argv) > 4 else 3
    cli = os.environ["XRAY_OFFICE_CLI"]

    files = sorted(p for d in sorted(root.glob(glob)) if d.is_dir()
                   for p in d.rglob("*") if p.is_file() and p.suffix.lower() in EXTENSIONS)
    (out / "ours").mkdir(parents=True, exist_ok=True)
    print(f"measuring {cli}\n{len(files)} documents into {out}", flush=True)

    def render(item):
        index, src = item
        work = out / "t" / f"d{index}"
        if work.exists():
            shutil.rmtree(work)
        work.mkdir(parents=True)
        # `timeout -k` because a bare SIGTERM does not bound every renderer.
        subprocess.run(["timeout", "-k", "30", "240", cli, "render", str(src),
                        "--format", "pdf", "--outdir", str(work)],
                       capture_output=True)
        made = work / (src.stem + ".pdf")
        ok = made.exists()
        if ok:
            made.replace(out / "ours" / f"{src.stem}__{src.suffix.lower().lstrip('.')}.pdf")
        shutil.rmtree(work, ignore_errors=True)
        return ok

    with ThreadPoolExecutor(max_workers=workers) as pool:
        results = list(pool.map(render, enumerate(files)))
    shutil.rmtree(out / "t", ignore_errors=True)
    print(f"rendered {sum(results)} of {len(files)}")


main()
