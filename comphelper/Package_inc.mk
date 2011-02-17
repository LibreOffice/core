#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Package_Package,comphelper_inc,$(SRCDIR)/comphelper/inc))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/flagguard.hxx,comphelper/flagguard.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/stlunosequence.hxx,comphelper/stlunosequence.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/documentconstants.hxx,comphelper/documentconstants.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/evtmethodhelper.hxx,comphelper/evtmethodhelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/weakbag.hxx,comphelper/weakbag.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/basicio.hxx,comphelper/basicio.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/classids.hxx,comphelper/classids.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/MasterPropertySet.hxx,comphelper/MasterPropertySet.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/stillreadwriteinteraction.hxx,comphelper/stillreadwriteinteraction.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/propagg.hxx,comphelper/propagg.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/scopeguard.hxx,comphelper/scopeguard.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/asyncnotification.hxx,comphelper/asyncnotification.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/namedvaluecollection.hxx,comphelper/namedvaluecollection.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/composedprops.hxx,comphelper/composedprops.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/mediadescriptor.hxx,comphelper/mediadescriptor.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/synchronousdispatch.hxx,comphelper/synchronousdispatch.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/accessiblecomponenthelper.hxx,comphelper/accessiblecomponenthelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/otransactedfilestream.hxx,comphelper/otransactedfilestream.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/propmultiplex.hxx,comphelper/propmultiplex.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/servicehelper.hxx,comphelper/servicehelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/servicedecl.hxx,comphelper/servicedecl.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/mimeconfighelper.hxx,comphelper/mimeconfighelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/implbase_var.hxx,comphelper/implbase_var.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/TypeGeneration.hxx,comphelper/TypeGeneration.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/MasterPropertySetInfo.hxx,comphelper/MasterPropertySetInfo.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/accessiblewrapper.hxx,comphelper/accessiblewrapper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/sequence.hxx,comphelper/sequence.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/uieventslogger.hxx,comphelper/uieventslogger.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/docpasswordhelper.hxx,comphelper/docpasswordhelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/propertystatecontainer.hxx,comphelper/propertystatecontainer.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/propertysetinfo.hxx,comphelper/propertysetinfo.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/accimplaccess.hxx,comphelper/accimplaccess.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/docpasswordrequest.hxx,comphelper/docpasswordrequest.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/accessibleeventbuffer.hxx,comphelper/accessibleeventbuffer.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/regpathhelper.hxx,comphelper/regpathhelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/extract.hxx,comphelper/extract.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/interaction.hxx,comphelper/interaction.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/uno3.hxx,comphelper/uno3.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/unwrapargs.hxx,comphelper/unwrapargs.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/propertycontainer.hxx,comphelper/propertycontainer.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/guarding.hxx,comphelper/guarding.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/fileformat.h,comphelper/fileformat.h))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/ChainablePropertySetInfo.hxx,comphelper/ChainablePropertySetInfo.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/enumhelper.hxx,comphelper/enumhelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/property.hxx,comphelper/property.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/propertysethelper.hxx,comphelper/propertysethelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/accessiblekeybindinghelper.hxx,comphelper/accessiblekeybindinghelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/propertycontainerhelper.hxx,comphelper/propertycontainerhelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/make_shared_from_uno.hxx,comphelper/make_shared_from_uno.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/comphelperdllapi.h,comphelper/comphelperdllapi.h))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/listenernotification.hxx,comphelper/listenernotification.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/attributelist.hxx,comphelper/attributelist.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/streamsection.hxx,comphelper/streamsection.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/accessibletexthelper.hxx,comphelper/accessibletexthelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/accessibleselectionhelper.hxx,comphelper/accessibleselectionhelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/serviceinfohelper.hxx,comphelper/serviceinfohelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/locale.hxx,comphelper/locale.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/numberedcollection.hxx,comphelper/numberedcollection.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/ChainablePropertySet.hxx,comphelper/ChainablePropertySet.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/stl_types.hxx,comphelper/stl_types.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/eventattachermgr.hxx,comphelper/eventattachermgr.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/componentcontext.hxx,comphelper/componentcontext.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/legacysingletonfactory.hxx,comphelper/legacysingletonfactory.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/accessibleeventnotifier.hxx,comphelper/accessibleeventnotifier.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/ihwrapnofilter.hxx,comphelper/ihwrapnofilter.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/sequenceasvector.hxx,comphelper/sequenceasvector.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/officeresourcebundle.hxx,comphelper/officeresourcebundle.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/types.hxx,comphelper/types.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/seekableinput.hxx,comphelper/seekableinput.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/IdPropArrayHelper.hxx,comphelper/IdPropArrayHelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/accessiblecontexthelper.hxx,comphelper/accessiblecontexthelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/SelectionMultiplex.hxx,comphelper/SelectionMultiplex.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/storagehelper.hxx,comphelper/storagehelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/logging.hxx,comphelper/logging.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/sharedmutex.hxx,comphelper/sharedmutex.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/documentinfo.hxx,comphelper/documentinfo.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/weakeventlistener.hxx,comphelper/weakeventlistener.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/componentfactory.hxx,comphelper/componentfactory.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/containermultiplexer.hxx,comphelper/containermultiplexer.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/broadcasthelper.hxx,comphelper/broadcasthelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/numbers.hxx,comphelper/numbers.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/evtlistenerhlp.hxx,comphelper/evtlistenerhlp.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/optional.hxx,comphelper/optional.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/componentbase.hxx,comphelper/componentbase.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/InlineContainer.hxx,comphelper/InlineContainer.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/propertybag.hxx,comphelper/propertybag.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/embeddedobjectcontainer.hxx,comphelper/embeddedobjectcontainer.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/anytostring.hxx,comphelper/anytostring.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/implementationreference.hxx,comphelper/implementationreference.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/proparrhlp.hxx,comphelper/proparrhlp.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/PropertyInfoHash.hxx,comphelper/PropertyInfoHash.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/propstate.hxx,comphelper/propstate.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/proxyaggregation.hxx,comphelper/proxyaggregation.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/oslfile2streamwrap.hxx,comphelper/oslfile2streamwrap.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/genericpropertyset.hxx,comphelper/genericpropertyset.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/makesequence.hxx,comphelper/makesequence.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/ofopxmlhelper.hxx,comphelper/ofopxmlhelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/SettingsHelper.hxx,comphelper/SettingsHelper.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/string.hxx,comphelper/string.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/weak.hxx,comphelper/weak.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/componentmodule.hxx,comphelper/componentmodule.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/container.hxx,comphelper/container.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/seqstream.hxx,comphelper/seqstream.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/namecontainer.hxx,comphelper/namecontainer.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/processfactory.hxx,comphelper/processfactory.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/sequenceashashmap.hxx,comphelper/sequenceashashmap.hxx))
$(eval $(call gb_Package_add_file,comphelper_inc,inc/comphelper/configurationhelper.hxx,comphelper/configurationhelper.hxx))
