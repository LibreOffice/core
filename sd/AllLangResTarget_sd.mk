###############################################################
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
###############################################################



$(eval $(call gb_AllLangResTarget_AllLangResTarget,sd))

$(eval $(call gb_AllLangResTarget_add_srs,sd,\
	sd/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sd/res))

$(eval $(call gb_SrsTarget_set_include,sd/res,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc \
	-I$(WORKDIR)/inc/sd \
	-I$(SRCDIR)/sd/inc \
	-I$(SRCDIR)/sd/source/ui/inc \
	-I$(SRCDIR)/sd/source/ui/slidesorter/inc \
))

$(eval $(call gb_SrsTarget_add_files,sd/res,\
	sd/source/core/glob.src \
	sd/source/filter/html/pubdlg.src \
	sd/source/ui/accessibility/accessibility.src \
	sd/source/ui/animations/CustomAnimation.src \
	sd/source/ui/animations/CustomAnimationCreateDialog.src \
	sd/source/ui/animations/CustomAnimationDialog.src \
	sd/source/ui/animations/CustomAnimationPane.src \
	sd/source/ui/animations/SlideTransitionPane.src \
	sd/source/ui/annotations/annotations.src \
	sd/source/ui/app/popup.src \
	sd/source/ui/app/res_bmp.src \
	sd/source/ui/app/strings.src \
	sd/source/ui/dlg/animobjs.src \
	sd/source/ui/dlg/brkdlg.src \
	sd/source/ui/dlg/copydlg.src \
	sd/source/ui/dlg/custsdlg.src \
	sd/source/ui/dlg/dlgass.src \
	sd/source/ui/dlg/dlgfield.src \
	sd/source/ui/dlg/dlgolbul.src \
	sd/source/ui/dlg/dlgpage.src \
	sd/source/ui/dlg/dlgsnap.src \
	sd/source/ui/dlg/dlg_char.src \
	sd/source/ui/dlg/headerfooterdlg.src \
	sd/source/ui/dlg/inspagob.src \
	sd/source/ui/dlg/ins_paste.src \
	sd/source/ui/dlg/LayerDialog.src \
	sd/source/ui/dlg/layeroptionsdlg.src \
	sd/source/ui/dlg/masterlayoutdlg.src \
	sd/source/ui/dlg/morphdlg.src \
	sd/source/ui/dlg/navigatr.src \
	sd/source/ui/dlg/PaneDockingWindow.src \
	sd/source/ui/dlg/paragr.src \
	sd/source/ui/dlg/present.src \
	sd/source/ui/dlg/prltempl.src \
	sd/source/ui/dlg/prntopts.src \
	sd/source/ui/dlg/sdpreslt.src \
	sd/source/ui/dlg/tabtempl.src \
	sd/source/ui/dlg/tpaction.src \
	sd/source/ui/dlg/tpoption.src \
	sd/source/ui/dlg/vectdlg.src \
	sd/source/ui/notes/NotesChildWindow.src \
	sd/source/ui/slideshow/slideshow.src \
	sd/source/ui/slidesorter/view/SlsResource.src \
	sd/source/ui/table/TableDesignPane.src \
	sd/source/ui/view/DocumentRenderer.src \
))
