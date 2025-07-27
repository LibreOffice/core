#!/usr/bin/env python3
"""
Debug script to test Dropbox integration and capture detailed output
"""

import subprocess
import time
import os
import sys

def debug_dropbox_integration():
    print("🔍 Dropbox Integration Debug Test")
    print("=" * 50)

    soffice_path = "./instdir/LibreOfficeDev.app/Contents/MacOS/soffice"

    if not os.path.exists(soffice_path):
        print(f"❌ LibreOffice not found at {soffice_path}")
        return False

    print("Starting LibreOffice with debug output...")
    print("Please:")
    print("1. Go to File → Open from Dropbox...")
    print("2. Complete the OAuth2 flow in your browser")
    print("3. Look at the file dialog that appears")
    print("4. Note what you see (empty, error message, etc.)")
    print("5. Close LibreOffice when done")
    print()
    print("LibreOffice will start in 3 seconds...")
    time.sleep(3)

    try:
        # Start with more verbose output
        process = subprocess.Popen([
            soffice_path,
            "--writer",
            "--norestore"
        ], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)

        print("✅ LibreOffice started (PID:", process.pid, ")")
        print("Waiting for you to test the integration...")
        print("Press Ctrl+C when done testing")

        # Wait for process to complete or user interruption
        try:
            process.wait()
        except KeyboardInterrupt:
            print("\n🛑 User interrupted, terminating LibreOffice...")
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()

        print("✅ Testing completed")
        return True

    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def main():
    print("This debug test will help us identify why the Dropbox file dialog")
    print("appears empty or doesn't show your files properly.")
    print()

    # Check if OAuth2 config exists
    config_file = "./config_host/config_oauth2.h"
    if os.path.exists(config_file):
        print("✅ OAuth2 configuration file found")
        with open(config_file, 'r') as f:
            content = f.read()
            if "DROPBOX_CLIENT_ID" in content and not "your_client_id_here" in content:
                print("✅ Dropbox credentials appear to be configured")
            else:
                print("⚠️  Warning: Dropbox credentials may not be properly configured")
                print("   Check that DROPBOX_CLIENT_ID and DROPBOX_CLIENT_SECRET")
                print("   are set to your actual Dropbox app credentials")
    else:
        print("❌ OAuth2 configuration file not found")
        return 1

    print()
    if input("Ready to start debug test? (y/n): ").lower() != 'y':
        print("Test cancelled.")
        return 1

    success = debug_dropbox_integration()

    print("\n" + "=" * 50)
    print("🔍 DEBUG QUESTIONS:")
    print("=" * 50)
    print("After testing, please tell us:")
    print("1. Did the 'Open from Dropbox...' menu item appear?")
    print("2. Did your browser open for OAuth2 authentication?")
    print("3. Did you successfully authorize LibreOffice?")
    print("4. What did you see in the file dialog?")
    print("   - Completely empty?")
    print("   - Error message?")
    print("   - Loading indicator?")
    print("   - Some files but not all?")
    print("5. Any error messages or unusual behavior?")

    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())
