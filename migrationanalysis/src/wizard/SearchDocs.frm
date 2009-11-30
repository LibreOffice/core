VERSION 5.00
Begin VB.Form SearchDocs 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Looking for Files"
   ClientHeight    =   1830
   ClientLeft      =   2505
   ClientTop       =   2325
   ClientWidth     =   7110
   ControlBox      =   0   'False
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1830
   ScaleWidth      =   7110
   ShowInTaskbar   =   0   'False
   Visible         =   0   'False
   Begin VB.CommandButton AbortScan 
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      CausesValidation=   0   'False
      Height          =   375
      Left            =   2880
      TabIndex        =   1
      Top             =   1350
      Width           =   1455
   End
   Begin VB.Label Label5 
      Caption         =   "Label5"
      Height          =   440
      Left            =   120
      TabIndex        =   5
      Top             =   120
      Width           =   6870
      WordWrap        =   -1  'True
   End
   Begin VB.Label Label4 
      Caption         =   "gefundene Dokumente:"
      Height          =   195
      Left            =   120
      TabIndex        =   4
      Top             =   960
      Width           =   1800
   End
   Begin VB.Label Label3 
      Caption         =   "Pfad:"
      Height          =   195
      Left            =   120
      TabIndex        =   3
      Top             =   680
      Width           =   1800
   End
   Begin VB.Label Label2 
      Caption         =   "Label2"
      Height          =   195
      Left            =   2040
      TabIndex        =   2
      Top             =   995
      Width           =   5595
   End
   Begin VB.Label Label1 
      Caption         =   "Label1"
      Height          =   195
      Left            =   2040
      TabIndex        =   0
      Top             =   680
      Width           =   5595
   End
End
Attribute VB_Name = "SearchDocs"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Declare Function GetTickCount Lib "kernel32" () As Long

Private Const C_MIN_WAIT_TIME   As Long = 1000
Private Const C_MIN_UPDATE_TIME As Long = 100

Private g_SD_StartTime As Long
Private g_SD_LastUpdate As Long

Public g_SD_Abort As Boolean

Private Sub Form_Load()

    g_SD_Abort = False
    g_SD_StartTime = GetTickCount()
    g_SD_LastUpdate = g_SD_StartTime
    
    SearchDocs.Visible = False
    SearchDocs.Caption = GetResString(SEARCH_CAPTION)
    
    Label3.Caption = GetResString(SEARCH_PATH_LABEL)
    Label4.Caption = GetResString(SEARCH_FOUND_LABEL)
    Label5.Caption = GetResString(SEARCH_INFO_LABEL)
End Sub

Public Sub SD_UpdateProgress(curObject As String, curParent As String)

    Dim currTicks As Long
    currTicks = GetTickCount()
    
    If (Not SearchDocs.Visible) Then
        If (currTicks - g_SD_StartTime > C_MIN_WAIT_TIME) Then
            SearchDocs.Visible = True
        End If
    End If
    If (currTicks - g_SD_LastUpdate > C_MIN_UPDATE_TIME) Then
        g_SD_LastUpdate = currTicks
        Label1.Caption = curParent
        Label2.Caption = curObject
    End If
End Sub

Private Sub AbortScan_Click()
    g_SD_Abort = True
End Sub

Private Sub Form_Deactivate()
    SearchDocs.ZOrder (0)
End Sub

