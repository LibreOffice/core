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

#ifndef _XMLOFF_XMLTEXTNUMRULEINFO_HXX
#define _XMLOFF_XMLTEXTNUMRULEINFO_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/container/XIndexReplace.hpp>

namespace com { namespace sun { namespace star {
    namespace text { class XTextContent; }
} } }
#include <sal/types.h>

class XMLTextListAutoStylePool;

/** information about list and list style for a certain paragraph

    OD 2008-04-24 #refactorlists#
    Complete refactoring of the class and enhancement of the class for lists.
    These changes are considered by method <XMLTextParagraphExport::exportListChange(..)>
*/
class XMLTextNumRuleInfo
{
    const ::rtl::OUString msNumberingRules;
    const ::rtl::OUString msNumberingLevel;
    const ::rtl::OUString msNumberingStartValue;
    const ::rtl::OUString msParaIsNumberingRestart;
    const ::rtl::OUString msNumberingIsNumber;
    const ::rtl::OUString msNumberingIsOutline;
    const ::rtl::OUString msPropNameListId;
    const ::rtl::OUString msPropNameStartWith;
    const ::rtl::OUString msContinueingPreviousSubTree;
    const ::rtl::OUString msListLabelStringProp;

    // numbering rules instance and its name
    ::com::sun::star::uno::Reference <
                        ::com::sun::star::container::XIndexReplace > mxNumRules;
    ::rtl::OUString     msNumRulesName;

    // paragraph's list attributes
    ::rtl::OUString     msListId;
    sal_Int16           mnListStartValue;
    sal_Int16           mnListLevel;
    sal_Bool            mbIsNumbered;
    sal_Bool            mbIsRestart;

    // numbering rules' attributes
    sal_Int16           mnListLevelStartValue;

    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
    sal_Bool mbOutlineStyleAsNormalListStyle;

    sal_Bool mbContinueingPreviousSubTree;
    ::rtl::OUString msListLabelString;

public:

    XMLTextNumRuleInfo();

    inline XMLTextNumRuleInfo& operator=( const XMLTextNumRuleInfo& rInfo );

    void Set( const ::com::sun::star::uno::Reference <
                        ::com::sun::star::text::XTextContent > & rTextContnt,
              const sal_Bool bOutlineStyleAsNormalListStyle,
              const XMLTextListAutoStylePool& rListAutoPool,
              const sal_Bool bExportTextNumberElement );
    inline void Reset();

    inline const ::rtl::OUString& GetNumRulesName() const
    {
        return msNumRulesName;
    }
    inline const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace >& GetNumRules() const
    {
        return mxNumRules;
    }
    inline sal_Int16 GetListLevelStartValue() const
    {
        return mnListLevelStartValue;
    }

    inline const ::rtl::OUString& GetListId() const
    {
        return msListId;
    }

    inline sal_Int16 GetLevel() const
    {
        return mnListLevel;
    }

    inline sal_Bool HasStartValue() const
    {
        return mnListStartValue != -1;
    }
    inline sal_uInt32 GetStartValue() const
    {
        return mnListStartValue;
    }

    inline sal_Bool IsNumbered() const
    {
        return mbIsNumbered;
    }
    inline sal_Bool IsRestart() const
    {
        return mbIsRestart;
    }

    sal_Bool BelongsToSameList( const XMLTextNumRuleInfo& rCmp ) const;

    inline sal_Bool HasSameNumRules( const XMLTextNumRuleInfo& rCmp ) const
    {
        return rCmp.msNumRulesName == msNumRulesName;
    }

    inline sal_Bool IsContinueingPreviousSubTree() const
    {
        return mbContinueingPreviousSubTree;
    }
    inline const ::rtl::OUString& ListLabelString() const
    {
        return msListLabelString;
    }
};

inline XMLTextNumRuleInfo& XMLTextNumRuleInfo::operator=(
        const XMLTextNumRuleInfo& rInfo )
{
    msNumRulesName = rInfo.msNumRulesName;
    mxNumRules = rInfo.mxNumRules;
    msListId = rInfo.msListId;
    mnListStartValue = rInfo.mnListStartValue;
    mnListLevel = rInfo.mnListLevel;
    mbIsNumbered = rInfo.mbIsNumbered;
    mbIsRestart = rInfo.mbIsRestart;
    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
    mbOutlineStyleAsNormalListStyle = rInfo.mbOutlineStyleAsNormalListStyle;
    mbContinueingPreviousSubTree = rInfo.mbContinueingPreviousSubTree;
    msListLabelString = rInfo.msListLabelString;

    return *this;
}

inline void XMLTextNumRuleInfo::Reset()
{
    mxNumRules = 0;
    msNumRulesName = ::rtl::OUString();
    msListId = ::rtl::OUString();
    mnListStartValue = -1;
    mnListLevel = 0;
    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
    mbIsNumbered = mbIsRestart =
    mbOutlineStyleAsNormalListStyle = sal_False;
    mbContinueingPreviousSubTree = sal_False;
    msListLabelString = ::rtl::OUString();
}
#endif  //  _XMLOFF_XMLTEXTNUMRULEINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
