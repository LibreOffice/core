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

#ifndef _COMPHELPER_MEDIADESCRIPTOR_HXX_
#define _COMPHELPER_MEDIADESCRIPTOR_HXX_

#include <comphelper/sequenceashashmap.hxx>
#include <rtl/ustring.hxx>
#include "comphelper/comphelperdllapi.h"

namespace com { namespace sun { namespace star { namespace io {
    class XInputStream;
} } } }

namespace comphelper{

/** @short  can be used to work with a ::com::sun::star::document::MediaDescriptor
            struct.

    @descr  It wraps a ::boost::unordered_map around the Sequence< css::beans::PropertyValue >, which
            represent the MediaDescriptor item.
            Further this helper defines often used functions (as e.g. open of the required streams,
            consistent checks etcpp.) and it defines all useable property names.

    @attention  This class isnt threadsafe and must be guarded from outside!
 */
class COMPHELPER_DLLPUBLIC MediaDescriptor : public SequenceAsHashMap
{
    public:

        //---------------------------------------
        /** @short  these methods can be used to get the different property names
                    as static const OUString values.

            @descr  Because definition and declaration of static const class members
                    does not work as expected under windows (under unix it works as well)
                    these way must be used :-(
          */
        static const OUString& PROP_ABORTED();
        static const OUString& PROP_ASTEMPLATE();
        static const OUString& PROP_COMPONENTDATA();
        static const OUString& PROP_DOCUMENTSERVICE();
        static const OUString& PROP_ENCRYPTIONDATA();
        static const OUString& PROP_FILENAME();
        static const OUString& PROP_FILTERNAME();
        static const OUString& PROP_FILTERPROVIDER();
        static const OUString& PROP_FILTEROPTIONS();
        static const OUString& PROP_FRAME();
        static const OUString& PROP_FRAMENAME();
        static const OUString& PROP_HIDDEN();
        static const OUString& PROP_INPUTSTREAM();
        static const OUString& PROP_INTERACTIONHANDLER();
        static const OUString& PROP_JUMPMARK();
        static const OUString& PROP_MACROEXECUTIONMODE();
        static const OUString& PROP_MEDIATYPE();
        static const OUString& PROP_MINIMIZED();
        static const OUString& PROP_NOAUTOSAVE();
        static const OUString& PROP_OPENNEWVIEW();
        static const OUString& PROP_OUTPUTSTREAM();
        static const OUString& PROP_PASSWORD();
        static const OUString& PROP_POSTDATA();
        static const OUString& PROP_PREVIEW();
        static const OUString& PROP_READONLY();
        static const OUString& PROP_REFERRER();
        static const OUString& PROP_SALVAGEDFILE();
        static const OUString& PROP_STATUSINDICATOR();
        static const OUString& PROP_STREAM();
        static const OUString& PROP_STREAMFOROUTPUT();
        static const OUString& PROP_TEMPLATENAME();
        static const OUString& PROP_TITLE();
        static const OUString& PROP_TYPENAME();
        static const OUString& PROP_UCBCONTENT();
        static const OUString& PROP_UPDATEDOCMODE();
        static const OUString& PROP_URL();
        static const OUString& PROP_VERSION();
        static const OUString& PROP_DOCUMENTTITLE();
        static const OUString& PROP_MODEL();
        static const OUString& PROP_VIEWONLY();
        static const OUString& PROP_DOCUMENTBASEURL();

        static const OUString& PROP_DEEPDETECTION();

    //-------------------------------------------
    // interface
    public:
        //---------------------------------------
        /** @short  these ctors do nothing - excepting that they forward
                    the given parameters to the base class ctors.

            @descr  The ctros must be overwritten to resolve conflicts with
                    the default ctors of the compiler :-(.
         */
        MediaDescriptor();
        MediaDescriptor(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lSource);

        //---------------------------------------
        /** @short  it checks if the descriptor already has a valid
                    InputStream item and creates a new one, if not.

            @descr  This method uses the current items of this MediaDescriptor,
                    to open the stream (as e.g. URL, ReadOnly, PostData etcpp.).
                    It creates a seekable stream and put it into the descriptor.

                    A might existing InteractionHandler will be used automaticly,
                    to solve problems!

                    In case of local file the system file locking is used.

            @return TRUE, if the stream was already part of the descriptor or could
                    be created as new item. FALSE otherwise.
         */
        sal_Bool addInputStream();

