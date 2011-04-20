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

$(eval $(call gb_Package_add_file,sd_xml,AccessibleDrawDocumentView.xml,xml/AccessibleDrawDocumentView.xml))
$(eval $(call gb_Package_add_file,sd_xml,AccessibleOutlineView.xml,xml/AccessibleOutlineView.xml))
$(eval $(call gb_Package_add_file,sd_xml,AccessiblePageShape.xml,xml/AccessiblePageShape.xml))
$(eval $(call gb_Package_add_file,sd_xml,AccessiblePresentationGraphicShape.xml,xml/AccessiblePresentationGraphicShape.xml))
$(eval $(call gb_Package_add_file,sd_xml,AccessiblePresentationOLEShape.xml,xml/AccessiblePresentationOLEShape.xml))
$(eval $(call gb_Package_add_file,sd_xml,AccessiblePresentationShape.xml,xml/AccessiblePresentationShape.xml))
$(eval $(call gb_Package_add_file,sd_xml,AccessibleSlideViewObject.xml,xml/AccessibleSlideViewObject.xml))
$(eval $(call gb_Package_add_file,sd_xml,AccessibleSlideView.xml,xml/AccessibleSlideView.xml))
$(eval $(call gb_Package_add_file,sd_xml,DrawingView.xml,xml/DrawingView.xml))
$(eval $(call gb_Package_add_file,sd_xml,effects.xml,xml/effects.xml))
$(eval $(call gb_Package_add_file,sd_xml,HandoutView.xml,xml/HandoutView.xml))
$(eval $(call gb_Package_add_file,sd_xml,NotesView.xml,xml/NotesView.xml))
$(eval $(call gb_Package_add_file,sd_xml,OutlineView.xml,xml/OutlineView.xml))
$(eval $(call gb_Package_add_file,sd_xml,PresentationView.xml,xml/PresentationView.xml))
$(eval $(call gb_Package_add_file,sd_xml,SdDocLinkTargets.xml,xml/SdDocLinkTargets.xml))
$(eval $(call gb_Package_add_file,sd_xml,SdDrawPagesAccess.xml,xml/SdDrawPagesAccess.xml))
$(eval $(call gb_Package_add_file,sd_xml,SdDrawPage.xml,xml/SdDrawPage.xml))
$(eval $(call gb_Package_add_file,sd_xml,SdGenericDrawPage.xml,xml/SdGenericDrawPage.xml))
$(eval $(call gb_Package_add_file,sd_xml,SdLayerManager.xml,xml/SdLayerManager.xml))
$(eval $(call gb_Package_add_file,sd_xml,SdLayer.xml,xml/SdLayer.xml))
$(eval $(call gb_Package_add_file,sd_xml,SdMasterPagesAccess.xml,xml/SdMasterPagesAccess.xml))
$(eval $(call gb_Package_add_file,sd_xml,SdMasterPage.xml,xml/SdMasterPage.xml))
$(eval $(call gb_Package_add_file,sd_xml,SdPageLinkTargets.xml,xml/SdPageLinkTargets.xml))
$(eval $(call gb_Package_add_file,sd_xml,SdXCustomPresentationAccess.xml,xml/SdXCustomPresentationAccess.xml))
$(eval $(call gb_Package_add_file,sd_xml,SdXCustomPresentation.xml,xml/SdXCustomPresentation.xml))
$(eval $(call gb_Package_add_file,sd_xml,SdXImpressDocument.xml,xml/SdXImpressDocument.xml))
$(eval $(call gb_Package_add_file,sd_xml,SdXPresentation.xml,xml/SdXPresentation.xml))
$(eval $(call gb_Package_add_file,sd_xml,SdXShape.xml,xml/SdXShape.xml))
$(eval $(call gb_Package_add_file,sd_xml,SlidesView.xml,xml/SlidesView.xml))
$(eval $(call gb_Package_add_file,sd_xml,transitions-ogl.xml,xml/transitions-ogl.xml))
$(eval $(call gb_Package_add_file,sd_xml,transitions.xml,xml/transitions.xml))

# vim: set noet ts=4 sw=4:
