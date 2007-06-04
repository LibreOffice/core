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
  and 
    ULONG StyleSettings::ImplNameToSymbolsStyle( const ::rtl::OUString &rName ) const
  both in vcl/source/app/settings.cxx, e.g.
    case STYLE_SYMBOLS_NEW_SET: return ::rtl::OUString::createFromAscii( "new_set" );
  and 
    else if ( rName == ::rtl::OUString::createFromAscii( "new_set" ) )
    	return STYLE_SYMBOLS_NEW_SET;

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

- Add localized item names to 'ListBox LB_ICONSTYLE' to
    'StringList [ de ]'
  and 'StringList [ en-US ]', e.g.
    < "New set" ; > ;
  both in svx/source/dialog/optgdlg.src

- Add all known localizations of the new item to svx/source/dialog/localize.sdf, e.g.
    svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	cs	New set				2002-02-02 02:02:02
  the check box item will not be visible in the other localizations otherwise, see http://www.openoffice.org/issues/show_bug.cgi?id=74982

- Add the new_set to 'SymbolsStyle' property
  in officecfg/registry/schema/org/openoffice/Office/Common.xcs.

- The default theme for various desktops (KDE, GNOME, ...) is defined in
    ULONG StyleSettings::GetCurrentSymbolsStyle() const in
  vcl/source/app/settings.cxx

- The fallback for particular icons is defined in
    instsetoo_native/packimages/makefile.mk


Sample patch that adds the Tango icon theme:

--- cut ---
--- officecfg/registry/schema/org/openoffice/Office/Common.xcs.old	2007-03-22 12:25:46.000000000 +0100
+++ officecfg/registry/schema/org/openoffice/Office/Common.xcs	2007-03-22 12:30:44.000000000 +0100
@@ -5183,6 +5183,11 @@ Dymamic border coloring means that when 
 							<desc>Crystal - the KDE default theme</desc>
 						</info>
 					</enumeration>
+					<enumeration oor:value="tango">
+						<info>
+							<desc>Tango</desc>
+						</info>
+					</enumeration>
 				</constraints>
 				<value>auto</value>
 			</prop>
--- svtools/inc/imgdef.hxx.old	2006-02-09 15:19:59.000000000 +0100
+++ svtools/inc/imgdef.hxx	2007-03-19 14:01:14.000000000 +0100
@@ -49,7 +49,8 @@ enum SfxSymbolsStyle
     SFX_SYMBOLS_STYLE_DEFAULT,
     SFX_SYMBOLS_STYLE_HICONTRAST,
     SFX_SYMBOLS_STYLE_INDUSTRIAL,
-    SFX_SYMBOLS_STYLE_CRYSTAL
+    SFX_SYMBOLS_STYLE_CRYSTAL,
+    SFX_SYMBOLS_STYLE_TANGO
 };
 
 #define SFX_TOOLBOX_CHANGESYMBOLSET		0x0001
--- svtools/source/config/miscopt.cxx.old	2007-03-15 16:55:37.000000000 +0100
+++ svtools/source/config/miscopt.cxx	2007-03-19 13:09:42.000000000 +0100
@@ -291,6 +291,7 @@ static sal_Int16 implSymbolsStyleFromVCL
 		case STYLE_SYMBOLS_HICONTRAST: return SFX_SYMBOLS_STYLE_HICONTRAST;
 		case STYLE_SYMBOLS_INDUSTRIAL: return SFX_SYMBOLS_STYLE_INDUSTRIAL;
 		case STYLE_SYMBOLS_CRYSTAL:    return SFX_SYMBOLS_STYLE_CRYSTAL;
+		case STYLE_SYMBOLS_TANGO:      return SFX_SYMBOLS_STYLE_TANGO;
 	}
 
 	return SFX_SYMBOLS_STYLE_AUTO;
@@ -305,6 +306,7 @@ static ULONG implSymbolsStyleToVCL( sal_
 		case SFX_SYMBOLS_STYLE_HICONTRAST: return STYLE_SYMBOLS_HICONTRAST;
 		case SFX_SYMBOLS_STYLE_INDUSTRIAL: return STYLE_SYMBOLS_INDUSTRIAL;
 		case SFX_SYMBOLS_STYLE_CRYSTAL:    return STYLE_SYMBOLS_CRYSTAL;
+		case SFX_SYMBOLS_STYLE_TANGO:      return STYLE_SYMBOLS_TANGO;
 	}
 
 	return STYLE_SYMBOLS_AUTO;
