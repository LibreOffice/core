#!/bin/bash

echo "🔍 Monitoring Google Drive library build..."
echo "Checking every 30 seconds..."
echo "Press Ctrl+C to stop monitoring"
echo ""

while true; do
    # Check if library was built
    if [ -f "workdir/LinkTarget/Library/libucpgdrivelo.dylib" ]; then
        echo "🎉 SUCCESS: Google Drive library built!"
        ls -la workdir/LinkTarget/Library/libucpgdrivelo.dylib
        echo ""
        echo "Running verification..."
        python3 verify_gdrive_build.py
        break
    fi

    # Check build progress
    echo -n "$(date): Checking... "

    # Count recent object files as progress indicator
    recent_files=$(find workdir -name "*.o" -newer workdir/LinkTarget/Executable/concat-deps 2>/dev/null | wc -l)
    echo "$recent_files object files compiled since build started"

    sleep 30
done