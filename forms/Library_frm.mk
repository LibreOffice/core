# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,frm))

$(eval $(call gb_Library_set_include,frm,\
    $$(INCLUDE) \
    -I$(SRCDIR)/forms/inc \
    -I$(SRCDIR)/forms/source/inc \
    -I$(SRCDIR)/forms/source/solar/inc \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_set_precompiled_header,frm,forms/inc/pch/precompiled_frm))

$(eval $(call gb_Library_use_api,frm,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_Library_use_libraries,frm,\
    comphelper \
    cppu \
    cppuhelper \
    $(call gb_Helper_optional,DBCONNECTIVITY,dbtools) \
    editeng \
    i18nlangtag \
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
))

$(eval $(call gb_Library_use_externals,frm,\
	boost_headers \
    icui18n \
    icuuc \
	icu_headers \
    libxml2 \
))

$(eval $(call gb_Library_set_componentfile,frm,forms/util/frm,services))

$(eval $(call gb_Library_add_exception_objects,frm,\
    forms/source/component/Button \
    forms/source/component/cachedrowset \
    forms/source/component/CheckBox \
    forms/source/component/clickableimage \
    forms/source/component/cloneable \
    forms/source/component/errorbroadcaster \
    forms/source/component/Columns \
    forms/source/component/ComboBox \
    forms/source/component/Currency \
    forms/source/component/Date \
    forms/source/component/DatabaseForm \
    forms/source/component/EditBase \
    forms/source/component/Edit \
    forms/source/component/entrylisthelper \
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
    forms/source/helper/commandimageprovider \
    forms/source/helper/controlfeatureinterception \
    forms/source/helper/formnavigation \
    forms/source/helper/resettable \
    forms/source/helper/urltransformer \
    forms/source/helper/windowstateguard \
    forms/source/misc/componenttools \
    forms/source/misc/InterfaceContainer \
    forms/source/misc/limitedformats \
    forms/source/misc/property \
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
