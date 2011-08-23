/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_SHAPEUNO_HXX
#define SC_SHAPEUNO_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_ 
#include <cppuhelper/weak.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace uno {
        class XAggregation;
    }
    namespace drawing {
        class XShape;
    }
}}}
struct SvEventDescription;
namespace binfilter {

class SdrObject;

//------------------------------------------------------------------------

//	object which aggregates all svx shape objects,
//	to add own properties

class ScShapeObj : public ::cppu::OWeakObject,
                    public ::com::sun::star::beans::XPropertySet,
                    public ::com::sun::star::beans::XPropertyState,
                    public ::com::sun::star::text::XTextContent,
                    public ::com::sun::star::text::XText,
                    public ::com::sun::star::lang::XTypeProvider
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation > mxShapeAgg;
    ::com::sun::star::uno::Sequence< sal_Int8 >*                              pImplementationId;
    BOOL																	bIsTextShape;

    SdrObject* GetSdrObject() const throw();

public:
    static const SvEventDescription* GetSupportedMacroItems();

                            // ctor modifies xShape parameter
                            ScShapeObj( ::com::sun::star::uno::Reference<
                                            ::com::sun::star::drawing::XShape > & xShape );
    virtual					~ScShapeObj();

                            // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	acquire() throw();
    virtual void SAL_CALL	release() throw();

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL
                            getPropertyStates( const ::com::sun::star::uno::Sequence<
                                        ::rtl::OUString >& aPropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setPropertyToDefault( const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault(
                                    const ::rtl::OUString& aPropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XTextContent
    virtual void SAL_CALL	attach(const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextRange > & xTextRange)
                                throw( ::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getAnchor(void) throw( ::com::sun::star::uno::RuntimeException );

                            // XComponent
    virtual void SAL_CALL	dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL	addEventListener(const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::lang::XEventListener > & aListener)
                                throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL	removeEventListener(const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::lang::XEventListener > & aListener)
                                throw( ::com::sun::star::uno::RuntimeException );

                            // XText
    virtual void SAL_CALL	insertTextContent( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextRange >& xRange,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextContent >& xContent,
                                sal_Bool bAbsorb )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeTextContent( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::text::XTextContent >& xContent )
                                    throw(::com::sun::star::container::NoSuchElementException,
                                            ::com::sun::star::uno::RuntimeException);

                            // XSimpleText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursorByRange( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& aTextPosition )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	insertString( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        const ::rtl::OUString& aString, sal_Bool bAbsorb )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	insertControlCharacter( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getEnd() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setString( const ::rtl::OUString& aString )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);
};

} //namespace binfilter
#endif

