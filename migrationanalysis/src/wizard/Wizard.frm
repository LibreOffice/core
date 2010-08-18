VERSION 5.00
Begin VB.Form frmWizard 
   Appearance      =   0  'Flat
   BackColor       =   &H80000005&
   BorderStyle     =   1  'Fixed Single
   Caption         =   "OpenOffice.org Document Analysis Wizard"
   ClientHeight    =   5520
   ClientLeft      =   1965
   ClientTop       =   1815
   ClientWidth     =   8175
   BeginProperty Font 
      Name            =   "Arial"
      Size            =   8.25
      Charset         =   0
      Weight          =   400
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   Icon            =   "Wizard.frx":0000
   KeyPreview      =   -1  'True
   LinkTopic       =   "Form1"
   LockControls    =   -1  'True
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5520
   ScaleWidth      =   8175
   Tag             =   "1000"
   Begin VB.Frame fraStep 
      BorderStyle     =   0  'None
      Caption         =   "Introduction"
      ClipControls    =   0   'False
      Enabled         =   0   'False
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   4905
      Index           =   0
      Left            =   -10000
      TabIndex        =   25
      Tag             =   "1000"
      Top             =   0
      Width           =   8235
      Begin VB.PictureBox Picture4 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         BorderStyle     =   0  'None
         ForeColor       =   &H80000008&
         Height          =   4935
         Index           =   0
         Left            =   0
         ScaleHeight     =   4935
         ScaleWidth      =   2565
         TabIndex        =   2
         TabStop         =   0   'False
         Top             =   0
         Width           =   2565
         Begin VB.PictureBox Picture10 
            Height          =   735
            Left            =   2580
            ScaleHeight     =   735
            ScaleWidth      =   30
            TabIndex        =   68
            TabStop         =   0   'False
            Top             =   2610
            Width           =   30
         End
         Begin VB.PictureBox Picture6 
            Appearance      =   0  'Flat
            BackColor       =   &H80000005&
            BorderStyle     =   0  'None
            ForeColor       =   &H80000008&
            Height          =   1485
            Left            =   150
            ScaleHeight     =   1485
            ScaleWidth      =   2355
            TabIndex        =   67
            TabStop         =   0   'False
            Top             =   3390
            Width           =   2355
            Begin VB.PictureBox Picture1 
               Appearance      =   0  'Flat
               BackColor       =   &H80000005&
               BorderStyle     =   0  'None
               ForeColor       =   &H80000008&
               Height          =   1200
               Index           =   0
               Left            =   200
               Picture         =   "Wizard.frx":482C2
               ScaleHeight     =   1200
               ScaleWidth      =   1980
               TabIndex        =   7
               TabStop         =   0   'False
               Tag             =   "1060"
               Top             =   300
               Width           =   1980
            End
         End
         Begin VB.Label lblStep1_4 
            BackColor       =   &H00EED3C2&
            BackStyle       =   0  'Transparent
            Caption         =   "4. Analyze"
            ForeColor       =   &H00BF4F59&
            Height          =   195
            Left            =   120
            TabIndex        =   89
            Tag             =   "1044"
            Top             =   1800
            Width           =   2140
         End
         Begin VB.Line Line2 
            BorderColor     =   &H00808080&
            Index           =   2
            X1              =   2550
            X2              =   2550
            Y1              =   0
            Y2              =   4920
         End
         Begin VB.Line Line3 
            Index           =   1
            X1              =   120
            X2              =   2280
            Y1              =   480
            Y2              =   480
         End
         Begin VB.Label Label7 
            BackColor       =   &H00EED3C2&
            Caption         =   "1. Introduction"
            ForeColor       =   &H00BF4F59&
            Height          =   255
            Index           =   1
            Left            =   120
            TabIndex        =   11
            Tag             =   "1041"
            Top             =   720
            Width           =   2140
         End
         Begin VB.Label Label8 
            BackColor       =   &H00EED3C2&
            BackStyle       =   0  'Transparent
            Caption         =   "3. Results"
            ForeColor       =   &H00BF4F59&
            Height          =   255
            Index           =   1
            Left            =   120
            TabIndex        =   9
            Tag             =   "1043"
            Top             =   1440
            Width           =   2140
         End
         Begin VB.Label Label9 
            BackColor       =   &H00EED3C2&
            BackStyle       =   0  'Transparent
            Caption         =   "2. Documents"
            ForeColor       =   &H00BF4F59&
            Height          =   255
            Index           =   1
            Left            =   120
            TabIndex        =   10
            Tag             =   "1042"
            Top             =   1080
            Width           =   2140
         End
         Begin VB.Label Label12 
            BackStyle       =   0  'Transparent
            Caption         =   "Steps"
            BeginProperty Font 
               Name            =   "Arial"
               Size            =   8.25
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Index           =   1
            Left            =   120
            TabIndex        =   8
            Tag             =   "1040"
            Top             =   240
            Width           =   2115
         End
      End
      Begin VB.PictureBox Picture8 
         Appearance      =   0  'Flat
         BorderStyle     =   0  'None
         ForeColor       =   &H80000008&
         Height          =   4935
         Left            =   2400
         ScaleHeight     =   4935
         ScaleWidth      =   5925
         TabIndex        =   3
         TabStop         =   0   'False
         Top             =   -30
         Width           =   5925
         Begin VB.CheckBox chkShowIntro 
            Caption         =   "Do not show this introduction again"
            Enabled         =   0   'False
            Height          =   315
            Left            =   690
            MaskColor       =   &H00000000&
            TabIndex        =   6
            Tag             =   "1103"
            Top             =   4890
            Visible         =   0   'False
            Width           =   3810
         End
         Begin VB.Label lblIntroduction1 
            AutoSize        =   -1  'True
            Caption         =   $"Wizard.frx":4F8B8
            Height          =   585
            Left            =   690
            TabIndex        =   93
            Tag             =   "1101"
            Top             =   750
            Width           =   4890
            WordWrap        =   -1  'True
         End
         Begin VB.Label lblIntroduction3 
            AutoSize        =   -1  'True
            Caption         =   "The wizard will remain on screen while the analysis is carried out."
            Height          =   195
            Left            =   690
            TabIndex        =   0
            Tag             =   "1104"
            Top             =   2670
            Width           =   4845
            WordWrap        =   -1  'True
         End
         Begin VB.Label lblIntroduction2 
            AutoSize        =   -1  'True
            Caption         =   "You will be able to select which documents you want to analyze as well as where you want the results to the analysis to be saved. "
            Height          =   390
            Left            =   690
            TabIndex        =   1
            Tag             =   "1102"
            Top             =   1800
            Width           =   4875
            WordWrap        =   -1  'True
         End
         Begin VB.Label Label12 
            BackStyle       =   0  'Transparent
            Caption         =   "Introduction"
            BeginProperty Font 
               Name            =   "Arial"
               Size            =   8.25
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Index           =   2
            Left            =   450
            TabIndex        =   5
            Tag             =   "1100"
            Top             =   270
            Width           =   4000
         End
      End
   End
   Begin VB.Frame fraStep 
      BorderStyle     =   0  'None
      Caption         =   "Setup"
      Enabled         =   0   'False
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   4905
      Index           =   1
      Left            =   -10000
      TabIndex        =   32
      Tag             =   "2000"
      Top             =   0
      Width           =   8235
      Begin VB.PictureBox Picture4 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         BorderStyle     =   0  'None
         ForeColor       =   &H80000008&
         Height          =   4905
         Index           =   1
         Left            =   0
         ScaleHeight     =   4905
         ScaleWidth      =   2565
         TabIndex        =   61
         TabStop         =   0   'False
         Top             =   0
         Width           =   2565
         Begin VB.PictureBox Picture1 
            Appearance      =   0  'Flat
            BackColor       =   &H80000005&
            BorderStyle     =   0  'None
            ForeColor       =   &H80000008&
            Height          =   1200
            Index           =   1
            Left            =   350
            Picture         =   "Wizard.frx":4F971
            ScaleHeight     =   1200
            ScaleWidth      =   1980
            TabIndex        =   62
            TabStop         =   0   'False
            Tag             =   "1060"
            Top             =   3690
            Width           =   1980
         End
         Begin VB.Label lblStep2_4 
            BackColor       =   &H00EED3C2&
            BackStyle       =   0  'Transparent
            Caption         =   "4. Analyze"
            ForeColor       =   &H00BF4F59&
            Height          =   195
            Left            =   120
            TabIndex        =   90
            Tag             =   "1044"
            Top             =   1800
            Width           =   2140
         End
         Begin VB.Line Line2 
            BorderColor     =   &H00808080&
            Index           =   1
            X1              =   2550
            X2              =   2550
            Y1              =   0
            Y2              =   4920
         End
         Begin VB.Label Label12 
            BackStyle       =   0  'Transparent
            Caption         =   "Steps"
            BeginProperty Font 
               Name            =   "Arial"
               Size            =   8.25
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Index           =   3
            Left            =   120
            TabIndex        =   66
            Tag             =   "1040"
            Top             =   240
            Width           =   1335
         End
         Begin VB.Label Label9 
            BackColor       =   &H00EED3C2&
            Caption         =   "2. Documents"
            ForeColor       =   &H00BF4F59&
            Height          =   255
            Index           =   2
            Left            =   120
            TabIndex        =   65
            Tag             =   "1042"
            Top             =   1080
            Width           =   2140
         End
         Begin VB.Label Label8 
            BackColor       =   &H00EED3C2&
            BackStyle       =   0  'Transparent
            Caption         =   "3. Results"
            ForeColor       =   &H00BF4F59&
            Height          =   255
            Index           =   2
            Left            =   120
            TabIndex        =   64
            Tag             =   "1043"
            Top             =   1440
            Width           =   2140
         End
         Begin VB.Label Label7 
            BackColor       =   &H00EED3C2&
            BackStyle       =   0  'Transparent
            Caption         =   "1. Introduction"
            ForeColor       =   &H00BF4F59&
            Height          =   255
            Index           =   2
            Left            =   120
            TabIndex        =   63
            Tag             =   "1041"
            Top             =   720
            Width           =   2140
         End
         Begin VB.Line Line3 
            Index           =   2
            X1              =   120
            X2              =   2280
            Y1              =   480
            Y2              =   480
         End
      End
      Begin VB.PictureBox Picture7 
         Appearance      =   0  'Flat
         BorderStyle     =   0  'None
         ForeColor       =   &H80000008&
         Height          =   4725
         Left            =   2580
         ScaleHeight     =   4725
         ScaleWidth      =   5535
         TabIndex        =   58
         TabStop         =   0   'False
         Top             =   0
         Width           =   5535
         Begin VB.ComboBox cbIgnoreOld 
            Height          =   330
            ItemData        =   "Wizard.frx":56F67
            Left            =   3950
            List            =   "Wizard.frx":56F74
            Style           =   2  'Dropdown List
            TabIndex        =   99
            Top             =   1570
            Width           =   1215
         End
         Begin VB.CheckBox chkIgnoreOld 
            Caption         =   "Ignore documents older than"
            Height          =   225
            Left            =   450
            TabIndex        =   98
            Top             =   1600
            Width           =   3400
         End
         Begin VB.CheckBox chkWordDoc 
            Caption         =   "Documents (*.doc)"
            Height          =   225
            Left            =   2160
            TabIndex        =   19
            Tag             =   "1208"
            Top             =   2600
            Value           =   1  'Checked
            Width           =   3200
         End
         Begin VB.CheckBox chkWordTemplate 
            Caption         =   "Templates (*.dot)"
            Height          =   225
            Left            =   2160
            TabIndex        =   20
            Tag             =   "1209"
            Top             =   2900
            Width           =   3200
         End
         Begin VB.CheckBox chkPPTemplate 
            Caption         =   "Templates (*.pot)"
            Height          =   225
            Left            =   2160
            TabIndex        =   24
            Tag             =   "1215"
            Top             =   4400
            Width           =   3200
         End
         Begin VB.CheckBox chkPPDoc 
            Caption         =   "Presentations (*.ppt)"
            Height          =   225
            Left            =   2160
            TabIndex        =   23
            Tag             =   "1214"
            Top             =   4100
            Width           =   3200
         End
         Begin VB.CheckBox chkExcelDoc 
            Caption         =   "Spreadsheets (*.xls)"
            Height          =   225
            Left            =   2160
            TabIndex        =   21
            Tag             =   "1211"
            Top             =   3350
            Width           =   3200
         End
         Begin VB.CheckBox chkExcelTemplate 
            Caption         =   "Templates (*.xlt)"
            Height          =   225
            Left            =   2160
            TabIndex        =   22
            Tag             =   "1212"
            Top             =   3650
            Width           =   3200
         End
         Begin VB.CommandButton btnBrowseDirInput 
            Caption         =   "..."
            Height          =   315
            Left            =   4740
            TabIndex        =   17
            Top             =   900
            Width           =   400
         End
         Begin VB.TextBox txtInputDir 
            Height          =   315
            Left            =   450
            TabIndex        =   16
            Tag             =   "1205"
            Text            =   "C:\"
            Top             =   900
            Width           =   4155
         End
         Begin VB.CheckBox chkIncludeSubdirs 
            Caption         =   "Include subdirectories in the analysis"
            Height          =   225
            Left            =   450
            TabIndex        =   18
            Tag             =   "1202"
            Top             =   1300
            Width           =   4965
         End
         Begin VB.Label lblDocTypes 
            Caption         =   "Document types to analyze"
            Height          =   225
            Left            =   450
            TabIndex        =   95
            Tag             =   "1206"
            Top             =   2250
            Width           =   4905
         End
         Begin VB.Label lblChooseDocs 
            AutoSize        =   -1  'True
            Caption         =   "Choose the documents you want to analyze"
            BeginProperty Font 
               Name            =   "Arial"
               Size            =   8.25
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   195
            Left            =   300
            TabIndex        =   94
            Tag             =   "1200"
            Top             =   240
            Width           =   5115
            WordWrap        =   -1  'True
         End
         Begin VB.Label Label13 
            AutoSize        =   -1  'True
            Caption         =   "PowerPoint"
            Height          =   225
            Index           =   2
            Left            =   690
            TabIndex        =   74
            Tag             =   "1213"
            Top             =   4100
            Width           =   1245
            WordWrap        =   -1  'True
         End
         Begin VB.Label Label13 
            AutoSize        =   -1  'True
            Caption         =   "Excel"
            Height          =   225
            Index           =   1
            Left            =   690
            TabIndex        =   73
            Tag             =   "1210"
            Top             =   3350
            Width           =   1245
            WordWrap        =   -1  'True
         End
         Begin VB.Label Label13 
            AutoSize        =   -1  'True
            Caption         =   "Word"
            Height          =   225
            Index           =   0
            Left            =   690
            TabIndex        =   72
            Tag             =   "1207"
            Top             =   2600
            Width           =   1245
            WordWrap        =   -1  'True
         End
         Begin VB.Label Label1 
            Caption         =   "Location of Microsoft Office documents"
            Height          =   200
            Left            =   450
            TabIndex        =   59
            Tag             =   "1201"
            Top             =   600
            Width           =   4935
         End
      End
   End
   Begin VB.Frame fraStep 
      BorderStyle     =   0  'None
      Caption         =   "Options"
      Enabled         =   0   'False
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   4905
      Index           =   2
      Left            =   -10000
      TabIndex        =   33
      Tag             =   "2002"
      Top             =   0
      Width           =   8235
      Begin VB.PictureBox Picture11 
         BorderStyle     =   0  'None
         Height          =   555
         Left            =   7260
         ScaleHeight     =   555
         ScaleWidth      =   705
         TabIndex        =   75
         Top             =   1890
         Width           =   705
         Begin VB.CommandButton btnBrowseDirOut 
            Caption         =   "..."
            Height          =   375
            Left            =   90
            TabIndex        =   28
            Top             =   90
            Width           =   495
         End
      End
      Begin VB.TextBox txtResultsName 
         Height          =   375
         Left            =   3030
         TabIndex        =   26
         Tag             =   "1302"
         Text            =   "Analysis Results.xls"
         Top             =   1140
         Width           =   3045
      End
      Begin VB.TextBox txtOutputDir 
         Height          =   375
         Left            =   3030
         TabIndex        =   27
         Top             =   1980
         Width           =   4185
      End
      Begin VB.PictureBox Picture5 
         Appearance      =   0  'Flat
         BorderStyle     =   0  'None
         ForeColor       =   &H80000008&
         Height          =   1365
         Left            =   3300
         ScaleHeight     =   1365
         ScaleWidth      =   4635
         TabIndex        =   57
         TabStop         =   0   'False
         Top             =   3210
         Width           =   4635
         Begin VB.OptionButton rdbResultsPrompt 
            Caption         =   "Ask me before overwriting"
            Height          =   435
            Left            =   0
            TabIndex        =   29
            Tag             =   "1312"
            Top             =   0
            Value           =   -1  'True
            Width           =   4485
         End
         Begin VB.OptionButton rdbResultsOverwrite 
            Caption         =   "Overwrite without asking me"
            Height          =   435
            Left            =   0
            TabIndex        =   30
            Tag             =   "1313"
            Top             =   450
            Width           =   4455
         End
         Begin VB.OptionButton rdbResultsAppend 
            Caption         =   "Append the new results to the existing results"
            Height          =   675
            Left            =   0
            TabIndex        =   31
            Tag             =   "1314"
            Top             =   780
            Visible         =   0   'False
            Width           =   4515
         End
      End
      Begin VB.Frame Frame3 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         BorderStyle     =   0  'None
         Enabled         =   0   'False
         ForeColor       =   &H0099A8AC&
         Height          =   5175
         Index           =   0
         Left            =   0
         TabIndex        =   39
         Top             =   0
         Width           =   2535
         Begin VB.PictureBox Picture1 
            Appearance      =   0  'Flat
            BackColor       =   &H80000005&
            BorderStyle     =   0  'None
            ForeColor       =   &H80000008&
            Height          =   1200
            Index           =   3
            Left            =   350
            Picture         =   "Wizard.frx":56F97
            ScaleHeight     =   1200
            ScaleWidth      =   1980
            TabIndex        =   40
            TabStop         =   0   'False
            Top             =   3690
            Width           =   1980
         End
         Begin VB.Label lblStep3_4 
            BackColor       =   &H00EED3C2&
            BackStyle       =   0  'Transparent
            Caption         =   "4. Analyze"
            ForeColor       =   &H00BF4F59&
            Height          =   195
            Left            =   120
            TabIndex        =   91
            Tag             =   "1044"
            Top             =   1800
            Width           =   2140
         End
         Begin VB.Label Label12 
            BackStyle       =   0  'Transparent
            Caption         =   "Steps"
            BeginProperty Font 
               Name            =   "Arial"
               Size            =   8.25
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Index           =   0
            Left            =   120
            TabIndex        =   53
            Tag             =   "1040"
            Top             =   240
            Width           =   1335
         End
         Begin VB.Label Label9 
            BackColor       =   &H00EED3C2&
            BackStyle       =   0  'Transparent
            Caption         =   "2. Documents"
            ForeColor       =   &H00BF4F59&
            Height          =   255
            Index           =   0
            Left            =   120
            TabIndex        =   43
            Tag             =   "1042"
            Top             =   1080
            Width           =   2140
         End
         Begin VB.Label Label8 
            BackColor       =   &H00EED3C2&
            Caption         =   "3. Results"
            ForeColor       =   &H00BF4F59&
            Height          =   255
            Index           =   0
            Left            =   120
            TabIndex        =   42
            Tag             =   "1043"
            Top             =   1440
            Width           =   2140
         End
         Begin VB.Label Label7 
            BackColor       =   &H00EED3C2&
            BackStyle       =   0  'Transparent
            Caption         =   "1. Introduction"
            ForeColor       =   &H00BF4F59&
            Height          =   255
            Index           =   0
            Left            =   120
            TabIndex        =   41
            Tag             =   "1041"
            Top             =   720
            Width           =   2140
         End
         Begin VB.Line Line3 
            Index           =   0
            X1              =   120
            X2              =   2280
            Y1              =   480
            Y2              =   480
         End
      End
      Begin VB.Label Label3 
         Caption         =   "File name for the results spreadsheet"
         Height          =   195
         Left            =   3030
         TabIndex        =   71
         Tag             =   "1301"
         Top             =   840
         Width           =   4785
      End
      Begin VB.Label lblResultsLocation 
         Caption         =   "Location"
         Height          =   195
         Left            =   3030
         TabIndex        =   70
         Tag             =   "1304"
         Top             =   1710
         Width           =   4755
      End
      Begin VB.Label Label13 
         AutoSize        =   -1  'True
         Caption         =   "If results already exisit under the same name and location:"
         Height          =   195
         Index           =   5
         Left            =   3030
         TabIndex        =   38
         Tag             =   "1311"
         Top             =   2730
         Width           =   4230
         WordWrap        =   -1  'True
      End
      Begin VB.Line Line2 
         BorderColor     =   &H00808080&
         Index           =   0
         X1              =   2550
         X2              =   2550
         Y1              =   0
         Y2              =   4920
      End
      Begin VB.Label lblChooseResults 
         AutoSize        =   -1  'True
         Caption         =   "Choose where and how to save the analysis results"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   8.25
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   195
         Left            =   2880
         TabIndex        =   37
         Tag             =   "1300"
         Top             =   240
         Width           =   5055
         WordWrap        =   -1  'True
      End
   End
   Begin VB.Frame fraStep 
      BorderStyle     =   0  'None
      Caption         =   "Analyze"
      Enabled         =   0   'False
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   4905
      Index           =   3
      Left            =   0
      TabIndex        =   34
      Tag             =   "3000"
      Top             =   0
      Width           =   2.45745e5
      Begin VB.PictureBox Picture12 
         Appearance      =   0  'Flat
         BorderStyle     =   0  'None
         ForeColor       =   &H80000008&
         Height          =   4905
         Left            =   10020
         ScaleHeight     =   4905
         ScaleWidth      =   8175
         TabIndex        =   69
         TabStop         =   0   'False
         Top             =   0
         Width           =   8175
      End
      Begin VB.CommandButton btnPrepare 
         Caption         =   "Prepare"
         Enabled         =   0   'False
         Height          =   375
         Left            =   3340
         TabIndex        =   97
         Tag             =   "1411"
         Top             =   4410
         Visible         =   0   'False
         Width           =   4000
      End
      Begin VB.CommandButton btnRunAnalysis 
         Caption         =   "Run"
         Height          =   375
         Left            =   3340
         TabIndex        =   35
         Tag             =   "1404"
         Top             =   3410
         Width           =   4000
      End
      Begin VB.CommandButton btnViewResults 
         Caption         =   "View"
         Enabled         =   0   'False
         Height          =   375
         Left            =   3340
         TabIndex        =   36
         Tag             =   "1406"
         Top             =   3910
         Width           =   4000
      End
      Begin VB.Frame Frame3 
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         BorderStyle     =   0  'None
         Enabled         =   0   'False
         ForeColor       =   &H0099A8AC&
         Height          =   5175
         Index           =   3
         Left            =   0
         TabIndex        =   44
         Top             =   0
         Width           =   2535
         Begin VB.PictureBox Picture4 
            Appearance      =   0  'Flat
            BackColor       =   &H80000005&
            BorderStyle     =   0  'None
            ForeColor       =   &H80000008&
            Height          =   1575
            Index           =   2
            Left            =   150
            ScaleHeight     =   1575
            ScaleWidth      =   2385
            TabIndex        =   55
            TabStop         =   0   'False
            Top             =   3390
            Width           =   2385
            Begin VB.PictureBox Picture1 
               Appearance      =   0  'Flat
               BackColor       =   &H80000005&
               BorderStyle     =   0  'None
               ForeColor       =   &H80000008&
               Height          =   1200
               Index           =   2
               Left            =   200
               Picture         =   "Wizard.frx":5E58D
               ScaleHeight     =   1200
               ScaleWidth      =   2475
               TabIndex        =   56
               TabStop         =   0   'False
               Tag             =   "1060"
               Top             =   300
               Width           =   2480
            End
         End
         Begin VB.Label lblStep4_4 
            BackColor       =   &H00EED3C2&
            Caption         =   "4. Analyze"
            ForeColor       =   &H00BF4F59&
            Height          =   255
            Left            =   120
            TabIndex        =   92
            Tag             =   "1044"
            Top             =   1800
            Width           =   2140
         End
         Begin VB.Label Label12 
            BackStyle       =   0  'Transparent
            Caption         =   "Steps"
            BeginProperty Font 
               Name            =   "Arial"
               Size            =   8.25
               Charset         =   0
               Weight          =   700
               Underline       =   0   'False
               Italic          =   0   'False
               Strikethrough   =   0   'False
            EndProperty
            Height          =   255
            Index           =   5
            Left            =   120
            TabIndex        =   54
            Tag             =   "1040"
            Top             =   240
            Width           =   1335
         End
         Begin VB.Label Label9 
            BackColor       =   &H00EED3C2&
            BackStyle       =   0  'Transparent
            Caption         =   "2. Documents"
            ForeColor       =   &H00BF4F59&
            Height          =   255
            Index           =   3
            Left            =   120
            TabIndex        =   47
            Tag             =   "1042"
            Top             =   1080
            Width           =   2140
         End
         Begin VB.Label Label8 
            BackColor       =   &H00EED3C2&
            BackStyle       =   0  'Transparent
            Caption         =   "3. Results"
            ForeColor       =   &H00BF4F59&
            Height          =   255
            Index           =   3
            Left            =   120
            TabIndex        =   46
            Tag             =   "1043"
            Top             =   1440
            Width           =   2140
         End
         Begin VB.Label Label7 
            BackColor       =   &H00EED3C2&
            BackStyle       =   0  'Transparent
            Caption         =   "1. Introduction"
            ForeColor       =   &H00BF4F59&
            Height          =   255
            Index           =   3
            Left            =   120
            TabIndex        =   45
            Tag             =   "1041"
            Top             =   720
            Width           =   2140
         End
         Begin VB.Line Line3 
            Index           =   3
            X1              =   120
            X2              =   2280
            Y1              =   480
            Y2              =   480
         End
      End
      Begin VB.Label lblSkippedOld 
         Caption         =   "Skipped <TOPIC> documets, because they were too old"
         Height          =   195
         Left            =   3180
         TabIndex        =   60
         Top             =   2880
         Width           =   4935
      End
      Begin VB.Label lblSetupDone 
         AutoSize        =   -1  'True
         Caption         =   "Run the analysis and view the results"
         BeginProperty Font 
            Name            =   "Arial"
            Size            =   8.25
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   195
         Left            =   2880
         TabIndex        =   96
         Tag             =   "1400"
         Top             =   240
         Width           =   4905
         WordWrap        =   -1  'True
      End
      Begin VB.Label lblNumPPT 
         Caption         =   "<TOPIC> Presentations"
         Height          =   255
         Left            =   4620
         TabIndex        =   88
         Tag             =   "1409"
         Top             =   2280
         Width           =   3375
      End
      Begin VB.Label lblNumPOT 
         Caption         =   "<TOPIC> Templates"
         Height          =   255
         Left            =   4620
         TabIndex        =   87
         Tag             =   "1403"
         Top             =   2550
         Width           =   3375
      End
      Begin VB.Label lblNumXLS 
         Caption         =   "<TOPIC> Spreadsheets"
         Height          =   255
         Left            =   4620
         TabIndex        =   86
         Tag             =   "1408"
         Top             =   1680
         Width           =   3375
      End
      Begin VB.Label lblNumXLT 
         Caption         =   "<TOPIC> Templates"
         Height          =   255
         Left            =   4620
         TabIndex        =   85
         Tag             =   "1403"
         Top             =   1950
         Width           =   3375
      End
      Begin VB.Label Label16 
         AutoSize        =   -1  'True
         Caption         =   "PowerPoint"
         Height          =   195
         Left            =   3360
         TabIndex        =   82
         Tag             =   "1213"
         Top             =   2280
         Width           =   1095
         WordWrap        =   -1  'True
      End
      Begin VB.Label Label13 
         Caption         =   "Word"
         Height          =   705
         Index           =   10
         Left            =   0
         TabIndex        =   81
         Tag             =   "1207"
         Top             =   0
         Width           =   1245
      End
      Begin VB.Label Label13 
         Caption         =   "Excel"
         Height          =   705
         Index           =   9
         Left            =   0
         TabIndex        =   80
         Tag             =   "1210"
         Top             =   810
         Width           =   1245
      End
      Begin VB.Label Label13 
         Caption         =   "PowerPoint"
         Height          =   585
         Index           =   8
         Left            =   0
         TabIndex        =   79
         Tag             =   "1213"
         Top             =   1620
         Width           =   1245
      End
      Begin VB.Label Label13 
         Caption         =   "Word"
         Height          =   585
         Index           =   7
         Left            =   0
         TabIndex        =   78
         Tag             =   "1207"
         Top             =   0
         Width           =   1245
      End
      Begin VB.Label Label13 
         Caption         =   "Excel"
         Height          =   585
         Index           =   6
         Left            =   0
         TabIndex        =   77
         Tag             =   "1210"
         Top             =   810
         Width           =   1245
      End
      Begin VB.Label Label13 
         Caption         =   "PowerPoint"
         Height          =   465
         Index           =   4
         Left            =   0
         TabIndex        =   76
         Tag             =   "1213"
         Top             =   1620
         Width           =   1245
      End
      Begin VB.Label lblNumTemplates 
         Caption         =   "<TOPIC> Templates"
         Height          =   255
         Left            =   4620
         TabIndex        =   52
         Tag             =   "1403"
         Top             =   1350
         Width           =   3375
      End
      Begin VB.Label lblNumDocs 
         Caption         =   "<TOPIC> Documents"
         Height          =   255
         Left            =   4620
         TabIndex        =   51
         Tag             =   "1402"
         Top             =   1080
         Width           =   3375
      End
      Begin VB.Line Line6 
         BorderColor     =   &H00808080&
         X1              =   2640
         X2              =   8040
         Y1              =   3270
         Y2              =   3270
      End
      Begin VB.Label Label15 
         AutoSize        =   -1  'True
         Caption         =   "Excel"
         Height          =   195
         Left            =   3360
         TabIndex        =   50
         Tag             =   "1210"
         Top             =   1680
         Width           =   1095
         WordWrap        =   -1  'True
      End
      Begin VB.Label Label14 
         AutoSize        =   -1  'True
         Caption         =   "Word"
         Height          =   195
         Left            =   3360
         TabIndex        =   49
         Tag             =   "1207"
         Top             =   1080
         Width           =   1110
         WordWrap        =   -1  'True
      End
      Begin VB.Label lblTotalNumDocs 
         AutoSize        =   -1  'True
         Caption         =   "A total of <TOPIC> documents will be analyzed:"
         Height          =   195
         Left            =   3180
         TabIndex        =   48
         Tag             =   "1401"
         Top             =   660
         Width           =   4800
         WordWrap        =   -1  'True
      End
      Begin VB.Line Line2 
         BorderColor     =   &H00808080&
         Index           =   3
         X1              =   2550
         X2              =   2550
         Y1              =   0
         Y2              =   4920
      End
   End
   Begin VB.PictureBox picNav 
      Align           =   2  'Align Bottom
      Appearance      =   0  'Flat
      BorderStyle     =   0  'None
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H80000008&
      Height          =   570
      Left            =   0
      ScaleHeight     =   570
      ScaleWidth      =   8175
      TabIndex        =   4
      TabStop         =   0   'False
      Top             =   4950
      Width           =   8175
      Begin VB.CommandButton cmdNav 
         Caption         =   "Finish"
         Height          =   312
         Index           =   4
         Left            =   5325
         MaskColor       =   &H00000000&
         TabIndex        =   14
         Tag             =   "1023"
         Top             =   120
         Width           =   1320
      End
      Begin VB.CommandButton cmdNav 
         Caption         =   "Next >>"
         Height          =   312
         Index           =   3
         Left            =   3870
         MaskColor       =   &H00000000&
         TabIndex        =   13
         Tag             =   "1022"
         Top             =   120
         Width           =   1320
      End
      Begin VB.CommandButton cmdNav 
         Caption         =   "<< Back"
         Height          =   312
         Index           =   2
         Left            =   2535
         MaskColor       =   &H00000000&
         TabIndex        =   12
         Tag             =   "1021"
         Top             =   120
         Width           =   1320
      End
      Begin VB.CommandButton cmdNav 
         Cancel          =   -1  'True
         Caption         =   "Cancel"
         Height          =   312
         Index           =   1
         Left            =   6750
         MaskColor       =   &H00000000&
         TabIndex        =   15
         Tag             =   "1024"
         Top             =   120
         Width           =   1320
      End
   End
   Begin VB.Label Label18 
      Caption         =   "<TOPIC> Documents"
      Height          =   255
      Left            =   0
      TabIndex        =   84
      Top             =   0
      Width           =   2085
      WordWrap        =   -1  'True
   End
   Begin VB.Label Label17 
      Caption         =   "<TOPIC> Templates"
      Height          =   255
      Left            =   0
      TabIndex        =   83
      Top             =   390
      Width           =   3615
      WordWrap        =   -1  'True
   End
   Begin VB.Line Line4 
      BorderColor     =   &H00808080&
      X1              =   0
      X2              =   8160
      Y1              =   4920
      Y2              =   4920
   End
