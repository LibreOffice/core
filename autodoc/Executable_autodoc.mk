# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
$(eval $(call gb_Executable_Executable,autodoc))

$(eval $(call gb_Executable_set_include,autodoc,\
    -I$(SRCDIR)/autodoc/inc \
    -I$(SRCDIR)/autodoc/source/inc \
    -I$(SRCDIR)/autodoc/source/parser/inc \
    -I$(SRCDIR)/autodoc/source/display/inc \
    -I$(SRCDIR)/autodoc/source/parser_i/inc \
    -I$(SRCDIR)/autodoc/source/ary/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_static_libraries,autodoc,\
    cosv \
    udm \
))

$(eval $(call gb_Executable_add_exception_objects,autodoc,\
    autodoc/source/exes/adc_uni/adc_cl \
    autodoc/source/exes/adc_uni/adc_cmds \
    autodoc/source/exes/adc_uni/cmd_sincedata \
    autodoc/source/exes/adc_uni/adc_cmd_parse \
    autodoc/source/exes/adc_uni/main \
    autodoc/source/exes/adc_uni/adc_msg \
    autodoc/source/exes/adc_uni/cmd_run \
    autodoc/source/parser/tokens/tkp \
    autodoc/source/parser/tokens/tokdeal \
    autodoc/source/parser/tokens/stmstate \
    autodoc/source/parser/tokens/tkpcontx \
    autodoc/source/parser/kernel/x_parse \
    autodoc/source/parser/kernel/parsefct \
    autodoc/source/parser/kernel/x_docu \
    autodoc/source/display/kernel/displfct \
    autodoc/source/display/idl/hi_main \
    autodoc/source/display/idl/hfi_property \
    autodoc/source/display/idl/hfi_xrefpage \
    autodoc/source/display/idl/hfi_struct \
    autodoc/source/display/idl/hfi_interface \
    autodoc/source/display/idl/hfi_method \
    autodoc/source/display/idl/hfi_typetext \
    autodoc/source/display/idl/hfi_module \
    autodoc/source/display/idl/hfi_enum \
    autodoc/source/display/idl/hfi_navibar \
    autodoc/source/display/idl/hi_display \
    autodoc/source/display/idl/hfi_service \
    autodoc/source/display/idl/hfi_globalindex \
    autodoc/source/display/idl/hfi_singleton \
    autodoc/source/display/idl/hi_factory \
    autodoc/source/display/idl/hi_ary \
    autodoc/source/display/idl/hi_env \
    autodoc/source/display/idl/hfi_siservice \
    autodoc/source/display/idl/hfi_typedef \
    autodoc/source/display/idl/hfi_constgroup \
    autodoc/source/display/idl/hi_linkhelper \
    autodoc/source/display/idl/hfi_tag \
    autodoc/source/display/idl/hfi_hierarchy \
    autodoc/source/display/idl/hfi_doc \
    autodoc/source/display/html/cfrstd \
    autodoc/source/display/toolkit/hf_linachain \
    autodoc/source/display/toolkit/outputstack \
    autodoc/source/display/toolkit/hf_navi_main \
    autodoc/source/display/toolkit/out_position \
    autodoc/source/display/toolkit/hf_funcdecl \
    autodoc/source/display/toolkit/out_tree \
    autodoc/source/display/toolkit/hf_docentry \
    autodoc/source/display/toolkit/out_node \
    autodoc/source/display/toolkit/hf_navi_sub \
    autodoc/source/display/toolkit/hf_title \
    autodoc/source/display/toolkit/htmlfile \
    autodoc/source/tools/tkpchars \
    autodoc/source/tools/filecoll \
    autodoc/source/parser_i/tokens/x_parse2 \
    autodoc/source/parser_i/tokens/stmstar2 \
    autodoc/source/parser_i/tokens/tkpstam2 \
    autodoc/source/parser_i/tokens/stmstfi2 \
    autodoc/source/parser_i/tokens/stmstat2 \
    autodoc/source/parser_i/tokens/tkp2 \
    autodoc/source/parser_i/tokens/tkpcont2 \
    autodoc/source/parser_i/idl/pestate \
    autodoc/source/parser_i/idl/pe_const \
    autodoc/source/parser_i/idl/pe_selem \
    autodoc/source/parser_i/idl/tk_const \
    autodoc/source/parser_i/idl/semnode \
    autodoc/source/parser_i/idl/pe_func2 \
    autodoc/source/parser_i/idl/tk_keyw \
    autodoc/source/parser_i/idl/pe_singl \
    autodoc/source/parser_i/idl/tk_ident \
    autodoc/source/parser_i/idl/pe_file2 \
    autodoc/source/parser_i/idl/pe_tydf2 \
    autodoc/source/parser_i/idl/cx_sub \
    autodoc/source/parser_i/idl/parsenv2 \
    autodoc/source/parser_i/idl/pe_enum2 \
    autodoc/source/parser_i/idl/pe_type2 \
    autodoc/source/parser_i/idl/pe_struc \
    autodoc/source/parser_i/idl/pe_attri \
    autodoc/source/parser_i/idl/pe_iface \
    autodoc/source/parser_i/idl/distrib \
    autodoc/source/parser_i/idl/pe_evalu \
    autodoc/source/parser_i/idl/pe_excp \
    autodoc/source/parser_i/idl/tkp_uidl \
    autodoc/source/parser_i/idl/pe_property \
    autodoc/source/parser_i/idl/tk_punct \
    autodoc/source/parser_i/idl/pe_servi \
    autodoc/source/parser_i/idl/cx_idlco \
    autodoc/source/parser_i/idl/unoidl \
    autodoc/source/parser_i/idl/pe_vari2 \
    autodoc/source/parser_i/idoc/tk_xml \
    autodoc/source/parser_i/idoc/cx_docu2 \
    autodoc/source/parser_i/idoc/tk_atag2 \
    autodoc/source/parser_i/idoc/cx_dsapi \
    autodoc/source/parser_i/idoc/tk_docw2 \
    autodoc/source/parser_i/idoc/docu_pe2 \
    autodoc/source/parser_i/idoc/tk_html \
    autodoc/source/ary_i/kernel/ci_atag2 \
    autodoc/source/ary_i/kernel/ci_text2 \
    autodoc/source/ary_i/kernel/d_token \
    autodoc/source/ary/info/all_tags \
    autodoc/source/ary/info/all_dts \
    autodoc/source/ary/info/ci_attag \
    autodoc/source/ary/info/ci_text \
    autodoc/source/ary/kernel/qualiname \
    autodoc/source/ary/kernel/reposy \
    autodoc/source/ary/kernel/namesort \
    autodoc/source/ary/idl/i_typedef \
    autodoc/source/ary/idl/i_enum \
    autodoc/source/ary/idl/i_constant \
    autodoc/source/ary/idl/is_type \
    autodoc/source/ary/idl/i_reposypart \
    autodoc/source/ary/idl/i_exception \
    autodoc/source/ary/idl/it_sequence \
    autodoc/source/ary/idl/i_siservice \
    autodoc/source/ary/idl/i_ce2s \
    autodoc/source/ary/idl/it_builtin \
    autodoc/source/ary/idl/ia_type \
    autodoc/source/ary/idl/i_property \
    autodoc/source/ary/idl/it_tplparam \
    autodoc/source/ary/idl/i_comrela \
    autodoc/source/ary/idl/it_ce \
    autodoc/source/ary/idl/i_structelem \
    autodoc/source/ary/idl/i_function \
    autodoc/source/ary/idl/is_ce \
    autodoc/source/ary/idl/i_constgroup \
    autodoc/source/ary/idl/i2s_calculator \
    autodoc/source/ary/idl/it_explicit \
    autodoc/source/ary/idl/i_module \
    autodoc/source/ary/idl/i_enumvalue \
    autodoc/source/ary/idl/i_sisingleton \
    autodoc/source/ary/idl/i_traits \
    autodoc/source/ary/idl/i_attribute \
    autodoc/source/ary/idl/i_singleton \
    autodoc/source/ary/idl/i_interface \
    autodoc/source/ary/idl/i_service \
    autodoc/source/ary/idl/i_struct \
    autodoc/source/ary/idl/i_ce \
    autodoc/source/ary/idl/it_xnameroom \
    autodoc/source/ary/idl/i_param \
    autodoc/source/ary/idl/i_namelookup \
    autodoc/source/ary/idl/ia_ce \
    autodoc/source/ary/doc/d_oldidldocu \
    autodoc/source/ary/doc/d_docu \
    autodoc/source/ary/doc/d_node \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
