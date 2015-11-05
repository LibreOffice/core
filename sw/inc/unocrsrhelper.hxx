/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_INC_UNOCRSRHELPER_HXX
#define INCLUDED_SW_INC_UNOCRSRHELPER_HXX

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>

#include <swtypes.hxx>
#include <flyenum.hxx>
#include <pam.hxx>

#include <map>

class SfxItemSet;
class SfxItemPropertySet;
struct SfxItemPropertySimpleEntry;
class SdrObject;
class SwTextNode;
class SwCursor;
class SwUnoCrsr;
class SwUnoTableCrsr;
class SwFormatColl;
struct SwSortOptions;
class SwDoc;

namespace sw { namespace mark { class IMark; } }

namespace com{ namespace sun{ namespace star{
    namespace uno{
        class Any;
    }
    namespace beans{
        struct PropertyValue;
    }
    namespace text {
        class XTextContent;
        class XFlatParagraphIterator;
    }
}}}

enum SwGetPropertyStatesCaller
{
    SW_PROPERTY_STATE_CALLER_DEFAULT,
    SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION,
    SW_PROPERTY_STATE_CALLER_SINGLE_VALUE_ONLY,
    SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION_TOLERANT
};

namespace SwUnoCursorHelper
{
    class SwAnyMapHelper
    {
        private:
            //  keep Any's mapped by (WhichId << 16 ) + (MemberId)
            std::map<sal_uInt32, css::uno::Any> m_Map;

        public:
            void    SetValue( sal_uInt16 nWhichId, sal_uInt16 nMemberId, const css::uno::Any& rAny );
            bool    FillValue( sal_uInt16 nWhichId, sal_uInt16 nMemberId, const css::uno::Any*& pAny );
    };

    css::uno::Reference< css::text::XTextContent >
        GetNestedTextContent(SwTextNode & rTextNode, sal_Int32 const nIndex,
            bool const bParent);

    bool                    getCrsrPropertyValue(const SfxItemPropertySimpleEntry& rEntry
                                        , SwPaM& rPam
                                        , css::uno::Any *pAny
                                        , css::beans::PropertyState& eState
                                        , const SwTextNode* pNode = 0 );

    void                        GetCurPageStyle(SwPaM& rPaM, OUString &rString);

    inline bool             IsStartOfPara(SwPaM& rUnoCrsr)
                                        { return rUnoCrsr.GetPoint()->nContent == 0;}
    inline bool             IsEndOfPara(SwPaM& rUnoCrsr)
                                        { return rUnoCrsr.GetContentNode() &&
                                            rUnoCrsr.GetPoint()->nContent == rUnoCrsr.GetContentNode()->Len();}

    void                        resetCrsrPropertyValue(const SfxItemPropertySimpleEntry& rEntry, SwPaM& rPam);
    void                        InsertFile(SwUnoCrsr* pUnoCrsr,
                                    const OUString& rURL,
                                    const css::uno::Sequence< css::beans::PropertyValue >& rOptions)
        throw (css::lang::IllegalArgumentException,
               css::io::IOException,
               css::uno::RuntimeException,
               std::exception);

    void                        getNumberingProperty(
                                    SwPaM& rPam,
                                    css::beans::PropertyState& eState,
                                    css::uno::Any *pAny );

    void                        setNumberingProperty(
                                    const css::uno::Any& rValue,
                                    SwPaM& rPam);

    sal_Int16                   IsNodeNumStart(
                                    SwPaM& rPam,
                                    css::beans::PropertyState& eState);

    bool    DocInsertStringSplitCR(  SwDoc &rDoc,
                    const SwPaM &rNewCursor, const OUString &rText,
                    const bool bForceExpandHints );
    void    makeRedline( SwPaM& rPaM, const OUString& RedlineType,
            const css::uno::Sequence< css::beans::PropertyValue >& RedlineProperties )
                throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    void    makeTableRowRedline( SwTableLine& rTableLine, const OUString& RedlineType,
            const css::uno::Sequence< css::beans::PropertyValue >& RedlineProperties )
                throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    SW_DLLPUBLIC void    makeTableCellRedline( SwTableBox& rTableBox, const OUString& RedlineType,
            const css::uno::Sequence< css::beans::PropertyValue >& RedlineProperties )
                throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    /// @param bTableMode: attributes should be applied to a table selection
    void SetCrsrAttr(SwPaM & rPam, const SfxItemSet & rSet,
                     const SetAttrMode nAttrMode,
                     const bool bTableMode = false);
    void GetCrsrAttr(SwPaM & rPam, SfxItemSet & rSet,
                     const bool bOnlyTextAttr = false,
                     const bool bGetFromChrFormat = true);
    void GetTextFromPam(SwPaM & rPam, OUString & rBuffer);
    SwFormatColl * GetCurTextFormatColl(SwPaM & rPam, const bool bConditional);

