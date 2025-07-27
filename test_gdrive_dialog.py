#!/usr/bin/env python3
"""
Test script to verify Google Drive dialog functionality
"""

import sys
import os
import time

# Add the LibreOffice python path
sys.path.insert(0, '/Users/georgejeffreys/lode/dev/core/instdir/LibreOfficeDev.app/Contents/Resources')

import uno
from com.sun.star.uno import Exception as UnoException

def test_google_drive_dialog():
    """Test the Google Drive dialog by connecting to LibreOffice and triggering the command"""

    try:
        # Connect to LibreOffice
        local_context = uno.getComponentContext()
        resolver = local_context.getServiceManager().createInstanceWithContext(
            "com.sun.star.bridge.UnoUrlResolver", local_context)

        # Connect to the running LibreOffice instance
        try:
            context = resolver.resolve("uno:socket,host=localhost,port=2002;urp;StarOffice.ComponentContext")
        except Exception:
            print("Could not connect to LibreOffice. Make sure LibreOffice is running with:")
            print("instdir/LibreOfficeDev.app/Contents/MacOS/soffice --accept='socket,host=localhost,port=2002;urp;' --norestore --nologo --nodefault")
            return False

        desktop = context.getServiceManager().createInstanceWithContext(
            "com.sun.star.frame.Desktop", context)

        # Create a new Writer document
        doc = desktop.loadComponentFromURL("private:factory/swriter", "_blank", 0, ())

        # Get the frame and dispatch helper
        frame = doc.getCurrentController().getFrame()
        dispatch_helper = context.getServiceManager().createInstanceWithContext(
            "com.sun.star.frame.DispatchHelper", context)

        # Dispatch the OpenGoogleDrive command
        print("Triggering OpenGoogleDrive command...")
        dispatch_helper.executeDispatch(frame, ".uno:OpenGoogleDrive", "", 0, ())

        print("Google Drive dialog should now be open!")
        print("Check the LibreOffice window and debug output for any errors.")

        # Wait a bit to see what happens
        time.sleep(5)

        # Close the document
        doc.close(True)

        return True

    except UnoException as e:
        print(f"UNO Exception: {e}")
        return False
    except Exception as e:
        print(f"Exception: {e}")
        return False

if __name__ == "__main__":
    success = test_google_drive_dialog()
    if success:
        print("Test completed. Check the debug output for Google Drive functionality.")
    else:
        print("Test failed.")