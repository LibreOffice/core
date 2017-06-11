#!/bin/bash

cd ${LODE_HOME}/dev/core

MYCMD='./workdir/LinkTarget/Executable/genlang.exe extract -s --target workdir/jan '

# .xrm files
${MYCMD} --base readlicense_oo/docs --files readme.xrm


# .tree and .xhp files
${MYCMD} --base swext/mediawiki/help --files help.tree wiki.xhp wikiaccount.xhp wikiformats.xhp wikisend.xhp wikisettings.xhp

${MYCMD} --base helpcontent2/source/auxiliary --files sbasic.tree scalc.tree schart.tree shared.tree simpress.tree smath.tree swriter.tree

${MYCMD} --files nlpsolver/help/en/com.sun.star.comp.Calc.NLPSolver/help.tree

${MYCMD} --base helpcontent2/source/text/sbasic/guide --files access2base.xhp control_properties.xhp create_dialog.xhp insert_control.xhp sample_code.xhp show_dialog.xhp translation.xhp

${MYCMD} --base helpcontent2/source/text/sbasic/shared --files 00000002.xhp 00000003.xhp 01000000.xhp 01010210.xhp 01020000.xhp 01020100.xhp 01020200.xhp 01020300.xhp 01020500.xhp 01030000.xhp 01030100.xhp 01030200.xhp 01030300.xhp 01030400.xhp 01040000.xhp 01050000.xhp 01050100.xhp 01050200.xhp 01050300.xhp 01170100.xhp 01170101.xhp 01170103.xhp 03000000.xhp 03010000.xhp 03010100.xhp 03010101.xhp 03010102.xhp 03010103.xhp 03010200.xhp 03010201.xhp 03010300.xhp 03010301.xhp 03010302.xhp 03010303.xhp 03010304.xhp 03010305.xhp 03020000.xhp 03020100.xhp 03020101.xhp 03020102.xhp 03020103.xhp 03020104.xhp 03020200.xhp 03020201.xhp 03020202.xhp 03020203.xhp 03020204.xhp 03020205.xhp 03020301.xhp 03020302.xhp 03020303.xhp 03020304.xhp 03020305.xhp 03020400.xhp 03020401.xhp 03020402.xhp 03020403.xhp 03020404.xhp 03020405.xhp 03020406.xhp 03020407.xhp 03020408.xhp 03020409.xhp 03020410.xhp 03020411.xhp 03020412.xhp 03020413.xhp 03020414.xhp 03020415.xhp 03030000.xhp 03030100.xhp 03030101.xhp 03030102.xhp 03030103.xhp 03030104.xhp 03030105.xhp 03030106.xhp 03030107.xhp 03030108.xhp 03030110.xhp 03030111.xhp 03030112.xhp 03030113.xhp 03030114.xhp 03030115.xhp 03030116.xhp 03030120.xhp 03030130.xhp 03030200.xhp 03030201.xhp 03030202.xhp 03030203.xhp 03030204.xhp 03030205.xhp 03030206.xhp 03030300.xhp 03030301.xhp 03030302.xhp 03030303.xhp 03050000.xhp 03050100.xhp 03050200.xhp 03050300.xhp 03050500.xhp 03060000.xhp 03060100.xhp 03060200.xhp 03060300.xhp 03060400.xhp 03060500.xhp 03060600.xhp 03070000.xhp 03070100.xhp 03070200.xhp 03070300.xhp 03070400.xhp 03070500.xhp 03070600.xhp 03080000.xhp 03080100.xhp 03080101.xhp 03080102.xhp 03080103.xhp 03080104.xhp 03080200.xhp 03080201.xhp 03080202.xhp 03080300.xhp 03080301.xhp 03080302.xhp 03080400.xhp 03080401.xhp 03080500.xhp 03080501.xhp 03080502.xhp 03080600.xhp 03080601.xhp 03080700.xhp 03080701.xhp 03080800.xhp 03080801.xhp 03080802.xhp 03090000.xhp 03090100.xhp 03090101.xhp 03090102.xhp 03090103.xhp 03090200.xhp 03090201.xhp 03090202.xhp 03090203.xhp 03090300.xhp 03090301.xhp 03090302.xhp 03090303.xhp 03090400.xhp 03090401.xhp 03090402.xhp 03090403.xhp 03090404.xhp 03090405.xhp 03090406.xhp 03090407.xhp 03090408.xhp 03090409.xhp 03090410.xhp 03090411.xhp 03090412.xhp 03100000.xhp 03100050.xhp 03100060.xhp 03100070.xhp 03100080.xhp 03100100.xhp 03100300.xhp 03100400.xhp 03100500.xhp 03100600.xhp 03100700.xhp 03100900.xhp 03101000.xhp 03101100.xhp 03101110.xhp 03101120.xhp 03101130.xhp 03101140.xhp 03101300.xhp 03101400.xhp 03101500.xhp 03101600.xhp 03101700.xhp 03102000.xhp 03102100.xhp 03102101.xhp 03102200.xhp 03102300.xhp 03102400.xhp 03102450.xhp 03102600.xhp 03102700.xhp 03102800.xhp 03102900.xhp 03103000.xhp 03103100.xhp 03103200.xhp 03103300.xhp 03103400.xhp 03103450.xhp 03103500.xhp 03103600.xhp 03103700.xhp 03103800.xhp 03103900.xhp 03104000.xhp 03104100.xhp 03104200.xhp 03104300.xhp 03104400.xhp 03104500.xhp 03104600.xhp 03104700.xhp 03110000.xhp 03110100.xhp 03120000.xhp 03120100.xhp 03120101.xhp 03120102.xhp 03120103.xhp 03120104.xhp 03120105.xhp 03120200.xhp 03120201.xhp 03120202.xhp 03120300.xhp 03120301.xhp 03120302.xhp 03120303.xhp 03120304.xhp 03120305.xhp 03120306.xhp 03120307.xhp 03120308.xhp 03120309.xhp 03120310.xhp 03120311.xhp 03120312.xhp 03120313.xhp 03120314.xhp 03120315.xhp 03120400.xhp 03120401.xhp 03120402.xhp 03120403.xhp 03130000.xhp 03130100.xhp 03130500.xhp 03130600.xhp 03130700.xhp 03130800.xhp 03131000.xhp 03131300.xhp 03131400.xhp 03131500.xhp 03131600.xhp 03131700.xhp 03131800.xhp 03131900.xhp 03132000.xhp 03132100.xhp 03132200.xhp 03132300.xhp 03132400.xhp 03132500.xhp 05060700.xhp code-stubs.xhp keys.xhp main0211.xhp main0601.xhp

${MYCMD} --base helpcontent2/source/text/sbasic/shared/01 --files 06130000.xhp 06130100.xhp 06130500.xhp

${MYCMD} --base helpcontent2/source/text/sbasic/shared/02 --files 11010000.xhp 11020000.xhp 11030000.xhp 11040000.xhp 11050000.xhp 11060000.xhp 11070000.xhp 11080000.xhp 11090000.xhp 11100000.xhp 11110000.xhp 11120000.xhp 11140000.xhp 11150000.xhp 11160000.xhp 11170000.xhp 11180000.xhp 11190000.xhp 20000000.xhp

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

${MYCMD} --base helpcontent2/source/text/shared/menu --files insert_chart.xhp insert_form_control.xhp insert_shape.xhp

${MYCMD} --base helpcontent2/source/text/shared/05 --files 00000001.xhp 00000002.xhp 00000100.xhp 00000110.xhp 00000120.xhp 00000130.xhp 00000140.xhp 00000150.xhp 00000160.xhp err_html.xhp

${MYCMD} --files helpcontent2/source/text/shared/07/09000000.xhp

${MYCMD} --base helpcontent2/source/text/shared --files 3dsettings_toolbar.xhp fontwork_toolbar.xhp main0108.xhp main0201.xhp main0204.xhp main0208.xhp main0212.xhp main0213.xhp main0214.xhp main0226.xhp main0227.xhp main0400.xhp main0500.xhp main0600.xhp main0650.xhp main0800.xhp need_help.xhp

${MYCMD} --base helpcontent2/source/text/shared/autokorr --files 01000000.xhp 02000000.xhp 03000000.xhp 04000000.xhp 05000000.xhp 06000000.xhp 07000000.xhp 08000000.xhp 09000000.xhp 10000000.xhp 12000000.xhp 13000000.xhp


${MYCMD} --base helpcontent2/source/text/shared/autopi --files 01000000.xhp 01010000.xhp 01010100.xhp 01010200.xhp 01010300.xhp 01010400.xhp 01010500.xhp 01010600.xhp 01020000.xhp 01020100.xhp 01020200.xhp 01020300.xhp 01020400.xhp 01020500.xhp 01040000.xhp 01040100.xhp 01040200.xhp 01040300.xhp 01040400.xhp 01040500.xhp 01040600.xhp 01050000.xhp 01050100.xhp 01050200.xhp 01050300.xhp 01050400.xhp 01050500.xhp 01090000.xhp 01090100.xhp 01090200.xhp 01090210.xhp 01090220.xhp 01090300.xhp 01090400.xhp 01090500.xhp 01090600.xhp 01100000.xhp 01100100.xhp 01100150.xhp 01100200.xhp 01100300.xhp 01100400.xhp 01100500.xhp 01110000.xhp 01110100.xhp 01110200.xhp 01110300.xhp 01110400.xhp 01110500.xhp 01110600.xhp 01120000.xhp 01120100.xhp 01120200.xhp 01120300.xhp 01120400.xhp 01120500.xhp 01130000.xhp 01130100.xhp 01130200.xhp 01150000.xhp 01170000.xhp 01170200.xhp 01170300.xhp 01170400.xhp 01170500.xhp

${MYCMD} --base helpcontent2/source/text/shared/explorer/database --files 02000000.xhp 02000002.xhp 02010100.xhp 02010101.xhp 04000000.xhp 04030000.xhp 05000000.xhp 05000001.xhp 05000002.xhp 05000003.xhp 05010000.xhp 05010100.xhp 05020000.xhp 05020100.xhp 05030000.xhp 05030100.xhp 05030200.xhp 05030300.xhp 05030400.xhp 05040000.xhp 05040100.xhp 05040200.xhp 11000002.xhp 11020000.xhp 11030000.xhp 11030100.xhp 11080000.xhp 11090000.xhp 11150200.xhp 11170000.xhp 11170100.xhp 30000000.xhp 30100000.xhp dabaadvprop.xhp dabaadvpropdat.xhp dabaadvpropgen.xhp dabadoc.xhp dabaprop.xhp dabapropadd.xhp dabapropcon.xhp dabapropgen.xhp dabawiz00.xhp dabawiz01.xhp dabawiz02.xhp dabawiz02access.xhp dabawiz02adabas.xhp dabawiz02ado.xhp dabawiz02dbase.xhp dabawiz02jdbc.xhp dabawiz02ldap.xhp dabawiz02mysql.xhp dabawiz02odbc.xhp dabawiz02oracle.xhp dabawiz02spreadsheet.xhp dabawiz02text.xhp dabawiz03auth.xhp main.xhp menubar.xhp menuedit.xhp menufile.xhp menufilesave.xhp menuinsert.xhp menutools.xhp menuview.xhp migrate_macros.xhp password.xhp querywizard00.xhp querywizard01.xhp querywizard02.xhp querywizard03.xhp querywizard04.xhp querywizard05.xhp querywizard06.xhp querywizard07.xhp querywizard08.xhp rep_datetime.xhp rep_main.xhp rep_navigator.xhp rep_pagenumbers.xhp rep_prop.xhp rep_sort.xhp tablewizard00.xhp tablewizard01.xhp tablewizard02.xhp tablewizard03.xhp tablewizard04.xhp toolbars.xhp

