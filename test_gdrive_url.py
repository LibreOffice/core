#!/usr/bin/env python3

import os
import sys
import subprocess

def test_gdrive_url():
    """Test accessing a gdrive:// URL directly"""

    # Test the gdrive URL from the logs
    gdrive_url = "gdrive://1SIAWn1iL8vX81hrQ6Jv4haI_R_VO2zQa"

    print(f"Testing LibreOffice with gdrive URL: {gdrive_url}")

    # Try to open the URL directly
    soffice_path = "instdir/LibreOfficeDev.app/Contents/MacOS/soffice"

    env = os.environ.copy()
    env['SAL_LOG'] = '+WARN.ucb.ucp.gdrive+WARN.ucb+INFO.ucb'

    cmd = [soffice_path, "--headless", "--convert-to", "pdf", gdrive_url]

    print(f"Running: {' '.join(cmd)}")
    print("Environment: SAL_LOG=" + env.get('SAL_LOG', ''))

    try:
        result = subprocess.run(cmd, env=env, capture_output=True, text=True, timeout=30)

        print(f"Return code: {result.returncode}")
        print("STDOUT:")
        print(result.stdout)
        print("STDERR:")
        print(result.stderr)

        # Look for UCB provider logs
        if "ucb.ucp.gdrive" in result.stderr:
            print("✅ Google Drive UCB provider was called!")
        else:
            print("❌ Google Drive UCB provider was NOT called")

        # Look for specific error messages
        if "ContentProvider::queryContent" in result.stderr:
            print("✅ ContentProvider::queryContent was called")
        else:
            print("❌ ContentProvider::queryContent was NOT called")

    except subprocess.TimeoutExpired:
        print("❌ Command timed out")
    except Exception as e:
        print(f"❌ Error running command: {e}")

if __name__ == "__main__":
    test_gdrive_url()
