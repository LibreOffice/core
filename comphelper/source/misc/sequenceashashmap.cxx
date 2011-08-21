/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

//_______________________________________________
// includes
#include <comphelper/sequenceashashmap.hxx>

//_______________________________________________
// namespace

namespace comphelper{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

/*-----------------------------------------------
    04.11.2003 09:29
-----------------------------------------------*/
SequenceAsHashMap::SequenceAsHashMap()
    : SequenceAsHashMapBase()
{
}

/*-----------------------------------------------
    04.11.2003 08:30
-----------------------------------------------*/
SequenceAsHashMap::SequenceAsHashMap(const css::uno::Any& aSource)
{
    (*this) << aSource;
}

//-----------------------------------------------
SequenceAsHashMap::SequenceAsHashMap(const css::uno::Sequence< css::uno::Any >& lSource)
{
    (*this) << lSource;
}

/*-----------------------------------------------
    04.11.2003 08:30
-----------------------------------------------*/
SequenceAsHashMap::SequenceAsHashMap(const css::uno::Sequence< css::beans::PropertyValue >& lSource)
{
    (*this) << lSource;
}

/*-----------------------------------------------
    04.11.2003 08:30
-----------------------------------------------*/
SequenceAsHashMap::SequenceAsHashMap(const css::uno::Sequence< css::beans::NamedValue >& lSource)
{
    (*this) << lSource;
}

/*-----------------------------------------------
    04.11.2003 09:04
-----------------------------------------------*/
SequenceAsHashMap::~SequenceAsHashMap()
{
}

/*-----------------------------------------------
    04.11.2003 10:21
-----------------------------------------------*/
void SequenceAsHashMap::operator<<(const css::uno::Any& aSource)
{
    // An empty Any reset this instance!
    if (!aSource.hasValue())
    {
        clear();
        return;
    }

    css::uno::Sequence< css::beans::NamedValue > lN;
    if (aSource >>= lN)
    {
        (*this) << lN;
        return;
    }

    css::uno::Sequence< css::beans::PropertyValue > lP;
    if (aSource >>= lP)
    {
        (*this) << lP;
        return;
    }

    throw css::beans::IllegalTypeException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Any contains wrong type." )),
            css::uno::Reference< css::uno::XInterface >());
}

//-----------------------------------------------
void SequenceAsHashMap::operator<<(const css::uno::Sequence< css::uno::Any >& lSource)
{
    sal_Int32 c = lSource.getLength();
    sal_Int32 i = 0;

    for (i=0; i<c; ++i)
    {
        css::beans::PropertyValue lP;
        if (lSource[i] >>= lP)
        {
            if (
                (!lP.Name.getLength()) ||
                (!lP.Value.hasValue())
               )
                throw css::beans::IllegalTypeException(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PropertyValue struct contains no usefull informations." )),
                        css::uno::Reference< css::uno::XInterface >());
            (*this)[lP.Name] = lP.Value;
            continue;
        }

        css::beans::NamedValue lN;
        if (lSource[i] >>= lN)
        {
            if (
                (!lN.Name.getLength()) ||
                (!lN.Value.hasValue())
               )
                throw css::beans::IllegalTypeException(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NamedValue struct contains no usefull informations." )),
                        css::uno::Reference< css::uno::XInterface >());
            (*this)[lN.Name] = lN.Value;
            continue;
        }

        // ignore VOID Any ... but reject wrong filled ones!
        if (lSource[i].hasValue())
            throw css::beans::IllegalTypeException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Any contains wrong type." )),
                    css::uno::Reference< css::uno::XInterface >());
    }
}

/*-----------------------------------------------
    04.11.2003 08:30
-----------------------------------------------*/
void SequenceAsHashMap::operator<<(const css::uno::Sequence< css::beans::PropertyValue >& lSource)
{
    clear();

          sal_Int32                  c       = lSource.getLength();
    const css::beans::PropertyValue* pSource = lSource.getConstArray();

    for (sal_Int32 i=0; i<c; ++i)
        (*this)[pSource[i].Name] = pSource[i].Value;
}

/*-----------------------------------------------
    04.11.2003 08:30
-----------------------------------------------*/
void SequenceAsHashMap::operator<<(const css::uno::Sequence< css::beans::NamedValue >& lSource)
{
    clear();

          sal_Int32               c       = lSource.getLength();
    const css::beans::NamedValue* pSource = lSource.getConstArray();

    for (sal_Int32 i=0; i<c; ++i)
        (*this)[pSource[i].Name] = pSource[i].Value;
}

/*-----------------------------------------------
    04.11.2003 08:30
-----------------------------------------------*/
void SequenceAsHashMap::operator>>(css::uno::Sequence< css::beans::PropertyValue >& lDestination) const
{
    sal_Int32 c = (sal_Int32)size();
    lDestination.realloc(c);
    css::beans::PropertyValue* pDestination = lDestination.getArray();

    sal_Int32 i = 0;
    for (const_iterator pThis  = begin();
                        pThis != end()  ;
                      ++pThis           )
    {
        pDestination[i].Name  = pThis->first ;
        pDestination[i].Value = pThis->second;
        ++i;
    }
}

/*-----------------------------------------------
    04.11.2003 08:30
-----------------------------------------------*/
void SequenceAsHashMap::operator>>(css::uno::Sequence< css::beans::NamedValue >& lDestination) const
{
    sal_Int32 c = (sal_Int32)size();
    lDestination.realloc(c);
    css::beans::NamedValue* pDestination = lDestination.getArray();

    sal_Int32 i = 0;
    for (const_iterator pThis  = begin();
                        pThis != end()  ;
                      ++pThis           )
    {
        pDestination[i].Name  = pThis->first ;
        pDestination[i].Value = pThis->second;
        ++i;
    }
}

