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



$(eval $(call gb_AllLangResTarget_AllLangResTarget,svx))

$(eval $(call gb_AllLangResTarget_set_reslocation,svx,svx))

$(eval $(call gb_AllLangResTarget_add_srs,svx,\
	svx/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,svx/res))

$(eval $(call gb_SrsTarget_set_include,svx/res,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc \
	-I$(WORKDIR)/inc \
	-I$(WORKDIR)/inc/svx \
	-I$(SRCDIR)/svx/source/inc \
	-I$(SRCDIR)/svx/source/dialog \
	-I$(SRCDIR)/svx/inc/ \
))

$(eval $(call gb_SrsTarget_add_files,svx/res,\
    svx/source/accessibility/accessibility.src \
    svx/source/dialog/bmpmask.src \
    svx/source/dialog/contdlg.src \
    svx/source/dialog/ctredlin.src \
    svx/source/dialog/dlgctrl.src \
    svx/source/dialog/docrecovery.src \
    svx/source/dialog/fontwork.src \
    svx/source/dialog/frmsel.src \
    svx/source/dialog/hdft.src \
    svx/source/dialog/hyprlink.src \
    svx/source/dialog/imapdlg.src \
    svx/source/dialog/langbox.src \
    svx/source/dialog/language.src \
    svx/source/dialog/optgrid.src \
    svx/source/dialog/passwd.src \
    svx/source/dialog/prtqry.src \
    svx/source/dialog/rubydialog.src \
    svx/source/dialog/ruler.src \
    svx/source/dialog/sdstring.src \
    svx/source/dialog/srchdlg.src \
    svx/source/dialog/svxbmpnumvalueset.src \
    svx/source/dialog/swframeposstrings.src \
    svx/source/dialog/txenctab.src \
    svx/source/dialog/ucsubset.src \
    svx/source/engine3d/float3d.src \
    svx/source/engine3d/string3d.src \
    svx/source/fmcomp/gridctrl.src \
    svx/source/form/datanavi.src \
    svx/source/form/filtnav.src \
    svx/source/form/fmexpl.src \
    svx/source/form/fmstring.src \
    svx/source/form/formshell.src \
    svx/source/items/svxerr.src \
    svx/source/items/svxitems.src \
    svx/source/mnuctrls/mnuctrls.src \
    svx/source/sidebar/EmptyPanel.src \
    svx/source/sidebar/area/AreaPropertyPanel.src \
    svx/source/sidebar/graphic/GraphicPropertyPanel.src \
    svx/source/sidebar/line/LinePropertyPanel.src \
    svx/source/sidebar/possize/PosSizePropertyPanel.src \
    svx/source/sidebar/text/TextPropertyPanel.src \
    svx/source/sidebar/paragraph/ParaPropertyPanel.src \
    svx/source/stbctrls/stbctrls.src \
    svx/source/svdraw/svdstr.src \
    svx/source/table/table.src \
    svx/source/tbxctrls/colrctrl.src \
    svx/source/tbxctrls/extrusioncontrols.src \
    svx/source/tbxctrls/fontworkgallery.src \
    svx/source/tbxctrls/grafctrl.src \
    svx/source/tbxctrls/lboxctrl.src \
    svx/source/tbxctrls/tbcontrl.src \
    svx/source/tbxctrls/tbunosearchcontrollers.src \
    svx/source/toolbars/extrusionbar.src \
    svx/source/toolbars/fontworkbar.src \
    svx/source/unodraw/unodraw.src \
))

$(call gb_SrsPartTarget_get_target,svx/source/fmcomp/gridctrl.src) : $(WORKDIR)/inc/svx/globlmn.hrc
$(call gb_SrsPartTarget_get_target,svx/source/form/fmexpl.src) : $(WORKDIR)/inc/svx/globlmn.hrc
$(call gb_SrsPartTarget_get_target,svx/source/form/datanavi.src) : $(WORKDIR)/inc/svx/globlmn.hrc
$(call gb_SrsPartTarget_get_target,svx/source/form/formshell.src) : $(WORKDIR)/inc/svx/globlmn.hrc
$(call gb_SrsTarget_get_clean_target,svx/res) : $(WORKDIR)/inc/svx/globlmn.hrc_clean

$(OUTDIR)/inc/svx/globlmn.hrc : $(WORKDIR)/inc/svx/globlmn.hrc
	$(call gb_Deliver_deliver,$<,$@)

# hack !!!
# just a temporary - globlmn.hrc about to be removed!
ifeq ($(strip $(WITH_LANG)),)
$(WORKDIR)/inc/svx/globlmn.hrc : $(SRCDIR)/svx/inc/globlmn_tmpl.hrc
	echo copying $@
	-mkdir -p $(WORKDIR)/inc/svx
	cp $(SRCDIR)/svx/inc/globlmn_tmpl.hrc $(WORKDIR)/inc/svx/globlmn.hrc
	rm -f $(WORKDIR)/inc/svx/lastrun.mk 
else
-include $(WORKDIR)/inc/svx/lastrun.mk
ifneq ($(gb_lastrun_globlmn),MERGED)
.PHONY : $(WORKDIR)/inc/svx/globlmn.hrc
endif
$(WORKDIR)/inc/svx/globlmn.hrc : $(SRCDIR)/svx/inc/globlmn_tmpl.hrc $(gb_SrsPartMergeTarget_SDFLOCATION)/svx/inc/localize.sdf
	echo merging $@
	-mkdir -p $(WORKDIR)/inc/svx
	rm -f $(WORKDIR)/inc/svx/lastrun.mk
	echo gb_lastrun_globlmn:=MERGED > $(WORKDIR)/inc/svx/lastrun.mk
	$(call gb_Helper_abbreviate_dirs_native, \
		$(gb_SrsPartMergeTarget_TRANSEXCOMMAND) \
		-p svx \
		 -i $< -o $@ -m $(gb_SrsPartMergeTarget_SDFLOCATION)/svx/inc/localize.sdf -l all)
endif

.PHONY : $(WORKDIR)/inc/svx/globlmn.hrc_clean
$(WORKDIR)/inc/svx/globlmn.hrc_clean :
	rm -f $(WORKDIR)/inc/svx/lastrun.mk \
		$(WORKDIR)/inc/svx/globlmn.hrc


# vim: set noet sw=4 ts=4:
