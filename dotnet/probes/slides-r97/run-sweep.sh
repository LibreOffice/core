#!/usr/bin/env bash
# Our half of the .ppt track, scored against the banked 26.2.4.2 reference.
set -uo pipefail
CLI="$1"; OUT="$2"
export XRAY_OFFICE_CLI="$CLI"
exec /home/user/r97-slides/sweep-ours.sh /home/user/sample-files slides '\.ppt$' /home/user/gate-orig-r83 "$OUT" 2