${MYCMD} --base helpcontent2/source/text/shared/guide --files aaa_start.xhp accessibility.xhp activex.xhp active_help_on_off.xhp assistive.xhp autocorr_url.xhp autohide.xhp background.xhp border_paragraph.xhp border_table.xhp breaking_lines.xhp change_title.xhp chart_axis.xhp chart_barformat.xhp chart_insert.xhp chart_legend.xhp chart_title.xhp collab.xhp configure_overview.xhp contextmenu.xhp copytable2application.xhp copytext2application.xhp copy_drawfunctions.xhp ctl.xhp database_main.xhp data_addressbook.xhp data_dbase2office.xhp data_enter_sql.xhp data_forms.xhp data_im_export.xhp data_new.xhp data_queries.xhp data_register.xhp data_report.xhp data_reports.xhp data_search.xhp data_search2.xhp data_tabledefine.xhp data_tables.xhp data_view.xhp digitalsign_receive.xhp digitalsign_send.xhp digital_signatures.xhp doc_autosave.xhp doc_open.xhp doc_save.xhp dragdrop.xhp dragdrop_beamer.xhp dragdrop_fromgallery.xhp dragdrop_gallery.xhp dragdrop_graphic.xhp dragdrop_table.xhp edit_symbolbar.xhp email.xhp error_report.xhp export_ms.xhp fax.xhp filternavigator.xhp find_attributes.xhp flat_icons.xhp floating_toolbar.xhp fontwork.xhp formfields.xhp gallery_insert.xhp groups.xhp hyperlink_edit.xhp hyperlink_insert.xhp hyperlink_rel_abs.xhp imagemap.xhp import_ms.xhp insert_bitmap.xhp insert_graphic_drawit.xhp insert_specialchar.xhp integratinguno.xhp keyboard.xhp labels.xhp labels_database.xhp language_select.xhp lineend_define.xhp linestyles.xhp linestyle_define.xhp line_intext.xhp macro_recording.xhp main.xhp measurement_units.xhp microsoft_terms.xhp ms_doctypes.xhp ms_import_export_limitations.xhp ms_user.xhp navigator.xhp navigator_setcursor.xhp navpane_on.xhp numbering_stop.xhp pageformat_max.xhp paintbrush.xhp pasting.xhp print_blackwhite.xhp print_faster.xhp protection.xhp redlining.xhp redlining_accept.xhp redlining_doccompare.xhp redlining_docmerge.xhp redlining_enter.xhp redlining_navigation.xhp redlining_protect.xhp redlining_versions.xhp round_corner.xhp scripting.xhp space_hyphen.xhp spadmin.xhp standard_template.xhp startcenter.xhp start_parameters.xhp tabs.xhp textmode_change.xhp text_color.xhp undo_formatting.xhp version_number.xhp viewing_file_properties.xhp workfolder.xhp xforms.xhp xsltfilter.xhp xsltfilter_create.xhp xsltfilter_distribute.xhp

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

${MYCMD} --base helpcontent2/source/text/swriter --files main0000.xhp main0100.xhp main0101.xhp main0102.xhp main0103.xhp main0104.xhp main0105.xhp main0106.xhp main0107.xhp main0110.xhp main0115.xhp main0200.xhp main0202.xhp main0203.xhp main0204.xhp main0205.xhp main0206.xhp main0208.xhp main0210.xhp main0213.xhp main0214.xhp main0215.xhp main0216.xhp main0220.xhp main0503.xhp

${MYCMD} --base helpcontent2/source/text/swriter/01 --files 01120000.xhp 01150000.xhp 01160100.xhp 01160200.xhp 01160300.xhp 01160400.xhp 01160500.xhp 02110000.xhp 02110100.xhp 02120000.xhp 02120100.xhp 02130000.xhp 02140000.xhp 02150000.xhp 02160000.xhp 02170000.xhp 03050000.xhp 03070000.xhp 03080000.xhp 03090000.xhp 03100000.xhp 03120000.xhp 03130000.xhp 03140000.xhp 04010000.xhp 04020000.xhp 04020100.xhp 04020200.xhp 04030000.xhp 04040000.xhp 04060000.xhp 04060100.xhp 04070000.xhp 04070100.xhp 04070200.xhp 04070300.xhp 04090000.xhp 04090001.xhp 04090002.xhp 04090003.xhp 04090004.xhp 04090005.xhp 04090006.xhp 04090007.xhp 04090100.xhp 04090200.xhp 04120000.xhp 04120100.xhp 04120200.xhp 04120201.xhp 04120210.xhp 04120211.xhp 04120212.xhp 04120213.xhp 04120214.xhp 04120215.xhp 04120216.xhp 04120217.xhp 04120219.xhp 04120220.xhp 04120221.xhp 04120222.xhp 04120223.xhp 04120224.xhp 04120225.xhp 04120226.xhp 04120227.xhp 04120229.xhp 04120250.xhp 04120300.xhp 04130000.xhp 04130100.xhp 04150000.xhp 04180400.xhp 04190000.xhp 04200000.xhp 04220000.xhp 04230000.xhp 04990000.xhp 05030200.xhp 05030400.xhp 05030800.xhp 05040000.xhp 05040500.xhp 05040501.xhp 05040600.xhp 05040700.xhp 05040800.xhp 05060000.xhp 05060100.xhp 05060200.xhp 05060201.xhp 05060300.xhp 05060700.xhp 05060800.xhp 05060900.xhp 05080000.xhp 05090000.xhp 05090100.xhp 05090200.xhp 05090201.xhp 05090300.xhp 05100000.xhp 05100300.xhp 05100400.xhp 05110000.xhp 05110100.xhp 05110200.xhp 05110300.xhp 05110500.xhp 05120000.xhp 05120100.xhp 05120200.xhp 05120300.xhp 05120400.xhp 05120500.xhp 05130000.xhp 05130002.xhp 05130004.xhp 05130100.xhp 05140000.xhp 05150000.xhp 05150100.xhp 05150101.xhp 05150104.xhp 05150200.xhp 05150300.xhp 05170000.xhp 05190000.xhp 05200000.xhp 05990000.xhp 06030000.xhp 06040000.xhp 06060000.xhp 06060100.xhp 06080000.xhp 06080100.xhp 06080200.xhp 06090000.xhp 06100000.xhp 06110000.xhp 06120000.xhp 06160000.xhp 06170000.xhp 06180000.xhp 06190000.xhp 06200000.xhp 06210000.xhp 06220000.xhp 06990000.xhp format_object.xhp mailmerge00.xhp mailmerge01.xhp mailmerge02.xhp mailmerge03.xhp mailmerge04.xhp mailmerge05.xhp mailmerge06.xhp mailmerge07.xhp mailmerge08.xhp mm_copyto.xhp mm_cusaddfie.xhp mm_cusaddlis.xhp mm_cusgrelin.xhp mm_emabod.xhp mm_finent.xhp mm_matfie.xhp mm_newaddblo.xhp mm_newaddlis.xhp mm_seladdblo.xhp mm_seladdlis.xhp mm_seltab.xhp selection_mode.xhp

${MYCMD} --base helpcontent2/source/text/swriter/02 --files 02110000.xhp 03210000.xhp 03220000.xhp 04090000.xhp 04100000.xhp 04220000.xhp 04230000.xhp 04240000.xhp 04250000.xhp 06040000.xhp 06070000.xhp 06080000.xhp 06090000.xhp 06120000.xhp 06130000.xhp 06140000.xhp 08010000.xhp 08080000.xhp 10010000.xhp 10020000.xhp 10030000.xhp 10050000.xhp 10070000.xhp 10080000.xhp 10090000.xhp 10110000.xhp 14010000.xhp 14020000.xhp 14030000.xhp 14040000.xhp 14050000.xhp 18010000.xhp 18030000.xhp 18030100.xhp 18030200.xhp 18030300.xhp 18030400.xhp 18030500.xhp 18030600.xhp 18030700.xhp 18120000.xhp 18130000.xhp 19010000.xhp 19020000.xhp 19030000.xhp 19040000.xhp 19050000.xhp word_count_stb.xhp

${MYCMD} --files helpcontent2/source/text/swriter/04/01020000.xhp

${MYCMD} --base helpcontent2/source/text/swriter/guide --files anchor_object.xhp arrange_chapters.xhp autocorr_except.xhp autotext.xhp auto_numbering.xhp auto_off.xhp auto_spellcheck.xhp background.xhp borders.xhp border_character.xhp border_object.xhp border_page.xhp calculate.xhp calculate_clipboard.xhp calculate_intable.xhp calculate_intext.xhp calculate_intext2.xhp calculate_multitable.xhp captions.xhp captions_numbers.xhp change_header.xhp chapter_numbering.xhp conditional_text.xhp conditional_text2.xhp delete_from_dict.xhp dragdroptext.xhp even_odd_sdw.xhp fields.xhp fields_date.xhp fields_enter.xhp fields_userdata.xhp field_convert.xhp finding.xhp footer_nextpage.xhp footer_pagenumber.xhp footnote_usage.xhp footnote_with_line.xhp form_letters_main.xhp globaldoc.xhp globaldoc_howtos.xhp header_footer.xhp header_pagestyles.xhp header_with_chapter.xhp header_with_line.xhp hidden_text.xhp hidden_text_display.xhp hyperlinks.xhp hyphen_prevent.xhp indenting.xhp indices_delete.xhp indices_edit.xhp indices_enter.xhp indices_form.xhp indices_index.xhp indices_literature.xhp indices_multidoc.xhp indices_toc.xhp indices_userdef.xhp insert_beforetable.xhp insert_graphic.xhp insert_graphic_dialog.xhp insert_graphic_fromchart.xhp insert_graphic_fromdraw.xhp insert_graphic_gallery.xhp insert_graphic_scan.xhp insert_tab_innumbering.xhp join_numbered_lists.xhp jump2statusbar.xhp keyboard.xhp load_styles.xhp main.xhp navigator.xhp nonprintable_text.xhp numbering_lines.xhp numbering_paras.xhp number_date_conv.xhp number_sequence.xhp pagebackground.xhp pagenumbers.xhp pageorientation.xhp pagestyles.xhp page_break.xhp printer_tray.xhp printing_order.xhp print_brochure.xhp print_preview.xhp print_small.xhp protection.xhp references.xhp references_modify.xhp registertrue.xhp removing_line_breaks.xhp reset_format.xhp resize_navigator.xhp ruler.xhp search_regexp.xhp sections.xhp section_edit.xhp section_insert.xhp send2html.xhp shortcut_writing.xhp smarttags.xhp spellcheck_dialog.xhp stylist_fillformat.xhp stylist_fromselect.xhp stylist_update.xhp subscript.xhp tablemode.xhp table_cellmerge.xhp table_cells.xhp table_delete.xhp table_insert.xhp table_repeat_multiple_headers.xhp table_select.xhp table_sizing.xhp templates_styles.xhp template_create.xhp template_default.xhp textdoc_inframe.xhp text_animation.xhp text_capital.xhp text_centervert.xhp text_direct_cursor.xhp text_emphasize.xhp text_frame.xhp text_nav_keyb.xhp text_rotate.xhp using_hyphen.xhp using_numbered_lists.xhp using_numbered_lists2.xhp using_numbering.xhp using_thesaurus.xhp words_count.xhp word_completion.xhp word_completion_adjust.xhp wrap.xhp

