#!/bin/bash
echo "Starting LibreOffice with debug output..."
export SAL_LOG="+INFO+WARN"
./instdir/LibreOfficeDev.app/Contents/MacOS/soffice --writer --norestore 2>&1 | tee dropbox_debug.log &
echo "LibreOffice started. Debug output will be saved to dropbox_debug.log"
echo "Now try File -> Open from Dropbox... and check the log file"
