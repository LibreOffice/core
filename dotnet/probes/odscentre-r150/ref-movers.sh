#!/bin/sh
# Render the movers through 26.2.4.2 so the two legs can be scored against it, not only against
# each other. One profile per document, and the -env:UserInstallation is ABSOLUTE: a relative one
# does not fail, it HANGS past its own timeout (measured at 167 s with nothing written).
#
# `set -e` is deliberately NOT used. The first cut built the key/path table with a shell loop whose
# last statement was a failing `[ ... ] && echo`, so the loop's exit status was 1 and `set -e` tore
# the script down AFTER the table was written and BEFORE a single render -- silently, because the
# only output was `time`'s. The table is built in Python now, which is also 50 s faster.
cd "$(dirname "$0")"
OUT=/home/user/odscentre-r150-sweep/ref
mkdir -p "$OUT"

python3 - <<'PY'
import hashlib, pathlib
keys = {m for m in pathlib.Path('movers.txt').read_text().split() if m}
rows = []
for path in sorted(pathlib.Path('/home/user/corpus-odf/ods').glob('*.ods')):
    key = hashlib.md5(str(path).encode()).hexdigest()[:12]
    if key in keys:
        rows.append('%s\t%s' % (key, path))
pathlib.Path('movers-paths.tsv').write_text('\n'.join(rows) + '\n')
print('%d of %d movers resolved to a path' % (len(rows), len(keys)))
PY

cut -f2 movers-paths.tsv | xargs -P 4 -I{} sh -c '
    key=$(printf %s "{}" | md5sum | cut -c1-12)
    d="'"$OUT"'/$key"
    mkdir -p "$d"
    timeout -k 30 900 /opt/libreoffice26.2/program/soffice --headless --norestore \
        -env:UserInstallation="file:///tmp/r150-ref-$key" \
        --convert-to pdf --outdir "$d" "{}" >/dev/null 2>&1
    ls "$d"/*.pdf >/dev/null 2>&1 || echo "FAILED $key"
'
echo "rendered $(find "$OUT" -name '*.pdf' | wc -l) of $(wc -l < movers-paths.tsv)"