${MYCMD} --files helpcontent2/source/text/swriter/librelogo/LibreLogo.xhp

${MYCMD} --base helpcontent2/source/text/swriter --files main0000.xhp main0100.xhp main0101.xhp main0102.xhp main0103.xhp main0104.xhp main0105.xhp main0106.xhp main0107.xhp main0110.xhp main0115.xhp main0200.xhp main0202.xhp main0203.xhp main0204.xhp main0205.xhp main0206.xhp main0208.xhp main0210.xhp main0213.xhp main0214.xhp main0215.xhp main0216.xhp main0220.xhp main0503.xhp menu/insert_footnote_endnote.xhp menu/insert_frame.xhp menu/insert_header_footer.xhp

${MYCMD} --base nlpsolver/help/en/com.sun.star.comp.Calc.NLPSolver --files Options.xhp Usage.xhp


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

${MYCMD} --files scp2/source/winexplorerext/module_winexplorerext.ulf

${MYCMD} --base scp2/source/writer --files folderitem_writer.ulf module_writer.ulf registryitem_writer.ulf

${MYCMD} --files scp2/source/xsltfilter/module_xsltfilter.ulf

${MYCMD} --files setup_native/source/mac/macinstall.ulf

${MYCMD} --files shell/source/win32/shlxthandler/res/shlxthdl.ulf

${MYCMD} --base sysui/desktop/share --files documents.ulf launcher_comment.ulf launcher_genericname.ulf launcher_unityquicklist.ulf


# .hrc files

${MYCMD} --files extensions/source/update/check/updatehdl.hrc


# .properties files

${MYCMD} --files dictionaries/en/dialog/en_en_US.properties

${MYCMD} --base dictionaries/hu_HU/dialog --files hu_HU_en_US.properties hu_HU_hu_HU.properties

${MYCMD} --files dictionaries/pt_BR/dialog/pt_BR_pt_BR.properties

${MYCMD} --target workdir/jan/dictionaries/ru_RU/dialog.pot --base dictionaries/ru_RU/dialog --files ru_RU_en_US.properties ru_RU_ru_RU.properties

${MYCMD} --files librelogo/source/pythonpath/LibreLogo_en_US.properties

${MYCMD} --base nlpsolver/src/locale --files NLPSolverCommon_en_US.properties NLPSolverStatusDialog_en_US.properties

${MYCMD} --base reportbuilder/java/org/libreoffice/report/function/metadata --files Author-Function.properties Author-Function_en_US.properties category.properties category_en_US.properties Title-Function.properties Title-Function_en_US.properties


# .src files

${MYCMD} --files accessibility/source/helper/accessiblestrings.src

${MYCMD} --files avmedia/source/framework/mediacontrol.src

${MYCMD} --files avmedia/source/viewer/mediawindow.src

${MYCMD} --base basctl/source/basicide --files basicprint.src basidesh.src macrodlg.src moduldlg.src objdlg.src

${MYCMD} --files basctl/source/dlged/dlgresid.src

${MYCMD} --files basic/source/classes/sb.src

${MYCMD} --files basic/source/sbx/format.src

${MYCMD} --base chart2/source/controller/dialogs --files Bitmaps.src Strings.src Strings_ChartTypes.src Strings_Scale.src Strings_Statistic.src

${MYCMD} --base connectivity/source/resource --files conn_error_message.src conn_log_res.src conn_shared_res.src

${MYCMD} --base cui/source/customize --files acccfg.src cfg.src macropg.src

${MYCMD} --base cui/source/dialogs --files cuires.src fmsearch.src gallery.src hyperdlg.src multipat.src passwdomdlg.src scriptdlg.src svuidlg.src

${MYCMD} --base cui/source/options --files connpooloptions.src dbregister.src doclinkdialog.src optchart.src optcolor.src optdict.src optfltr.src optgdlg.src optinet2.src optjava.src optlingu.src optpath.src personalization.src treeopt.src

${MYCMD} --base cui/source/tabpages --files align.src border.src frmdirlbox.src page.src paragrph.src strings.src

${MYCMD} --files dbaccess/source/core/resource/strings.src

${MYCMD} --files dbaccess/source/ext/macromigration/macromigration.src

${MYCMD} --files dbaccess/source/sdbtools/resource/sdbt_strings.src

${MYCMD} --files dbaccess/source/ui/app/app.src

${MYCMD} --base dbaccess/source/ui/browser --files sbabrw.src sbagrid.src

${MYCMD} --base dbaccess/source/ui/control --files TableGrantCtrl.src tabletree.src undosqledit.src

${MYCMD} --base dbaccess/source/ui/dlg --files AutoControls.src ConnectionPage.src dbadmin.src dbadmin2.src dbadminsetup.src directsql.src indexdialog.src paramdialog.src sqlmessage.src UserAdmin.src

${MYCMD} --base dbaccess/source/ui/misc --files dbumiscres.src WizardPages.src

${MYCMD} --base dbaccess/source/ui/querydesign --files query.src querydlg.src

${MYCMD} --files dbaccess/source/ui/relationdesign/relation.src

${MYCMD} --files dbaccess/source/ui/tabledesign/table.src

${MYCMD} --base dbaccess/source/ui/uno --files copytablewizard.src

${MYCMD} --files desktop/source/app/desktop.src

${MYCMD} --base desktop/source/deployment/gui --files dp_gui_backend.src dp_gui_dialog.src dp_gui_dialog2.src dp_gui_updatedialog.src dp_gui_updateinstalldialog.src dp_gui_versionboxes.src

${MYCMD} --files desktop/source/deployment/manager/dp_manager.src

${MYCMD} --files desktop/source/deployment/misc/dp_misc.src

${MYCMD} --files desktop/source/deployment/registry/component/dp_component.src

${MYCMD} --files desktop/source/deployment/registry/configuration/dp_configuration.src

${MYCMD} --files desktop/source/deployment/registry/dp_registry.src

${MYCMD} --files desktop/source/deployment/registry/help/dp_help.src

${MYCMD} --files desktop/source/deployment/registry/package/dp_package.src

${MYCMD} --files desktop/source/deployment/registry/script/dp_script.src

${MYCMD} --files desktop/source/deployment/registry/sfwk/dp_sfwk.src

${MYCMD} --files desktop/source/deployment/unopkg/unopkg.src

${MYCMD} --files editeng/source/accessibility/accessibility.src

${MYCMD} --files editeng/source/editeng/editeng.src

${MYCMD} --base editeng/source/items --files page.src svxitems.src

${MYCMD} --files editeng/source/misc/lingu.src

${MYCMD} --files editeng/source/outliner/outliner.src

${MYCMD} --files extensions/source/abpilot/abspilot.src

${MYCMD} --base extensions/source/bibliography --files bib.src sections.src toolbar.src

${MYCMD} --base extensions/source/dbpilots --files commonpagesdbp.src dbpilots.src gridpages.src groupboxpages.src listcombopages.src

${MYCMD} --base extensions/source/propctrlr --files formlinkdialog.src formres.src pcrmiscres.src propres.src selectlabeldialog.src

${MYCMD} --files extensions/source/scanner/strings.src

${MYCMD} --files filter/source/graphicfilter/eps/epsstr.src

${MYCMD} --base filter/source/pdf --files impdialog.src pdf.src

${MYCMD} --files filter/source/t602/t602filter.src

${MYCMD} --files filter/source/xsltdialog/xmlfilterdialogstrings.src

${MYCMD} --base forms/source/resource --files strings.src xforms.src

${MYCMD} --files formula/source/core/resource/core_resource.src

${MYCMD} --base formula/source/ui/dlg --files formdlgs.src parawin.src

${MYCMD} --base fpicker/source/office --files iodlg.src OfficeFilePicker.src

${MYCMD} --files framework/source/classes/resource.src

${MYCMD} --files reportdesign/source/core/resource/strings.src

${MYCMD} --base reportdesign/source/ui/dlg --files CondFormat.src GroupsSorting.src Navigator.src PageNumber.src

${MYCMD} --files reportdesign/source/ui/inspection/inspection.src

${MYCMD} --files reportdesign/source/ui/report/report.src

${MYCMD} --files sc/source/core/src/compiler.src

${MYCMD} --files sc/source/ui/cctrl/checklistmenu.src

${MYCMD} --base sc/source/ui/dbgui --files outline.src pvfundlg.src

${MYCMD} --files sc/source/ui/formdlg/dwfunctr.src

${MYCMD} --base sc/source/ui/miscdlgs --files acredlin.src conflictsdlg.src retypepassdlg.src

${MYCMD} --files sc/source/ui/navipi/navipi.src

${MYCMD} --base sc/source/ui/sidebar --files CellAppearancePropertyPanel.src NumberFormatPropertyPanel.src

${MYCMD} --base sc/source/ui/src --files condformatdlg.src globstr.src iconsets.src sc.src scerrors.src scfuncs.src scstring.src scwarngs.src toolbox.src

${MYCMD} --files sc/source/ui/StatisticsDialogs/StatisticsDialogs.src

${MYCMD} --files sc/source/ui/styleui/scstyles.src

${MYCMD} --base scaddins/source/analysis --files analysis.src analysis_deffuncnames.src analysis_funcnames.src

