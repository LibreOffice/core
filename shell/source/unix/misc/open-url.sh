#!/bin/sh
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# tries to locate the executable specified
# as first parameter in the user's path.
which() {
  if [ ! -z "$1" ]; then
    for i in `echo $PATH | sed -e 's/^:/.:/g' -e 's/:$/:./g' -e 's/::/:.:/g' -e 's/:/ /g'`; do
      if [ -x "$i/$1" -a ! -d "$i/$1" ]; then
        echo "$i/$1"
        break;
      fi
    done
  fi
}

# checks for the original mozilla start script(s)
# and restrict the "-remote" semantics to those.
run_mozilla() {
  if file "$1" | grep "script" > /dev/null && grep "NPL" "$1" > /dev/null; then
    "$1" -remote 'ping()' 2>/dev/null >/dev/null
    if [ $? -eq 2 ]; then
      "$1" "$2" &
    else
      "$1" -remote "openURL($2, new-window)" &
    fi
  else
    "$1" "$2" &
  fi
}

# checks the browser value for a %s as defined in
# http://www.catb.org/~esr/BROWSER/index.html
run_browser() {
  echo "$1|$2" | awk '
{
    FS="|";
    $syscmd="";
    if (index($1,"%s") > 0) {
        $syscmd=sprintf($1,$2);
    } else {
        $syscmd=sprintf("%s \"%s\"",$1,$2);
    }
    system($syscmd " &");
}' > /dev/null
}

# special handling for mailto: uris
if echo $1 | grep '^mailto:' > /dev/null; then
  # check for xdg-email
  mailer=`which xdg-email`
  if [ ! -z "$mailer" ]; then
    $mailer "$1" &
    exit 0
  fi
  # check $MAILER variable
  if [ ! -z "$MAILER" ]; then
    $MAILER "$1" &
    exit 0
  fi
  # mozilla derivates may need -remote semantics
  for i in thunderbird mozilla netscape icedove iceape; do
    mailer=`which $i`
    if [ ! -z "$mailer" ]; then
      run_mozilla "$mailer" "$1"
      exit 0
    fi
  done
  # handle all non mozilla mail clients below
  # ..
else
  # check for xdg-open
  browser=`which xdg-open`
  if [ ! -z "$browser" ]; then
    $browser "$1" &
    exit 0
  fi
  # check $BROWSER variable
  if [ ! -z "$BROWSER" ]; then
    $BROWSER "$1" &
    exit 0
  fi
  # mozilla derivates may need -remote semantics
  for i in chrome seamonkey firefox iceweasel iceape; do
    browser=`which $i`
    if [ ! -z "$browser" ]; then
      run_mozilla "$browser" "$1"
      exit 0
    fi
  done
  # handle all non mozilla browers below
  # ..
fi
exit 1