/*-----------------------------------------------
    30.07.2007 14:10
-----------------------------------------------*/
const css::uno::Any SequenceAsHashMap::getAsConstAny(::sal_Bool bAsPropertyValueList) const
{
    css::uno::Any aDestination;
    if (bAsPropertyValueList)
        aDestination = css::uno::makeAny(getAsConstPropertyValueList());
    else
        aDestination = css::uno::makeAny(getAsConstNamedValueList());
    return aDestination;
}

/*-----------------------------------------------
    30.07.2007 14:10
-----------------------------------------------*/
const css::uno::Sequence< css::uno::Any > SequenceAsHashMap::getAsConstAnyList(::sal_Bool bAsPropertyValueList) const
{
    ::sal_Int32                         i            = 0;
    ::sal_Int32                         c            = (::sal_Int32)size();
    css::uno::Sequence< css::uno::Any > lDestination(c);
    css::uno::Any*                      pDestination = lDestination.getArray();

    for (const_iterator pThis  = begin();
                        pThis != end()  ;
                      ++pThis           )
    {
        if (bAsPropertyValueList)
        {
            css::beans::PropertyValue aProp;
            aProp.Name      = pThis->first;
            aProp.Value     = pThis->second;
            pDestination[i] = css::uno::makeAny(aProp);
        }
        else
        {
            css::beans::NamedValue aProp;
            aProp.Name      = pThis->first;
            aProp.Value     = pThis->second;
            pDestination[i] = css::uno::makeAny(aProp);
        }

        ++i;
    }

    return lDestination;
}

/*-----------------------------------------------
    04.11.2003 08:30
-----------------------------------------------*/
const css::uno::Sequence< css::beans::NamedValue > SequenceAsHashMap::getAsConstNamedValueList() const
{
    css::uno::Sequence< css::beans::NamedValue > lReturn;
    (*this) >> lReturn;
    return lReturn;
}

/*-----------------------------------------------
    04.11.2003 08:30
-----------------------------------------------*/
const css::uno::Sequence< css::beans::PropertyValue > SequenceAsHashMap::getAsConstPropertyValueList() const
{
    css::uno::Sequence< css::beans::PropertyValue > lReturn;
    (*this) >> lReturn;
    return lReturn;
}

/*-----------------------------------------------
    07.03.2007 12:45
-----------------------------------------------*/
sal_Bool SequenceAsHashMap::match(const SequenceAsHashMap& rCheck) const
{
    const_iterator pCheck;
    for (  pCheck  = rCheck.begin();
           pCheck != rCheck.end()  ;
         ++pCheck                  )
    {
        const ::rtl::OUString& sCheckName  = pCheck->first;
        const css::uno::Any&   aCheckValue = pCheck->second;
              const_iterator   pFound      = find(sCheckName);

        if (pFound == end())
            return sal_False;

        const css::uno::Any& aFoundValue = pFound->second;
        if (aFoundValue != aCheckValue)
            return sal_False;
    }

    return sal_True;
}

/*-----------------------------------------------
    30.07.2007 14:30
-----------------------------------------------*/
void SequenceAsHashMap::update(const SequenceAsHashMap& rUpdate)
{
    const_iterator pUpdate;
    for (  pUpdate  = rUpdate.begin();
           pUpdate != rUpdate.end()  ;
         ++pUpdate                   )
    {
        const ::rtl::OUString& sName  = pUpdate->first;
        const css::uno::Any&   aValue = pUpdate->second;

        (*this)[sName] = aValue;
    }
}

/*-----------------------------------------------
    04.11.2003 08:30
-----------------------------------------------*/
#if OSL_DEBUG_LEVEL > 1
void SequenceAsHashMap::dbg_dumpToFile(const char* pFileName,
                                       const char* pComment ) const
{
    if (!pFileName || !pComment)
        return;

    FILE* pFile = fopen(pFileName, "a");
    if (!pFile)
        return;

    ::rtl::OUStringBuffer sBuffer(1000);
    sBuffer.appendAscii("\n----------------------------------------\n");
    sBuffer.appendAscii(pComment                                      );
    sBuffer.appendAscii("\n----------------------------------------\n");
    sal_Int32 i = 0;
    for (const_iterator pIt  = begin();
                        pIt != end()  ;
                      ++pIt           )
    {
        sBuffer.appendAscii("["       );
        sBuffer.append     (i++       );
        sBuffer.appendAscii("] "      );
        sBuffer.appendAscii("\""      );
        sBuffer.append     (pIt->first);
        sBuffer.appendAscii("\" = \"" );

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xv;
        ::rtl::OUString                                                       sv;
        sal_Int32                                                             nv;
        sal_Bool                                                              bv;

        if (pIt->second >>= sv)
            sBuffer.append(sv);
        else
        if (pIt->second >>= nv)
            sBuffer.append(nv);
        else
        if (pIt->second >>= bv)
            sBuffer.appendAscii(bv ? "true" : "false");
        else
        if (pIt->second >>= xv)
            sBuffer.appendAscii(xv.is() ? "object" : "null");
        else
            sBuffer.appendAscii("???");

        sBuffer.appendAscii("\"\n");
    }

    fprintf(pFile, ::rtl::OUStringToOString(sBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8).getStr());
    fclose(pFile);
}
#endif // OSL_DEBUG_LEVEL > 1

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
