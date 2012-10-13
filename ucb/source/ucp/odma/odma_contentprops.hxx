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
#ifndef ODMA_CONTENTPROPS_HXX
#define ODMA_CONTENTPROPS_HXX

#include "rtl/ref.hxx"
#include "salhelper/simplereferenceobject.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <functional>

namespace odma
{
    class ContentProperties : public salhelper::SimpleReferenceObject
    {
    public:
        com::sun::star::util::DateTime  m_aDateCreated; // when was the document created
        com::sun::star::util::DateTime  m_aDateModified;    // when was the document last modified
        ::rtl::OUString                 m_sTitle;       // Title
        ::rtl::OUString                 m_sContentType; // ContentType
        ::rtl::OString                  m_sDocumentId;  // the document id given from the DMS
        ::rtl::OUString                 m_sDocumentName;// document name
        ::rtl::OUString                 m_sFileURL;     // the temporary file location
        ::rtl::OUString                 m_sAuthor;      // the Author of the document
        ::rtl::OUString                 m_sSubject;     // the subject of the document
        ::rtl::OUString                 m_sKeywords;    // the keywords of the document
        ::rtl::OUString                 m_sSavedAsName; // the name which was used to save it
        sal_Bool                        m_bIsDocument;  // IsDocument
        sal_Bool                        m_bIsFolder;    // IsFolder
        sal_Bool                        m_bIsOpen;      // is true when OpenDoc was called
        sal_Bool                        m_bIsReadOnly;  // true when the document is read-only

        // @@@ Add other properties supported by your content.

        ContentProperties()
        :m_bIsDocument( sal_True )
        ,m_bIsFolder( sal_False )
        ,m_bIsOpen( sal_False )
        ,m_bIsReadOnly( sal_False )
        {}

        inline ::rtl::OUString getTitle()       const { return m_sTitle;        }
        inline ::rtl::OUString getDocumentId()  const { return ::rtl::OStringToOUString( m_sDocumentId, RTL_TEXTENCODING_ASCII_US ); }
        inline ::rtl::OUString getSavedAsName() const { return m_sSavedAsName;  }
    };
    typedef ::std::binary_function< ::rtl::Reference<ContentProperties>, ::rtl::OUString,bool> TContentPropertiesFunctorBase;
    /// binary_function Functor object for class ContentProperties return type is bool
    class ContentPropertiesMemberFunctor : public TContentPropertiesFunctorBase
    {
        ::std::const_mem_fun_t< ::rtl::OUString,ContentProperties> m_aFunction;
    public:
        ContentPropertiesMemberFunctor(const ::std::const_mem_fun_t< ::rtl::OUString,ContentProperties>& _rFunc)
            : m_aFunction(_rFunc){}

        inline bool operator()(const ::rtl::Reference<ContentProperties>& lhs,const ::rtl::OUString& rhs) const
        {
            return !!(m_aFunction(lhs.get()) == rhs);
        }
    };
}
#endif // ODMA_CONTENTPROPS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
