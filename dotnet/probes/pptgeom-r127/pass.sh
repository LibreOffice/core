#!/bin/bash
set -e
BASE=/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad/r127
CLI="dotnet run --project /home/user/wt-pptgeom/dotnet/tools/XRay.Office.Cli/XRay.Office.Cli.csproj --no-build --"
while read -r d; do
  n=$(basename "$d" .ppt)
  mkdir -p "$BASE/pass-$1/$n"
  $CLI render --outdir "$BASE/pass-$1/$n" "/home/user/sample-files/$d" >/dev/null 2>&1 || echo "FAILED $n"
done < "$BASE/adjdocs.txt"
echo "pass-$1 done"
