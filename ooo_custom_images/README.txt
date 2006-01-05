How to add a new image set:
---------------------------

- Create a directory for it here (let's call it e.g. new_set)

- Add it to CUSTOM_IMAGE_SETS in solenv/inc/settings.mk
  (e.g. CUSTOM_IMAGE_SETS=hicontrast industrial crystal new_set

- Add a SfxSymbolsStyle constant for it in svtools/inc/imgdef.hxx, e.g.
  SFX_SYMBOLS_STYLE_NEW_SET

- Add a vcl constant for it in vcl/inc/settings.hxx, e.g.
  #define STYLE_SYMBOLS_NEW_SET ((ULONG)5)

- Map the vcl constant to its real name in
    ::rtl::OUString StyleSettings::GetCurrentSymbolsStyleName() const
  in vcl/source/app/settings.cxx, e.g.
    case STYLE_SYMBOLS_NEW_SET: return ::rtl::OUString::createFromAscii( "new_set" );

- Enhance the SFX_SYMBOLS_STYLE <-> STYLE_SYMBOLS converting functions
    static sal_Int16 implSymbolsStyleFromVCL( ULONG nStyle )
  and
    static ULONG implSymbolsStyleToVCL( sal_Int16 nStyle )
  in svtools/source/config/miscopt.cxx.

- Add SFX_SYMBOLS_STYLE constant to 'switch( nStyleLB_NewSelection )' in 
    BOOL OfaViewTabPage::FillItemSet( SfxItemSet& rSet )
  and to 'switch ( aMiscOptions.GetCurrentSymbolsStyle() )' in
    void OfaViewTabPage::Reset( const SfxItemSet& rSet )
  both in svx/source/dialog/optgdlg.cxx. 

- Add the new_set to 'SymbolsStyle' property
  in officecfg/registry/schema/org/openoffice/Office/Common.xcs.

NOTE: The Crystal icon set is currently disabled, but reserved to be no. 4; it
cannot be committed to the OOo CVS at the moment.
Some Linux distros enable it and use ooo_crystal_images-*.tar.bz2 from
http://www.go-oo.org/packages/SRC680/ , here is the patch:

--- officecfg/registry/schema/org/openoffice/Office/Common.xcs	2005-07-21 19:25:53.000000000 +0200
+++ officecfg/registry/schema/org/openoffice/Office/Common.xcs	2005-07-21 19:49:08.000000000 +0200
@@ -4949,11 +4949,11 @@ Dymamic border coloring means that when 
 							<desc>Industrial</desc>
 						</info>
 					</enumeration>
-					<!--enumeration oor:value="4">
+					<enumeration oor:value="4">
 						<info>
 							<desc>Crystal</desc>
 						</info>
-					</enumeration-->
+					</enumeration>
 				</constraints>
 				<value>0</value>
 			</prop>
--- solenv/inc/settings.mk	2005-07-21 17:47:34.000000000 +0200
+++ solenv/inc/settings.mk	2005-07-21 20:11:52.000000000 +0200
@@ -1111,7 +1111,7 @@ RSCDEFS+= -DDEBUG
 .ENDIF
 
 # additional image sets
-CUSTOM_IMAGE_SETS=hicontrast industrial
+CUSTOM_IMAGE_SETS=hicontrast industrial crystal
 
 UNOIDL=unoidl
 
--- svtools/inc/imgdef.hxx	2005-07-21 19:38:26.000000000 +0200
+++ svtools/inc/imgdef.hxx	2005-07-20 16:04:06.000000000 +0200
@@ -74,8 +74,8 @@ enum SfxSymbolsStyle
     SFX_SYMBOLS_STYLE_AUTO,
     SFX_SYMBOLS_STYLE_DEFAULT,
     SFX_SYMBOLS_STYLE_HICONTRAST,
-    SFX_SYMBOLS_STYLE_INDUSTRIAL
-//    SFX_SYMBOLS_STYLE_CRYSTAL
+    SFX_SYMBOLS_STYLE_INDUSTRIAL,
+    SFX_SYMBOLS_STYLE_CRYSTAL
 };
 
 #define SFX_TOOLBOX_CHANGESYMBOLSET		0x0001
--- svtools/source/config/miscopt.cxx	2005-07-21 19:39:08.000000000 +0200
+++ svtools/source/config/miscopt.cxx	2005-07-20 16:23:18.000000000 +0200
@@ -282,7 +282,7 @@ static sal_Int16 implSymbolsStyleFromVCL
 		case STYLE_SYMBOLS_DEFAULT:    return SFX_SYMBOLS_STYLE_DEFAULT;
 		case STYLE_SYMBOLS_HICONTRAST: return SFX_SYMBOLS_STYLE_HICONTRAST;
 		case STYLE_SYMBOLS_INDUSTRIAL: return SFX_SYMBOLS_STYLE_INDUSTRIAL;
-//		case STYLE_SYMBOLS_CRYSTAL:    return SFX_SYMBOLS_STYLE_CRYSTAL;
+		case STYLE_SYMBOLS_CRYSTAL:    return SFX_SYMBOLS_STYLE_CRYSTAL;
 	}
 
 	return SFX_SYMBOLS_STYLE_AUTO;
