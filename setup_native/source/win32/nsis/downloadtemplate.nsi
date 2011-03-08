!define PRODUCT_NAME "PRODUCTNAMEPLACEHOLDER"
!define PRODUCT_VERSION "PRODUCTVERSIONPLACEHOLDER"
!define PRODUCT_PUBLISHER "PUBLISHERPLACEHOLDER"
!define PRODUCT_WEB_SITE "WEBSITEPLACEHOLDER"

SetCompressor zlib
SetDatablockOptimize On

; Helpful for debugging, disable for products
; RequestExecutionLevel user

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
    /S : Silent installation $\n \
    /D=<path> : NSIS installation directory (must be the last option!) $\n \
    /EXTRACTONLY=ON : NSIS only extracts the PRODUCTNAMEPLACEHOLDER PRODUCTVERSIONPLACEHOLDER installation set $\n \
    /INSTALLLOCATION=<path> : PRODUCTNAMEPLACEHOLDER PRODUCTVERSIONPLACEHOLDER installation directory $\n \
    /POSTREMOVE=ON : Removes the unpacked installation set after PRODUCTNAMEPLACEHOLDER PRODUCTVERSIONPLACEHOLDER installation $\n \
    /INSTALLJAVA=ON : Installs JRE located in sub directory java, if exists $\n \
    /GUILEVEL=<guilevel> : Setting Windows Installer GUI level: qr, qb, qn, qf, ... $\n \
    /PARAM1=$\"key=value$\" : Flexible parameter 1. Example: /PARAM1=$\"INSTALLLEVEL=70$\" $\n \
    /PARAM2=$\"key=value$\" : Flexible parameter 2. $\n \
    /PARAM3=$\"key=value$\" : Flexible parameter 3. $\n \
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
!define MUI_CUSTOMFUNCTION_GUIINIT     SetupBranding
!define MUI_CUSTOMFUNCTION_UNGUIINIT   SetupBranding

; Welcome page
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

#231#!macro MUI_LANGUAGE_PACK LANGUAGE
#231#  !verbose push
#231#  !verbose ${MUI_VERBOSE}
#231#  !insertmacro MUI_INSERT
#231#  LoadLanguageFile "NSISPATHPLACEHOLDER\${LANGUAGE}_pack.nlf"
#231#  ;Set default language file for MUI and backup user setting
#231#  !ifdef LANGFILE_DEFAULT
#231#    !define MUI_LANGFILE_DEFAULT_TEMP "${LANGFILE_DEFAULT}"
#231#    !undef LANGFILE_DEFAULT
#231#  !endif
#231#  !define LANGFILE_DEFAULT "${NSISDIR}\Contrib\Language files\English.nsh"
#231#  ;Include language file
#231#  !insertmacro LANGFILE_INCLUDE "NSISPATHPLACEHOLDER\${LANGUAGE}_pack.nsh"
#231#  ;Restore user setting for default language file
#231#  !undef LANGFILE_DEFAULT
#231#  !ifdef MUI_LANGFILE_DEFAULT_TEMP
#231#    !define LANGFILE_DEFAULT "${MUI_LANGFILE_DEFAULT}"
#231#  !endif
#231#  ;Add language to list of languages for selection dialog  
#231#  !ifndef MUI_LANGDLL_LANGUAGES
#231#   !define MUI_LANGDLL_LANGUAGES "'${LANGFILE_${LANGUAGE}_NAME}' '${LANG_${LANGUAGE}}' "
#231#    !define MUI_LANGDLL_LANGUAGES_CP "'${LANGFILE_${LANGUAGE}_NAME}' '${LANG_${LANGUAGE}}' '${LANG_${LANGUAGE}_CP}' "
#231#  !else
#231#    !ifdef MUI_LANGDLL_LANGUAGES_TEMP
#231#      !undef MUI_LANGDLL_LANGUAGES_TEMP
#231#    !endif
#231#    !define MUI_LANGDLL_LANGUAGES_TEMP "${MUI_LANGDLL_LANGUAGES}"
#231#    !undef MUI_LANGDLL_LANGUAGES
#231#	!ifdef MUI_LANGDLL_LANGUAGES_CP_TEMP
#231#      !undef MUI_LANGDLL_LANGUAGES_CP_TEMP
#231#    !endif
#231#    !define MUI_LANGDLL_LANGUAGES_CP_TEMP "${MUI_LANGDLL_LANGUAGES_CP}"
#231#    !undef MUI_LANGDLL_LANGUAGES_CP
#231#    !define MUI_LANGDLL_LANGUAGES "'${LANGFILE_${LANGUAGE}_NAME}' '${LANG_${LANGUAGE}}' ${MUI_LANGDLL_LANGUAGES_TEMP}"
#231#    !define MUI_LANGDLL_LANGUAGES_CP "'${LANGFILE_${LANGUAGE}_NAME}' '${LANG_${LANGUAGE}}' '${LANG_${LANGUAGE}_CP}' ${MUI_LANGDLL_LANGUAGES_CP_TEMP}"
#231#  !endif
#231#  !verbose pop
#231#!macroend

#204#!macro MUI_LANGUAGE_PACK LANGUAGE
#204#  !verbose push
#204#  !verbose ${MUI_VERBOSE}
#204#  !include "NSISPATHPLACEHOLDER\${LANGUAGE}_pack.nsh"
#204#  !verbose pop
#204#!macroend

#204#!macro MUI_LANGUAGEFILE_PACK_BEGIN LANGUAGE
#204#  !ifndef MUI_INSERT
#204#    !define MUI_INSERT
#204#    !insertmacro MUI_INSERT
#204#  !endif
#204#  LoadLanguageFile "NSISPATHPLACEHOLDER\${LANGUAGE}_pack.nlf"
#204#!macroend

