/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_descriptioninfoset.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 14:26:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_DESCRIPTIONINFOSET_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_DESCRIPTIONINFOSET_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#include "boost/optional.hpp"
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include "com/sun/star/uno/Sequence.hxx"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_MISC_API_HXX
#include "dp_misc_api.hxx"
#endif

/// @HTML

namespace com { namespace sun { namespace star {
    namespace uno { class XComponentContext; }
    namespace xml {
        namespace dom {
            class XNode;
            class XNodeList;
        }
        namespace xpath { class XXPathAPI; }
    }
} } }
namespace rtl { class OUString; }

namespace dp_misc {

/**
   Access to the content of an XML <code>description</code> element.

   <p>This works for <code>description</code> elements in both the
   <code>description.xml</code> file and online update information formats.</p>
*/
class DESKTOP_DEPLOYMENTMISC_DLLPUBLIC DescriptionInfoset {
public:
    /**
       Create an instance.

       @param context
       a non-null component context

       @param element
       a <code>description</code> element; may be null (equivalent to an element
       with no content)
    */
    DescriptionInfoset(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & context,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::dom::XNode > const & element);

    ~DescriptionInfoset();

    /**
       Return the identifier.

       @return
       the identifier, or an empty <code>optional</code> if none is specified
    */
    ::boost::optional< ::rtl::OUString > getIdentifier() const;

    /**
       Return the textual version representation.

       @return
       textual version representation
    */
    ::rtl::OUString getVersion() const;

    /**
       Return the dependencies.

       @return
       dependencies; will never be null
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::dom::XNodeList >
    getDependencies() const;

    /**
       Return the update information URLs.

       @return
       update information URLs
    */
    ::com::sun::star::uno::Sequence< ::rtl::OUString >
    getUpdateInformationUrls() const;

     /**
        Return the download URLs from the update information.

        @return
        download URLs
     */
    ::com::sun::star::uno::Sequence< ::rtl::OUString >
    getUpdateDownloadUrls() const;

    /**
       Return the download website URL from the update information.

       @return
       the download website URL, or an empty <code>optional</code> if none is
       specified
     */
    ::boost::optional< ::rtl::OUString > getUpdateWebsiteUrl() const;

    /**
       Allow direct access to the XPath functionality.

       @return
       direct access to the XPath functionality; null iff this instance was
       constructed with a null <code>element</code>
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::xpath::XXPathAPI >
    getXpath() const;

private:
    SAL_DLLPRIVATE ::boost::optional< ::rtl::OUString > getOptionalValue(
        ::rtl::OUString const & expression) const;

    SAL_DLLPRIVATE ::com::sun::star::uno::Sequence< ::rtl::OUString > getUrls(
        ::rtl::OUString const & expression) const;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::dom::XNode > m_element;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::xpath::XXPathAPI > m_xpath;
};

}

#endif
