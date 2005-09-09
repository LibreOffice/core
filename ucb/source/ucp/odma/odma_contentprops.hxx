/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: odma_contentprops.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:51:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef ODMA_CONTENTPROPS_HXX
#define ODMA_CONTENTPROPS_HXX

#ifndef _VOS_REFERNCE_HXX_
#include <vos/refernce.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#include <functional>

namespace odma
{
    class ContentProperties : public ::vos::OReference
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
        inline ::rtl::OUString getSavedAsName() const { return m_sSavedAsName;  }
    };
    typedef ::std::binary_function< ::vos::ORef<ContentProperties>, ::rtl::OUString,bool> TContentPropertiesFunctorBase;
    /// binary_function Functor object for class ContentProperties return type is bool
    class ContentPropertiesMemberFunctor : public TContentPropertiesFunctorBase
    {
        ::std::const_mem_fun_t< ::rtl::OUString,ContentProperties> m_aFunction;
    public:
        ContentPropertiesMemberFunctor(const ::std::const_mem_fun_t< ::rtl::OUString,ContentProperties>& _rFunc)
            : m_aFunction(_rFunc){}

        inline bool operator()(const ::vos::ORef<ContentProperties>& lhs,const ::rtl::OUString& rhs) const
        {
            return !!(m_aFunction(lhs.getBodyPtr()) == rhs);
        }
    };
}
#endif // ODMA_CONTENTPROPS_HXX

