/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_UNOTOOLS_MEDIADESCRIPTOR_HXX
#define INCLUDED_UNOTOOLS_MEDIADESCRIPTOR_HXX

#include <sal/config.h>

#include <vector>

#include <rtl/ustring.hxx>
#include <unotools/unotoolsdllapi.h>

namespace com::sun::star::io {
    class XInputStream;
}
namespace com::sun::star::beans {
    struct NamedValue;
}
namespace com::sun::star::uno { class Any; }
namespace com::sun::star::uno { template <class E> class Sequence; }
namespace comphelper { class IDocPasswordVerifier; class SequenceAsHashMap; }
namespace comphelper { enum class DocPasswordRequestType; }

namespace utl::MediaDescriptor {

/** @short  can be used to work with a css::document::MediaDescriptor
            struct.

    @descr  Defines often used functions (as e.g. open of the required streams, consistent checks
            etcpp.) and all usable property names, for use with comphelper::SequenceAsHashMap,
            which represent the MediaDescriptor item.

    @attention  The functions aren't threadsafe and must be guarded from outside!
 */
constexpr inline OUString PROP_ABORTED = u"Aborted"_ustr;
constexpr inline OUString PROP_ASTEMPLATE = u"AsTemplate"_ustr;
constexpr inline OUString PROP_COMPONENTDATA = u"ComponentData"_ustr;
constexpr inline OUString PROP_DOCUMENTSERVICE = u"DocumentService"_ustr;
constexpr inline OUString PROP_ENCRYPTIONDATA = u"EncryptionData"_ustr;
constexpr inline OUString PROP_FILENAME = u"FileName"_ustr;
constexpr inline OUString PROP_FILTERNAME = u"FilterName"_ustr;
constexpr inline OUString PROP_FILTERPROVIDER = u"FilterProvider"_ustr;
constexpr inline OUString PROP_FILTEROPTIONS = u"FilterOptions"_ustr;
constexpr inline OUString PROP_FRAME = u"Frame"_ustr;
constexpr inline OUString PROP_FRAMENAME = u"FrameName"_ustr;
constexpr inline OUString PROP_HIDDEN = u"Hidden"_ustr;
constexpr inline OUString PROP_INPUTSTREAM = u"InputStream"_ustr;
constexpr inline OUString PROP_INTERACTIONHANDLER = u"InteractionHandler"_ustr;
constexpr inline OUString PROP_AUTHENTICATIONHANDLER = u"AuthenticationHandler"_ustr;
constexpr inline OUString PROP_JUMPMARK = u"JumpMark"_ustr;
constexpr inline OUString PROP_MACROEXECUTIONMODE = u"MacroExecutionMode"_ustr;
constexpr inline OUString PROP_MEDIATYPE = u"MediaType"_ustr;
constexpr inline OUString PROP_MINIMIZED = u"Minimized"_ustr;
constexpr inline OUString PROP_NOAUTOSAVE = u"NoAutoSave"_ustr;
constexpr inline OUString PROP_OPENNEWVIEW = u"OpenNewView"_ustr;
constexpr inline OUString PROP_OUTPUTSTREAM = u"OutputStream"_ustr;
constexpr inline OUString PROP_PASSWORD = u"Password"_ustr;
constexpr inline OUString PROP_POSTDATA = u"PostData"_ustr;
constexpr inline OUString PROP_PREVIEW = u"Preview"_ustr;
constexpr inline OUString PROP_READONLY = u"ReadOnly"_ustr;
constexpr inline OUString PROP_REFERRER = u"Referer"_ustr;
constexpr inline OUString PROP_REPLACEABLE = u"Replaceable"_ustr;
constexpr inline OUString PROP_SALVAGEDFILE = u"SalvagedFile"_ustr;
constexpr inline OUString PROP_SILENT = u"Silent"_ustr;
constexpr inline OUString PROP_STATUSINDICATOR = u"StatusIndicator"_ustr;
constexpr inline OUString PROP_STREAM = u"Stream"_ustr;
constexpr inline OUString PROP_STREAMFOROUTPUT = u"StreamForOutput"_ustr;
constexpr inline OUString PROP_TEMPLATENAME = u"TemplateName"_ustr;
constexpr inline OUString PROP_TITLE = u"Title"_ustr;
constexpr inline OUString PROP_TYPENAME = u"TypeName"_ustr;
constexpr inline OUString PROP_UCBCONTENT = u"UCBContent"_ustr;
constexpr inline OUString PROP_UPDATEDOCMODE = u"UpdateDocMode"_ustr;
constexpr inline OUString PROP_URL = u"URL"_ustr;
constexpr inline OUString PROP_VERSION = u"Version"_ustr;
constexpr inline OUString PROP_DOCUMENTTITLE = u"DocumentTitle"_ustr;
constexpr inline OUString PROP_MODEL = u"Model"_ustr;
constexpr inline OUString PROP_VIEWONLY = u"ViewOnly"_ustr;
constexpr inline OUString PROP_DOCUMENTBASEURL = u"DocumentBaseURL"_ustr;
constexpr inline OUString PROP_SUGGESTEDSAVEASNAME = u"SuggestedSaveAsName"_ustr;
constexpr inline OUString PROP_EXPORTDIRECTORY = u"ExportDirectory"_ustr;
constexpr inline OUString PROP_AUTOSAVEEVENT = u"AutoSaveEvent"_ustr;

/** @short  it checks if the descriptor already has a valid
            InputStream item and creates a new one, if not.

    @descr  This function uses the current items of this MediaDescriptor,
            to open the stream (as e.g. URL, ReadOnly, PostData etcpp.).
            It creates a seekable stream and put it into the descriptor.

            A might existing InteractionHandler will be used automatically,
            to solve problems!

            In case of local file the system file locking is used.

    @return TRUE, if the stream was already part of the descriptor or could
            be created as new item. FALSE otherwise.
 */
UNOTOOLS_DLLPUBLIC bool addInputStream(comphelper::SequenceAsHashMap& rMediaDescriptor);

/** @short  it checks if the descriptor already has a valid
            InputStream item and creates a new one, if not.

    @descr  This function uses the current items of this MediaDescriptor,
            to open the stream (as e.g. URL, ReadOnly, PostData etcpp.).
            It creates a seekable stream and put it into the descriptor.

            A might existing InteractionHandler will be used automatically,
            to solve problems!

            In case of local file the system file locking is used based on
            configuration settings.

    @return TRUE, if the stream was already part of the descriptor or could
            be created as new item. FALSE otherwise.
 */
UNOTOOLS_DLLPUBLIC bool addInputStreamOwnLock(comphelper::SequenceAsHashMap& rMediaDescriptor);

/** @short  it checks if the descriptor describes a readonly stream.

    @descr  The descriptor itself isn't changed doing so.
            It's only checked if the stream seems to be based
            of a real readonly file.

    @Attention
            We don't check the property "ReadOnly" here. Because
            this property can be set from outside and overwrites
            the readonly state of  the stream.
            If e.g. the stream could be opened read/write ...
            but "ReadOnly" property is set to TRUE, this means:
            show a readonly UI on top of this read/write stream.

    @return TRUE, if the stream must be interpreted as readonly ...
            FALSE otherwise.
 */
UNOTOOLS_DLLPUBLIC bool isStreamReadOnly(const comphelper::SequenceAsHashMap& rMediaDescriptor);

/** Returns a value from the sequence contained in the property
    'ComponentData' of this media descriptor.

    @descr  The property 'ComponentData' should be empty, or should
        contain a value of type sequence<com.sun.star.beans.NamedValue>
        or sequence<com.sun.star.beans.PropertyValue>.

    @return  The value with the specified name, if existing in the
        sequence of the 'ComponentData' property, otherwise an empty
        Any.
 */
UNOTOOLS_DLLPUBLIC css::uno::Any
getComponentDataEntry(const comphelper::SequenceAsHashMap& rMediaDescriptor, const OUString& rName);

/** Inserts a value into the sequence contained in the property
    'ComponentData' of the media descriptor.

    @descr  The property 'ComponentData' should be empty, or should
        contain a value of type sequence<com.sun.star.beans.NamedValue>
        or sequence<com.sun.star.beans.PropertyValue>. The passed value
        will be inserted into the sequence, or, if already existing,
        will be overwritten.

    @param rName  The name of the value to be inserted into the
        sequence of the 'ComponentData' property.

    @param rValue  The value to be inserted into the sequence of the
        'ComponentData' property.
 */
UNOTOOLS_DLLPUBLIC void setComponentDataEntry(comphelper::SequenceAsHashMap& rMediaDescriptor,
                                              const OUString& rName, const css::uno::Any& rValue);

/** Removes a value from the sequence contained in the property
    'ComponentData' of the media descriptor.

    @descr  The property 'ComponentData' should be empty, or should
        contain a value of type sequence<com.sun.star.beans.NamedValue>
        or sequence<com.sun.star.beans.PropertyValue>. The value with
        the passed name will be removed from the sequence, if existing.

    @param rName  The name of the value to be removed from the sequence
        of the 'ComponentData' property.
 */
UNOTOOLS_DLLPUBLIC void clearComponentDataEntry(comphelper::SequenceAsHashMap& rMediaDescriptor,
                                                const OUString& rName);

/** This helper function tries to find a password for the document
    described by this media descriptor.

    First, the list of default passwords will be tried if provided. This
    is needed by import filters for external file formats that have to
    check a predefined password in some cases without asking the user
    for a password. Every password is checked using the passed password
    verifier.

    If not successful, this media descriptor is asked for a password,
    that has been set e.g. by an API call to load a document. If
    existing, the password is checked using the passed password
    verifier.

    If still not successful, the interaction handler contained in this
    media descriptor is used to request a password from the user. This
    will be repeated until the passed password verifier validates the
    entered password, or if the user chooses to cancel password input.

    If a valid password (that is not contained in the passed list of
    default passwords) was found, it will be inserted into the
    "Password" property of this descriptor.

    @param rVerifier
    The password verifier used to check every processed password.

    @param eRequestType
    The password request type that will be passed to the
    DocPasswordRequest object created internally. See
    docpasswordrequest.hxx for more details.

    @param pDefaultPasswords
    If not null, contains default passwords that will be tried before a
    password will be requested from the media descriptor or the user.

    @return
    If not empty, contains the password that has been validated by the
    passed password verifier. If empty, no valid password has been
    found, or the user has chosen to cancel password input.
 */
UNOTOOLS_DLLPUBLIC css::uno::Sequence<css::beans::NamedValue>
requestAndVerifyDocPassword(comphelper::SequenceAsHashMap& rMediaDescriptor,
                            comphelper::IDocPasswordVerifier& rVerifier,
                            comphelper::DocPasswordRequestType eRequestType,
                            const ::std::vector<OUString>* pDefaultPasswords);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
