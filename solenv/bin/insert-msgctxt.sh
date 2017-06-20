#!/bin/sh

for a in `git ls-files *.hrc`; do solenv/bin/insert-msgctxt $a > /tmp/context.hack && mv /tmp/context.hack $a; done