End
Attribute VB_Name = "frmWizard"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
' *******************************************************************************
' *
' * Copyright 2000, 2010 Oracle and/or its affiliates. All rights reserved. Use of this
' * product is subject to license terms.
' *
' *******************************************************************************

Option Explicit

Const TOPIC_STR = "<TOPIC>"
Const TOPIC2_STR = "<TOPIC2>"
Const CR_STR = "<CR>"
Const CDEBUG_LEVEL_DEFAULT = 1 'Will output all Debug output to analysis.log file
Const CSUPPORTED_VERSION = 9#

Const NUM_STEPS = 4

Const CAPPNAME_WORD = "Word"
Const CAPPNAME_EXCEL = "Excel"
Const CAPPNAME_POWERPOINT = "PowerPoint"
Const CANALYZING = "Analyzing"

Const BTN_CANCEL = 1
Const BTN_BACK = 2
Const BTN_NEXT = 3
Const BTN_FINISH = 4

Const STEP_INTRO = 0
Const STEP_1 = 1
Const STEP_2 = 2
Const STEP_FINISH = 3

Const DIR_NONE = 0
Const DIR_BACK = 1
Const DIR_NEXT = 2

Const CPRODUCTNAME_STR = "<PRODUCTNAME>"

Const CSTR_ANALYSIS_LOG_DONE = "Done"

