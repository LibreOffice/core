#!/usr/bin/env bash
# Rank a sweep's documents by unaccounted ink, largest first.
S=/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad
D="$S/${1:?usage: sl14-rank.sh <outdir-name> [count]}"
N="${2:-25}"
sort -t"$(printf '\t')" -k3 -rn "$D/ink.tsv" | head -"$N" |
  awk -F'\t' '{n=split($1,p,"/"); printf "%8.2f  %4s major of %4s  %s\n", $3, $4, $2, p[n]}'
echo
awk -F'\t' '$3!="-" && $3!="?" {i+=$3} END{printf "track total %.2f\n", i}' "$D/ink.tsv"
