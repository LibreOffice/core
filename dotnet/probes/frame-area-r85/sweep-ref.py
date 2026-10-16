#!/usr/bin/env python3
"""Render the reference half of a corpus range, in `batch-check.sh`'s file naming.

Separated from our own half because the two have different costs and different lifetimes: the
reference is what a diff confined to `dotnet/src` cannot reach, so one reference sweep serves
every binary a round builds and a *before* and an *after* verdict cost one reference render
between them rather than two.

It prints the resolved binary and its version first, because a sweep that does not record
which of the installed LibreOffices it used cannot be attributed to a reference at all.

Usage:  REF_SOFFICE=<abs path> python3 sweep-ref.py <corpus-root> <glob> <outdir> [workers]
"""
import hashlib, os, shutil, subprocess, sys
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
    soffice = os.environ.get("REF_SOFFICE", "soffice")
    version = subprocess.run([soffice, "--version"], capture_output=True, text=True).stdout.strip()

    files = sorted(p for d in sorted(root.glob(glob)) if d.is_dir()
                   for p in d.rglob("*") if p.is_file() and p.suffix.lower() in EXTENSIONS)
    (out / "ref").mkdir(parents=True, exist_ok=True)
    print(f"reference {soffice} -- {version}\n{len(files)} documents into {out}", flush=True)

    def render(item):
        index, src = item
        target = out / "ref" / f"{src.stem}__{src.suffix.lower().lstrip('.')}.pdf"
        if target.exists():
            return True
        work = out / "t" / f"d{index}"
        if work.exists():
            shutil.rmtree(work)
        work.mkdir(parents=True)
        # The profile path is keyed on a hex digest because `soffice` truncates
        # `-env:UserInstallation` at the first space, and `timeout` needs `-k`: `soffice` execs
        # `oosplash`, which ignores SIGTERM, so a bare timeout waits for ever.
        profile = work / ("p" + hashlib.sha1(str(src).encode()).hexdigest()[:12])
        subprocess.run(["timeout", "-k", "30", "240", soffice,
                        f"-env:UserInstallation=file://{profile}", "--headless", "--norestore",
                        "--convert-to", "pdf", "--outdir", str(work), str(src)],
                       capture_output=True)
        made = work / (src.stem + ".pdf")
        ok = made.exists()
        if ok:
            made.replace(target)
        shutil.rmtree(work, ignore_errors=True)
        return ok

    with ThreadPoolExecutor(max_workers=workers) as pool:
        results = list(pool.map(render, enumerate(files)))
    shutil.rmtree(out / "t", ignore_errors=True)
    print(f"rendered {sum(results)} of {len(files)}")


main()