${MYCMD} --files scaddins/source/datefunc/datefunc.src

${MYCMD} --files scaddins/source/pricing/pricing.src

${MYCMD} --files sccomp/source/solver/solver.src

${MYCMD} --files sd/source/core/glob.src

${MYCMD} --files sd/source/ui/accessibility/accessibility.src

${MYCMD} --files sd/source/ui/animations/CustomAnimation.src

${MYCMD} --files sd/source/ui/annotations/annotations.src

${MYCMD} --base sd/source/ui/app --files popup.src res_bmp.src sdstring.src strings.src toolbox.src

${MYCMD} --files sd/source/ui/dlg/navigatr.src

${MYCMD} --files sd/source/ui/slideshow/slideshow.src

${MYCMD} --files sd/source/ui/view/DocumentRenderer.src

${MYCMD} --base sfx2/source/appl --files app.src newhelp.src sfx.src

${MYCMD} --base sfx2/source/bastyp --files bastyp.src fltfnc.src

${MYCMD} --files sfx2/source/control/templateview.src

${MYCMD} --base sfx2/source/dialog --files dialog.src dinfdlg.src filedlghelper.src newstyle.src recfloat.src taskpane.src templdlg.src titledockwin.src versdlg.src

${MYCMD} --base sfx2/source/doc --files doc.src doctempl.src graphhelp.src templatelocnames.src

${MYCMD} --files sfx2/source/sidebar/Sidebar.src

${MYCMD} --files sfx2/source/view/view.src

${MYCMD} --base starmath/source --files commands.src smres.src symbol.src

${MYCMD} --files svl/source/misc/mediatyp.src

${MYCMD} --base svtools/source/contnr --files fileview.src svcontnr.src templwin.src

${MYCMD} --base svtools/source/control --files calendar.src ctrlbox.src ctrltool.src filectrl.src ruler.src

${MYCMD} --base svtools/source/dialogs --files addresstemplate.src filedlg2.src formats.src so3res.src wizardmachine.src

${MYCMD} --files svtools/source/java/javaerror.src

${MYCMD} --base svtools/source/misc --files ehdl.src imagemgr.src langtab.src svtools.src undo.src

${MYCMD} --files svx/source/accessibility/accessibility.src

${MYCMD} --files svx/source/core/graphichelper.src

${MYCMD} --base svx/source/dialog --files bmpmask.src compressgraphicdialog.src dlgctrl.src docrecovery.src frmsel.src imapdlg.src langbox.src language.src passwd.src prtqry.src ruler.src sdstring.src srchdlg.src svxbmpnumvalueset.src swframeposstrings.src txenctab.src ucsubset.src

${MYCMD} --base svx/source/engine3d --files float3d.src string3d.src

${MYCMD} --files svx/source/fmcomp/gridctrl.src

${MYCMD} --base svx/source/form --files datanavi.src filtnav.src fmexpl.src fmstring.src

${MYCMD} --base svx/source/gallery2 --files gallery.src galtheme.src

${MYCMD} --base svx/source/items --files svxerr.src svxitems.src

${MYCMD} --files svx/source/sidebar/area/AreaPropertyPanel.src

${MYCMD} --files svx/source/sidebar/EmptyPanel.src

#${MYCMD} --files svx/source/sidebar/line/LinePropertyPanel.src

${MYCMD} --files svx/source/sidebar/text/TextPropertyPanel.src

${MYCMD} --files svx/source/src/errtxt.src

${MYCMD} --files svx/source/stbctrls/stbctrls.src

${MYCMD} --files svx/source/svdraw/svdstr.src

${MYCMD} --files svx/source/table/table.src

${MYCMD} --base svx/source/tbxctrls --files colrctrl.src extrusioncontrols.src fontworkgallery.src grafctrl.src lboxctrl.src tbcontrl.src tbunosearchcontrollers.src

${MYCMD} --base svx/source/toolbars --files extrusionbar.src fontworkbar.src

${MYCMD} --files sw/source/core/layout/pagefrm.src

${MYCMD} --files sw/source/core/undo/undo.src

${MYCMD} --files sw/source/core/unocore/unocore.src

${MYCMD} --base sw/source/ui/app --files app.src error.src mn.src

${MYCMD} --files sw/source/ui/chrdlg/chrdlg.src

${MYCMD} --base sw/source/ui/config --files optdlg.src optload.src

${MYCMD} --base sw/source/ui/dbui --files dbui.src mailmergewizard.src mmaddressblockpage.src selectdbtabledialog.src

${MYCMD} --files sw/source/ui/dialog/dialog.src

${MYCMD} --files sw/source/ui/dochdl/dochdl.src

${MYCMD} --base sw/source/ui/docvw --files access.src annotation.src

${MYCMD} --base sw/source/ui/envelp --files envelp.src envprt.src label.src labfmt.src

${MYCMD} --files sw/source/ui/fldui/fldui.src

${MYCMD} --files sw/source/ui/frmdlg/frmui.src

${MYCMD} --files sw/source/ui/globdoc/globdoc.src

${MYCMD} --base sw/source/ui/index --files cnttab.src idxmrk.src

${MYCMD} --base sw/source/ui/misc --files glossary.src swruler.src

${MYCMD} --files sw/source/ui/shells/shells.src

${MYCMD} --files sw/source/ui/sidebar/PagePropertyPanel.src

${MYCMD} --base sw/source/ui/utlui --files poolfmt.src utlui.src

${MYCMD} --files sw/source/ui/web/web.src

${MYCMD} --files sw/source/uibase/dbui/mailmergechildwindow.src

${MYCMD} --files sw/source/uibase/dialog/regionsw.src

${MYCMD} --files sw/source/uibase/docvw/docvw.src

${MYCMD} --files sw/source/uibase/lingu/olmenu.src

${MYCMD} --files sw/source/uibase/misc/redlndlg.src

${MYCMD} --base sw/source/uibase/ribbar --files inputwin.src workctrl.src

${MYCMD} --files sw/source/uibase/uiview/view.src

${MYCMD} --base sw/source/uibase/utlui --files attrdesc.src initui.src navipi.src statusbar.src unotools.src

${MYCMD} --files sw/source/uibase/wrtsh/wrtsh.src

${MYCMD} --base uui/source --files alreadyopen.src filechanged.src ids.src lockfailed.src lockcorrupt.src nameclashdlg.src openlocked.src passworddlg.src passworderrs.src trylater.src

${MYCMD} --files wizards/source/euro/euro.src

${MYCMD} --files wizards/source/formwizard/dbwizres.src

${MYCMD} --files wizards/source/importwizard/importwi.src

${MYCMD} --files wizards/source/template/template.src

${MYCMD} --files xmlsecurity/source/component/warnbox.src

${MYCMD} --base xmlsecurity/source/dialogs --files certificateviewer.src digitalsignaturesdialog.src


# .xcu files

${MYCMD} --files connectivity/registry/ado/org/openoffice/Office/DataAccess/Drivers.xcu

${MYCMD} --files connectivity/registry/calc/org/openoffice/Office/DataAccess/Drivers.xcu

${MYCMD} --files connectivity/registry/dbase/org/openoffice/Office/DataAccess/Drivers.xcu

${MYCMD} --files connectivity/registry/evoab2/org/openoffice/Office/DataAccess/Drivers.xcu

${MYCMD} --files connectivity/registry/firebird/org/openoffice/Office/DataAccess/Drivers.xcu

${MYCMD} --files connectivity/registry/flat/org/openoffice/Office/DataAccess/Drivers.xcu

${MYCMD} --files connectivity/registry/hsqldb/org/openoffice/Office/DataAccess/Drivers.xcu

${MYCMD} --files connectivity/registry/jdbc/org/openoffice/Office/DataAccess/Drivers.xcu

${MYCMD} --files connectivity/registry/macab/org/openoffice/Office/DataAccess/Drivers.xcu

${MYCMD} --files connectivity/registry/mork/org/openoffice/Office/DataAccess/Drivers.xcu

${MYCMD} --files connectivity/registry/mysql/org/openoffice/Office/DataAccess/Drivers.xcu

${MYCMD} --files connectivity/registry/odbc/org/openoffice/Office/DataAccess/Drivers.xcu

${MYCMD} --files connectivity/registry/postgresql/org/openoffice/Office/DataAccess/Drivers.xcu

${MYCMD} --files dictionaries/af_ZA/dictionaries.xcu

${MYCMD} --files dictionaries/an_ES/dictionaries.xcu

${MYCMD} --files dictionaries/ar/dictionaries.xcu

${MYCMD} --files dictionaries/be_BY/dictionaries.xcu

${MYCMD} --files dictionaries/bg_BG/dictionaries.xcu

${MYCMD} --files dictionaries/bn_BD/dictionaries.xcu

${MYCMD} --files dictionaries/br_FR/dictionaries.xcu

${MYCMD} --files dictionaries/bs_BA/dictionaries.xcu

${MYCMD} --files dictionaries/da_DK/dictionaries.xcu

${MYCMD} --files dictionaries/de/dictionaries.xcu

${MYCMD} --files dictionaries/el_GR/dictionaries.xcu

${MYCMD} --files dictionaries/en/dialog/registry/data/org/openoffice/Office/OptionsDialog.xcu

${MYCMD} --base dictionaries/en --files dictionaries.xcu Linguistic.xcu

${MYCMD} --files dictionaries/es/dictionaries.xcu

${MYCMD} --files dictionaries/et_EE/dictionaries.xcu

${MYCMD} --files dictionaries/fr_FR/dictionaries.xcu

${MYCMD} --files dictionaries/gl/dictionaries.xcu

${MYCMD} --files dictionaries/gu_IN/dictionaries.xcu

${MYCMD} --files dictionaries/he_IL/dictionaries.xcu

${MYCMD} --files dictionaries/hi_IN/dictionaries.xcu

${MYCMD} --files dictionaries/hr_HR/dictionaries.xcu

${MYCMD} --files dictionaries/hu_HU/dialog/registry/data/org/openoffice/Office/OptionsDialog.xcu

${MYCMD} --base dictionaries/hu_HU --files dictionaries.xcu Linguistic.xcu

${MYCMD} --files dictionaries/is/dictionaries.xcu

${MYCMD} --files dictionaries/it_IT/dictionaries.xcu

${MYCMD} --files dictionaries/kmr_Latn/dictionaries.xcu

${MYCMD} --files dictionaries/lo_LA/dictionaries.xcu

${MYCMD} --files dictionaries/lt_LT/dictionaries.xcu

${MYCMD} --files dictionaries/lv_LV/dictionaries.xcu

${MYCMD} --files dictionaries/ne_NP/dictionaries.xcu

${MYCMD} --files dictionaries/nl_NL/dictionaries.xcu

${MYCMD} --files dictionaries/no/dictionaries.xcu

