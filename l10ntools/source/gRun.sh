#!/bin/bash

cd ${LODE_HOME}/dev/work

MYCMD='./workdir/LinkTarget/Executable/genlang.exe extract --target workdir/jan '

# .xrm files
${MYCMD} --base readlicense_oo/docs --files readme.xrm

# .tree and .xhp files
${MYCMD} --base swext/mediawiki/help --files help.tree wiki.xhp wikiaccount.xhp wikiformats.xhp wikisend.xhp wikisettings.xhp

${MYCMD} --base dictionaries/da_DK/help --files da/help.tree en/help.tree da/org.openoffice.da.hunspell.dictionaries/page1.xhp da/org.openoffice.da.hunspell.dictionaries/page2.xhp

${MYCMD} --base dictionaries/hu_HU/help --files en/help.tree hu/help.tree hu/org.openoffice.hu.hunspell.dictionaries/page1.xhp
exit

${MYCMD} --base helpcontent2/source/auxiliary --files sbasic.tree scalc.tree schart.tree shared.tree simpress.tree smath.tree swriter.tree

${MYCMD} --files nlpsolver/help/en/com.sun.star.comp.Calc.NLPSolver/help.tree

${MYCMD} --base helpcontent2/source/text/sbasic/guide --files access2base.xhp control_properties.xhp create_dialog.xhp insert_control.xhp sample_code.xhp show_dialog.xhp translation.xhp

${MYCMD} --base helpcontent2/source/text/sbasic/shared --files 00000002.xhp 00000003.xhp 01000000.xhp 01010210.xhp 01020000.xhp 01020100.xhp 01020200.xhp 01020300.xhp 01020500.xhp 01030000.xhp 01030100.xhp 01030200.xhp 01030300.xhp 01030400.xhp 01040000.xhp 01050000.xhp 01050100.xhp 01050200.xhp 01050300.xhp 01170100.xhp 01170101.xhp 01170103.xhp 03000000.xhp 03010000.xhp 03010100.xhp 03010101.xhp 03010102.xhp 03010103.xhp 03010200.xhp 03010201.xhp 03010300.xhp 03010301.xhp 03010302.xhp 03010303.xhp 03010304.xhp 03010305.xhp 03020000.xhp 03020100.xhp 03020101.xhp 03020102.xhp 03020103.xhp 03020104.xhp 03020200.xhp 03020201.xhp 03020202.xhp 03020203.xhp 03020204.xhp 03020205.xhp 03020301.xhp 03020302.xhp 03020303.xhp 03020304.xhp 03020305.xhp 03020400.xhp 03020401.xhp 03020402.xhp 03020403.xhp 03020404.xhp 03020405.xhp 03020406.xhp 03020407.xhp 03020408.xhp 03020409.xhp 03020410.xhp 03020411.xhp 03020412.xhp 03020413.xhp 03020414.xhp 03020415.xhp 03030000.xhp 03030100.xhp 03030101.xhp 03030102.xhp 03030103.xhp 03030104.xhp 03030105.xhp 03030106.xhp 03030107.xhp 03030108.xhp 03030110.xhp 03030111.xhp 03030112.xhp 03030113.xhp 03030114.xhp 03030115.xhp 03030116.xhp 03030120.xhp 03030130.xhp 03030200.xhp 03030201.xhp 03030202.xhp 03030203.xhp 03030204.xhp 03030205.xhp 03030206.xhp 03030300.xhp 03030301.xhp 03030302.xhp 03030303.xhp 03050000.xhp 03050100.xhp 03050200.xhp 03050300.xhp 03050500.xhp 03060000.xhp 03060100.xhp 03060200.xhp 03060300.xhp 03060400.xhp 03060500.xhp 03060600.xhp 03070000.xhp 03070100.xhp 03070200.xhp 03070300.xhp 03070400.xhp 03070500.xhp 03070600.xhp 03080000.xhp 03080100.xhp 03080101.xhp 03080102.xhp 03080103.xhp 03080104.xhp 03080200.xhp 03080201.xhp 03080202.xhp 03080300.xhp 03080301.xhp 03080302.xhp 03080400.xhp 03080401.xhp 03080500.xhp 03080501.xhp 03080502.xhp 03080600.xhp 03080601.xhp 03080700.xhp 03080701.xhp 03080800.xhp 03080801.xhp 03080802.xhp 03090000.xhp 03090100.xhp 03090101.xhp 03090102.xhp 03090103.xhp 03090200.xhp 03090201.xhp 03090202.xhp 03090203.xhp 03090300.xhp 03090301.xhp 03090302.xhp 03090303.xhp 03090400.xhp 03090401.xhp 03090402.xhp 03090403.xhp 03090404.xhp 03090405.xhp 03090406.xhp 03090407.xhp 03090408.xhp 03090409.xhp 03090410.xhp 03090411.xhp 03090412.xhp 03100000.xhp 03100050.xhp 03100060.xhp 03100070.xhp 03100080.xhp 03100100.xhp 03100300.xhp 03100400.xhp 03100500.xhp 03100600.xhp 03100700.xhp 03100900.xhp 03101000.xhp 03101100.xhp 03101110.xhp 03101120.xhp 03101130.xhp 03101140.xhp 03101300.xhp 03101400.xhp 03101500.xhp 03101600.xhp 03101700.xhp 03102000.xhp 03102100.xhp 03102101.xhp 03102200.xhp 03102300.xhp 03102400.xhp 03102450.xhp 03102600.xhp 03102700.xhp 03102800.xhp 03102900.xhp 03103000.xhp 03103100.xhp 03103200.xhp 03103300.xhp 03103400.xhp 03103450.xhp 03103500.xhp 03103600.xhp 03103700.xhp 03103800.xhp 03103900.xhp 03104000.xhp 03104100.xhp 03104200.xhp 03104300.xhp 03104400.xhp 03104500.xhp 03104600.xhp 03104700.xhp 03110000.xhp 03110100.xhp 03120000.xhp 03120100.xhp 03120101.xhp 03120102.xhp 03120103.xhp 03120104.xhp 03120105.xhp 03120200.xhp 03120201.xhp 03120202.xhp 03120300.xhp 03120301.xhp 03120302.xhp 03120303.xhp 03120304.xhp 03120305.xhp 03120306.xhp 03120307.xhp 03120308.xhp 03120309.xhp 03120310.xhp 03120311.xhp 03120312.xhp 03120313.xhp 03120314.xhp 03120315.xhp 03120400.xhp 03120401.xhp 03120402.xhp 03120403.xhp 03130000.xhp 03130100.xhp 03130500.xhp 03130600.xhp 03130700.xhp 03130800.xhp 03131000.xhp 03131300.xhp 03131400.xhp 03131500.xhp 03131600.xhp 03131700.xhp 03131800.xhp 03131900.xhp 03132000.xhp 03132100.xhp 03132200.xhp 03132300.xhp 03132400.xhp 03132500.xhp 05060700.xhp code-stubs.xhp keys.xhp main0211.xhp main0601.xhp

${MYCMD} --base helpcontent2/source/text/sbasic/shared/01 --files 06130000.xhp 06130100.xhp 06130500.xhp

${MYCMD} --base helpcontent2/source/text/sbasic/shared/02 --file 11010000.xhp 11020000.xhp 11030000.xhp 11040000.xhp 11050000.xhp 11060000.xhp 11070000.xhp 11080000.xhp 11090000.xhp 11100000.xhp 11110000.xhp 11120000.xhp 11140000.xhp 11150000.xhp 11160000.xhp 11170000.xhp 11180000.xhp 11190000.xhp 20000000.xhp

${MYCMD} --base helpcontent2/source/text/scalc/00 --files 00000004.xhp 00000402.xhp 00000403.xhp 00000404.xhp 00000405.xhp 00000406.xhp 00000407.xhp 00000412.xhp sheet_menu.xhp

