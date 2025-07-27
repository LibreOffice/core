#!/usr/bin/env python3

import sys
import os

# Add LibreOffice Python path
sys.path.insert(0, './instdir/LibreOfficeDev.app/Contents/Resources/program')

import uno
from com.sun.star.connection import NoConnectException

def test_gdrive():
    try:
        # Get component context
        localContext = uno.getComponentContext()

        # Get service manager
        sm = localContext.ServiceManager

        print("Testing Google Drive UCB provider...")

        # Try to create our Google Drive content provider
        try:
            gdrive_provider = sm.createInstanceWithContext(
                "com.sun.star.ucb.GoogleDriveContentProvider", localContext)
            print("✓ Google Drive provider created successfully!")

            # Try to create a content identifier
            identifier = sm.createInstanceWithContext(
                "com.sun.star.ucb.ContentIdentifier", localContext)

            # Test with gdrive://root URL
            identifier.initialize(("gdrive://root",))
            print(f"✓ Content identifier created: {identifier.getContentIdentifier()}")

        except Exception as e:
            print(f"✗ Failed to create Google Drive provider: {e}")

    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    test_gdrive()