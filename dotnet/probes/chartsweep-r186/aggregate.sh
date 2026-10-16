#!/usr/bin/env bash
# Per chart-type scores over the sweep's rows.
#
#   ./aggregate.sh [rows.tsv]
#
# Columns: kinds, path, worst |ink|%, summed |ink|%, pages, worst diff%, status.
#
# **Rank on `diff%`, not on `|ink|%`.** Both are fractions of the whole PAGE, and a chart is
# a small part of one: a 3-D pie drawn as a flat circle on an A4 page that is otherwise white
# scores 0.30 unsigned ink and 2.76 differing pixels. `|ink|%` is additionally *signed before
# the absolute value is taken per page*, so ink we add where the reference removes it cancels
# within a page -- which is exactly what a wrong-shaped pie of the right colours does.
#
# Grouped by the *set* of plot elements a document's chart parts state, so a combination chart
# is its own row: the ink of a page holding two plot kinds cannot be attributed to one of them.
set -uo pipefail
ROWS=${1:-$(dirname "$0")/rows.tsv}
printf '%-56s %4s %11s %11s\n' 'plot elements stated' 'docs' 'mean diff%' 'worst diff%'
awk -F'\t' '$7=="ok"{n[$1]++; s[$1]+=$6; if($6+0>m[$1])m[$1]=$6+0;
                     si[$1]+=$3; if($3+0>mi[$1])mi[$1]=$3+0}
  END{for(k in n) printf "%-56s %4d %11.2f %11.2f   (|ink| mean %.2f worst %.2f)\n",
        k, n[k], s[k]/n[k], m[k], si[k]/n[k], mi[k]}' "$ROWS" | sort -k4 -g -r