${MYCMD} --files dictionaries/oc_FR/dictionaries.xcu

${MYCMD} --files dictionaries/pl_PL/dictionaries.xcu

${MYCMD} --base  dictionaries/pt_BR --files dictionaries.xcu Linguistic.xcu

${MYCMD} --files dictionaries/pt_BR/dialog/OptionsDialog.xcu

${MYCMD} --files dictionaries/pt_BR/dialog/registry/data/org/openoffice/Office/OptionsDialog.xcu

${MYCMD} --files dictionaries/pt_PT/dictionaries.xcu

${MYCMD} --files dictionaries/ro/dictionaries.xcu

${MYCMD} --base dictionaries/ru_RU --files dictionaries.xcu Linguistic.xcu

${MYCMD} --files dictionaries/ru_RU/dialog/registry/data/org/openoffice/Office/OptionsDialog.xcu

${MYCMD} --files dictionaries/ru_RU/dialog/registry/data/org/openoffice/Office/OptionsDialog.xcu

${MYCMD} --files dictionaries/si_LK/dictionaries.xcu

${MYCMD} --files dictionaries/sk_SK/dictionaries.xcu

${MYCMD} --files dictionaries/sl_SI/dictionaries.xcu

${MYCMD} --files dictionaries/sr/dictionaries.xcu

${MYCMD} --files dictionaries/sv_SE/dictionaries.xcu

${MYCMD} --files dictionaries/sw_TZ/dictionaries.xcu

${MYCMD} --files dictionaries/te_IN/dictionaries.xcu

${MYCMD} --files dictionaries/th_TH/dictionaries.xcu

${MYCMD} --files dictionaries/uk_UA/dictionaries.xcu

${MYCMD} --files dictionaries/vi/dictionaries.xcu

${MYCMD} --files dictionaries/zu_ZA/dictionaries.xcu

${MYCMD} --base extensions/source/update/check/org/openoffice/Office --files Addons.xcu Jobs.xcu

${MYCMD} --base filter/source/config/fragments/filters --files AbiWord.xcu ADO_rowset_XML.xcu AppleKeynote.xcu AppleNumbers.xcu ApplePages.xcu BMP___MS_Windows.xcu BroadBand_eBook.xcu calc8.xcu calc8_template.xcu calc_Gnumeric.xcu calc_HTML_WebQuery.xcu calc_MS_Excel_2007_Binary.xcu calc_MS_Excel_2007_VBA_XML.xcu calc_MS_Excel_2007_XML.xcu calc_MS_Excel_2007_XML_Template.xcu calc_OOXML.xcu calc_OOXML_Template.xcu calc_pdf_Export.xcu calc_png_Export.xcu calc_StarOffice_XML_Calc_Template.xcu CGM___Computer_Graphics_Metafile.xcu chart8.xcu ClarisWorks.xcu ClarisWorks_Calc.xcu ClarisWorks_Draw.xcu ClarisWorks_Impress.xcu Claris_Resolve_Calc.xcu CorelDrawDocument.xcu CorelPresentationExchange.xcu dBase.xcu DIF.xcu DocBook_File.xcu DosWord.xcu draw8.xcu draw8_template.xcu draw_bmp_Export.xcu draw_emf_Export.xcu draw_eps_Export.xcu draw_flash_Export.xcu draw_gif_Export.xcu draw_html_Export.xcu draw_jpg_Export.xcu draw_PCD_Photo_CD_Base.xcu draw_PCD_Photo_CD_Base16.xcu draw_PCD_Photo_CD_Base4.xcu draw_pdf_Export.xcu draw_png_Export.xcu draw_StarOffice_XML_Draw_Template.xcu draw_svg_Export.xcu draw_tif_Export.xcu draw_wmf_Export.xcu DXF___AutoCAD_Interchange.xcu EMF___MS_Windows_Metafile.xcu EPS___Encapsulated_PostScript.xcu FictionBook_2.xcu FreehandDocument.xcu GIF___Graphics_Interchange.xcu HTML.xcu HTML_MasterDoc.xcu HTML__StarCalc_.xcu HTML__StarWriter_.xcu impress8.xcu impress8_draw.xcu impress8_template.xcu impress_bmp_Export.xcu impress_emf_Export.xcu impress_eps_Export.xcu impress_flash_Export.xcu impress_gif_Export.xcu impress_html_Export.xcu impress_jpg_Export.xcu impress_MS_PowerPoint_2007_XML.xcu impress_MS_PowerPoint_2007_XML_AutoPlay.xcu impress_MS_PowerPoint_2007_XML_Template.xcu impress_OOXML.xcu impress_OOXML_AutoPlay.xcu impress_OOXML_Template.xcu impress_pdf_Export.xcu impress_png_Export.xcu impress_StarOffice_XML_Draw.xcu impress_StarOffice_XML_Impress_Template.xcu impress_svg_Export.xcu impress_tif_Export.xcu impress_wmf_Export.xcu JPG___JPEG.xcu Lotus.xcu LotusWordPro.xcu MacWrite.xcu Mac_Word.xcu Mac_Works.xcu Mac_Works_Calc.xcu Mariner_Write.xcu math8.xcu MathML_XML__Math_.xcu MathType_3_x.xcu math_pdf_Export.xcu MET___OS_2_Metafile.xcu mov__MOV.xcu MS_Excel_2003_XML.xcu MS_Excel_4_0.xcu MS_Excel_4_0_Vorlage_Template.xcu MS_Excel_5_0_95.xcu MS_Excel_5_0_95_Vorlage_Template.xcu MS_Excel_95.xcu MS_Excel_95_Vorlage_Template.xcu MS_Excel_97.xcu MS_Excel_97_Vorlage_Template.xcu MS_PowerPoint_97.xcu MS_PowerPoint_97_AutoPlay.xcu MS_PowerPoint_97_Vorlage.xcu MS_WinWord_5.xcu MS_WinWord_6_0.xcu MS_Word_2003_XML.xcu MS_Word_2007_XML.xcu MS_Word_2007_XML_Template.xcu MS_Word_95.xcu MS_Word_95_Vorlage.xcu MS_Word_97.xcu MS_Word_97_Vorlage.xcu MS_Works.xcu MS_Works_Calc.xcu MS_Write.xcu MWAW_Bitmap.xcu MWAW_Database.xcu MWAW_Drawing.xcu MWAW_Presentation.xcu MWAW_Spreadsheet.xcu MWAW_Text_Document.xcu ODG_FlatXML.xcu ODP_FlatXML.xcu ODS_FlatXML.xcu ODT_FlatXML.xcu OOXML_Text.xcu OOXML_Text_Template.xcu PageMakerDocument.xcu PalmDoc.xcu Palm_Text_Document.xcu PBM___Portable_Bitmap.xcu PCT___Mac_Pict.xcu PCX___Zsoft_Paintbrush.xcu PGM___Portable_Graymap.xcu Plucker_eBook.xcu PNG___Portable_Network_Graphic.xcu PPM___Portable_Pixelmap.xcu PSD___Adobe_Photoshop.xcu PublisherDocument.xcu QPro.xcu RAS___Sun_Rasterfile.xcu Rich_Text_Format.xcu Rich_Text_Format__StarCalc_.xcu SGF___StarOffice_Writer_SGF.xcu SGV___StarDraw_2_0.xcu StarBaseReport.xcu StarBaseReportChart.xcu StarOffice_XML__Base_.xcu StarOffice_XML__Calc_.xcu StarOffice_XML__Chart_.xcu StarOffice_XML__Draw_.xcu StarOffice_XML__Impress_.xcu StarOffice_XML__Math_.xcu StarOffice_XML__Writer_.xcu SVG___Scalable_Vector_Graphics.xcu SVM___StarView_Metafile.xcu SYLK.xcu T602Document.xcu Text.xcu Text__encoded_.xcu Text__encoded___StarWriter_GlobalDocument_.xcu Text__encoded___StarWriter_Web_.xcu Text__StarWriter_Web_.xcu Text___txt___csv__StarCalc_.xcu TGA___Truevision_TARGA.xcu TIF___Tag_Image_File.xcu UOF_presentation.xcu UOF_spreadsheet.xcu UOF_text.xcu VisioDocument.xcu WMF___MS_Windows_Metafile.xcu WordPerfect.xcu WordPerfectGraphics.xcu WPS_Lotus_Calc.xcu WPS_QPro_Calc.xcu WriteNow.xcu writer8.xcu writer8_template.xcu writerglobal8.xcu writerglobal8_HTML.xcu writerglobal8_template.xcu writerglobal8_writer.xcu writerweb8_writer.xcu writerweb8_writer_template.xcu writer_globaldocument_pdf_Export.xcu writer_globaldocument_StarOffice_XML_Writer.xcu writer_globaldocument_StarOffice_XML_Writer_GlobalDocument.xcu writer_jpg_Export.xcu writer_layout_dump.xcu writer_MIZI_Hwp_97.xcu writer_pdf_Export.xcu writer_png_Export.xcu writer_StarOffice_XML_Writer_Template.xcu writer_web_HTML_help.xcu writer_web_jpg_Export.xcu writer_web_pdf_Export.xcu writer_web_png_Export.xcu writer_web_StarOffice_XML_Writer.xcu writer_web_StarOffice_XML_Writer_Web_Template.xcu XBM___X_Consortium.xcu XHTML_Calc_File.xcu XHTML_Draw_File.xcu XHTML_Impress_File.xcu XHTML_Writer_File.xcu XPM.xcu

${MYCMD} --base filter/source/config/fragments/internalgraphicfilters --files bmp_Export.xcu bmp_Import.xcu dxf_Import.xcu emf_Export.xcu emf_Import.xcu eps_Export.xcu eps_Import.xcu gif_Export.xcu gif_Import.xcu jpg_Export.xcu jpg_Import.xcu met_Import.xcu mov_Import.xcu pbm_Import.xcu pcd_Import_Base.xcu pcd_Import_Base16.xcu pcd_Import_Base4.xcu pct_Import.xcu pcx_Import.xcu pgm_Import.xcu png_Export.xcu png_Import.xcu ppm_Import.xcu psd_Import.xcu ras_Import.xcu sgf_Import.xcu sgv_Import.xcu svg_Export.xcu svg_Import.xcu svm_Export.xcu svm_Import.xcu tga_Import.xcu tif_Export.xcu tif_Import.xcu wmf_Export.xcu wmf_Import.xcu xbm_Import.xcu xpm_Import.xcu

