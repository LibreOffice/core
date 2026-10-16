#!/usr/bin/env python3
"""Sweep `first-divergence.py`'s analysis over pre-rendered pairs, in parallel.

    divsweep.py <gate.tsv> <ours-dir> <out.tsv> [workers]

`first-divergence.py --corpus` re-renders both halves with the CLI and with `soffice`, which
is a whole second corpus sweep and, on this track, hours. Both halves already exist on disk:
ours from the gate run, the reference at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`
under the same per-format identity. This imports `analyse()` unmodified and feeds it those
files, so the classification is the skill's and only the plumbing is mine.
"""
from __future__ import annotations

import importlib.util
import sys
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path

SKILL = Path("/c/sandbox/workdir/libreoffice-core/.claude/skills/render-comparison/scripts/first-divergence.py")
REFS = Path("/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words")

spec = importlib.util.spec_from_file_location("fd", SKILL)
fd = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fd)


def ident(path: str) -> str:
    stem = Path(path).stem
    ext = Path(path).suffix.lower().lstrip(".")
    return f"{stem}__{ext}.pdf"


def one(job):
    path, verdict, oursdir = job
    o = Path(oursdir) / ident(path)
    r = REFS / ident(path)
    if not o.exists() or not r.exists():
        return (path, verdict, None)
    try:
        return (path, verdict, fd.analyse(o, r))
    except Exception as exc:  # noqa: BLE001 - a failed document must not kill the sweep
        return (path, verdict, {"error": repr(exc)})


def main() -> int:
    gate, oursdir, out = Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3])
    workers = int(sys.argv[4]) if len(sys.argv) > 4 else 6
    rows = [l.split("\t") for l in gate.read_text().splitlines() if l.strip()]
    rows = [r for r in rows if r[0] != "path"]
    jobs = [(r[0], r[6], str(oursdir)) for r in rows]
    lines = ["path\text\tverdict\tpages\tfirst_page\tof\tink\tdominant\tgdelta"
             "\tonly_ours\tonly_ref\tblank_ref\tcounts\ttext"]
    done = 0
    with ProcessPoolExecutor(max_workers=workers) as pool:
        for path, verdict, a in pool.map(one, jobs):
            done += 1
            if a is None:
                lines.append(f"{path}\t{Path(path).suffix.lower().lstrip('.')}\t{verdict}\tMISSING")
                print(f"[{done}] {path} MISSING", flush=True)
                continue
            if "error" in a:
                lines.append(f"{path}\t{Path(path).suffix.lower().lstrip('.')}\t{verdict}\tERROR\t{a['error']}")
                print(f"[{done}] {path} ERROR {a['error']}", flush=True)
                continue
            counts = ",".join(f"{k}={v}" for k, v in (a.get("counts") or {}).items() if v)
            lines.append("\t".join(str(x) for x in [
                path, Path(path).suffix.lower().lstrip('.'), verdict,
                f"{a['pages_ours']}/{a['pages_ref']}", a["first"] or "",
                min(a["pages_ours"], a["pages_ref"]), f"{a['ink']:.2f}",
                a["dominant"], a["gdelta"], a["only_ours"], a["only_ref"],
                a["blank_ref"], counts, a.get("text", "")]))
            print(f"[{done}] {path} first={a['first']} {a['dominant']}", flush=True)
            out.write_text("\n".join(lines) + "\n")
    out.write_text("\n".join(lines) + "\n")
    print(f"wrote {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
