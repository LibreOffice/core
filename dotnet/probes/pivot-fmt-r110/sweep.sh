#!/bin/bash
# sweep.sh <list> <base-cli> <head-cli> <out.tsv>
#
# Render every document of the list at two binaries, hash each PDF, delete it again, and say
# which documents moved. SOURCE_DATE_EPOCH is fixed so a header printing the date does not read
# as a mover; %%EOF is checked before hashing so a render truncated by a full disk is reported as
# `failed` rather than scoring as a difference; and the sweep stops itself while there is still
# room to write, because on a full disk deletes keep succeeding and writes do not.
set -u
LIST=$1; BASE=$2; HEAD=$3; OUT=$4
export SOURCE_DATE_EPOCH=1700000000
printf 'document\tbase\thead\tmoved\n' > "$OUT"
while IFS= read -r doc; do
    [ -n "$doc" ] || continue
    free=$(df -Pk / | awk 'NR==2{print $4}')
    if [ "$free" -lt 1048576 ]; then
        printf 'STOPPED\tless than 1 GiB free\t-\t-\n' >> "$OUT"; break
    fi
    name=$(basename "$doc")
    tb=$(mktemp -d); th=$(mktemp -d)
    "$BASE/XRay.Office.Cli" render --format pdf --outdir "$tb" --quiet "$doc" >/dev/null 2>&1
    "$HEAD/XRay.Office.Cli" render --format pdf --outdir "$th" --quiet "$doc" >/dev/null 2>&1
    pb=$(find "$tb" -name '*.pdf' | head -1); ph=$(find "$th" -name '*.pdf' | head -1)
    hb=failed; hh=failed
    [ -n "$pb" ] && tail -c 2048 "$pb" | grep -q '%%EOF' && hb=$(md5sum "$pb" | cut -d' ' -f1)
    [ -n "$ph" ] && tail -c 2048 "$ph" | grep -q '%%EOF' && hh=$(md5sum "$ph" | cut -d' ' -f1)
    m=no; [ "$hb" = "$hh" ] || m=YES
    printf '%s\t%s\t%s\t%s\n' "$name" "$hb" "$hh" "$m" >> "$OUT"
    rm -rf "$tb" "$th"
done < "$LIST"