Const CINPUT_DIR = "indir"
Const COUTPUT_DIR = "outdir"
Const CRESULTS_FILE = "resultsfile"
Const CLOG_FILE = "logfile"
Const CRESULTS_TEMPLATE = "resultstemplate"
Const CRESULTS_EXIST = "resultsexist"
Const CPROMPT_FILE = "promptfile"
Const COVERWRITE_FILE = "overwritefile"
Const CAPPEND_FILE = "appendfile"
Const CNEW_RESULTS_FILE = "newresultsfile"
Const CINCLUDE_SUBDIRS = "includesubdirs"
Const CDEBUG_LEVEL = "debuglevel"
Const CTYPE_WORDDOC = "typeworddoc"
Const CTYPE_WORDDOT = "typeworddot"
Const CTYPE_EXCELDOC = "typeexceldoc"
Const CTYPE_EXCELDOT = "typeexceldot"
Const CTYPE_PPDOC = "typepowerpointdoc"
Const CTYPE_PPDOT = "typepowerpointdot"
Const COUTPUT_TYPE = "outputtype"
Const COUTPUT_TYPE_XLS = "xls"
Const COUTPUT_TYPE_XML = "xml"
Const COUTPUT_TYPE_BOTH = "both"
Const CVERSION = "version"
Const CDOPREPARE = "prepare"
Const CTITLE = "title"
Const CIGNORE_OLD_DOCS = "ignoreolddocuments"
Const CISSUE_LIMIT = "issuesmonthlimit"
Const CISSUE_LIMIT_DAW = 6
Private mIssueLimit As Integer
Const CDEFAULT_PASSWORD = "defaultpassword"
Const CSTR_TEST_PASSWORD = "test"
Private mDefaultPassword As String

Const CLAST_CHECKPOINT  As String = "LastCheckpoint"
Const CNEXT_FILE        As String = "NextFile"
Const C_ABORT_ANALYSIS  As String = "AbortAnalysis"

Const CNUMBER_TOTAL_DOCS = "total_numberdocs"
Const CNUMBER_DOCS_DOC = "numberdocs_doc"
Const CNUMBER_TEMPLATES_DOT = "numbertemplates_dot"
Const CNUMBER_DOCS_XLS = "numberdocs_xls"
Const CNUMBER_TEMPLATES_XLT = "numbertemplates_xlt"
Const CNUMBER_DOCS_PPT = "numberdocs_ppt"
Const CNUMBER_TEMPLATES_POT = "numbertemplates_pot"
Const CSTART_TIME = "start"
Const CEND_TIME = "end"
Const CELAPSED_TIME = "time_for_analysis"
Const CWINVERSION = "win_version"
Const CUSER_LOCALE_INFO = "user_locale"
Const CSYS_LOCALE_INFO = "system_locale"
Const CWORD_VERSION = "word_ver"
Const CEXCEL_VERSION = "excel_ver"
Const CPOWERPOINT_VERSION = "powerpoint_ver"
Const CNOT_INSTALLED = "not installed"
   
Const CRESULTS_FILE_EXTENSION = ".xls"
Const CCONFIG_BACKUP_EXT = "_bak"
Const CDEFAULT_README_NAME = "UserGuide"

Const C_DOCS_LESS_3_MONTH = "DocumentsYoungerThan3Month"
Const C_DOCS_LESS_6_MONTH = "DocumentsYoungerThan6Month"
Const C_DOCS_LESS_12_MONTH = "DocumentsYoungerThan12Month"
Const C_DOCS_MORE_12_MONTH = "DocumentsOlderThan12Month"

'module level vars
Dim mnCurStep As Integer
Dim mbTrue As Boolean
Dim mbFalse As Boolean
Dim mLblSteps As String
Dim mChbSubdirs As String

Dim mWordDocCount As Long
Dim mExcelDocCount As Long
Dim mPPDocCount As Long

Dim mWordTemplateCount As Long
Dim mExcelTemplateCount As Long
Dim mPPTemplateCount As Long
Dim mTotalDocCount As Long
Dim mIgnoredDocCount As Long

Public VBInst       As VBIDE.VBE
Dim mbFinishOK      As Boolean
Dim mbAllowExit   As Boolean
Private mStrTrue As String
Private mLogFilePath As String
Private mDebugLevel As String
Private mIniFilePath As String
Private mbDocCountCurrent As Boolean
Private mbDoPrepare As Boolean

Dim mDocFiles As CollectedFiles

Private Declare Sub InitCommonControls Lib "comctl32" ()
Private Declare Function GetTickCount Lib "kernel32" () As Long
Private Declare Sub Sleep Lib "kernel32" (ByVal dwMilliseconds As Long)

Const FORMAT_MESSAGE_FROM_SYSTEM = &H1000

Private Declare Function FormatMessage Lib "kernel32" Alias _
      "FormatMessageA" (ByVal dwFlags As Long, lpSource As Long, _
      ByVal dwMessageId As Long, ByVal dwLanguageId As Long, _
      ByVal lpBuffer As String, ByVal nSize As Long, Arguments As Any) As Long
      

Private Const HKEY_CURRENT_USER   As Long = &H80000001
Private Const HKEY_LOCAL_MACHINE  As Long = &H80000002

Const WORD_APP = "word"
Const EXCEL_APP = "excel"
Const PP_APP = "pp"
Const REG_KEY_APP_PATH = "Software\Microsoft\Windows\CurrentVersion\App Paths\"


Function GetAppPath(myApp As String) As String
    Dim myPath As String
    
    If (myApp = WORD_APP) Then
        myPath = GetRegistryInfo(HKEY_LOCAL_MACHINE, REG_KEY_APP_PATH & "winword.exe", "")
    ElseIf (myApp = EXCEL_APP) Then
        myPath = GetRegistryInfo(HKEY_LOCAL_MACHINE, REG_KEY_APP_PATH & "excel.exe", "")
    ElseIf (myApp = PP_APP) Then
        myPath = GetRegistryInfo(HKEY_LOCAL_MACHINE, REG_KEY_APP_PATH & "powerpnt.exe", "")
    Else
        MsgBox "Unknown application: " & myApp, vbCritical
        Exit Function
    End If
    
    If (myPath = "") Then
        If (myApp = WORD_APP) Then
            myPath = GetRegistryInfo(HKEY_CURRENT_USER, REG_KEY_APP_PATH & "winword.exe", "")
        ElseIf (myApp = EXCEL_APP) Then
            myPath = GetRegistryInfo(HKEY_CURRENT_USER, REG_KEY_APP_PATH & "excel.exe", "")
        ElseIf (myApp = PP_APP) Then
            myPath = GetRegistryInfo(HKEY_CURRENT_USER, REG_KEY_APP_PATH & "powerpnt.exe", "")
        End If
    End If
    
    GetAppPath = myPath
End Function

Function GetDriverDoc(myApp As String) As String
    Dim myPath As String
    Dim errStr As String
    Dim fso As New FileSystemObject

    If (myApp = WORD_APP) Then
        myPath = fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CWORD_DRIVER_FILE)
    ElseIf (myApp = EXCEL_APP) Then
        myPath = fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CEXCEL_DRIVER_FILE)
    ElseIf (myApp = PP_APP) Then
        myPath = fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CPP_DRIVER_FILE)
    Else
        MsgBox "Unknown application: " & myApp, vbCritical
        GoTo FinalExit
    End If

    If Not fso.FileExists(myPath) Then
        errStr = ReplaceTopic2Tokens(GetResString(ERR_MISSING_WORD_DRIVER), _
                    TOPIC_STR, myPath, CR_STR, Chr(13))
        WriteDebug errStr
        MsgBox errStr, vbCritical
        GoTo FinalExit
    End If

    GetDriverDoc = myPath

FinalExit:
    Set fso = Nothing
End Function