${MYCMD} --base filter/source/config/fragments/types --files bmp_MS_Windows.xcu calc8.xcu calc8_template.xcu calc_ADO_rowset_XML.xcu calc_AppleNumbers.xcu calc_ClarisWorks.xcu calc_Claris_Resolve.xcu calc_dBase.xcu calc_DIF.xcu calc_Gnumeric.xcu calc_HTML.xcu calc_Lotus.xcu calc_Mac_Works.xcu calc_MS_Excel_2003_XML.xcu calc_MS_Excel_40.xcu calc_MS_Excel_40_VorlageTemplate.xcu calc_MS_Excel_5095.xcu calc_MS_Excel_5095_VorlageTemplate.xcu calc_MS_Excel_95.xcu calc_MS_Excel_95_VorlageTemplate.xcu calc_MS_Excel_97.xcu calc_MS_Excel_97_VorlageTemplate.xcu calc_MS_Works_Document.xcu calc_ODS_FlatXML.xcu calc_OOXML.xcu calc_OOXML_Template.xcu calc_QPro.xcu calc_StarOffice_XML_Calc.xcu calc_StarOffice_XML_Calc_Template.xcu calc_SYLK.xcu calc_WPS_Lotus_Document.xcu calc_WPS_QPro_Document.xcu chart8.xcu chart_StarOffice_XML_Chart.xcu component_Bibliography.xcu component_DB.xcu draw8.xcu draw8_template.xcu draw_ClarisWorks.xcu draw_CorelDraw_Document.xcu draw_Corel_Presentation_Exchange.xcu draw_Freehand_Document.xcu draw_ODG_FlatXML.xcu draw_PageMaker_Document.xcu draw_Publisher_Document.xcu draw_StarOffice_XML_Draw.xcu draw_StarOffice_XML_Draw_Template.xcu draw_Visio_Document.xcu draw_WordPerfect_Graphics.xcu dxf_AutoCAD_Interchange.xcu emf_MS_Windows_Metafile.xcu eps_Encapsulated_PostScript.xcu generic_HTML.xcu generic_Text.xcu gif_Graphics_Interchange.xcu graphic_HTML.xcu graphic_SWF.xcu impress8.xcu impress8_template.xcu impress_AppleKeynote.xcu impress_CGM_Computer_Graphics_Metafile.xcu impress_ClarisWorks.xcu impress_MS_PowerPoint_97.xcu impress_MS_PowerPoint_97_AutoPlay.xcu impress_MS_PowerPoint_97_Vorlage.xcu impress_ODP_FlatXML.xcu impress_OOXML_Presentation.xcu impress_OOXML_Presentation_AutoPlay.xcu impress_OOXML_Presentation_Template.xcu impress_StarOffice_XML_Impress.xcu impress_StarOffice_XML_Impress_Template.xcu jpg_JPEG.xcu math8.xcu math_MathML_XML_Math.xcu math_MathType_3x.xcu math_StarOffice_XML_Math.xcu met_OS2_Metafile.xcu mov_MOV.xcu MS_Excel_2007_Binary.xcu MS_Excel_2007_VBA_XML.xcu MS_Excel_2007_XML.xcu MS_Excel_2007_XML_Template.xcu MS_PowerPoint_2007_XML.xcu MS_PowerPoint_2007_XML_AutoPlay.xcu MS_PowerPoint_2007_XML_Template.xcu MWAW_Bitmap.xcu MWAW_Database.xcu MWAW_Drawing.xcu MWAW_Presentation.xcu MWAW_Spreadsheet.xcu MWAW_Text_Document.xcu oxt_OpenOffice_Extension.xcu Palm_Text_Document.xcu pbm_Portable_Bitmap.xcu pcd_Photo_CD_Base.xcu pcd_Photo_CD_Base16.xcu pcd_Photo_CD_Base4.xcu pct_Mac_Pict.xcu pcx_Zsoft_Paintbrush.xcu pdf_Portable_Document_Format.xcu pgm_Portable_Graymap.xcu png_Portable_Network_Graphic.xcu ppm_Portable_Pixelmap.xcu psd_Adobe_Photoshop.xcu ras_Sun_Rasterfile.xcu sgf_StarOffice_Writer_SGF.xcu sgv_StarDraw_20.xcu StarBase.xcu StarBaseReport.xcu StarBaseReportChart.xcu svg_Scalable_Vector_Graphics.xcu svm_StarView_Metafile.xcu tga_Truevision_TARGA.xcu tif_Tag_Image_File.xcu Unified_Office_Format_presentation.xcu Unified_Office_Format_spreadsheet.xcu Unified_Office_Format_text.xcu wav_Wave_Audio_File.xcu wmf_MS_Windows_Metafile.xcu writer8.xcu writer8_template.xcu writerglobal8.xcu writerglobal8_template.xcu writerweb8_writer_template.xcu writer_AbiWord_Document.xcu writer_ApplePages.xcu writer_BroadBand_eBook.xcu writer_ClarisWorks.xcu writer_DocBook_File.xcu writer_DosWord.xcu writer_FictionBook_2.xcu writer_globaldocument_StarOffice_XML_Writer_GlobalDocument.xcu writer_layout_dump_xml.xcu writer_LotusWordPro_Document.xcu writer_MacWrite.xcu writer_Mac_Word.xcu writer_Mac_Works.xcu writer_Mariner_Write.xcu writer_MIZI_Hwp_97.xcu writer_MS_WinWord_5.xcu writer_MS_WinWord_60.xcu writer_MS_Word_2003_XML.xcu writer_MS_Word_2007_XML.xcu writer_MS_Word_2007_XML_Template.xcu writer_MS_Word_95.xcu writer_MS_Word_95_Vorlage.xcu writer_MS_Word_97.xcu writer_MS_Word_97_Vorlage.xcu writer_MS_Works_Document.xcu writer_MS_Write.xcu writer_ODT_FlatXML.xcu writer_OOXML.xcu writer_OOXML_Template.xcu writer_PalmDoc.xcu writer_Plucker_eBook.xcu writer_Rich_Text_Format.xcu writer_StarOffice_XML_Writer.xcu writer_StarOffice_XML_Writer_Template.xcu writer_T602_Document.xcu writer_web_HTML_help.xcu writer_web_StarOffice_XML_Writer_Web_Template.xcu writer_WordPerfect_Document.xcu writer_WriteNow.xcu xbm_X_Consortium.xcu XHTML_File.xcu xpm_XPM.xcu

${MYCMD} --files mysqlc/source/DataAccess.xcu

${MYCMD} --files mysqlc/source/registry/data/org/openoffice/Office/DataAccess/Drivers.xcu

${MYCMD} --base officecfg/registry/data/org/openoffice --files Inet.xcu Interaction.xcu

${MYCMD} --base officecfg/registry/data/org/openoffice/Office --files Accelerators.xcu Addons.xcu BasicIDE.xcu Calc.xcu Canvas.xcu Common.xcu Compatibility.xcu DataAccess.xcu Embedding.xcu ExtendedColorScheme.xcu ExtensionDependencies.xcu ExtensionManager.xcu FormWizard.xcu Histories.xcu Impress.xcu Jobs.xcu Linguistic.xcu Logging.xcu Math.xcu Paths.xcu PresentationMinimizer.xcu PresenterScreen.xcu ProtocolHandler.xcu Recovery.xcu ReportDesign.xcu Scripting.xcu Security.xcu TableWizard.xcu TypeDetection.xcu

${MYCMD} --base officecfg/registry/data/org/openoffice/Office/UI --files BaseWindowState.xcu BasicIDECommands.xcu BasicIDEWindowState.xcu BibliographyCommands.xcu CalcCommands.xcu CalcWindowState.xcu ChartCommands.xcu ChartWindowState.xcu Controller.xcu DbBrowserWindowState.xcu DbQueryWindowState.xcu DbRelationWindowState.xcu DbReportWindowState.xcu DbTableDataWindowState.xcu DbTableWindowState.xcu DbuCommands.xcu DrawImpressCommands.xcu DrawWindowState.xcu Effects.xcu Factories.xcu GenericCategories.xcu GenericCommands.xcu ImpressWindowState.xcu MathCommands.xcu MathWindowState.xcu ReportCommands.xcu Sidebar.xcu StartModuleCommands.xcu StartModuleWindowState.xcu WriterCommands.xcu WriterFormWindowState.xcu WriterGlobalWindowState.xcu WriterReportWindowState.xcu WriterWebWindowState.xcu WriterWindowState.xcu XFormsWindowState.xcu

${MYCMD} --files officecfg/registry/data/org/openoffice/Office/UI.xcu

${MYCMD} --base officecfg/registry/data/org/openoffice/Office --files Views.xcu Writer.xcu

${MYCMD} --base officecfg/registry/data/org/openoffice --files Setup.xcu System.xcu

${MYCMD} --base officecfg/registry/data/org/openoffice --files UserProfile.xcu VCL.xcu

${MYCMD} --files swext/mediawiki/src/registry/data/org/openoffice/Office/Addons.xcu

${MYCMD} --files swext/mediawiki/src/registry/data/org/openoffice/Office/Custom/WikiExtension.xcu

${MYCMD} --base swext/mediawiki/src/registry/data/org/openoffice/Office --files OptionsDialog.xcu Paths.xcu ProtocolHandler.xcu

${MYCMD} --target workdir/jan/swext/mediawiki/src.pot --base swext/mediawiki/src/registry/data/org/openoffice/TypeDetection --files Filter.xcu Types.xcu


# .ui files

${MYCMD} --base basctl/uiconfig/basicide/ui --files basicmacrodialog.ui defaultlanguage.ui deletelangdialog.ui dialogpage.ui exportdialog.ui gotolinedialog.ui importlibdialog.ui libpage.ui managebreakpoints.ui managelanguages.ui modulepage.ui newlibdialog.ui organizedialog.ui

${MYCMD} --base chart2/uiconfig/ui --files 3dviewdialog.ui attributedialog.ui chardialog.ui chartdatadialog.ui charttypedialog.ui datarangedialog.ui dlg_DataLabel.ui dlg_InsertErrorBars.ui dlg_InsertLegend.ui insertaxisdlg.ui insertgriddlg.ui inserttitledlg.ui paradialog.ui sidebaraxis.ui sidebarelements.ui sidebarerrorbar.ui sidebarseries.ui smoothlinesdlg.ui steppedlinesdlg.ui titlerotationtabpage.ui tp_3D_SceneAppearance.ui tp_3D_SceneGeometry.ui tp_3D_SceneIllumination.ui tp_axisLabel.ui tp_AxisPositions.ui tp_ChartType.ui tp_DataLabel.ui tp_DataSource.ui tp_ErrorBars.ui tp_LegendPosition.ui tp_PolarOptions.ui tp_RangeChooser.ui tp_Scale.ui tp_SeriesToAxis.ui tp_Trendline.ui wizelementspage.ui

