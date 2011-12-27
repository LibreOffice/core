How to add a new image set:
---------------------------

- Create a directory for it here (let's call it e.g. new_set)

- Add its images_ name to CUSTOM_IMAGE_SETS in
  postprocess/packimages/pack/makefile.mk

- Add its gid to the Files section of gid_Module_Root_Brand in
  scp2/source/ooo/module_hidden_ooo.scp and add a corresponding File entry
  to scp2/source/ooo/file_ooo.scp (see for example
  gid_Brand_File_ImagesCrystal_Zip)

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

- Add localized item names to 'ListBox LB_ICONSTYLE' to
    'StringList [ de ]'
  and 'StringList [ en-US ]', e.g.
    < "New set" ; > ;
  both in cui/source/options/optgdlg.src

- Add all known localizations of the new item to svx/source/dialog/localize.sdf, e.g.
    svx	source\dialog\optgdlg.src	0	stringlist	OFA_TP_VIEW.LB_ICONSTYLE	6			0	cs	New set				2002-02-02 02:02:02
  the check box item will not be visible in the other localizations otherwise, see http://www.openoffice.org/issues/show_bug.cgi?id=74982

- Add the new_set to 'SymbolsStyle' property
  in officecfg/registry/schema/org/openoffice/Office/Common.xcs.

- The default theme for various desktops (KDE, GNOME, ...) is defined in
    ULONG StyleSettings::GetCurrentSymbolsStyle() const in
  vcl/source/app/settings.cxx

- The fallback for particular icons is defined in
    postprocess/packimages/pack/makefile.mk
