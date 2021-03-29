# Miscellaneous Modules

This module contains a grab-bag of unrelated miscellaneous libraries, *none* of
which is an extension.

## Application Online Update Checking

When we start LO, first `InitUpdateCheckJobThread` is created, via
`UpdateCheckJob::execute()` (from `extensions/source/update/check/updatecheckjob.cxx`),
as a reaction to a `onFirstVisibleTask` event. It waits 25 seconds (so that it
does not interfere with the startup itself), and then calls
`UpdateCheck::initialize()` (from `extensions/source/update/check/updatecheck.cxx`).

This creates one more thread, `UpdateCheckThread`, that regularly checks whether
we have reached the time when we should ask for the update.  If yes, asks for
that, and shows the download button in the menu (if the new update is
available).

## OLE Automation Bridge

A bridge between "OLE automation" and UNO, so you can use UNO services
from JScript, VBScript, etc.

<https://www.openoffice.org/udk/common/man/spec/ole_bridge.html>

See `udkapi/com/sun/star/bridge/oleautomation/ApplicationRegistration.idl`

This is initialized in `Desktop::Main()` in `Desktop::OpenClients_Impl()`
by creating the service `com.sun.star.bridge.OleApplicationRegistration`,
which is implemented by `OleServer_Impl`.

See `extensions/source/ole/`

## ActiveX Control

This allows embedding LO into a Win32 application as an ActiveX control.

See `extensions/source/activex/`

## Spotlight Provider

On macOS, this allows indexing ODF documents with Spotlight.

See `extensions/source/macosx/spotlight/`

## Scanner Support

You can scan from LibreOffice, using platform specific backends like TWAIN/SANE.

See `extensions/source/scanner/`

