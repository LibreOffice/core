# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,frm))

$(eval $(call gb_Library_set_include,frm,\
    $$(INCLUDE) \
    -I$(SRCDIR)/forms/source/inc \
    -I$(SRCDIR)/forms/source/solar/inc \
))

$(eval $(call gb_Library_use_api,frm,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_Library_use_libraries,frm,\
    comphelper \
    cppu \
    cppuhelper \
    dbtools \
    editeng \
    i18nisolang1 \
    sal \
    salhelper \
    sfx \
    svl \
    svt \
    svxcore \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_Library_use_externals,frm,\
    icuin \
    icuuc \
    libxml2 \
))
else
$(eval $(call gb_Library_use_externals,frm,\
    icui18n \
    icuuc \
    libxml2 \
))
endif

$(eval $(call gb_Library_set_componentfile,frm,forms/util/frm))

$(eval $(call gb_Library_add_exception_objects,frm,\
    forms/source/component/Button \
    forms/source/component/cachedrowset \
    forms/source/component/CheckBox \
    forms/source/component/clickableimage \
    forms/source/component/cloneable \
    forms/source/component/Columns \
    forms/source/component/ComboBox \
    forms/source/component/Currency \
    forms/source/component/DatabaseForm \
    forms/source/component/Date \
    forms/source/component/EditBase \
    forms/source/component/Edit \
    forms/source/component/entrylisthelper \
    forms/source/component/errorbroadcaster \
    forms/source/component/EventThread \
    forms/source/component/File \
    forms/source/component/Filter \
    forms/source/component/findpos \
    forms/source/component/FixedText \
    forms/source/component/FormattedField \
    forms/source/component/FormattedFieldWrapper \
    forms/source/component/FormComponent \
    forms/source/component/formcontrolfont \
    forms/source/component/FormsCollection \
    forms/source/component/Grid \
    forms/source/component/GroupBox \
    forms/source/component/GroupManager \
    forms/source/component/Hidden \
    forms/source/component/ImageButton \
    forms/source/component/ImageControl \
    forms/source/component/imgprod \
    forms/source/component/ListBox \
    forms/source/component/navigationbar \
    forms/source/component/Numeric \
    forms/source/component/Pattern \
    forms/source/component/propertybaghelper \
    forms/source/component/RadioButton \
    forms/source/component/refvaluecomponent \
    forms/source/component/scrollbar \
    forms/source/component/spinbutton \
    forms/source/component/Time \
    forms/source/helper/commanddescriptionprovider \
    forms/source/helper/commandimageprovider \
    forms/source/helper/controlfeatureinterception \
    forms/source/helper/formnavigation \
    forms/source/helper/resettable \
    forms/source/helper/urltransformer \
    forms/source/helper/windowstateguard \
    forms/source/misc/componenttools \
    forms/source/misc/frm_module \
    forms/source/misc/frm_strings \
    forms/source/misc/ids \
    forms/source/misc/InterfaceContainer \
    forms/source/misc/limitedformats \
    forms/source/misc/listenercontainers \
    forms/source/misc/property \
    forms/source/misc/services \
    forms/source/resource/frm_resource \
    forms/source/richtext/attributedispatcher \
    forms/source/richtext/clipboarddispatcher \
    forms/source/richtext/featuredispatcher \
    forms/source/richtext/parametrizedattributedispatcher \
    forms/source/richtext/richtextcontrol \
    forms/source/richtext/richtextengine \
    forms/source/richtext/richtextimplcontrol \
    forms/source/richtext/richtextmodel \
    forms/source/richtext/richtextunowrapper \
    forms/source/richtext/richtextvclcontrol \
    forms/source/richtext/richtextviewport \
    forms/source/richtext/rtattributehandler \
    forms/source/richtext/specialdispatchers \
    forms/source/runtime/formoperations \
    forms/source/solar/component/navbarcontrol \
    forms/source/solar/control/navtoolbar \
    forms/source/xforms/binding \
    forms/source/xforms/boolexpression \
    forms/source/xforms/computedexpression \
    forms/source/xforms/convert \
    forms/source/xforms/datatyperepository \
    forms/source/xforms/datatypes \
    forms/source/xforms/enumeration \
    forms/source/xforms/mip \
    forms/source/xforms/model \
    forms/source/xforms/model_ui \
    forms/source/xforms/pathexpression \
    forms/source/xforms/propertysetbase \
    forms/source/xforms/resourcehelper \
    forms/source/xforms/submission \
    forms/source/xforms/submission/replace \
    forms/source/xforms/submission/serialization_app_xml \
    forms/source/xforms/submission/serialization_urlencoded \
    forms/source/xforms/submission/submission_get \
    forms/source/xforms/submission/submission_post \
    forms/source/xforms/submission/submission_put \
    forms/source/xforms/unohelper \
    forms/source/xforms/xformsevent \
    forms/source/xforms/xforms_services \
    forms/source/xforms/xmlhelper \
    forms/source/xforms/xpathlib/extension \
    forms/source/xforms/xpathlib/xpathlib \
))

# vim: set noet sw=4 ts=4:
