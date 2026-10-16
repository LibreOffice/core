#!/usr/bin/env bash
# Score batch-check.sh's three gate columns for one rendering against the reference.
#
#   gate-changed.sh <base-sweep> <after-sweep> <refdir>
#
# Only renderings that differ between the two legs can move a verdict; the rest are
# byte-identical and their verdicts are identical by construction. So this scores the
# changed ones on both legs, using batch-check.sh's own arithmetic column for column:
# page count exact, extracted words inside a 2%-and-3-word band, and zero unembedded
# fonts.
set -uo pipefail

BASE=$1
AFTER=$2
REF=$3

verdict() {
    local ours=$1 ref=$2 op rp ow rw un v=""
    op=$(pdfinfo "$ours" 2>/dev/null | awk '/^Pages/{print $2}')
    rp=$(pdfinfo "$ref"  2>/dev/null | awk '/^Pages/{print $2}')
    ow=$(pdftotext "$ours" - 2>/dev/null | wc -w)
    rw=$(pdftotext "$ref"  - 2>/dev/null | wc -w)
    un=$(pdffonts "$ours" 2>/dev/null | tail -n +3 | awk 'NF>=8 && $(NF-4)=="no"' | wc -l)

    [ "$op" = "$rp" ] || v="pages"
    if [ "${rw:-0}" -gt 0 ] 2>/dev/null; then
        awk -v a="$ow" -v b="$rw" 'BEGIN{d=(a>b?a-b:b-a); exit !(d > b*0.02 && d > 3)}' \
            && v="${v:+$v,}words"
    elif [ "${ow:-0}" -gt 3 ]; then v="${v:+$v,}words"
    fi
    [ "${un:-0}" = "0" ] || v="${v:+$v,}unembedded"
    printf '%s\t%s/%s\t%s/%s\t%s' "${v:-match}" "$op" "$rp" "$ow" "$rw" "$un"
}

printf 'rendering\tbase verdict\tpages\twords\tunemb\tafter verdict\tpages\twords\tunemb\tmoved\n'
moved=0
total=0
for f in "$BASE"/*.pdf; do
    name=$(basename "$f")
    [ -f "$AFTER/$name" ] || continue
    cmp -s "$f" "$AFTER/$name" && continue
    [ -f "$REF/$name" ] || { printf '%s\tNO REFERENCE\n' "$name"; continue; }

    total=$((total + 1))
    b=$(verdict "$f" "$REF/$name")
    a=$(verdict "$AFTER/$name" "$REF/$name")
    bv=${b%%$'\t'*}
    av=${a%%$'\t'*}
    if [ "$bv" = "$av" ]; then m="no"; else m="YES"; moved=$((moved + 1)); fi
    printf '%s\t%s\t%s\t%s\n' "$name" "$b" "$a" "$m"
done

printf '\n%s of %s changed renderings moved their verdict\n' "$moved" "$total"
