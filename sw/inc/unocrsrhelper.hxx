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
#ifndef _UNOCRSRHELPER_HXX
#define _UNOCRSRHELPER_HXX

#include <map>

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <swtypes.hxx>
#include <pam.hxx>


class String;
class SfxItemSet;
class SfxItemPropertySet;
struct SfxItemPropertySimpleEntry;
class SwTxtNode;
class SwPaM;
class SwCursor;
class SwUnoCrsr;
class SwFmtColl;
struct SwSortOptions;
class SwDoc;

namespace com{ namespace sun{ namespace star{
    namespace uno{
        class Any;
    }
    namespace beans{
        struct PropertyValue;
    }
    namespace text {
        class XTextContent;
    }
}}}

/* -----------------03.12.98 12:16-------------------
 *
 * --------------------------------------------------*/
enum SwGetPropertyStatesCaller
{
    SW_PROPERTY_STATE_CALLER_DEFAULT,
    SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION,
    SW_PROPERTY_STATE_CALLER_SINGLE_VALUE_ONLY,
    SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION_TOLERANT
};

/* -----------------------------14.12.00 15:06--------------------------------

 ---------------------------------------------------------------------------*/
namespace SwUnoCursorHelper
{
    //  keep Any's mapped by (WhichId << 16 ) + (MemberId)
    typedef std::map< sal_uInt32, com::sun::star::uno::Any *> AnyMapHelper_t;
    class SwAnyMapHelper : public AnyMapHelper_t
    {
        public:
            ~SwAnyMapHelper();

            void    SetValue( sal_uInt16 nWhichId, sal_uInt16 nMemberId, const com::sun::star::uno::Any& rAny );
            bool    FillValue( sal_uInt16 nWhichId, sal_uInt16 nMemberId, const com::sun::star::uno::Any*& pAny );
    };

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >
        GetNestedTextContent(SwTxtNode & rTextNode, xub_StrLen const nIndex,
            bool const bParent);

    sal_Bool                    getCrsrPropertyValue(const SfxItemPropertySimpleEntry& rEntry
                                        , SwPaM& rPam
                                        , com::sun::star::uno::Any *pAny
                                        , com::sun::star::beans::PropertyState& eState
                                        , const SwTxtNode* pNode = 0 );

    void                        GetCurPageStyle(SwPaM& rPaM, String &rString);

    inline sal_Bool             IsStartOfPara(SwPaM& rUnoCrsr)
                                        { return rUnoCrsr.GetPoint()->nContent == 0;}
    inline sal_Bool             IsEndOfPara(SwPaM& rUnoCrsr)
                                        { return rUnoCrsr.GetCntntNode() &&
                                            rUnoCrsr.GetPoint()->nContent == rUnoCrsr.GetCntntNode()->Len();}

    void                        resetCrsrPropertyValue(const SfxItemPropertySimpleEntry& rEntry, SwPaM& rPam);
    void                        InsertFile(SwUnoCrsr* pUnoCrsr,
                                    const String& rURL,
                                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rOptions
                                    ) throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException );

    void                        getNumberingProperty(
                                    SwPaM& rPam,
                                    com::sun::star::beans::PropertyState& eState,
                                    com::sun::star::uno::Any *pAny );

    void                        setNumberingProperty(
                                    const com::sun::star::uno::Any& rValue,
                                    SwPaM& rPam);

    sal_Int16                   IsNodeNumStart(
                                    SwPaM& rPam,
                                    com::sun::star::beans::PropertyState& eState);

    sal_Bool    DocInsertStringSplitCR(  SwDoc &rDoc,
                    const SwPaM &rNewCursor, const String &rText,
                    const bool bForceExpandHints );
    void    makeRedline( SwPaM& rPaM, const ::rtl::OUString& RedlineType,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& RedlineProperties )
                throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);


    /// @param bTableMode: attributes should be applied to a table selection
    void SetCrsrAttr(SwPaM & rPam, const SfxItemSet & rSet,
                     const SetAttrMode nAttrMode,
                     const bool bTableMode = false);
    void GetCrsrAttr(SwPaM & rPam, SfxItemSet & rSet,
                     const bool bOnlyTxtAttr = false,
                     const bool bGetFromChrFmt = true);
    void GetTextFromPam(SwPaM & rPam, ::rtl::OUString & rBuffer);
    SwFmtColl * GetCurTxtFmtColl(SwPaM & rPam, const bool bConditional);

    void SelectPam(SwPaM & rPam, const bool bExpand);
    void SetString(SwCursor & rCursor, const ::rtl::OUString & rString);

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
           CreateSortDescriptor(const bool bFromTable);
    sal_Bool ConvertSortProperties(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue >& rDescriptor,
            SwSortOptions & rSortOpt);

    /// @param bTableMode: attributes should be applied to a table selection
    void SetPropertyValue(
            SwPaM& rPaM,
            const SfxItemPropertySet & rPropSet,
            const ::rtl::OUString & rPropertyName,
            const ::com::sun::star::uno::Any & rValue,
            const SetAttrMode nAttrMode = nsSetAttrMode::SETATTR_DEFAULT,
            const bool bTableMode = false)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any  GetPropertyValue(
            SwPaM& rPaM,
            const SfxItemPropertySet & rPropSet,
            const ::rtl::OUString & rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyState > GetPropertyStates(
            SwPaM & rPaM,
            const SfxItemPropertySet & rPropSet,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >&
                rPropertyNames,
            const SwGetPropertyStatesCaller eCaller =
                SW_PROPERTY_STATE_CALLER_DEFAULT)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::beans::PropertyState GetPropertyState(
            SwPaM & rPaM,
            const SfxItemPropertySet & rPropSet,
            const ::rtl::OUString & rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::uno::RuntimeException);
    void SetPropertyToDefault(
            SwPaM & rPaM,
            const SfxItemPropertySet & rPropSet,
            const ::rtl::OUString & rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any  GetPropertyDefault(
            SwPaM & rPaM,
            const SfxItemPropertySet & rPropSet,
            const ::rtl::OUString & rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    bool SetPageDesc(
            const ::com::sun::star::uno::Any& rValue,
            SwDoc & rDoc, SfxItemSet & rSet);
    void SetTxtFmtColl(const ::com::sun::star::uno::Any & rAny, SwPaM & rPaM)
        throw (::com::sun::star::lang::IllegalArgumentException);
    bool SetCursorPropertyValue(
            SfxItemPropertySimpleEntry const& rEntry,
            ::com::sun::star::uno::Any const& rValue,
            SwPaM & rPam, SfxItemSet & rItemSet)
        throw (::com::sun::star::lang::IllegalArgumentException);

} // namespace SwUnoCursorHelper

#endif