--- svx/source/dialog/optgdlg.cxx.old	2007-03-22 12:25:42.000000000 +0100
+++ svx/source/dialog/optgdlg.cxx	2007-03-22 12:29:53.000000000 +0100
@@ -915,6 +915,7 @@ BOOL OfaViewTabPage::FillItemSet( SfxIte
             case 2: eSet = SFX_SYMBOLS_STYLE_HICONTRAST; break;
             case 3: eSet = SFX_SYMBOLS_STYLE_INDUSTRIAL; break;
             case 4: eSet = SFX_SYMBOLS_STYLE_CRYSTAL;    break;
+            case 5: eSet = SFX_SYMBOLS_STYLE_TANGO;      break;
             default:
                 DBG_ERROR( "OfaViewTabPage::FillItemSet(): This state of aIconStyleLB should not be possible!" );
         }
@@ -1085,6 +1086,7 @@ void OfaViewTabPage::Reset( const SfxIte
             case SFX_SYMBOLS_STYLE_HICONTRAST: nStyleLB_InitialSelection = 2; break;
             case SFX_SYMBOLS_STYLE_INDUSTRIAL: nStyleLB_InitialSelection = 3; break;
             case SFX_SYMBOLS_STYLE_CRYSTAL:    nStyleLB_InitialSelection = 4; break;
+            case SFX_SYMBOLS_STYLE_TANGO:      nStyleLB_InitialSelection = 5; break;
             default:                           nStyleLB_InitialSelection = 0; break;
         }
     }
--- svx/source/dialog/optgdlg.src.old	2006-02-28 11:44:17.000000000 +0100
+++ svx/source/dialog/optgdlg.src	2007-03-19 13:21:03.000000000 +0100
@@ -241,6 +241,7 @@
             < "HiContrast" ; > ;
             < "Industrial" ; > ;
             < "Crystal" ; > ;
+            < "Tango" ; > ;
         };
     };
 	CheckBox CB_SYSTEM_FONT
--- vcl/source/app/settings.cxx.old	2007-03-15 16:55:35.000000000 +0100
+++ vcl/source/app/settings.cxx	2007-03-19 13:07:16.000000000 +0100
@@ -734,6 +734,7 @@ void StyleSettings::Set3DColors( const C
 		case STYLE_SYMBOLS_HICONTRAST: return ::rtl::OUString::createFromAscii( "hicontrast" );
 		case STYLE_SYMBOLS_INDUSTRIAL: return ::rtl::OUString::createFromAscii( "industrial" );
 		case STYLE_SYMBOLS_CRYSTAL:    return ::rtl::OUString::createFromAscii( "crystal" );
+		case STYLE_SYMBOLS_TANGO:      return ::rtl::OUString::createFromAscii( "tango" );
 	}
 
 	return ::rtl::OUString::createFromAscii( "auto" );
@@ -751,6 +752,8 @@ ULONG StyleSettings::ImplNameToSymbolsSt
 		return STYLE_SYMBOLS_INDUSTRIAL;
 	else if ( rName == ::rtl::OUString::createFromAscii( "crystal" ) )
 		return STYLE_SYMBOLS_CRYSTAL;
+	else if ( rName == ::rtl::OUString::createFromAscii( "tango" ) )
+		return STYLE_SYMBOLS_TANGO;
 
 	return STYLE_SYMBOLS_AUTO;
 }
--- vcl/inc/vcl/settings.hxx.old	2006-07-19 16:57:26.000000000 +0200
+++ vcl/inc/vcl/settings.hxx	2007-03-19 13:03:39.000000000 +0100
@@ -528,6 +528,7 @@ private:
 #define STYLE_SYMBOLS_HICONTRAST	((ULONG)2)
 #define STYLE_SYMBOLS_INDUSTRIAL	((ULONG)3)
 #define STYLE_SYMBOLS_CRYSTAL		((ULONG)4)
+#define STYLE_SYMBOLS_TANGO		((ULONG)5)
 
 #define STYLE_CURSOR_NOBLINKTIME    ((ULONG)0xFFFFFFFF)
 
--- solenv/inc/settings.mk.old	2007-03-22 12:25:49.000000000 +0100
+++ solenv/inc/settings.mk	2007-03-22 12:29:53.000000000 +0100
@@ -998,7 +998,7 @@
 .ENDIF
 
 # additional image sets
-CUSTOM_IMAGE_SETS=hicontrast industrial crystal
+CUSTOM_IMAGE_SETS=hicontrast industrial crystal tango
 
 # settings for mozilla idl compiler
 XPIDL=xpidl
--- instsetoo_native/packimages/makefile.mk.old	2007-03-19 20:25:22.000000000 +0100
+++ instsetoo_native/packimages/makefile.mk	2007-03-22 16:25:37.000000000 +0100
@@ -44,7 +44,8 @@ RSCCUSTOMIMG*=$(PRJ)$/util
 
 IMAGES := $(COMMONBIN)$/images.zip
 # Custom sets, at 24x24 & 16x16 fall-back to industrial preferentially
