'*************************************************************************
 '
 '  $RCSfile: VBEventListener.cls,v $
 '
 '  $Revision: 1.5 $
 '
 '  last change: $Author: vg $ $Date: 2005-03-23 09:01:05 $
 '
 '  The Contents of this file are made available subject to the terms of
 '  either of the following licenses
 '
 '         - GNU Lesser General Public License Version 2.1
 '         - Sun Industry Standards Source License Version 1.1
 '
 '  Sun Microsystems Inc., October, 2000
 '
 '  GNU Lesser General Public License Version 2.1
 '  =============================================
 '  Copyright 2000 by Sun Microsystems, Inc.
 '  901 San Antonio Road, Palo Alto, CA 94303, USA
 '
 '  This library is free software; you can redistribute it and/or
 '  modify it under the terms of the GNU Lesser General Public
 '  License version 2.1, as published by the Free Software Foundation.
 '
 '  This library is distributed in the hope that it will be useful,
 '  but WITHOUT ANY WARRANTY; without even the implied warranty of
 '  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 '  Lesser General Public License for more details.
 '
 '  You should have received a copy of the GNU Lesser General Public
 '  License along with this library; if not, write to the Free Software
 '  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 '  MA  02111-1307  USA
 '
 '
 '  Sun Industry Standards Source License Version 1.1
 '  =================================================
 '  The contents of this file are subject to the Sun Industry Standards
 '  Source License Version 1.1 (the "License"); You may not use this file
 '  except in compliance with the License. You may obtain a copy of the
 '  License at http://www.openoffice.org/license.html.
 '
 '  Software provided under this License is provided on an "AS IS" basis,
 '  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 '  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 '  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 '  See the License for the specific provisions governing your rights and
 '  obligations concerning the Software.
 '
 '  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 '
 '  Copyright: 2000 by Sun Microsystems, Inc.
 '
 '  All Rights Reserved.
 '
 '  Contributor(s): _______________________________________
 '
 '
 '***********************************************************************/

VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "VBEventListener"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Option Explicit
Private interfaces(0) As String
Private bDisposingCalled As Boolean
Private bQuiet As Boolean

Public Property Get Bridge_ImplementedInterfaces() As Variant
    Bridge_ImplementedInterfaces = interfaces
End Property

Private Sub Class_Initialize()
interfaces(0) = "com.sun.star.lang.XEventListener"
bDisposingCalled = False
bQuiet = False
End Sub

Private Sub Class_Terminate()
   On Error Resume Next
   Debug.Print "Terminate VBEventListener"
End Sub

Public Sub disposing(ByVal source As Object)
    If bQuiet = False Then
    MsgBox "disposing called"
    End If
    bDisposingCalled = True
End Sub

Public Sub setQuiet(quiet As Boolean)
    bQuiet = quiet
End Sub

Public Sub resetDisposing()
    bDisposingCalled = False
End Sub

Public Function disposingCalled()
    disposingCalled = bDisposingCalled
End Function