; Language files
ALLLANGUAGESPLACEHOLDER

Function SetupBranding
	File "/oname=$PLUGINSDIR\modern-header.bmp" BANNERBMPPLACEHOLDER
	File "/oname=$PLUGINSDIR\modern-header-br.bmp" BANNERBMPPLACEHOLDER_BR
	File "/oname=$PLUGINSDIR\modern-wizard.bmp" WELCOMEBMPPLACEHOLDER
	File "/oname=$PLUGINSDIR\modern-wizard-br.bmp" WELCOMEBMPPLACEHOLDER_BR

	; use broffice specific branding if we are in Brazil
;	messageBox MB_OK "Language: $LANGUAGE"
	StrCmp $LANGUAGE "1046" BrOfficeSplash DefaultSplash
	DefaultSplash:
;		messageBox MB_OK "LibreOffice"
		SetBrandingImage /IMGID=1046 "$PLUGINSDIR\modern-header.bmp"
		!insertmacro INSTALLOPTIONS_WRITE "ioSpecial.ini" "Field 1" "Text" "$PLUGINSDIR\modern-wizard.bmp"
		GoTo atEnd
	BrOfficeSplash:
;		messageBox MB_OK "BrOffice"
		SetBrandingImage /IMGID=1046 "$PLUGINSDIR\modern-header-br.bmp"
		!insertmacro INSTALLOPTIONS_WRITE "ioSpecial.ini" "Field 1" "Text" "$PLUGINSDIR\modern-wizard-br.bmp"
		GoTo atEnd
	atEnd:
FunctionEnd

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

  StrCpy $R9 "false"
  
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

  Push $1
  Push "/INSTALLJAVA="
  Call GetOptions
  Pop $2
  ;MessageBox MB_OK "INSTALLJAVA: $2"

  StrCmp $2 "ON" setinstalljava setdontinstalljava 
  setinstalljava:
    StrCpy $R9 "true"
  setdontinstalljava:

  Push $1
  Push "/GUILEVEL="
  Call GetOptions
  Pop $2

  StrCmp $2 "" dontsetguilevel setguilevel 
  setguilevel:
    StrCpy $7 "/"
    StrCpy $7 $7$2
    GoTo afterguilevel
  dontsetguilevel:
    StrCpy $7 "/qr"
  afterguilevel:

  ;MessageBox MB_OK "GUILEVEL: $7"

  Push $1
  Push "/PARAM1="
  Call GetOptions
  Pop $2
  ;MessageBox MB_OK "PARAM1: $2"

  StrCmp $2 "" param1notset param1set
  param1set:
    StrCpy $4 "$2"
  param1notset:


  Push $1
  Push "/PARAM2="
  Call GetOptions
  Pop $2
  ;MessageBox MB_OK "PARAM2: $2"

  StrCmp $2 "" param2notset param2set
  param2set:
    StrCpy $5 "$2"
  param2notset:


  Push $1
  Push "/PARAM3="
  Call GetOptions
  Pop $2
  ;MessageBox MB_OK "PARAM3: $2"

  StrCmp $2 "" param3notset param3set
  param3set:
    StrCpy $6 "$2"
  param3notset:

  IfSilent onPostSilent onPostNoSilent

  onPostSilent:
    Push $1
    Push "/POSTREMOVE="
    Call GetOptions
    Pop $2
    ;MessageBox MB_OK "POSTREMOVE: $2"

    StrCmp $2 "ON" postremovesilent nopostremovesilent
    nopostremovesilent:
      StrCmp $R9 "true" installjava1 dontinstalljava1 
      installjava1:
        ExecWait '$INSTDIR\java\WINDOWSJAVAFILENAMEPLACEHOLDER /s /v"/qn REBOOT=Suppress"'
      dontinstalljava1:
      ExecWait "$INSTDIR\setup.exe -lang $LANGUAGE $3 $4 $5 $6 $7 -ignore_running" $0
      SetErrorLevel $0
      Quit
      GoTo onPostDone
    postremovesilent:
      StrCmp $R9 "true" installjava2 dontinstalljava2 
      installjava2:
        ExecWait '$INSTDIR\java\WINDOWSJAVAFILENAMEPLACEHOLDER /s /v"/qn REBOOT=Suppress"'
      dontinstalljava2:
      ExecWait "$INSTDIR\setup.exe -lang $LANGUAGE $3 $4 $5 $6 $7 -ignore_running" $0
      RMDir /r $INSTDIR
      RMDir $INSTDIR
      SetErrorLevel $0
      Quit
      GoTo onPostDone

  onPostNoSilent:
    Push $1
    Push "/POSTREMOVE="
    Call GetOptions
    Pop $2
    ;MessageBox MB_OK "POSTREMOVE: $2"

    StrCmp $2 "ON" postremove nopostremove
    nopostremove:
      Exec "$INSTDIR\setup.exe -lang $LANGUAGE $3 $4 $5 $6"
      Quit
      GoTo onPostDone
    postremove:
      StrCmp $R9 "true" installjava3 dontinstalljava3 
      installjava3:
        ExecWait '$INSTDIR\java\WINDOWSJAVAFILENAMEPLACEHOLDER /s /v"/qr REBOOT=Suppress"'
      dontinstalljava3:
      ExecWait "$INSTDIR\setup.exe -lang $LANGUAGE $3 $4 $5 $6" $0
      RMDir /r $INSTDIR
      RMDir $INSTDIR
      SetErrorLevel $0
      Quit
      GoTo onPostDone

  onPostDone:

SectionEnd
