#!/bin/sh
# Render the movers through 26.2.4.2 so the two legs can be scored against it. No `set -e`: the
# first cut of this script in `probes/odscentre-r150/` had one, and a failing `[ ... ] && echo` at
# the end of its table-building loop tore it down after the table and before a single render --
# silently. Absolute -env:UserInstallation, because a relative one hangs past its own timeout.
cd "$(dirname "$0")"
OUT=/home/user/charscale-r150-sweep/ref
mkdir -p "$OUT"

python3 - <<'PY'
import hashlib, pathlib
keys = {m for m in pathlib.Path('movers.txt').read_text().split() if m}
rows = []
for folder in (pathlib.Path('/home/user/corpus-odf/rtf'), pathlib.Path('/home/user/sample-files')):
    for path in sorted(folder.rglob('*')):
        if not path.is_file():
            continue
        if hashlib.md5(str(path).encode()).hexdigest()[:12] in keys:
            rows.append('%s\t%s' % (hashlib.md5(str(path).encode()).hexdigest()[:12], path))
pathlib.Path('movers-paths.tsv').write_text('\n'.join(rows) + '\n')
print('%d of %d movers resolved to a path' % (len(rows), len(keys)))
PY

cut -f2 movers-paths.tsv | xargs -P 4 -I{} sh -c '
    key=$(printf %s "{}" | md5sum | cut -c1-12)
    d="'"$OUT"'/$key"
    mkdir -p "$d"
    timeout -k 30 900 /opt/libreoffice26.2/program/soffice --headless --norestore \
        -env:UserInstallation="file:///tmp/cs150-ref-$key" \
        --convert-to pdf --outdir "$d" "{}" >/dev/null 2>&1
    ls "$d"/*.pdf >/dev/null 2>&1 || echo "FAILED $key"
'
echo "rendered $(find "$OUT" -name '*.pdf' | wc -l) of $(wc -l < movers-paths.tsv)"
