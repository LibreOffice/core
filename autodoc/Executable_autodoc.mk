#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Executable_Executable,autodoc))

$(eval $(call gb_Executable_set_include,autodoc,\
	$$(INCLUDE) \
	-I$(SRCDIR)/autodoc/inc/ \
	-I$(SRCDIR)/autodoc/source/inc/ \
	-I$(SRCDIR)/autodoc/source/ary/inc/ \
	-I$(SRCDIR)/autodoc/source/display/inc/ \
	-I$(SRCDIR)/autodoc/source/parser/inc/ \
	-I$(SRCDIR)/autodoc/source/parser_i/inc/ \
))

#$(eval $(call gb_Executable_add_defs,autodoc,\
#	-D_TOOLS_STRINGLIST \
#))

$(eval $(call gb_Executable_add_linked_libs,autodoc,\
	stl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Executable_add_linked_static_libs,autodoc,\
	cosv \
	udm \
))

# See <http://porting.openoffice.org/servlets/ReadMsg?list=mac&msgNo=6911>
ifeq ($(OS),MACOSX)
$(eval $(call gb_Executable_add_ldflags,autodoc,\
	-Wl,-all_load \
))
endif

$(eval $(call gb_Executable_add_exception_objects,autodoc,\
	autodoc/source/ary/cpp/c_builtintype \
	autodoc/source/ary/cpp/c_class \
	autodoc/source/ary/cpp/c_de \
	autodoc/source/ary/cpp/c_define \
	autodoc/source/ary/cpp/c_enum \
	autodoc/source/ary/cpp/c_enuval \
	autodoc/source/ary/cpp/c_funct \
	autodoc/source/ary/cpp/c_macro \
	autodoc/source/ary/cpp/c_namesp \
	autodoc/source/ary/cpp/c_osigna \
	autodoc/source/ary/cpp/c_reposypart \
	autodoc/source/ary/cpp/c_slots \
	autodoc/source/ary/cpp/c_traits \
	autodoc/source/ary/cpp/c_tydef \
	autodoc/source/ary/cpp/c_vari \
	autodoc/source/ary/cpp/ca_ce \
	autodoc/source/ary/cpp/ca_def \
	autodoc/source/ary/cpp/ca_type \
	autodoc/source/ary/cpp/cs_ce \
	autodoc/source/ary/cpp/cs_def \
	autodoc/source/ary/cpp/cs_type \
	autodoc/source/ary/cpp/namechain \
	autodoc/source/ary/cpp/tplparam \
	autodoc/source/ary/cpp/usedtype \
	autodoc/source/ary/doc/d_boolean \
	autodoc/source/ary/doc/d_docu \
	autodoc/source/ary/doc/d_node \
	autodoc/source/ary/doc/d_oldcppdocu \
	autodoc/source/ary/doc/d_oldidldocu \
	autodoc/source/ary/idl/i_attribute \
	autodoc/source/ary/idl/i_ce \
	autodoc/source/ary/idl/i_ce2s \
	autodoc/source/ary/idl/i_comrela \
	autodoc/source/ary/idl/i_constant \
	autodoc/source/ary/idl/i_constgroup \
	autodoc/source/ary/idl/i_enum \
	autodoc/source/ary/idl/i_enumvalue \
	autodoc/source/ary/idl/i_exception \
	autodoc/source/ary/idl/i_function \
	autodoc/source/ary/idl/i_interface \
	autodoc/source/ary/idl/i_module \
	autodoc/source/ary/idl/i_namelookup \
	autodoc/source/ary/idl/i_param \
	autodoc/source/ary/idl/i_property \
	autodoc/source/ary/idl/i_reposypart \
	autodoc/source/ary/idl/i_service \
	autodoc/source/ary/idl/i_singleton \
	autodoc/source/ary/idl/i_siservice \
	autodoc/source/ary/idl/i_sisingleton \
	autodoc/source/ary/idl/i_struct \
	autodoc/source/ary/idl/i_structelem \
	autodoc/source/ary/idl/i_traits \
	autodoc/source/ary/idl/i_typedef \
	autodoc/source/ary/idl/i2s_calculator \
	autodoc/source/ary/idl/ia_ce \
	autodoc/source/ary/idl/ia_type \
	autodoc/source/ary/idl/is_ce \
	autodoc/source/ary/idl/is_type \
	autodoc/source/ary/idl/it_builtin \
	autodoc/source/ary/idl/it_ce \
	autodoc/source/ary/idl/it_explicit \
	autodoc/source/ary/idl/it_sequence \
	autodoc/source/ary/idl/it_tplparam \
	autodoc/source/ary/idl/it_xnameroom \
	autodoc/source/ary/info/all_dts \
	autodoc/source/ary/info/all_tags \
	autodoc/source/ary/info/ci_attag \
	autodoc/source/ary/info/ci_text \
	autodoc/source/ary/kernel/ary_disp \
	autodoc/source/ary/kernel/cessentl \
	autodoc/source/ary/kernel/namesort \
	autodoc/source/ary/kernel/qualiname \
	autodoc/source/ary/kernel/reposy \
	autodoc/source/ary/kernel/slots \
	autodoc/source/ary/loc/loc_dir \
	autodoc/source/ary/loc/loc_file \
	autodoc/source/ary/loc/loc_filebase \
	autodoc/source/ary/loc/loc_root \
	autodoc/source/ary/loc/loc_traits \
	autodoc/source/ary/loc/loca_le \
	autodoc/source/ary/loc/locs_le \
	autodoc/source/ary_i/kernel/ci_atag2 \
	autodoc/source/ary_i/kernel/ci_text2 \
	autodoc/source/ary_i/kernel/d_token \
	autodoc/source/display/html/aryattrs \
	autodoc/source/display/html/cfrstd \
	autodoc/source/display/html/chd_udk2 \
	autodoc/source/display/html/cre_link \
	autodoc/source/display/html/dsply_cl \
	autodoc/source/display/html/dsply_da \
	autodoc/source/display/html/dsply_op \
	autodoc/source/display/html/easywri \
	autodoc/source/display/html/hd_chlst \
	autodoc/source/display/html/hd_docu \
	autodoc/source/display/html/hdimpl \
	autodoc/source/display/html/html_kit \
	autodoc/source/display/html/nav_main \
	autodoc/source/display/html/navibar \
	autodoc/source/display/html/outfile \
	autodoc/source/display/html/opageenv \
	autodoc/source/display/html/pagemake \
	autodoc/source/display/html/pm_aldef \
	autodoc/source/display/html/pm_base \
	autodoc/source/display/html/pm_class \
	autodoc/source/display/html/pm_help \
	autodoc/source/display/html/pm_index \
	autodoc/source/display/html/pm_namsp \
	autodoc/source/display/html/pm_start \
	autodoc/source/display/html/protarea \
	autodoc/source/display/kernel/displfct \
	autodoc/source/display/idl/hfi_constgroup \
	autodoc/source/display/idl/hfi_doc \
	autodoc/source/display/idl/hfi_enum \
	autodoc/source/display/idl/hfi_globalindex \
	autodoc/source/display/idl/hfi_hierarchy \
	autodoc/source/display/idl/hfi_interface \
	autodoc/source/display/idl/hfi_method \
	autodoc/source/display/idl/hfi_module \
	autodoc/source/display/idl/hfi_navibar \
	autodoc/source/display/idl/hfi_property \
	autodoc/source/display/idl/hfi_service \
	autodoc/source/display/idl/hfi_singleton \
	autodoc/source/display/idl/hfi_siservice \
	autodoc/source/display/idl/hfi_struct \
	autodoc/source/display/idl/hfi_tag \
	autodoc/source/display/idl/hfi_typedef \
	autodoc/source/display/idl/hfi_typetext \
	autodoc/source/display/idl/hfi_xrefpage \
	autodoc/source/display/idl/hi_ary \
	autodoc/source/display/idl/hi_display \
	autodoc/source/display/idl/hi_env \
	autodoc/source/display/idl/hi_factory \
	autodoc/source/display/idl/hi_linkhelper \
	autodoc/source/display/idl/hi_main \
	autodoc/source/display/toolkit/hf_docentry \
	autodoc/source/display/toolkit/hf_funcdecl \
	autodoc/source/display/toolkit/hf_linachain \
	autodoc/source/display/toolkit/hf_navi_main \
	autodoc/source/display/toolkit/hf_navi_sub \
	autodoc/source/display/toolkit/hf_title \
	autodoc/source/display/toolkit/htmlfile \
	autodoc/source/display/toolkit/out_node \
	autodoc/source/display/toolkit/out_position \
	autodoc/source/display/toolkit/out_tree \
	autodoc/source/display/toolkit/outputstack \
	autodoc/source/parser/adoc/adoc_tok \
	autodoc/source/parser/adoc/a_rdocu \
	autodoc/source/parser/adoc/cx_a_std \
	autodoc/source/parser/adoc/cx_a_sub \
	autodoc/source/parser/adoc/docu_pe \
	autodoc/source/parser/adoc/prs_adoc \
	autodoc/source/parser/adoc/tk_attag \
	autodoc/source/parser/adoc/tk_docw \
	autodoc/source/parser/cpp/all_toks \
	autodoc/source/parser/cpp/c_dealer \
	autodoc/source/parser/cpp/c_rcode \
	autodoc/source/parser/cpp/cpp_pe \
	autodoc/source/parser/cpp/cx_base \
	autodoc/source/parser/cpp/cx_c_pp \
	autodoc/source/parser/cpp/cx_c_std \
	autodoc/source/parser/cpp/cx_c_sub \
	autodoc/source/parser/cpp/cxt2ary \
	autodoc/source/parser/cpp/defdescr \
	autodoc/source/parser/cpp/icprivow \
	autodoc/source/parser/cpp/pe_base \
	autodoc/source/parser/cpp/pe_class \
	autodoc/source/parser/cpp/pe_defs \
	autodoc/source/parser/cpp/pe_expr \
	autodoc/source/parser/cpp/pe_enum \
	autodoc/source/parser/cpp/pe_enval \
	autodoc/source/parser/cpp/pe_file \
	autodoc/source/parser/cpp/pe_funct \
	autodoc/source/parser/cpp/pe_ignor \
	autodoc/source/parser/cpp/pe_namsp \
	autodoc/source/parser/cpp/pe_param \
	autodoc/source/parser/cpp/pe_tpltp \
	autodoc/source/parser/cpp/pe_type \
	autodoc/source/parser/cpp/pe_tydef \
	autodoc/source/parser/cpp/pe_vafu \
	autodoc/source/parser/cpp/pe_vari \
	autodoc/source/parser/cpp/preproc \
	autodoc/source/parser/cpp/prs_cpp \
	autodoc/source/parser/cpp/tkp_cpp \
	autodoc/source/parser/kernel/parsefct \
	autodoc/source/parser/kernel/x_docu \
	autodoc/source/parser/kernel/x_parse \
	autodoc/source/parser/semantic/parseenv \
	autodoc/source/parser/tokens/stmstarr \
	autodoc/source/parser/tokens/stmstate \
	autodoc/source/parser/tokens/stmstfin \
	autodoc/source/parser/tokens/tkpstama \
	autodoc/source/parser/tokens/tkp \
	autodoc/source/parser/tokens/tkpcontx \
	autodoc/source/parser/tokens/tokdeal \
	autodoc/source/parser_i/idl/cx_idlco \
	autodoc/source/parser_i/idl/cx_sub \
	autodoc/source/parser_i/idl/distrib \
	autodoc/source/parser_i/idl/parsenv2 \
	autodoc/source/parser_i/idl/pe_attri \
	autodoc/source/parser_i/idl/pe_const \
	autodoc/source/parser_i/idl/pe_enum2 \
	autodoc/source/parser_i/idl/pe_evalu \
	autodoc/source/parser_i/idl/pe_excp \
	autodoc/source/parser_i/idl/pe_file2 \
	autodoc/source/parser_i/idl/pe_func2 \
	autodoc/source/parser_i/idl/pe_iface \
	autodoc/source/parser_i/idl/pe_property \
	autodoc/source/parser_i/idl/pe_selem \
	autodoc/source/parser_i/idl/pe_servi \
	autodoc/source/parser_i/idl/pe_singl \
	autodoc/source/parser_i/idl/pe_struc \
	autodoc/source/parser_i/idl/pe_tydf2 \
	autodoc/source/parser_i/idl/pe_type2 \
	autodoc/source/parser_i/idl/pe_vari2 \
	autodoc/source/parser_i/idl/pestate \
	autodoc/source/parser_i/idl/semnode \
	autodoc/source/parser_i/idl/tk_const \
	autodoc/source/parser_i/idl/tk_ident \
	autodoc/source/parser_i/idl/tk_keyw \
	autodoc/source/parser_i/idl/tk_punct \
	autodoc/source/parser_i/idl/tkp_uidl \
	autodoc/source/parser_i/idl/unoidl \
	autodoc/source/parser_i/idoc/cx_docu2 \
	autodoc/source/parser_i/idoc/cx_dsapi \
	autodoc/source/parser_i/idoc/docu_pe2 \
	autodoc/source/parser_i/idoc/tk_atag2 \
	autodoc/source/parser_i/idoc/tk_docw2 \
	autodoc/source/parser_i/idoc/tk_html \
	autodoc/source/parser_i/idoc/tk_xml \
	autodoc/source/parser_i/tokens/stmstar2 \
	autodoc/source/parser_i/tokens/stmstat2 \
	autodoc/source/parser_i/tokens/stmstfi2 \
	autodoc/source/parser_i/tokens/tkpstam2 \
	autodoc/source/parser_i/tokens/tkp2 \
	autodoc/source/parser_i/tokens/tkpcont2 \
	autodoc/source/parser_i/tokens/x_parse2 \
	autodoc/source/tools/filecoll \
	autodoc/source/tools/tkpchars \
	autodoc/source/exes/adc_uni/adc_cl \
	autodoc/source/exes/adc_uni/adc_cmd_parse \
	autodoc/source/exes/adc_uni/adc_cmds \
	autodoc/source/exes/adc_uni/adc_msg \
	autodoc/source/exes/adc_uni/cmd_run \
	autodoc/source/exes/adc_uni/cmd_sincedata \
	autodoc/source/exes/adc_uni/main \
))

# vim: set noet sw=4 ts=4:
