#!/bin/sh

for a in `git ls-files *.hrc`; do solenv/bin/insert-msgctxt-hrc $a > /tmp/context.hack && (cmp $a /tmp/context.hack && echo $a unchanged || (mv /tmp/context.hack $a && echo $a updated)); done
for a in `git ls-files *.ui`; do solenv/bin/insert-msgctxt-ui $a > /tmp/context.hack && (cmp $a /tmp/context.hack && echo $a unchanged || (mv /tmp/context.hack $a && echo $a updated)); done