Private Function AutomationMessageText(lCode As Long) As String
    Dim sRtrnCode As String
    Dim lRet As Long

    sRtrnCode = Space$(256)
    lRet = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0&, lCode, 0&, _
              sRtrnCode, 256&, 0&)
    If lRet > 0 Then
       AutomationMessageText = Left(sRtrnCode, lRet)
    Else
       AutomationMessageText = "Error not found."
    End If

End Function

Private Sub btnBrowseDirInput_Click()
    Dim folder As String
    Dim StartDir As String
    
    If Len(txtInputDir.Text) > 0 Then
        StartDir = txtInputDir.Text
    End If
    
    folder = BrowseForFolder(Me, GetResString(BROWSE_FOR_DOC_DIR_ID), StartDir)
    If Len(folder) = 0 Then
        Exit Sub  'User Selected Cancel
    End If
    txtInputDir.Text = folder
    txtInputDir.ToolTipText = folder
    
    If Len(txtOutputDir.Text) = 0 Then
        txtOutputDir.Text = folder
        txtOutputDir.ToolTipText = folder
    End If
End Sub

Private Sub btnBrowseDirOut_Click()
    Dim folder As String
    Dim StartDir As String
    
    If Len(txtOutputDir.Text) > 0 Then
        StartDir = txtOutputDir.Text
    End If
    
    folder = BrowseForFolder(Me, GetResString(BROWSE_FOR_RES_DIR_ID), StartDir)
    If Len(folder) = 0 Then
        Exit Sub  'User Selected Cancel
    End If
    txtOutputDir.Text = folder
    txtOutputDir.ToolTipText = folder
End Sub

Private Sub btnPrepare_Click()
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "btnPrepare_Click"
    
    mbDoPrepare = True
    mbAllowExit = True

    btnViewResults.Enabled = False
    btnRunAnalysis.Enabled = False
    btnPrepare.Enabled = False

    cmdNav(BTN_CANCEL).Enabled = False
    cmdNav(BTN_BACK).Enabled = False
    cmdNav(BTN_NEXT).Enabled = False
    cmdNav(BTN_FINISH).Enabled = False
    btnPrepare.Caption = GetResString(RUNBTN_RUNNING_ID)

    Dim str As String

    If RunAnalysis(True) Then
        cmdNav(BTN_FINISH).Enabled = True
        btnRunAnalysis.Enabled = True
        btnViewResults.Enabled = True
        btnPrepare.Enabled = True
        btnViewResults.SetFocus
        str = ReplaceTopic2Tokens(GetResString(RID_STR_ENG_OTHER_PREPARE_COMPLETED_PREP_ID), _
                    TOPIC_STR, getOutputDir, CR_STR, Chr(13))
        MsgBox str, vbInformation
    Else
        cmdNav(BTN_FINISH).Enabled = False
        btnRunAnalysis.Enabled = True
        btnViewResults.Enabled = False
        btnPrepare.Enabled = False
    End If

FinalExit:
    mbDoPrepare = False
    cmdNav(BTN_CANCEL).Enabled = True
    cmdNav(BTN_BACK).Enabled = True
    cmdNav(BTN_NEXT).Enabled = False
    btnPrepare.Caption = GetResString(PREPAREBTN_START_ID)
    Exit Sub
    
HandleErrors:
    cmdNav(BTN_FINISH).Enabled = False
    btnRunAnalysis.Enabled = True
    btnViewResults.Enabled = False
    btnPrepare.Enabled = False
    
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub


Private Sub cmdNav_Click(Index As Integer)
    On Error GoTo HandleError
    Dim currentFunctionName As String
    currentFunctionName = "cmdNav_Click"
    Dim nAltStep As Integer
    Dim rc As Long
    Dim fso As Scripting.FileSystemObject
        
    Select Case Index
        Case BTN_CANCEL
            'Copy backup configuration file over existing
            If fso Is Nothing Then
                Set fso = New Scripting.FileSystemObject
            End If
            If fso.FileExists(mIniFilePath & CCONFIG_BACKUP_EXT) Then
                DeleteFile mIniFilePath
                AttemptToCopyFile mIniFilePath & CCONFIG_BACKUP_EXT, mIniFilePath
            End If
            Set mDocFiles = Nothing

            Unload Me
          
        Case BTN_BACK
            nAltStep = mnCurStep - 1
            SetStep nAltStep, DIR_BACK
          
        Case BTN_NEXT
            nAltStep = mnCurStep + 1
            SetStep nAltStep, DIR_NEXT
          
        Case BTN_FINISH
            If (Not mbAllowExit) Then
                Dim str As String
                Dim response As Integer

                str = ReplaceTopicTokens(GetResString(RID_STR_ENG_ANALYSE_NOT_RUN), CR_STR, Chr(13))
                response = MsgBox(str, vbOKCancel + vbInformation)
                If response = vbOK Then   ' User chose Ok.
                    mbAllowExit = True
                End If
            End If

            If (mbAllowExit) Then
                DeleteFile mIniFilePath & CCONFIG_BACKUP_EXT
                Set mDocFiles = Nothing
                Unload Me
            End If
    End Select

FinalExit:
    Set fso = Nothing
    Exit Sub
    
HandleError:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    
    Resume FinalExit
End Sub

Private Sub btnRunAnalysis_Click()
    On Error GoTo HandleErrors
    Dim bViewResults As Boolean
    Dim str As String
    Dim response As Integer

    btnViewResults.Enabled = False
    btnRunAnalysis.Enabled = False
    btnPrepare.Enabled = False
    bViewResults = False
    mbAllowExit = True

    cmdNav(BTN_CANCEL).Enabled = False
    cmdNav(BTN_BACK).Enabled = False
    cmdNav(BTN_NEXT).Enabled = False
    cmdNav(BTN_FINISH).Enabled = False
    btnRunAnalysis.Caption = GetResString(RUNBTN_RUNNING_ID)

    If RunAnalysis(False) Then
        cmdNav(BTN_FINISH).Enabled = True
        btnRunAnalysis.Enabled = True
        btnViewResults.Enabled = True
        btnPrepare.Enabled = True
        btnViewResults.SetFocus
        btnRunAnalysis.Caption = GetResString(RUNBTN_START_ID)

        str = ReplaceTopicTokens(GetResString(RID_STR_ENG_ANALYZE_COMPLETED_ID), CR_STR, Chr(13))
        response = MsgBox(str, vbOKCancel + vbInformation)
        If response = vbOK Then   ' User chose Ok.
            bViewResults = True
        End If
    Else
        btnRunAnalysis.Enabled = True
        btnViewResults.Enabled = False
        btnPrepare.Enabled = False
    End If

FinalExit:
    cmdNav(BTN_CANCEL).Enabled = True
    cmdNav(BTN_BACK).Enabled = True
    cmdNav(BTN_NEXT).Enabled = False
    btnRunAnalysis.Caption = GetResString(RUNBTN_START_ID)
    
    If bViewResults Then
        btnViewResults_Click
    End If
    
    Exit Sub
    
HandleErrors:
    cmdNav(BTN_FINISH).Enabled = False
    btnRunAnalysis.Enabled = True
    btnViewResults.Enabled = False
    btnPrepare.Enabled = False
    WriteDebug "Document Analysis: View Analysis Results" & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Private Sub btnViewResults_Click()
    On Error GoTo HandleErrors
    Dim resultsFile As String
    Dim fso As New FileSystemObject
    Dim str As String
    
    mbAllowExit = True
    
    resultsFile = getOutputDir & "\" & txtResultsName.Text

    If GetIniSetting(COUTPUT_TYPE) = COUTPUT_TYPE_XML Or _
        GetIniSetting(COUTPUT_TYPE) = COUTPUT_TYPE_BOTH Then

        Dim base As String
        Dim path As String
        base = fso.GetParentFolderName(resultsFile) & "\" & fso.GetBaseName(txtResultsName.Text)
        If CheckWordDocsToAnalyze Then
            path = base & "_" & CAPPNAME_WORD & "." & COUTPUT_TYPE_XML
        End If
        If CheckExcelDocsToAnalyze Then
            If path <> "" Then path = path & vbLf
            path = path & base & "_" & CAPPNAME_EXCEL & "." & COUTPUT_TYPE_XML
        End If
        If CheckPPDocsToAnalyze Then
            If path <> "" Then path = path & vbLf
            path = path & base & "_" & CAPPNAME_POWERPOINT & "." & COUTPUT_TYPE_XML
        End If

        str = ReplaceTopic2Tokens(GetResString(XML_RESULTS_ID), _
                    TOPIC_STR, path, CR_STR, Chr(13))
        WriteDebug str
        MsgBox str, vbInformation
        If GetIniSetting(COUTPUT_TYPE) = COUTPUT_TYPE_XML Then
            Resume FinalExit
        End If
    End If
    
    If Not fso.FileExists(resultsFile) Then
        str = ReplaceTopic2Tokens(GetResString(ERR_MISSING_RESULTS_DOC), _
                    TOPIC_STR, resultsFile, CR_STR, Chr(13))
        WriteDebug str
        MsgBox str, vbCritical
        Resume FinalExit
    End If

    Dim xl As Excel.application
    Set xl = New Excel.application
    xl.Visible = True
    xl.Workbooks.Open resultsFile
    
FinalExit:
    Set xl = Nothing
    Set fso = Nothing
    
    Exit Sub
HandleErrors:
    WriteDebug "Document Analysis: View Analysis Results" & Err.Number & " " & Err.Description & " " & Err.Source
End Sub

Private Sub Form_Activate()
    Dim currentFunctionName As String
    Dim missingFile As String
    currentFunctionName = "Form_Activate"
    On Error GoTo HandleErrors
    
    If Not CheckNeededFiles(missingFile) Then
        Dim str As String
        str = ReplaceTopic2Tokens(GetResString(ERR_MISSING_WORD_DRIVER), _
                    TOPIC_STR, missingFile, CR_STR, Chr(13))
        WriteDebug str
        MsgBox str, vbCritical

        End 'Exit application - some needed files are missing
    End If

FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    GoTo FinalExit
End Sub

Private Sub Form_Initialize()
    Dim currentFunctionName As String
    currentFunctionName = "Form_Initialize"
    On Error GoTo ErrorHandler
    Call InitCommonControls 'Use Windows XP Visual Style
    
FinalExit:
    Exit Sub
    
ErrorHandler:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    GoTo FinalExit
End Sub

Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
    If KeyCode = vbKeyF1 Then
        'cmdNav_Click BTN_HELP
    End If
End Sub

