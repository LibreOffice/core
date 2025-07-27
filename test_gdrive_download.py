#!/usr/bin/env python3
"""
Test script to simulate file selection and download from Google Drive
"""

import sys
import os
import time

# Add the LibreOffice python path
sys.path.insert(0, '/Users/georgejeffreys/lode/dev/core/instdir/LibreOfficeDev.app/Contents/Resources')

import uno
from com.sun.star.uno import Exception as UnoException

def test_gdrive_file_download():
    """Test downloading a file by creating a gdrive:// URL and opening it"""

    try:
        # Connect to LibreOffice
        local_context = uno.getComponentContext()
        resolver = local_context.getServiceManager().createInstanceWithContext(
            "com.sun.star.bridge.UnoUrlResolver", local_context)

        # Connect to the running LibreOffice instance
        try:
            context = resolver.resolve("uno:socket,host=localhost,port=2002;urp;StarOffice.ComponentContext")
        except Exception:
            print("Could not connect to LibreOffice. Make sure LibreOffice is running.")
            return False

        desktop = context.getServiceManager().createInstanceWithContext(
            "com.sun.star.frame.Desktop", context)

        # From the debug output, we know there's a file with ID: 1m425_Vlnzxpreag5T85s8mm47xiMYFLA
        # Let's try to open it using the gdrive:// URL scheme
        file_id = "1m425_Vlnzxpreag5T85s8mm47xiMYFLA"  # Untitled 2a.ods
        gdrive_url = f"gdrive://{file_id}"

        print(f"Attempting to open file from Google Drive: {gdrive_url}")

        # Try to load the document from Google Drive
        doc = desktop.loadComponentFromURL(gdrive_url, "_blank", 0, ())

        if doc:
            print("SUCCESS: File opened successfully from Google Drive!")

            # Wait a moment then close
            time.sleep(2)
            doc.close(True)
            return True
        else:
            print("FAILED: Could not open file from Google Drive")
            return False

    except UnoException as e:
        print(f"UNO Exception while opening gdrive URL: {e}")
        return False
    except Exception as e:
        print(f"Exception while opening gdrive URL: {e}")
        return False

if __name__ == "__main__":
    success = test_gdrive_file_download()
    if success:
        print("Google Drive file download test PASSED!")
    else:
        print("Google Drive file download test FAILED.")
        print("Check the LibreOffice debug output for detailed error information.")