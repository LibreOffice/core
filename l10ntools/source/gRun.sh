#!/bin/bash

cd ${LODE_HOME}/dev/work

MYCMD='./workdir/LinkTarget/Executable/genlang.exe extract --target workdir/jan --files '

${MYCMD} readlicense_oo/docs/readme.xrm

${MYCMD} swext/mediawiki/help/help.tree
exit

${MYCMD} extras/source/autocorr/emoji/emoji.ulf
${MYCMD} extras/source/gallery/share/gallery_names.ulf
${MYCMD} instsetoo_native/inc_openoffice/windows/msi_languages/ActionTe.ulf instsetoo_native/inc_openoffice/windows/msi_languages/Control.ulf instsetoo_native/inc_openoffice/windows/msi_languages/CustomAc.ulf instsetoo_native/inc_openoffice/windows/msi_languages/Error.ulf instsetoo_native/inc_openoffice/windows/msi_languages/LaunchCo.ulf instsetoo_native/inc_openoffice/windows/msi_languages/Property.ulf instsetoo_native/inc_openoffice/windows/msi_languages/RadioBut.ulf instsetoo_native/inc_openoffice/windows/msi_languages/UIText.ulf
${MYCMD} scp2/source/accessories/module_accessories.ulf scp2/source/accessories/module_font_accessories.ulf scp2/source/accessories/module_gallery_accessories.ulf scp2/source/accessories/module_samples_accessories.ulf scp2/source/accessories/module_templates_accessories.ulf
${MYCMD} scp2/source/activex/module_activex.ulf
${MYCMD} scp2/source/base/module_base.ulf scp2/source/base/folderitem_base.ulf scp2/source/base/postgresqlsdbc.ulf scp2/source/base/registryitem_base.ulf
${MYCMD} scp2/source/calc/module_calc.ulf scp2/source/calc/folderitem_calc.ulf scp2/source/calc/registryitem_calc.ulf
${MYCMD} scp2/source/draw/module_draw.ulf scp2/source/draw/folderitem_draw.ulf scp2/source/draw/registryitem_draw.ulf
${MYCMD} scp2/source/extensions/module_extensions.ulf scp2/source/extensions/module_extensions_sun_templates.ulf
${MYCMD} scp2/source/gnome/module_gnome.ulf
${MYCMD} scp2/source/graphicfilter/module_graphicfilter.ulf
${MYCMD} scp2/source/impress/folderitem_impress.ulf scp2/source/impress/module_impress.ulf scp2/source/impress/registryitem_impress.ulf
${MYCMD} scp2/source/kde/module_kde.ulf
${MYCMD} scp2/source/math/folderitem_math.ulf scp2/source/math/module_math.ulf scp2/source/math/registryitem_math.ulf
${MYCMD} scp2/source/onlineupdate/module_onlineupdate.ulf
${MYCMD} scp2/source/ooo/folderitem_ooo.ulf scp2/source/ooo/module_helppack.ulf scp2/source/ooo/module_langpack.ulf scp2/source/ooo/module_ooo.ulf scp2/source/ooo/module_reportbuilder.ulf scp2/source/ooo/module_systemint.ulf scp2/source/ooo/registryitem_ooo.ulf
${MYCMD} scp2/source/python/module_python_librelogo.ulf
${MYCMD} scp2/source/quickstart/module_quickstart.ulf
${MYCMD} scp2/source/tde/module_tde.ulf
${MYCMD} scp2/source/winexplorerext/module_winexplorerext.ulf
${MYCMD} scp2/source/writer/folderitem_writer.ulf scp2/source/writer/module_writer.ulf scp2/source/writer/registryitem_writer.ulf
${MYCMD} scp2/source/xsltfilter/module_xsltfilter.ulf
${MYCMD} setup_native/source/mac/macinstall.ulf
${MYCMD} shell/source/win32/shlxthandler/res/shlxthdl.ulf
${MYCMD} sysui/desktop/share/documents.ulf sysui/desktop/share/launcher_comment.ulf sysui/desktop/share/launcher_genericname.ulf sysui/desktop/share/launcher_unityquicklist.ulf