        //---------------------------------------
        /** @short  it checks if the descriptor already has a valid
                    InputStream item and creates a new one, if not.

            @descr  This method uses the current items of this MediaDescriptor,
                    to open the stream (as e.g. URL, ReadOnly, PostData etcpp.).
                    It creates a seekable stream and put it into the descriptor.

                    A might existing InteractionHandler will be used automaticly,
                    to solve problems!

                    In case of local file the system file locking is used based on
                    configuration settings.

            @return TRUE, if the stream was already part of the descriptor or could
                    be created as new item. FALSE otherwise.
         */
        sal_Bool addInputStreamOwnLock();

        //---------------------------------------
        /** @short  it checks if the descriptor describes a readonly stream.

            @descr  The descriptor itself isnt changed doing so.
                    It's only checked if the stream seems to be based
                    of a real readonly file.

            @Attention
                    We dont check the property "ReadOnly" here. Because
                    this property can be set from outside and overwrites
                    the readonly state of  the stream.
                    If e.g. the stream could be opened read/write ...
                    but "ReadOnly" property is set to TRUE, this means:
                    show a readonly UI on top of this read/write stream.

            @return TRUE, if the stream must be interpreted as readonly ...
                    FALSE otherwise.
         */
        sal_Bool isStreamReadOnly() const;

        //---------------------------------------
        /** Returns a value from the sequence contained in the property
            'ComponentData' of this media descriptor.

            @descr  The property 'ComponentData' should be empty, or should
                contain a value of type sequence<com.sun.star.beans.NamedValue>
                or sequence<com.sun.star.beans.PropertyValue>.

            @return  The value with the specified name, if existing in the
                sequence of the 'ComponentData' property, otherwise an empty
                Any.
         */
        ::com::sun::star::uno::Any getComponentDataEntry(
            const OUString& rName ) const;

        //---------------------------------------
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
        void setComponentDataEntry(
            const OUString& rName,
            const ::com::sun::star::uno::Any& rValue );

        //---------------------------------------
        /** Removes a value from the sequence contained in the property
            'ComponentData' of the media descriptor.

            @descr  The property 'ComponentData' should be empty, or should
                contain a value of type sequence<com.sun.star.beans.NamedValue>
                or sequence<com.sun.star.beans.PropertyValue>. The value with
                the passed name will be removed from the sequence, if existing.

            @param rName  The name of the value to be removed from the sequence
                of the 'ComponentData' property.
         */
        void clearComponentDataEntry(
            const OUString& rName );

    //-------------------------------------------
    // helper
    private:

        //---------------------------------------
        /** @short  tries to open a stream by using the given PostData stream.

            @descr  The stream is used directly ...

                    The MediaDescriptor itself is changed inside this method.
                    Means: the stream is added internal and not returned by a value.

            @param  _rxPostData
                    the PostData stream.

            @return TRUE if the stream could be added successfully.
                    Note: If FALSE is returned, the error was already handled inside!

            @throw  [css::uno::RuntimeException]
                    if the MediaDescriptor seems to be invalid!

            @throw  [css::lang::IllegalArgumentException]
                    if the given PostData stream is <NULL/>.
         */
        COMPHELPER_DLLPRIVATE sal_Bool impl_openStreamWithPostData(
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& _rxPostData
            )   throw(::com::sun::star::uno::RuntimeException);

        //---------------------------------------
        /** @short  tries to open a stream by using the given URL.

            @descr  First it tries to open the content in r/w mode (if its
                    allowed to do so). Only in case its not allowed or it failed
                    the stream will be tried to open in readonly mode.

                    The MediaDescriptor itself is changed inside this method.
                    Means: the stream is added internal and not returned by a value.

            @param  sURL
                    the URL for open.

            @param  bLockFile
                    specifies whether the file should be locked

            @return TRUE if the stream could be added successfully.
                    Note: If FALSE is returned, the error was already handled inside!

            @throw  [css::uno::RuntimeException]
                    if the MediaDescriptor seems to be invalid!
         */
        COMPHELPER_DLLPRIVATE sal_Bool impl_openStreamWithURL(
            const OUString& sURL,
            sal_Bool bLockFile
            ) throw(::com::sun::star::uno::RuntimeException);

        //---------------------------------------
        /** @short  it checks if the descriptor already has a valid
                    InputStream item and creates a new one, if not.

            @descr  This method uses the current items of this MediaDescriptor,
                    to open the stream (as e.g. URL, ReadOnly, PostData etcpp.).
                    It creates a seekable stream and put it into the descriptor.

                    A might existing InteractionHandler will be used automaticly,
                    to solve problems!

            @param  bLockFile
                    specifies whether the file should be locked

            @return TRUE, if the stream was already part of the descriptor or could
                    be created as new item. FALSE otherwise.
         */
        COMPHELPER_DLLPRIVATE sal_Bool impl_addInputStream( sal_Bool bLockFile );
};

} // namespace comphelper

#endif // _COMPHELPER_MEDIADESCRIPTOR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
