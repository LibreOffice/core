#!/usr/bin/env python3
"""Record one per-document reading. Kept as one file per rank so the pass is resumable."""
import json, pathlib, sys
rank = int(sys.argv[1])
text = sys.stdin.read().strip()
p = pathlib.Path("/data/bench/analysis") / f"{rank:03d}.json"
p.write_text(json.dumps({"rank": rank, "text": text}))
print(f"{rank} ok ({len(text)} chars)")
