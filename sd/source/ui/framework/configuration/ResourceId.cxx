/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ResourceId.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 15:47:44 $
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

#include "precompiled_sd.hxx"

#include "framework/ResourceId.hxx"

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include <rtl/ref.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

/** When the USE_OPTIMIZATIONS symbol is defined then at some optimizations
    are activated that work only together with XResourceId objects that are
    implemented by the ResourceId class.  For other implementations of when
    the USE_OPTIMIZATIONS symbol is not defined then alternative code is
    used instead.
*/
#define USE_OPTIMIZATIONS

namespace sd { namespace framework {

Reference<XInterface> SAL_CALL ResourceId_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    (void)rxContext;
    return Reference<XInterface>(static_cast<XWeak*>(new ::sd::framework::ResourceId()));
}




::rtl::OUString ResourceId_getImplementationName (void) throw(RuntimeException)
{
    return ::rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Draw.framework.ResourceId"));
}




Sequence<rtl::OUString> SAL_CALL ResourceId_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const ::rtl::OUString sServiceName(
        ::rtl::OUString::createFromAscii("com.sun.star.drawing.framework.ResourceId"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




//===== ResourceId ============================================================

ResourceId::ResourceId (void)
    : ResourceIdInterfaceBase(),
      maResourceURLs(0)
{
}




ResourceId::ResourceId (
    const std::vector<OUString>& rResourceURLs)
    : ResourceIdInterfaceBase(),
      maResourceURLs(rResourceURLs)
{
}




ResourceId::ResourceId (
    const OUString& rsResourceURL)
    : ResourceIdInterfaceBase(),
      maResourceURLs(1, rsResourceURL)
{
    // Handle the special case of an empty resource URL.
    if (rsResourceURL.getLength() == 0)
        maResourceURLs.clear();
}




ResourceId::ResourceId (
    const OUString& rsResourceURL,
    const OUString& rsAnchorURL)
    : ResourceIdInterfaceBase(),
      maResourceURLs(2)
{
    maResourceURLs[0] = rsResourceURL;
    maResourceURLs[1] = rsAnchorURL;
}




ResourceId::ResourceId (
    const OUString& rsResourceURL,
    const ::std::vector<OUString>& rAnchorURLs)
    : ResourceIdInterfaceBase(),
      maResourceURLs(1+rAnchorURLs.size())
{
    maResourceURLs[0] = rsResourceURL;
    for (sal_uInt32 nIndex=0; nIndex<rAnchorURLs.size(); ++nIndex)
        maResourceURLs[nIndex+1] = rAnchorURLs[nIndex];
}




ResourceId::ResourceId (
    const OUString& rsResourceURL,
    const OUString& rsFirstAnchorURL,
    const Sequence<OUString>& rAnchorURLs)
    : ResourceIdInterfaceBase(),
      maResourceURLs(2+rAnchorURLs.getLength())
{
    maResourceURLs[0] = rsResourceURL;
    maResourceURLs[1] = rsFirstAnchorURL;
    for (sal_Int32 nIndex=0; nIndex<rAnchorURLs.getLength(); ++nIndex)
        maResourceURLs[nIndex+2] = rAnchorURLs[nIndex];
}




ResourceId::~ResourceId (void)
{
}




OUString SAL_CALL
    ResourceId::getResourceURL (void)
    throw(com::sun::star::uno::RuntimeException)
{
    if (maResourceURLs.size() > 0)
        return maResourceURLs[0];
    else
        return OUString();
}




sal_Bool SAL_CALL
    ResourceId::hasAnchor (void)
    throw (RuntimeException)
{
    return maResourceURLs.size()>1;
}




Reference<XResourceId> SAL_CALL
    ResourceId::getAnchor (void)
    throw (RuntimeException)
{
    ::rtl::Reference<ResourceId> rResourceId (new ResourceId());
    const sal_Int32 nAnchorCount (maResourceURLs.size()-1);
    if (nAnchorCount > 0)
    {
        rResourceId->maResourceURLs.resize(nAnchorCount);
        for (sal_Int32 nIndex=0; nIndex<nAnchorCount; ++nIndex)
            rResourceId->maResourceURLs[nIndex] = maResourceURLs[nIndex+1];
    }
    return Reference<XResourceId>(rResourceId.get());
}




Sequence<OUString> SAL_CALL
    ResourceId::getAnchorURLs (void)
    throw (RuntimeException)
{
    const sal_Int32 nAnchorCount (maResourceURLs.size() - 1);
    if (nAnchorCount > 0)
    {
        Sequence<OUString> aAnchorURLs (nAnchorCount);
        for (sal_Int32 nIndex=0; nIndex<nAnchorCount; ++nIndex)
            aAnchorURLs[nIndex] = maResourceURLs[nIndex+1];
        return aAnchorURLs;
    }
    else
        return Sequence<OUString>();
}




OUString SAL_CALL
    ResourceId::getResourceTypePrefix (void)
    throw (RuntimeException)
{
    if (maResourceURLs.size() > 0)
    {
        // Return the "private:resource/<type>/" prefix.

        // Get the the prefix that ends with the second "/".
        const OUString& rsResourceURL (maResourceURLs[0]);
        sal_Int32 nPrefixEnd (rsResourceURL.indexOf(sal_Unicode('/'), 0));
        if (nPrefixEnd >= 0)
            nPrefixEnd = rsResourceURL.indexOf(sal_Unicode('/'), nPrefixEnd+1) + 1;
        else
            nPrefixEnd = 0;

        return rsResourceURL.copy(0,nPrefixEnd);
    }
    else
        return OUString();
}




sal_Int16 SAL_CALL
    ResourceId::compareTo (const Reference<XResourceId>& rxResourceId)
    throw (RuntimeException)
{
    sal_Int16 nResult (0);

    if ( ! rxResourceId.is())
    {
        // The empty reference is interpreted as empty resource id object.
        if (maResourceURLs.size() > 0)
            nResult = +1;
        else
            nResult = 0;
    }
    else
    {
        ResourceId* pId = NULL;
#ifdef USE_OPTIMIZATIONS
        pId = dynamic_cast<ResourceId*>(rxResourceId.get());
#endif
        if (pId != NULL)
        {
            // We have direct access to the implementation of the given
            // resource id object.
            nResult = CompareToLocalImplementation(*pId);
        }
        else
        {
            // We have to do the comparison via the UNO interface of the
            // given resource id object.
            nResult = CompareToExternalImplementation(rxResourceId);
        }
    }

    return nResult;
}




sal_Int16 ResourceId::CompareToLocalImplementation (const ResourceId& rId) const
{
    sal_Int16 nResult (0);

    sal_uInt32 nLocalURLCount (maResourceURLs.size());
    sal_uInt32 nURLCount(rId.maResourceURLs.size());
    if (nLocalURLCount != nURLCount)
    {
        if (nLocalURLCount < nURLCount)
            nResult = -1;
        else
            nResult = +1;
    }
    else
    {
        for (sal_uInt32 nIndex=0; nIndex<nLocalURLCount; ++nIndex)
        {
            sal_Int32 nLocalResult ( maResourceURLs[nIndex].compareTo(rId.maResourceURLs[nIndex]));
            if (nLocalResult != 0)
            {
                if (nLocalResult < 0)
                    nResult = -1;
                else
                    nResult = +1;
                break;
            }
        }
    }

    return nResult;
}




sal_Int16 ResourceId::CompareToExternalImplementation (const Reference<XResourceId>& rxId) const
{
    sal_Int16 nResult (0);

    const Sequence<OUString> aAnchorURLs (rxId->getAnchorURLs());
    sal_uInt32 nLocalURLCount (maResourceURLs.size());
    sal_uInt32 nURLCount(1+aAnchorURLs.getLength());

    if (nLocalURLCount != nURLCount)
    {
        if (nLocalURLCount < nURLCount)
            nResult = -1;
        else
            nResult = +1;
    }
    else
    {
        sal_Int32 nLocalResult (0);
        for (sal_uInt32 nIndex=0; nIndex<nLocalURLCount; ++nIndex)
        {
            if (nIndex == 0 )
                nLocalResult = maResourceURLs[nIndex].compareTo(rxId->getResourceURL());
            else
                nLocalResult = maResourceURLs[nIndex].compareTo(aAnchorURLs[nIndex-1]);
            if (nLocalResult != 0)
            {
                if (nLocalResult < 0)
                    nResult = -1;
                else
                    nResult = +1;
                break;
            }
        }
    }

    return nResult;
}




sal_Bool SAL_CALL
    ResourceId::isBoundTo (
        const Reference<XResourceId>& rxResourceId,
        AnchorBindingMode eMode)
    throw (RuntimeException)
{
    if ( ! rxResourceId.is())
    {
        // An empty reference is interpreted as empty resource id.
        return IsBoundToAnchor(NULL, NULL, eMode);
    }

    ResourceId* pId = NULL;
#ifdef USE_OPTIMIZATIONS
    pId = dynamic_cast<ResourceId*>(rxResourceId.get());
#endif
    if (pId != NULL)
    {
        return IsBoundToAnchor(pId->maResourceURLs, eMode);
    }
    else
    {
        const OUString sResourceURL (rxResourceId->getResourceURL());
        const Sequence<OUString> aAnchorURLs (rxResourceId->getAnchorURLs());
        return IsBoundToAnchor(&sResourceURL, &aAnchorURLs, eMode);
    }
}




sal_Bool SAL_CALL
    ResourceId::isBoundToURL (
        const OUString& rsAnchorURL,
        AnchorBindingMode eMode)
    throw (RuntimeException)
{
    return IsBoundToAnchor(&rsAnchorURL, NULL, eMode);
}




Reference<XResourceId> SAL_CALL
    ResourceId::clone (void)
    throw(RuntimeException)
{
    return new ResourceId(maResourceURLs);
}




//----- XInitialization -------------------------------------------------------

void SAL_CALL ResourceId::initialize (const Sequence<Any>& aArguments)
    throw (RuntimeException)
{
    sal_uInt32 nCount (aArguments.getLength());
    for (sal_uInt32 nIndex=0; nIndex<nCount; ++nIndex)
    {
        OUString sResourceURL;
        if (aArguments[nIndex] >>= sResourceURL)
            maResourceURLs.push_back(sResourceURL);
        else
        {
            Reference<XResourceId> xAnchor;
            if (aArguments[nIndex] >>= xAnchor)
            {
                if (xAnchor.is())
                {
                    maResourceURLs.push_back(xAnchor->getResourceURL());
                    Sequence<OUString> aAnchorURLs (xAnchor->getAnchorURLs());
                    for (sal_Int32 nURLIndex=0; nURLIndex<aAnchorURLs.getLength(); ++nURLIndex)
                    {
                        maResourceURLs.push_back(aAnchorURLs[nURLIndex]);
                    }
                }
            }

        }
    }
}




//-----------------------------------------------------------------------------

/** When eMode is DIRECTLY then the anchor of the called object and the
    anchor represented by the given sequence of anchor URLs have to be
    identical.   When eMode is RECURSIVE then the anchor of the called
    object has to start with the given anchor URLs.
*/
bool ResourceId::IsBoundToAnchor (
    const OUString* psFirstAnchorURL,
    const Sequence<OUString>* paAnchorURLs,
    AnchorBindingMode eMode) const
{
    const sal_uInt32 nLocalAnchorURLCount (maResourceURLs.size() - 1);
    const bool bHasFirstAnchorURL (psFirstAnchorURL!=NULL);
    const sal_uInt32 nAnchorURLCount ((bHasFirstAnchorURL?1:0)
        + (paAnchorURLs!=NULL ? paAnchorURLs->getLength() : 0));

    // Check the lengths.
    if (nLocalAnchorURLCount<nAnchorURLCount ||
        (eMode==AnchorBindingMode_DIRECT && nLocalAnchorURLCount!=nAnchorURLCount))
    {
        return false;
    }

    // Compare the nAnchorURLCount bottom-most anchor URLs of this resource
    // id and the given anchor.
    sal_uInt32 nOffset = 0;
    if (paAnchorURLs != NULL)
    {
        sal_uInt32 nCount = paAnchorURLs->getLength();
        while (nOffset < nCount)
        {
            if ( ! maResourceURLs[nLocalAnchorURLCount - nOffset].equals(
                (*paAnchorURLs)[nCount - 1 - nOffset]))
            {
                return false;
            }
            ++nOffset;
        }
    }
    if (bHasFirstAnchorURL)
    {
        if ( ! psFirstAnchorURL->equals(maResourceURLs[nLocalAnchorURLCount - nOffset]))
            return false;
    }

    return true;
}




bool ResourceId::IsBoundToAnchor (
    const ::std::vector<OUString>& rAnchorURLs,
    AnchorBindingMode eMode) const
{
    const sal_uInt32 nLocalAnchorURLCount (maResourceURLs.size() - 1);
    const sal_uInt32 nAnchorURLCount (rAnchorURLs.size());

    // Check the lengths.
    if (nLocalAnchorURLCount<nAnchorURLCount ||
        (eMode==AnchorBindingMode_DIRECT && nLocalAnchorURLCount!=nAnchorURLCount))
    {
        return false;
    }

    // Compare the nAnchorURLCount bottom-most anchor URLs of this resource
    // id and the given anchor.
    for (sal_uInt32 nOffset=0; nOffset<nAnchorURLCount; ++nOffset)
    {
        if ( ! maResourceURLs[nLocalAnchorURLCount - nOffset].equals(
            rAnchorURLs[nAnchorURLCount - 1 - nOffset]))
        {
            return false;
        }
    }

    return true;
}




bool ResourceId::IsValid (void) const
{
    return maResourceURLs.size() == 0
        || maResourceURLs[0].getLength()>0
        || maResourceURLs.size() == 1;

}



} } // end of namespace sd::framework
