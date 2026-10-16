#!/usr/bin/env bash
# Sanity-check a sweep before believing it. The skill's two rules: the row count must equal
# the track size exactly, and no path may appear twice.
S=/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad
D="$S/${1:?usage: sl14-sanity.sh <outdir-name>}"
echo "rows.tsv:  $(wc -l < "$D/rows.tsv")   (must be 163)"
echo "ink.tsv:   $(wc -l < "$D/ink.tsv")    (must be 163)"
dupes=$(cut -f1 "$D/rows.tsv" | sort | uniq -d)
if [ -n "$dupes" ]; then echo "DUPLICATE PATHS:"; echo "$dupes"; else echo "no duplicate paths"; fi
echo "other tracks present (must be 0): $(grep -cv '^slides/' "$D/rows.tsv")"
echo
echo "verdicts:"
awk -F'\t' '{c[$7]++} END{for (v in c) printf "  %-24s %d\n", v, c[v]}' "$D/rows.tsv" | sort -k2 -rn
echo
echo "page counts exact: $(awk -F'\t' '{split($3,p,"/"); if (p[1]==p[2]) n++} END{print n}' "$D/rows.tsv") / 163"
awk -F'\t' '$3!="-" && $3!="?" {i+=$3; m+=$4; n++}
            END{printf "INK %.2f  MAJOR %d  over %d documents\n", i, m, n}' "$D/ink.tsv"
