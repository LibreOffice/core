#!/bin/sh

# special handling for mailto: uris
if echo $1 | grep '^mailto:' > /dev/null; then
  if which kde-open; then
    kde-open "$1" &
  else
    mailservice "$1" &
  fi
else
  sensible-browser "$1" &
fi

exit 0
