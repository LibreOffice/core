#!/usr/bin/env bash
# Verify the machine can produce trustworthy LibreOffice reference output.
# Exits non-zero if anything would silently corrupt a comparison.
set -uo pipefail

problems=0
warnings=0

say()  { printf '%s\n' "$*"; }
ok()   { printf '  \033[32mOK\033[0m    %s\n' "$*"; }
bad()  { printf '  \033[31mFAIL\033[0m  %s\n' "$*"; problems=$((problems + 1)); }
warn() { printf '  \033[33mWARN\033[0m  %s\n' "$*"; warnings=$((warnings + 1)); }

say "== 1. soffice binary =="
if ! command -v soffice >/dev/null 2>&1; then
    bad "soffice not on PATH. Install libreoffice-writer libreoffice-calc libreoffice-impress"
else
    version="$(soffice --version 2>/dev/null | head -1)"
    if [ -z "$version" ]; then
        bad "soffice exists but '--version' produced nothing"
    else
        ok "$version"
        say "        (record this version alongside any reference output you keep)"
    fi
fi

say "== 2. application modules =="
# The decisive test is behavioural, not package-based: soffice from libreoffice-core
# alone runs fine but cannot load *any* document. So actually convert something.
probe_dir="$(mktemp -d)"
trap 'rm -rf "$probe_dir"' EXIT
printf 'probe\n' > "$probe_dir/probe.txt"
soffice --headless --norestore --nolockcheck \
        -env:UserInstallation="file://$probe_dir/profile" \
        --convert-to pdf --outdir "$probe_dir/out" "$probe_dir/probe.txt" \
        >/dev/null 2>&1
if [ -f "$probe_dir/out/probe.pdf" ]; then
    ok "a document actually converts (writer module present)"
else
    bad "conversion produced no output - application modules are missing"
    say "        apt-get install -y --no-install-recommends \\"
    say "            libreoffice-writer libreoffice-calc libreoffice-impress"
fi

say "== 3. metric-compatible fonts =="
# Wrong substitutions here reflow text and make every later page differ, which reads
# as a layout bug in whatever you are testing. This is the highest-value check.
if ! command -v fc-match >/dev/null 2>&1; then
    warn "fc-match not available; cannot verify font substitution"
else
    check_font() {  # check_font <requested> <required-substitute>
        actual="$(fc-match "$1" family 2>/dev/null | head -1)"
        if [ "$actual" = "$2" ]; then
            ok "$1 -> $actual"
        else
            bad "$1 -> $actual (need $2)"
        fi
    }
    check_font Calibri           Carlito
    check_font Cambria           Caladea
    check_font Arial             "Liberation Sans"
    check_font "Times New Roman" "Liberation Serif"
    check_font "Courier New"     "Liberation Mono"
    if [ "$problems" -gt 0 ]; then
        say "        apt-get install -y --no-install-recommends \\"
        say "            fonts-crosextra-carlito fonts-crosextra-caladea fonts-liberation"
    fi
fi

say "== 4. PDF rasteriser (needed only for image comparison) =="
if command -v pdftoppm >/dev/null 2>&1; then
    ok "pdftoppm $(pdftoppm -v 2>&1 | head -1 | sed 's/^[^0-9]*//')"
else
    warn "pdftoppm missing: apt-get install -y --no-install-recommends poppler-utils"
fi

say ""
if [ "$problems" -gt 0 ]; then
    printf '\033[31m%s problem(s) found - reference output would be unreliable.\033[0m\n' "$problems"
    exit 1
fi
if [ "$warnings" -gt 0 ]; then
    printf '\033[33mUsable, with %s warning(s).\033[0m\n' "$warnings"
    exit 0
fi
printf '\033[32mEnvironment is good.\033[0m\n'
