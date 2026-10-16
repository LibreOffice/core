#!/bin/bash
# For each named document: render the 26.2.4.2 reference once, then score the base and the
# head renderings the reach sweep saved against it, span for span.
#
# The reference is rendered here and nowhere else in this round, so the run announces which
# binary it is (dotnet/CLAUDE.md: "a sweep takes the reference from PATH and says nothing").
set -u
LIST="$1"; MOVERS="$2"; REFDIR="$3"; OUT="$4"; JOBS="${5:-2}"
HERE="$(cd "$(dirname "$0")" && pwd)"
REF="${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}"
echo "reference $REF -- $("$REF" --version 2>/dev/null | head -1)" >&2
mkdir -p "$REFDIR"
: > "$OUT"

one() {
    doc="$1"
    key=$(printf '%s' "$doc" | md5sum | cut -c1-16)
    d="$MOVERS/$key"
    [ -f "$d/base.pdf" ] || return
    stem=$(basename "${doc%.*}")
    rd="$REFDIR/$key"
    if [ ! -f "$rd/$stem.pdf" ]; then
        "$HERE/ref-render.sh" "$doc" "$rd" || { echo -e "$doc\tref-failed" >> "$OUT"; return; }
    fi
    read hb hh n < <(python3 "$HERE/score.py" "$rd/$stem.pdf" "$d/base.pdf" "$d/head.pdf" 2>/dev/null)
    [ -z "${n:-}" ] && { echo -e "$doc\tscore-failed" >> "$OUT"; return; }
    echo -e "$doc\t$hb\t$hh\t$n" >> "$OUT"
}
export -f one
export MOVERS REFDIR OUT HERE

xargs -d '\n' -a "$LIST" -I{} -P "$JOBS" bash -c 'one "$@"' _ {}
awk -F'\t' 'NF==4 {b+=$2; h+=$3; n+=$4; d++; if ($3>$2) up++; else if ($3<$2) down++; else lev++}
     END {printf "documents %d  spans %d  base-agreeing %d  head-agreeing %d  better %d  worse %d  level %d\n", d, n, b, h, up, down, lev}' "$OUT"