${MYCMD} --base helpcontent2/source/text/scalc/01 --files 01120000.xhp 02110000.xhp 02120000.xhp 02120100.xhp 02140000.xhp 02140100.xhp 02140200.xhp 02140300.xhp 02140400.xhp 02140500.xhp 02140600.xhp 02140700.xhp 02150000.xhp 02160000.xhp 02170000.xhp 02180000.xhp 02190000.xhp 02190100.xhp 02190200.xhp 02200000.xhp 02210000.xhp 03070000.xhp 03080000.xhp 03090000.xhp 03100000.xhp 04010000.xhp 04010100.xhp 04010200.xhp 04020000.xhp 04030000.xhp 04040000.xhp 04050000.xhp 04050100.xhp 04060000.xhp 04060100.xhp 04060101.xhp 04060102.xhp 04060103.xhp 04060104.xhp 04060105.xhp 04060106.xhp 04060107.xhp 04060108.xhp 04060109.xhp 04060110.xhp 04060111.xhp 04060112.xhp 04060115.xhp 04060116.xhp 04060118.xhp 04060119.xhp 04060120.xhp 04060181.xhp 04060182.xhp 04060183.xhp 04060184.xhp 04060185.xhp 04060199.xhp 04070000.xhp 04070100.xhp 04070200.xhp 04070300.xhp 04070400.xhp 04080000.xhp 04090000.xhp 05020000.xhp 05020600.xhp 05030000.xhp 05030200.xhp 05030300.xhp 05030400.xhp 05040000.xhp 05040200.xhp 05050000.xhp 05050100.xhp 05050300.xhp 05060000.xhp 05070000.xhp 05070500.xhp 05080000.xhp 05080100.xhp 05080200.xhp 05080300.xhp 05080400.xhp 05100000.xhp 05100100.xhp 05100200.xhp 05110000.xhp 05120000.xhp 06020000.xhp 06030000.xhp 06030100.xhp 06030200.xhp 06030300.xhp 06030400.xhp 06030500.xhp 06030600.xhp 06030700.xhp 06030800.xhp 06030900.xhp 06031000.xhp 06040000.xhp 06050000.xhp 06060000.xhp 06060100.xhp 06060200.xhp 06070000.xhp 06080000.xhp 06130000.xhp 06990000.xhp 07080000.xhp 07090000.xhp 12010000.xhp 12010100.xhp 12020000.xhp 12030000.xhp 12030100.xhp 12030200.xhp 12040000.xhp 12040100.xhp 12040201.xhp 12040300.xhp 12040400.xhp 12040500.xhp 12050000.xhp 12050100.xhp 12050200.xhp 12060000.xhp 12070000.xhp 12070100.xhp 12080000.xhp 12080100.xhp 12080200.xhp 12080300.xhp 12080400.xhp 12080500.xhp 12080600.xhp 12080700.xhp 12090000.xhp 12090100.xhp 12090101.xhp 12090102.xhp 12090103.xhp 12090104.xhp 12090105.xhp 12090106.xhp 12090200.xhp 12090300.xhp 12090400.xhp 12100000.xhp 12120000.xhp 12120100.xhp 12120200.xhp 12120300.xhp exponsmooth_embd.xhp ex_data_stat_func.xhp format_graphic.xhp ful_func.xhp func_aggregate.xhp func_averageif.xhp func_averageifs.xhp func_color.xhp func_countifs.xhp func_date.xhp func_datedif.xhp func_datevalue.xhp func_day.xhp func_days.xhp func_days360.xhp func_eastersunday.xhp func_edate.xhp func_eomonth.xhp func_error_type.xhp func_forecastetsadd.xhp func_forecastetsmult.xhp func_forecastetspiadd.xhp func_forecastetspimult.xhp func_forecastetsseason.xhp func_forecastetsstatadd.xhp func_forecastetsstatmult.xhp func_hour.xhp func_imcos.xhp func_imcosh.xhp func_imcot.xhp func_imcsc.xhp func_imcsch.xhp func_imsec.xhp func_imsech.xhp func_imsin.xhp func_imsinh.xhp func_imtan.xhp func_isoweeknum.xhp func_minute.xhp func_month.xhp func_networkdays.xhp func_now.xhp func_numbervalue.xhp func_second.xhp func_skewp.xhp func_sumifs.xhp func_time.xhp func_timevalue.xhp func_today.xhp func_webservice.xhp func_weekday.xhp func_weeknum.xhp func_weeknumadd.xhp func_weeknum_ooo.xhp func_workday.xhp func_year.xhp func_yearfrac.xhp ODFF.xhp solver.xhp solver_options.xhp statistics.xhp statistics_regression.xhp stat_data.xhp text2columns.xhp

${MYCMD} --base helpcontent2/source/text/scalc/02 --files 02130000.xhp 02140000.xhp 02150000.xhp 02160000.xhp 02170000.xhp 06010000.xhp 06030000.xhp 06040000.xhp 06050000.xhp 06060000.xhp 06070000.xhp 06080000.xhp 08010000.xhp 08080000.xhp 10050000.xhp 10060000.xhp 18010000.xhp 18020000.xhp

${MYCMD} --files helpcontent2/source/text/scalc/04/01020000.xhp

${MYCMD} --base helpcontent2/source/text/scalc/05 --files 02140000.xhp empty_cells.xhp OpenCL_options.xhp

${MYCMD} --base helpcontent2/source/text/scalc/guide --files address_auto.xhp autofilter.xhp autoformat.xhp auto_off.xhp background.xhp borders.xhp calculate.xhp calc_date.xhp calc_series.xhp calc_timevalues.xhp cellcopy.xhp cellreferences.xhp cellreferences_url.xhp cellreference_dragdrop.xhp cellstyle_by_formula.xhp cellstyle_conditional.xhp cellstyle_minusvalue.xhp cell_enter.xhp cell_protect.xhp cell_unprotect.xhp consolidate.xhp csv_files.xhp csv_formula.xhp currency_format.xhp database_define.xhp database_filter.xhp database_sort.xhp datapilot.xhp datapilot_createtable.xhp datapilot_deletetable.xhp datapilot_edittable.xhp datapilot_filtertable.xhp datapilot_grouping.xhp datapilot_tipps.xhp datapilot_updatetable.xhp dbase_files.xhp design.xhp edit_multitables.xhp filters.xhp finding.xhp format_table.xhp format_value.xhp format_value_userdef.xhp formulas.xhp formula_copy.xhp formula_enter.xhp formula_value.xhp fraction_enter.xhp goalseek.xhp html_doc.xhp integer_leading_zero.xhp keyboard.xhp line_fix.xhp main.xhp mark_cells.xhp matrixformula.xhp move_dragdrop.xhp multioperation.xhp multitables.xhp multi_tables.xhp note_insert.xhp numbers_text.xhp printranges.xhp print_details.xhp print_exact.xhp print_landscape.xhp print_title_row.xhp relativ_absolut_ref.xhp rename_table.xhp rounding_numbers.xhp row_height.xhp scenario.xhp sorted_list.xhp specialfilter.xhp super_subscript.xhp table_cellmerge.xhp table_rotate.xhp table_view.xhp text_numbers.xhp text_rotate.xhp text_wrap.xhp userdefined_function.xhp validity.xhp value_with_name.xhp webquery.xhp year2000.xhp

${MYCMD} --base helpcontent2/source/text/scalc --files main0000.xhp main0100.xhp main0101.xhp main0102.xhp main0103.xhp main0104.xhp main0105.xhp main0106.xhp main0107.xhp main0112.xhp main0116.xhp main0200.xhp main0202.xhp main0203.xhp main0205.xhp main0206.xhp main0208.xhp main0210.xhp main0214.xhp main0218.xhp main0503.xhp

${MYCMD} --files helpcontent2/source/text/schart/00/00000004.xhp

${MYCMD} --base helpcontent2/source/text/schart/01 --files 03010000.xhp 04010000.xhp 04020000.xhp 04030000.xhp 04040000.xhp 04050000.xhp 04050100.xhp 04060000.xhp 04070000.xhp 05010000.xhp 05010100.xhp 05010200.xhp 05020000.xhp 05020100.xhp 05020101.xhp 05020200.xhp 05020201.xhp 05030000.xhp 05040000.xhp 05040100.xhp 05040200.xhp 05040201.xhp 05040202.xhp 05050000.xhp 05050100.xhp 05060000.xhp 05070000.xhp 05080000.xhp 05120000.xhp choose_chart_type.xhp smooth_line_properties.xhp stepped_line_properties.xhp three_d_view.xhp type_area.xhp type_bubble.xhp type_column_bar.xhp type_column_line.xhp type_line.xhp type_net.xhp type_pie.xhp type_stock.xhp type_xy.xhp wiz_chart_elements.xhp wiz_chart_type.xhp wiz_data_range.xhp wiz_data_series.xhp

${MYCMD} --base helpcontent2/source/text/schart/02 --files 01190000.xhp 01200000.xhp 01210000.xhp 01220000.xhp 02020000.xhp

${MYCMD} --files helpcontent2/source/text/schart/04/01020000.xhp

${MYCMD} --base helpcontent2/source/text/schart --files main0000.xhp main0202.xhp main0503.xhp

${MYCMD} --files helpcontent2/source/text/sdraw/00/00000004.xhp

${MYCMD} --files helpcontent2/source/text/sdraw/01/04010000.xhp

${MYCMD} --files helpcontent2/source/text/sdraw/04/01020000.xhp

${MYCMD} --base helpcontent2/source/text/sdraw/guide --files align_arrange.xhp color_define.xhp combine_etc.xhp cross_fading.xhp draw_sector.xhp duplicate_object.xhp eyedropper.xhp gradient.xhp graphic_insert.xhp groups.xhp join_objects.xhp join_objects3d.xhp keyboard.xhp main.xhp rotate_object.xhp text_enter.xhp

