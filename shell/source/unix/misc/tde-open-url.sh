#!/bin/sh

# special handling for mailto: uris
if echo $1 | grep '^mailto:' > /dev/null; then
  kmailservice "$1" &
else
  kfmclient openURL "$1" &
fi

exit 0