Private Sub Form_Load()
    Const COS_CHECK = "oscheck"
    
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "Form_Load"
    
    Dim fso As New FileSystemObject
    Dim i As Integer
    'init all vars
    mbFinishOK = False
    mbTrue = True
    mbFalse = False
    
    mLogFilePath = GetLogFilePath
    mIniFilePath = GetIniFilePath
    mbDocCountCurrent = False
    mbDoPrepare = False
    mbAllowExit = False
    
    'Check OS before running
    Dim bOSCheck As Boolean
    bOSCheck = IIf(GetIniSetting(COS_CHECK) = "False", False, True)
    
    If bOSCheck Then
        If Not IsWin98Plus Then
            Dim str As String
            Dim winVer As RGB_WINVER
            str = ReplaceTopic2Tokens(GetResString(ERR_SUPPORTED_OSVERSION), _
                        TOPIC_STR, GetWinVersion(winVer), CR_STR, Chr(13))
            WriteDebug str
            MsgBox str, vbCritical
        
            End 'Exit application - unsupported OS
        End If
    Else
        Err.Clear
        WriteDebug "IsWin2000Plus OS Check bypassed by analysis.ini oscheck=False setting"
    End If
    
    
    For i = 0 To NUM_STEPS - 1
      fraStep(i).Left = -10000
    Next
    
    'Load All string info for Form
    LoadResStrings Me
     
    frmWizard.Caption = ReplaceTopicTokens(GetResString(TITLE_ID), CPRODUCTNAME_STR, _
        GetResString(PRODUCTNAME_ID))
    lblIntroduction1.Caption = ReplaceTopicTokens(GetResString(INTRO1_ID), CPRODUCTNAME_STR, _
        GetResString(PRODUCTNAME_ID))
    mLblSteps = GetResString(LBL_STEPS_ID)
    mChbSubdirs = GetResString(CHK_SUBDIRS_ID)
    
    ' Setup Doc Preparation specific strings
    If gBoolPreparation Then
        ' Steps
        lblStep1_4.Caption = GetResString(RID_STR_ENG_SIDEBAR_ANALYZE_PREP_ID)
        lblStep2_4.Caption = GetResString(RID_STR_ENG_SIDEBAR_ANALYZE_PREP_ID)
        lblStep3_4.Caption = GetResString(RID_STR_ENG_SIDEBAR_ANALYZE_PREP_ID)
        lblStep4_4.Caption = GetResString(RID_STR_ENG_SIDEBAR_ANALYZE_PREP_ID)
        
        ' Preparation - Step 1. Introduction
        lblIntroduction1.Caption = ReplaceTopicTokens(GetResString(RID_STR_ENG_INTRODUCTION_INTRO1_PREP_ID), CPRODUCTNAME_STR, _
            GetResString(PRODUCTNAME_ID))
        lblIntroduction2.Caption = GetResString(RID_STR_ENG_INTRODUCTION_INTRO2_PREP_ID)
        lblIntroduction3.Caption = GetResString(RID_STR_ENG_INTRODUCTION_INTRO3_PREP_ID)
        
        ' Preparation - Step 2. Documents
        lblChooseDocs.Caption = GetResString(RID_STR_ENG_DOCUMENTS_CHOOSE_DOCUMENTS_PREP_ID)
        lblDocTypes.Caption = GetResString(RID_STR_ENG_DOCUMENTS_CHOOSE_DOC_TYPES_PREP_ID)
        'mChbSubdirs = GetResString(RID_STR_ENG_DOCUMENTS_INCLUDE_SUBDIRECTORIES_PREP_ID)
        chkIgnoreOld.Caption = GetResString(RID_STR_IGNORE_OLDER_CB_ID)

        cbIgnoreOld.Clear
        cbIgnoreOld.AddItem (GetResString(RID_STR_IGNORE_OLDER_3_MONTHS_ID))
        cbIgnoreOld.AddItem (GetResString(RID_STR_IGNORE_OLDER_6_MONTHS_ID))
        cbIgnoreOld.AddItem (GetResString(RID_STR_IGNORE_OLDER_12_MONTHS_ID))
        cbIgnoreOld.ListIndex = 0

        ' Preparation - Step 3. Results
        lblChooseResults.Caption = GetResString(RID_STR_ENG_RESULTS_CHOOSE_OPTIONS_PREP_ID)
        txtResultsName.Text = GetResString(RID_STR_ENG_RESULTS_ANALYSIS_XLS_PREP_ID)
        
        'Show Append option
        rdbResultsAppend.Visible = True
        
        ' Preparation - Step 4. Analysis
        lblSetupDone.Caption = GetResString(RID_STR_ENG_ANALYZE_SETUP_COMPLETE_PREP_ID)
        btnPrepare.Visible = True
    Else
        ' The next line is a work around for a wrong translated string and should be removed
        ' when RID_STR_ENG_RESULTS_CHOOSE_OPTIONS has been corrected
        lblChooseResults.Caption = GetResString(RID_STR_ENG_RESULTS_CHOOSE_OPTIONS_PREP_ID)
        mDefaultPassword = IIf(GetIniSetting(CDEFAULT_PASSWORD) = "", _
            CSTR_TEST_PASSWORD, GetIniSetting(CDEFAULT_PASSWORD))
    End If
        
    SetStep 0, DIR_NEXT
    Dim tmpStr As String
    
    'Setup Params
    tmpStr = GetIniSetting(CINPUT_DIR)
    If tmpStr <> "" Then
        txtInputDir.Text = tmpStr
        txtInputDir.ToolTipText = tmpStr
    End If
    tmpStr = GetIniSetting(COUTPUT_DIR)
    If tmpStr <> "" Then
        If Right(tmpStr, 1) = ":" And Len(tmpStr) = 2 Then
            tmpStr = tmpStr & "\"
        End If
        txtOutputDir.Text = tmpStr
        txtOutputDir.ToolTipText = tmpStr
    End If
    tmpStr = GetIniSetting(CRESULTS_FILE)
    If tmpStr <> "" Then txtResultsName.Text = tmpStr
    
    rdbResultsPrompt.value = False
    rdbResultsOverwrite.value = False
    rdbResultsAppend.value = False
    Dim resultsSetting As String
    resultsSetting = GetIniSetting(CRESULTS_EXIST)
    If resultsSetting = CPROMPT_FILE Then
        rdbResultsPrompt.value = True
    ElseIf resultsSetting = CAPPEND_FILE Then
        rdbResultsAppend.value = True
    Else
        rdbResultsOverwrite.value = True
    End If
    
    chkWordDoc.value = IIf(GetIniSetting(CTYPE_WORDDOC) = CStr(True), vbChecked, 0)
    chkWordTemplate.value = IIf(GetIniSetting(CTYPE_WORDDOT) = CStr(True), vbChecked, 0)
    chkExcelDoc.value = IIf(GetIniSetting(CTYPE_EXCELDOC) = CStr(True), vbChecked, 0)
    chkExcelTemplate.value = IIf(GetIniSetting(CTYPE_EXCELDOT) = CStr(True), vbChecked, 0)
    chkPPDoc.value = IIf(GetIniSetting(CTYPE_PPDOC) = CStr(True), vbChecked, 0)
    chkPPTemplate.value = IIf(GetIniSetting(CTYPE_PPDOT) = CStr(True), vbChecked, 0)
    chkIncludeSubdirs.value = IIf(GetIniSetting(CINCLUDE_SUBDIRS) = CStr(True), vbChecked, 0)
    mDebugLevel = IIf(GetIniSetting(CDEBUG_LEVEL) = "", CDEBUG_LEVEL_DEFAULT, GetIniSetting(CDEBUG_LEVEL))
    chkIgnoreOld.value = IIf(GetIniSetting(CIGNORE_OLD_DOCS) = CStr(True), vbChecked, 0)
    
    mIssueLimit = IIf(GetIniSetting(CISSUE_LIMIT) = "", CISSUE_LIMIT_DAW, GetIniSetting(CISSUE_LIMIT))
    If (mIssueLimit <= 3) Then
        cbIgnoreOld.ListIndex = 0
    ElseIf (mIssueLimit <= 6) Then
        cbIgnoreOld.ListIndex = 1
    Else
        cbIgnoreOld.ListIndex = 2
    End If

    'Always ensure at least one doc type is selected on startup
    If (chkWordDoc.value <> vbChecked) And _
        (chkWordTemplate.value <> vbChecked) And _
        (chkExcelDoc.value <> vbChecked) And _
        (chkExcelTemplate.value <> vbChecked) And _
        (chkPPDoc.value <> vbChecked) And _
        (chkPPTemplate.value <> vbChecked) Then
        
        chkWordDoc.value = vbChecked
    End If