${MYCMD} --base helpcontent2/source/text/sdraw --files main0000.xhp main0100.xhp main0101.xhp main0102.xhp main0103.xhp main0104.xhp main0105.xhp main0106.xhp main0200.xhp main0202.xhp main0210.xhp main0213.xhp main0503.xhp

${MYCMD} --base helpcontent2/source/text/shared/00 --files 00000001.xhp 00000002.xhp 00000003.xhp 00000004.xhp 00000005.xhp 00000007.xhp 00000010.xhp 00000011.xhp 00000020.xhp 00000021.xhp 00000099.xhp 00000200.xhp 00000206.xhp 00000207.xhp 00000208.xhp 00000210.xhp 00000215.xhp 00000401.xhp 00000402.xhp 00000403.xhp 00000404.xhp 00000406.xhp 00000407.xhp 00000408.xhp 00000409.xhp 00000450.xhp 00040500.xhp 00040501.xhp 00040502.xhp 00040503.xhp 01000000.xhp 01010000.xhp 01020000.xhp 01050000.xhp icon_alt.xhp

${MYCMD} --base helpcontent2/source/text/shared/01 --files 01010000.xhp 01010001.xhp 01010100.xhp 01010200.xhp 01010201.xhp 01010202.xhp 01010203.xhp 01010300.xhp 01010301.xhp 01010302.xhp 01010303.xhp 01010304.xhp 01020000.xhp 01020101.xhp 01020103.xhp 01050000.xhp 01060000.xhp 01070000.xhp 01070001.xhp 01100000.xhp 01100100.xhp 01100200.xhp 01100300.xhp 01100400.xhp 01100600.xhp 01110000.xhp 01110101.xhp 01110300.xhp 01110400.xhp 01130000.xhp 01140000.xhp 01160000.xhp 01160200.xhp 01160300.xhp 01170000.xhp 01180000.xhp 01190000.xhp 01990000.xhp 02010000.xhp 02020000.xhp 02030000.xhp 02040000.xhp 02050000.xhp 02060000.xhp 02070000.xhp 02090000.xhp 02100000.xhp 02100001.xhp 02100100.xhp 02100200.xhp 02100300.xhp 02110000.xhp 02180000.xhp 02180100.xhp 02190000.xhp 02200000.xhp 02200100.xhp 02200200.xhp 02210101.xhp 02220000.xhp 02220100.xhp 02230000.xhp 02230100.xhp 02230150.xhp 02230200.xhp 02230300.xhp 02230400.xhp 02230401.xhp 02230402.xhp 02230500.xhp 02240000.xhp 02250000.xhp 03010000.xhp 03020000.xhp 03040000.xhp 03050000.xhp 03060000.xhp 03110000.xhp 03150100.xhp 03170000.xhp 03990000.xhp 04050000.xhp 04060000.xhp 04060100.xhp 04060200.xhp 04100000.xhp 04140000.xhp 04150000.xhp 04150100.xhp 04150200.xhp 04150400.xhp 04150500.xhp 04160300.xhp 04160500.xhp 04180100.xhp 04990000.xhp 05010000.xhp 05020000.xhp 05020100.xhp 05020200.xhp 05020300.xhp 05020301.xhp 05020400.xhp 05020500.xhp 05020600.xhp 05020700.xhp 05030000.xhp 05030100.xhp 05030300.xhp 05030500.xhp 05030600.xhp 05030700.xhp 05030800.xhp 05040100.xhp 05040200.xhp 05040300.xhp 05040400.xhp 05050000.xhp 05060000.xhp 05070000.xhp 05070100.xhp 05070200.xhp 05070300.xhp 05070400.xhp 05070500.xhp 05070600.xhp 05080000.xhp 05080100.xhp 05080200.xhp 05080300.xhp 05080400.xhp 05090000.xhp 05100000.xhp 05100100.xhp 05100200.xhp 05100500.xhp 05100600.xhp 05100700.xhp 05110000.xhp 05110100.xhp 05110200.xhp 05110300.xhp 05110400.xhp 05110500.xhp 05110600m.xhp 05110700.xhp 05110800.xhp 05120000.xhp 05120100.xhp 05120200.xhp 05120300.xhp 05120600.xhp 05140100.xhp 05150101.xhp 05190000.xhp 05190100.xhp 05200000.xhp 05200100.xhp 05200200.xhp 05200300.xhp 05210000.xhp 05210100.xhp 05210200.xhp 05210300.xhp 05210400.xhp 05210500.xhp 05210600.xhp 05210700.xhp 05220000.xhp 05230000.xhp 05230100.xhp 05230300.xhp 05230400.xhp 05230500.xhp 05240000.xhp 05240100.xhp 05240200.xhp 05250000.xhp 05250100.xhp 05250200.xhp 05250300.xhp 05250400.xhp 05250500.xhp 05250600.xhp 05260000.xhp 05260100.xhp 05260200.xhp 05260300.xhp 05260400.xhp 05260500.xhp 05260600.xhp 05270000.xhp 05280000.xhp 05290000.xhp 05290100.xhp 05290200.xhp 05290300.xhp 05290400.xhp 05320000.xhp 05340100.xhp 05340200.xhp 05340300.xhp 05340400.xhp 05340402.xhp 05340404.xhp 05340405.xhp 05340500.xhp 05340600.xhp 05350000.xhp 05350200.xhp 05350300.xhp 05350400.xhp 05350500.xhp 05350600.xhp 05360000.xhp 05990000.xhp 06010000.xhp 06010101.xhp 06010500.xhp 06010600.xhp 06010601.xhp 06020000.xhp 06030000.xhp 06040000.xhp 06040100.xhp 06040200.xhp 06040300.xhp 06040400.xhp 06040500.xhp 06040600.xhp 06040700.xhp 06050000.xhp 06050100.xhp 06050200.xhp 06050300.xhp 06050400.xhp 06050500.xhp 06050600.xhp 06130000.xhp 06130001.xhp 06130010.xhp 06130100.xhp 06130200.xhp 06130500.xhp 06140000.xhp 06140100.xhp 06140101.xhp 06140102.xhp 06140200.xhp 06140400.xhp 06140402.xhp 06140500.xhp 06150000.xhp 06150100.xhp 06150110.xhp 06150120.xhp 06150200.xhp 06150210.xhp 06200000.xhp 06201000.xhp 06202000.xhp 06990000.xhp 07010000.xhp 07080000.xhp about_meta_tags.xhp digitalsignatures.xhp extensionupdate.xhp formatting_mark.xhp gallery.xhp gallery_files.xhp grid.xhp guides.xhp mediaplayer.xhp menu_edit_find.xhp menu_view_sidebar.xhp moviesound.xhp online_update.xhp online_update_dialog.xhp packagemanager.xhp password_dlg.xhp password_main.xhp prop_font_embed.xhp ref_pdf_export.xhp ref_pdf_send_as.xhp securitywarning.xhp selectcertificate.xhp webhtml.xhp xformsdata.xhp xformsdataadd.xhp xformsdataaddcon.xhp xformsdatachange.xhp xformsdataname.xhp xformsdatatab.xhp

${MYCMD} --base helpcontent2/source/text/shared/02 --files 01110000.xhp 01140000.xhp 01170000.xhp 01170001.xhp 01170002.xhp 01170003.xhp 01170004.xhp 01170100.xhp 01170101.xhp 01170102.xhp 01170103.xhp 01170200.xhp 01170201.xhp 01170202.xhp 01170203.xhp 01170300.xhp 01170400.xhp 01170500.xhp 01170600.xhp 01170700.xhp 01170800.xhp 01170801.xhp 01170802.xhp 01170900.xhp 01170901.xhp 01170902.xhp 01170903.xhp 01170904.xhp 01171000.xhp 01171100.xhp 01171200.xhp 01171300.xhp 01171400.xhp 01220000.xhp 01230000.xhp 02010000.xhp 02020000.xhp 02030000.xhp 02040000.xhp 02050000.xhp 02130000.xhp 02140000.xhp 02160000.xhp 02170000.xhp 03110000.xhp 03120000.xhp 03130000.xhp 03140000.xhp 03150000.xhp 03200000.xhp 04210000.xhp 05020000.xhp 05090000.xhp 05110000.xhp 06050000.xhp 06060000.xhp 06100000.xhp 06110000.xhp 06120000.xhp 07010000.xhp 07060000.xhp 07070000.xhp 07070100.xhp 07070200.xhp 07080000.xhp 07090000.xhp 08010000.xhp 08020000.xhp 09070000.xhp 09070100.xhp 09070200.xhp 09070300.xhp 09070400.xhp 10010000.xhp 10020000.xhp 10030000.xhp 10040000.xhp 10100000.xhp 12000000.xhp 12010000.xhp 12020000.xhp 12030000.xhp 12040000.xhp 12050000.xhp 12070000.xhp 12070100.xhp 12070200.xhp 12070300.xhp 12080000.xhp 12090000.xhp 12090100.xhp 12090101.xhp 12100000.xhp 12100100.xhp 12100200.xhp 12110000.xhp 12120000.xhp 12130000.xhp 12140000.xhp 13010000.xhp 13020000.xhp 14010000.xhp 14020000.xhp 14020100.xhp 14020200.xhp 14030000.xhp 14040000.xhp 14050000.xhp 14060000.xhp 14070000.xhp 18010000.xhp 18030000.xhp 19090000.xhp 20020000.xhp 20030000.xhp 20040000.xhp 20050000.xhp 20060000.xhp 20090000.xhp 20100000.xhp 24010000.xhp 24020000.xhp 24030000.xhp 24040000.xhp 24050000.xhp 24060000.xhp 24070000.xhp 24080000.xhp 24090000.xhp 24100000.xhp basicshapes.xhp blockarrows.xhp callouts.xhp colortoolbar.xhp flowcharts.xhp fontwork.xhp limit.xhp more_controls.xhp paintbrush.xhp querypropdlg.xhp stars.xhp symbolshapes.xhp

