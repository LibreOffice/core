'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'


Function doUnitTest as Integer
  Dim oNamedValue as new com.sun.star.beans.NamedValue 
  Dim oCellAddress as new com.sun.star.table.CellAddress
  oNamedValue.Value = oCellAddress ' fdo#60065 - this would throw an error
  doUnitTest = 1
End Function
