#!/bin/bash

# Script that adds "com.apple.security.get-task-allow" entitlement to soffice
#
# To connect Xcode's Instruments application to profile LibreOffice, the
# "com.apple.security.get-task-allow" entitlement must be added to the
# soffice executable.
#
# This script will set the "com.apple.security.get-task-allow" entitlement
# instdir/LibreOfficeDev.app/Contents/MacOS/soffice executable so that
# profiling can be done using a local build.
#
# Credit for documenting this Xcode requirement goes to the following blog:
#   https://cocoaphony.micro.blog/2022/10/29/solving-required-kernel.html

SOFFICE=`dirname "$0"`/../instdir/LibreOfficeDev.app/Contents/MacOS/soffice
if [ ! -f "$SOFFICE" -o ! -x "$SOFFICE" ] ; then
    echo "Error: '$SOFFICE' is not an executable file" >&2
    exit 1
fi

codesign -s - -v -f --entitlements /dev/stdin "$SOFFICE" << !
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "https://www.apple.com/DTDs/PropertyList-1.0.dtd"\>
<plist version="1.0">
    <dict>
        <key>com.apple.security.get-task-allow</key>
        <true/>
    </dict>
</plist>
!

exit 0