${MYCMD} --base helpcontent2/source/text/shared/04 --files 01010000.xhp 01020000.xhp

${MYCMD} --base helpcontent2/source/text/shared/05 --files 00000001.xhp 00000002.xhp 00000100.xhp 00000110.xhp 00000120.xhp 00000130.xhp 00000140.xhp 00000150.xhp 00000160.xhp err_html.xhp

${MYCMD} --files helpcontent2/source/text/shared/07/09000000.xhp

${MYCMD} --files helpcontent2/source/text/shared/3dsettings_toolbar.xhp

${MYCMD} --base helpcontent2/source/text/shared/autokorr --files 01000000.xhp 02000000.xhp 03000000.xhp 04000000.xhp 05000000.xhp 06000000.xhp 07000000.xhp 08000000.xhp 09000000.xhp 10000000.xhp 12000000.xhp 13000000.xhp

${MYCMD} --base helpcontent2/source/text/shared/autopi --files 01000000.xhp 01010000.xhp 01010100.xhp 01010200.xhp 01010300.xhp 01010400.xhp 01010500.xhp 01010600.xhp 01020000.xhp 01020100.xhp 01020200.xhp 01020300.xhp 01020400.xhp 01020500.xhp 01040000.xhp 01040100.xhp 01040200.xhp 01040300.xhp 01040400.xhp 01040500.xhp 01040600.xhp 01050000.xhp 01050100.xhp 01050200.xhp 01050300.xhp 01050400.xhp 01050500.xhp 01090000.xhp 01090100.xhp 01090200.xhp 01090210.xhp 01090220.xhp 01090300.xhp 01090400.xhp 01090500.xhp 01090600.xhp 01100000.xhp 01100100.xhp 01100150.xhp 01100200.xhp 01100300.xhp 01100400.xhp 01100500.xhp 01110000.xhp 01110100.xhp 01110200.xhp 01110300.xhp 01110400.xhp 01110500.xhp 01110600.xhp 01120000.xhp 01120100.xhp 01120200.xhp 01120300.xhp 01120400.xhp 01120500.xhp 01130000.xhp 01130100.xhp 01130200.xhp 01150000.xhp 01170000.xhp 01170200.xhp 01170300.xhp 01170400.xhp 01170500.xhp webwizard00.xhp webwizard01.xhp webwizard02.xhp webwizard03.xhp webwizard04.xhp webwizard05.xhp webwizard05bi.xhp webwizard05is.xhp webwizard06.xhp webwizard07.xhp webwizard07fc.xhp

${MYCMD} --base helpcontent2/source/text/shared/explorer/database --files 02000000.xhp 02000002.xhp 02010100.xhp 02010101.xhp 04000000.xhp 04030000.xhp 05000000.xhp 05000001.xhp 05000002.xhp 05000003.xhp 05010000.xhp 05010100.xhp 05020000.xhp 05020100.xhp 05030000.xhp 05030100.xhp 05030200.xhp 05030300.xhp 05030400.xhp 05040000.xhp 05040100.xhp 05040200.xhp 11000002.xhp 11020000.xhp 11030000.xhp 11030100.xhp 11080000.xhp 11090000.xhp 11150200.xhp 11170000.xhp 11170100.xhp 30000000.xhp 30100000.xhp dabaadvprop.xhp dabaadvpropdat.xhp dabaadvpropgen.xhp dabadoc.xhp dabaprop.xhp dabapropadd.xhp dabapropcon.xhp dabapropgen.xhp dabawiz00.xhp dabawiz01.xhp dabawiz02.xhp dabawiz02access.xhp dabawiz02adabas.xhp dabawiz02ado.xhp dabawiz02dbase.xhp dabawiz02jdbc.xhp dabawiz02ldap.xhp dabawiz02mysql.xhp dabawiz02odbc.xhp dabawiz02oracle.xhp dabawiz02spreadsheet.xhp dabawiz02text.xhp dabawiz03auth.xhp main.xhp menubar.xhp menuedit.xhp menufile.xhp menufilesave.xhp menuinsert.xhp menutools.xhp menuview.xhp migrate_macros.xhp password.xhp querywizard00.xhp querywizard01.xhp querywizard02.xhp querywizard03.xhp querywizard04.xhp querywizard05.xhp querywizard06.xhp querywizard07.xhp querywizard08.xhp rep_datetime.xhp rep_main.xhp rep_navigator.xhp rep_pagenumbers.xhp rep_prop.xhp rep_sort.xhp tablewizard00.xhp tablewizard01.xhp tablewizard02.xhp tablewizard03.xhp tablewizard04.xhp toolbars.xhp

${MYCMD} --files helpcontent2/source/text/shared/fontwork_toolbar.xhp

${MYCMD} --base helpcontent2/source/text/shared/guide --files aaa_start.xhp accessibility.xhp activex.xhp active_help_on_off.xhp assistive.xhp autocorr_url.xhp autohide.xhp background.xhp border_paragraph.xhp border_table.xhp breaking_lines.xhp change_title.xhp chart_axis.xhp chart_barformat.xhp chart_insert.xhp chart_legend.xhp chart_title.xhp collab.xhp configure_overview.xhp contextmenu.xhp copytable2application.xhp copytext2application.xhp copy_drawfunctions.xhp ctl.xhp database_main.xhp data_addressbook.xhp data_dbase2office.xhp data_enter_sql.xhp data_forms.xhp data_im_export.xhp data_new.xhp data_queries.xhp data_register.xhp data_report.xhp data_reports.xhp data_search.xhp data_search2.xhp data_tabledefine.xhp data_tables.xhp data_view.xhp digitalsign_receive.xhp digitalsign_send.xhp digital_signatures.xhp doc_autosave.xhp doc_open.xhp doc_save.xhp dragdrop.xhp dragdrop_beamer.xhp dragdrop_fromgallery.xhp dragdrop_gallery.xhp dragdrop_graphic.xhp dragdrop_table.xhp edit_symbolbar.xhp email.xhp error_report.xhp export_ms.xhp fax.xhp filternavigator.xhp find_attributes.xhp flat_icons.xhp floating_toolbar.xhp fontwork.xhp formfields.xhp gallery_insert.xhp groups.xhp hyperlink_edit.xhp hyperlink_insert.xhp hyperlink_rel_abs.xhp imagemap.xhp import_ms.xhp insert_bitmap.xhp insert_graphic_drawit.xhp insert_specialchar.xhp integratinguno.xhp keyboard.xhp labels.xhp labels_database.xhp language_select.xhp lineend_define.xhp linestyles.xhp linestyle_define.xhp line_intext.xhp macro_recording.xhp main.xhp measurement_units.xhp microsoft_terms.xhp ms_doctypes.xhp ms_import_export_limitations.xhp ms_user.xhp navigator.xhp navigator_setcursor.xhp navpane_on.xhp numbering_stop.xhp pageformat_max.xhp paintbrush.xhp pasting.xhp print_blackwhite.xhp print_faster.xhp protection.xhp redlining.xhp redlining_accept.xhp redlining_doccompare.xhp redlining_docmerge.xhp redlining_enter.xhp redlining_navigation.xhp redlining_protect.xhp redlining_versions.xhp round_corner.xhp scripting.xhp space_hyphen.xhp spadmin.xhp standard_template.xhp startcenter.xhp start_parameters.xhp tabs.xhp textmode_change.xhp text_color.xhp undo_formatting.xhp version_number.xhp viewing_file_properties.xhp workfolder.xhp xforms.xhp xsltfilter.xhp xsltfilter_create.xhp xsltfilter_distribute.xhp

