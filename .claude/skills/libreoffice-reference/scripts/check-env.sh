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

    # The five above are *substitutions* — a requested face mapped to a metric-compatible one.
    # A missing FALLBACK face is a different failure and passes every check above, because
    # nothing requests it by name. It is not hypothetical: this container shipped without
    # fonts-dejavu-core, DejaVu sits ahead of WenQuanYi Zen Hei in the fallback chain, and
    # 267 of the 534 corpus reference PDFs resolve a fallback. Holding LibreOffice constant
    # and varying only that one package moved 53 of 534 reference page counts and 426 pages.
    #
    # The tell is that fc-match NEVER FAILS. Asked for a face it does not have, it returns
    # something else and exits 0, which reads as success unless you look at what came back —
    # which is why the gap survived a whole pass of work, and why this checks the answer
    # rather than the exit status.
    #
    # The repository's own test suite is the authority for what belongs here:
    # SheetColumnDigitsTests pins DejaVu Sans at 1303/2048 of an em against values read from
    # LibreOffice 24.2.7.2's own output, so DejaVu was present when the stored figures were
    # measured. If that test is ever retired, revisit this check rather than deleting it.
    check_font "DejaVu Sans"      "DejaVu Sans"
    if ! fc-match "DejaVu Sans" family 2>/dev/null | head -1 | grep -qx "DejaVu Sans"; then
        say "        apt-get install -y --no-install-recommends fonts-dejavu-core"
        say "        (see MISSING_PACKAGES.md — this moves pagination, not just glyphs)"
    fi
fi

say "== 4. PDF rasteriser (needed only for image comparison) =="
if command -v pdftoppm >/dev/null 2>&1; then
    ok "pdftoppm $(pdftoppm -v 2>&1 | head -1 | sed 's/^[^0-9]*//')"
else
    warn "pdftoppm missing: apt-get install -y --no-install-recommends poppler-utils"
fi

say "== 5. PDF extractor — a measurement input, not a utility =="
# poppler's version is an uncontrolled input to every figure this project records, because
# the gate's second check has been `pdftotext | wc -w`. That is not a theoretical worry:
# with our renderer's code PROVABLY unchanged (git log over dotnet/src returning nothing),
# our own word counts moved on 169 of 200 documents, and 86 of them moved by the exact
# amount the reference moved. A term that shifts both sides of a comparison equally belongs
# to neither renderer.
#
# So this prints the version rather than merely checking presence: a stored figure whose
# extractor version is unrecorded cannot be compared with a new one. `xray-office analyze`
# exists to remove this dependency by reading PDFs in process; until every caller has moved
# over, record what is printed here beside any figure you keep.
if command -v pdftotext >/dev/null 2>&1; then
    ok "pdftotext $(pdftotext -v 2>&1 | head -1 | sed 's/^[^0-9]*//')"
    say "        (record this beside any word count — it is part of the measurement)"
else
    warn "pdftotext missing: apt-get install -y --no-install-recommends poppler-utils"
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
