/*************************************************************************
 *
 *  $RCSfile: idhelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-23 14:47:31 $
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

#ifndef _UNOTOOLS_ID_HELPER_HXX_
#define _UNOTOOLS_ID_HELPER_HXX_

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.........................................................................
namespace utl
{
//.........................................................................

//=========================================================================
// to shorten some lines ...
typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  TypeSequence;

// compare to Sequences of Types
struct TypeSequenceLess : public ::std::binary_function<TypeSequence, TypeSequence, bool>
{
public:
    inline bool operator() (const TypeSequence& lhs, const TypeSequence& rhs) const
    {
        sal_Int32 nLengthLeft = lhs.getLength();
        sal_Int32 nLengthRight = rhs.getLength();

        // first check the two lengths
        if (nLengthLeft < nLengthRight)
            return sal_True;
        if (nLengthLeft > nLengthRight)
            return sal_False;

        // both sequences have the same length -> check the type names
        const ::com::sun::star::uno::Type* pTypesLeft = lhs.getConstArray();
        const ::com::sun::star::uno::Type* pTypesRight = rhs.getConstArray();
        for (sal_Int32 i=0; i<nLengthLeft; ++i, ++pTypesLeft, ++pTypesRight)
        {
            sal_Int32 nTypeNameCompare = pTypesLeft->getTypeName().compareTo(pTypesRight->getTypeName());
            if (nTypeNameCompare < 0)
                return sal_True;
            if (nTypeNameCompare > 0)
                return sal_False;
        }

        // both sequences are equal ...
        return sal_False;
    }
};

// declare the map
DECLARE_STL_MAP (   TypeSequence,
                    ::cppu::OImplementationId,
                    TypeSequenceLess,
                    MapType2Id
                );

//.........................................................................
}   // namespace utl
//.........................................................................

//=========================================================================
/** defines a helper class for implementing the XTypeProvider::getImplementationId.
    it maps sequences of ::com::sun::star::uno::Type to implementation ids
    (which means sequences of bytes).<BR>
    As there is no possibility to determine the time where the id's are no longer
    needed (e.g. because the last instance of the class using this mechanism died)
    the helper is "refcounted", i.e. there are acquire and release methods.
    To simplify this there is a class classname##Ref which you may want to
    use as an member of your classes.
    <BR><BR>
    As we don't want a global helper class which handles implementation id's
    of components from all over the office (supposing somebody want's to use this :)
    this is only a define. Wherever you have a "closed" area (which is small enough
    and large enough :), see below) where diffenrent components want to use an id helper,
    define your own one with this macro.<BR>
    The more classes use this helper, the later redundant map entries will be
    cleared. The less classes use it, the earlier map entries which may have
    been reused will be cleared.
*/
#define DECLARE_IMPLEMENTATIONID_HELPER(_namespace, classname)      \
namespace _namespace {                                              \
class classname                                                     \
{                                                                   \
    friend class classname##Ref;                                \
                                                                    \
    static sal_Int32    s_nReferenced;                              \
    static void*        s_pMap;                                     \
                                                                    \
    static ::osl::Mutex s_aMutex;                                   \
                                                                    \
public:                                                             \
    static void acquire();                                          \
    static void release();                                          \
                                                                    \
    static ::com::sun::star::uno::Sequence< sal_Int8 > getImplementationId( \
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider >& _rxProvider);  \
                                                                    \
    static ::com::sun::star::uno::Sequence< sal_Int8 > getImplementationId( \
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >& _rTypes); \
                                                                    \
private:                                                            \
    static void implCreateMap();                                    \
                                                                    \
    classname() { }                                                 \
};                                                                  \
                                                                    \
/*=======================================================================*/ \
class classname##Ref                                                \
{                                                                   \
public:                                                             \
    classname##Ref() { classname::acquire(); }  \
    ~classname##Ref() { classname::release(); } \
};                                                                  \
                                                                    \
}   /* _namespace */                                                \
                                                                    \

/*************************************************************************
**************************************************************************
*************************************************************************/

/** implement an id helper
*/
#define IMPLEMENT_IMPLEMENTATIONID_HELPER(_namespace, classname)        \
namespace _namespace {  \
    \
/*=======================================================================*/ \
    \
sal_Int32       classname::s_nReferenced(0);    \
void*           classname::s_pMap = NULL;   \
::osl::Mutex    classname::s_aMutex;    \
    \
/*-----------------------------------------------------------------------*/ \
void classname::acquire()   \
{   \
    ::osl::MutexGuard aGuard(s_aMutex); \
    ++s_nReferenced;    \
}   \
    \
/*-----------------------------------------------------------------------*/ \
void classname::release()   \
{   \
    ::osl::MutexGuard aGuard(s_aMutex); \
    if (!--s_nReferenced)   \
    {   \
        delete s_pMap;  \
        s_pMap = NULL;  \
    }   \
}   \
    \
/*-----------------------------------------------------------------------*/ \
::com::sun::star::uno::Sequence< sal_Int8 > classname::getImplementationId( \
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider >& _rxProvider)   \
{   \
    ::osl::MutexGuard aGuard(s_aMutex); \
    if (_rxProvider.is())   \
        return getImplementationId(_rxProvider->getTypes());    \
    return ::com::sun::star::uno::Sequence< sal_Int8 >();   \
}   \
    \
/*-----------------------------------------------------------------------*/ \
::com::sun::star::uno::Sequence< sal_Int8 > classname::getImplementationId( \
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >& _rTypes)  \
{   \
    ::osl::MutexGuard aGuard(s_aMutex); \
    DBG_ASSERT(s_nReferenced,   \
        "classname::getImplementationId : you need to hold a reference to this class in order to use it !");    \
        /* give the calling class a member of type classname##Ref and all is fine .... */   \
    \
    implCreateMap();    \
    \
    ::utl::MapType2Id* pMap = reinterpret_cast< ::utl::MapType2Id *>(s_pMap);   \
    \
    ::cppu::OImplementationId& rId = (*pMap)[_rTypes];  \
    /* this will create an entry for the given type sequence, if neccessary */  \
    \
    return rId.getImplementationId();   \
}   \
    \
/*-----------------------------------------------------------------------*/ \
void classname::implCreateMap() \
{   \
    if (s_pMap) \
        return; \
    s_pMap = new ::utl::MapType2Id();   \
}   \
    \
    \
}   /* _namespace */    \
    \


#endif // _UNOTOOLS_ID_HELPER_HXX_

