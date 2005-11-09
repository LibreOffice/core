!define PRODUCT_NAME "PRODUCTNAMEPLACEHOLDER"
!define PRODUCT_VERSION "PRODUCTVERSIONPLACEHOLDER"
!define PRODUCT_PUBLISHER "PUBLISHERPLACEHOLDER"
!define PRODUCT_WEB_SITE "WEBSITEPLACEHOLDER"

; SetCompressor lzma
SetCompressor zlib

; MUI 1.67 compatible ------
!include "MUI.nsh"

Function .onInit

  Call GetParameters
  Pop $1
  ;MessageBox MB_OK "$1"

  Push $1
  Push "/HELP="
  Call GetOptions
  Pop $2
  ;MessageBox MB_OK "HELP: $2"

  StrCmp $2 "ON" showhelp nohelp
  showhelp:
    MessageBox MB_OK|MB_ICONINFORMATION \
    "DOWNLOADNAMEPLACEHOLDER options: $\n $\n \
    /S : Silent information $\n \
    /D=<path> : NSIS installation directory (must be the last option!) $\n \
    /EXTRACTONLY=ON : NSIS only extracts the PRODUCTNAMEPLACEHOLDER PRODUCTVERSIONPLACEHOLDER installation set $\n \
    /INSTALLLOCATION=<path> : PRODUCTNAMEPLACEHOLDER PRODUCTVERSIONPLACEHOLDER installation directory $\n \
    /POSTREMOVE=ON : Removes the unpacked installation set after PRODUCTNAMEPLACEHOLDER PRODUCTVERSIONPLACEHOLDER installation $\n \
    /HELP=ON : Shows this help $\n"
    Quit
    GoTo onInitDone
    
  nohelp:

  StrCmp $INSTDIR "" pathnotset pathset
  pathnotset:

  IfSilent onInitSilent onInitNoSilent

  onInitSilent:
    StrCpy $INSTDIR "$TEMP\PRODUCTPATHPLACEHOLDER Installation Files"
  GoTo onInitDone

  onInitNoSilent:
    StrCpy $INSTDIR "$DESKTOP\PRODUCTPATHPLACEHOLDER Installation Files"
  GoTo onInitDone

  onInitDone:
  pathset:
 
FunctionEnd

Function GetParameters

  Push $R0
  Push $R1
  Push $R2
  Push $R3

  StrCpy $R2 1

  StrLen $R3 $CMDLINE

  ;Check for quote or space
  StrCpy $R0 $CMDLINE $R2

  StrCmp $R0 '"' 0 +3
    StrCpy $R1 '"'
    Goto loop
  StrCpy $R1 " "

  loop:
    IntOp $R2 $R2 + 1
    StrCpy $R0 $CMDLINE 1 $R2
    StrCmp $R0 $R1 get
    StrCmp $R2 $R3 get
    Goto loop

  get:
    IntOp $R2 $R2 + 1
    StrCpy $R0 $CMDLINE 1 $R2
    StrCmp $R0 " " get
    StrCpy $R0 $CMDLINE "" $R2

  Pop $R3
  Pop $R2
  Pop $R1
  Exch $R0

FunctionEnd

