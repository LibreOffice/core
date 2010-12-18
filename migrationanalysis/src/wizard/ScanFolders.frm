VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.ocx"
Begin VB.Form ShowProgress 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Looking for Files"
   ClientHeight    =   2160
   ClientLeft      =   2505
   ClientTop       =   2325
   ClientWidth     =   7110
   ControlBox      =   0   'False
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   2160
   ScaleWidth      =   7110
   ShowInTaskbar   =   0   'False
   Visible         =   0   'False
   Begin MSComctlLib.ProgressBar ScanProgress 
      Height          =   255
      Left            =   120
      TabIndex        =   2
      Top             =   1400
      Width           =   5500
      _ExtentX        =   9710
      _ExtentY        =   450
      _Version        =   393216
      Appearance      =   1
   End
   Begin VB.CommandButton AbortScan 
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      CausesValidation=   0   'False
      Height          =   375
      Left            =   2880
      TabIndex        =   1
      Top             =   1760
      Width           =   1455
   End
   Begin VB.Label Label6 
      Caption         =   "X / Y"
      Height          =   195
      Left            =   5760
      TabIndex        =   7
      Top             =   1430
      Width           =   1200
   End
   Begin VB.Label Label5 
      Caption         =   "Label5"
      Height          =   440
      Left            =   120
      TabIndex        =   6
      Top             =   120
      Width           =   6870
   End
   Begin VB.Label Label4 
      Caption         =   "Label4"
      Height          =   195
      Left            =   120
      TabIndex        =   5
      Top             =   995
      Width           =   1155
   End
   Begin VB.Label Label3 
      Caption         =   "Label3"
      Height          =   195
      Left            =   120
      TabIndex        =   4
      Top             =   680
      Width           =   1155
   End
   Begin VB.Label Label2 
      Caption         =   "Label2"
      Height          =   195
      Left            =   1395
      TabIndex        =   3
      Top             =   995
      Width           =   5595
   End
   Begin VB.Label Label1 
      Caption         =   "Label1"
      Height          =   195
      Left            =   1395
      TabIndex        =   0
      Top             =   680
      Width           =   5595
   End
End
Attribute VB_Name = "ShowProgress"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Declare Function GetTickCount Lib "kernel32" () As Long

Private Const C_MIN_WAIT_TIME   As Long = 0
Private Const C_MIN_UPDATE_TIME As Long = 100

Private g_SP_StartTime As Long
Private g_SP_LastUpdate As Long

Public g_SP_Abort As Boolean
Public g_SP_AllowOtherDLG As Boolean

Public Sub SP_Init(maxIndex As Long)
    g_SP_Abort = False
    g_SP_AllowOtherDLG = False
    g_SP_StartTime = GetTickCount()
    g_SP_LastUpdate = g_SP_StartTime
    
    ShowProgress.Visible = False
    ShowProgress.Caption = GetResString(PROGRESS_CAPTION)
    
    Label3.Caption = GetResString(PROGRESS_PATH_LABEL)
    Label4.Caption = GetResString(PROGRESS_FILE_LABEL)
    Label5.Caption = GetResString(PROGRESS_INFO_LABEL)
    ScanProgress.Max = maxIndex
    
    ShowProgress.Top = frmWizard.Top + 3200
    ShowProgress.Left = frmWizard.Left + 500
End Sub

Public Sub SP_UpdateProgress(curObject As String, curParent As String, _
                             curIndex As Long)

    Dim currTicks As Long
    currTicks = GetTickCount()
    
    ScanProgress.value = curIndex

    If (Not ShowProgress.Visible) Then
        If (currTicks - g_SP_StartTime > C_MIN_WAIT_TIME) Then
            ShowProgress.Visible = True
        End If
    End If
    If (currTicks - g_SP_LastUpdate > C_MIN_UPDATE_TIME) Then
        g_SP_LastUpdate = currTicks
        Label1.Caption = curParent
        Label2.Caption = curObject
        Label6.Caption = "(" & str$(curIndex) & "/" & str$(ScanProgress.Max) & ")"
    End If
End Sub

Private Sub AbortScan_Click()
    g_SP_Abort = True
    Label5.Caption = GetResString(PROGRESS_WAIT_LABEL)
    AbortScan.Caption = GetResString(PROGRESS_ABORTING)
    AbortScan.Enabled = False
End Sub

Private Sub Form_Deactivate()
    If Not g_SP_AllowOtherDLG Then
        ShowProgress.ZOrder (0)
    End If
End Sub

