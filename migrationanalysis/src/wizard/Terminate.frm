VERSION 5.00
Begin VB.Form TerminateMSO 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Dialog Caption"
   ClientHeight    =   2280
   ClientLeft      =   3165
   ClientTop       =   2835
   ClientWidth     =   6030
   ControlBox      =   0   'False
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   2280
   ScaleWidth      =   6030
   ShowInTaskbar   =   0   'False
   Begin VB.CommandButton NoButton 
      Cancel          =   -1  'True
      Caption         =   "No"
      CausesValidation=   0   'False
      Default         =   -1  'True
      Height          =   375
      Left            =   4560
      TabIndex        =   0
      Top             =   1800
      Width           =   1215
   End
   Begin VB.CommandButton YesButton 
      Caption         =   "Yes"
      CausesValidation=   0   'False
      Height          =   375
      Left            =   3120
      TabIndex        =   1
      Top             =   1800
      Width           =   1215
   End
   Begin VB.Label Info 
      Caption         =   "Label1"
      Height          =   1455
      Left            =   120
      TabIndex        =   2
      Top             =   120
      Width           =   5775
   End
End
Attribute VB_Name = "TerminateMSO"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Const CR_STR = "<CR>"

Private Sub Form_Activate()
    NoButton.Default = True
End Sub

Private Sub Form_Load()
    
    TerminateMSO.Top = frmWizard.Top + 3000
    TerminateMSO.Left = frmWizard.Left + 1000
    
    TerminateMSO.Caption = GetResString(TERMINATE_CAPTION)
    Info.Caption = ReplaceTopicTokens(GetResString(TERMINATE_INFO), CR_STR, Chr(13))
    YesButton.Caption = GetResString(TERMINATE_YES)
    NoButton.Caption = GetResString(TERMINATE_NO)
    NoButton.Default = True
End Sub

Private Sub YesButton_Click()
    Dim fso As New FileSystemObject
    TerminateOfficeApps fso, " --kill"
    TerminateMSO.Hide
    Set fso = Nothing
End Sub

Private Sub NoButton_Click()
    TerminateMSO.Hide
End Sub