    void SelectPam(SwPaM & rPam, const bool bExpand);
    void SetString(SwCursor & rCursor, const OUString & rString);

    css::uno::Sequence< css::beans::PropertyValue >
           CreateSortDescriptor(const bool bFromTable);
    bool ConvertSortProperties(
            const css::uno::Sequence< css::beans::PropertyValue >& rDescriptor,
            SwSortOptions & rSortOpt);

    /// @param bTableMode: attributes should be applied to a table selection
    void SetPropertyValue(
            SwPaM& rPaM,
            const SfxItemPropertySet & rPropSet,
            const OUString & rPropertyName,
            const css::uno::Any & rValue,
            const SetAttrMode nAttrMode = SetAttrMode::DEFAULT,
            const bool bTableMode = false)
        throw (css::beans::UnknownPropertyException,
                css::beans::PropertyVetoException,
                css::lang::IllegalArgumentException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException);
    /// @param bTableMode: attributes should be applied to a table selection
    void SetPropertyValues(
            SwPaM& rPaM,
            const SfxItemPropertySet & rPropSet,
            const css::uno::Sequence< css::beans::PropertyValue > &
            rPropertyValues,
            const SetAttrMode nAttrMode = SetAttrMode::DEFAULT,
            const bool bTableMode = false)
        throw (css::beans::UnknownPropertyException,
               css::beans::PropertyVetoException,
               css::lang::IllegalArgumentException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException,
               std::exception);
    css::uno::Any  GetPropertyValue(
            SwPaM& rPaM,
            const SfxItemPropertySet & rPropSet,
            const OUString & rPropertyName)
        throw (css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException,
               std::exception);
    css::uno::Sequence< css::beans::PropertyState > GetPropertyStates(
            SwPaM & rPaM,
            const SfxItemPropertySet & rPropSet,
            const css::uno::Sequence< OUString >&
                rPropertyNames,
            const SwGetPropertyStatesCaller eCaller =
                SW_PROPERTY_STATE_CALLER_DEFAULT)
        throw (css::beans::UnknownPropertyException,
               css::uno::RuntimeException,
               std::exception);
    css::beans::PropertyState GetPropertyState(
            SwPaM & rPaM,
            const SfxItemPropertySet & rPropSet,
            const OUString & rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::uno::RuntimeException);
    void SetPropertyToDefault(
            SwPaM & rPaM,
            const SfxItemPropertySet & rPropSet,
            const OUString & rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::uno::RuntimeException, std::exception);
    css::uno::Any  GetPropertyDefault(
            SwPaM & rPaM,
            const SfxItemPropertySet & rPropSet,
            const OUString & rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException);

    bool SetPageDesc(
            const css::uno::Any& rValue,
            SwDoc & rDoc, SfxItemSet & rSet);
    void SetTextFormatColl(const css::uno::Any & rAny, SwPaM & rPaM)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception);
    bool SetCursorPropertyValue(
            SfxItemPropertySimpleEntry const& rEntry,
            css::uno::Any const& rValue,
            SwPaM & rPam, SfxItemSet & rItemSet)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException,
               css::uno::DeploymentException, std::exception);

    /// try to get something that can be selected out of the XInterface
    /// at most one of the out parameters gets assigned a non-null value
    /// o_rpPaM is newly allocated and must be deleted; other parameters not
    SW_DLLPUBLIC void GetSelectableFromAny(
        css::uno::Reference<css::uno::XInterface> const& xIfc,
        SwDoc & rTargetDoc,
        SwPaM *& o_rpPaM, std::pair<OUString, FlyCntType> & o_rFrame,
        OUString & o_rTableName, SwUnoTableCrsr const*& o_rpTableCursor,
        ::sw::mark::IMark const*& o_rpMark,
        std::vector<SdrObject *> & o_rSdrObjects);

    css::uno::Reference<css::text::XFlatParagraphIterator>
            CreateFlatParagraphIterator(SwDoc &, sal_Int32,  bool);

} // namespace SwUnoCursorHelper

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