FinalExit:
    Set fso = Nothing
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Private Sub SetStep(nStep As Integer, nDirection As Integer)
    On Error GoTo HandleError
    Const driveTypeIsCDROM = 4
    Dim val As String
    Dim fso As Scripting.FileSystemObject
    Set fso = New Scripting.FileSystemObject
    Dim drive


    mbDocCountCurrent = False
    
    Select Case nStep
        Case STEP_INTRO
            'MsgBox "Enter Intro"
        Case STEP_1
            'Leave Introduction
            'Workaround - resource bug for SubDir checkbox, have to set it explicitly
            chkIncludeSubdirs.Caption = mChbSubdirs
        Case STEP_2
            'Leave Documents
            
            Set drive = fso.GetDrive(fso.GetDriveName(txtInputDir.Text))
            If drive.DriveType = driveTypeIsCDROM Then
                If Not drive.IsReady Then
                    MsgBox GetResString(ERR_CDROM_NOT_READY), vbCritical
                    Exit Sub
                End If
            End If
                    
            If txtInputDir.Text = "" Or Not fso.FolderExists(txtInputDir.Text) Then ' fso.FolderExists() has replaced dir()
                MsgBox ReplaceTopicTokens(GetResString(ERR_NO_DOC_DIR), _
                    CR_STR, Chr(13)), vbCritical
                Exit Sub
            End If
            
            If Not CheckUserChosenDocsToAnalyze Then
                MsgBox GetResString(ERR_NO_DOC_TYPES), vbCritical
                Exit Sub
            End If
            'Expand directory name only without path to full path
            txtInputDir.Text = fso.GetAbsolutePathName(txtInputDir.Text)
            
            If txtOutputDir.Text = "" Then
                txtOutputDir.Text = txtInputDir.Text
            End If
            
            mbFinishOK = False
            
            'Workaround - label resource bug for Steps, have to set it explicitly
            Label12(0).Caption = mLblSteps
            Label12(5).Caption = mLblSteps
        Case STEP_FINISH
            'Leave Results
            If Not CheckResultsDir(getOutputDir) Then
                Exit Sub
            End If
            
            'Expand directory name only without path to full path
            txtOutputDir.Text = fso.GetAbsolutePathName(txtOutputDir)
            
            'Check Results file is there and has a valid extension
            If fso.GetBaseName(txtResultsName.Text) = "" Then
                txtResultsName.Text = GetResString(SETUP_ANALYSIS_XLS_ID)
            End If
            txtResultsName.Text = fso.GetBaseName(txtResultsName.Text) & CRESULTS_FILE_EXTENSION
            
            Screen.MousePointer = vbHourglass
            DeleteFile mLogFilePath
            Set mDocFiles = Nothing
            If Not CheckNumberDocsToAnalyze Then
                Screen.MousePointer = vbDefault
                Exit Sub
            End If
            
            Screen.MousePointer = vbDefault

            btnRunAnalysis.Enabled = True
            
            If GetNumberOfDocsToAnalyze = 0 Then
                btnRunAnalysis.Enabled = False
            End If
            
            'Backup configuration
            If Not AttemptToCopyFile(mIniFilePath, mIniFilePath & CCONFIG_BACKUP_EXT) Then
                Exit Sub
            End If
            
            'Save current Wizard Settings
            WriteWizardSettingsToLog mIniFilePath
            
            'If results file already exists, enable View and Prepare
            If fso.FileExists(getOutputDir & "\" & txtResultsName.Text) Then
                btnViewResults.Enabled = True
                btnPrepare.Enabled = True
            End If

            mbFinishOK = True
    End Select
    
    'move to new step
    fraStep(mnCurStep).Enabled = False
    fraStep(nStep).Left = 0
    If nStep <> mnCurStep Then
        fraStep(mnCurStep).Left = -10000
        fraStep(mnCurStep).Enabled = False
    End If
    fraStep(nStep).Enabled = True
  
    SetNavBtns nStep
    Exit Sub
    
FinalExit:
    Set fso = Nothing
    Set drive = Nothing
    Exit Sub
    
HandleError:
    Screen.MousePointer = vbDefault
    WriteDebug "Document Analysis: SetStep() " & Err.Number & " " & Err.Description & " " & Err.Source
    
    Resume FinalExit
End Sub

Function CheckResultsDir(resultsDir As String) As Boolean
    On Error GoTo HandleError
    Dim fso As Scripting.FileSystemObject
    Set fso = New Scripting.FileSystemObject
    Const driveTypeIsCDROM = 4
    Const readOnlyFolderRemainder = 1
    Dim drive
    CheckResultsDir = False
    
    If resultsDir = "" Then
        MsgBox ReplaceTopicTokens(GetResString(ERR_NO_RESULTS_DIRECTORY), _
            CR_STR, Chr(13)), vbCritical
        CheckResultsDir = False
        Exit Function
    End If
    
    Set drive = fso.GetDrive(fso.GetDriveName(resultsDir))
    If drive.DriveType = driveTypeIsCDROM Then 'If CD-ROM Drive Then
        Dim Msg1 As String
        Msg1 = ReplaceTopic2Tokens(GetResString(ERR_CREATE_FILE), _
                    TOPIC_STR, txtResultsName.Text, CR_STR, Chr(13))
        MsgBox Msg1, vbCritical
        CheckResultsDir = False
        Exit Function
    End If
    
            
    If Not fso.FolderExists(resultsDir) Then
        Dim Msg, Style, response

        Msg = ReplaceTopicTokens(GetResString(ERR_NO_RES_DIR), CR_STR, Chr(13))
        Style = vbYesNo + vbQuestion + vbDefaultButton1   ' Define buttons.
        
        response = MsgBox(Msg, Style)
        If response = vbYes Then   ' User chose Yes.
            If Not CreateDir(getOutputDir) Then
                CheckResultsDir = False
                Exit Function
            End If
        Else   ' User chose No.
            CheckResultsDir = False
            Exit Function
        End If
    End If

    Dim testFile As String
    testFile = resultsDir & "\" & fso.GetTempName
    Do While fso.FileExists(testFile)
        testFile = resultsDir & "\" & fso.GetTempName
    Loop
    
    On Error GoTo HandleReadOnly
    Dim aText As TextStream
    Set aText = fso.CreateTextFile(testFile, False, False)
    aText.WriteLine ("Dies ist ein Test.")
    aText.Close
    fso.DeleteFile (testFile)
    
'   GetAttr doesn't work reliable ( returns read only for 'my Documents' and rw for read only network folder
'    If ((GetAttr(resultsDir) Mod 2) = readOnlyFolderRemainder) Then 'If the attribute is odd then the folder is read-only
'        MsgBox GetResString(ERR_NO_WRITE_TO_READ_ONLY_FOLDER), vbCritical
'        CheckResultsDir = False
'        Exit Function
'    End If
    
    CheckResultsDir = True
    
    Exit Function
HandleError:
    WriteDebug "Document Analysis: CheckResultsDir() " & Err.Number & " " & Err.Description & " " & Err.Source
    CheckResultsDir = False
    Exit Function
HandleReadOnly:
    Dim str As String
    str = ReplaceTopic2Tokens(GetResString(ERR_CREATE_FILE), _
                    TOPIC_STR, txtResultsName.Text, CR_STR, Chr(13))
    MsgBox str, vbCritical
    CheckResultsDir = False
    Exit Function
End Function

Function CheckUserChosenDocsToAnalyze() As Boolean
    CheckUserChosenDocsToAnalyze = Not ((chkWordDoc.value <> vbChecked) And (chkWordTemplate.value <> vbChecked) And _
                (chkExcelDoc.value <> vbChecked) And (chkExcelTemplate.value <> vbChecked) And _
                (chkPPDoc.value <> vbChecked) And (chkPPTemplate.value <> vbChecked))
End Function
           
Function AttemptToCopyFile(Source As String, dest As String) As Boolean
    On Error GoTo HandleErrors
    Dim fso As Scripting.FileSystemObject
    Set fso = New Scripting.FileSystemObject
    
    If fso.FileExists(Source) Then
        fso.CopyFile Source, dest
    End If
    
    'True if no source or copy succeded
    AttemptToCopyFile = True
    
FinalExit:
    Set fso = Nothing
    Exit Function
    
HandleErrors:
    AttemptToCopyFile = False
    Dim str As String
    str = ReplaceTopic2Tokens(GetResString(ERR_CREATE_FILE), _
                    TOPIC_STR, mIniFilePath & CCONFIG_BACKUP_EXT, CR_STR, Chr(13))
    Resume FinalExit

End Function

Function CreateDir(dir As String) As Boolean
    On Error GoTo HandleErrors
    Dim fso As Scripting.FileSystemObject
    Set fso = New Scripting.FileSystemObject
    
    fso.CreateFolder (dir)
    
    CreateDir = True
    
FinalExit:
    Set fso = Nothing
    Exit Function
    
HandleErrors:
    Dim str As String
    str = ReplaceTopic2Tokens(GetResString(ERR_CREATE_DIR), _
                    TOPIC_STR, dir, CR_STR, Chr(13))
    Select Case Err.Number
        Case 76
            WriteDebug str
            MsgBox str, vbCritical
            CreateDir = False
        Case 58
            'Don't care if it exists already
            CreateDir = True
        Case Else
            WriteDebug str
            MsgBox str, vbCritical
            CreateDir = False
    End Select
    Resume FinalExit

End Function
Private Sub SetNavBtns(nStep As Integer)
    mnCurStep = nStep
    
    If mnCurStep = 0 Then
        cmdNav(BTN_BACK).Enabled = False
        cmdNav(BTN_NEXT).Enabled = True
    ElseIf mnCurStep = NUM_STEPS - 1 Then
        cmdNav(BTN_NEXT).Enabled = False
        cmdNav(BTN_BACK).Enabled = True
    Else
        cmdNav(BTN_BACK).Enabled = True
        cmdNav(BTN_NEXT).Enabled = True
    End If
    
    If mbFinishOK Then
        cmdNav(BTN_FINISH).Enabled = True
    Else
        cmdNav(BTN_FINISH).Enabled = False
    End If
End Sub
Function CheckForSupportedApp(app As String, lowerVerLimit As Long) As Boolean
    Dim appRegStr As String
    Dim appVer As Long
    appRegStr = GetRegistryInfo(HKEY_CLASSES_ROOT, app & ".Application\CurVer", "")
    appVer = val(Right(appRegStr, Len(appRegStr) - Len(app & ".Application.")))
    If appVer >= lowerVerLimit Then
        CheckForSupportedApp = True
    Else
        CheckForSupportedApp = False
    End If
End Function
Function GetAppVersion(app As String) As Long
    Dim appRegStr As String
    Dim appVer As Long
    appRegStr = GetRegistryInfo(HKEY_CLASSES_ROOT, app & ".Application\CurVer", "")
    GetAppVersion = val(Right(appRegStr, Len(appRegStr) - Len(app & ".Application.")))
End Function
Function GetInstalledApp(app As String) As String
    GetInstalledApp = GetRegistryInfo(HKEY_CLASSES_ROOT, app & ".Application\CurVer", "")
End Function

Sub WriteInfoToApplicationLog(wordAppStr As String, excelAppStr As String, ppAppStr As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteInfoToApplicationLog"
    
    Dim userLCID As Long
    userLCID = GetUserDefaultLCID()
    Dim sysLCID As Long
    sysLCID = GetSystemDefaultLCID()
    
    WriteToLog CWORD_VERSION, IIf(wordAppStr <> "", wordAppStr, CNOT_INSTALLED)
    WriteToLog CEXCEL_VERSION, IIf(excelAppStr <> "", excelAppStr, CNOT_INSTALLED)
    WriteToLog CPOWERPOINT_VERSION, IIf(ppAppStr <> "", ppAppStr, CNOT_INSTALLED)
    
    WriteToLog CUSER_LOCALE_INFO, _
        "langid: " & GetUserLocaleInfo(userLCID, LOCALE_ILANGUAGE) & ": " & _
        GetUserLocaleInfo(userLCID, LOCALE_SENGLANGUAGE) & _
        "-" & GetUserLocaleInfo(userLCID, LOCALE_SENGCOUNTRY) & _
        " abrv: " & GetUserLocaleInfo(userLCID, LOCALE_SISO639LANGNAME) & _
        "-" & GetUserLocaleInfo(userLCID, LOCALE_SISO3166CTRYNAME) & _
        " sdate: " & GetUserLocaleInfo(userLCID, LOCALE_SSHORTDATE)

    WriteToLog CSYS_LOCALE_INFO, _
        "langid: " & GetUserLocaleInfo(sysLCID, LOCALE_ILANGUAGE) & ": " & _
        GetUserLocaleInfo(sysLCID, LOCALE_SENGLANGUAGE) & _
        "-" & GetUserLocaleInfo(sysLCID, LOCALE_SENGCOUNTRY) & _
        " abrv: " & GetUserLocaleInfo(sysLCID, LOCALE_SISO639LANGNAME) & _
        "-" & GetUserLocaleInfo(sysLCID, LOCALE_SISO3166CTRYNAME) & _
        " sdate: " & GetUserLocaleInfo(userLCID, LOCALE_SSHORTDATE)
        
    Dim myWinVer As RGB_WINVER
    GetWinVersion myWinVer
    WriteToLog CWINVERSION, myWinVer.VersionName & " " & myWinVer.VersionNo & _
        " " & myWinVer.ServicePack & _
        " build " & myWinVer.BuildNo
    WriteToLog CNUMBER_TOTAL_DOCS, CStr(mTotalDocCount)
    WriteToLog CNUMBER_DOCS_DOC, CStr(mWordDocCount)
    WriteToLog CNUMBER_TEMPLATES_DOT, CStr(mWordTemplateCount)
    WriteToLog CNUMBER_DOCS_XLS, CStr(mExcelDocCount)
    WriteToLog CNUMBER_TEMPLATES_XLT, CStr(mExcelTemplateCount)
    WriteToLog CNUMBER_DOCS_PPT, CStr(mPPDocCount)
    WriteToLog CNUMBER_TEMPLATES_POT, CStr(mPPTemplateCount)
FinalExit:
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Function CheckTemplatePath(sMigrationResultsTemplatePath As String, fso As FileSystemObject) As Boolean
    If Not fso.FileExists(sMigrationResultsTemplatePath) Then
        Dim str As String
        str = ReplaceTopic2Tokens(GetResString(ERR_MISSING_RESULTS_TEMPLATE), _
                    TOPIC_STR, sMigrationResultsTemplatePath, CR_STR, Chr(13))
        WriteDebug str
        MsgBox str, vbCritical
        CheckTemplatePath = False
    Else
        CheckTemplatePath = True
    End If
End Function

Function RunAnalysis(bDoPrepare) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "RunAnalysis"
    Dim tstart As Single   'timer var for this routine only
    Dim tend As Single     'timer var for this routine only
    Dim fso As New FileSystemObject
    Dim wordAppStr As String
    Dim excelAppStr As String
    Dim ppAppStr As String
    Dim sMigrationResultsTemplatePath As String
    Dim startDate As Variant
    Dim bSuccess
    
    bSuccess = True
    startDate = Now
    tstart = GetTickCount()
    
    app.OleRequestPendingMsgText = GetResString(RUNBTN_RUNNING_ID)
    app.OleRequestPendingMsgTitle = frmWizard.Caption
        
    wordAppStr = GetInstalledApp(CAPPNAME_WORD)
    excelAppStr = GetInstalledApp(CAPPNAME_EXCEL)
    ppAppStr = GetInstalledApp(CAPPNAME_POWERPOINT)
    'Write locale, version info and settings to the Application log
    WriteInfoToApplicationLog wordAppStr, excelAppStr, ppAppStr
        
    'Check for template
    sMigrationResultsTemplatePath = fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CRESULTS_TEMPLATE_FILE)
    If Not CheckTemplatePath(sMigrationResultsTemplatePath, fso) Then
        bSuccess = False
        GoTo FinalExit
    End If
    
    'Check for installed Apps
    If Not CheckInstalledApps(wordAppStr, excelAppStr, ppAppStr) Then
        bSuccess = False
        GoTo FinalExit
    End If
    
    If bDoPrepare Then
        'Show MsgBox ( to give apps some time to quit )
        Dim strMsgBox As String
        Dim response As Integer
        
        strMsgBox = ReplaceTopic2Tokens(GetResString(RID_STR_ENG_OTHER_PREPARE_PROMPT_PREP_ID), _
                        TOPIC_STR, getOutputDir & "\" & txtResultsName.Text, TOPIC2_STR, getOutputDir)
        strMsgBox = ReplaceTopicTokens(strMsgBox, CR_STR, Chr(13))
        response = MsgBox(strMsgBox, Buttons:=vbOKCancel + vbInformation)

        If response <> vbOK Then
            bSuccess = False
            GoTo FinalExit
        End If
    End If
        
    'Write Wizard Setting to Application log
    WriteWizardSettingsToLog mLogFilePath

    'Write to Analysis ini file - used by driver docs
    WriteCommonParamsToLog sMigrationResultsTemplatePath, mLogFilePath, mIniFilePath, fso

    Screen.MousePointer = vbHourglass
    ' Doc Counts are setup by CheckNumberDocsToAnalyze() when user moves to Analysis Panel
    ' Takes account of user Options selected and inspects source directory
    Dim analysisAborted As Boolean
    analysisAborted = False
    
    SetupInputVariables mLogFilePath, fso

    Load ShowProgress
    Call ShowProgress.SP_Init(mDocFiles.WordFiles.count + _
                              mDocFiles.ExcelFiles.count + _
                              mDocFiles.PowerPointFiles.count)

    Dim myOffset As Long
    myOffset = 0
    If (mDocFiles.WordFiles.count > 0) Then
        bSuccess = AnalyseList(mDocFiles.WordFiles, "word", mIniFilePath, myOffset, analysisAborted)
        'bSuccess = RunWordAnalysis(sMigrationResultsTemplatePath, mLogFilePath, fso)
    End If

    myOffset = mDocFiles.WordFiles.count
    If ((mDocFiles.ExcelFiles.count > 0) And (Not analysisAborted)) Then
        bSuccess = bSuccess And _
                   AnalyseList(mDocFiles.ExcelFiles, "excel", mIniFilePath, myOffset, analysisAborted)
        'bSuccess = RunExcelAnalysis(sMigrationResultsTemplatePath, mLogFilePath, fso)
    End If

    myOffset = myOffset + mDocFiles.ExcelFiles.count
    If ((mDocFiles.PowerPointFiles.count > 0) And (Not analysisAborted)) Then
        bSuccess = bSuccess And _
                   AnalyseList(mDocFiles.PowerPointFiles, "pp", mIniFilePath, myOffset, analysisAborted)
        'bSuccess = RunPPAnalysis(sMigrationResultsTemplatePath, mLogFilePath, fso)
    End If

    SetupInputVariables mLogFilePath, fso
    
    tend = GetTickCount()
    WriteToLog CELAPSED_TIME, (FormatNumber((tend - tstart) / 1000, 0) & " seconds: ") & _
    (FormatNumber((tend - tstart), 0) & " miliseconds")

FinalExit:
    Unload ShowProgress
    Screen.MousePointer = vbDefault
    WriteToLog CSTART_TIME, CDate(startDate)
    WriteToLog CEND_TIME, Now
    Set fso = Nothing
    
    RunAnalysis = bSuccess
    Exit Function
    
HandleErrors:
    bSuccess = False
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function CheckInstalledApps(wordAppStr As String, excelAppStr As String, ppAppStr As String) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    Dim str As String
    currentFunctionName = "CheckInstalledApps"
    
    Dim missingInstalledApps As String
    Dim unsupportedApps As String
    Dim runningApps As String
    Dim bSuccess As Boolean
    
    bSuccess = False
    
    If mWordDocCount > 0 Or mWordTemplateCount > 0 Then
        If wordAppStr = "" Then 'Word not installed
            missingInstalledApps = CAPPNAME_WORD
        ElseIf Not CheckForSupportedApp(CAPPNAME_WORD, CSUPPORTED_VERSION) Then
            unsupportedApps = CAPPNAME_WORD
        ElseIf IsOfficeAppRunning(CAPPNAME_WORD) Then
            runningApps = CAPPNAME_WORD
        End If
    End If
    
    If excelAppStr = "" Then
        If missingInstalledApps <> "" Then missingInstalledApps = missingInstalledApps & ", "
        missingInstalledApps = missingInstalledApps & CAPPNAME_EXCEL
    ElseIf Not CheckForSupportedApp(CAPPNAME_EXCEL, CSUPPORTED_VERSION) Then
        If unsupportedApps <> "" Then unsupportedApps = unsupportedApps & ", "
        unsupportedApps = unsupportedApps & CAPPNAME_EXCEL
    ElseIf IsOfficeAppRunning(CAPPNAME_EXCEL) Then
        If runningApps <> "" Then runningApps = runningApps & ", "
        runningApps = runningApps & CAPPNAME_EXCEL
    End If
    
    If mPPDocCount > 0 Or mPPTemplateCount > 0 Then
        If ppAppStr = "" Then 'PP not installed
            If missingInstalledApps <> "" Then missingInstalledApps = missingInstalledApps & ", "
            missingInstalledApps = missingInstalledApps & CAPPNAME_POWERPOINT
        ElseIf Not CheckForSupportedApp(CAPPNAME_POWERPOINT, CSUPPORTED_VERSION) Then
            If unsupportedApps <> "" Then unsupportedApps = unsupportedApps & ", "
            unsupportedApps = unsupportedApps & CAPPNAME_POWERPOINT
        ElseIf IsOfficeAppRunning(CAPPNAME_POWERPOINT) Then
            If runningApps <> "" Then runningApps = runningApps & ", "
            runningApps = runningApps & CAPPNAME_POWERPOINT
        End If
    End If

    If missingInstalledApps <> "" Then
        str = ReplaceTopic2Tokens(GetResString(ERR_NOT_INSTALLED), _
                    TOPIC_STR, missingInstalledApps, CR_STR, Chr(13))
        WriteDebug str
        MsgBox str, vbCritical
        GoTo FinalExit
    End If

    If unsupportedApps <> "" Then
        str = ReplaceTopic2Tokens(GetResString(ERR_SUPPORTED_VERSION), _
                    TOPIC_STR, unsupportedApps, CR_STR, Chr(13))
        WriteDebug str
        MsgBox str, vbCritical
        GoTo FinalExit
    End If
    
    If runningApps <> "" Then
        str = ReplaceTopic2Tokens(GetResString(ERR_APPLICATION_IN_USE), _
                    TOPIC_STR, runningApps, CR_STR, Chr(13))
        WriteDebug str
        MsgBox str, vbCritical
        GoTo FinalExit
    End If
    
    'Check for Excel automation server
    If CheckForExcel Then
        str = ReplaceTopicTokens(GetResString(ERR_EXCEL_OPEN), _
                    CR_STR, Chr(13))
        WriteDebug str
        MsgBox str, vbCritical
        bSuccess = False
        GoTo FinalExit
    End If

    bSuccess = True
    
FinalExit:
    CheckInstalledApps = bSuccess
    Exit Function
    
HandleErrors:
    bSuccess = False
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function RunPPAnalysis(resultsTemplate As String, logFile As String, fsObject As FileSystemObject) As Boolean
'DV: do we need this? get some error handling ideas here
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "RunPPAnalysis"
    Const APP_PP = "PowerPoint"
    Dim str As String
    Dim bSuccess
    bSuccess = False
    
    If (chkPPDoc.value <> vbChecked) And (chkPPTemplate.value <> vbChecked) Then
        RunPPAnalysis = True
        Exit Function
    End If
    
    Dim sPPDriverDocPath As String

    sPPDriverDocPath = fsObject.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CPP_DRIVER_FILE)
    If Not fsObject.FileExists(sPPDriverDocPath) Then
        str = ReplaceTopic2Tokens(GetResString(ERR_MISSING_PP_DRIVER), _
                    TOPIC_STR, sPPDriverDocPath, CR_STR, Chr(13))
        WriteDebug str
        MsgBox str, vbCritical
        bSuccess = False
        GoTo FinalExit
    End If

    Dim pp As PowerPoint.application
    Dim po As Object
    Dim aPres As PowerPoint.Presentation
    Dim RegValue As Long
    Set po = GetObject(sPPDriverDocPath)
    Set pp = po.application
    
    If val(pp.Version) < CSUPPORTED_VERSION Then
        str = ReplaceTopic2Tokens(GetResString(ERR_SUPPORTED_VERSION), _
                    TOPIC_STR, pp.Version, CR_STR, Chr(13))
        WriteDebug str
        MsgBox str, vbCritical
        bSuccess = False
        GoTo FinalExit
    End If
    
    If Not CheckForAccesToPPVBProject(pp, aPres) Then
        RegValue = -1
        If Not GiveAccessToMacroProject(APP_PP, pp.Version, RegValue) Then
            Dim Style, response
            str = ReplaceTopic2Tokens(GetResString(ERR_NO_ACCESS_TO_VBPROJECT), _
                TOPIC_STR, CAPPNAME_POWERPOINT, CR_STR, Chr(13))
            WriteDebug str
            Style = vbYesNo + vbQuestion + vbDefaultButton1
        
            response = MsgBox(str, Style)
            If response <> vbYes Then
                bSuccess = False
                GoTo FinalExit
            End If
        End If
    End If
    
    Set aPres = pp.Presentations(1)
    Dim ppSlideHidden As PowerPoint.Slide
    Set ppSlideHidden = aPres.Slides(2)
    
    'Setup Input Variables
    'SetupInputVariables resultsTemplate, logFile, fsObject, CAPPNAME_POWERPOINT
        
    'Run PowerPoint Analysis
    pp.Run (fsObject.GetFileName(CBASE_RESOURCE_DIR & "\" & CPP_DRIVER_FILE) & "!AnalysisDriver.AnalyseDirectory")
    
    bSuccess = True
    
FinalExit:
    'Cannot seem to close it down from VB
    'Workaround is to close it in macro
    '
    'If Not aPres Is Nothing Then
    '    aPres.Saved = msoTrue
    'End If
    'If Not pp Is Nothing Then pp.Quit
    
    'Swallow error as we are closing down PP from macro
    'Does not seem to be possible to close it down from VB
    On Error Resume Next
    If RegValue <> -1 Then
        SetDefaultRegValue APP_PP, pp.Version, RegValue
    End If
    If RegValue = 0 Then
        DeleteRegValue APP_PP, pp.Version
    End If

    If Not pp Is Nothing Then
        pp.Run (fsObject.GetFileName(CBASE_RESOURCE_DIR & "\" & CPP_DRIVER_FILE) & "!ApplicationSpecific.QuitPowerPoint")
    End If
    

    Set aPres = Nothing
    Set pp = Nothing
    Set po = Nothing

    RunPPAnalysis = bSuccess
    Exit Function

HandleErrors:
    bSuccess = False
    Set pp = Nothing
    Dim failedDoc As String
    
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    
    failedDoc = GetDebug(CAPPNAME_POWERPOINT, CANALYZING)
    If failedDoc = "" Or failedDoc = CSTR_ANALYSIS_LOG_DONE Then
        str = ReplaceTopic2Tokens(GetResString(ERR_AUTOMATION_FAILURE), _
                    TOPIC_STR, CAPPNAME_POWERPOINT, CR_STR, Chr(13))
    Else
        str = ReplaceTopic2Tokens(GetResString(ERR_PP_DRIVER_CRASH), _
                    TOPIC_STR, failedDoc, CR_STR, Chr(13))
    End If
    
    WriteDebug str
    MsgBox str, vbCritical

    Resume FinalExit
End Function
    
Sub SetupInputVariables(logFile As String, fso As FileSystemObject)
    Dim bNewResultsFile As Boolean
    
    bNewResultsFile = CheckCreateNewResultsFile(fso)
    
    WriteToLog CNEW_RESULTS_FILE, IIf(bNewResultsFile, "True", "False"), mIniFilePath
    WriteToLog CNEW_RESULTS_FILE, IIf(bNewResultsFile, "True", "False"), logFile
End Sub



Function RunExcelAnalysis(resultsTemplate As String, logFile As String, fsObject As FileSystemObject) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "RunExcelAnalysis"
    Const APP_EXCEL = "Excel"
    Dim str As String
    Dim bSuccess
    bSuccess = False
    
    If (chkExcelDoc.value <> vbChecked) And (chkExcelTemplate.value <> vbChecked) Then
        RunExcelAnalysis = True
        Exit Function
    End If
    
    Dim xl As Excel.application
    Dim aWb As Excel.Workbook
    Dim sExcelDriverDocPath As String
    Dim RegValue As Long

    sExcelDriverDocPath = fsObject.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CEXCEL_DRIVER_FILE)
    If Not fsObject.FileExists(sExcelDriverDocPath) Then
        str = ReplaceTopic2Tokens(GetResString(ERR_MISSING_EXCEL_DRIVER), _
                    TOPIC_STR, sExcelDriverDocPath, CR_STR, Chr(13))
        WriteDebug str
        MsgBox str, vbCritical
        bSuccess = False
        GoTo FinalExit
    End If
    
    Set xl = GetExcelInstance
    If val(xl.Version) < CSUPPORTED_VERSION Then
        str = ReplaceTopic2Tokens(GetResString(ERR_SUPPORTED_VERSION), _
                    TOPIC_STR, xl.Version, CR_STR, Chr(13))
        WriteDebug str
        MsgBox str, vbCritical
        bSuccess = False
        GoTo FinalExit
    End If

    If Not CheckForAccesToExcelVBProject(xl) Then
        RegValue = -1
        If Not GiveAccessToMacroProject(APP_EXCEL, xl.Version, RegValue) Then
            Dim Style, response
            str = ReplaceTopic2Tokens(GetResString(ERR_NO_ACCESS_TO_VBPROJECT), _
                TOPIC_STR, CAPPNAME_EXCEL, CR_STR, Chr(13))
            WriteDebug str
            Style = vbYesNo + vbQuestion + vbDefaultButton1
        
            response = MsgBox(str, Style)
            If response <> vbYes Then
                bSuccess = False
                GoTo FinalExit
            End If
        End If
    End If
    
    Set aWb = xl.Workbooks.Open(fileName:=sExcelDriverDocPath)
    'Setup Input Variables
    'SetupInputVariables resultsTemplate, logFile, fsObject, CAPPNAME_EXCEL
    
    'Run Excel Analysis
    xl.Run ("AnalysisTool.AnalysisDriver.AnalyseDirectory")
    
    bSuccess = True
FinalExit:
    If RegValue <> -1 Then
        SetDefaultRegValue APP_EXCEL, xl.Version, RegValue
    End If
    If RegValue = 0 Then
        DeleteRegValue APP_EXCEL, xl.Version
    End If
    
    If Not aWb Is Nothing Then
        If xl.Workbooks.count = 1 Then
            xl.Visible = False
        End If
        aWb.Close (False)
    End If
    Set aWb = Nothing

    If Not xl Is Nothing Then
        If xl.Workbooks.count = 0 Then
            xl.Quit
        End If
    End If

    Set xl = Nothing

    RunExcelAnalysis = bSuccess
    Exit Function

HandleErrors:
    bSuccess = False
    Set aWb = Nothing
    Set xl = Nothing
    Dim failedDoc As String
    
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    
    failedDoc = GetDebug(CAPPNAME_EXCEL, CANALYZING)
    If failedDoc = "" Or failedDoc = CSTR_ANALYSIS_LOG_DONE Then
        str = ReplaceTopic2Tokens(GetResString(ERR_AUTOMATION_FAILURE), _
                    TOPIC_STR, CAPPNAME_EXCEL, CR_STR, Chr(13))
    Else
        str = ReplaceTopic2Tokens(GetResString(ERR_EXCEL_DRIVER_CRASH), _
                    TOPIC_STR, failedDoc, CR_STR, Chr(13))
    End If
    
    WriteDebug str
    MsgBox str, vbCritical

    On Error Resume Next
    Resume FinalExit
End Function

Sub WriteWizardSettingsToLog(path As String)
    '### DO NOT USE Boolean True/ False it is loaclised by the OS - use "True"/ "False"
    WriteToLog CINPUT_DIR, getInputDir, path
    WriteToLog CINCLUDE_SUBDIRS, IIf(chkIncludeSubdirs.value, "True", "False"), path
    WriteToLog COUTPUT_DIR, getOutputDir, path
    WriteToLog CRESULTS_FILE, txtResultsName.Text, path
    
    WriteToLog CTYPE_WORDDOC, IIf(chkWordDoc.value, "True", "False"), path
    WriteToLog CTYPE_WORDDOT, IIf(chkWordTemplate.value, "True", "False"), path
    WriteToLog CTYPE_EXCELDOC, IIf(chkExcelDoc.value, "True", "False"), path
    WriteToLog CTYPE_EXCELDOT, IIf(chkExcelTemplate.value, "True", "False"), path
    WriteToLog CTYPE_PPDOC, IIf(chkPPDoc.value, "True", "False"), path
    WriteToLog CTYPE_PPDOT, IIf(chkPPTemplate.value, "True", "False"), path

    Dim resultsSetting As String
    If rdbResultsPrompt.value Then
        resultsSetting = CPROMPT_FILE
    ElseIf rdbResultsAppend.value Then
        resultsSetting = CAPPEND_FILE
    Else
        resultsSetting = COVERWRITE_FILE
    End If
    WriteToLog CRESULTS_EXIST, resultsSetting, path

    WriteToLog CIGNORE_OLD_DOCS, IIf(chkIgnoreOld.value, "True", "False"), path
    WriteToLog CISSUE_LIMIT, CStr(mIssueLimit), path

    'WriteToLog CVERSION, Version, path
End Sub

Sub WriteCommonParamsToLog(resultsTemplate As String, logFile As String, path As String, fso As Scripting.FileSystemObject)
    WriteToLog CLOG_FILE, logFile, path
    WriteToLog CRESULTS_TEMPLATE, resultsTemplate, path
    WriteToLog CDEBUG_LEVEL, CLng(mDebugLevel), path
    WriteToLog CDOPREPARE, IIf(mbDoPrepare, "True", "False"), path
    WriteToLog CTITLE, frmWizard.Caption, path
    WriteToLog CLAST_CHECKPOINT, ""
    WriteToLog CNEXT_FILE, ""
    WriteToLog C_ABORT_ANALYSIS, ""
End Sub

Function GetNumberOfDocsToAnalyze() As Long
    Dim count As Long
    
    count = 0
        
    If CheckWordDocsToAnalyze Then
        count = mWordDocCount + mWordTemplateCount
    End If
    If CheckExcelDocsToAnalyze Then
        count = count + mExcelDocCount + mExcelTemplateCount
    End If
    If CheckPPDocsToAnalyze Then
        count = count + mPPDocCount + mPPTemplateCount
    End If
    
    GetNumberOfDocsToAnalyze = count
End Function

Function CheckWordDocsToAnalyze() As Boolean
    
    CheckWordDocsToAnalyze = mbDocCountCurrent And (chkWordDoc.value = vbChecked And mWordDocCount > 0) Or _
        (chkWordTemplate.value = vbChecked And mWordTemplateCount > 0)
End Function

Function CheckExcelDocsToAnalyze() As Boolean
    CheckExcelDocsToAnalyze = mbDocCountCurrent And (chkExcelDoc.value = vbChecked And mExcelDocCount > 0) Or _
        (chkExcelTemplate.value = vbChecked And mExcelTemplateCount > 0)
End Function

Function CheckPPDocsToAnalyze() As Boolean
    CheckPPDocsToAnalyze = mbDocCountCurrent And (chkPPDoc.value = vbChecked And mPPDocCount > 0) Or _
        (chkPPTemplate.value = vbChecked And mPPTemplateCount > 0)
End Function

Function CheckNumberDocsToAnalyze() As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "CheckNumberDocsToAnalyze"
    
    Set mDocFiles = New CollectedFiles
    
    Dim docSearchTypes As Collection
    Set docSearchTypes = New Collection
    
    mbDocCountCurrent = False
        
    SetupDocSearchTypes docSearchTypes
    
    If (cbIgnoreOld.ListIndex = 0) Then
        mIssueLimit = 3
    ElseIf (cbIgnoreOld.ListIndex = 1) Then
        mIssueLimit = 6
    Else
        mIssueLimit = 12
    End If

    If Not mDocFiles.Search(rootDir:=getInputDir, FileSpecs:=docSearchTypes, _
            IncludeSubdirs:=IIf(chkIncludeSubdirs.value, mbTrue, mbFalse), _
            ignoreOld:=IIf(chkIgnoreOld.value, mbTrue, mbFalse), Months:=mIssueLimit) Then
        CheckNumberDocsToAnalyze = False
        GoTo FinalExit
    End If

    SetDocCountsFromFileSearch mDocFiles
    WriteFileDateCountsToLog mDocFiles

    'WriteDocsToAnalyzeToLog mDocFiles 'UNCOMMENT Recovery - want to list out files to analyze
        
    mbDocCountCurrent = True
    
    lblNumDocs.Caption = ReplaceTopicTokens(GetResString(ANALYZE_DOCUMENTS_ID), TOPIC_STR, _
        CStr(mWordDocCount))
    lblNumTemplates.Caption = ReplaceTopicTokens(GetResString(ANALYZE_TEMPLATES_ID), TOPIC_STR, _
        CStr(mWordTemplateCount))

    lblNumXLS.Caption = ReplaceTopicTokens(GetResString(ANALYZE_DOCUMENTS_XLS_ID), TOPIC_STR, _
        CStr(mExcelDocCount))
    lblNumXLT.Caption = ReplaceTopicTokens(GetResString(ANALYZE_TEMPLATES_ID), TOPIC_STR, _
        CStr(mExcelTemplateCount))
    
    lblNumPPT.Caption = ReplaceTopicTokens(GetResString(ANALYZE_DOCUMENTS_PPT_ID), TOPIC_STR, _
        CStr(mPPDocCount))
    lblNumPOT.Caption = ReplaceTopicTokens(GetResString(ANALYZE_TEMPLATES_ID), TOPIC_STR, _
        CStr(mPPTemplateCount))
            
    lblTotalNumDocs.Caption = ReplaceTopicTokens(GetResString(ANALYZE_TOTAL_NUM_DOCS_ID), TOPIC_STR, _
        CStr(mTotalDocCount))
    
    If (mIgnoredDocCount > 0) Then
        lblSkippedOld.Caption = ReplaceTopicTokens(GetResString(RID_STR_ENG_ANALYZE_IGNORED_DOCS_ID), _
                                    TOPIC_STR, CStr(mIgnoredDocCount))
        lblSkippedOld.Visible = True
    Else
        lblSkippedOld.Visible = False
    End If

    CheckNumberDocsToAnalyze = True

FinalExit:
    Set docSearchTypes = Nothing
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Sub SetDocCountsFromFileSearch(myDocFiles As CollectedFiles)
    'No Error handling required
    mWordDocCount = myDocFiles.DocCount
    mWordTemplateCount = myDocFiles.DotCount
    mExcelDocCount = myDocFiles.XlsCount
    mExcelTemplateCount = myDocFiles.XltCount
    mPPDocCount = myDocFiles.PptCount
    mPPTemplateCount = myDocFiles.PotCount
    mTotalDocCount = mWordDocCount + mWordTemplateCount + mExcelDocCount + mExcelTemplateCount + _
        mPPDocCount + mPPTemplateCount
    mIgnoredDocCount = myDocFiles.IgnoredDocCount
End Sub

Sub SetupDocSearchTypes(docSearchTypes As Collection)
    'No Error handling required
    If chkWordDoc.value Then docSearchTypes.add ("*.doc")
    If chkWordTemplate.value Then docSearchTypes.add ("*.dot")
    If chkExcelDoc.value Then docSearchTypes.add ("*.xls")
    If chkExcelTemplate.value Then docSearchTypes.add ("*.xlt")
    If chkPPDoc.value Then docSearchTypes.add ("*.ppt")
    If chkPPTemplate.value Then docSearchTypes.add ("*.pot")
End Sub

Sub WriteDocsToAnalyzeToLog(myDocFiles As CollectedFiles)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteDocsToAnalyzeToLog"
    
    Dim vFileName As Variant
    Dim Index As Long
    Dim limit As Long
    limit = myDocFiles.WordFiles.count
    For Index = 1 To limit
        vFileName = myDocFiles.WordFiles(Index)
        WriteToLog "Doc" & Index, CStr(vFileName), section:=(WIZARD_NAME & "ListFor" & CAPPNAME_WORD)
    Next
    limit = myDocFiles.ExcelFiles.count
    For Index = 1 To limit
        vFileName = myDocFiles.ExcelFiles(Index)
        WriteToLog "Doc" & Index, CStr(vFileName), section:=(WIZARD_NAME & "ListFor" & CAPPNAME_EXCEL)
    Next
    limit = myDocFiles.PowerPointFiles.count
    For Index = 1 To limit
        vFileName = myDocFiles.PowerPointFiles(Index)
        WriteToLog "Doc" & Index, CStr(vFileName), section:=(WIZARD_NAME & "ListFor" & CAPPNAME_POWERPOINT)
    Next
    
FinalExit:
    Exit Sub
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Sub WriteFileDateCountsToLog(myDocFiles As CollectedFiles)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "WriteFileDateCountsToLog"

    WriteToLog C_DOCS_LESS_3_MONTH, CStr(myDocFiles.DocsLessThan3Months), mIniFilePath
    WriteToLog C_DOCS_LESS_6_MONTH, CStr(myDocFiles.DocsLessThan6Months), mIniFilePath
    WriteToLog C_DOCS_LESS_12_MONTH, CStr(myDocFiles.DocsLessThan12Months), mIniFilePath
    WriteToLog C_DOCS_MORE_12_MONTH, CStr(myDocFiles.DocsMoreThan12Months), mIniFilePath

FinalExit:
    Exit Sub
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub


Function RunWordAnalysis(resultsTemplate As String, logFile As String, fsObject As FileSystemObject) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "RunWordAnalysis"
    Const APP_WORD = "Word"
    Dim str As String
    Dim bSuccess
    bSuccess = False
    
    Dim wrd As Word.application
    Dim aDoc As Word.Document
    Dim sWordDriverDocPath As String
    Dim RegValue As Long
        
    If (chkWordDoc.value <> vbChecked) And (chkWordTemplate.value <> vbChecked) Then
        'No Word doc filters selected
        RunWordAnalysis = True
        Exit Function
    End If

    sWordDriverDocPath = fsObject.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CWORD_DRIVER_FILE)
    If Not fsObject.FileExists(sWordDriverDocPath) Then
        str = ReplaceTopic2Tokens(GetResString(ERR_MISSING_WORD_DRIVER), _
                    TOPIC_STR, sWordDriverDocPath, CR_STR, Chr(13))
        WriteDebug str
        MsgBox str, vbCritical
        bSuccess = False
        GoTo FinalExit
    End If
    
    Set wrd = New Word.application
    If val(wrd.Version) < CSUPPORTED_VERSION Then
        str = ReplaceTopic2Tokens(GetResString(ERR_SUPPORTED_VERSION), _
                    TOPIC_STR, wrd.Version, CR_STR, Chr(13))
        WriteDebug str
        MsgBox str, vbCritical
        bSuccess = False
        GoTo FinalExit
    End If
                
    If Not CheckForAccesToWordVBProject(wrd) Then
        RegValue = -1
        If Not GiveAccessToMacroProject(APP_WORD, wrd.Version, RegValue) Then
            Dim Style, response
            str = ReplaceTopic2Tokens(GetResString(ERR_NO_ACCESS_TO_VBPROJECT), _
                TOPIC_STR, CAPPNAME_WORD, CR_STR, Chr(13))
            WriteDebug str
            Style = vbYesNo + vbQuestion + vbDefaultButton1
        
            response = MsgBox(str, Style)
            If response <> vbYes Then
                bSuccess = False
                GoTo FinalExit
            End If
        End If
    End If
      
    Set aDoc = wrd.Documents.Open(fileName:=sWordDriverDocPath)
    'Clear out any doc vars
    Dim MyObj As Variable
    For Each MyObj In aDoc.Variables
        MyObj.Delete
    Next
    
    'Setup Input Variables
    'SetupInputVariables resultsTemplate, logFile, fsObject, CAPPNAME_WORD
        
    wrd.Run ("AnalysisTool.AnalysisDriver.AnalyseDirectory")
    
    wrd.Visible = False
    bSuccess = True
    
FinalExit:
    If RegValue <> -1 Then
        SetDefaultRegValue APP_WORD, wrd.Version, RegValue
    End If
    If RegValue = 0 Then
        DeleteRegValue APP_WORD, wrd.Version
    End If
    If Not aDoc Is Nothing Then aDoc.Close (False)
    Set aDoc = Nothing

    If Not wrd Is Nothing Then wrd.Quit (False)
    Set wrd = Nothing

    RunWordAnalysis = bSuccess
    Exit Function
    
HandleErrors:
    On Error Resume Next

    bSuccess = False
    Set aDoc = Nothing
    Set wrd = Nothing
    
    Dim failedDoc As String
    
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    
    failedDoc = GetDebug(CAPPNAME_WORD, CANALYZING)
    If failedDoc = "" Or failedDoc = CSTR_ANALYSIS_LOG_DONE Then
        str = ReplaceTopic2Tokens(GetResString(ERR_AUTOMATION_FAILURE), _
                    TOPIC_STR, CAPPNAME_WORD, CR_STR, Chr(13))
    Else
        str = ReplaceTopic2Tokens(GetResString(ERR_WORD_DRIVER_CRASH), _
                    TOPIC_STR, failedDoc, CR_STR, Chr(13))
    End If
    
    WriteDebug str
    MsgBox str, vbCritical

    Resume FinalExit
End Function

Function stripLastBackslash(inputStr As String) As String
    Const MIN_DIR_SIZE = 3
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "stripLastBackslash"
    
    If Len(inputStr) > MIN_DIR_SIZE Then
        Dim lastStrChar As String
        lastStrChar = Right(inputStr, 1)
        If lastStrChar = "\" Then
            inputStr = Left(inputStr, Len(inputStr) - 1)
        End If
    End If
    stripLastBackslash = inputStr
    
    Exit Function
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    stripLastBackslash = inputStr
End Function

Function getInputDir() As String
    getInputDir = stripLastBackslash(txtInputDir.Text)
End Function

Function getOutputDir() As String
    Dim tmpStr As String
    
    tmpStr = stripLastBackslash(txtOutputDir.Text)
    
    'Bug when specifying C:\
    If tmpStr <> "" Then
        If Right(tmpStr, 1) = "\" Then
            tmpStr = Left(tmpStr, Len(tmpStr) - 1)
        End If
    End If
    getOutputDir = tmpStr
End Function

Function CheckCreateNewResultsFile(fsObject As FileSystemObject) As Boolean
    If Not fsObject.FileExists(getOutputDir & "\" & txtResultsName.Text) Then
        'No Results File - Create it
        CheckCreateNewResultsFile = True
    ElseIf rdbResultsAppend.value Then
        'Results File exists and user wants to append to it
        CheckCreateNewResultsFile = False
    Else
        'Results File exists and user has elected not to append
        CheckCreateNewResultsFile = True
    End If
End Function
    
Sub DeleteFile(file As String)
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "DeleteFile"
    Dim fso As Scripting.FileSystemObject
    Set fso = New Scripting.FileSystemObject
    Dim filePath As String
    
    filePath = fso.GetAbsolutePathName(file)
    If fso.FileExists(filePath) Then
        fso.DeleteFile filePath, True
    End If
    
FinalExit:
    Set fso = Nothing
    Exit Sub
    
HandleErrors:
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Sub

Public Property Get Version() As String
   Version = app.Major & "." & app.Minor & "." & app.Revision
End Property

Function GetExcelInstance() As Excel.application
    Dim xl As Excel.application
    On Error Resume Next
    'Try and get an existing instance
    Set xl = GetObject(, "Excel.Application")
    If Err.Number = 429 Then
        Set xl = CreateObject("Excel.Application")
    ElseIf Err.Number <> 0 Then
        Set xl = Nothing
        MsgBox "Error: " & Err.Description
        Exit Function
    End If
    Set GetExcelInstance = xl
    Set xl = Nothing
End Function

Function CheckForAnalysisResultsWorkbook(analysisResultsName As String) As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "CheckForAnalysisResultsWorkbook"
    
    CheckForAnalysisResultsWorkbook = False
    
    Dim xl As Excel.application
    Set xl = GetExcelInstance
    
    Dim aWb As Excel.Workbook
    For Each aWb In xl.Workbooks
    
        If aWb.Name = analysisResultsName Then
            CheckForAnalysisResultsWorkbook = True
            Exit For
        End If
    Next aWb
    
FinalExit:
    If Not xl Is Nothing Then
        If xl.Workbooks.count = 0 Then
            xl.Quit
        End If
    End If
    
    Set xl = Nothing

    Exit Function

HandleErrors:
    Set xl = Nothing
    
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Function CheckForExcel() As Boolean
    On Error GoTo HandleErrors
    Dim currentFunctionName As String
    currentFunctionName = "CheckForExcel"
    
    CheckForExcel = False
    
    Dim xl As Excel.application
    Set xl = GetExcelInstance
    
   
    If xl.Workbooks.count > 0 Then
        CheckForExcel = True
    End If
    
FinalExit:
    If Not xl Is Nothing Then
        If xl.Workbooks.count = 0 Then
            xl.Quit
        End If
    End If
    
    Set xl = Nothing

    Exit Function

HandleErrors:
    Set xl = Nothing
    
    WriteDebug currentFunctionName & " : " & Err.Number & " " & Err.Description & " " & Err.Source
    Resume FinalExit
End Function

Public Function GetIniSetting(key As String) As String
    
    If mIniFilePath = "" Or key = "" Then Exit Function
    
    GetIniSetting = ProfileGetItem(WIZARD_NAME, key, "", mIniFilePath)
End Function

Sub WriteIniSetting(key As String, value As String)
    
    If mIniFilePath = "" Or key = "" Then Exit Sub
    
    Call WritePrivateProfileString(WIZARD_NAME, key, value, mIniFilePath)
End Sub

Private Sub lblSetupComplete_Click(Index As Integer)

End Sub

Private Function CheckNeededFiles(missingFile As String) As Boolean
    
    Dim fso As New FileSystemObject
    Dim filePath As String

    CheckNeededFiles = False
    filePath = fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CWORD_DRIVER_FILE)
    If Not fso.FileExists(filePath) Then
        missingFile = filePath
        Exit Function
    End If
        
    filePath = fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CEXCEL_DRIVER_FILE)
    If Not fso.FileExists(filePath) Then
        missingFile = filePath
        Exit Function
    End If
        
    filePath = fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CPP_DRIVER_FILE)
    If Not fso.FileExists(filePath) Then
        missingFile = filePath
        Exit Function
    End If

    filePath = fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CLAUNCH_DRIVERS_EXE)
    If Not fso.FileExists(filePath) Then
        missingFile = filePath
        Exit Function
    End If

    filePath = fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CMSO_KILL_EXE)
    If Not fso.FileExists(filePath) Then
        missingFile = filePath
        Exit Function
    End If

    filePath = fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CRESULTS_TEMPLATE_FILE)
    If Not fso.FileExists(filePath) Then
        missingFile = filePath
        Exit Function
    End If

    filePath = fso.GetAbsolutePathName(CBASE_RESOURCE_DIR & "\" & CRESOURCE_DLL)
    If Not fso.FileExists(filePath) Then
        missingFile = filePath
        Exit Function
    End If

    CheckNeededFiles = True
End Function
