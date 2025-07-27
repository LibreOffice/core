#!/usr/bin/env python3
"""
Test script to verify Dropbox OAuth2 integration works
"""

import subprocess
import time
import os
import sys

def test_dropbox_oauth():
    print("Testing Dropbox OAuth2 Integration...")

    # Start LibreOffice Writer
    print("Starting LibreOffice Writer...")
    soffice_path = "./instdir/LibreOfficeDev.app/Contents/MacOS/soffice"

    if not os.path.exists(soffice_path):
        print(f"Error: LibreOffice not found at {soffice_path}")
        return False

    try:
        # Start LibreOffice in the background
        process = subprocess.Popen([
            soffice_path,
            "--writer",
            "--norestore"
        ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        print("LibreOffice started successfully!")
        print("To test the Dropbox integration:")
        print("1. Go to File menu")
        print("2. Look for 'Open from Dropbox...' option")
        print("3. Click it to test the OAuth2 flow")
        print("4. Your browser should open for Dropbox authentication")
        print("5. After authorization, the file dialog should show your Dropbox files")

        print("\nPress Enter to terminate LibreOffice when done testing...")
        input()

        # Terminate LibreOffice
        process.terminate()
        process.wait(timeout=5)
        print("LibreOffice terminated.")

        return True

    except subprocess.TimeoutExpired:
        print("Warning: LibreOffice did not terminate cleanly")
        process.kill()
        return False
    except Exception as e:
        print(f"Error: {e}")
        return False

if __name__ == "__main__":
    success = test_dropbox_oauth()
    sys.exit(0 if success else 1)
