/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
            ::rtl::OUString::createFromAscii("Any contains wrong type."),
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
                (lP.Name.isEmpty()) ||
                (!lP.Value.hasValue())
               )
                throw css::beans::IllegalTypeException(
                        ::rtl::OUString::createFromAscii("PropertyValue struct contains no usefull informations."),
                        css::uno::Reference< css::uno::XInterface >());
            (*this)[lP.Name] = lP.Value;
            continue;
        }

        css::beans::NamedValue lN;
        if (lSource[i] >>= lN)
        {
            if (
                (lN.Name.isEmpty()) ||
                (!lN.Value.hasValue())
               )
                throw css::beans::IllegalTypeException(
                        ::rtl::OUString::createFromAscii("NamedValue struct contains no usefull informations."),
                        css::uno::Reference< css::uno::XInterface >());
            (*this)[lN.Name] = lN.Value;
            continue;
        }

        // ignore VOID Any ... but reject wrong filled ones!
        if (lSource[i].hasValue())
            throw css::beans::IllegalTypeException(
                    ::rtl::OUString::createFromAscii("Any contains wrong type."),
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