${MYCMD} --base helpcontent2/source/text/shared --files main0108.xhp main0201.xhp main0204.xhp main0208.xhp main0212.xhp main0213.xhp main0214.xhp main0226.xhp main0227.xhp main0400.xhp main0500.xhp main0600.xhp main0650.xhp main0800.xhp

${MYCMD} --base helpcontent2/source/text/shared/menu --files insert_chart.xhp insert_form_control.xhp insert_shape.xhp

${MYCMD} --files helpcontent2/source/text/shared/need_help.xhp

${MYCMD} --base helpcontent2/source/text/shared/optionen --files 01000000.xhp 01010000.xhp 01010100.xhp 01010200.xhp 01010300.xhp 01010301.xhp 01010400.xhp 01010401.xhp 01010500.xhp 01010501.xhp 01010600.xhp 01010700.xhp 01010800.xhp 01010900.xhp 01011000.xhp 01012000.xhp 01013000.xhp 01014000.xhp 01020000.xhp 01020100.xhp 01020300.xhp 01030000.xhp 01030300.xhp 01030500.xhp 01040000.xhp 01040200.xhp 01040300.xhp 01040301.xhp 01040400.xhp 01040500.xhp 01040600.xhp 01040700.xhp 01040900.xhp 01041000.xhp 01041100.xhp 01050000.xhp 01050100.xhp 01050300.xhp 01060000.xhp 01060100.xhp 01060300.xhp 01060400.xhp 01060401.xhp 01060500.xhp 01060600.xhp 01060700.xhp 01060800.xhp 01060900.xhp 01061000.xhp 01070000.xhp 01070100.xhp 01070300.xhp 01070400.xhp 01070500.xhp 01080000.xhp 01090000.xhp 01090100.xhp 01110000.xhp 01110100.xhp 01130100.xhp 01130200.xhp 01140000.xhp 01150000.xhp 01150100.xhp 01150200.xhp 01150300.xhp 01160000.xhp 01160100.xhp 01160200.xhp 01160201.xhp BasicIDE.xhp experimental.xhp expertconfig.xhp java.xhp javaclasspath.xhp javaparameters.xhp macrosecurity.xhp macrosecurity_sl.xhp macrosecurity_ts.xhp mailmerge.xhp online_update.xhp opencl.xhp persona_firefox.xhp serverauthentication.xhp testaccount.xhp viewcertificate.xhp viewcertificate_c.xhp viewcertificate_d.xhp viewcertificate_g.xhp

${MYCMD} --base helpcontent2/source/text/simpress/00 --files 00000004.xhp 00000401.xhp 00000402.xhp 00000403.xhp 00000404.xhp 00000405.xhp 00000406.xhp 00000407.xhp 00000413.xhp

${MYCMD} --base helpcontent2/source/text/simpress/01 --files 01170000.xhp 01180000.xhp 01180001.xhp 01180002.xhp 02110000.xhp 02120000.xhp 02130000.xhp 02140000.xhp 02150000.xhp 02160000.xhp 03050000.xhp 03060000.xhp 03070000.xhp 03080000.xhp 03090000.xhp 03100000.xhp 03110000.xhp 03120000.xhp 03130000.xhp 03150000.xhp 03150100.xhp 03150300.xhp 03151000.xhp 03151100.xhp 03151200.xhp 03152000.xhp 03180000.xhp 04010000.xhp 04020000.xhp 04030000.xhp 04030000m.xhp 04030100.xhp 04040000m.xhp 04080100.xhp 04110000.xhp 04110100.xhp 04110200.xhp 04120000.xhp 04130000.xhp 04140000.xhp 04990000.xhp 04990100.xhp 04990200.xhp 04990300.xhp 04990400.xhp 04990500.xhp 04990600.xhp 04990700.xhp 05090000m.xhp 05100000.xhp 05110500m.xhp 05120000.xhp 05120100.xhp 05120500m.xhp 05130000.xhp 05140000.xhp 05150000.xhp 05170000.xhp 05250000.xhp 05250500.xhp 05250600.xhp 05250700.xhp 06030000.xhp 06040000.xhp 06050000.xhp 06060000.xhp 06070000.xhp 06080000.xhp 06100000.xhp 06100100.xhp 13050000.xhp 13050100.xhp 13050200.xhp 13050300.xhp 13050400.xhp 13050500.xhp 13050600.xhp 13050700.xhp 13140000.xhp 13150000.xhp 13160000.xhp 13170000.xhp 13180000.xhp 13180100.xhp 13180200.xhp 13180300.xhp animationeffect.xhp effectoptions.xhp effectoptionseffect.xhp effectoptionstext.xhp effectoptionstiming.xhp slidesorter.xhp taskpanel.xhp

${MYCMD} --base helpcontent2/source/text/simpress/02 --files 04010000.xhp 04020000.xhp 04030000.xhp 04040000.xhp 04060000.xhp 04070000.xhp 08020000.xhp 08060000.xhp 10020000.xhp 10030000.xhp 10030200.xhp 10050000.xhp 10060000.xhp 10070000.xhp 10080000.xhp 10090000.xhp 10100000.xhp 10110000.xhp 10120000.xhp 10130000.xhp 11060000.xhp 11070000.xhp 11080000.xhp 11090000.xhp 11100000.xhp 11110000.xhp 13010000.xhp 13020000.xhp 13030000.xhp 13040000.xhp 13050000.xhp 13060000.xhp 13090000.xhp 13100000.xhp 13140000.xhp 13150000.xhp 13160000.xhp 13170000.xhp 13180000.xhp 13190000.xhp

${MYCMD} --files helpcontent2/source/text/simpress/04/01020000.xhp

${MYCMD} --base helpcontent2/source/text/simpress/guide --files 3d_create.xhp animated_gif_create.xhp animated_gif_save.xhp animated_objects.xhp animated_slidechange.xhp arrange_slides.xhp background.xhp change_scale.xhp footer.xhp gluepoints.xhp html_export.xhp html_import.xhp individual.xhp keyboard.xhp layers.xhp layer_move.xhp layer_new.xhp layer_tipps.xhp line_arrow_styles.xhp line_draw.xhp line_edit.xhp main.xhp masterpage.xhp move_object.xhp orgchart.xhp page_copy.xhp palette_files.xhp printing.xhp print_tofit.xhp rehearse_timings.xhp select_object.xhp show.xhp table_insert.xhp text2curve.xhp vectorize.xhp

${MYCMD} --base helpcontent2/source/text/simpress --files main0000.xhp main0100.xhp main0101.xhp main0102.xhp main0103.xhp main0104.xhp main0105.xhp main0106.xhp main0107.xhp main0113.xhp main0114.xhp main0117.xhp main0200.xhp main0202.xhp main0203.xhp main0204.xhp main0206.xhp main0209.xhp main0210.xhp main0211.xhp main0212.xhp main0213.xhp main0214.xhp main0503.xhp presenter.xhp

${MYCMD} --files helpcontent2/source/text/smath/00/00000004.xhp

${MYCMD} --base helpcontent2/source/text/smath/01 --files 02080000.xhp 02090000.xhp 02100000.xhp 02110000.xhp 03040000.xhp 03050000.xhp 03060000.xhp 03070000.xhp 03080000.xhp 03090000.xhp 03090100.xhp 03090200.xhp 03090300.xhp 03090400.xhp 03090500.xhp 03090600.xhp 03090700.xhp 03090800.xhp 03090900.xhp 03090901.xhp 03090902.xhp 03090903.xhp 03090904.xhp 03090905.xhp 03090906.xhp 03090907.xhp 03090908.xhp 03090909.xhp 03090910.xhp 03091100.xhp 03091200.xhp 03091300.xhp 03091400.xhp 03091500.xhp 03091501.xhp 03091502.xhp 03091503.xhp 03091504.xhp 03091505.xhp 03091506.xhp 03091507.xhp 03091508.xhp 03091509.xhp 03091600.xhp 05010000.xhp 05010100.xhp 05020000.xhp 05030000.xhp 05040000.xhp 05050000.xhp 06010000.xhp 06010100.xhp 06020000.xhp

${MYCMD} --files helpcontent2/source/text/smath/02/03010000.xhp

${MYCMD} --files helpcontent2/source/text/smath/04/01020000.xhp

${MYCMD} --base helpcontent2/source/text/smath/guide --files align.xhp attributes.xhp brackets.xhp comment.xhp keyboard.xhp limits.xhp main.xhp newline.xhp parentheses.xhp text.xhp

${MYCMD} --base helpcontent2/source/text/smath --files main0000.xhp main0100.xhp main0101.xhp main0102.xhp main0103.xhp main0105.xhp main0106.xhp main0107.xhp main0200.xhp main0202.xhp main0203.xhp main0503.xhp