Function GetOptions
 
    Exch $1
    Exch
    Exch $0
    Exch
    Push $2
    Push $3
    Push $4
    Push $5
    Push $6
    Push $7
 
    StrCpy $2 $1 '' 1
    StrCpy $1 $1 1
    StrLen $3 $2
    StrCpy $7 0
  
    begin:
    StrCpy $4 -1
    StrCpy $6 ''
 
    quote:
    IntOp $4 $4 + 1
    StrCpy $5 $0 1 $4
    StrCmp $5$7 '0' notfound
    StrCmp $5 '' trimright
    StrCmp $5 '"' 0 +7
    StrCmp $6 '' 0 +3
    StrCpy $6 '"'
    goto quote
    StrCmp $6 '"' 0 +3
    StrCpy $6 ''
    goto quote
    StrCmp $5 `'` 0 +7
    StrCmp $6 `` 0 +3
    StrCpy $6 `'`
    goto quote
    StrCmp $6 `'` 0 +3
    StrCpy $6 ``
    goto quote
    StrCmp $5 '`' 0 +7
    StrCmp $6 '' 0 +3
    StrCpy $6 '`'
    goto quote
    StrCmp $6 '`' 0 +3
    StrCpy $6 ''
    goto quote
    StrCmp $6 '"' quote
    StrCmp $6 `'` quote
    StrCmp $6 '`' quote
    StrCmp $5 $1 0 quote
    StrCmp $7 0 trimleft trimright
 
    trimleft:
    IntOp $4 $4 + 1
    StrCpy $5 $0 $3 $4
    StrCmp $5 '' notfound
    StrCmp $5 $2 0 quote
    IntOp $4 $4 + $3
    StrCpy $0 $0 '' $4
    StrCpy $4 $0 1
    StrCmp $4 ' ' 0 +3
    StrCpy $0 $0 '' 1
    goto -3
    StrCpy $7 1
    goto begin
 
    trimright:
    StrCpy $0 $0 $4
    StrCpy $4 $0 1 -1
    StrCmp $4 ' ' 0 +3
    StrCpy $0 $0 -1
    goto -3
    StrCpy $3 $0 1
    StrCpy $4 $0 1 -1
    StrCmp $3 $4 0 end
    StrCmp $3 '"' +3
    StrCmp $3 `'` +2
    StrCmp $3 '`' 0 end
    StrCpy $0 $0 -1 1
    goto end
 
    notfound:
    StrCpy $0 ''
 
    end:
    Pop $7
    Pop $6
    Pop $5
    Pop $4
    Pop $3
    Pop $2
    Pop $1
    Exch $0
    
FunctionEnd

; MUI Settings
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
!define MUI_HEADERIMAGE_BITMAP BANNERBMPPLACEHOLDER
!define MUI_WELCOMEFINISHPAGE_BITMAP WELCOMEBMPPLACEHOLDER
!define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_ABORTWARNING
!define MUI_ICON "SETUPICOPLACEHOLDER"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!macro MUI_LANGUAGE_PACK LANGUAGE
  !verbose push
  !verbose ${MUI_VERBOSE}
  !include "NSISPATHPLACEHOLDER\${LANGUAGE}_pack.nsh"
  !verbose pop
!macroend

!macro MUI_LANGUAGEFILE_PACK_BEGIN LANGUAGE
  !ifndef MUI_INSERT
    !define MUI_INSERT
    !insertmacro MUI_INSERT
  !endif
  LoadLanguageFile "NSISPATHPLACEHOLDER\${LANGUAGE}_pack.nlf"
!macroend

; Language files
ALLLANGUAGESPLACEHOLDER

; Reserve files
;!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "PRODUCTNAMEPLACEHOLDER PRODUCTVERSIONPLACEHOLDER"
OutFile "OUTPUTDIRPLACEHOLDER\DOWNLOADNAMEPLACEHOLDER"
; InstallDir "$DESKTOP\PRODUCTNAMEPLACEHOLDER PRODUCTVERSIONPLACEHOLDER Installation Files"
; ShowInstDetails show

Section "MainSection" SEC01
ALLFILESPLACEHOLDER
SectionEnd

Section -Post

  Call GetParameters
  Pop $1

  Push $1
  Push "/EXTRACTONLY="
  Call GetOptions
  Pop $2
  ;MessageBox MB_OK "EXTRACTONLY: $2"

  StrCmp $2 "ON" onPostDone callsetup
  callsetup:
  
  Push $1
  Push "/INSTALLLOCATION="
  Call GetOptions
  Pop $2
  ;MessageBox MB_OK "INSTALLLOCATION: $2"

  StrCmp $2 "" installnotset installset
  installset:
    StrCpy $3 'INSTALLLOCATION="$2"'
  installnotset:

  IfSilent onPostSilent onPostNoSilent

  onPostSilent:
    ExecWait "$INSTDIR\setup.exe -lang $LANGUAGE /qr -ignore_running"
    RMDir /r $INSTDIR
    RMDir $INSTDIR
  GoTo onPostDone

  onPostNoSilent:
    Push $1
    Push "/POSTREMOVE="
    Call GetOptions
    Pop $2
    ;MessageBox MB_OK "POSTREMOVE: $2"

    StrCmp $2 "ON" postremove nopostremove
    nopostremove:
      Exec "$INSTDIR\setup.exe -lang $LANGUAGE $3"
      Quit
      GoTo onPostDone
    postremove:
      ExecWait "$INSTDIR\setup.exe -lang $LANGUAGE $3"
      RMDir /r $INSTDIR
      RMDir $INSTDIR
      Quit
      GoTo onPostDone

  onPostDone:

SectionEnd
