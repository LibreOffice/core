# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Package_Package,sd_xml,$(SRCDIR)/sd/xml))

$(eval $(call gb_Package_add_file,sd_xml,xml/AccessibleDrawDocumentView.xml,AccessibleDrawDocumentView.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/AccessibleOutlineView.xml,AccessibleOutlineView.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/AccessiblePageShape.xml,AccessiblePageShape.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/AccessiblePresentationGraphicShape.xml,AccessiblePresentationGraphicShape.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/AccessiblePresentationOLEShape.xml,AccessiblePresentationOLEShape.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/AccessiblePresentationShape.xml,AccessiblePresentationShape.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/AccessibleSlideViewObject.xml,AccessibleSlideViewObject.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/AccessibleSlideView.xml,AccessibleSlideView.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/DrawingView.xml,DrawingView.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/effects.xml,effects.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/HandoutView.xml,HandoutView.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/NotesView.xml,NotesView.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/OutlineView.xml,OutlineView.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/PresentationView.xml,PresentationView.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SdDocLinkTargets.xml,SdDocLinkTargets.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SdDrawPagesAccess.xml,SdDrawPagesAccess.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SdDrawPage.xml,SdDrawPage.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SdGenericDrawPage.xml,SdGenericDrawPage.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SdLayerManager.xml,SdLayerManager.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SdLayer.xml,SdLayer.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SdMasterPagesAccess.xml,SdMasterPagesAccess.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SdMasterPage.xml,SdMasterPage.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SdPageLinkTargets.xml,SdPageLinkTargets.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SdXCustomPresentationAccess.xml,SdXCustomPresentationAccess.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SdXCustomPresentation.xml,SdXCustomPresentation.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SdXImpressDocument.xml,SdXImpressDocument.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SdXPresentation.xml,SdXPresentation.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SdXShape.xml,SdXShape.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/SlidesView.xml,SlidesView.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/transitions-ogl.xml,transitions-ogl.xml))
$(eval $(call gb_Package_add_file,sd_xml,xml/transitions.xml,transitions.xml))

# vim: set noet ts=4 sw=4:
