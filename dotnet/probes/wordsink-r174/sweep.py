#!/usr/bin/env python3
"""Render a list of documents with one renderer into one directory per document.

One directory per *document*, never per worker slot: a thread pool does not work consecutive
indices, so two live renders land in one slot's directory and one deletes the other's output.
That failure is silent -- the sweep reports fewer rows rather than an error.
"""
import concurrent.futures, hashlib, os, pathlib, subprocess, sys

SOFFICE = "/opt/libreoffice26.2/program/soffice"


def key(path):
    return hashlib.md5(str(path).encode()).hexdigest()[:16]


def render(path, root, cli):
    outdir = root / key(path)
    outdir.mkdir(parents=True, exist_ok=True)
    if next(outdir.glob("*.pdf"), None):
        return path, "cached"
    try:
        if cli:
            subprocess.run([cli, "render", str(path), "--outdir", str(outdir)], check=True,
                           capture_output=True, timeout=900,
                           env={**os.environ, "SOURCE_DATE_EPOCH": "0"})
        else:
            subprocess.run([SOFFICE, f"-env:UserInstallation=file://{outdir / 'p'}", "--headless",
                            "--norestore", "--convert-to", "pdf", "--outdir", str(outdir),
                            str(path)], check=True, capture_output=True, timeout=900)
    except Exception as error:
        return path, f"failed: {type(error).__name__}"
    return path, "ok" if next(outdir.glob("*.pdf"), None) else "no output"


def main():
    paths = [pathlib.Path(p.strip()) for p in open(sys.argv[1]) if p.strip()]
    root = pathlib.Path(sys.argv[2])
    cli = sys.argv[3] if len(sys.argv) > 3 and sys.argv[3] != "-" else None
    root.mkdir(parents=True, exist_ok=True)
    done = 0
    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
        for path, status in pool.map(lambda p: render(p, root, cli), paths):
            done += 1
            if status not in ("ok", "cached"):
                print(f"  {status}\t{path.name}", flush=True)
    print(f"{done} documents into {root}")


if __name__ == "__main__":
    main()