${MYCMD} --base helpcontent2/source/text/swriter/00 --files 00000004.xhp 00000401.xhp 00000402.xhp 00000403.xhp 00000404.xhp 00000405.xhp 00000406.xhp

${MYCMD} --base helpcontent2/source/text/swriter/01 --files 01120000.xhp 01150000.xhp 01160100.xhp 01160200.xhp 01160300.xhp 01160400.xhp 01160500.xhp 02110000.xhp 02110100.xhp 02120000.xhp 02120100.xhp 02130000.xhp 02140000.xhp 02150000.xhp 02160000.xhp 02170000.xhp 03050000.xhp 03070000.xhp 03080000.xhp 03090000.xhp 03100000.xhp 03120000.xhp 03130000.xhp 03140000.xhp 04010000.xhp 04020000.xhp 04020100.xhp 04020200.xhp 04030000.xhp 04040000.xhp 04060000.xhp 04060100.xhp 04070000.xhp 04070100.xhp 04070200.xhp 04070300.xhp 04090000.xhp 04090001.xhp 04090002.xhp 04090003.xhp 04090004.xhp 04090005.xhp 04090006.xhp 04090007.xhp 04090100.xhp 04090200.xhp 04120000.xhp 04120100.xhp 04120200.xhp 04120201.xhp 04120210.xhp 04120211.xhp 04120212.xhp 04120213.xhp 04120214.xhp 04120215.xhp 04120216.xhp 04120217.xhp 04120219.xhp 04120220.xhp 04120221.xhp 04120222.xhp 04120223.xhp 04120224.xhp 04120225.xhp 04120226.xhp 04120227.xhp 04120229.xhp 04120250.xhp 04120300.xhp 04130000.xhp 04130100.xhp 04150000.xhp 04180400.xhp 04190000.xhp 04200000.xhp 04220000.xhp 04230000.xhp 04990000.xhp 05030200.xhp 05030400.xhp 05030800.xhp 05040000.xhp 05040500.xhp 05040501.xhp 05040600.xhp 05040700.xhp 05040800.xhp 05060000.xhp 05060100.xhp 05060200.xhp 05060201.xhp 05060300.xhp 05060700.xhp 05060800.xhp 05060900.xhp 05080000.xhp 05090000.xhp 05090100.xhp 05090200.xhp 05090201.xhp 05090300.xhp 05100000.xhp 05100300.xhp 05100400.xhp 05110000.xhp 05110100.xhp 05110200.xhp 05110300.xhp 05110500.xhp 05120000.xhp 05120100.xhp 05120200.xhp 05120300.xhp 05120400.xhp 05120500.xhp 05130000.xhp 05130002.xhp 05130004.xhp 05130100.xhp 05140000.xhp 05150000.xhp 05150100.xhp 05150101.xhp 05150104.xhp 05150200.xhp 05150300.xhp 05170000.xhp 05190000.xhp 05200000.xhp 05990000.xhp 06030000.xhp 06040000.xhp 06060000.xhp 06060100.xhp 06080000.xhp 06080100.xhp 06080200.xhp 06090000.xhp 06100000.xhp 06110000.xhp 06120000.xhp 06160000.xhp 06170000.xhp 06180000.xhp 06190000.xhp 06200000.xhp 06210000.xhp 06220000.xhp 06990000.xhp format_object.xhp mailmerge00.xhp mailmerge01.xhp mailmerge02.xhp mailmerge03.xhp mailmerge04.xhp mailmerge05.xhp mailmerge06.xhp mailmerge07.xhp mailmerge08.xhp mm_copyto.xhp mm_cusaddfie.xhp mm_cusaddlis.xhp mm_cusgrelin.xhp mm_emabod.xhp mm_finent.xhp mm_matfie.xhp mm_newaddblo.xhp mm_newaddlis.xhp mm_seladdblo.xhp mm_seladdlis.xhp mm_seltab.xhp selection_mode.xhp

${MYCMD} --base helpcontent2/source/text/swriter/02 --files 02110000.xhp 03210000.xhp 03220000.xhp 04090000.xhp 04100000.xhp 04220000.xhp 04230000.xhp 04240000.xhp 04250000.xhp 06040000.xhp 06070000.xhp 06080000.xhp 06090000.xhp 06120000.xhp 06130000.xhp 06140000.xhp 08010000.xhp 08080000.xhp 10010000.xhp 10020000.xhp 10030000.xhp 10050000.xhp 10070000.xhp 10080000.xhp 10090000.xhp 10110000.xhp 14010000.xhp 14020000.xhp 14030000.xhp 14040000.xhp 14050000.xhp 18010000.xhp 18030000.xhp 18030100.xhp 18030200.xhp 18030300.xhp 18030400.xhp 18030500.xhp 18030600.xhp 18030700.xhp 18120000.xhp 18130000.xhp 19010000.xhp 19020000.xhp 19030000.xhp 19040000.xhp 19050000.xhp word_count_stb.xhp

${MYCMD} --files helpcontent2/source/text/swriter/04/01020000.xhp

${MYCMD} --base helpcontent2/source/text/swriter/guide --files anchor_object.xhp arrange_chapters.xhp autocorr_except.xhp autotext.xhp auto_numbering.xhp auto_off.xhp auto_spellcheck.xhp background.xhp borders.xhp border_character.xhp border_object.xhp border_page.xhp calculate.xhp calculate_clipboard.xhp calculate_intable.xhp calculate_intext.xhp calculate_intext2.xhp calculate_multitable.xhp captions.xhp captions_numbers.xhp change_header.xhp chapter_numbering.xhp conditional_text.xhp conditional_text2.xhp delete_from_dict.xhp dragdroptext.xhp even_odd_sdw.xhp fields.xhp fields_date.xhp fields_enter.xhp fields_userdata.xhp field_convert.xhp finding.xhp footer_nextpage.xhp footer_pagenumber.xhp footnote_usage.xhp footnote_with_line.xhp form_letters_main.xhp globaldoc.xhp globaldoc_howtos.xhp header_footer.xhp header_pagestyles.xhp header_with_chapter.xhp header_with_line.xhp hidden_text.xhp hidden_text_display.xhp hyperlinks.xhp hyphen_prevent.xhp indenting.xhp indices_delete.xhp indices_edit.xhp indices_enter.xhp indices_form.xhp indices_index.xhp indices_literature.xhp indices_multidoc.xhp indices_toc.xhp indices_userdef.xhp insert_beforetable.xhp insert_graphic.xhp insert_graphic_dialog.xhp insert_graphic_fromchart.xhp insert_graphic_fromdraw.xhp insert_graphic_gallery.xhp insert_graphic_scan.xhp insert_tab_innumbering.xhp join_numbered_lists.xhp jump2statusbar.xhp keyboard.xhp load_styles.xhp main.xhp navigator.xhp nonprintable_text.xhp numbering_lines.xhp numbering_paras.xhp number_date_conv.xhp number_sequence.xhp pagebackground.xhp pagenumbers.xhp pageorientation.xhp pagestyles.xhp page_break.xhp printer_tray.xhp printing_order.xhp print_brochure.xhp print_preview.xhp print_small.xhp protection.xhp references.xhp references_modify.xhp registertrue.xhp removing_line_breaks.xhp reset_format.xhp resize_navigator.xhp ruler.xhp search_regexp.xhp sections.xhp section_edit.xhp section_insert.xhp send2html.xhp shortcut_writing.xhp smarttags.xhp spellcheck_dialog.xhp stylist_fillformat.xhp stylist_fromselect.xhp stylist_update.xhp subscript.xhp tablemode.xhp table_cellmerge.xhp table_cells.xhp table_delete.xhp table_insert.xhp table_repeat_multiple_headers.xhp table_select.xhp table_sizing.xhp templates_styles.xhp template_create.xhp template_default.xhp textdoc_inframe.xhp text_animation.xhp text_capital.xhp text_centervert.xhp text_direct_cursor.xhp text_emphasize.xhp text_frame.xhp text_nav_keyb.xhp text_rotate.xhp using_hyphen.xhp using_numbered_lists.xhp using_numbered_lists2.xhp using_numbering.xhp using_thesaurus.xhp words_count.xhp word_completion.xhp word_completion_adjust.xhp wrap.xhp

${MYCMD} --files helpcontent2/source/text/swriter/librelogo/LibreLogo.xhp

${MYCMD} --base helpcontent2/source/text/swriter --files main0000.xhp main0100.xhp main0101.xhp main0102.xhp main0103.xhp main0104.xhp main0105.xhp main0106.xhp main0107.xhp main0110.xhp main0115.xhp main0200.xhp main0202.xhp main0203.xhp main0204.xhp main0205.xhp main0206.xhp main0208.xhp main0210.xhp main0213.xhp main0214.xhp main0215.xhp main0216.xhp main0220.xhp main0503.xhp

