/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ResourceId.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:04:09 $
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

#ifndef SD_FRAMEWORK_RESOURCE_ID_HXX
#define SD_FRAMEWORK_RESOURCE_ID_HXX

#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <cppuhelper/compbase2.hxx>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;


namespace {

typedef ::cppu::WeakImplHelper2 <
    css::drawing::framework::XResourceId,
    css::lang::XInitialization
    > ResourceIdInterfaceBase;

} // end of anonymous namespace.




namespace sd { namespace framework {

/** Implementation of the css::drawing::framework::ResourceId
    service and the css::drawing::framework::XResourceId
    interface.
*/
class ResourceId
    : public ResourceIdInterfaceBase
{
public:
    /** Create a new, empty resource id.
    */
    ResourceId (void);

    /** Create a new resource id that is described by the given URLs.
        @param rsResourceURLs
            The first URL specifies the type of resource.  The other URLs
            describe its anchor.
            The set of URLs may be empty.  The result is then the same as
            returned by ResourceId() default constructor.
    */
    ResourceId (const ::std::vector<rtl::OUString>& rsResourceURLs);

    /** Create a new resource id that has an empty anchor.
        @param rsResourceURL
            When this resource URL is empty then the resulting ResourceId
            object is identical to when the ResourceId() default constructor
            had been called.
    */
    ResourceId (
        const ::rtl::OUString& rsResourceURL);

    /** Create a new resource id for the given resource type and an anchor
        that is specified by a single URL.  This constructor can be used for
        example for views that are bound to panes.
        @param rsResourceURL
            The URL of the actual resource.
        @param rsAnchorURL
            The single URL of the anchor.
    */
    ResourceId (
        const ::rtl::OUString& rsResourceURL,
        const ::rtl::OUString& rsAnchorURL);

    /** Create a new resource id for the specified resource type and the
        given list of anchor URLs.
        @param rsResourceURL
            The URL of the actual resource.
        @param rsAnchorURLs
            The possibly empty list of anchor URLs.
    */
    ResourceId (
        const ::rtl::OUString& rsResourceURL,
        const ::std::vector<rtl::OUString>& rAnchorURLs);

    /** Create a new resource id with an anchor that consists of a sequence
        of URLs that is extended by a further URL.
        @param rsResourceURL
            The URL of the actual resource.
        @param rsFirstAnchorURL
            This URL extends the anchor given by rAnchorURLs.
        @param rAnchorURLs
            An anchor as it is returned by XResourceId::getAnchorURLs().
    */
    ResourceId (
        const ::rtl::OUString& rsResourceURL,
        const ::rtl::OUString& rsFirstAnchorURL,
        const css::uno::Sequence<rtl::OUString>& rAnchorURLs);

    virtual ~ResourceId (void);

    /** Return the list of URLs of both the resource URL and the anchor
        URLs.
    */
    const ::std::vector<rtl::OUString>& GetResourceURLs (void) const;

    //===== XResourceId =======================================================

    virtual ::rtl::OUString SAL_CALL
        getResourceURL (void)
        throw(css::uno::RuntimeException);

    virtual css::util::URL SAL_CALL
        getFullResourceURL (void)
        throw(css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        hasAnchor (void)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<
        css::drawing::framework::XResourceId> SAL_CALL
        getAnchor (void)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence<rtl::OUString> SAL_CALL
        getAnchorURLs (void)
        throw (css::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL
        getResourceTypePrefix (void)
        throw (css::uno::RuntimeException);

    virtual sal_Int16 SAL_CALL
        compareTo (const css::uno::Reference<
            css::drawing::framework::XResourceId>& rxResourceId)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        isBoundTo (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxResourceId,
            css::drawing::framework::AnchorBindingMode eMode)
        throw(css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        isBoundToURL (
            const ::rtl::OUString& rsAnchorURL,
            css::drawing::framework::AnchorBindingMode eMode)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<
        css::drawing::framework::XResourceId> SAL_CALL
        clone (void)
        throw(css::uno::RuntimeException);

    //===== XInitialization ===================================================

    void SAL_CALL initialize (
        const css::uno::Sequence<css::uno::Any>& aArguments)
        throw (css::uno::RuntimeException);

private:
    /** The set of URLs that consist of the resource URL at index 0 and the
        anchor URLs and indices 1 and above.
    */
    ::std::vector<rtl::OUString> maResourceURLs;

    ::boost::scoped_ptr<css::util::URL> mpURL;

    static css::uno::WeakReference<css::util::XURLTransformer> mxURLTransformerWeak;

    /** Compare the called ResourceId object to the given ResourceId object.
        This uses the implementation of both objects to speed up the
        comparison.
    */
    sal_Int16 CompareToLocalImplementation (const ResourceId& rId) const;

    /** Compare the called ResourceId object to the given XResourceId object
        reference.  The comparison is done via the UNO interface.  Namely,
        it uses the getResourceURL() and the getAnchorURLs() methods to get
        access to the URLs of the given objec.
    */
    sal_Int16 CompareToExternalImplementation (const css::uno::Reference<
        css::drawing::framework::XResourceId>& rxId) const;

    /** Return whether the called ResourceId object is bound to the anchor
        consisting of the URLs given by psFirstAnchorURL and paAnchorURLs.
        @param psFirstAnchorURL
            Optional first URL of the anchor. This can be missing or present
            independently of paAnchorURLs.
        @param paAnchorURLs
            Optional set of additional anchor URLs.  This can be missing or
            present independently of psFirstAnchorURL.
        @param eMode
            This specifies whether the called resource has to be directly
            bound to the given anchor in order to return <TRUE/> or whether
            it can be bound indirectly, too.
    */
    bool IsBoundToAnchor (
        const rtl::OUString* psFirstAnchorURL,
        const css::uno::Sequence<rtl::OUString>* paAnchorURLs,
        css::drawing::framework::AnchorBindingMode eMode) const;

    /** Return whether the called ResourceId object is bound to the anchor
        consisting of the URLs in rResourceURLs.
        @param rResourceURLs
            A possibly empty list of anchor URLs.
        @param eMode
            This specifies whether the called resource has to be directly
            bound to the given anchor in order to return <TRUE/> or whether
            it can be bound indirectly, too.
    */
    bool IsBoundToAnchor (
        const ::std::vector<rtl::OUString>& rResourceURLs,
        css::drawing::framework::AnchorBindingMode eMode) const;

    bool IsValid (void) const;

    void ParseResourceURL (void);
};

} } // end of namespace sd::framework

#endif
