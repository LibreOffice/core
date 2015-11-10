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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLTEXTNUMRULEINFO_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLTEXTNUMRULEINFO_HXX

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
    const OUString msNumberingRules;
    const OUString msNumberingLevel;
    const OUString msNumberingStartValue;
    const OUString msParaIsNumberingRestart;
    const OUString msNumberingIsNumber;
    const OUString msNumberingIsOutline;
    const OUString msPropNameListId;
    const OUString msPropNameStartWith;
    const OUString msContinueingPreviousSubTree;
    const OUString msListLabelStringProp;

    // numbering rules instance and its name
    ::com::sun::star::uno::Reference <
                        ::com::sun::star::container::XIndexReplace > mxNumRules;
    OUString     msNumRulesName;

    // paragraph's list attributes
    OUString     msListId;
    sal_Int16           mnListStartValue;
    sal_Int16           mnListLevel;
    bool            mbIsNumbered;
    bool            mbIsRestart;

    // numbering rules' attributes
    sal_Int16           mnListLevelStartValue;

    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
    bool mbOutlineStyleAsNormalListStyle;

    bool mbContinueingPreviousSubTree;
    OUString msListLabelString;

public:

    XMLTextNumRuleInfo();

    inline XMLTextNumRuleInfo& operator=( const XMLTextNumRuleInfo& rInfo );

    void Set( const ::com::sun::star::uno::Reference <
                        ::com::sun::star::text::XTextContent > & rTextContnt,
              const bool bOutlineStyleAsNormalListStyle,
              const XMLTextListAutoStylePool& rListAutoPool,
              const bool bExportTextNumberElement );
    inline void Reset();

    inline const OUString& GetNumRulesName() const
    {
        return msNumRulesName;
    }
    inline sal_Int16 GetListLevelStartValue() const
    {
        return mnListLevelStartValue;
    }

    inline const OUString& GetListId() const
    {
        return msListId;
    }

    inline sal_Int16 GetLevel() const
    {
        return mnListLevel;
    }

    inline bool HasStartValue() const
    {
        return mnListStartValue != -1;
    }
    inline sal_uInt32 GetStartValue() const
    {
        return mnListStartValue;
    }

    inline bool IsNumbered() const
    {
        return mbIsNumbered;
    }
    inline bool IsRestart() const
    {
        return mbIsRestart;
    }

    bool BelongsToSameList( const XMLTextNumRuleInfo& rCmp ) const;

    inline bool HasSameNumRules( const XMLTextNumRuleInfo& rCmp ) const
    {
        return rCmp.msNumRulesName == msNumRulesName;
    }

    inline bool IsContinueingPreviousSubTree() const
    {
        return mbContinueingPreviousSubTree;
    }
    inline const OUString& ListLabelString() const
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
    mxNumRules = nullptr;
    msNumRulesName.clear();
    msListId.clear();
    mnListStartValue = -1;
    mnListLevel = 0;
    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
    mbIsNumbered = mbIsRestart =
    mbOutlineStyleAsNormalListStyle = false;
    mbContinueingPreviousSubTree = false;
    msListLabelString.clear();
}
#endif // INCLUDED_XMLOFF_SOURCE_TEXT_XMLTEXTNUMRULEINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