-CUSTOM_PREFERRED_FALLBACK=$(SOLARSRC)$/ooo_custom_images$/industrial
+CUSTOM_PREFERRED_FALLBACK_1=$(SOLARSRC)$/ooo_custom_images$/tango
+CUSTOM_PREFERRED_FALLBACK_2=$(SOLARSRC)$/ooo_custom_images$/industrial
 CUSTOM_IMAGES=$(foreach,i,$(CUSTOM_IMAGE_SETS) images_$i)
 
 CRYSTAL_TARBALL=$(SOLARSRC)$/external_images$/ooo_crystal_images-1.tar.gz
@@ -65,7 +66,7 @@ $(COMMONBIN)$/images.zip .PHONY: $(RES)$
     $(PERL) $(SOLARENV)$/bin$/packimages.pl -g $(SOLARSRC)$/$(RSCDEFIMG) -m $(SOLARSRC)$/$(RSCDEFIMG) -c $(RSCCUSTOMIMG) -l $(SOLARCOMMONRESDIR)$/img -l $(RES)$/img -o $@
 
 images_% : $(RES)$/img$/commandimagelist.ilst
-    $(PERL) $(SOLARENV)$/bin$/packimages.pl -g $(SOLARSRC)$/$(RSCDEFIMG) -m $(SOLARSRC)$/$(RSCDEFIMG) -c $(RSCCUSTOMIMG) -c $(SOLARSRC)$/ooo_custom_images$/$(@:s/images_//) -c $(MISC)$/$(@:s/images_//) -c $(CUSTOM_PREFERRED_FALLBACK) -l $(SOLARCOMMONRESDIR)$/img -l $(RES)$/img -o $(COMMONBIN)$/$@.zip
+    $(PERL) $(SOLARENV)$/bin$/packimages.pl -g $(SOLARSRC)$/$(RSCDEFIMG) -m $(SOLARSRC)$/$(RSCDEFIMG) -c $(RSCCUSTOMIMG) -c $(SOLARSRC)$/ooo_custom_images$/$(@:s/images_//) -c $(MISC)$/$(@:s/images_//) -c $(CUSTOM_PREFERRED_FALLBACK_1) -c $(CUSTOM_PREFERRED_FALLBACK_2) -l $(SOLARCOMMONRESDIR)$/img -l $(RES)$/img -o $(COMMONBIN)$/$@.zip
 
 # generate the HiContrast icon set
 $(MISC)$/hicontrast.flag .PHONY :
--- svx/source/dialog/localize.sdf.old	2007-03-22 12:24:42.000000000 +0100
+++ svx/source/dialog/localize.sdf	2007-03-22 12:36:58.000000000 +0100
@@ -120898,6 +120898,85 @@
 svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	5			0	zh-CN	Crystal				2002-02-02 02:02:02
 svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	5			0	zh-TW	Crystal				2002-02-02 02:02:02
 svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	5			0	zu	Crystal				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	af	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ar	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	be-BY	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	bg	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	bn	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	bn-BD	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	bn-IN	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	br	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	bs	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ca	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	cs	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	cy	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	da	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	el	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	en-GB	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	en-ZA	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	eo	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	es	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	et	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	eu	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	fa	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	fi	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	fr	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ga	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	gl	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	gu	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	gu-IN	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	he	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	hi-IN	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	hr	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	hu	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	it	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ja	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ka	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	km	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	kn-IN	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ko	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ku	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	lo	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	lt	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	lv	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	mk	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ms	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	nb	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ne	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	nl	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	nn	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	nr	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ns	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	pa-IN	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	pl	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	pt	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	pt-BR	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ru	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	rw	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	sh-YU	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	sk	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	sl	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	sr-CS	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ss	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	st	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	sv	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	sw	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	sw-TZ	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	sx	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ta	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ta-IN	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	tg	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	th	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	tn	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	tr	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ts	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	uk	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	ve	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	vi	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	xh	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	zh-CN	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	zh-TW	Tango				2002-02-02 02:02:02
+svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	zu	Tango				2002-02-02 02:02:02
 svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_MOUSEMIDDLE	1			0	af	Geen funksie				2002-02-02 02:02:02
 svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_MOUSEMIDDLE	1			0	ar	لصق الحافظة				20040507 14:25:45
 svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_MOUSEMIDDLE	1			0	as-IN	কোনো ফাংকশ্বন নাই				2002-02-02 02:02:02
--- cut ---
