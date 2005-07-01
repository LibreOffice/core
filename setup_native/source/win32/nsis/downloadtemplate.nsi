!define PRODUCT_NAME "PRODUCTNAMEPLACEHOLDER"
!define PRODUCT_VERSION "PRODUCTVERSIONPLACEHOLDER"
!define PRODUCT_PUBLISHER "PUBLISHERPLACEHOLDER"
!define PRODUCT_WEB_SITE "WEBSITEPLACEHOLDER"

; SetCompressor lzma
SetCompressor zlib

; MUI 1.67 compatible ------
!include "MUI.nsh"

Function .onInit
IfSilent onInitSilent onInitNoSilent

onInitSilent:
   StrCpy $INSTDIR "$TEMP\PRODUCTNAMEPLACEHOLDER PRODUCTVERSIONPLACEHOLDER Installation Files"
GoTo onInitDone

onInitNoSilent:
   StrCpy $INSTDIR "$DESKTOP\PRODUCTNAMEPLACEHOLDER PRODUCTVERSIONPLACEHOLDER Installation Files"
GoTo onInitDone

onInitDone:
 
FunctionEnd

; MUI Settings
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
!define MUI_HEADERIMAGE_BITMAP BANNERBMPPLACEHOLDER
!define MUI_WELCOMEFINISHPAGE_BITMAP WELCOMEBMPPLACEHOLDER
!define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH
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

IfSilent onPostSilent onPostNoSilent

onPostSilent:
	ExecWait "$INSTDIR\setup.exe -lang $LANGUAGE /qr -ignore_running"
	RMDir /r $INSTDIR
	RMDir $INSTDIR
GoTo onPostDone

onPostNoSilent:
	Exec "$INSTDIR\setup.exe -lang $LANGUAGE"
	Quit
GoTo onPostDone

onPostDone:

SectionEnd
