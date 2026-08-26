#!/usr/bin/env bash
set -e
set +x

# Modify the Info.plist to ensure that CFBundleVersion is always incremented
# AppStoreConnect requires each upload, whether it is released or not, to be
# higher than the previous successful upload's CFBundleVersion. So this
# script sets the CFBundleVersion to the first component of
# CFBundleShortVersionString and the UTC timestamp of when this script was run
# is appended as the second component
info_plist="$BUILT_PRODUCTS_DIR/$INFOPLIST_PATH"
if [ ! -f "$info_plist" ]; then
    echo "Error: $info_plist does not exist or is not a regular file" >&2
    exit 1
fi

bundle_short_version=`/usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" "$info_plist"`
if [ -z "$bundle_short_version" ]; then
    echo "Error: CFBundleShortVersionString key in $info_plist empty" >&2
    exit 1
fi

major_version=`echo "$bundle_short_version" | cut -d. -f1`
if [ -z "$major_version" -o "$major_version" = "0" ]; then
    echo "Error: CFBundleShortVersionString major version is empty or 0" >&2
    exit 1
fi

bundle_version="$major_version.`date -u '+%Y%m%d%H%M'`"
/usr/libexec/PlistBuddy -c "Set :CFBundleVersion $bundle_version" "$info_plist"

echo "Succesfully Updated CFBundleVersion"