${MYCMD} --base helpcontent2/source/text/swriter/menu --files insert_footnote_endnote.xhp insert_frame.xhp insert_header_footer.xhp

${MYCMD} --base instdir/sdk/examples/DevelopersGuide/Extensions/DialogWithHelp/help/de --files com.foocorp.foo-ext/page1.xhp com.foocorp.foo-ext/page2.xhp com.foocorp.foo-ext/subfolder/anotherpage.xhp

${MYCMD} --base instdir/sdk/examples/DevelopersGuide/Extensions/DialogWithHelp/help/en --fles com.foocorp.foo-ext/page1.xhp com.foocorp.foo-ext/page2.xhp com.foocorp.foo-ext/subfolder/anotherpage.xhp

${MYCMD} --base nlpsolver/help/en/com.sun.star.comp.Calc.NLPSolver --files Options.xhp Usage.xhp

${MYCMD} --base odk/examples/DevelopersGuide/Extensions/DialogWithHelp/help/de/com.foocorp.foo-ext --files page1.xhp page2.xhp subfolder/anotherpage.xhp

${MYCMD} --base odk/examples/DevelopersGuide/Extensions/DialogWithHelp/help/en/com.foocorp.foo-ext --files page1.xhp page2.xhp subfolder/anotherpage.xhp

# .ulf files

${MYCMD} --files extras/source/autocorr/emoji/emoji.ulf

${MYCMD} --files extras/source/gallery/share/gallery_names.ulf

${MYCMD} --base instsetoo_native/inc_openoffice/windows/msi_languages --files ActionTe.ulf Control.ulf CustomAc.ulf Error.ulf LaunchCo.ulf Property.ulf RadioBut.ulf UIText.ulf

${MYCMD} --base scp2/source/accessories --files module_accessories.ulf module_font_accessories.ulf module_gallery_accessories.ulf module_samples_accessories.ulf module_templates_accessories.ulf

${MYCMD} --files scp2/source/activex/module_activex.ulf

${MYCMD} --base scp2/source/base --files module_base.ulf folderitem_base.ulf postgresqlsdbc.ulf registryitem_base.ulf

${MYCMD} --base scp2/source/calc --files module_calc.ulf folderitem_calc.ulf registryitem_calc.ulf

${MYCMD} --base scp2/source/draw --files module_draw.ulf folderitem_draw.ulf registryitem_draw.ulf

${MYCMD} --base scp2/source/extensions --files module_extensions.ulf module_extensions_sun_templates.ulf

${MYCMD} --files scp2/source/gnome/module_gnome.ulf

${MYCMD} --files scp2/source/graphicfilter/module_graphicfilter.ulf

${MYCMD} --base scp2/source/impress --files folderitem_impress.ulf module_impress.ulf registryitem_impress.ulf

${MYCMD} --files scp2/source/kde/module_kde.ulf

${MYCMD} --base scp2/source/math --files folderitem_math.ulf module_math.ulf registryitem_math.ulf

${MYCMD} --files scp2/source/onlineupdate/module_onlineupdate.ulf

${MYCMD} --base scp2/source/ooo --files folderitem_ooo.ulf module_helppack.ulf module_langpack.ulf module_ooo.ulf module_reportbuilder.ulf module_systemint.ulf registryitem_ooo.ulf

${MYCMD} --files scp2/source/python/module_python_librelogo.ulf

${MYCMD} --files scp2/source/quickstart/module_quickstart.ulf

${MYCMD} --files scp2/source/tde/module_tde.ulf

${MYCMD} --files scp2/source/winexplorerext/module_winexplorerext.ulf

${MYCMD} --base scp2/source/writer --files folderitem_writer.ulf module_writer.ulf registryitem_writer.ulf

${MYCMD} --files scp2/source/xsltfilter/module_xsltfilter.ulf

${MYCMD} --files setup_native/source/mac/macinstall.ulf

${MYCMD} --files shell/source/win32/shlxthandler/res/shlxthdl.ulf

${MYCMD} --base sysui/desktop/share --files documents.ulf launcher_comment.ulf launcher_genericname.ulf launcher_unityquicklist.ulf


# .hrc files


${MYCMD} --files accessibility/inc/accessibility/helper/accessiblestrings.hrc

${MYCMD} --files avmedia/inc/helpids.hrc

${MYCMD} --files avmedia/source/framework/mediacontrol.hrc

${MYCMD} --files avmedia/source/viewer/mediawindow.hrc

${MYCMD} --files basctl/inc/basidesh.hrc basctl/inc/helpid.hrc

${MYCMD} --files basctl/sdi/basslots.hrc

${MYCMD} --base basctl/source/basicide --files baside2.hrc macrodlg.hrc objdlg.hrc

${MYCMD} --files basctl/source/inc/dlgresid.hrc

${MYCMD} --files basic/inc/sb.hrc

${MYCMD} --base chart2/source/controller/dialogs --files Bitmaps.hrc ResourceIds.hrc

${MYCMD} --base chart2/source/controller/inc --files HelpIds.hrc MenuResIds.hrc ShapeController.hrc

${MYCMD} --files chart2/source/controller/main/DrawCommandDispatch.hrc

${MYCMD} --files chart2/source/inc/Strings.hrc

${MYCMD} --files connectivity/source/drivers/hsqldb/hsqlui.hrc

${MYCMD} --base connectivity/source/inc/resource --files ado_res.hrc calc_res.hrc common_res.hrc conn_shared_res.hrc dbase_res.hrc evoab2_res.hrc file_res.hrc hsqldb_res.hrc jdbc_log.hrc kab_res.hrc macab_res.hrc mork_res.hrc

${MYCMD} --base cui/source/customize --files acccfg.hrc cfg.hrc

${MYCMD} --base cui/source/dialogs --files fmsearch.hrc scriptdlg.hrc svuidlg.hrc

${MYCMD} --base cui/source/inc --files cuires.hrc gallery.hrc helpid.hrc

${MYCMD} --files cui/source/options/optsave.hrc

${MYCMD} --base cui/source/tabpages --files align.hrc border.hrc numpages.hrc paragrph.hrc

${MYCMD} --files dbaccess/inc/dbaccess_helpid.hrc dbaccess/inc/dbaccess_slotid.hrc

${MYCMD} --base dbaccess/source/core/inc --files core_resource.hrc dbamiscres.hrc

${MYCMD} --base dbaccess/source/ext/macromigration --files dbmm_global.hrc macromigration.hrc

${MYCMD} --base dbaccess/source/inc --files dbastrings.hrc dbustrings.hrc sdbtstrings.hrc stringconstants.hrc xmlstrings.hrc

${MYCMD} --files dbaccess/source/sdbtools/inc/sdbt_resource.hrc

${MYCMD} --files dbaccess/source/ui/app/dbu_app.hrc

${MYCMD} --base dbaccess/source/ui/inc --files dbu_brw.hrc dbu_control.hrc dbu_dlg.hrc dbu_misc.hrc dbu_qry.hrc dbu_rel.hrc dbu_resource.hrc dbu_tbl.hrc dbu_uno.hrc sbagrid.hrc

${MYCMD} --files dbaccess/source/ui/querydesign/Query.hrc

${MYCMD} --files desktop/inc/deployment.hrc

${MYCMD} --files desktop/source/app/desktop.hrc

${MYCMD} --files desktop/source/deployment/gui/dp_gui.hrc

${MYCMD} --files desktop/source/deployment/manager/dp_manager.hrc

${MYCMD} --files desktop/source/deployment/registry/component/dp_component.hrc

${MYCMD} --files desktop/source/deployment/registry/configuration/dp_configuration.hrc

${MYCMD} --files desktop/source/deployment/registry/help/dp_help.hrc

${MYCMD} --files desktop/source/deployment/registry/inc/dp_registry.hrc

${MYCMD} --files desktop/source/deployment/registry/package/dp_package.hrc

${MYCMD} --files desktop/source/deployment/registry/script/dp_script.hrc

${MYCMD} --files desktop/source/deployment/registry/sfwk/dp_sfwk.hrc

${MYCMD} --files desktop/source/inc/helpid.hrc

${MYCMD} --files editeng/inc/editeng.hrc editeng/inc/helpid.hrc

${MYCMD} --base extensions/inc --files abpilot.hrc bibliography.hrc dbpilots.hrc extensio.hrc propctrlr.hrc update.hrc

${MYCMD} --files extensions/source/abpilot/abpresid.hrc

${MYCMD} --base extensions/source/bibliography --files bib.hrc bibprop.hrc

${MYCMD} --files extensions/source/dbpilots/dbpresid.hrc

${MYCMD} --base extensions/source/propctrlr --files formlocalid.hrc formresid.hrc listselectiondlg.hrc propresid.hrc

${MYCMD} --files extensions/source/scanner/strings.hrc

