' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as String
    ' CreateUnoService
    Dim filepicker
    filepicker = CreateUnoService("com.sun.star.ui.dialogs.FilePicker")
    doUnitTest = "OK"
End Function