${MYCMD} --base cui/uiconfig/ui --files aboutconfigdialog.ui aboutconfigvaluedialog.ui aboutdialog.ui accelconfigpage.ui acorexceptpage.ui acoroptionspage.ui acorreplacepage.ui agingdialog.ui applyautofmtpage.ui applylocalizedpage.ui areadialog.ui areatabpage.ui asiantypography.ui assigncomponentdialog.ui autocorrectdialog.ui backgroundpage.ui baselinksdialog.ui bitmaptabpage.ui blackorwhitelistentrydialog.ui borderareatransparencydialog.ui borderbackgrounddialog.ui borderpage.ui breaknumberoption.ui calloutdialog.ui calloutpage.ui cellalignment.ui certdialog.ui charnamepage.ui colorconfigwin.ui colorpage.ui colorpickerdialog.ui comment.ui connectortabpage.ui connpooloptions.ui croppage.ui cuiimapdlg.ui customizedialog.ui databaselinkdialog.ui dbregisterpage.ui dimensionlinestabpage.ui distributiondialog.ui distributionpage.ui editdictionarydialog.ui editmodulesdialog.ui effectspage.ui embossdialog.ui eventassigndialog.ui eventassignpage.ui eventsconfigpage.ui fmsearchdialog.ui formatcellsdialog.ui formatnumberdialog.ui galleryapplyprogress.ui galleryfilespage.ui gallerygeneralpage.ui gallerysearchprogress.ui gallerythemedialog.ui gallerythemeiddialog.ui gallerytitledialog.ui galleryupdateprogress.ui gradientpage.ui hangulhanjaadddialog.ui hangulhanjaconversiondialog.ui hangulhanjaeditdictdialog.ui hangulhanjaoptdialog.ui hatchpage.ui hyperlinkdialog.ui hyperlinkdocpage.ui hyperlinkinternetpage.ui hyperlinkmailpage.ui hyperlinkmarkdialog.ui hyperlinknewdocpage.ui hyphenate.ui iconchangedialog.ui iconselectordialog.ui insertfloatingframe.ui insertoleobject.ui insertrowcolumn.ui javaclasspathdialog.ui javastartparametersdialog.ui javastartparameterseditdialog.ui linedialog.ui lineendstabpage.ui linestyletabpage.ui linetabpage.ui macroassigndialog.ui macroassignpage.ui macroselectordialog.ui menuassignpage.ui messbox.ui mosaicdialog.ui movemenu.ui multipathdialog.ui namedialog.ui newlibdialog.ui newtabledialog.ui newtoolbardialog.ui numberingformatpage.ui numberingoptionspage.ui numberingpositionpage.ui objectnamedialog.ui objecttitledescdialog.ui optaccessibilitypage.ui optadvancedpage.ui optappearancepage.ui optasianpage.ui optbasicidepage.ui optchartcolorspage.ui optctlpage.ui optemailpage.ui optfltrembedpage.ui optfltrpage.ui optfontspage.ui optgeneralpage.ui opthtmlpage.ui optionsdialog.ui optjsearchpage.ui optlanguagespage.ui optlingupage.ui optmemorypage.ui optnewdictionarydialog.ui optonlineupdatepage.ui optopenclpage.ui optpathspage.ui optproxypage.ui optsavepage.ui optsecuritypage.ui optuserpage.ui optviewpage.ui pageformatpage.ui paragalignpage.ui paraindentspacing.ui paratabspage.ui password.ui pastespecial.ui percentdialog.ui personalization_tab.ui pickbulletpage.ui pickgraphicpage.ui picknumberingpage.ui pickoutlinepage.ui positionpage.ui positionsizedialog.ui possizetabpage.ui posterdialog.ui querychangelineenddialog.ui querydeletebitmapdialog.ui querydeletechartcolordialog.ui querydeletecolordialog.ui querydeletedictionarydialog.ui querydeletegradientdialog.ui querydeletehatchdialog.ui querydeletelineenddialog.ui querydeletelinestyledialog.ui queryduplicatedialog.ui querynoloadedfiledialog.ui querynosavefiledialog.ui querysavelistdialog.ui queryupdategalleryfilelistdialog.ui recordnumberdialog.ui rotationtabpage.ui scriptorganizer.ui searchattrdialog.ui searchformatdialog.ui securityoptionsdialog.ui selectpathdialog.ui select_persona_dialog.ui shadowtabpage.ui showcoldialog.ui similaritysearchdialog.ui slantcornertabpage.ui smarttagoptionspage.ui smoothdialog.ui solarizedialog.ui specialcharacters.ui spellingdialog.ui spelloptionsdialog.ui splitcellsdialog.ui storedwebconnectiondialog.ui swpossizepage.ui textanimtabpage.ui textattrtabpage.ui textdialog.ui textflowpage.ui thesaurus.ui transparencytabpage.ui tsaurldialog.ui twolinespage.ui wordcompletionpage.ui zoomdialog.ui

${MYCMD} --base dbaccess/uiconfig/ui --files admindialog.ui advancedsettingsdialog.ui applycolpage.ui authentificationpage.ui autocharsetpage.ui backuppage.ui choosedatasourcedialog.ui collectionviewdialog.ui colwidthdialog.ui connectionpage.ui copytablepage.ui dbaseindexdialog.ui dbasepage.ui dbwizconnectionpage.ui dbwizmysqlintropage.ui dbwizmysqlnativepage.ui dbwizspreadsheetpage.ui dbwiztextpage.ui deleteallrowsdialog.ui designsavemodifieddialog.ui directsqldialog.ui emptypage.ui fielddialog.ui finalpagewizard.ui generalpagedialog.ui generalpagewizard.ui generalspecialjdbcdetailspage.ui generatedvaluespage.ui indexdesigndialog.ui jdbcconnectionpage.ui joindialog.ui ldapconnectionpage.ui ldappage.ui migratepage.ui mysqlnativepage.ui mysqlnativesettings.ui namematchingpage.ui odbcpage.ui parametersdialog.ui password.ui preparepage.ui queryfilterdialog.ui querypropertiesdialog.ui relationdialog.ui rowheightdialog.ui rtfcopytabledialog.ui savedialog.ui saveindexdialog.ui savemodifieddialog.ui sortdialog.ui specialjdbcconnectionpage.ui specialsettingspage.ui sqlexception.ui summarypage.ui tabledesignsavemodifieddialog.ui tablesfilterdialog.ui tablesfilterpage.ui tablesjoindialog.ui textconnectionsettings.ui textpage.ui typeselectpage.ui useradmindialog.ui useradminpage.ui userdetailspage.ui

${MYCMD} --base desktop/uiconfig/ui --files dependenciesdialog.ui extensionmanager.ui installforalldialog.ui licensedialog.ui showlicensedialog.ui updatedialog.ui updateinstalldialog.ui updaterequireddialog.ui

${MYCMD} --base extensions/uiconfig/sabpilot/ui --files contentfieldpage.ui contenttablepage.ui datasourcepage.ui defaultfieldselectionpage.ui fieldassignpage.ui fieldlinkpage.ui gridfieldsselectionpage.ui groupradioselectionpage.ui invokeadminpage.ui optiondbfieldpage.ui optionsfinalpage.ui optionvaluespage.ui selecttablepage.ui selecttypepage.ui tableselectionpage.ui

${MYCMD} --base extensions/uiconfig/sbibliography/ui --files choosedatasourcedialog.ui generalpage.ui mappingdialog.ui toolbar.ui

${MYCMD} --base extensions/uiconfig/scanner/ui --files griddialog.ui sanedialog.ui

${MYCMD} --base extensions/uiconfig/spropctrlr/ui --files controlfontdialog.ui datatypedialog.ui fieldlinkrow.ui formlinksdialog.ui labelselectiondialog.ui listselectdialog.ui taborder.ui

${MYCMD} --base filter/uiconfig/ui --files impswfdialog.ui pdfgeneralpage.ui pdflinkspage.ui pdfoptionsdialog.ui pdfsecuritypage.ui pdfsignpage.ui pdfuserinterfacepage.ui pdfviewpage.ui testxmlfilter.ui warnpdfdialog.ui xmlfiltersettings.ui xmlfiltertabpagegeneral.ui xmlfiltertabpagetransformation.ui xsltfilterdialog.ui

${MYCMD} --base formula/uiconfig/ui --files formuladialog.ui functionpage.ui parameter.ui structpage.ui

${MYCMD} --base fpicker/uiconfig/ui --files explorerfiledialog.ui foldernamedialog.ui remotefilesdialog.ui

${MYCMD} --base reportdesign/uiconfig/dbreport/ui --files backgrounddialog.ui chardialog.ui condformatdialog.ui conditionwin.ui datetimedialog.ui floatingnavigator.ui floatingsort.ui pagedialog.ui pagenumberdialog.ui

${MYCMD} --base sc/uiconfig/scalc/ui --files advancedfilterdialog.ui allheaderfooterdialog.ui analysisofvariancedialog.ui autoformattable.ui cellprotectionpage.ui changesourcedialog.ui chardialog.ui chisquaretestdialog.ui colorrowdialog.ui colwidthdialog.ui condformatmanager.ui conditionalformatdialog.ui conflictsdialog.ui consolidatedialog.ui correlationdialog.ui covariancedialog.ui createnamesdialog.ui dapiservicedialog.ui databaroptions.ui datafielddialog.ui datafieldoptionsdialog.ui dataform.ui datastreams.ui definedatabaserangedialog.ui definename.ui deletecells.ui deletecontents.ui descriptivestatisticsdialog.ui doubledialog.ui erroralerttabpage.ui exponentialsmoothingdialog.ui externaldata.ui filldlg.ui footerdialog.ui formatcellsdialog.ui formulacalculationoptions.ui goalseekdlg.ui groupbydate.ui groupbynumber.ui groupdialog.ui headerdialog.ui headerfootercontent.ui headerfooterdialog.ui imoptdialog.ui inputstringdialog.ui insertcells.ui insertname.ui insertsheet.ui integerdialog.ui leftfooterdialog.ui leftheaderdialog.ui managenamesdialog.ui movecopysheet.ui movingaveragedialog.ui multipleoperationsdialog.ui namerangesdialog.ui nosolutiondialog.ui optcalculatepage.ui optchangespage.ui optcompatibilitypage.ui optdefaultpage.ui optdlg.ui optformula.ui optimalcolwidthdialog.ui optimalrowheightdialog.ui optsortlists.ui pagetemplatedialog.ui paradialog.ui paratemplatedialog.ui pastespecial.ui pivotfielddialog.ui pivotfilterdialog.ui pivottablelayoutdialog.ui printareasdialog.ui printeroptions.ui protectsheetdlg.ui queryrunstreamscriptdialog.ui randomnumbergenerator.ui regressiondialog.ui retypepassdialog.ui retypepassworddialog.ui rightfooterdialog.ui rightheaderdialog.ui rowheightdialog.ui samplingdialog.ui scenariodialog.ui scgeneralpage.ui searchresults.ui selectdatasource.ui selectrange.ui selectsource.ui sharedfooterdialog.ui sharedheaderdialog.ui sharedocumentdlg.ui sheetprintpage.ui showchangesdialog.ui showdetaildialog.ui showsheetdialog.ui sidebaralignment.ui sidebarcellappearance.ui sidebarnumberformat.ui simplerefdialog.ui solverdlg.ui solveroptionsdialog.ui solverprogressdialog.ui solversuccessdialog.ui sortcriteriapage.ui sortdialog.ui sortkey.ui sortoptionspage.ui sortwarning.ui standardfilterdialog.ui statisticsinfopage.ui subtotaldialog.ui subtotalgrppage.ui subtotaloptionspage.ui tabcolordialog.ui textimportcsv.ui textimportoptions.ui tpviewpage.ui ttestdialog.ui ungroupdialog.ui validationcriteriapage.ui validationdialog.ui validationhelptabpage.ui xmlsourcedialog.ui ztestdialog.ui

