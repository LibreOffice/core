#ifndef REPORTDESIGN_TOOLS_HXX
#define REPORTDESIGN_TOOLS_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Tools.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:44:34 $
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

#ifndef _COM_SUN_STAR_REPORT_XREPORTDEFINITION_HPP_
#include <com/sun/star/report/XReportDefinition.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XSECTION_HPP_
#include <com/sun/star/report/XSection.hpp>
#endif
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/container/XChild.hpp>
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#include "Section.hxx"
#include "corestrings.hrc"

namespace reportdesign
{
    template <class T> void lcl_createSectionIfNeeded(sal_Bool _bOn,const T& _xParent,::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection/*in/out*/,bool _bPageSection = false)
    {
        if ( _bOn && !_xSection.is() )
            _xSection = new OSection(_xParent,_xParent->getContext(),_bPageSection);
        else if ( !_bOn )
            //_xSection.clear();
            ::comphelper::disposeComponent(_xSection);
    }

    /** gets the properties which should be removed form the property set implementation.
     *
     * \return A sequence of all properties which should be removed for none char able implementations.
     */
    ::com::sun::star::uno::Sequence< ::rtl::OUString > lcl_getCharOptionals();

    /** uses the XChild interface to get the section from any child of it.
     *
     * \param _xReportComponent A report component which is a child of the section.
     * \return The sectin where this report component resists in.
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection> lcl_getSection(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xReportComponent);

    /** throws an illegal argument exception. The message text is the resource RID_STR_ERROR_WRONG_ARGUMENT + the type as reference.
     *
     * \param _sTypeName The reference where to look for the correct values.
     * \param ExceptionContext_ The exception context.
     * \param ArgumentPosition_ The argument position.
     * \param Context_ The context to get the factory service.
     */
    void throwIllegallArgumentException(const ::rtl::OUString& _sTypeName
                                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& ExceptionContext_
                                        ,const ::sal_Int16& ArgumentPosition_
                                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context_);

    /** clones the given object
    *
    * \param _xReportComponent the object to be cloned
    * \param _xFactory  the factory to create the clone
    * \param _sServiceName the service of the to be cloned object
    * \return the clone
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > cloneObject(
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xReportComponent
                                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _xFactory
                                        ,const ::rtl::OUString& _sServiceName);

    class OShapeHelper
    {
    public:
        template<typename T> static void setSize(const ::com::sun::star::awt::Size& aSize,T* _pShape)
        {
            OSL_ENSURE(aSize.Width > 0 && aSize.Height > 0,"Illegal with or height!");

            ::osl::MutexGuard aGuard(_pShape->m_aMutex);
            if ( _pShape->m_aProps.aComponent.m_xShape.is() )
            {
                ::com::sun::star::awt::Size aOldSize = _pShape->m_aProps.aComponent.m_xShape->getSize();
                _pShape->m_aProps.aComponent.m_nWidth = aOldSize.Width;
                _pShape->m_aProps.aComponent.m_nHeight = aOldSize.Height;
                _pShape->m_aProps.aComponent.m_xShape->setSize(aSize);
            }
            _pShape->set(PROPERTY_WIDTH,aSize.Width,_pShape->m_aProps.aComponent.m_nWidth);
            _pShape->set(PROPERTY_HEIGHT,aSize.Height,_pShape->m_aProps.aComponent.m_nHeight);
        }
        template<typename T> static ::com::sun::star::awt::Size getSize( T* _pShape )
        {
            ::osl::MutexGuard aGuard(_pShape->m_aMutex);
            if ( _pShape->m_aProps.aComponent.m_xShape.is() )
            {
                ::com::sun::star::awt::Size aSize = _pShape->m_aProps.aComponent.m_xShape->getSize();
                OSL_ENSURE(aSize.Width > 0 && aSize.Height > 0,"Illegal with or height!");
                return aSize;
            }
            return ::com::sun::star::awt::Size(_pShape->m_aProps.aComponent.m_nWidth,_pShape->m_aProps.aComponent.m_nHeight);
        }

        template<typename T> static void setPosition( const ::com::sun::star::awt::Point& aPosition ,T* _pShape)
        {
            OSL_ENSURE(aPosition.X >= 0 && aPosition.Y >= 0,"Illegal position!");
            ::osl::MutexGuard aGuard(_pShape->m_aMutex);
            if ( _pShape->m_aProps.aComponent.m_xShape.is() )
            {
                ::com::sun::star::awt::Point aOldPos = _pShape->m_aProps.aComponent.m_xShape->getPosition();
                _pShape->m_aProps.aComponent.m_nPosX = aOldPos.X;
                _pShape->m_aProps.aComponent.m_nPosY = aOldPos.Y;
                _pShape->m_aProps.aComponent.m_xShape->setPosition(aPosition);
            }
            _pShape->set(PROPERTY_POSITIONX,aPosition.X,_pShape->m_aProps.aComponent.m_nPosX);
            _pShape->set(PROPERTY_POSITIONY,aPosition.Y,_pShape->m_aProps.aComponent.m_nPosY);
        }
        template<typename T> static ::com::sun::star::awt::Point getPosition(T* _pShape)
        {
            ::osl::MutexGuard aGuard(_pShape->m_aMutex);
            if ( _pShape->m_aProps.aComponent.m_xShape.is() )
            {
                ::com::sun::star::awt::Point aPosition = _pShape->m_aProps.aComponent.m_xShape->getPosition();
                OSL_ENSURE(aPosition.X >= 0 && aPosition.Y >= 0,"Illegal position!");
                return aPosition;
            }
            return ::com::sun::star::awt::Point(_pShape->m_aProps.aComponent.m_nPosX,_pShape->m_aProps.aComponent.m_nPosY);
        }
        template<typename T> static void setParent( const com::sun::star::uno::Reference< com::sun::star::uno::XInterface >& Parent, T* _pShape)
        {
            ::osl::MutexGuard aGuard(_pShape->m_aMutex);
            _pShape->m_aProps.aComponent.m_xParent = ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >(Parent,::com::sun::star::uno::UNO_QUERY);
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild > xChild;
            comphelper::query_aggregation(_pShape->m_aProps.aComponent.m_xProxy,xChild);
            if ( xChild.is() )
                xChild->setParent(Parent);
        }
        template<typename T> static com::sun::star::uno::Reference< com::sun::star::uno::XInterface > getParent( T* _pShape )
        {
            ::osl::MutexGuard aGuard(_pShape->m_aMutex);
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild > xChild;
            comphelper::query_aggregation(_pShape->m_aProps.aComponent.m_xProxy,xChild);
            if ( xChild.is() )
                    return xChild->getParent();
            return _pShape->m_aProps.aComponent.m_xParent;
        }
    };
// =============================================================================
} // namespace reportdesign
// =============================================================================
#endif // REPORTDESIGN_TOOLS_HXX