@@ -296,7 +296,7 @@ static ULONG implSymbolsStyleToVCL( sal_
 		case SFX_SYMBOLS_STYLE_DEFAULT:    return STYLE_SYMBOLS_DEFAULT;
 		case SFX_SYMBOLS_STYLE_HICONTRAST: return STYLE_SYMBOLS_HICONTRAST;
 		case SFX_SYMBOLS_STYLE_INDUSTRIAL: return STYLE_SYMBOLS_INDUSTRIAL;
-//		case SFX_SYMBOLS_STYLE_CRYSTAL:    return STYLE_SYMBOLS_CRYSTAL;
+		case SFX_SYMBOLS_STYLE_CRYSTAL:    return STYLE_SYMBOLS_CRYSTAL;
 	}
 
 	return STYLE_SYMBOLS_AUTO;
--- svx/source/dialog/optgdlg.cxx	2005-07-21 19:34:37.000000000 +0200
+++ svx/source/dialog/optgdlg.cxx	2005-07-20 16:13:44.000000000 +0200
@@ -853,7 +853,7 @@ BOOL OfaViewTabPage::FillItemSet( SfxIte
             case 1: eSet = SFX_SYMBOLS_STYLE_DEFAULT;    break;
             case 2: eSet = SFX_SYMBOLS_STYLE_HICONTRAST; break;
             case 3: eSet = SFX_SYMBOLS_STYLE_INDUSTRIAL; break;
-//            case 4: eSet = SFX_SYMBOLS_STYLE_CRYSTAL;    break;
+            case 4: eSet = SFX_SYMBOLS_STYLE_CRYSTAL;    break;
             default:
                 DBG_ERROR( "OfaViewTabPage::FillItemSet(): This state of aIconStyleLB should not be possible!" );
         }
@@ -1040,7 +1040,7 @@ void OfaViewTabPage::Reset( const SfxIte
             case SFX_SYMBOLS_STYLE_DEFAULT:    nStyleLB_InitialSelection = 1; break;
             case SFX_SYMBOLS_STYLE_HICONTRAST: nStyleLB_InitialSelection = 2; break;
             case SFX_SYMBOLS_STYLE_INDUSTRIAL: nStyleLB_InitialSelection = 3; break;
-//            case SFX_SYMBOLS_STYLE_CRYSTAL:    nStyleLB_InitialSelection = 4; break;
+            case SFX_SYMBOLS_STYLE_CRYSTAL:    nStyleLB_InitialSelection = 4; break;
             default:                           nStyleLB_InitialSelection = 0; break;
         }
     }
--- svx/source/dialog/optgdlg.src	2005-07-21 19:35:26.000000000 +0200
+++ svx/source/dialog/optgdlg.src	2005-07-20 16:13:46.000000000 +0200
@@ -289,7 +289,7 @@ TabPage OFA_TP_VIEW
             < "Standard" ; > ;
             < "HiContrast" ; > ;
             < "Industrial" ; > ;
-//            < "Crystal" ; > ;
+            < "Crystal" ; > ;
         };
         StringList [ en-US ] =
         {
@@ -297,7 +297,7 @@ TabPage OFA_TP_VIEW
             < "Default" ; > ;
             < "HiContrast" ; > ;
             < "Industrial" ; > ;
-//            < "Crystal" ; > ;
+            < "Crystal" ; > ;
         };
     };
 	CheckBox CB_SYSTEM_FONT
--- vcl/inc/settings.hxx	2005-07-21 19:36:18.000000000 +0200
+++ vcl/inc/settings.hxx	2005-07-20 16:04:07.000000000 +0200
@@ -548,7 +548,7 @@ private:
 #define STYLE_SYMBOLS_DEFAULT		((ULONG)1)
 #define STYLE_SYMBOLS_HICONTRAST	((ULONG)2)
 #define STYLE_SYMBOLS_INDUSTRIAL	((ULONG)3)
-//#define STYLE_SYMBOLS_CRYSTAL		((ULONG)4)
+#define STYLE_SYMBOLS_CRYSTAL		((ULONG)4)
 
 #define STYLE_CURSOR_NOBLINKTIME    ((ULONG)0xFFFFFFFF)
 
--- vcl/source/app/settings.cxx	2005-07-21 19:37:27.000000000 +0200
+++ vcl/source/app/settings.cxx	2005-07-20 16:04:08.000000000 +0200
@@ -755,8 +755,8 @@ ULONG StyleSettings::GetCurrentSymbolsSt
 
 			if( rDesktopEnvironment.equalsIgnoreAsciiCaseAscii( "gnome" ) )
 				snDesktopStyle = STYLE_SYMBOLS_INDUSTRIAL;
-//			else if( rDesktopEnvironment.equalsIgnoreAsciiCaseAscii( "kde" ) )
-//				snDesktopStyle = STYLE_SYMBOLS_CRYSTAL;
+			else if( rDesktopEnvironment.equalsIgnoreAsciiCaseAscii( "kde" ) )
+				snDesktopStyle = STYLE_SYMBOLS_CRYSTAL;
 
 			sbDesktopChecked = true;
 		}
@@ -775,7 +775,7 @@ ULONG StyleSettings::GetCurrentSymbolsSt
 	{
 		case STYLE_SYMBOLS_HICONTRAST: return ::rtl::OUString::createFromAscii( "hicontrast" );
 		case STYLE_SYMBOLS_INDUSTRIAL: return ::rtl::OUString::createFromAscii( "industrial" );
-//		case STYLE_SYMBOLS_CRYSTAL:    return ::rtl::OUString::createFromAscii( "crystal" );
+		case STYLE_SYMBOLS_CRYSTAL:    return ::rtl::OUString::createFromAscii( "crystal" );
 	}
 
 	return ::rtl::OUString();
