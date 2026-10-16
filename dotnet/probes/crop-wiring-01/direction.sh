#!/usr/bin/env bash
# Which way each changed page moved, against the canonical reference renderings.
#
#   direction.sh <base-sweep> <after-sweep> <refdir> <workdir> <name> [<name> ...]
#
# Magnitude alone does not say whether a change is a fix. So every page of every
# rendering that differs between the two legs is rasterised three times — base, after
# and reference — and each leg's fraction of differing pixels against the reference is
# compared. A page is "closer" when the after leg differs from the reference less than
# the base leg did, "further" when it differs more, and "same" when neither moves by
# more than a thousandth of the page, which is the floor of the rasteriser's own noise.
#
# Reference pages come from /c/sandbox/workdir/refpdfs-26.2.4.2-fonts, rendered by
# LibreOffice 26.2.4.2 with the correct font set; they are reused rather than
# re-rendered, and a page count that disagrees with ours is reported rather than
# silently paired off, because a page inserted anywhere makes every later comparison
# meaningless.
set -uo pipefail

BASE=$1; AFTER=$2; REF=$3; WORK=$4; shift 4
DPI=${DPI:-100}
SCRIPTS=/c/sandbox/workdir/libreoffice-core/.claude/skills/render-comparison/scripts

mkdir -p "$WORK"
printf 'rendering\tpage\tbase%%\tafter%%\tdirection\n'

closer=0; further=0; same=0; skipped=0

for name in "$@"; do
    stem=${name%.pdf}
    for leg in base after ref; do
        case $leg in
            base) src="$BASE/$name" ;;
            after) src="$AFTER/$name" ;;
            ref) src="$REF/$name" ;;
        esac
        rm -rf "$WORK/$stem-$leg"; mkdir -p "$WORK/$stem-$leg"
        [ -f "$src" ] || continue
        pdftoppm -r "$DPI" -png "$src" "$WORK/$stem-$leg/page" 2>/dev/null
    done

    nb=$(find "$WORK/$stem-base" -name '*.png' | wc -l)
    na=$(find "$WORK/$stem-after" -name '*.png' | wc -l)
    nr=$(find "$WORK/$stem-ref" -name '*.png' | wc -l)

    # The two legs must agree with each other; neither has to agree with the reference.
    # A rendering whose page count already differs from LibreOffice's is the ordinary case
    # on this track — five of the seven here fail check 1 before this round touches them —
    # and refusing to measure those would throw away the whole result. Both legs are paired
    # against the reference the same way, page 1 to page 1, so whatever misalignment the
    # extra pages cause is identical on both sides and cancels out of the comparison. It is
    # reported rather than hidden.
    if [ "$nb" -eq 0 ] || [ "$nb" -ne "$na" ]; then
        printf '%s\t-\t-\t-\tSKIPPED legs disagree %s/%s\n' "$stem" "$nb" "$na"
        skipped=$((skipped + 1))
        rm -rf "$WORK/$stem-base" "$WORK/$stem-after" "$WORK/$stem-ref"
        continue
    fi
    [ "$nb" -eq "$nr" ] || printf '%s\t-\t-\t-\tNOTE ours %s pages, reference %s\n' \
        "$stem" "$nb" "$nr"

    python3 "$SCRIPTS/compare-images.py" --expected "$WORK/$stem-ref" \
        --actual "$WORK/$stem-base" --report "$WORK/$stem-base.md" >/dev/null 2>&1
    python3 "$SCRIPTS/compare-images.py" --expected "$WORK/$stem-ref" \
        --actual "$WORK/$stem-after" --report "$WORK/$stem-after.md" >/dev/null 2>&1

    while IFS=$'\t' read -r page b a dir; do
        printf '%s\t%s\t%s\t%s\t%s\n' "$stem" "$page" "$b" "$a" "$dir"
        case $dir in closer) closer=$((closer + 1)) ;;
                     further) further=$((further + 1)) ;;
                     *) same=$((same + 1)) ;; esac
    done < <(python3 - "$WORK/$stem-base.md" "$WORK/$stem-after.md" <<'PY'
import re, sys


def fractions(path):
    """page -> differing fraction, from compare-images.py's own markdown report.

    Its shape is a `## Page N` heading followed by a `| differing_fraction | f |`
    row, the fraction being 0..1 rather than a percentage. A page whose dimensions
    did not match emits no such row and is left out here, so a size change shows up
    as a missing page rather than as a zero.
    """
    out, page = {}, None
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'^## Page (\d+)', line)
        if m:
            page = int(m.group(1))
            continue
        m = re.match(r'^\|\s*differing_fraction\s*\|\s*([\d.]+)\s*\|', line)
        if m and page is not None:
            out[page] = float(m.group(1)) * 100
    return out


base, after = fractions(sys.argv[1]), fractions(sys.argv[2])
for page in sorted(set(base) & set(after)):
    b, a = base[page], after[page]
    if abs(a - b) < 0.01:
        d = 'same'
    else:
        d = 'closer' if a < b else 'further'
    print(f'{page}\t{b:.3f}\t{a:.3f}\t{d}')
PY
    )

    rm -rf "$WORK/$stem-base" "$WORK/$stem-after" "$WORK/$stem-ref"
done

printf '\nTOTAL\tcloser %d\tfurther %d\tsame %d\tskipped %d\n' \
    "$closer" "$further" "$same" "$skipped"
