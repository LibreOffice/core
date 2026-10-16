"""Score a sweep against the gate's stored verdicts and name every document that moved."""
import csv, sys
from collections import Counter

# The sweep's first line is the validation note, which is a comment rather than a column header.
lines = [l for l in open(sys.argv[1]) if not l.startswith("#")]
rows = [r for r in csv.DictReader(lines, delimiter="\t") if r.get("path")]

tracks = Counter()
moved = []
for r in rows:
    track = r["path"].split("/")[0]
    tracks[(track, r["verdict"])] += 1
    if r["verdict"] != r["was"]:
        moved.append(r)

print(f"rows scored: {len(rows)}")
for track in ("words", "slides", "sheets"):
    got = {v: c for (t, v), c in tracks.items() if t == track}
    total = sum(got.values())
    print(f"  {track:7s} {total:4d}  " + "  ".join(f"{k} {v}" for k, v in sorted(got.items())))

print(f"\nverdicts that moved: {len(moved)}")
for r in moved:
    print(f"  {r['was']:12s} -> {r['verdict']:12s}  pages {r['pages']:>10s}  "
          f"glyphs {r['glyphs']:>13s}  {r['path']}")