${MYCMD} accessibility/inc/accessibility/helper/accessiblestrings.hrc
${MYCMD} avmedia/inc/helpids.hrc
${MYCMD} avmedia/source/framework/mediacontrol.hrc
${MYCMD} avmedia/source/viewer/mediawindow.hrc
${MYCMD} basctl/inc/basidesh.hrc basctl/inc/helpid.hrc
${MYCMD} basctl/sdi/basslots.hrc
${MYCMD} basctl/source/basicide/baside2.hrc basctl/source/basicide/macrodlg.hrc basctl/source/basicide/objdlg.hrc
${MYCMD} basctl/source/inc/dlgresid.hrc
${MYCMD} basic/inc/sb.hrc
${MYCMD} chart2/source/controller/dialogs/Bitmaps.hrc chart2/source/controller/dialogs/ResourceIds.hrc
${MYCMD} chart2/source/controller/inc/HelpIds.hrc chart2/source/controller/inc/MenuResIds.hrc chart2/source/controller/inc/ShapeController.hrc
${MYCMD} chart2/source/controller/main/DrawCommandDispatch.hrc
${MYCMD} chart2/source/inc/Strings.hrc
${MYCMD} connectivity/source/drivers/hsqldb/hsqlui.hrc
${MYCMD} connectivity/source/inc/resource/ado_res.hrc connectivity/source/inc/resource/calc_res.hrc connectivity/source/inc/resource/common_res.hrc connectivity/source/inc/resource/conn_shared_res.hrc connectivity/source/inc/resource/dbase_res.hrc connectivity/source/inc/resource/evoab2_res.hrc connectivity/source/inc/resource/file_res.hrc connectivity/source/inc/resource/hsqldb_res.hrc connectivity/source/inc/resource/jdbc_log.hrc connectivity/source/inc/resource/kab_res.hrc connectivity/source/inc/resource/macab_res.hrc connectivity/source/inc/resource/mork_res.hrc
${MYCMD} cui/source/customize/acccfg.hrc cui/source/customize/cfg.hrc
${MYCMD} cui/source/dialogs/fmsearch.hrc cui/source/dialogs/scriptdlg.hrc cui/source/dialogs/svuidlg.hrc
${MYCMD} cui/source/inc/cuires.hrc cui/source/inc/gallery.hrc cui/source/inc/helpid.hrc
${MYCMD} cui/source/options/optsave.hrc
${MYCMD} cui/source/tabpages/align.hrc cui/source/tabpages/border.hrc cui/source/tabpages/numpages.hrc cui/source/tabpages/paragrph.hrc
${MYCMD} dbaccess/inc/dbaccess_helpid.hrc dbaccess/inc/dbaccess_slotid.hrc
${MYCMD} dbaccess/source/core/inc/core_resource.hrc dbaccess/source/core/inc/dbamiscres.hrc
${MYCMD} dbaccess/source/ext/macromigration/dbmm_global.hrc dbaccess/source/ext/macromigration/macromigration.hrc
${MYCMD} dbaccess/source/inc/dbastrings.hrc dbaccess/source/inc/dbustrings.hrc dbaccess/source/inc/sdbtstrings.hrc dbaccess/source/inc/stringconstants.hrc dbaccess/source/inc/xmlstrings.hrc
${MYCMD} dbaccess/source/sdbtools/inc/sdbt_resource.hrc
${MYCMD} dbaccess/source/ui/app/dbu_app.hrc
${MYCMD} dbaccess/source/ui/inc/dbu_brw.hrc dbaccess/source/ui/inc/dbu_control.hrc dbaccess/source/ui/inc/dbu_dlg.hrc dbaccess/source/ui/inc/dbu_misc.hrc dbaccess/source/ui/inc/dbu_qry.hrc dbaccess/source/ui/inc/dbu_rel.hrc dbaccess/source/ui/inc/dbu_resource.hrc dbaccess/source/ui/inc/dbu_tbl.hrc dbaccess/source/ui/inc/dbu_uno.hrc dbaccess/source/ui/inc/sbagrid.hrc
${MYCMD} dbaccess/source/ui/querydesign/Query.hrc
${MYCMD} desktop/inc/deployment.hrc
${MYCMD} desktop/source/app/desktop.hrc
${MYCMD} desktop/source/deployment/gui/dp_gui.hrc
${MYCMD} desktop/source/deployment/manager/dp_manager.hrc
${MYCMD} desktop/source/deployment/registry/component/dp_component.hrc
${MYCMD} desktop/source/deployment/registry/configuration/dp_configuration.hrc
${MYCMD} desktop/source/deployment/registry/help/dp_help.hrc
${MYCMD} desktop/source/deployment/registry/inc/dp_registry.hrc
${MYCMD} desktop/source/deployment/registry/package/dp_package.hrc
${MYCMD} desktop/source/deployment/registry/script/dp_script.hrc
${MYCMD} desktop/source/deployment/registry/sfwk/dp_sfwk.hrc
${MYCMD} desktop/source/inc/helpid.hrc
${MYCMD} editeng/inc/editeng.hrc editeng/inc/helpid.hrc
${MYCMD} extensions/inc/abpilot.hrc extensions/inc/bibliography.hrc extensions/inc/dbpilots.hrc extensions/inc/extensio.hrc extensions/inc/propctrlr.hrc extensions/inc/update.hrc
${MYCMD} extensions/source/abpilot/abpresid.hrc
${MYCMD} extensions/source/bibliography/bib.hrc extensions/source/bibliography/bibprop.hrc
${MYCMD} extensions/source/dbpilots/dbpresid.hrc
${MYCMD} extensions/source/propctrlr/formlocalid.hrc extensions/source/propctrlr/formresid.hrc extensions/source/propctrlr/listselectiondlg.hrc extensions/source/propctrlr/propresid.hrc
${MYCMD} extensions/source/scanner/strings.hrc
${MYCMD} extensions/source/update/check/updatehdl.hrc extensions/source/update/ui/updatecheckui.hrc
${MYCMD} filter/inc/filter.hrc
${MYCMD} filter/source/graphicfilter/eps/strings.hrc
${MYCMD} filter/source/pdf/impdialog.hrc filter/source/pdf/pdf.hrc
${MYCMD} filter/source/t602/t602filter.hrc
${MYCMD} filter/source/xsltdialog/xmlfilterdialogstrings.hrc
${MYCMD} forms/source/inc/frm_resource.hrc forms/source/inc/property.hrc
${MYCMD} formula/inc/helpids.hrc
${MYCMD} formula/source/core/inc/core_resource.hrc
${MYCMD} formula/source/ui/dlg/formdlgs.hrc
${MYCMD} formula/source/ui/inc/ForResId.hrc
${MYCMD} fpicker/source/office/iodlg.hrc fpicker/source/office/OfficeFilePicker.hrc
${MYCMD} framework/inc/classes/resource.hrc
${MYCMD} framework/inc/helpid.hrc
${MYCMD} include/editeng/editids.hrc include/editeng/editrids.hrc include/editeng/memberids.hrc
${MYCMD} include/formula/compiler.hrc
${MYCMD} include/sfx2/cntids.hrc include/sfx2/dialogs.hrc include/sfx2/sfx.hrc include/sfx2/sfxsids.hrc
${MYCMD} include/sfx2/sidebar/ResourceDefinitions.hrc include/sfx2/sidebar/Sidebar.hrc
${MYCMD} include/sfx2/templatelocnames.hrc
${MYCMD} include/svl/memberid.hrc include/svl/solar.hrc include/svl/style.hrc include/svl/svl.hrc
${MYCMD} include/svtools/controldims.hrc include/svtools/filedlg2.hrc include/svtools/helpid.hrc include/svtools/imagemgr.hrc include/svtools/svtools.hrc
${MYCMD} include/svx/dialogs.hrc include/svx/exthelpid.hrc include/svx/fmresids.hrc include/svx/svdstr.hrc include/svx/svxids.hrc include/svx/svxitems.hrc include/svx/ucsubset.hrc
${MYCMD} include/vcl/fpicker.hrc
${MYCMD} include/version.hrc
${MYCMD} reportdesign/inc/helpids.hrc reportdesign/inc/RptResId.hrc reportdesign/inc/rptui_slotid.hrc
${MYCMD} reportdesign/source/core/inc/core_resource.hrc
${MYCMD} reportdesign/source/inc/corestrings.hrc reportdesign/source/inc/stringconstants.hrc reportdesign/source/inc/uistrings.hrc reportdesign/source/inc/xmlstrings.hrc
${MYCMD} reportdesign/source/ui/dlg/CondFormat.hrc
${MYCMD} sc/inc/globstr.hrc sc/inc/sc.hrc sc/inc/scfuncs.hrc
${MYCMD} sc/sdi/scslots.hrc
${MYCMD} sc/source/ui/dbgui/asciiopt.hrc sc/source/ui/dbgui/pvfundlg.hrc
${MYCMD} sc/source/ui/formdlg/dwfunctr.hrc
${MYCMD} sc/source/ui/inc/acredlin.hrc sc/source/ui/inc/checklistmenu.hrc sc/source/ui/inc/condformatdlg.hrc sc/source/ui/inc/filter.hrc sc/source/ui/inc/iconsets.hrc sc/source/ui/inc/miscdlgs.hrc
${MYCMD} sc/source/ui/navipi/navipi.hrc
${MYCMD} sc/source/ui/pagedlg/pagedlg.hrc
${MYCMD} sc/source/ui/sidebar/CellAppearancePropertyPanel.hrc sc/source/ui/sidebar/NumberFormatPropertyPanel.hrc
${MYCMD} sc/source/ui/StatisticsDialogs/StatisticsDialogs.hrc
${MYCMD} scaddins/source/analysis/analysis.hrc
${MYCMD} scaddins/source/datefunc/datefunc.hrc
${MYCMD} scaddins/source/pricing/pricing.hrc
${MYCMD} sccomp/source/solver/solver.hrc
${MYCMD} sd/inc/app.hrc sd/inc/glob.hrc sd/inc/sdattr.hrc
${MYCMD} sd/sdi/sdslots.hrc
${MYCMD} sd/source/ui/accessibility/accessibility.hrc
${MYCMD} sd/source/ui/animations/CustomAnimation.hrc
${MYCMD} sd/source/ui/annotations/annotations.hrc
${MYCMD} sd/source/ui/inc/dialogs.hrc sd/source/ui/inc/navigatr.hrc sd/source/ui/inc/prltempl.hrc sd/source/ui/inc/res_bmp.hrc sd/source/ui/inc/sdstring.hrc sd/source/ui/inc/strings.hrc
${MYCMD} sd/source/ui/slideshow/slideshow.hrc
${MYCMD} sd/source/ui/slidesorter/inc/view/SlsResource.hrc
${MYCMD} sd/source/ui/view/DocumentRenderer.hrc
${MYCMD} sdext/source/minimizer/optimizerdialog.hrc
${MYCMD} sfx2/inc/filedlghelper.hrc
${MYCMD} sfx2/source/appl/app.hrc sfx2/source/appl/newhelp.hrc
${MYCMD} sfx2/source/bastyp/bastyp.hrc
${MYCMD} sfx2/source/control/templateview.hrc
${MYCMD} sfx2/source/dialog/dialog.hrc sfx2/source/dialog/dinfdlg.hrc
${MYCMD} sfx2/source/doc/doc.hrc
${MYCMD} sfx2/source/inc/helpid.hrc sfx2/source/inc/sfxlocal.hrc
${MYCMD} sfx2/source/view/view.hrc
${MYCMD} starmath/inc/starmath.hrc
${MYCMD} svtools/source/brwbox/editbrowsebox.hrc
${MYCMD} svtools/source/contnr/fileview.hrc svtools/source/contnr/templwin.hrc
${MYCMD} svtools/source/dialogs/addresstemplate.hrc
${MYCMD} svtools/source/inc/filectrl.hrc
${MYCMD} svx/inc/accessibility.hrc svx/inc/float3d.hrc svx/inc/fmhelp.hrc svx/inc/gallery.hrc svx/inc/galtheme.hrc svx/inc/helpid.hrc
${MYCMD} svx/sdi/svxslots.hrc
${MYCMD} svx/source/dialog/bmpmask.hrc svx/source/dialog/compressgraphicdialog.hrc svx/source/dialog/docrecovery.hrc svx/source/dialog/imapdlg.hrc svx/source/dialog/ruler.hrc
${MYCMD} svx/source/inc/datanavi.hrc svx/source/inc/fmprop.hrc svx/source/inc/frmsel.hrc
${MYCMD} svx/source/sidebar/area/AreaPropertyPanel.hrc
${MYCMD} svx/source/sidebar/EmptyPanel.hrc
${MYCMD} svx/source/sidebar/line/LinePropertyPanel.hrc
${MYCMD} svx/source/sidebar/text/TextPropertyPanel.hrc
${MYCMD} svx/source/tbxctrls/extrusioncontrols.hrc
${MYCMD} sw/inc/access.hrc sw/inc/chrdlg.hrc sw/inc/comcore.hrc sw/inc/dbui.hrc sw/inc/dialog.hrc sw/inc/dochdl.hrc sw/inc/envelp.hrc sw/inc/fldui.hrc sw/inc/globals.hrc sw/inc/index.hrc sw/inc/poolfmt.hrc sw/inc/rcid.hrc sw/inc/statstr.hrc sw/inc/unocore.hrc
${MYCMD} sw/sdi/swslots.hrc
${MYCMD} sw/source/core/inc/pagefrm.hrc
${MYCMD} sw/source/core/undo/undo.hrc
${MYCMD} sw/source/ui/config/config.hrc
${MYCMD} sw/source/ui/envelp/envprt.hrc sw/source/ui/envelp/labfmt.hrc
${MYCMD} sw/source/uibase/dialog/regionsw.hrc
${MYCMD} sw/source/uibase/docvw/annotation.hrc sw/source/uibase/docvw/docvw.hrc
${MYCMD} sw/source/uibase/envelp/label.hrc
${MYCMD} sw/source/uibase/inc/app.hrc sw/source/uibase/inc/frmui.hrc sw/source/uibase/inc/globdoc.hrc sw/source/uibase/inc/misc.hrc sw/source/uibase/inc/popup.hrc sw/source/uibase/inc/ribbar.hrc sw/source/uibase/inc/shells.hrc sw/source/uibase/inc/table.hrc sw/source/uibase/inc/utlui.hrc sw/source/uibase/inc/web.hrc
${MYCMD} sw/source/uibase/lingu/olmenu.hrc
${MYCMD} sw/source/uibase/misc/redlndlg.hrc
${MYCMD} sw/source/uibase/ribbar/inputwin.hrc sw/source/uibase/ribbar/workctrl.hrc
${MYCMD} sw/source/uibase/sidebar/PagePropertyPanel.hrc sw/source/uibase/sidebar/PropertyPanel.hrc
${MYCMD} sw/source/uibase/uiview/view.hrc
${MYCMD} sw/source/uibase/utlui/attrdesc.hrc sw/source/uibase/utlui/initui.hrc sw/source/uibase/utlui/navipi.hrc sw/source/uibase/utlui/unotools.hrc
${MYCMD} sw/source/uibase/wrtsh/wrtsh.hrc
${MYCMD} uui/source/ids.hrc
${MYCMD} vcl/inc/svids.hrc vcl/inc/win/salids.hrc
${MYCMD} vcl/source/edit/textundo.hrc
${MYCMD} writerperfect/inc/strings.hrc
${MYCMD} xmlsecurity/inc/xmlsecurity/global.hrc
${MYCMD} xmlsecurity/source/dialogs/dialogs.hrc xmlsecurity/source/dialogs/digitalsignaturesdialog.hrc xmlsecurity/source/dialogs/helpids.hrc




    285 filelist_hrc.txt
    238 filelist_pot.txt
    640 filelist_properties.txt
    718 filelist_src.txt
    916 filelist_xcu.txt
   2340 filelist_xhp.txt
  37963 filelist_po.txt
