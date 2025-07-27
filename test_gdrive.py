#!/usr/bin/env python3

import sys
sys.path.append('./instdir/LibreOfficeDev.app/Contents/Resources')

import uno
import unohelper

# Start LibreOffice in headless mode
localContext = uno.getComponentContext()
resolver = localContext.ServiceManager.createInstanceWithContext(
    "com.sun.star.bridge.UnoUrlResolver", localContext)

# List all available UCB content providers
ucb = localContext.ServiceManager.createInstanceWithContext(
    "com.sun.star.ucb.UniversalContentBroker", localContext)

print("Available content providers:")
providers = ucb.queryContentProviders()
for provider in providers:
    print(f"  Scheme: {provider.Scheme}")

# Try to create Google Drive provider directly
print("\nTrying to create Google Drive provider...")
try:
    gdrive_provider = localContext.ServiceManager.createInstanceWithContext(
        "com.sun.star.ucb.GoogleDriveContentProvider", localContext)
    print("Success! Google Drive provider created.")
except Exception as e:
    print(f"Failed to create Google Drive provider: {e}")