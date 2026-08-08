#!/usr/bin/env bash
# What group scales does PptSlideLayout's text branch actually see?
#
#     group-scale-census.sh <worktree> <scratch>
#
# `PptSlideLayout.Text` splits on `placement is { B: 0, C: 0, A: > 0, D: > 0 }`: an upright
# placement has its text *rectangle* mapped through the matrix and the text laid out again
# inside it at its own font size, while a rotated or mirrored one keeps its local rectangle and
# carries the matrix. The comment defending that split cites "this format's commonest group — a
# client anchor one master unit wider than the union of its children's — the factor is 1.00025",
# which had been read and never measured for five rounds.
#
# This instruments the branch, renders every `.ppt` in the slides corpus, and folds the factors.
# The instrumentation is temporary: applied, built into a snapshot CLI, reverted, and the tree
# rebuilt, so nothing here is ever committed and no measured run is made against a tree that
# still carries it.
#
# Measured at d7fd6cf13 over the corpus's 51 `.ppt`:
#
#   7555 text shapes, 51 documents
#     rotated or mirrored branch          41  over 8 documents
#     upright branch                    7514
#       exactly 1                       7364  over 51 documents
#       under 0.1%                         0
#       0.1% - 1%                          3  over 1 document
#       1% - 10%                          31  over 4 documents
#       10% or more                      116  over 10 documents
#
# So the comment's premise is wrong in both directions: nothing in the corpus sits near
# 1.00025 — the whole band under 0.1% is empty — and the branch is not a rounding guard but is
# load-bearing on 150 text shapes across 11 documents, at factors from 0.00063 to 1.385.
#
# The reading it defends is nonetheless right, which is the separate half of the question and
# needs a document rather than a histogram. `hofman.ppt` carries the largest ordinary factor,
# A = 1.1163 and D = 1.0315 on 34 shapes; the reference draws 24.01, 28.01, 32.00 and 43.99 pt
# there and so do we. A group scale that reached the font would have put those at 26.8, 31.3,
# 35.7 and 49.1.
set -uo pipefail
W="${1:?usage: group-scale-census.sh <worktree> <scratch>}"
S="${2:?}"
F="$W/dotnet/src/Paperless.Presentations/MsBinary/PptSlideLayout.cs"

restore() { git -C "$W" checkout -- dotnet/src/Paperless.Presentations/MsBinary/PptSlideLayout.cs; }
trap restore EXIT
restore

python3 - "$F" <<'PY'
import sys
p = sys.argv[1]; t = open(p).read()
a = "        bool upright = placement is { B: 0, C: 0, A: > 0, D: > 0 };\n        DocRect area = upright"
b = ("        bool upright = placement is { B: 0, C: 0, A: > 0, D: > 0 };\n"
     "        Console.Error.WriteLine(\n"
     "            $\"GROUPSCALE\\t{placement.A:F6}\\t{placement.D:F6}\\t{placement.B:F6}\\t\"\n"
     "            + $\"{placement.C:F6}\\t{upright}\\t{rectangle.Width.Emu}\\t{rectangle.Height.Emu}\");\n"
     "        DocRect area = upright")
assert t.count(a) == 1
open(p, 'w').write(t.replace(a, b))
PY

mkdir -p "$S"
( cd "$W/dotnet" && dotnet build tools/Paperless.Cli/Paperless.Cli.csproj -c Debug ) > "$S/build.log" 2>&1 \
  || { echo "instrumented build failed"; tail -20 "$S/build.log"; exit 1; }
rm -rf "$S/cli-probe"
cp -r "$W/dotnet/tools/Paperless.Cli/bin/Debug/net10.0/linux-x64" "$S/cli-probe"
restore
# Back to a clean tree *before* anything is measured, and rebuilt: a --no-build run after an
# instrumentation cycle measures the instrumentation.
( cd "$W/dotnet" && dotnet build Paperless.slnx -c Debug ) > "$S/rebuild.log" 2>&1 \
  || { echo "rebuild failed"; tail -20 "$S/rebuild.log"; exit 1; }

rm -rf "$S/groupscale"; mkdir -p "$S/groupscale/out"
: > "$S/groupscale/raw.tsv"
while IFS= read -r f; do
  timeout 300 "$S/cli-probe/Paperless.Cli" render "$f" --format pdf --outdir "$S/groupscale/out" \
    2>&1 >/dev/null | grep '^GROUPSCALE' | sed "s|^|${f##*/}\t|" >> "$S/groupscale/raw.tsv"
  rm -f "$S/groupscale/out"/*.pdf
done < <(find /workspace/sample-files/slides -type f -iname '*.ppt' | sort)

python3 - "$S/groupscale/raw.tsv" <<'PY'
import collections, sys
rows = [l.rstrip('\n').split('\t') for l in open(sys.argv[1])]
rows = [r for r in rows if len(r) >= 8]
rot = [r for r in rows if r[6] != 'True']
pos = [r for r in rows if r[6] == 'True']
print(f'{len(rows)} text shapes, {len({r[0] for r in rows})} documents')
print(f'  rotated or mirrored branch  {len(rot):6}  over {len({r[0] for r in rot})} documents')
print(f'  upright branch              {len(pos):6}')
b = collections.Counter(); docs = collections.defaultdict(set); pairs = collections.Counter()
for r in pos:
    a, d = float(r[2]), float(r[3]); off = max(abs(a - 1), abs(d - 1))
    k = ('exactly 1' if off == 0 else 'under 0.1%' if off < 1e-3
         else '0.1% - 1%' if off < 1e-2 else '1% - 10%' if off < 0.1 else '10% or more')
    b[k] += 1; docs[k].add(r[0])
    if off > 0: pairs[(round(a, 5), round(d, 5), r[0])] += 1
for k in ['exactly 1', 'under 0.1%', '0.1% - 1%', '1% - 10%', '10% or more']:
    print(f'    {k:14} {b[k]:6}  over {len(docs[k])} documents')
print('  the non-unit factors, by document:')
for (a, d, doc), n in pairs.most_common(15):
    print(f'    A={a:<9} D={d:<9} x{n:<4} {doc}')
PY