${MYCMD} --base sd/uiconfig/sdraw/ui --files breakdialog.ui bulletsandnumbering.ui copydlg.ui crossfadedialog.ui dlgsnap.ui drawchardialog.ui drawpagedialog.ui drawparadialog.ui drawprtldialog.ui insertlayer.ui insertslidesdialog.ui namedesign.ui paranumberingtab.ui printeroptions.ui queryunlinkimagedialog.ui tabledesigndialog.ui vectorize.ui

${MYCMD} --base sd/uiconfig/simpress/ui --files assistentdialog.ui customanimationeffecttab.ui customanimationproperties.ui customanimationspanel.ui customanimationtexttab.ui customanimationtimingtab.ui customslideshows.ui definecustomslideshow.ui dlgfield.ui dockinganimation.ui headerfooterdialog.ui headerfootertab.ui insertslides.ui interactiondialog.ui interactionpage.ui masterlayoutdlg.ui optimpressgeneralpage.ui photoalbum.ui presentationdialog.ui printeroptions.ui prntopts.ui publishingdialog.ui remotedialog.ui sdviewpage.ui slidedesigndialog.ui slidetransitionspanel.ui tabledesignpanel.ui templatedialog.ui

${MYCMD} --base sfx2/uiconfig/ui --files alienwarndialog.ui bookmarkdialog.ui checkin.ui cmisinfopage.ui cmisline.ui custominfopage.ui descriptioninfopage.ui documentfontspage.ui documentinfopage.ui documentpropertiesdialog.ui editdurationdialog.ui errorfindemaildialog.ui floatingrecord.ui helpbookmarkpage.ui helpcontentpage.ui helpcontrol.ui helpindexpage.ui helpsearchpage.ui inputdialog.ui licensedialog.ui linkeditdialog.ui loadtemplatedialog.ui managestylepage.ui newstyle.ui optprintpage.ui password.ui printeroptionsdialog.ui querysavedialog.ui searchdialog.ui securityinfopage.ui singletabdialog.ui startcenter.ui templatedlg.ui templateinfodialog.ui versioncommentdialog.ui versionscmis.ui versionsofdialog.ui

${MYCMD} --base starmath/uiconfig/smath/ui --files alignmentdialog.ui catalogdialog.ui dockingelements.ui fontdialog.ui fontsizedialog.ui fonttypedialog.ui printeroptions.ui savedefaultsdialog.ui smathsettings.ui spacingdialog.ui symdefinedialog.ui

${MYCMD} --base svtools/uiconfig/ui --files addresstemplatedialog.ui graphicexport.ui GraphicExportOptionsDialog.ui javadisableddialog.ui placeedit.ui printersetupdialog.ui querydeletedialog.ui restartdialog.ui wizarddialog.ui

${MYCMD} --base svx/uiconfig/ui --files acceptrejectchangesdialog.ui addconditiondialog.ui adddataitemdialog.ui addinstancedialog.ui addmodeldialog.ui addnamespacedialog.ui addsubmissiondialog.ui asianphoneticguidedialog.ui chineseconversiondialog.ui chinesedictionary.ui colorwindow.ui compressgraphicdialog.ui crashreportdlg.ui datanavigator.ui deletefooterdialog.ui deleteheaderdialog.ui docking3deffects.ui dockingcolorreplace.ui dockingfontwork.ui docrecoverybrokendialog.ui docrecoveryprogressdialog.ui docrecoveryrecoverdialog.ui docrecoverysavedialog.ui extrustiondepthdialog.ui findreplacedialog.ui floatingcontour.ui floatingundoredo.ui fontworkgallerydialog.ui fontworkspacingdialog.ui formlinkwarndialog.ui headfootformatpage.ui imapdialog.ui linkwarndialog.ui namespacedialog.ui optgridpage.ui paralinespacingcontrol.ui passwd.ui querydeletecontourdialog.ui querydeleteobjectdialog.ui querydeletethemedialog.ui querymodifyimagemapchangesdialog.ui querynewcontourdialog.ui querysavecontchangesdialog.ui querysaveimagemapchangesdialog.ui queryunlinkgraphicsdialog.ui redlinecontrol.ui redlinefilterpage.ui redlineviewpage.ui savemodifieddialog.ui sidebararea.ui sidebargraphic.ui sidebarinsert.ui sidebarline.ui sidebarparagraph.ui sidebarpossize.ui sidebarshadow.ui sidebarstylespanel.ui sidebartextpanel.ui textcontrolchardialog.ui textcontrolparadialog.ui xformspage.ui

${MYCMD} --base sw/uiconfig/swriter/ui --files abstractdialog.ui addentrydialog.ui addressblockdialog.ui alreadyexistsdialog.ui asciifilterdialog.ui asksearchdialog.ui assignfieldsdialog.ui assignstylesdialog.ui attachnamedialog.ui authenticationsettingsdialog.ui autoformattable.ui autotext.ui bibliographyentry.ui bulletsandnumbering.ui businessdatapage.ui cannotsavelabeldialog.ui captiondialog.ui captionoptions.ui cardformatpage.ui cardmediumpage.ui ccdialog.ui characterproperties.ui charurlpage.ui columndialog.ui columnpage.ui columnwidth.ui conditionpage.ui converttexttable.ui createaddresslist.ui createauthorentry.ui createautomarkdialog.ui customizeaddrlistdialog.ui datasourcesunavailabledialog.ui dropcapspage.ui dropdownfielddialog.ui editcategories.ui editfielddialog.ui editsectiondialog.ui endnotepage.ui envaddresspage.ui envdialog.ui envformatpage.ui envprinterpage.ui exchangedatabases.ui fielddialog.ui findentrydialog.ui flddbpage.ui flddocinfopage.ui flddocumentpage.ui fldfuncpage.ui fldrefpage.ui fldvarpage.ui floatingnavigation.ui floatingsync.ui footendnotedialog.ui footnoteareapage.ui footnotepage.ui footnotesendnotestabpage.ui formatsectiondialog.ui formattablepage.ui framedialog.ui frmaddpage.ui frmtypepage.ui frmurlpage.ui indentpage.ui indexentry.ui infonotfounddialog.ui inforeadonlydialog.ui inputfielddialog.ui insertautotextdialog.ui insertbookmark.ui insertbreak.ui insertcaption.ui insertdbcolumnsdialog.ui insertfootnote.ui insertscript.ui insertsectiondialog.ui inserttable.ui labeldialog.ui labelformatpage.ui labeloptionspage.ui linenumbering.ui mailconfigpage.ui mailmerge.ui mailmergedialog.ui managechangessidebar.ui mergeconnectdialog.ui mergetabledialog.ui mmaddressblockpage.ui mmcreatingdialog.ui mmlayoutpage.ui mmmailbody.ui mmoutputtypepage.ui mmresultemaildialog.ui mmresultprintdialog.ui mmresultsavedialog.ui mmsalutationpage.ui mmselectpage.ui mmsendmails.ui newuserindexdialog.ui numberingnamedialog.ui numparapage.ui objectdialog.ui optcaptionpage.ui optcomparison.ui optcompatpage.ui optfonttabpage.ui optformataidspage.ui optgeneralpage.ui optredlinepage.ui opttablepage.ui opttestpage.ui outlinenumbering.ui outlinenumberingpage.ui outlinepositionpage.ui paradialog.ui picturedialog.ui picturepage.ui previewzoomdialog.ui printeroptions.ui printmergedialog.ui printmonitordialog.ui printoptionspage.ui privateuserpage.ui querycontinuebegindialog.ui querycontinueenddialog.ui querydefaultcompatdialog.ui queryrotateintostandarddialog.ui querysavelabeldialog.ui queryshowchangesdialog.ui renameautotextdialog.ui renameentrydialog.ui renameobjectdialog.ui rowheight.ui saveashtmldialog.ui savelabeldialog.ui sectionpage.ui selectaddressdialog.ui selectautotextdialog.ui selectblockdialog.ui selectindexdialog.ui selecttabledialog.ui sidebarpage.ui sidebarstylepresets.ui sidebartheme.ui sidebarwrap.ui sortdialog.ui splittable.ui statisticsinfopage.ui stringinput.ui subjectdialog.ui tablecolumnpage.ui tablepreviewdialog.ui tableproperties.ui tabletextflowpage.ui templatedialog1.ui templatedialog16.ui templatedialog2.ui templatedialog4.ui templatedialog8.ui testmailsettings.ui textgridpage.ui titlepage.ui tocdialog.ui tocentriespage.ui tocindexpage.ui tocstylespage.ui tokenwidget.ui viewoptionspage.ui warndatasourcedialog.ui warnemaildialog.ui wordcount.ui wrapdialog.ui wrappage.ui

${MYCMD} --base uui/uiconfig/ui --files authfallback.ui filterselect.ui logindialog.ui macrowarnmedium.ui masterpassworddlg.ui password.ui setmasterpassworddlg.ui simplenameclash.ui sslwarndialog.ui unknownauthdialog.ui

${MYCMD} --base vcl/uiconfig/ui --files cupspassworddialog.ui errornocontentdialog.ui errornoprinterdialog.ui notebookbar.ui printdialog.ui printerdevicepage.ui printerpaperpage.ui printerpropertiesdialog.ui printprogressdialog.ui querydialog.ui screenshotannotationdialog.ui

${MYCMD} --base xmlsecurity/uiconfig/ui --files certdetails.ui certgeneral.ui certpage.ui digitalsignaturesdialog.ui macrosecuritydialog.ui securitylevelpage.ui securitytrustpage.ui selectcertificatedialog.ui viewcertdialog.ui


# .xml files

${MYCMD} --files nlpsolver/src/com/sun/star/comp/Calc/NLPSolver/description.xml

${MYCMD} --files dictionaries/pt_BR/description.xml