${MYCMD} --base extensions/source/update/check --files updatehdl.hrc updatecheckui.hrc

${MYCMD} --files filter/inc/filter.hrc

${MYCMD} --files filter/source/graphicfilter/eps/strings.hrc

${MYCMD} --files filter/source/pdf/impdialog.hrc filter/source/pdf/pdf.hrc

${MYCMD} --files filter/source/t602/t602filter.hrc

${MYCMD} --files filter/source/xsltdialog/xmlfilterdialogstrings.hrc

${MYCMD} --files forms/source/inc/frm_resource.hrc forms/source/inc/property.hrc

${MYCMD} --files formula/inc/helpids.hrc

${MYCMD} --files formula/source/core/inc/core_resource.hrc

${MYCMD} --files formula/source/ui/dlg/formdlgs.hrc

${MYCMD} --files formula/source/ui/inc/ForResId.hrc

${MYCMD} --base fpicker/source/office --files iodlg.hrc OfficeFilePicker.hrc

${MYCMD} --files framework/inc/classes/resource.hrc

${MYCMD} --files framework/inc/helpid.hrc

${MYCMD} --base include/editeng --files editids.hrc include/editeng/editrids.hrc include/editeng/memberids.hrc

${MYCMD} --files include/formula/compiler.hrc

${MYCMD} --base include/sfx2 --files cntids.hrc dialogs.hrc sfx.hrc sfxsids.hrc

${MYCMD} --base include/sfx2/sidebar --files ResourceDefinitions.hrc Sidebar.hrc

${MYCMD} --files include/sfx2/templatelocnames.hrc

${MYCMD} --base include/svl --files memberid.hrc solar.hrc style.hrc svl.hrc

${MYCMD} --base include/svtools --files controldims.hrc filedlg2.hrc helpid.hrc imagemgr.hrc svtools.hrc

${MYCMD} --base include/svx --files dialogs.hrc exthelpid.hrc fmresids.hrc svdstr.hrc svxids.hrc svxitems.hrc ucsubset.hrc

${MYCMD} --files include/vcl/fpicker.hrc

${MYCMD} --files include/version.hrc

${MYCMD} --base reportdesign/inc --files helpids.hrc RptResId.hrc rptui_slotid.hrc

${MYCMD} --files reportdesign/source/core/inc/core_resource.hrc

${MYCMD} --base reportdesign/source/inc --files corestrings.hrc stringconstants.hrc uistrings.hrc xmlstrings.hrc

${MYCMD} --files reportdesign/source/ui/dlg/CondFormat.hrc

${MYCMD} --base sc/inc --files globstr.hrc sc.hrc scfuncs.hrc

${MYCMD} --files sc/sdi/scslots.hrc

${MYCMD} --base sc/source/ui/dbgui --files asciiopt.hrc pvfundlg.hrc

${MYCMD} --files sc/source/ui/formdlg/dwfunctr.hrc

${MYCMD} --base sc/source/ui/inc --files acredlin.hrc checklistmenu.hrc condformatdlg.hrc filter.hrc iconsets.hrc miscdlgs.hrc

${MYCMD} --files sc/source/ui/navipi/navipi.hrc

${MYCMD} --files sc/source/ui/pagedlg/pagedlg.hrc

${MYCMD} --base sc/source/ui/sidebar --files CellAppearancePropertyPanel.hrc NumberFormatPropertyPanel.hrc

${MYCMD} --files sc/source/ui/StatisticsDialogs/StatisticsDialogs.hrc

${MYCMD} --files scaddins/source/analysis/analysis.hrc

${MYCMD} --files scaddins/source/datefunc/datefunc.hrc

${MYCMD} --files scaddins/source/pricing/pricing.hrc

${MYCMD} --files sccomp/source/solver/solver.hrc

${MYCMD} --base sd/inc --files app.hrc glob.hrc sdattr.hrc

${MYCMD} --files sd/sdi/sdslots.hrc

${MYCMD} --files sd/source/ui/accessibility/accessibility.hrc

${MYCMD} --files sd/source/ui/animations/CustomAnimation.hrc

${MYCMD} --files sd/source/ui/annotations/annotations.hrc

${MYCMD} --base sd/source/ui/inc --files dialogs.hrc navigatr.hrc prltempl.hrc res_bmp.hrc sdstring.hrc strings.hrc

${MYCMD} --files sd/source/ui/slideshow/slideshow.hrc

${MYCMD} --files sd/source/ui/slidesorter/inc/view/SlsResource.hrc

${MYCMD} --files sd/source/ui/view/DocumentRenderer.hrc

${MYCMD} --files sdext/source/minimizer/optimizerdialog.hrc

${MYCMD} --files sfx2/inc/filedlghelper.hrc

${MYCMD} --base sfx2/source/appl --files app.hrc newhelp.hrc

${MYCMD} --files sfx2/source/bastyp/bastyp.hrc

${MYCMD} --files sfx2/source/control/templateview.hrc

${MYCMD} --base sfx2/source/dialog --files dialog.hrc dinfdlg.hrc

${MYCMD} --files sfx2/source/doc/doc.hrc

${MYCMD} --base sfx2/source/inc --files helpid.hrc sfxlocal.hrc

${MYCMD} --files sfx2/source/view/view.hrc

${MYCMD} --files starmath/inc/starmath.hrc

${MYCMD} --files svtools/source/brwbox/editbrowsebox.hrc

${MYCMD} --base svtools/source/contnr --files fileview.hrc templwin.hrc

${MYCMD} --files svtools/source/dialogs/addresstemplate.hrc

${MYCMD} --files svtools/source/inc/filectrl.hrc

${MYCMD} --base svx/inc --files accessibility.hrc float3d.hrc fmhelp.hrc gallery.hrc galtheme.hrc helpid.hrc

${MYCMD} --files svx/sdi/svxslots.hrc

${MYCMD} --base svx/source/dialog --files bmpmask.hrc compressgraphicdialog.hrc docrecovery.hrc imapdlg.hrc ruler.hrc

${MYCMD} --base svx/source/inc --files datanavi.hrc fmprop.hrc frmsel.hrc

${MYCMD} --files svx/source/sidebar/area/AreaPropertyPanel.hrc

${MYCMD} --files svx/source/sidebar/EmptyPanel.hrc

${MYCMD} --files svx/source/sidebar/line/LinePropertyPanel.hrc

${MYCMD} --files svx/source/sidebar/text/TextPropertyPanel.hrc

${MYCMD} --files svx/source/tbxctrls/extrusioncontrols.hrc

${MYCMD} --base sw/inc --files access.hrc chrdlg.hrc comcore.hrc dbui.hrc dialog.hrc dochdl.hrc envelp.hrc fldui.hrc globals.hrc index.hrc poolfmt.hrc rcid.hrc statstr.hrc unocore.hrc

${MYCMD} --files sw/sdi/swslots.hrc

${MYCMD} --files sw/source/core/inc/pagefrm.hrc

${MYCMD} --files sw/source/core/undo/undo.hrc

${MYCMD} --files sw/source/ui/config/config.hrc

${MYCMD} --files sw/source/ui/envelp/envprt.hrc sw/source/ui/envelp/labfmt.hrc

${MYCMD} --files sw/source/uibase/dialog/regionsw.hrc

${MYCMD} --base sw/source/uibase/docvw --files annotation.hrc docvw.hrc

${MYCMD} --files sw/source/uibase/envelp/label.hrc

${MYCMD} --base sw/source/uibase/inc --files app.hrc frmui.hrc globdoc.hrc misc.hrc popup.hrc ribbar.hrc shells.hrc table.hrc utlui.hrc web.hrc

${MYCMD} --files sw/source/uibase/lingu/olmenu.hrc

${MYCMD} --files sw/source/uibase/misc/redlndlg.hrc

${MYCMD} --base sw/source/uibase/ribbar --files inputwin.hrc workctrl.hrc

${MYCMD} --base sw/source/uibase/sidebar --files PagePropertyPanel.hrc PropertyPanel.hrc

${MYCMD} --files sw/source/uibase/uiview/view.hrc

${MYCMD} --base sw/source/uibase/utlui --files attrdesc.hrc initui.hrc navipi.hrc unotools.hrc

${MYCMD} --files sw/source/uibase/wrtsh/wrtsh.hrc

${MYCMD} --files uui/source/ids.hrc

${MYCMD} --base vcl/inc --files svids.hrc win/salids.hrc

${MYCMD} --files vcl/source/edit/textundo.hrc

${MYCMD} --files writerperfect/inc/strings.hrc

${MYCMD} --files xmlsecurity/inc/xmlsecurity/global.hrc

${MYCMD} --base xmlsecurity/source/dialogs --files dialogs.hrc digitalsignaturesdialog.hrc helpids.hrc




    640 filelist_properties.txt
    718 filelist_src.txt
    916 filelist_xcu.txt
