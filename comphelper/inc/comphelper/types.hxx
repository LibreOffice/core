/*************************************************************************
 *
 *  $RCSfile: types.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COMPHELPER_TYPES_HXX_
#define _COMPHELPER_TYPES_HXX_

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif


namespace com { namespace sun { namespace star { namespace awt {
    struct FontDescriptor;
} } } }

//.........................................................................
namespace comphelper
{
//.........................................................................

    namespace staruno       = ::com::sun::star::uno;
    namespace starawt       = ::com::sun::star::awt;
    namespace starlang      = ::com::sun::star::lang;

    typedef staruno::Reference< staruno::XInterface >           InterfaceRef;
    typedef staruno::Sequence< ::rtl::OUString >                StringSequence;

    //-------------------------------------------------------------------------
    /** compare the two given Anys
        The comparison is deep, means if one of the Any's contains an Any which contains an Any ..., this is resolved <br/>
        Other types recognized currently : FontDescriptor, ::com::sun::star::util::Date/Tim/DateTime, staruno::Sequence<sal_Int8>
    */
    sal_Bool compare(const staruno::Any& rLeft, const staruno::Any& rRight);

    //-------------------------------------------------------------------------
    /** compare two FontDescriptor's
    */
            sal_Bool    operator ==(const starawt::FontDescriptor& _rLeft, const starawt::FontDescriptor& _rRight);
    inline  sal_Bool    operator !=(const starawt::FontDescriptor& _rLeft, const starawt::FontDescriptor& _rRight)
    {
        return !(_rLeft == _rRight);
    }

    //-------------------------------------------------------------------------
    /// returns sal_True if objects of the types given are "compatible"
    sal_Bool isAssignableFrom(const staruno::Type& _rAssignable, const staruno::Type& _rFrom);

    //-------------------------------------------------------------------------
    /** just a small shortcut ...
        check if a type you have at hand at runtime is equal to another type you have at compile time
        if all our compiler would accept function calls with explicit template arguments (like
        isA<classFoo>(runtimeType)), we wouldn't need the second parameter. But unfortunally at
        least the current solaris compiler doesn't allow this ....
        So this function is nearly senseless ....
    */
    template <class TYPE>
    sal_Bool isA(const staruno::Type& _rType, TYPE* pDummy)
    {
        return  _rType.equals(::getCppuType(pDummy));
    }

    //-------------------------------------------------------------------------
    /** check if a type you have at hand at runtime is equal to another type you have at compile time
        same comment as for the other isA ....
    */
    template <class TYPE>
    sal_Bool isA(const staruno::Any& _rVal, TYPE* pDummy)
    {
        return  _rVal.getValueType().equals(::getCppuType(pDummy));
    }

    //-------------------------------------------------------------------------
    /** check if a type you have at hand at runtime is equal to another type you have at compile time
    */
    template <class TYPE>
    sal_Bool isAReference(const staruno::Type& _rType, TYPE* pDummy)
    {
        return  _rType.equals(::getCppuType(reinterpret_cast<staruno::Reference<TYPE>*>(NULL)));
    }

    //-------------------------------------------------------------------------
    /** check if a type you have at hand at runtime is equal to another type you have at compile time
    */
    template <class TYPE>
    sal_Bool isAReference(const staruno::Any& _rVal, TYPE* pDummy)
    {
        return  _rVal.getValueType().equals(::getCppuType(reinterpret_cast<staruno::Reference<TYPE>*>(NULL)));
    }

    //-------------------------------------------------------------------------
    /** ask the given object for an XComponent interface and dispose on it
    */
    template <class TYPE>
    void disposeComponent(staruno::Reference<TYPE>& _rxComp)
    {
        staruno::Reference<starlang::XComponent> xComp(_rxComp, staruno::UNO_QUERY);
        if (xComp.is())
        {
            xComp->dispose();
            _rxComp = NULL;
        }
    }

    //-------------------------------------------------------------------------
    /** get a com::sun::star::awt::FontDescriptor that is fully initialized with
        the XXX_DONTKNOW enum values (which isn't the case if you instantiate it
        via the default constructor)
    */
    starawt::FontDescriptor getDefaultFont();

//=========================================================================
//= replacement of the former UsrAny.getXXX methods

    // may be used if you need the return value just as temporary, else it's may be too inefficient ....

    // no, we don't use templates here. This would lead to a lot of implicit uses of the conversion methods,
    // which would be difficult to trace ...

    sal_Int32       getINT32(const staruno::Any& _rAny);
    sal_Int16       getINT16(const staruno::Any& _rAny);
    double          getDouble(const staruno::Any& _rAny);
    float           getFloat(const staruno::Any& _rAny);
    ::rtl::OUString getString(const staruno::Any& _rAny);
    sal_Bool        getBOOL(const staruno::Any& _rAny);

    sal_Int32       getEnumAsINT32(const staruno::Any& _rAny) throw(starlang::IllegalArgumentException);

//= replacement of some former UsrAny.setXXX methods - can be used with rvalues
    inline void setBOOL(staruno::Any& _rAny, sal_Bool _b)
    { _rAny.setValue(&_b, ::getBooleanCppuType()); }

//= extension of ::cppu::makeAny()
    inline staruno::Any makeBoolAny(sal_Bool _b)
    { return staruno::Any(&_b, ::getBooleanCppuType()); }

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_TYPES_HXX_

