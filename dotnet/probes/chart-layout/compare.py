"""Side by side ink, before against after, over the documents a change can reach."""
import csv, sys

def rows(p):
    return {r["path"]: r for r in csv.DictReader(open(p), delimiter="\t")}

a, b = rows(sys.argv[1]), rows(sys.argv[2])
print(f"{'mean':>16}  {'worst':>16}   document")
tot_a = tot_b = 0.0
better = worse = same = 0
for k in a:
    if k not in b:
        continue
    try:
        ma, mb = float(a[k]["inkmean"]), float(b[k]["inkmean"])
        wa, wb = float(a[k]["inkworst"]), float(b[k]["inkworst"])
    except (ValueError, KeyError):
        print(f"{'-':>16}  {'-':>16}   {k}  (unscoreable)")
        continue
    tot_a += ma
    tot_b += mb
    mark = " " if abs(mb - ma) < 0.005 else ("+" if mb < ma else "!")
    if mark == "+":
        better += 1
    elif mark == "!":
        worse += 1
    else:
        same += 1
    print(f"{ma:7.2f} -> {mb:6.2f}  {wa:7.2f} -> {wb:6.2f} {mark} {k}")
print(f"\nsum of means {tot_a:.2f} -> {tot_b:.2f}   better {better}  worse {worse}  unchanged {same}")
