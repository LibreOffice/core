#!/bin/bash
# $1 = outdir tag
set -e
CLI="dotnet run --project /home/user/wt-pptgeom/dotnet/tools/XRay.Office.Cli/XRay.Office.Cli.csproj --no-build --"
BASE=/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad/r127
for d in slides/done-007/ppt/introduction_to_bea_tuxedo.ppt \
         slides/done-002/ppt/ws_prod-g-doc-Events-2007-Approval-of-Flight-Conditions-RM-NAA-16032007.ppt \
         slides/done-005/ppt/pods05.ppt; do
  n=$(basename "$d" .ppt)
  mkdir -p "$BASE/conn-$1/$n"
  $CLI render --outdir "$BASE/conn-$1/$n" "/home/user/sample-files/$d" >/dev/null 2>&1
done
echo "$1 done"
